"""Build four candidates per leaf. Assign existing names and roles to compatible leaves."""
import json,sys,shutil,itertools
from briefs import brief_for,realize
from pathlib import Path
import numpy as np
from scipy.optimize import linear_sum_assignment
here=Path(__file__).resolve().parents[2]
native=Path(sys.argv[2]) if len(sys.argv)>2 else here/'Albedo-JUCE-Native'
names=['material_factory_bank.json','recursive_forms_bank.json']
rows=sum([json.loads((native/'Metadata'/n).read_text()) for n in names],[])
roots=[('Voltage',(0,0,8,2)),('Vector',(1,8,0,7)),('Partial',(2,4,1,5)),('Particle',(3,7,6,2)),('Resonator',(5,2,6,0)),('Air',(6,3,5,4)),('Recorded motion',(7,3,1,0)),('Cellular phase',(9,8,1,2))]
branches=['Struck','Sustained','Swept','Stepped']
leaves=list(itertools.product(range(8),range(4),range(4)))
cost=np.zeros((128,128))
for i,p in enumerate(rows):
    v=p['overrides'];role=p['role']
    for j,(r,b,k) in enumerate(leaves):
        fav={'Percussion':0,'Pluck':0,'Drone':1,'Pad':1,'Texture':2,'Lead':2,'Bass':3,'Keys':3}[role]
        cost[i,j]=(0 if fav==b else 4)+(0 if v['1000']==roots[r][1][0] else 2 if v['1000'] in roots[r][1] else 3)+((i*37+j*17+i*j*7)%97)*.001
assign=dict(zip(*linear_sum_assignment(cost)))
for variant in range(4):
    out=Path(sys.argv[1])/str(variant);out.mkdir(parents=True,exist_ok=True)
    built=[]
    seen={}
    for i,p in enumerate(rows):
        r,b,k=leaves[assign[i]];root,methods=roots[r];role=p['role'];v=dict(p['overrides'])
        # Keep the root audible. Permute the supporting layer, register and articulation independently.
        order=[methods[0],methods[1+(k+variant)%3],methods[1+(k+variant+1)%3],methods[1+(k+variant+2)%3]]
        styles=['Fundamental','Octave','Resonant interval','Moving layer']
        for e,m in enumerate(order):
            a=1000+32*e;v.update({str(a):m,str(a+1):1,str(a+2):[93,58+variant*6,20+k*8,18+variant*7][e],
              str(a+3):0 if not e else ([0,-12,7,12][(k+e+variant)%4] if role!='Bass' or e>1 else -12),
              str(a+4):0 if not e else [-26,28,52,-54][(k+e)%4],str(a+6):[8,38,72,96][(k+e+variant)%4],
              str(a+7):[6,28,57,89][(b+variant+2*e)%4],str(a+8):[16,44,71,92][(r+k+e+variant)%4],
              str(a+9):[12,48,77,94][(k+e+2*variant)%4],str(a+10):(r*4+k+variant*3+7*e)%32,
              str(a+11):3 if b==1 and e==0 and m in (0,1,8) else 1,str(a+12):8+k*3,str(a+13):40+k*16,
              str(a+16):[3,7,15,24][(k+variant+e)%4],str(a+17):[.55,1,1.85,3.2][(k+e)%4],str(a+18):[7,30,67,94][(k+variant)%4]})
        env=[(.002,.09+.09*k,[2,9,20,31][k],.06+.08*k),(.13+.19*k,.55+k*.21,68+6*k,.74+.65*k),(.014+.03*k,.32+.2*k,35+k*12,.24+.18*k),(.002+.002*k,.1+.07*k,42+9*k,.09+.05*k)][b]
        arp=b==3 and (k in (1,3)) and i!=0
        for pid,val in {2100:15+variant*8,2101:16+k*8,3010:[1,0,2,4,3,2,0,1][r],3011:[1,2,4,0][(k+variant)%4],3012:[0,1,0,2][variant],
          3000:[240,680,1820,5150][(k+variant)%4]*(1+.11*r),3003:[570,1330,3820,8400][(k+variant+1)%4],3001:[13,35,61,79][(k+variant)%4],
          3120:[74,28,-57,51][b],3122:[.002,.19,.45,.003][b],3123:.12+.13*k,3124:12+9*k,3100:env[0],3101:env[1],3102:env[2],3103:env[3],
          6020:int(arp),6024:(r+k+variant)%4,6025:1+(r+k)%3,6026:[0,12,0,34][k],6027:[16,7,12,15][k],6028:(r+k+variant)%8,6029:120,6030:1,6021:[.25,.25,.5,.125][k],6022:30+15*k,6023:40+k*14,
          5004:[5,26,15,8][b]+variant*2,5005:1,4003:(r+k)%4}.items():v[str(pid)]=val
        for step in range(16):v[str(6000+step)]=int(step==0 or ((step*(k+2)+r)%7)<5)
        ordinal=seen.get(role,0);seen[role]=ordinal+1
        brief=brief_for(p,ordinal);recipe=realize(p,brief,v,k,variant)
        built.append(dict(p,overrides=v,recipe=recipe,ideaPath=[role,recipe['function'],recipe['instrument'],recipe['transformation']],voicing=f'{root} / {branches[b]} / {styles[k]}',permutation=variant))
    for n,bank in zip(names,(built[:64],built[64:])):(out/n).write_text(json.dumps(bank))
    for n in ['AlbedoPrograms.hpp','AlbedoProgramTrim.hpp']:shutil.copy(native/'Source'/n,out/n)
