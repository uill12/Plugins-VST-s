/* Albedo CP59: extend the existing synthesis Lab, keep native UI styling. */
(function(){'use strict';
const labels=["WT · Tidal","WT · Sohler Root","WT · Microwave Haze","WT · Saw Bend","WT · Vocal Form","WT · Vocal Prism","WT · Talking","WT · Kawai Motion","WT · Soft Machine","WT · Bowing","WT · Organic Organ","WT · Harmonix","WT · High Harmonic","WT · Harmonic Pulse","WT · Enshturz","WT · Chebyshev","WT · Wavetable Shift","WT · Fourier Keys","WT · Spectral Bloom","WT · Braids Pluck","WT · AM Sine","WT · Sax Contour","WT · Astral Grain","WT · Tidy Pluck"];
function register(){
 const api=window.ALBEDO_CP04;if(!api||!Array.isArray(api.materials))return;
 if(api.materials.length===32)api.materials.push(...labels);
 for(const p of (api.registry?.parameters||[])){
  if(p.id>=1000&&p.id<=1127&&(p.id-1000)%32===10)p.max=55;
 }
 if(typeof api.refresh==='function')api.refresh();
}
if(document.readyState==='loading')document.addEventListener('DOMContentLoaded',register,{once:true});else register();
})();
