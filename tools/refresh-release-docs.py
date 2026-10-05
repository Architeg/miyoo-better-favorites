#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Refresh RC7 documents only; preserve binary/source provenance and all payload bytes.

Use a verified downloaded ZIP and its external SHA256SUMS as the baseline.
No builds, card access or GitHub writes. The output directory must be new.
"""
import argparse
import hashlib
import json
import posixpath
import re
import subprocess
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
COMPUTER = 'App/BetterFavorites/computer/'
ZIP_NAME = 'better-favorites-1.0.0-rc.7.zip'
TOP_DOCS = ('README.md', 'CONTRIBUTING.md', 'CHANGELOG.md', 'THIRD_PARTY_NOTICES.md',
            'tools/bootstrap/README.md')
HASH_FILES = ('SHA256SUMS', COMPUTER + 'SHA256SUMS')
TRANSPORT = COMPUTER + 'transport.json'


def sha(data):
    return hashlib.sha256(data).hexdigest()


def bundle_text(path, data):
    # External release notes hold the artifact hash. Including it in the ZIP
    # would make its checksum self-referential. Preserve all other wording.
    if path == 'docs/release/notes-rc.7.md':
        data = re.sub(rb'Install ZIP SHA-256:\s*```text\s*[0-9a-f]{64}\s*```',
                      b'Install ZIP SHA-256: see the external `SHA256SUMS` download on this release.', data)
    return data


def local_links(path, data, members, commit):
    text = data.decode('utf-8')
    def destination(target):
        if not target or target.startswith(('#', '/', 'https:', 'http:', 'mailto:', 'data:')):
            return target
        file, sep, anchor = target.partition('#')
        resolved = posixpath.normpath(posixpath.join(posixpath.dirname(path), file))
        if resolved in members:
            return target
        source = resolved[len(COMPUTER):] if resolved.startswith(COMPUTER) else resolved
        if (ROOT / source).is_file():
            return f'https://github.com/Architeg/miyoo-better-favorites/blob/{commit}/{source}' + (sep + anchor if sep else '')
        raise ValueError(f'Unresolved documentation link: {path}: {target}')
    text = re.sub(r'(?<!!)(\[[^\]\n]*\]\()([^\s)]+)(\))',
                  lambda m: m[1] + destination(m[2]) + m[3], text)
    text = re.sub(r'((?:src|href)=")([^"\n]+)(")',
                  lambda m: m[1] + destination(m[2]) + m[3], text)
    return text.encode()


def refresh(original, sums, output):
    if subprocess.check_output(['git', 'status', '--porcelain'], cwd=ROOT).strip():
        raise ValueError('Commit reviewed documentation first; clean checkout required')
    commit = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT).decode().strip()
    expected = dict((n, h) for h, n in (line.split('  ', 1) for line in sums.read_text().splitlines()))
    if original.name != ZIP_NAME or sha(original.read_bytes()) != expected[ZIP_NAME]:
        raise ValueError('Baseline ZIP does not match the supplied external checksums')
    if output.exists():
        raise ValueError('Fresh output directory required; earlier archives are retained')
    with zipfile.ZipFile(original) as z:
        if z.testzip():
            raise ValueError('Baseline CRC failure')
        infos = {e.filename: e for e in z.infolist()}
        files = {n: z.read(n) for n in infos}
        for line in files['SHA256SUMS'].decode().splitlines():
            h, n = line.split('  ', 1)
            if sha(files[n]) != h:
                raise ValueError('Baseline internal checksum mismatch: ' + n)
    before = dict(files)
    docs = list(TOP_DOCS) + [p.relative_to(ROOT).as_posix() for p in sorted((ROOT / 'docs').rglob('*.md'))]
    updates = {}
    for rel in docs:
        for prefix in ('', COMPUTER):
            updates[prefix + rel] = bundle_text(rel, (ROOT / rel).read_bytes())
    # Existing icon location differs inside the computer tools subtree.
    updates[COMPUTER + 'README.md'] = updates[COMPUTER + 'README.md'].replace(
        b'src="App/BetterFavorites/icon.png"', b'src="payload/App/BetterFavorites/icon.png"')
    members = set(files) | set(updates)
    updates = {n: local_links(n, data, members, commit) for n, data in updates.items()}
    inventory = dict(format=1, documentation_commit=commit, binary_source_commit=json.loads(files['App/BetterFavorites/release.json'])['app_source_commit'],
                     baseline_zip_sha256=expected[ZIP_NAME],
                     note='Source inventories/companions retain the exact executable source. Release-note artifact checksum is external only.',
                     files={n: sha(data) for n, data in sorted(updates.items())})
    for prefix in ('', COMPUTER):
        updates[prefix + 'DOCUMENTATION-INVENTORY.json'] = (json.dumps(inventory, indent=2) + '\n').encode()
    files.update(updates)
    # Recompute nested checksums before the transport manifest, then root sums.
    files[COMPUTER + 'SHA256SUMS'] = ''.join(sha(data) + '  ' + n[len(COMPUTER):] + '\n' for n, data in sorted(files.items())
        if n.startswith(COMPUTER) and n not in (TRANSPORT, COMPUTER + 'SHA256SUMS')).encode()
    transport = json.loads(files[TRANSPORT])
    existing = {entry['path']: entry for entry in transport['files']}
    for n, data in sorted(files.items()):
        if n.startswith(COMPUTER) and n != TRANSPORT:
            path = n[len('App/BetterFavorites/'):]
            entry = existing.get(path, dict(path=path, mode=0o644))
            entry['sha256'] = sha(data)
            existing[path] = entry
    transport['files'] = [existing[n] for n in sorted(existing)]
    files[TRANSPORT] = (json.dumps(transport, indent=2) + '\n').encode()
    files['SHA256SUMS'] = ''.join(sha(data) + '  ' + n + '\n' for n, data in sorted(files.items()) if n != 'SHA256SUMS').encode()
    allowed = set(updates) | set(HASH_FILES) | {TRANSPORT}
    changed = [n for n in files if n not in infos or files[n] != before[n]]
    if any(n not in allowed for n in changed):
        raise ValueError('Non-document payload changed')
    output.mkdir(parents=True)
    with zipfile.ZipFile(output / ZIP_NAME, 'x', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for n, data in sorted(files.items()):
            if n in infos:
                info = infos[n]
            else:
                info = zipfile.ZipInfo(n, infos['README.md'].date_time)
                info.external_attr = 0o100644 << 16
                info.compress_type = zipfile.ZIP_DEFLATED
            z.writestr(info, data)
    final_sha = sha((output / ZIP_NAME).read_bytes())
    if any(final_sha.encode() in data for data in files.values()):
        raise ValueError('ZIP contains its own final checksum')
    expected[ZIP_NAME] = final_sha
    (output / 'SHA256SUMS').write_text(''.join(h + '  ' + n + '\n' for n, h in sorted(expected.items())))
    report = dict(documentation_commit=commit, zip_sha256=final_sha, changed_members=sorted(changed),
                  unchanged_members=len(infos) - len([n for n in changed if n in infos]),
                  binary_source_commit=inventory['binary_source_commit'])
    (output / 'documentation-refresh.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--original', type=Path, required=True)
    parser.add_argument('--checksums', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    refresh(args.original, args.checksums, args.output)
