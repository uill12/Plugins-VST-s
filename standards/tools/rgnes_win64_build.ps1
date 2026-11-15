# RGNES Windows x64 build + runtime verification launcher.
# Requires a REAL project build script/CMake config; this tool does not synthesize absent source or SDKs.
[CmdletBinding()]
param(
  [Parameter(Mandatory=$true)][string]$ProjectRoot,
  [Parameter(Mandatory=$true)][string]$ArtifactPath,
  [ValidateSet('Auto','PowerShell','CMake','MSBuild')][string]$BuildMode='Auto',
  [ValidateSet('Debug','Release','RelWithDebInfo')][string]$Configuration='Release',
  [string]$BuildScript='Build_Windows_x64.ps1',
  [string]$Formats='clap,vst3',
  [string]$StageOut='',
  [string]$ReportPath=''
)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
function Fail([string]$Message) { throw "RGNES BUILD BLOCKED: $Message" }
function Command-Exists([string]$Tool) { return [bool](Get-Command $Tool -ErrorAction SilentlyContinue) }
if($env:OS -ne 'Windows_NT') { Fail 'Windows x64 runtime required. Use windows-2022 GitHub Actions or a Windows host with MSVC/Windows SDK.' }
if(!(Test-Path -LiteralPath $ProjectRoot -PathType Container)) { Fail "Project directory missing: $ProjectRoot" }
$Root=(Resolve-Path -LiteralPath $ProjectRoot).Path
$Log=[System.Collections.Generic.List[string]]::new()
$Log.Add("RGNES Windows build $(Get-Date -Format 'yyyy-MM-ddTHH:mm:ssK')")
$Log.Add("Source root: $Root")
$Log.Add("Requested formats: $Formats")
$Log.Add("PowerShell: $($PSVersionTable.PSVersion)")
$Log.Add("OS: $([Environment]::OSVersion.VersionString)")
$Log.Add("CMake available: $(Command-Exists 'cmake')")
$Log.Add("MSBuild available: $(Command-Exists 'msbuild')")
$Log.Add("Python available: $(Command-Exists 'python')")
if(!(Command-Exists 'python')) { Fail 'Python 3 unavailable. Install Python 3.9+ or use actions/setup-python in Windows CI.' }
$Verifier=Join-Path $PSScriptRoot 'rgnes_verify_win64.py'
if(!(Test-Path -LiteralPath $Verifier -PathType Leaf)) { Fail "Missing companion verifier: $Verifier" }
if(!$StageOut) {$StageOut=Join-Path ([IO.Path]::GetTempPath()) ("rgnes-runtime-" + [guid]::NewGuid().ToString('N'))}
if(!$ReportPath) {$ReportPath=Join-Path $Root 'RGNES_WINDOWS_BUILD_REPORT.json'}
$BuildScriptPath=Join-Path $Root $BuildScript
$CMakeFile=Join-Path $Root 'CMakeLists.txt'
$Solutions=@(Get-ChildItem -LiteralPath $Root -File -Filter '*.sln' -ErrorAction SilentlyContinue)
if($BuildMode -eq 'Auto') {
  if(Test-Path -LiteralPath $BuildScriptPath -PathType Leaf) {$BuildMode='PowerShell'}
  elseif(Test-Path -LiteralPath $CMakeFile -PathType Leaf) {$BuildMode='CMake'}
  elseif($Solutions.Count -eq 1) {$BuildMode='MSBuild'}
  else { Fail 'No Build_Windows_x64.ps1, CMakeLists.txt or unambiguous .sln. Add one or pass an explicit supported build mode.' }
}
$Log.Add("Selected build mode: $BuildMode")
$BuildStartedUtc=[datetime]::UtcNow
Push-Location $Root
try {
  if($BuildMode -eq 'PowerShell') {
    if(!(Test-Path -LiteralPath $BuildScriptPath -PathType Leaf)){ Fail "PowerShell build script missing: $BuildScriptPath" }
    $ci=Get-Command -Name $BuildScriptPath -ErrorAction Stop
    $Log.Add("Running project script: $BuildScriptPath")
    if($ci.Parameters.ContainsKey('Configuration')) { & $BuildScriptPath -Configuration $Configuration }
    else { & $BuildScriptPath }
    if(!$?) { Fail 'Project PowerShell build script failed. Inspect build logs and check all required embedded assets.' }
  } elseif ($BuildMode -eq 'CMake') {
    if(!(Test-Path -LiteralPath $CMakeFile -PathType Leaf)){Fail "CMakeLists.txt missing: $CMakeFile"}
    if(!(Command-Exists 'cmake')){Fail 'CMake missing. Install CMake or use windows-2022 GitHub Actions.'}
    $BuildDir=Join-Path $Root 'build_rgnes_windows_x64'
    $Log.Add("Configuring Visual Studio 2022 x64 in $BuildDir")
    & cmake -S $Root -B $BuildDir -G 'Visual Studio 17 2022' -A 'x64'
    if($LASTEXITCODE -ne 0){Fail 'CMake configure failed. Check Visual Studio C++ workload, Windows SDK, VST3/CLAP SDK paths and all resources.'}
    & cmake --build $BuildDir --config $Configuration --parallel
    if($LASTEXITCODE -ne 0){Fail 'CMake build failed. Inspect compiler/linker output; do not promote a partial binary.'}
  } elseif($BuildMode -eq 'MSBuild') {
    if(!(Command-Exists 'msbuild')){Fail 'MSBuild missing. Run from VS Developer PowerShell or use windows-2022 GitHub Actions.'}
    if($Solutions.Count -ne 1){Fail 'MSBuild requires exactly one solution in project root; use explicit CMake or build script instead.'}
    & msbuild $Solutions[0].FullName /m /p:Configuration=$Configuration /p:Platform=x64 /t:Build
    if($LASTEXITCODE -ne 0){Fail 'MSBuild failed. Inspect errors for missing Windows SDK, resources, ABI or linker libraries.'}
  }
} finally { Pop-Location }
# ArtifactPath must point at binaries produced by this build (not a general folder of historical releases).
if([IO.Path]::IsPathRooted($ArtifactPath)){$ArtifactFull=$ArtifactPath}
else{$ArtifactFull=Join-Path $Root $ArtifactPath}
if(!(Test-Path -LiteralPath $ArtifactFull)) { Fail "Build finished but release artifacts missing: $ArtifactFull. Fix project output path rather than staging old files." }
# Do not accidentally validate yesterday's stale distribution after an unsuccessful/no-op packaging step.
if(Test-Path -LiteralPath $ArtifactFull -PathType Leaf) {
  $Candidates=@(Get-Item -LiteralPath $ArtifactFull)
} else {
  $Candidates=@(Get-ChildItem -LiteralPath $ArtifactFull -File -Recurse | Where-Object {
    $_.Name -match '(?i)\.(clap|vst3|zip|7z\.001)$'
  })
}
if($Candidates.Count -eq 0){Fail 'Artifact output has no installable plugin files or distribution archive.'}
$Fresh=@($Candidates | Where-Object { $_.LastWriteTimeUtc -ge $BuildStartedUtc.AddSeconds(-5) })
if($Fresh.Count -eq 0){Fail 'No binary/package was refreshed by this build; refusing to release stale artifacts. Configure the project to copy/package its current outputs into ArtifactPath.'}
$Log.Add("Fresh artifact candidates: $($Fresh.Count)")
$AuditRoot=$ArtifactFull
if(Test-Path -LiteralPath $ArtifactFull -PathType Leaf) {
  $Temporary=Join-Path ([IO.Path]::GetTempPath()) ("rgnes-unpack-" + [guid]::NewGuid().ToString('N'))
  New-Item -Path $Temporary -ItemType Directory | Out-Null
  if($ArtifactFull.ToLowerInvariant().EndsWith('.zip')) {
    Expand-Archive -LiteralPath $ArtifactFull -DestinationPath $Temporary -Force
  } elseif($ArtifactFull -match '\.7z\.001$') {
    $z=Get-Command 7z.exe -ErrorAction SilentlyContinue
    if(!$z){Fail '7-Zip binary (7z.exe) required to validate multi-volume archive. Install it or build an unpacked distribution.'}
    & $z.Source x '-y' ("-o$Temporary") $ArtifactFull
    if($LASTEXITCODE -ne 0){Fail '7-Zip extraction failed. Verify all .7z.001–.NNN volumes are present.'}
  } else {Fail 'ArtifactPath must be a directory, .zip, or .7z.001 multi-volume archive.'}
  $AuditRoot=$Temporary
}
$reportDir=Split-Path -Parent $ReportPath
if($reportDir -and !(Test-Path -LiteralPath $reportDir)){New-Item -ItemType Directory -Path $reportDir -Force|Out-Null}
$Log.Add("Audit root: $AuditRoot")
& python $Verifier --root $AuditRoot --formats $Formats --stage-out $StageOut --json-out $ReportPath
if($LASTEXITCODE -ne 0){Fail "Static Windows binary verification failed. See $ReportPath; no distribution promoted."}
$Log.Add("Static PE64/exports/bundle gate PASS; Bitwig/DAW host QA NOT performed")
$Log.Add("Verified runtime staged in: $StageOut")
$Log | ForEach-Object { Write-Host $_ }
Write-Host "RGNES_RELEASE_VERIFIED=$StageOut"
Write-Host "RGNES_REPORT=$ReportPath"
