# RGNES CI stall prevention and ChatGPT-first escalation

Goal: prevent GitHub Actions from becoming an unbounded retry queue while maintaining native Windows x64 CLAP/VST3 proof.

1. Complete most work in ChatGPT: audit source/resources, run fast platform-neutral unit/DSP tests, generate configs, create manifest/checksums, and try local Windows cross-build when a supported target/sysroot is present.
2. Do not dispatch GitHub Actions if source, SDKs, bank resources or build script are missing. A workflow declaration is not a successful build.
3. Group generic CI by workflow, project path and branch to avoid multiple queued copies of the same project. Prefer a 45-minute maximum for the generic fallback, but keep project-specific, measured longer exceptions.
4. A timed-out, canceled or failed runner is **not** a compiled release. Capture run URL, revision, log reason and stage; record one `build.ci-stall` or specific `build.*` event, and investigate locally before another dispatch.
5. Do not run the same exact revision and failure signature again automatically. One retry only after a corrective source/env change; no polling indefinitely within an active ChatGPT turn. CI cannot be used to access hidden ChatGPT Library workspaces.
6. Static PE/exports validation plus runtime package verification precede QA; Bitwig host testing and real-time audio behavior remain separate.
7. Feedback from the next real development interaction is ingested into the learning ledger; two independent recurrences trigger a general regression test/checklist proposal. No autonomous model training or silent source rewrites.
