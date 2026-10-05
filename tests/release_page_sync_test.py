#!/usr/bin/env python3
"""Offline guards for the maintainer release-page tool; no real GitHub writes."""
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location("sync_release", Path(__file__).resolve().parents[1] / "tools/sync-release-page.py")
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)


class ReleasePageTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        p = self.root / "docs/release/notes-rc.7.md"
        p.parent.mkdir(parents=True)
        p.write_text("# Better Favorites\n\n## ✨ Features\n\n## 🐛 Fixes\n")
        self.current = dict(id=403315285, tag_name=mod.CURRENT, draft=False,
                            prerelease=True, body="Old notes", assets=[dict(
                                id=1, name="better-favorites-1.0.0-rc.7.zip", size=100, digest="same")])
        self.old = dict(id=402740906, tag_name=mod.OLD, draft=True, assets=[])
        self.refs = [dict(ref="refs/tags/" + tag, object=dict(sha=sha))
                     for tag, sha in [(mod.OLD, mod.OLD_SHA), (mod.CURRENT, mod.CURRENT_SHA)]]
        self.writes = []
        self.mock = patch.object(mod, "run", self.gh_run)
        self.mock.start()
        self.addCleanup(self.mock.stop)

    def gh_run(self, *args):
        if args[0] == "git":
            return str(self.root / ".git")
        if args[1:3] == ("release", "edit"):
            self.writes.append(args)
            self.current.update(body=(self.root / "docs/release/notes-rc.7.md").read_text(), prerelease=False)
            return ""
        if "DELETE" in args:
            self.writes.append(args)
            if args[-1].endswith("/releases/402740906"):
                self.old = None
            elif args[-1].endswith("/tags/" + mod.OLD):
                self.refs = [r for r in self.refs if r["ref"] != "refs/tags/" + mod.OLD]
            else:
                raise AssertionError("Unexpected deletion")
            return ""
        path = args[2]
        if path.endswith("/releases"):
            return json.dumps([[self.current] + ([self.old] if self.old else [])])
        if path.endswith("/git/matching-refs/tags/"):
            return json.dumps(self.refs)
        if path.endswith("/releases/403315285"):
            return json.dumps(self.current)
        raise AssertionError(args)

    def test_success_and_idempotent_repeat(self):
        mod.sync(self.root)
        self.assertEqual(len(self.writes), 3)
        self.assertFalse(self.current["prerelease"])
        self.assertEqual(len(self.refs), 1)
        backup = list((self.root / ".git/better-favorites-release-backups").glob("*/before.json"))
        self.assertEqual(json.loads(backup[0].read_text())["current"]["body"], "Old notes")
        mod.sync(self.root)
        self.assertEqual(len(self.writes), 4)

    def test_check_has_no_writes(self):
        mod.sync(self.root, check=True)
        self.assertEqual(self.writes, [])
        self.assertFalse((self.root / ".git").exists())

    def test_nonempty_or_published_old_release_preserved(self):
        for fields in [dict(assets=[dict(id=9)]), dict(draft=False), dict(id=0)]:
            saved = self.old.copy()
            self.old.update(fields)
            with self.assertRaises(RuntimeError):
                mod.sync(self.root)
            self.old = saved
        self.assertEqual(self.writes, [])

    def test_changed_tag_preserved(self):
        for ref in self.refs:
            saved = ref["object"]["sha"]
            ref["object"]["sha"] = "changed"
            with self.assertRaises(RuntimeError):
                mod.sync(self.root)
            ref["object"]["sha"] = saved
        self.assertEqual(self.writes, [])

    def test_read_failure_not_treated_as_missing_release(self):
        with patch.object(mod, "run", side_effect=RuntimeError("Permission denied")):
            with self.assertRaises(RuntimeError):
                mod.sync(self.root)
        self.assertEqual(self.writes, [])

    def test_asset_change_stops_before_cleanup(self):
        original = self.gh_run
        def changed(*args):
            output = original(*args)
            if args[1:3] == ("release", "edit"):
                self.current["assets"][0]["size"] += 1
            return output
        with patch.object(mod, "run", changed):
            with self.assertRaises(RuntimeError):
                mod.sync(self.root)
        self.assertEqual(len(self.writes), 1)
        self.assertIsNotNone(self.old)


if __name__ == "__main__":
    unittest.main()
