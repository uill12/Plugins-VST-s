/* CP38 browser geometry contract. Run on Linux GitHub Actions with Chromium.
 * Reads exactly the canonical HTML embedded by JUCE BinaryData. */
const { chromium } = require('playwright');
const path=require('path');
const fs=require('fs');
(async()=>{
 const browser=await chromium.launch({headless:true,args:['--disable-gpu-sandbox']});
 const outputs=[];
 const viewports=[{width:1320,height:880},{width:1152,height:768},{width:900,height:600}];
 const target='file://'+path.resolve('ui/Albedo_CP30_CP22_Landscape_3x2.html');
 const assert=(v,msg)=>{if(!v)throw Error(msg)};
 for(const viewport of viewports){
  const page=await browser.newPage({viewport,deviceScaleFactor:1});
  const failures=[];
  page.on('pageerror',e=>failures.push(e.message));
  await page.goto(target,{waitUntil:'load'});
  await page.waitForSelector('#filterKnobsA .dial',{timeout:15000});
  await page.waitForSelector('#filterKnobsB .dial',{timeout:15000});
  await page.evaluate(()=>document.fonts.ready);
  await page.waitForTimeout(250);
  const metrics=await page.evaluate(()=>{
   const rect=e=>{const r=e.getBoundingClientRect();return {x:r.x,y:r.y,w:r.width,h:r.height,right:r.right,bottom:r.bottom}};
   const get=id=>{
    const root=document.getElementById(id);
    const box=root.closest('.shape-block');
    const dial=[...root.querySelectorAll('.dial')].map(rect);
    const label=[...root.querySelectorAll('.param-label')].map(rect);
    const value=[...root.querySelectorAll('.param-value')].map(rect);
    const svg=box.querySelector('svg.filter-vis');
    const header=box.querySelector('.shape-head');
    return {container:rect(root),card:rect(box),graph:rect(svg),header:rect(header),dial,label,value};
   };
   return {a:get('filterKnobsA'),b:get('filterKnobsB'),
       page:rect(document.getElementById('page')),
       scale:getComputedStyle(document.documentElement).getPropertyValue('--fit-scale').trim()};
  });
  const {a,b}=metrics;
  const near=(x,y,tol=1.1)=>Math.abs(x-y)<=tol;
  const prefix=JSON.stringify(viewport);
  assert(a.dial.length===3 && b.dial.length===3,'not three knobs in each filter '+prefix);
  for(const part of [a,b]){
    assert(part.card.w>70 && part.card.h>70,'missing card: '+prefix);
    assert(part.graph.y>part.header.y,'response graph overlaps title '+prefix);
    assert(part.dial[0].y>=part.graph.bottom-1.5,'dial overlaps response graph '+prefix);
    for(let i=0;i<3;i++){
      const d=part.dial[i],l=part.label[i],v=part.value[i];
      assert(near(d.w,d.h,.9),'non-square dial '+i+' '+prefix+' '+JSON.stringify(d));
      assert(d.w>=18 && d.h>=18,'unreadably small dial '+prefix);
      assert(d.x>=part.card.x-1 && d.right<=part.card.right+1,'dial outside filter card '+prefix);
      assert(l.y>=d.bottom-1 && v.y>=l.bottom-.6,'value/label overlap dial '+prefix);
      assert(v.bottom<=part.card.bottom+1,'value clipped by card '+prefix);
    }
    const gap1=part.dial[1].x-part.dial[0].x;
    const gap2=part.dial[2].x-part.dial[1].x;
    assert(near(gap1,gap2,1.2),'uneven column spacing '+prefix);
  }
  assert(near(a.card.y,b.card.y),'filter card top mismatch '+prefix);
  assert(near(a.card.h,b.card.h),'filter card height mismatch '+prefix);
  assert(near(a.graph.y,b.graph.y),'graph baseline mismatch '+prefix);
  assert(near(a.graph.h,b.graph.h),'graph height mismatch '+prefix);
  for(let i=0;i<3;i++){
    assert(near(a.dial[i].y,b.dial[i].y),'A/B knob center Y mismatch '+i+' '+prefix);
    assert(near(a.label[i].y,b.label[i].y),'A/B knob label Y mismatch '+i+' '+prefix);
    assert(near(a.value[i].y,b.value[i].y),'A/B value Y mismatch '+i+' '+prefix);
  }
  assert(failures.length===0,'Browser JS exceptions '+prefix+': '+JSON.stringify(failures));
  const name='Albedo_CP38_Corpus_AB_'+viewport.width+'x'+viewport.height+'.png';
  await page.screenshot({path:name,fullPage:false});
  outputs.push({viewport,metrics,jsErrors:failures,screenshot:name});
  console.log('PASS Filter A/B geometry '+viewport.width+'x'+viewport.height+
     ' dial='+a.dial[0].w.toFixed(1)+'x'+a.dial[0].h.toFixed(1)+
     ' centersY='+a.dial.map(d=>d.y.toFixed(1)).join(','));
  await page.close();
 }
 fs.writeFileSync('Albedo_CP38_Filter_AB_Geometry_QA.json',JSON.stringify(outputs,null,2));
 await browser.close();
})().catch(e=>{console.error(e.stack||e);process.exit(1)});
