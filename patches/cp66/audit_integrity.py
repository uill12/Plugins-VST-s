#!/usr/bin/env python3
"""CP66 source/GUI/parameter semantic audit for generated 192 patches.

This test intentionally DOES NOT claim that candidates are studio-quality:
audio rendering, loudness/spectral metrics and listening remain separate gates.
"""
import argparse,collections,json,math
from pathlib import Path

BANKS=("material_factory_bank.json","recursive_forms_bank.json","morph_expansion_bank.json")
MATERIAL_IDS=(1010,1042,1074,1106)
def read_registry(path):
    html=path.read_text(encoding="utf-8")
    marker="const cp04Registry="
    i=html.find(marker)
    assert i>=0,"canonical GUI parameter schema missing"
    registry=json.JSONDecoder().raw_decode(html[i+len(marker):])[0]
    params={str(p["id"]):p for p in registry["parameters"]}
    assert len(params)>=190
    assert all(params[str(k)]["max"]==55 for k in MATERIAL_IDS)
    return params

def audit(metadata,gui,report_path):
    definitions=read_registry(gui)
    rows=sum((json.loads((metadata/name).read_text(encoding="utf-8")) for name in BANKS),[])
    assert len(rows)==192 and [r["index"] for r in rows]==list(range(192))
    assert len({r["name"] for r in rows})==192
    roots=collections.Counter();complexity=collections.Counter()
    methods=collections.Counter();source=collections.Counter()
    noise_count=0
    # Extra native 9003 dB trim is explicitly exposed in CP37, separately
    # from the original embedded cp04Registry.
    native_extra={"9003":(-18,12)}
    results=[]
    for p in rows:
        idx=p["index"];v=p["overrides"];root=p["root_material"]
        leaf=p["concept_tree_terminal"];quality=p["instrument_quality_search"]
        profile=p["timbre_identity"]
        roots[root["type"]]+=1;complexity[root["complexity"]]+=1
        assert not quality["is_audio_validated"] and quality["requires_rendered_qa"]
        assert quality["candidates_generated"]==8
        assert profile["target_vs_render_validation"]=="pending"
        assert p["wolfram_generation"]["arpeggiator_character"]==int(v["6031"])
        assert 1<=int(v["6031"])<=8 and 0<=int(v["6024"])<=3
        assert int(v["3111"])==0 and int(v["3131"])==0
        assert int(v["1000"])==int(root["primary_method"])
        active=0
        has_stochastic=False
        for e in range(4):
            b=1000+32*e
            method=int(v[str(b)])
            level=float(v[str(b+2)])
            power=int(v[str(b+1)])
            assert power==(1 if level>0 else 0),(idx,"enabled/level mismatch",e)
            methods[method]+=power
            active+=power
            if method==6 and power:has_stochastic=True
            if method==1 and power and int(v[str(b+10)])==int(root["table_id"]):
                source[int(root["table_id"])]+=1
        expected={"minimal":1,"restrained":2,"layered":3,"dense":4}[root["complexity"]]
        assert active==expected==leaf["active_engines"],(idx,active,expected,leaf["active_engines"])
        assert leaf["noise"]["audible"]==has_stochastic,(idx,"noise metadata mismatch")
        if root["type"]=="stochastic":
            assert int(v["1000"])!=6 and has_stochastic,(idx,"noise lacks pitched core")
        if expected>1 or int(v["1000"])==1:
            assert int(root["table_id"]) in source,(idx,"source missing from bank")
            # Check per-patch rather than cumulative table use
            assert any(int(v[str(1000+32*e)])==1
                       and int(v[str(1001+32*e)])==1
                       and int(v[str(1010+32*e)])==int(root["table_id"])
                       for e in range(4)),(idx,"selected source inaudible")
        assert leaf["source_levels"]==[float(v[str(1002+32*e)]) for e in range(4)]
        for mix,enabled in ((5002,5003),(5004,5005),(5006,5007)):
            assert int(v[str(enabled)]) in (0,1)
            assert int(v[str(enabled)])==(1 if float(v[str(mix)])>1 and active>1 else 0),(idx,mix)
        for key,value in v.items():
            assert isinstance(value,(int,float)) and math.isfinite(float(value)),(idx,key)
            if key in definitions:
                d=definitions[key]
                assert float(d["min"])-1e-5<=float(value)<=float(d["max"])+1e-5,(idx,key,value,d["min"],d["max"])
                if d.get("stepped"):
                    assert abs(float(value)-round(float(value)))<1e-5,(idx,key,value,"noninteger enum")
            else:
                assert key in native_extra,(idx,key,"unregistered parameter override")
                lo,hi=native_extra[key]
                assert lo<=float(value)<=hi
        noise_count+=has_stochastic
        results.append({"index":idx,"root":root["type"],"active_sources":active,
                        "Wolfram_character":v["6031"],"noise_audible":has_stochastic,
                        "proxy_score_not_audio_quality":quality["proxy_score"]})
    assert len(roots)==8 and len(complexity)==4
    assert len(source)>=16 and len(methods)>=6
    assert 0<noise_count<192
    report={"checkpoint":"CP66","audit":"native/GUI schema and semantic generation gates",
            "result":"source-level checks pass only; no audio quality certified",
            "preset_count":len(rows),"root_distribution":dict(roots),
            "complexity_distribution":dict(complexity),
            "sounding_wavetables":len(source),"audible_noise_presets":noise_count,
            "live_method_count":len(methods),
            "audio_render_verified":False,"manual_bitwig_verified":False,
            "presets":results}
    report_path.write_text(json.dumps(report,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
    print("CP66: 192 final presets pass source/GUI range, power, source, noise and safety contract. AUDIO QA PENDING.")

if __name__=="__main__":
    parser=argparse.ArgumentParser()
    parser.add_argument("--metadata",required=True,type=Path)
    parser.add_argument("--gui",required=True,type=Path)
    parser.add_argument("--report",required=True,type=Path)
    args=parser.parse_args()
    audit(args.metadata,args.gui,args.report)
