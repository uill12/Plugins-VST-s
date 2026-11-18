# Windows x64 CP12R2 CLAP installed-binary integrity and loader diagnostic.
# Safe: only reads the file and calls LoadLibrary/GetProcAddress/FreeLibrary.
param(
 [string]$PluginPath = 'C:\Program Files\Common Files\CLAP\Rubedo_Atlas_Drumkit.clap'
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$expected = 'a1a27a1edaff750c2a83f233255455f29a95827095914ff388d700e9067ac1f0'
if(!(Test-Path -LiteralPath $PluginPath -PathType Leaf)) {throw "CLAP not found: $PluginPath"}
$full = (Resolve-Path -LiteralPath $PluginPath).Path
$actual = (Get-FileHash -LiteralPath $full -Algorithm SHA256).Hash.ToLowerInvariant()
if($actual -ne $expected) {
 throw "NOT CP12R2 Windows-smoke-tested build. SHA256=$actual expected=$expected"
}
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public static class RubedoCp12r2Loader {
  [DllImport("kernel32.dll", EntryPoint="LoadLibraryW", CharSet=CharSet.Unicode, SetLastError=true)]
  public static extern IntPtr LoadLibrary(string path);
  [DllImport("kernel32.dll", EntryPoint="GetProcAddress", CharSet=CharSet.Ansi, SetLastError=true)]
  public static extern IntPtr GetProcAddress(IntPtr module, string symbol);
  [DllImport("kernel32.dll", EntryPoint="FreeLibrary", SetLastError=true)]
  public static extern bool FreeLibrary(IntPtr module);
}
'@
$module = [RubedoCp12r2Loader]::LoadLibrary($full)
if($module -eq [IntPtr]::Zero) {
 $win32 = [Runtime.InteropServices.Marshal]::GetLastWin32Error()
 throw "FAIL: Windows LoadLibrary error $win32 for $full"
}
try {
 $entry = [RubedoCp12r2Loader]::GetProcAddress($module, 'clap_entry')
 if($entry -eq [IntPtr]::Zero) {
  $win32 = [Runtime.InteropServices.Marshal]::GetLastWin32Error()
  throw "FAIL: clap_entry GetProcAddress error $win32"
 }
 Write-Host 'PASS: Installed CLAP SHA-256 verified; LoadLibrary and clap_entry resolved.'
 Write-Host "PATH: $full"
} finally {
 if(![RubedoCp12r2Loader]::FreeLibrary($module)) {
  Write-Warning 'FreeLibrary returned false; this is independent of the plugin scan.'
 }
}
