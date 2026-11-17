# AssembleRubedoCP12R1VST3.ps1
# Creates a self-contained VST3 *locally* from a verified CP12R1 CLAP
# and a newly compiled Windows x64 VST3 PE core. Does NOT install anything.
# This is a source/build recovery utility, not a claim of Bitwig validation.
[CmdletBinding()]
param(
  [Parameter(Mandatory=$true)][string]$ClapPath,
  [Parameter(Mandatory=$true)][string]$VST3CorePath,
  [string]$OutputRoot = (Join-Path (Get-Location).Path 'Rubedo_CP12R1_VST3_Assembly')
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$expectedClapSha = '65668f7a244c0e311741d6545b71d603e188ea27eed2e9b57c963da1edf28e1a'
$clap = (Resolve-Path -LiteralPath $ClapPath).Path
$core = (Resolve-Path -LiteralPath $VST3CorePath).Path
$sha = (Get-FileHash -LiteralPath $clap -Algorithm SHA256).Hash.ToLowerInvariant()
if($sha -ne $expectedClapSha){throw "Wrong CLAP source version or altered file: $sha. Expected CP12R1: $expectedClapSha"}
$coreBytes = [IO.File]::ReadAllBytes($core)
if($coreBytes.Length -lt 1024 -or $coreBytes[0] -ne 77 -or $coreBytes[1] -ne 90){throw 'VST3 PE core is not a valid candidate'}
# The core MUST be the compile-only output of the CP12R1 VST3 workflow.
$src = [IO.File]::OpenRead($clap)
try {
  if($src.Length -lt 32){throw 'CLAP source is too small'}
  [void]$src.Seek(-32,[IO.SeekOrigin]::End)
  $footer = [byte[]]::new(32)
  if($src.Read($footer,0,32) -ne 32){throw 'Incomplete resource footer'}
  $magic = [Text.Encoding]::ASCII.GetString($footer,0,8)
  if($magic -ne 'RBOVR103'){throw "Unexpected footer magic: $magic"}
  $oldBank = [BitConverter]::ToUInt64($footer,8)
  $pictureBytes = [BitConverter]::ToUInt64($footer,24)
  $expectedPicture = [UInt64](1512 * 1009 * 3)
  if($pictureBytes -ne $expectedPicture -or $oldBank -lt $pictureBytes -or $oldBank -ge [UInt64]($src.Length-32)){
    throw "Invalid embedded resource offsets"
  }
  $overlayStart = [Int64]($oldBank-$pictureBytes)
  [void]$src.Seek([Int64]$oldBank,[IO.SeekOrigin]::Begin)
  $header = [byte[]]::new(8)
  if($src.Read($header,0,8) -ne 8 -or [Text.Encoding]::ASCII.GetString($header,0,6) -ne 'RBDK24'){
    throw 'Embedded Rubedo 2,304-hit bank header not found'
  }
  $folder = Join-Path $OutputRoot 'Rubedo Atlas Drumkit.vst3\Contents\x86_64-win'
  $outFile = Join-Path $folder 'Rubedo Atlas Drumkit.vst3'
  if(Test-Path -LiteralPath $outFile){throw "Output already exists; refusing overwrite: $outFile"}
  New-Item -Path $folder -ItemType Directory -Force | Out-Null
  $dst = [IO.File]::Open($outFile,[IO.FileMode]::CreateNew,[IO.FileAccess]::ReadWrite)
  try {
    $dst.Write($coreBytes,0,$coreBytes.Length)
    [void]$src.Seek($overlayStart,[IO.SeekOrigin]::Begin)
    $src.CopyTo($dst,1048576)
    $newBank = [UInt64]$coreBytes.Length + $pictureBytes
    $newOffset = [BitConverter]::GetBytes($newBank)
    [void]$dst.Seek(-24,[IO.SeekOrigin]::End)
    $dst.Write($newOffset,0,8)
    $dst.Flush($true)
    $newLength = $dst.Length
  } finally {$dst.Dispose()}
  if($newLength -lt 373000000){throw 'Assembled plugin unexpectedly small'}
  $outHash = (Get-FileHash -LiteralPath $outFile -Algorithm SHA256).Hash
  $report = @(
    'Rubedo CP12R1 VST3 / locally assembled from version-matched CP12R1 CLAP',
    "CLAP_SHA256=$sha",
    "VST3_SHA256=$outHash",
    "VST3_BYTES=$newLength",
    'STATUS=ASSEMBLED_NOT_TESTED_IN_BITWIG',
    'NOTICE=VST3 MIDI Learn and pad feedback are source-implemented via host outputParameterChanges and mock-host-tested; actual Bitwig delivery and state persistence remain unverified.'
  )
  $report | Set-Content -LiteralPath (Join-Path $OutputRoot 'ASSEMBLY_MANIFEST.txt') -Encoding UTF8
  Write-Host "Assembled: $outFile"
  Write-Host "SHA-256 : $outHash"
  Write-Host 'This is not host-certified. Test in a sandbox Bitwig session before normal use.'
} finally {$src.Dispose()}
