package com.aobhub.speakerpro
import android.app.*
import android.content.Intent
import android.os.IBinder
import androidx.core.app.NotificationCompat
class AudioReceiverService:Service(){private external fun nativeStart(port:Int):Boolean;private external fun nativeStop();override fun onCreate(){super.onCreate();System.loadLibrary("aob_audio");val nm=getSystemService(NotificationManager::class.java);nm.createNotificationChannel(NotificationChannel("aob","AOB Audio",NotificationManager.IMPORTANCE_LOW));startForeground(7,NotificationCompat.Builder(this,"aob").setContentTitle("AOB Speaker Pro").setContentText("Receiving PC audio").setSmallIcon(android.R.drawable.ic_media_play).build());nativeStart(4677)}override fun onDestroy(){nativeStop();super.onDestroy()}override fun onBind(i:Intent?):IBinder?=null}