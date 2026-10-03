package com.aobhub.speakerpro

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.core.content.ContextCompat
import kotlinx.coroutines.delay

class MainActivity : ComponentActivity() {
    private val notificationPermission =
        registerForActivityResult(ActivityResultContracts.RequestPermission()) { }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        if (Build.VERSION.SDK_INT >= 33 &&
            ContextCompat.checkSelfPermission(this, Manifest.permission.POST_NOTIFICATIONS)
            != PackageManager.PERMISSION_GRANTED) {
            notificationPermission.launch(Manifest.permission.POST_NOTIFICATIONS)
        }

        setContent {
            var running by remember { mutableStateOf(AudioReceiverService.isRunning(this)) }
            var status by remember { mutableStateOf(AudioReceiverService.status(this)) }

            LaunchedEffect(Unit) {
                while (true) {
                    running = AudioReceiverService.isRunning(this@MainActivity)
                    status = AudioReceiverService.status(this@MainActivity)
                    delay(500)
                }
            }

            MaterialTheme {
                Surface(Modifier.fillMaxSize()) {
                    Column(
                        Modifier.fillMaxSize().padding(24.dp),
                        verticalArrangement = Arrangement.spacedBy(16.dp)
                    ) {
                        Spacer(Modifier.height(24.dp))
                        Text("AOB Speaker Pro", style = MaterialTheme.typography.headlineMedium)
                        Text("Turn Your Android Into Your PC Speaker",
                            style = MaterialTheme.typography.bodyLarge)
                        Card(Modifier.fillMaxWidth()) {
                            Column(Modifier.padding(16.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
                                Text(if (running) "● RECEIVER RUNNING" else "○ RECEIVER STOPPED",
                                    style = MaterialTheme.typography.titleMedium)
                                Text(status)
                                Text("UDP audio port: 4677")
                            }
                        }
                        Button(
                            modifier = Modifier.fillMaxWidth(),
                            onClick = {
                                ContextCompat.startForegroundService(
                                    this@MainActivity,
                                    Intent(this@MainActivity, AudioReceiverService::class.java)
                                )
                            },
                            enabled = !running
                        ) { Text("Start Receiver") }
                        OutlinedButton(
                            modifier = Modifier.fillMaxWidth(),
                            onClick = { stopService(Intent(this@MainActivity, AudioReceiverService::class.java)) },
                            enabled = running
                        ) { Text("Stop Receiver") }
                        Text("Windows PC: run aob-speaker-pro <Android-IP> 4677")
                        Text("Keep both devices on the same Wi-Fi network. The receiver stays active with a foreground notification.",
                            style = MaterialTheme.typography.bodySmall)
                    }
                }
            }
        }
    }
}
