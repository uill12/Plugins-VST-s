from pathlib import Path
r=Path(__file__).resolve().parents[1]
# Landscape desktop presentation: change geometry, not just stretch bitmap.
f=r/'ui/Nigredo_CP06_Standalone.html';h=f.read_text()
h=h.replace('CP06 Alchemical GUI','CP07 Landscape GUI').replace('CP06','CP07')
css='''
/* CP07 — 16:10 landscape: geometric reflow, not a stretched square. */
@media (min-width:1121px) {
 #app {max-width:none;padding:8px 15px 9px;}
 .topbar {min-height:62px;padding:3px 8px 5px;}
 .brand-mark {width:42px;height:42px}
 .brand-name {font-size:21px}
 .brand-sub {font-size:8px;margin-top:0}
 .mode-button {padding:7px 12px}
 .main-power {width:34px;height:34px}
 .intro {margin:7px 0 8px;min-height:51px}
 .intro-ornament {font-size:25px}
 .intro h1 {font-size:24px;margin:0}
 .intro p {font-size:10px}
 .eyebrow {font-size:8px}
 .counter {font-size:24px}
 .trinity {gap:7px;margin-bottom:7px}
 .trinity-tile {min-height:29px}
 .trinity-tile b {font-size:12px}
 .trinity-tile small {font-size:8px}
 .workbench {grid-template-columns:214px minmax(540px,1fr) 416px;gap:10px;align-items:stretch;}
 .panel-heading {min-height:32px;padding:8px 13px;font-size:10px}
 .stages-panel {min-height:0}
 .ledger-caption {padding:7px 8px 5px;font-size:12px}
 .stages-list {padding:4px 7px 3px;gap:1px}
 .stage-item {min-height:54px}
 .stage-select {padding:6px 2px 6px 4px;gap:5px}
 .stage-item .number {font-size:15px;width:19px}
 .stage-glyph {height:26px;width:26px;font-size:17px}
 .stage-copy b {font-size:13px}
 .stage-copy small {font-size:7px}
 .stage-toggle {height:26px;width:22px}
 .stages-footer {margin:auto 10px 8px;padding:6px 0 0}
 .stages-footer-mark {font-size:17px}
 .stages-footer div span {font-size:11px;margin:2px 0}
 .stages-footer small {font-size:8px;line-height:1.25}
 .field-top {padding:8px 16px 0}
 .field-stage h2 {font-size:23px}
 .stage-id {font-size:24px;height:38px;width:38px}
 .field-metadata {font-size:9px}
 .cosmogram-shell {min-height:0;flex:0 0 auto;padding:4px 11px 0}
 .cosmogram {width:clamp(310px,27vw,416px);}
 .path-node {width:48px;height:48px}
 .node-symbol {font-size:17px}
 .path-node b {font-size:9px}
 .path-node small {font-size:6px;top:calc(100% + 4px)}
 .wheel-caption {font-size:12px;margin:0 0 5px}
 .hermetic-scroll {margin:3px 15px 6px;width:calc(100% - 30px);padding:4px 8px;font-size:11px}
 .scope-wrap {height:76px !important}
 .scope-title {font-size:8px}
 .stage-editor-header {padding:6px 13px 5px}
 .editor-glyph {height:34px;width:34px;font-size:23px}
 .editor-names h2 {font-size:22px}
 .editor-names div {font-size:8px}
 .stage-power {padding:5px 7px}
 .editor-desc {font-size:10px;line-height:1.3;margin:5px 0 0;min-height:29px}
 .parameters {padding:0 11px}
 .param-group {padding:6px 0 3px}
 .param-heading {font-size:8px}
 .knob-row {margin-top:3px;gap:3px}
 .knob {width:46px;height:46px}
 .knob:before {inset:4px}
 .knob:after {inset:10px}
 .param-name {font-size:8px;min-height:20px;margin-top:0}
 .param-val {font-size:9px}
 .fader-track {height:46px}
 .param-choice {height:30px;margin:7px auto;font-size:14px}
 .param-switch {width:62px;height:40px;margin:4px auto 2px}
 .param-switch .switch-gem {font-size:19px}
 .parameter-footer {margin:5px 11px 5px;padding-top:5px}
 .parameter-footer button {font-size:9px;padding:4px 6px}
 .intent-scroll {padding:5px 12px 7px}
 .intent-quote {font-size:13px;margin:2px 0 3px}
 .intent-scroll p {font-size:9px;line-height:1.3;margin:2px 0 0}
 .intent-scroll .section-small {font-size:7px}
 .lineage {font-size:7px;gap:6px;margin-top:4px}
 .bottom-altar {gap:9px;margin-top:8px;grid-template-columns:.8fr 1.1fr 1fr}
 .altar-panel {min-height:0;padding:6px 12px}
 .altar-title {font-size:9px;padding:0 0 4px;margin-bottom:4px}
 .altar-panel p {font-size:10px;line-height:1.35}
 .master-controls .knob {width:36px;height:36px}
 .master-controls .knob:after {inset:9px}
 .master-controls .param-name {min-height:12px}
 .master-controls .param-val {font-size:8px}
 .master-controls {gap:2px}
 .preset-space {gap:3px}
 select {min-height:26px;padding:4px 7px;font-size:11px}
 .preset-actions button {padding:4px 3px;font-size:9px}
 .master-footer {margin-top:3px;padding-top:3px;font-size:7px}
 .site-footer {margin-top:5px;padding:5px 4px 0;font-size:8px}
}
'''
h=h.replace('</style>',css+'</style>')
f2=r/'ui/Nigredo_CP07_Standalone.html';f2.write_text(h)
# Keep index and source css aligned.
f3=r/'ui/styles.css';f3.write_text(f3.read_text()+css)
print('Wrote',f2)
