#!/usr/bin/env python3
"""Host-side, offline-card installer. No installation occurs without an action."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

PACKAGE = Path(__file__).resolve().parent

def digest(data):
    return hashlib.sha256(data).hexdigest()

def regular(path):
    if path.is_symlink() or not path.is_file():
        raise RuntimeError(f'Expected a regular file: {path}')
    return path.read_bytes()

def atomic(path, data, mode=0o700):
    fd, name = tempfile.mkstemp(prefix=path.name + '.install-', dir=path.parent)
    try:
        with os.fdopen(fd, 'wb') as out:
            os.fchmod(out.fileno(), mode)
            out.write(data)
            out.flush()
            os.fsync(out.fileno())
        os.replace(name, path)
        if regular(path) != data:
            raise RuntimeError(f'Publication verification failed: {path}')
    finally:
        if os.path.exists(name):
            os.unlink(name)

def paths(root):
    root = root.resolve(strict=True)
    system = root / '.tmp_update'
    for path in (system, system / 'config', system / 'script'):
        if path.is_symlink() or not path.is_dir():
            raise RuntimeError(f'Unsafe installation directory: {path}')
    return (system / 'runtime.sh', system / 'script/better_favorites_return.sh',
            system / 'config/better-favorites-return-backup')

def check_version(root, spec):
    if regular(root / '.tmp_update/onionVersion/version.txt').decode().strip() != spec['version']:
        raise RuntimeError('Unsupported Onion version')

def apply_patch(original):
    with tempfile.TemporaryDirectory(prefix='better-favorites-runtime-install-') as directory:
        target = Path(directory) / 'runtime.sh'
        target.write_bytes(original)
        subprocess.run(['patch', '--batch', '--forward', str(target),
                        str(PACKAGE / 'runtime.patch')], check=True, capture_output=True)
        subprocess.run(['sh', '-n', str(target)], check=True)
        return target.read_bytes()

def install(root):
    runtime, helper, backup = paths(root)
    spec = json.loads((PACKAGE / 'hashes.json').read_text())
    check_version(root, spec)
    original = regular(runtime)
    blob = hashlib.sha1(b'blob ' + str(len(original)).encode() + b'\0' + original).hexdigest()
    if blob != spec['original_git_blob'] or digest(original) != spec['original_sha256']:
        raise RuntimeError('Runtime is not the verified unmodified v4.3.1-1 runtime')
    if any(p.exists() or p.is_symlink() for p in (helper, backup)):
        raise RuntimeError('Integration/backup already exists; refusing to replace it')
    payload = regular(PACKAGE / 'better_favorites_return.sh')
    patched = apply_patch(original)
    if digest(payload) != spec['helper_sha256'] or digest(patched) != spec['patched_sha256']:
        raise RuntimeError('Package hash mismatch')
    subprocess.run(['sh', '-n', str(PACKAGE / 'better_favorites_return.sh')], check=True)
    backup.mkdir(mode=0o700)
    shutil.copy2(runtime, backup / 'runtime.sh')
    with (backup / 'runtime.sh').open('rb') as saved:
        os.fsync(saved.fileno())
    if regular(backup / 'runtime.sh') != original or regular(runtime) != original:
        raise RuntimeError('Backup verification failed; runtime not replaced')
    mode = runtime.stat().st_mode & 0o777
    manifest = dict(spec, mode=mode, status='prepared')
    atomic(backup / 'manifest.json', (json.dumps(manifest, indent=2)+'\n').encode(), 0o600)
    helper_written = False
    try:
        # Card must be offline: hashes are conflict checks, not filesystem CAS.
        if regular(runtime) != original or helper.exists() or helper.is_symlink():
            raise RuntimeError('Installation files changed')
        helper_written = True
        atomic(helper, payload)
        if regular(runtime) != original:
            raise RuntimeError('Runtime changed before publication')
        atomic(runtime, patched, mode)
        manifest['status'] = 'installed'
        atomic(backup / 'manifest.json', (json.dumps(manifest, indent=2)+'\n').encode(), 0o600)
    except Exception:
        # Restore only bytes published by this installation; preserve strangers.
        if digest(regular(runtime)) == spec['patched_sha256']:
            atomic(runtime, original, mode)
        if helper_written and helper.is_file() and not helper.is_symlink() and digest(helper.read_bytes()) == spec['helper_sha256']:
            helper.unlink()
        raise
    print(f'Installed; app setting unchanged (default off). Verified backup: {backup}')

def installed(root, allow_restored=False):
    runtime, helper, backup = paths(root)
    if backup.is_symlink() or not backup.is_dir():
        raise RuntimeError('Missing or unsafe verified backup directory')
    manifest = json.loads(regular(backup / 'manifest.json'))
    spec = json.loads((PACKAGE / 'hashes.json').read_text())
    for key in spec:
        if manifest.get(key) != spec[key]:
            raise RuntimeError('Backup manifest does not match this package')
    check_version(root, spec)
    if digest(regular(backup / 'runtime.sh')) != spec['original_sha256']:
        raise RuntimeError('Original backup verification failed')
    runtime_hash = digest(regular(runtime))
    restored = allow_restored and runtime_hash == spec['original_sha256']
    if runtime_hash != spec['patched_sha256'] and not restored:
        raise RuntimeError('Installed runtime changed; refusing to overwrite unrelated changes')
    if helper.exists() or helper.is_symlink():
        if digest(regular(helper)) != spec['helper_sha256']:
            raise RuntimeError('Installed helper changed; refusing to overwrite unrelated changes')
    elif not restored:
        raise RuntimeError('Installed helper is missing')
    if not isinstance(manifest.get('mode'), int) or not 0 <= manifest['mode'] <= 0o777:
        raise RuntimeError('Invalid backup mode')
    return runtime, helper, backup, manifest

def manage(root, action):
    if action == 'install':
        install(root)
        return
    runtime, helper, backup, manifest = installed(root, action == 'uninstall')
    if action == 'uninstall':
        # Publish verified original first. A subsequent failure leaves an inert
        # helper, not a runtime depending on missing installed files.
        original = regular(backup / 'runtime.sh')
        atomic(runtime, original, manifest['mode'])
        if helper.exists():
            helper.unlink()
        manifest['status'] = 'uninstalled'
        atomic(backup / 'manifest.json', (json.dumps(manifest, indent=2)+'\n').encode(), 0o600)
        print(f'Original runtime restored and verified; backup retained: {backup}')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=['install', 'uninstall'])
    parser.add_argument('--sd-root', type=Path, required=True,
                        help='Offline mounted card root; do not run against a live device')
    args = parser.parse_args()
    try:
        manage(args.sd_root, args.action)
    except (OSError, RuntimeError, ValueError, subprocess.CalledProcessError) as error:
        parser.exit(1, f'{error}\n')
