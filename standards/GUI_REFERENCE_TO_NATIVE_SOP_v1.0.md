# Reference-to-Native GUI Engineering Standard (RGNES) v1.0

**Scope:** Audio software GUIs, especially CLAP/VST3 instruments and effects on Windows, with adaptable principles for desktop applications.  
**Reference case:** Rubedo Atlas Drumkit, CP01–CP10; CP08R2 was user-confirmed to open/render in Windows Bitwig, while CP09/CP10 have offline QA only as of this standard.  
**Status:** Project engineering convention; not a system-wide ChatGPT account configuration.

## Non-negotiable principles

1. **The image is the visual reference, never the behavior model.** A bitmap may represent immutable ornamentation; editable labels, meters, knobs, pads, indicators, menus and controls must be separately rendered, hit-tested, and driven by real state.
2. **One source of truth for geometry, state, parameters and MIDI mapping.** HTML, native painting, hit detection, host parameter routing, saved state and tests consume the same authoritative specification. Do not maintain unrelated hard-coded coordinates or mappings.
3. **No decorative-but-dead controls.** Every visible interactive affordance has a specific action and observable result, or is honestly disabled and labeled accordingly.
4. **Visual approval is a design lock, not a functional release.** After approval, no redesign during porting unless the user requests it. Functional fixes must preserve proportions, artwork, typographic hierarchy and identity.
5. **Host certification requires a host test.** A valid DLL, ZIP, screenshot or offline DSP pass does not establish that the editor, MIDI, automation or presets work in Bitwig. Label the exact test evidence.
6. **Audio-thread safety and deterministic state** take precedence over visual convenience.
7. **Keep rollback checkpoints** and release both binaries and source in reproducible, verifiable form.

## Stage 1 — Reference intake and visual analysis

**Inputs:** Original reference image(s), brand palette, target formats, intended minimum/default UI dimensions, platform/DPI assumptions, required sound/control functions. Archive the unmodified reference and its hash.

- Record width, height, aspect ratio, color palette, lighting, ornamentation, typography, shadows, baseline grid, major visual anchors and spacing.
- Segment the artwork into: (a) immutable/background art, (b) stateful/dynamic artwork, (c) interactive controls, (d) editable data/labels, (e) temporary feedback and popovers.
- Mark all visible text or highlights likely to change (kit/preset name, pad note, MIDI Learn status, tempo, parameter value). **Never leave contradictory baked-in labels behind live labels.** Mask, remove, or fully replace those areas in the approved artwork.
- Annotate each component with a stable ID and a bounding box in **reference coordinates**, e.g. a 1512×1009 canvas for the Rubedo case. Capture grouping, z-order, clickable extent, and label/value areas.
- Record ambiguous UI symbols and decide their function before coding; unassigned symbols remain visual, not secretly clickable.

**Gate G1:** Approved visual atlas and control inventory. All control-like elements classified; no unexplained dynamic labels embedded in the background.

## Stage 2 — Single-source design specification

Create machine-readable definitions rather than copying the image as a single functional interface:

- `design_tokens.json`: colors, type sizes, corner radii, border treatments, effects, spacing and input feedback states.
- `layout_spec.json`: reference canvas, components, rectangles, grouping, anchors, minimum dimensions, clipping/z-order and breakpoint rules.
- `control_registry.csv` or JSON: **stable control ID; visual bounds; input gesture; display format; parameter/state ID; unit; range; default; taper; update cadence; automatable/saved flag; owning thread; test ID.**
- `midi_map` / `event_contract`: authoritative mapping between actual incoming note numbers and instrument pad IDs, including an explicit duplicate-assignment policy and unmapped-note behavior.
- `resource_manifest`: original art, generated assets, fonts or fallbacks, bank pointers, CRC/SHA-256, file packaging.

**Coordinate rule:** Use one matrix `M` to map the design canvas to the physical editor, including scale/letterbox offsets. Painting uses `M`, pointer hit-testing uses `inverse(M)`, text layout and invalidation use the same transform. No ad hoc pad-coordinate special cases.

**State rule:** The visible value is a projection of authoritative plugin state. UI event → requested change → host/engine state → notification → repaint. Do not maintain an unrelated “display-only” number.

**Gate G2:** Every visible action has exactly one registry entry and a tested behavior; no overlapping hit regions without an explicit priority; the layout can be reproduced independently from the reference raster.

## Stage 3 — Interactive HTML reference implementation

Build a full-resolution, interactive HTML/CSS/SVG/Canvas prototype **before** native porting. Use the approved reference and the design specification, not independent re-interpretation.

- Implement real press, drag, mouse wheel, keyboard focus/activation, menu/popup and active/hover/disabled states appropriate to each control.
- Simulate the actual event/state model (note-on, note-off, pad selection, MIDI Learn, kit/preset change, step sequencer and parameter change). A demo audio simulation must be labeled as simulation.
- Distinguish immutable decoration from interactive controls; do not use an image-map of guessed clickable areas as the sole interface.
- Match baseline dimensions and compare a captured screenshot against the approved reference. Check component landmark placement, labels, value alignment, line wrapping, text occlusion and color contrast. Document intentional deviations.
- Inspect 100%, 125%, 150% and 200% DPI/scale, plus the approved default and smallest allowed window. Do not assume browser CSS pixels equal native device pixels.

**Gate G3 (visual lock):** User-approved reference prototype; no overlapping components, truncated labels, unresponsive controls or visually ambiguous active states. Freeze the approved hierarchy and stable IDs.

## Stage 4 — Native rendering and input implementation

Implement the frozen design with a renderer appropriate to the host (native Win32/GDI, a proven GPU path, or an established framework); a lightweight native renderer is acceptable if the dynamic controls are genuinely dynamic.

**Recommended layers (bottom → top):** static reference ornamentation → reactive shapes and meters → labels/values → transient pad/knob states → focused popovers/tooltips. Debug counters and test hit boxes are **development-only** and absent from a release.

- Use the same layout registry for painting, hit-testing, focus, accessibility labels where supported and dirty-rectangle invalidation.
- Re-render damaged regions after host resizes/exposes/reparents the editor. Implement `show`, `hide`, `set_size`, focus and close/open behavior; use DPI-aware dimensions.
- Draw foreground text with appropriate transparent text/background handling, clip to component regions, and give text explicit bounds. A bright opaque diagnostic rectangle crossing controls is a build failure.
- Knobs must correctly map angular/drag/wheel gestures through their parameter taper and quantization, support fine adjustment and keyboard controls, and display exact stored values.
- Do not invent working behavior for baked-in labels such as MIDI hardware “connect” when that responsibility belongs to the DAW. Label the actual workflow instead.
- In Win64, validate system ABI structures with compile-time assertions and use host-safe window life-cycle handling. For Rubedo, the incorrect `CREATESTRUCTA` layout caused `GWLP_USERDATA` to contain an invalid pointer and a blank editor. Maintain the correct `lpCreateParams` offset and check all creation paths.

**Gate G4:** Native editor opens, paints, resizes, and receives input reliably in a test harness. Reference-versus-native overlays and interaction recorder pass; no blank surfaces or debug overlays.

## Stage 5 — Functional plugin integration and thread model

**Plugin loader:** Verify PE architecture, DLL entry point, exported CLAP/VST3 factory entry points, correct module handle and resource resolution. Perform a real host-load smoke test. For Rubedo CP08, a PE DLL with no entry point prevented `DllMain` from recording the module handle; `clap_entry.init()` then failed to locate the embedded bank. This class of failure requires a build and resource-load gate.

**Separate ownership:**

- Audio thread owns voices, DSP and real-time event consumption. No GUI-triggered heap allocation, file I/O, mutex blocking or direct unsafe mutation of live voices.
- GUI thread owns windows, painting, focus, pointer/keyboard input and temporary animation. It sends bounded requests/events to the engine.
- Host/plugin interfaces own parameter automation, transport, preset/state transactions and sanctioned main-thread callbacks. Respect the CLAP/VST3 thread and lifecycle contracts.

**Controls:** Every visible continuous control must have an explicit DSP/state destination, correct range/taper, and a detectable or documented sonic/behavioral response. New per-voice parameters are stored reliably; do not silently claim host automation for values that are only in plugin-private state.

**Host notifications:** Implement parameter changes and automation gestures through the host-supported mechanism; mark state dirty when supported, request host main-thread callbacks from the correct context, and avoid stale UI mirrors. Preserve stable plugin IDs, parameter IDs and compatible preset/state version migration.

**MIDI:** One map `note → pad` drives sound triggering, pad highlight, displayed note number, selected pad and saved state. MIDI Learn binds the **currently selected** target; next valid note-on captures the note; cancel works; collision policy is deterministic (e.g. swap assignments); unmapped input is reported but not silently routed elsewhere. Core and Extended page labels resolve from their actual pad IDs, not the first page of a background bitmap. Incoming event feedback remains visible long enough to recognize (for example 180–250 ms) but must not alter note timing.

**Gate G5:** Loader, DSP, MIDI, editor and host notifications pass integration tests. With neutral settings, audio comparisons prove unrelated GUI changes did not alter the existing sound bank.

## Stage 6 — Verification matrix and acceptance thresholds

| Category | Mandatory checks | Release criterion |
|---|---|---|
| Visual parity | Approved reference vs native screenshot; default/min window; 100/125/150/200% scaling | No unapproved layout drift, occlusion, debug overlays or stale labels |
| Control completeness | Sweep control registry; click, drag, wheel, reset, focus; hover/press/disabled | Every actionable visual control has a wired action and observable outcome |
| Parameter correctness | min/default/max; nonlinear taper; host automation; restore after reopening | Display, stored state and engine agree |
| MIDI and pads | All default notes, remapped notes, Core/Extended boundaries, repeated chords, learn conflict, unassigned notes | Note → pad → sound → highlight → saved note number always agree |
| Sequencer | Generate, Clear, per-step edit, four variations, tempo/transport, save/reopen | Exact pattern state and UI recovered; no unhandled buttons |
| Host lifecycle | Scan, instantiate, destroy, reopen, resize, change preset rapidly, multiple instances | No init failures, blank views, stale class/pointer, crash or deadlock |
| Audio | 44.1/48/88.2/96 kHz; varying buffers; polyphony/stress; silence/clipping | No NaNs, unsafe spikes, unexpected timbral regression; document any intended change |
| Packaging | Validate archive volumes, hashes, full extraction, installed binary/exports | All parts present; one-step 7-Zip extraction succeeds; checksums match |

**Two levels of evidence:**
- **Offline verified:** unit tests, screenshot diff, native PE/export checks, archive decode and DSP harness. This does **not** imply Bitwig tested.
- **Host verified:** Windows Bitwig interaction tests, MIDI controller routing, automation, preset change, state save/reopen, editor reopen and resizing. Clearly identify which plugin **format** (CLAP and/or VST3) actually passed.

Document failures with reproducible steps, screenshots, exact host/OS/binary version, and log excerpt. Any regression in G4/G5 returns to the owning stage; do not make cosmetic patches that obscure an incorrect event/state model.

## Stage 7 — Versioning, delivery and rollback

Use a small number of meaningful checkpoints:

- **CP-A — Design lock:** reference atlas, HTML/SVG prototype, design tokens, control registry and approved screenshot.
- **CP-B — Native implementation:** source, ABI checks, render/input state, MIDI/parameter bindings, compatibility tests.
- **CP-C — Integration/release:** Windows CLAP/VST3 builds that have actually passed their available tests, host-test report, hashes, approved resources, installation and rollback instructions.

Number individual bug-fix checkpoints sequentially when necessary and include build date in metadata. Preserve at least the last host-confirmed functional build. Do not overwrite the only usable release with an untested one.

**Distribution convention:** `Google Drive / PROJETOS / <Project Name> / <Checkpoint>`; one package per format; source/QA alongside; native 7-Zip multi-volume `.7z.001`, `.002` etc. of approximately 28–30 MiB for connector limits. Put all volumes in one folder and open/extract **only `.7z.001`**. Verify archive integrity and extracted binary hash before publishing. GitHub holds source and audit reports; binary distribution must be explicitly confirmed (do not imply GitHub Release assets exist when they do not).

## Failure-prevention rules distilled from Rubedo

| Observed incident | Engineering failure | Preventive gate |
|---|---|---|
| CP08 `init() returned false` | DLL entry-point and embedded-resource path not initialized | PE entry-point/export/resource validation and real host-load smoke test |
| CP08R1 blank GUI | Incorrect Win64 `CREATESTRUCTA` field order, invalid window userdata | ABI compile-time assertions, native create/paint/reopen test |
| CP09 overlapping status / dead controls | Diagnostics and control regions not integrated with approved visual layers; incomplete engine bindings | No debug overlays; control-registry coverage and interaction sweep |
| CP09/CP10 mapping drift | Static pad-card labels and divergent MIDI Learn target/mapping state | Unified note-map registry, dynamic labels, Core/Extended and state round-trip tests |
| ZIP extraction/download failures | Archive transport and corruption assumptions | Native 7-Zip multipart archive, per-volume checksums, independent full decode, Drive readback |

## Definition of Done

A GUI is **not finished** when it resembles a reference, compiles, or plays a note. It is finished only when visual fidelity, every intended control, bidirectional state, MIDI/automation, lifecycle, real-time safety, preset restoration, native host behavior and distribution integrity have each passed their documented release gate. A partial success is labeled as partial; unsupported controls are not presented as functional.

**Applicability:** This standard is independent of visual style. Rubedo's alchemical parchment design is not imposed on Nereida, Albedo, Nigredo, or other projects; only the engineering method is reused.