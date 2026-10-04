package com.aobhub.speakerpro

import android.content.Intent
import android.os.Build
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.material3.*
import androidx.compose.foundation.layout.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.core.content.ContextCompat

class MainActivity:ComponentActivity(){
 private val notificationPermission=registerForActivityResult(ActivityResultContracts.RequestPermission()){}
 override fun onCreate(b:Bundle?){
  super.onCreate(b)
  setContent{
   MaterialTheme{
    Column(Modifier.fillMaxSize().padding(24.dp),verticalArrangement=Arrangement.Center){
     Text("AOB Speaker Pro",style=MaterialTheme.typography.headlineMedium)
     Text("Windows PC → Android speaker over Wi-Fi")
     Spacer(Modifier.height(16.dp))
     Button(onClick={
      if(Build.VERSION.SDK_INT>=33) notificationPermission.launch("android.permission.POST_NOTIFICATIONS")
      ContextCompat.startForegroundService(this@MainActivity,Intent(this@MainActivity,AudioReceiverService::class.java))
     }){Text("Start Receiver")}
    }
   }
  }
 }
}