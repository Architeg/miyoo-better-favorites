#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Independent docs-only archive check; no firmware/card/device required."""
import argparse
import hashlib
import importlib.util
import json
import re
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('refresh', ROOT / 'tools/refresh-release-docs.py')
refresh = importlib.util.module_from_spec(spec)
spec.loader.exec_module(refresh)
sha = lambda b: hashlib.sha256(b).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline', type=Path)
    parser.add_argument('--updated', type=Path)
    args = parser.parse_args()
    text = b'Intro\nInstall ZIP SHA-256:\n```text\n' + b'a' * 64 + b'\n```\nEnd\n'
    assert b'a' * 64 not in refresh.bundle_text('docs/release/notes-rc.7.md', text)
    assert refresh.bundle_text('README.md', text) == text
    assert refresh.local_links('docs/a.md', b'[Source](../src/main.cpp)', {'docs/a.md'}, 'abc').startswith(b'[Source](https://github.com/')
    try:
        refresh.local_links('docs/a.md', b'[Bad](absent.md)', {'docs/a.md'}, 'abc')
        raise AssertionError('Unresolved link accepted')
    except ValueError:
        pass
    if args.baseline:
        with zipfile.ZipFile(args.baseline) as before, zipfile.ZipFile(args.updated) as after:
            assert after.testzip() is None
            names = set(after.namelist())
            inv = json.loads(after.read('DOCUMENTATION-INVENTORY.json'))
            allowed = set(inv['files']) | set(refresh.HASH_FILES) | {refresh.TRANSPORT, 'DOCUMENTATION-INVENTORY.json', refresh.COMPUTER + 'DOCUMENTATION-INVENTORY.json'}
            for entry in before.infolist():
                if entry.filename not in allowed:
                    assert before.read(entry.filename) == after.read(entry.filename), entry.filename
                    assert entry.external_attr == after.getinfo(entry.filename).external_attr, entry.filename
            assert set(after.namelist()) - set(before.namelist()) <= allowed
            for n, h in inv['files'].items():
                assert sha(after.read(n)) == h, n
                refresh.local_links(n, after.read(n), names, inv['documentation_commit'])
            for n in refresh.HASH_FILES:
                prefix = refresh.COMPUTER if n.startswith(refresh.COMPUTER) else ''
                for line in after.read(n).decode().splitlines():
                    h, rel = line.split('  ', 1)
                    assert sha(after.read(prefix + rel)) == h, rel
            for f in json.loads(after.read(refresh.TRANSPORT))['files']:
                assert sha(after.read('App/BetterFavorites/' + f['path'])) == f['sha256'], f['path']
            final = sha(args.updated.read_bytes()).encode()
            assert all(final not in after.read(n) for n in names)
            assert before.read('SOURCE-INVENTORY.json') == after.read('SOURCE-INVENTORY.json')
            assert before.read('App/BetterFavorites/release.json') == after.read('App/BetterFavorites/release.json')
    print('PASS: checksum exclusion, broken-link rejection, document inventories, unchanged payload bytes/modes and binary provenance')


if __name__ == '__main__':
    main()
