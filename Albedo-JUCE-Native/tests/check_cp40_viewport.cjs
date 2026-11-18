// CP40: native host size, CSS viewport and visualViewport may differ under Bitwig/Windows DPI.
// Test the real geometry controller without browser/network dependencies.
'use strict';
const fs=require('fs'),vm=require('vm'),assert=require('assert'),path=require('path');
const root=path.resolve(__dirname,'..');
const source=fs.readFileSync(path.join(root,'Assets/cp35_editor_geometry.js'),'utf8');
const html=fs.readFileSync(path.join(root,'../ui/Albedo_CP30_CP22_Landscape_3x2.html'),'utf8');
const rotary=fs.readFileSync(path.join(root,'Assets/cp38_rotary_overlays.js'),'utf8');
assert(!rotary.includes('cp38-visual'),'Unexpected floating control popup');
assert(!rotary.includes('document.addEventListener(\'pointerdown\''),'Unexpected contextual trigger');
assert(html.includes("id='cp37AdsrPopup'"),'ADSR must remain');
const properties=new Map(),listeners={};
const rootNode={clientWidth:1200,clientHeight:800,style:{setProperty:(k,v)=>properties.set(k,v)}};
const grip={addEventListener(){},setPointerCapture(){},hasPointerCapture(){return false}};
const controls={id:'',style:{},setAttribute(){},querySelector(s){
  return s==='#albedo-cp35-resize-grip'?grip:{addEventListener(){}};
}};
const page={};
const document={
  documentElement:rootNode,body:{appendChild(){}},head:{appendChild(){}},
  getElementById:id=>id==='page'?page:id==='albedo-cp35-size-value'?{textContent:''}:null,
  createElement:tag=>tag==='div'?controls:{style:{},textContent:'',id:''}
};
const emitted=[],hostListeners={};
const window={
  innerWidth:1200,innerHeight:800,
  visualViewport:{width:1000,height:675,addEventListener(){}},
  __JUCE__:{backend:{emitEvent:(name,data)=>emitted.push([name,data]),addEventListener:(name,fn)=>hostListeners[name]=fn}},
  addEventListener:(name,fn)=>listeners[name]=fn
};
const context={window,document,ResizeObserver:class{observe(){}},requestAnimationFrame:fn=>{fn();return 1},console};
vm.runInNewContext(source,context,{filename:'cp35_editor_geometry.js'});
const check=(w,h,scale)=>{
  assert.strictEqual(properties.get('--albedo-viewport-w'),w+'px');
  assert.strictEqual(properties.get('--albedo-viewport-h'),h+'px');
  assert(Math.abs(Number(properties.get('--fit-scale'))-scale)<1e-7,
    'Wrong CSS viewport scale: '+properties.get('--fit-scale')+' expected '+scale);
};
check(1200,800,.8); // VisualViewport=1000x675 and host size differ: must NOT shrink.
hostListeners.albedoViewport('1122,748');
window.ALBEDO_CP35_WINDOW.fit();
check(1200,800,.8); // JUCE logical size must not cap the CSS canvas.
rootNode.clientWidth=900;rootNode.clientHeight=600;
window.ALBEDO_CP35_WINDOW.fit();check(900,600,.6);
rootNode.clientWidth=1500;rootNode.clientHeight=1000;
window.ALBEDO_CP35_WINDOW.fit();check(1500,1000,1);
assert(!source.includes('window.setInterval'),'Avoid repeated rescale polling');
assert(source.includes('right:8px;bottom:8px'),'Resize widget must anchor to actual viewport');
console.log('CP40 PASS: no contextual overlays, ADSR retained, CSS viewport fills 1200x800, 900x600, 1500x1000');