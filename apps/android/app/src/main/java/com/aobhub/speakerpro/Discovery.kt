package com.aobhub.speakerpro
import android.content.Context
import android.net.nsd.NsdManager
import android.net.nsd.NsdServiceInfo
class Discovery(context:Context){
 private val nsd=context.getSystemService(NsdManager::class.java)
 fun discover(onFound:(String,Int)->Unit){nsd.discoverServices("_aob._udp",NsdManager.PROTOCOL_DNS_SD,object:NsdManager.DiscoveryListener{
 override fun onServiceFound(s:NsdServiceInfo){nsd.resolveService(s,object:NsdManager.ResolveListener{
 override fun onServiceResolved(r:NsdServiceInfo){r.host?.hostAddress?.let{onFound(it,r.port)}}
 override fun onResolveFailed(s:NsdServiceInfo,e:Int){}
 })}
 override fun onServiceLost(s:NsdServiceInfo){}
 override fun onDiscoveryStarted(s:String){}
 override fun onDiscoveryStopped(s:String){}
 override fun onStartDiscoveryFailed(s:String,e:Int){nsd.stopServiceDiscovery(this)}
 override fun onStopDiscoveryFailed(s:String,e:Int){}
})}
}