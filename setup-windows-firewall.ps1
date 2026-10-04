# AOB Speaker Pro Windows LAN setup
# Run PowerShell as Administrator from this folder.

$ErrorActionPreference = "Stop"
$rules = @(
  @{Name="AOB Speaker Pro Discovery"; Port=4678},
  @{Name="AOB Speaker Pro Audio"; Port=4677}
)
foreach ($r in $rules) {
  Get-NetFirewallRule -DisplayName $r.Name -ErrorAction SilentlyContinue | Remove-NetFirewallRule
  New-NetFirewallRule -DisplayName $r.Name -Direction Inbound -Protocol UDP -LocalPort $r.Port -Action Allow -Profile Private
}
Write-Host "AOB Speaker Pro LAN firewall rules installed."
Write-Host "Make sure the PC and Android phone are connected to the same private Wi-Fi network."
Write-Host "Start aob-speaker-pro.exe; it will wait for the Android receiver automatically."
