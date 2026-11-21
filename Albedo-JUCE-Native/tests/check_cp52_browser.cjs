'use strict';
// CP52 contract: Nereida-style hierarchy on Albedo's original indexed host loader.
const fs=require('node:fs'),vm=require('node:vm'),path=require('node:path'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'..');
const read=p=>fs.readFileSync(path.join(root,p),'utf8');
const browser=read('Assets/cp52_nereida_browser.js');
const fit=read('Assets/cp35_editor_geometry.js');
const cpp=read('Source/AlbedoWebViewEditor.cpp');
const cmake=read('CMakeLists.txt');
const processor=read('Source/AlbedoProcessor.h');
const html=read('../ui/Albedo_CP30_CP22_Landscape_3x2.html');
new vm.Script(browser,{filename:'cp52_nereida_browser.js'});
for(const token of ['albedo-cp52-workspace','albedo-cp52-navigation','atlas.newSounds||[]','overflow-y:auto!important','cp52-category','pb-item-name','sort?.dispatchEvent','aria-pressed','aria-label','albedoProgram']){
  assert(browser.includes(token),'CP52 browser missing '+token);
}
assert(browser.includes('NOT the offline Morph Tree'), 'Morph Tree must not be exposed as UI navigation');
assert(cmake.includes('Assets/cp52_nereida_browser.js'),'JUCE binary data registration missing');
assert(cpp.includes('cp52_nereida_browser_js'),'WebView2 resource missing');
assert(cpp.includes('cp52_nereida_browser.js'),'HTML injection missing');
assert(cpp.includes('id64<processor.getNumPrograms()'),'Host program IDs must be preserved');
assert(processor.includes('albedo_morph::patches.size()'),'192-program DSP backend missing');
for(const token of ['--fit-scale-x','--fit-scale-y','transform:scale(var(--fit-scale-x','root.clientWidth||window.innerWidth']){
 assert(fit.includes(token),'Viewport geometry missing '+token);
}
assert(!browser.includes('location.reload')&&!browser.includes('innerHTML='),'Browser must not reload or replace original page');
if(process.argv.includes('--expanded')){
 const match=html.split('<script type="application/json" id="albedoNewSoundsCatalog">')[1]?.split('</script>')[0];
 assert(match,'CI-expanded 64 new sounds catalog is missing');
 const patches=JSON.parse(match);
 assert.equal(patches.length,64);
 for(let i=0;i<64;i++)assert.equal(patches[i].id,128+i);
 assert(html.includes('data-collection="New"'),'New Sounds library tab missing');
}
console.log('CP52 PASS: JUCE HTML viewport fill; Nereida-style collection/family/role browser; stable indexed factory loader');
