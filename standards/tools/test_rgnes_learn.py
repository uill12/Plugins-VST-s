#!/usr/bin/env python3
import copy
import unittest
from rgnes_learn import ingest, report, validate

BASE = dict(event_id="e1", project="Albedo", rule_id="midi.stuck-notes", area="midi",
            source_kind="user_feedback", status="reported", observed_at="2026-11-18",
            symptom="Sustain release hangs", source_ref="CP test feedback",
            acceptance_test="Sustain and note-off no hang")


class TestRgnesLearn(unittest.TestCase):
    def fresh(self):
        return {"schema_version": 1, "events": []}

    def test_deduplicates_identical_events(self):
        l = self.fresh()
        self.assertEqual(ingest(l, [BASE]), 1)
        self.assertEqual(ingest(l, [BASE]), 0)
        self.assertEqual(len(l["events"]), 1)

    def test_recurrence_triggers_test_but_not_success(self):
        l = self.fresh()
        e2 = {**BASE, "event_id": "e2", "observed_at": "2026-11-19", "status": "regressed"}
        ingest(l, [BASE, e2])
        row = report(l)["patterns"][0]
        self.assertTrue(row["recurring"])
        self.assertFalse(row["current_fix_tested"])
        self.assertTrue(row["needs_regression"])

    def test_assistant_claim_does_not_certify_fix(self):
        e = {**BASE, "event_id": "e3", "status": "host_test_passed", "source_kind": "user_feedback", "source_revision": "commit123"}
        with self.assertRaises(ValueError):
            validate(e)

    def test_verified_host_fix_needs_matching_evidence(self):
        e = {**BASE, "event_id": "e4", "source_kind": "host_test", "status": "host_test_passed", "source_revision": "abc123"}
        l = self.fresh()
        ingest(l, [BASE, e])
        self.assertTrue(report(l)["patterns"][0]["current_fix_tested"])
        e5 = {**BASE, "event_id": "e5", "status": "regressed", "observed_at": "2026-11-20"}
        ingest(l, [e5])
        self.assertFalse(report(l)["patterns"][0]["current_fix_tested"])

    def test_rejects_credentials_and_chat_dump(self):
        with self.assertRaises(ValueError):
            validate({**BASE, "source_ref": "secret@example.com"})
        with self.assertRaises(ValueError):
            validate({**BASE, "conversation_text": "entire transcript"})
        with self.assertRaises(ValueError):
            validate({**BASE, "source_ref": "token=ghp_abcd1234abcd1234"})

    def test_assistant_history_is_candidate_only(self):
        l = self.fresh()
        hist = {**BASE, "event_id": "old-report", "source_kind": "assistant_history", "status": "reported"}
        ingest(l, [hist])
        self.assertFalse(report(l)["patterns"][0]["current_fix_tested"])

    def test_conflicting_reused_id_is_blocked(self):
        l = self.fresh()
        ingest(l, [BASE])
        with self.assertRaises(ValueError):
            ingest(l, [{**BASE, "symptom": "different"}])


if __name__ == "__main__":
    unittest.main()
