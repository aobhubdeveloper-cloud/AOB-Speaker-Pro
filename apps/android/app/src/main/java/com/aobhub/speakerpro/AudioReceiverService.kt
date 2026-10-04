package com.aobhub.speakerpro

import android.app.*
import android.content.Intent
import android.os.IBinder
import androidx.core.app.NotificationCompat
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.InetAddress
import java.util.concurrent.atomic.AtomicBoolean

class AudioReceiverService: Service() {
    private external fun nativeStart(port:Int):Boolean
    private external fun nativeStop()
    private val running = AtomicBoolean(false)
    private var discoveryThread: Thread? = null

    override fun onCreate() {
        super.onCreate()
        System.loadLibrary("aob_audio")
        val nm=getSystemService(NotificationManager::class.java)
        nm.createNotificationChannel(NotificationChannel("aob","AOB Audio",NotificationManager.IMPORTANCE_LOW))
        startForeground(7, NotificationCompat.Builder(this,"aob")
            .setContentTitle("AOB Speaker Pro")
            .setContentText("Waiting for Windows PC on Wi-Fi")
            .setSmallIcon(android.R.drawable.ic_media_play).build())

        if (!nativeStart(4677)) {
            stopSelf()
            return
        }
        running.set(true)
        startDiscoveryBeacon()
    }

    private fun startDiscoveryBeacon() {
        discoveryThread = Thread {
            try {
                DatagramSocket().use { socket ->
                    socket.broadcast = true
                    val payload = "AOB_ANDROID_HELLO_V1".toByteArray()
                    val packet = DatagramPacket(
                        payload, payload.size,
                        InetAddress.getByName("255.255.255.255"), 4678)
                    while (running.get()) {
                        socket.send(packet)
                        Thread.sleep(1000)
                    }
                }
            } catch (_: Exception) {}
        }.apply { isDaemon = true; start() }
    }

    override fun onDestroy() {
        running.set(false)
        discoveryThread?.interrupt()
        nativeStop()
        super.onDestroy()
    }
    override fun onBind(i:Intent?):IBinder?=null
}