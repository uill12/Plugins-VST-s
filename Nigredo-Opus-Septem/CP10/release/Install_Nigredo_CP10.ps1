# NIGREDO OPUS SEPTEM CP10 — Native Win32 GUI, no WebView2 required.
[CmdletBinding()]
param([ValidateSet('User','AllUsers')][string]$Scope='User')
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $MyInvocation.MyCommand.Path
$clap=Join-Path $root 'Windows\CLAP'
$vst=Join-Path $root 'Windows\VST3'
if(!(Test-Path -LiteralPath $clap) -or !(Test-Path -LiteralPath $vst)){ throw 'Extract the full CP10 archive before running this script.' }
if($Scope -eq 'User') {
 $destClap=Join-Path $env:LOCALAPPDATA 'Programs\Common\CLAP'
 $destVst=Join-Path $env:LOCALAPPDATA 'Programs\Common\VST3'
} else {
 $destClap=Join-Path $env:CommonProgramFiles 'CLAP'
 $destVst=Join-Path $env:CommonProgramFiles 'VST3'
}
New-Item -ItemType Directory -Path $destClap,$destVst -Force | Out-Null
foreach($f in Get-ChildItem -LiteralPath $clap -Filter '*.clap' -File) {
 $path=Join-Path $destClap $f.Name
 if(Test-Path -LiteralPath $path) {
  $backup=$path+'.backup-before-cp10-'+(Get-Date -Format 'yyyyMMdd-HHmmss')
  Move-Item -LiteralPath $path -Destination $backup
  Write-Host ('Old CLAP backed up: '+$backup)
 }
 Copy-Item -LiteralPath $f.FullName -Destination $path -Force
 Write-Host ('CLAP: '+$path)
}
foreach($d in Get-ChildItem -LiteralPath $vst -Filter '*.vst3' -Directory) {
 $target=Join-Path $destVst $d.Name
 if(Test-Path -LiteralPath $target) {
  # The CP05 folder may contain WebView2Loader.dll. Keep it as an inert, non-plugin backup.
  $backup=$target+'.backup-before-cp10-'+(Get-Date -Format 'yyyyMMdd-HHmmss')
  Move-Item -LiteralPath $target -Destination $backup
  Write-Host ('Old plugin backed up: '+$backup)
 }
 Copy-Item -LiteralPath $d.FullName -Destination $target -Recurse -Force
 Write-Host ('VST3: '+$target)
}
Write-Host ''
Write-Host 'Nigredo CP10 installed. No WebView2 runtime or loader is required.'
Write-Host 'The native GUI uses standard Windows GDI/GDI+ (Windows built-in).'
Write-Host 'Restart Bitwig Studio, then rescan plugins.'
Write-Host 'This development build has NOT been validated in a running Windows Bitwig host.'
