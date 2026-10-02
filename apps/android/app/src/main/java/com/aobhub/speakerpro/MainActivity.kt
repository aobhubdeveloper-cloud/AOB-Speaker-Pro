package com.aobhub.speakerpro
import android.content.Intent
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.material3.*
import androidx.compose.foundation.layout.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
class MainActivity:ComponentActivity(){override fun onCreate(b:Bundle?){super.onCreate(b);setContent{MaterialTheme{Column(Modifier.fillMaxSize().padding(24.dp),verticalArrangement=Arrangement.Center){Text("AOB Speaker Pro",style=MaterialTheme.typography.headlineMedium);Text("Turn Your Android Into Your PC Speaker");Spacer(Modifier.height(16.dp));Button(onClick={startService(Intent(this@MainActivity,AudioReceiverService::class.java))}){Text("Start Receiver")}}}}}}