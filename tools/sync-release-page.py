#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Maintainer-only: apply committed RC7 notes and remove the unused RC2 draft/tag.

Requires an authenticated GitHub CLI. Does not rebuild/upload packages, touch an
SD card, move RC7's tag, or change repository visibility. Use --check for preflight.
"""
import argparse
import datetime
import json
from pathlib import Path
import subprocess

REPO = "Architeg/miyoo-better-favorites"
OLD = "v1.0.0-rc.2"
CURRENT = "v1.0.0-rc.7"
OLD_SHA = "f2960183c78be14f0598748e0909594688506ef5"
CURRENT_SHA = "e9abdc1359ee483b871d2985756379076219878a"


def run(*args):
    result = subprocess.run(args, text=True, capture_output=True)
    if result.returncode:
        raise RuntimeError(result.stderr.strip() or "GitHub command failed")
    return result.stdout


def api(path):
    return json.loads(run("gh", "api", f"repos/{REPO}/{path}"))


def assets(release):
    return sorted((a["id"], a["name"], a["size"], a.get("digest"))
                  for a in release["assets"])


def sync(root, check=False):
    notes_path = root / "docs/release/notes-rc.7.md"
    notes = notes_path.read_text()
    if "## ✨ Features" not in notes or "## 🐛 Fixes" not in notes:
        raise RuntimeError("Pull the current main branch before running this tool")
    # Paginated list includes authenticated drafts; never interpret an API error
    # as an absent release or tag.
    pages = json.loads(run("gh", "api", f"repos/{REPO}/releases",
                          "--paginate", "--slurp"))
    releases = [r for page in pages for r in page]
    current = next(r for r in releases if r["tag_name"] == CURRENT)
    old = next((r for r in releases if r["tag_name"] == OLD), None)
    refs = api("git/matching-refs/tags/")
    old_ref = next((r for r in refs if r["ref"] == f"refs/tags/{OLD}"), None)
    current_ref = next(r for r in refs if r["ref"] == f"refs/tags/{CURRENT}")
    if current["id"] != 403315285 or current["draft"]:
        raise RuntimeError("Current release identity changed; no edits made")
    if current_ref["object"]["sha"] != CURRENT_SHA:
        raise RuntimeError("RC7 tag changed; no edits made")
    if not any(a["name"] == "better-favorites-1.0.0-rc.7.zip"
               for a in current["assets"]):
        raise RuntimeError("Current install ZIP is missing; no edits made")
    if old and (old["id"] != 402740906 or not old["draft"] or old["assets"]):
        raise RuntimeError("RC2 is no longer the empty draft; no edits made")
    if old_ref and old_ref["object"]["sha"] != OLD_SHA:
        raise RuntimeError("RC2 tag changed; no edits made")
    if check:
        print("Preflight passed: RC7 notes ready; only empty RC2 draft/tag eligible.")
        return
    git_dir = Path(run("git", "-C", str(root), "rev-parse", "--absolute-git-dir").strip())
    stamp = datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%S.%fZ")
    backup = git_dir / "better-favorites-release-backups" / stamp
    backup.mkdir(parents=True)
    (backup / "before.json").write_text(json.dumps(
        {"current": current, "old": old, "refs": refs}, indent=2) + "\n")
    run("gh", "release", "edit", CURRENT, "--repo", REPO,
        "--title", "Better Favorites", "--notes-file", str(notes_path),
        "--prerelease=false", "--latest")
    updated = api(f"releases/{current['id']}")
    if (updated["body"].strip() != notes.strip() or updated["prerelease"]
            or assets(updated) != assets(current)):
        raise RuntimeError(f"Release verification failed; RC2 retained. Backup: {backup}")
    if old:
        run("gh", "api", "--method", "DELETE", f"repos/{REPO}/releases/{old['id']}")
    if old_ref:
        run("gh", "api", "--method", "DELETE", f"repos/{REPO}/git/refs/tags/{OLD}")
    final_pages = json.loads(run("gh", "api", f"repos/{REPO}/releases",
                                "--paginate", "--slurp"))
    if any(r["tag_name"] == OLD for page in final_pages for r in page):
        raise RuntimeError("RC2 release is still present; inspect the GitHub response")
    final_refs = api("git/matching-refs/tags/")
    if any(r["ref"] == f"refs/tags/{OLD}" for r in final_refs):
        raise RuntimeError("RC2 tag is still present; inspect the GitHub response")
    if next(r for r in final_refs if r["ref"] == f"refs/tags/{CURRENT}")["object"]["sha"] != CURRENT_SHA:
        raise RuntimeError("RC7 tag changed during operation; inspect GitHub")
    print("Verified: release notes updated; RC2 draft/tag removed; RC7 assets/tag retained.")
    print(f"Previous release text and refs archived: {backup}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    try:
        sync(Path(__file__).resolve().parents[1], args.check)
    except (OSError, RuntimeError, ValueError, KeyError, StopIteration) as error:
        parser.exit(1, f"Release synchronization stopped: {error}\n")
