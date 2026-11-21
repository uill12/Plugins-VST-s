#!/usr/bin/env python3
"""Create verified CP58 portable continuation source archive from GitHub Actions.
Inputs: checked-out repository, proven CI artifacts, vendored sources and WebView2.
This is a packaging operation: DSP, presets, UI and plugin code are NOT modified.
"""
from pathlib import Path
import argparse,hashlib,shutil,subprocess,sys,json,tempfile,zipfile
p=argparse.ArgumentParser()
p.add_argument('--inputs',default='_handoff_inputs')
p.add_argument('--deps',default='_handoff_deps')
p.add_argument('--output',default='_handoff_export')
a=p.parse_args()
work=Path.cwd(); inputs=work/a.inputs; deps=work/a.deps; output=work/a.output
root=output/'Albedo_CP58_Complete_Handoff'
root.mkdir(parents=True,exist_ok=True)
def sha(path):
    h=hashlib.sha256()
    with open(path,'rb') as f:
        for block in iter(lambda:f.read(1024*1024),b''):h.update(block)
    return h.hexdigest()
def execv(cmd,**kw):
    print('$', ' '.join(map(str,cmd)),flush=True)
    return subprocess.check_output([str(c) for c in cmd],**kw).decode().strip()
def copyall(src,dst):
    if dst.exists():shutil.rmtree(dst)
    shutil.copytree(src,dst,ignore=shutil.ignore_patterns('.git','__pycache__','*.pyc'))
# SourceRepo contains every tracked source and script, not just a GitHub URL.
repo=root/'SourceRepo'
repo.mkdir(exist_ok=True)
paths=execv(['git','ls-files','-z']).split('\x00')
for relative in filter(None,paths):
    x=work/relative
    if not x.is_file():continue
    dest=repo/relative;dest.parent.mkdir(parents=True,exist_ok=True)
    shutil.copy2(x,dest)
release_sha='3481912073e23cef753521e3a64cceafb19257ac'
package_sha=execv(['git','rev-parse','HEAD'])
# Real final, patched CP58 source recovered from successful native CI run.
dsp=root/'ReconstructedDSP'; copyall(inputs/'DSP',dsp)
for relative in ('src/vitriol_dsp.cpp','include/vitriol_dsp.hpp','include/vitriol_params.hpp','include/AlbedoCellArp.inc'):
    assert (dsp/relative).is_file(),relative
# Verify source archive hash and regenerate original 32-material bank.
archive=repo/'build-sources/Albedo_CP13_CI_Source_NoBank.zip'
assert sha(archive)=='5d5bceedcb6b0a4c78880dd6339d12250c51c9a543e80826c78189028d71d325'
bank=dsp/'resources/Albedo_Materia32_PCM24_96k.vtrb'
bank.parent.mkdir(exist_ok=True)
with tempfile.TemporaryDirectory(prefix='albedo_cp58_original_') as temp:
    with zipfile.ZipFile(archive) as z:z.extractall(temp)
    script=Path(temp)/'Albedo-Quintessential-Synth/resources/generate_soundbank.py'
    assert script.is_file(),str(script)
    execv([sys.executable,script,'--output',bank])
assert sha(bank)=='ee5e625a17833cd8d8640151a98ae8ae52038e0f830db1890a583eb98da0a838','CP58 PCM24 bank SHA mismatch'
# Preserve the exact 192 *selected* sound programs and generation report.
sel=root/'SelectedPresets';sel.mkdir(exist_ok=True)
names=['material_factory_bank.json','recursive_forms_bank.json',
       'morph_expansion_bank.json','morph_selection_report.json',
       'cp51_existing_preset_regeneration.json']
for name in names:
    found=list((inputs/'SelectedPresets').rglob(name))
    assert len(found)==1, (name,found)
    shutil.copy2(found[0],sel/name)
# Include source-only third-party dependencies; do not remove license files.
vendor=root/'Dependencies';vendor.mkdir(exist_ok=True)
for name in ['JUCE','clap-juce-extensions','cp33-webview2-sdk']:
    assert (deps/name).is_dir(),name
    copyall(deps/name,vendor/name)
assert (vendor/'JUCE/CMakeLists.txt').is_file()
assert (vendor/'clap-juce-extensions/clap-libs/clap/CMakeLists.txt').is_file()
web=vendor/'cp33-webview2-sdk/Microsoft.Web.WebView2.1.0.4258.31/build/native'
assert (web/'include/WebView2.h').is_file()
assert (web/'x64/WebView2LoaderStatic.lib').is_file()
# Compiled CLAP/VST3 and the previous working rollback, independently.
binroot=root/'Binaries'
for release,source in [('CP58','Binaries58'),('CP57_rollback','Binaries57')]:
    copyall(inputs/source,binroot/release)
assert len(list((binroot/'CP58').rglob('*.clap')))==1
assert len(list((binroot/'CP58').rglob('*.vst3')))==1
# Native QA and metadata reports (not a claim of hardware ASIO certification).
qa=root/'QA'
for report in ['ASIO_20_cursors','Presets192','PresetPairs','PCMQuality']:
    copyall(inputs/report,qa/report)
for name in ['START_HERE.md','Build_Windows_CP58.ps1','verify_bundle.py','requirements-build.txt']:
    shutil.copy2(repo/'handoff/cp58'/name,root/name)
shutil.copy2(repo/'CHECKPOINTS/ALBEDO_CP58_ASIO_REALTIME.md',root/'CP58_CHECKPOINT.md')
jucesha=execv(['git','-C',str(deps/'JUCE'),'rev-parse','HEAD'])
clapsha=execv(['git','-C',str(deps/'clap-juce-extensions'),'rev-parse','HEAD'])
clapsubsha=execv(['git','-C',str(deps/'clap-juce-extensions/clap-libs/clap'),'rev-parse','HEAD'])
lock=dict(project='Albedo Quintessential Synth',checkpoint='CP58',
  version='1.0.58',cp58_release_commit=release_sha,
  handoff_package_commit=package_sha,
  ci_run=44879814659,
  gui_run=44879814690,
  juce=dict(ref='8.0.17-security',sha=jucesha,source='https://github.com/juce-framework/JUCE'),
  clap_juce_extensions=dict(sha=clapsha,source='https://github.com/free-audio/clap-juce-extensions'),
  clap_sdk_submodule=dict(sha=clapsubsha),
  webview2_sdk=dict(version='1.0.4258.31',source='https://www.nuget.org/packages/Microsoft.Web.WebView2/1.0.4258.31'),
  pcm24_bank=dict(filename=bank.name,sha256=sha(bank)),
  build_host='Windows x64; Visual Studio 2022/MSVC v143; Windows SDK; CMake >=3.24; Python 3.12',
  manual_bitwig_asio_qa='pending',
  dependencies_licenses='Original license documents retained in vendor trees. JUCE licensing obligations apply.')
(root/'DEPENDENCIES_LOCK.json').write_text(json.dumps(lock,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
lines=[]
for file in sorted(root.rglob('*')):
    if not file.is_file() or file.name=='SHA256SUMS.txt':continue
    lines.append(f'{sha(file)}  {file.relative_to(root).as_posix()}')
(root/'SHA256SUMS.txt').write_text('\n'.join(lines)+'\n',encoding='utf-8')
execv([sys.executable,root/'verify_bundle.py'])
zipout=output/'Albedo_CP58_Complete_Source_Dependencies_Handoff.zip'
with zipfile.ZipFile(zipout,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=6,allowZip64=True) as z:
    for f in sorted(root.rglob('*')):
        if f.is_file():z.write(f,f.relative_to(root).as_posix())
with zipfile.ZipFile(zipout) as z:
    assert z.testzip() is None
    assert 'ReconstructedDSP/src/vitriol_dsp.cpp' in z.namelist()
    assert 'SourceRepo/Albedo-JUCE-Native/CMakeLists.txt' in z.namelist()
    assert 'Binaries/CP58/Albedo_CP58_ASIO_Optimized_WebView2.clap' in z.namelist()
print(json.dumps({'zip':str(zipout),'sha256':sha(zipout),
  'size':zipout.stat().st_size,'count':len(lines),
  'release_sha':release_sha,'packaging_sha':package_sha,
  'juce_sha':jucesha,'clap_extension_sha':clapsha},indent=2))
