#!/usr/bin/env python3
"""CP66 source-aware instrument potential for each terminal leaf.

Source root defines characteristic spectral/temporal cues; terminal generators
can transform beyond the original color when useful for a quality instrument. This is a
parameter/provenance gate, not a substitute for listening to rendered audio.
"""
import argparse, json, math
from collections import Counter
from pathlib import Path

BANKS=("material_factory_bank.json","recursive_forms_bank.json","morph_expansion_bank.json")
# The identity is descriptive, not a false claim that all of these acoustic
# properties were measured in the DSP output.
IDENTITIES={
 "fundamental":dict(anchor="stable low partial concentration",permit=("envelope","register","subtle phase","dynamics"),avoid=("broadband noise","heavy spectral spread"),harmonic="sparse"),
 "low-harmonic":dict(anchor="dominant lower harmonics",permit=("sub articulation","filter slope","envelope","gentle wavetable scanning"),avoid=("constant bright overtone blanket",),harmonic="low-weighted"),
 "vocal-formant":dict(anchor="formant peaks and vowel-like spectral notches",permit=("formant movement","phrase contour","articulation","air onset"),avoid=("flatten formant contrast",),harmonic="formant"),
 "acoustic-resonant":dict(anchor="characteristic resonant modes and decay",permit=("damping","excitation","modal decay","controlled friction"),avoid=("uniform distortion wash",),harmonic="modal"),
 "metallic-spectral":dict(anchor="distinct upper/inharmonic partial clusters",permit=("partial spacing","transient shaping","resonance","controlled detune"),avoid=("lowpass until metallic signature disappears",),harmonic="high-contrasted"),
 "digital-inharmonic":dict(anchor="discrete nonuniform digital partial relationships",permit=("phase shape","alias-safe spectral shifts","articulation","digital contour"),avoid=("smoothing all peaks into generic saw",),harmonic="inharmonic"),
 "evolving-granular":dict(anchor="recognizable moving grain/formant source",permit=("grain time","density","texture","dynamic frame scanning"),avoid=("fixed static common spectrum",),harmonic="moving"),
 "stochastic":dict(anchor="noise excitation texture and controlled bandwidth",permit=("excitation duration","filter color","grain articulation","resonance"),avoid=("undifferentiated permanent hiss",),harmonic="noise")
}
def evaluate(p):
    root=p["root_material"];leaf=p["concept_tree_terminal"]
    typ=root["type"];v=p["overrides"]
    identity=IDENTITIES[typ]
    methods=[int(v[str(1000+32*i)]) for i in range(4)]
    levels=[float(v[str(1002+32*i)]) for i in range(4)]
    active=[i for i,x in enumerate(levels) if x>0]
    complexity=root["complexity"]
    assert len(active)=={"minimal":1,"restrained":2,"layered":3,"dense":4}[complexity],(p["index"],complexity,levels)
    assert methods[0]==int(root["primary_method"]),(p["index"],methods[0],root["primary_method"])
    # A selected table must be a *sounding* source (unless consciously
    # minimal with a non-wavetable primary where no wavetable is claimed).
    table=int(root["table_id"])
    wavetable_found=any(methods[i]==1 and int(v.get(str(1010+32*i),-1))==table for i in active)
    uses_root_table=complexity!="minimal" or methods[0]==1
    if uses_root_table:
        assert wavetable_found,(p["index"],typ,methods,active,table)
    # Never use number of methods or number of branches as an acoustic
    # similarity metric; demand audio analyses for tone identity and novelty.
    if typ=="fundamental":
        assert complexity=="minimal",(p["index"],complexity)
        assert all(levels[i]==0 for i in (1,2,3))
    # The intended stochastic source cannot be counted if its engine is muted.
    noise_active=any(methods[i]==6 for i in active)
    claimed=leaf["noise"]["enabled"]
    # Annotation may show inherited dormant engine method, but no *audible*
    # noise is present unless its generator level is nonzero.
    leaf["noise"]["audible"]=noise_active
    p["timbre_identity"]={
      "root":typ,"source_table_id":table,"source_sha256":root["licensed_source_sha256"],
      "identity_anchor":identity["anchor"],
      "harmonic_identity":identity["harmonic"],
      "promising_transformations":list(identity["permit"]),
      "risk_factors_to_evaluate":list(identity["avoid"]),
      "transformations_are_recommendations_not_hard_limits":True,
      "selection_criterion":"rendered instrument quality, musical function, perceptual usefulness and diversity",
      "complexity":complexity,"active_engine_indices":active,
      "table_is_audible":wavetable_found,"noise_audible":noise_active,
      "target_vs_render_validation":"pending",
      "source_provenance_retained":True,
      "original_timbre_preservation_optional":True}
    return {"index":p["index"],"root":typ,"active":len(active),
            "table_is_audible":wavetable_found,"noise_audible":noise_active,
            "identity_anchor":identity["anchor"],"render_comparison_required":True}
def main():
    a=argparse.ArgumentParser()
    a.add_argument("--metadata",required=True,type=Path)
    a.add_argument("--report",required=True,type=Path)
    args=a.parse_args()
    paths=[args.metadata/f for f in BANKS]
    banks=[json.loads(p.read_text(encoding="utf-8")) for p in paths]
    patches=sum(banks,[])
    assert len(patches)==192
    results=[evaluate(p) for p in patches]
    assert len({p["root"] for p in results})==8
    for path,bank in zip(paths,banks):
        path.write_text(json.dumps(bank,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
    args.report.write_text(json.dumps({"checkpoint":"CP66",
      "identity_contracts":len(results),"root_count":8,
      "complexity":dict(Counter(p["active"] for p in results)),
      "status":"source provenance and structural checks only; instrument quality and distinctness pending rendered DSP QA",
      "presets":results},ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
    print("CP66 potential: 192 source-aware possibility profiles; instrument-quality render QA required")
if __name__=="__main__": main()
