"""Design briefs: intended function, reference instrument, trait, source material and permissible treatments."""
# Each line assigns a primary engine, a supporting engine/material, and a palette of transformations.
DATA = {
"Bass":[
("Weight","Minimoog-style bass","rounded fundamental and a short resonant attack",0,0,24,"ladder,sub,pluck","sub floor carbon pressure foundation"),
("Elastic pulse","Bitwig Polymer-style bass","moving pulse width above a stable bass note",1,8,15,"sweep,cell,sub","elastic circuit phase pulse"),
("Pitched impact","plucked bass string","woody strike followed by a pitched decay",5,7,6,"damp,pluck,sub","wood root stone felt membrane")],
"Lead":[
("Solo line","double reed","an odd-harmonic core with an airy edge",2,6,4,"formant,breath,sweep","reed voice air breath"),
("Brass gesture","Diva-style analog brass","two oscillators swelling into a bright attack",0,0,1,"detune,swell,ladder","brass filament copper flare"),
("Bending line","phase-distortion lead","a sharp moving spectral knee",8,1,15,"sweep,fold,cell","prism signal beam edge")],
"Pad":[
("Ensemble bed","Diva-style strings","slightly detuned layers and slow bow onset",0,1,10,"detune,swell,room","drift string velvet loom"),
("Suspended harmony","glass harmonica","a stable harmonic body with a changing upper halo",2,4,1,"partials,swell,room","glass halo crystal porcelain"),
("Diffuse canopy","granular chamber ensemble","smoothed particulate motion around a chord",3,7,21,"grain,swell,room","canopy grain memory cloud")],
"Pluck":[
("Dry attack","thumb piano","a short tine transient and a hollow body",5,2,27,"pluck,damp,partials","timber reed wooden resonant"),
("Articulated motion","Polymer-style vector pluck","a moving spectral attack with a clean release",1,8,26,"fold,pluck,cell","pulse step lattice relay"),
("Brittle accent","porcelain or glass bar","inharmonic upper modes with a quick decay",4,7,7,"partials,pluck,damp","glass ceramic quartz prism")],
"Keys":[
("Chord articulation","reed electric piano","velocity-brightened attack over a soft body",2,7,22,"partials,damp,pluck","amber reed felt chord"),
("Sustained chord","analog polysynth","wide pulse beating with an opening filter",0,0,17,"detune,ladder,swell","brass interval velvet dusk"),
("Bell key","struck mineral","a bright metallic onset which separates from its tail",5,4,2,"pluck,partials,room","mineral porcelain oblique glass")],
"Percussion":[
("Hollow hit","wood block","short impact and a strongly tuned wooden body",5,7,6,"pluck,damp,sub","knock block wood transient"),
("Metal tick","small gong or bell","a bright noisy strike with a ringing upper mode",4,6,3,"partials,pluck,room","tin shard ice copper"),
("Noise hit","shaker or scraped membrane","a distributed noisy onset and short filtered decay",6,3,17,"breath,grain,damp","rain dust sand skin paper")],
"Drone":[
("Pedal tone","reed organ","stationary pitch with slow beating upper partials",2,0,4,"partials,detune,room","horizon held interval"),
("Evolving low bed","bowed metal","a swelling inharmonic body and a slowly moving resonance",4,5,20,"swell,formant,room","orbit pressure turbine"),
("Continuous field","stretched resonant sample","slow changes in grain position over a tonal anchor",7,3,16,"grain,sweep,room","exposure submerged archive")],
"Texture":[
("Particulate layer","Max Cooper-inspired granular instrument","a stable body against individually moving grains",3,7,21,"grain,cell,room","particle dust scatter"),
("Breath layer","filtered air column","a noisy excitation shaped into a shifting resonance",6,5,10,"breath,formant,sweep","air ember breath"),
("Interlocking motion","cellular phase instrument","periodic partials whose accents evolve independently",9,8,30,"cell,fold,room","clock gate circuit magnetic")]
}
def brief_for(p,ordinal):
    pool=DATA[p['role']]
    words=(p['name']+' '+p.get('description','')).lower()
    return max(enumerate(pool),key=lambda it:(sum(w in words for w in it[1][-1].split()),it[0]==ordinal%len(pool)))[1]
def realize(p,b,v,k,variant):
    job,ref,trait,primary,support,material,choices,_=b
    operators=choices.split(',');operation=operators[(k+variant)%3]
    # Root comes from the reference brief. Swap the supporting technique only within that palette.
    v['1000']=primary;v['1032']=support
    v['1042']=material;v['1010']=material
    if support not in (2,3,7) and primary not in (2,3,7):
        v['1064']=7;v['1074']=material;v['1066']=16+5*k
    v['1014']=min(98,38+variant*16)
    if operation=='ladder':v.update({'3010':1,'3002':5+2*k,'3120':48+8*k})
    if operation=='sub':v.update({'1019':32+7*k,'1035':-12,'1004':0})
    if operation=='fold':v.update({'1008':62+10*k,'1064':8,'1072':75})
    if operation=='cell':v.update({'8000':4,'8001':2,'8002':34+9*k,'4001':54+8*k,'8300':max(2,v['8300'])})
    if operation=='formant':v.update({'3011':0,'3003':650+390*k,'3004':38+9*k})
    if operation=='breath':v.update({'1064':6,'1066':21+7*k,'1082':25+19*k,'2100':32})
    if operation=='swell':v.update({'3100':.09+.13*k,'3122':.08+.11*k,'3120':42})
    if operation=='room':v.update({'5004':22+5*k,'5005':1,'5011':83+12*k})
    if operation=='grain':v.update({'1064':3,'1074':material,'1079':k,'1066':32+8*k})
    if operation=='partials':v.update({'1064':2,'1080':4+6*k,'1081':.65+.38*variant,'1066':37+5*k})
    if operation=='detune':v.update({'1032':0,'1053':-5-3*k,'1011':3 if primary in (0,1,2,8,9) else 1,'1012':6+2*k})
    if operation=='pluck':v.update({'3100':.002+.002*k,'3101':.09+.08*k,'3102':5+4*k,'3123':.08+.04*k})
    if operation=='damp':v.update({'3000':max(100,v['3000']*.64),'3103':.09+.11*k,'5004':4})
    if operation=='sweep':v.update({'8000':0,'8001':0,'8002':28+9*k,'4000':.19+.44*k,'8300':max(2,v['8300'])})
    return dict(function=job,instrument=ref,extract=trait,primaryEngine=primary,supportEngine=v['1032'],material=material,allowedTransformations=operators,transformation=operation)
