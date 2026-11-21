# Requires Windows x64, Visual Studio 2022 C++ toolchain, CMake, Python 3.12.
# Builds original CP58 Windows JUCE CLAP/VST3 using the INCLUDED final DSP.
param([string]$BuildDir="Build-Windows-CP58",[switch]$NoTests)
$ErrorActionPreference='Stop'
$base=Split-Path -Parent $MyInvocation.MyCommand.Path
$repo=Join-Path $base 'SourceRepo'
$juce=Join-Path $repo 'Albedo-JUCE-Native'
$dsp=Join-Path $base 'ReconstructedDSP'
$deps=Join-Path $base 'Dependencies'
$metadata=Join-Path $juce 'Metadata'
$source=Join-Path $juce 'Source'
$target=Join-Path $base $BuildDir
$webview=Join-Path $deps 'cp33-webview2-sdk'
$generator=Join-Path $juce 'tools/generate_recursive_forms.py'
if(!(Test-Path (Join-Path $dsp 'src/vitriol_dsp.cpp'))) {throw 'Missing CP58 reconstructed DSP source'}
if(!(Test-Path (Join-Path $dsp 'resources/Albedo_Materia32_PCM24_96k.vtrb'))) {throw 'Missing original 32-material PCM bank'}
foreach($vendor in @('JUCE','clap-juce-extensions')){
  $dst=Join-Path $juce ('ext/'+$vendor)
  if(!(Test-Path $dst)) {
    New-Item -ItemType Directory -Force -Path (Split-Path $dst -Parent) | Out-Null
    Copy-Item (Join-Path $deps $vendor) -Destination $dst -Recurse -Force
  }
}
foreach($entry in @(@('material','material_factory_bank.json','AlbedoMaterialBank.hpp'),
                     @('creative','recursive_forms_bank.json','AlbedoCreativeBank.hpp'),
                     @('morph','morph_expansion_bank.json','AlbedoMorphBank.hpp'))){
  $bank=$entry[0];$name=$entry[1];$header=$entry[2]
  $file=Join-Path $base ('SelectedPresets/'+$name)
  if(!(Test-Path $file)){throw "Missing selected preset $file"}
  Copy-Item $file -Destination (Join-Path $metadata $name) -Force
  & python $generator --bank $bank --metadata (Join-Path $metadata $name) --output (Join-Path $source $header)
  if($LASTEXITCODE -ne 0){throw "Preset header generation failed for $bank"}
}
if(!(Test-Path (Join-Path $webview 'Microsoft.Web.WebView2.1.0.4258.31/build/native/include/WebView2.h'))){
 throw 'Pinned WebView2 1.0.4258.31 SDK unavailable'
}
if(!(Test-Path (Join-Path $webview 'Microsoft.Web.WebView2.1.0.4258.31/build/native/x64/WebView2LoaderStatic.lib'))){
 throw 'Pinned WebView2 static loader unavailable'
}
$configure=@('-S',$juce,'-B',$target,'-G','Visual Studio 17 2022','-A','x64',
 '-DALBEDO_BUILD_CLAP=ON','-DBUILD_TESTING=ON',
 "-DALBEDO_DSP_ROOT=$dsp","-DJUCE_WEBVIEW2_PACKAGE_LOCATION=$webview")
& cmake @configure
if($LASTEXITCODE -ne 0){throw 'CMake JUCE configuration failed'}
& cmake --build $target --config Release --parallel 4 --target AlbedoNative_VST3 AlbedoNative_CLAP AlbedoPresetTaxonomyTest AlbedoCP36HifiDspTest AlbedoCP38CreativeAudioTest AlbedoRearFieldTest AlbedoTargetRenderer AlbedoArpTest AlbedoControlResponseTest AlbedoShapeAudition AlbedoViewportHost AlbedoCP53OutputGuardTest AlbedoCP53VoiceHifiTest AlbedoCP58AsioBlocks
if($LASTEXITCODE -ne 0){throw 'CLAP/VST3 compile failed'}
if(!$NoTests) {
 & ctest --test-dir $target -C Release --output-on-failure
 if($LASTEXITCODE -ne 0){throw 'Native test suite failed'}
}
Write-Host 'CP58 rebuild finished. See build directory for CLAP and VST3. Bitwig/ASIO manual QA is still required.'
