# RGNES learning-event schema and persistence

Feed **small, technical, sanitized JSON events** into `rgnes_learn.py`. Never ingest full chat transcripts by default. The event must cite either a user correction, exact checkpoint/document, real build/QA log, source commit, or host verification. Avoid secrets, emails, tokens, passwords, personal data and direct message text. Store only what is necessary for engineering reproducibility.

Required: `event_id`, `project`, `rule_id`, `area`, `source_kind`, `status`, `observed_at`, `symptom`, `source_ref`.
Optional: `corrective_action`, `acceptance_test`, `source_revision`, `target_format`, `environment`, `severity`.

- `source_kind`: `user_feedback`, `assistant_history` (unverified prior assistant report), `build_log`, `source_audit`, `offline_test`, `host_test`, `user_confirmation`.
- `status`: `reported`, `reproduced`, `fix_attempted`, `offline_test_passed`, `host_test_passed`, `user_confirmed`, `regressed`, `superseded`, `blocked`.
- `source_ref`: a relevant, non-sensitive public commit/filename/checkpoint reference, not a complete private transcript or prompt.
- `rule_id`: stable namespace such as `midi.stuck-notes`, `gui.hover-rerender`, or `build.ci-stall`.

Example:
```json
{"event_id":"albedo-88200-report-2026-11-15","project":"Albedo","rule_id":"audio.88200-crash","area":"audio","source_kind":"user_feedback","status":"reported","observed_at":"2026-11-15","symptom":"Host playback crashes or freezes at 88.2 kHz","source_ref":"prior project development feedback","acceptance_test":"Host load, sustained chords, release and preset switching at 88.2 kHz with no crash","severity":"high"}
```

Persistence: `python rgnes_learn.py record --event event.json --ledger learning/ledger.json`; `python rgnes_learn.py report --ledger learning/ledger.json`. The script deduplicates by event_id, computes cumulative patterns, and flags repeated issues; it deliberately **does not** rewrite plugin source automatically. Commit only sanitized ledger and necessary test scripts to GitHub. When source_ref refers to an inaccessible chat, cite the type of earlier feedback but do not fake a permanent link.

Promotions: `offline_test_passed` requires a real corresponding test and source revision. `host_test_passed` requires host evidence. A user saying “fix this” is `reported`, not `user_confirmed`. A new `regressed` event reopens a previously passing rule. Automated suggestions never override a newer explicit user requirement.
