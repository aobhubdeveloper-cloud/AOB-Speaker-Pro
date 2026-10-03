package com.aobhub.speakerpro

import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Context
import android.content.Intent
import android.content.pm.ServiceInfo
import android.os.Build
import android.os.IBinder
import androidx.core.app.NotificationCompat
import androidx.core.app.ServiceCompat

class AudioReceiverService : Service() {
    private external fun nativeStart(port: Int): Boolean
    private external fun nativeStop()

    companion object {
        private const val PREFS = "receiver"
        private const val RUNNING = "running"
        private const val STATUS = "status"
        private const val CHANNEL = "aob"
        private const val NOTIFICATION_ID = 7

        fun isRunning(context: Context) = context.getSharedPreferences(PREFS, 0).getBoolean(RUNNING, false)
        fun status(context: Context) = context.getSharedPreferences(PREFS, 0).getString(STATUS, "Ready") ?: "Ready"

        private fun setState(context: Context, running: Boolean, status: String) {
            context.getSharedPreferences(PREFS, 0).edit()
                .putBoolean(RUNNING, running).putString(STATUS, status).apply()
        }
    }

    override fun onCreate() {
        super.onCreate()
        createNotificationChannel()
        try {
            System.loadLibrary("aob_audio")
        } catch (e: Throwable) {
            setState(this, false, "Native audio engine failed to load: ${e.javaClass.simpleName}")
            stopSelf()
            return
        }

        val notification = NotificationCompat.Builder(this, CHANNEL)
            .setContentTitle("AOB Speaker Pro")
            .setContentText("Android speaker receiver is active")
            .setSmallIcon(android.R.drawable.ic_media_play)
            .setOngoing(true)
            .setCategory(NotificationCompat.CATEGORY_SERVICE)
            .setPriority(NotificationCompat.PRIORITY_LOW)
            .build()

        try {
            if (Build.VERSION.SDK_INT >= 29) {
                ServiceCompat.startForeground(this, NOTIFICATION_ID, notification,
                    ServiceInfo.FOREGROUND_SERVICE_TYPE_MEDIA_PLAYBACK)
            } else {
                startForeground(NOTIFICATION_ID, notification)
            }
        } catch (e: Throwable) {
            setState(this, false, "Foreground service failed: ${e.javaClass.simpleName}")
            stopSelf()
        }
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        if (isRunning(this)) return START_STICKY
        val port = intent?.getIntExtra("port", 4677) ?: 4677
        setState(this, false, "Starting audio engine on UDP $port…")
        try {
            if (!nativeStart(port)) {
                setState(this, false, "Audio engine could not open the speaker stream.")
                stopSelf()
                return START_NOT_STICKY
            }
            setState(this, true, "Listening for PC audio on UDP $port")
        } catch (e: Throwable) {
            setState(this, false, "Audio engine error: ${e.javaClass.simpleName}")
            stopSelf()
            return START_NOT_STICKY
        }
        return START_STICKY
    }

    override fun onDestroy() {
        try { nativeStop() } catch (_: Throwable) { }
        setState(this, false, "Receiver stopped")
        ServiceCompat.stopForeground(this, ServiceCompat.STOP_FOREGROUND_REMOVE)
        super.onDestroy()
    }

    override fun onBind(intent: Intent?): IBinder? = null

    private fun createNotificationChannel() {
        getSystemService(NotificationManager::class.java).createNotificationChannel(
            NotificationChannel(CHANNEL, "AOB Audio Receiver", NotificationManager.IMPORTANCE_LOW)
        )
    }
}
