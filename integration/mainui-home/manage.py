#!/usr/bin/env python3
"""Offline, exact-hash MainUI installer. Never modifies runtime or app preferences."""
import argparse
import datetime
import json
import os
from pathlib import Path
import stat
import tempfile
import prototype

PACKAGE = Path(__file__).resolve().parent
RETURN = PACKAGE.parent/'onion-return'


def regular(path):
    fd = os.open(path, os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK)
    try:
        info = os.fstat(fd)
        if not stat.S_ISREG(info.st_mode) or info.st_size > 4*1024*1024:
            raise RuntimeError(f'Unsafe/nonregular file: {path}')
        with os.fdopen(fd, 'rb', closefd=False) as stream:
            return stream.read(4*1024*1024+1)
    finally:
        os.close(fd)


def snapshot(path):
    try:
        return regular(path)
    except FileNotFoundError:
        return None


def directory(root, path):
    for item in [root]+list(reversed(path.relative_to(root).parents)):
        part = item if item == root else root/item
        if part.is_symlink() or not part.is_dir():
            raise RuntimeError(f'Unsafe directory: {part}')
    if path.is_symlink() or not path.is_dir():
        raise RuntimeError(f'Unsafe directory: {path}')


def card(root):
    root = root.absolute()
    for path in (root, root/'.tmp_update/bin', root/'.tmp_update/config', root/'App/BetterFavoritesTest'):
        directory(root, path)
    version = regular(root/'.tmp_update/onionVersion/version.txt').decode().strip()
    if version != 'v4.3.1-1':
        raise RuntimeError('Unsupported Onion version')
    spec = json.loads(regular(RETURN/'hashes.json'))
    runtime_hash = prototype.digest(regular(root/'.tmp_update/runtime.sh'))
    if runtime_hash == spec['patched_sha256']:
        backup = root/'.tmp_update/config/better-favorites-return-backup'
        directory(root, backup)
        manifest = json.loads(regular(backup/'manifest.json'))
        if (manifest.get('status') != 'installed' or
                any(manifest.get(k) != spec[k] for k in spec) or
                prototype.digest(regular(backup/'runtime.sh')) != spec['original_sha256'] or
                prototype.digest(regular(root/'.tmp_update/script/better_favorites_return.sh')) != spec['helper_sha256']):
            raise RuntimeError('Permanent Automatic return integration verification failed')
    elif runtime_hash != spec['original_sha256']:
        raise RuntimeError('Unsupported runtime hash')
    return root, runtime_hash


def stage(path, data, mode):
    fd, temporary = tempfile.mkstemp(prefix=path.name+'.home-stage-', dir=path.parent)
    try:
        with os.fdopen(fd, 'wb') as stream:
            os.fchmod(stream.fileno(), mode)
            stream.write(data); stream.flush(); os.fsync(stream.fileno())
        if regular(Path(temporary)) != data:
            raise RuntimeError('Staged bytes do not match')
        return Path(temporary)
    except Exception:
        Path(temporary).unlink(missing_ok=True)
        raise


# Test injection is host-only; there is no environment-variable failure switch.
def transaction(changes, check=lambda: None, hook=lambda phase, path: None):
    """All files staged first; rollback only bytes published by this invocation.

    Offline card + no concurrent filesystem writer is required. Separate checks
    are conflict detection, NOT filesystem compare-and-swap or a runtime lock.
    """
    staged, published = {}, []
    try:
        for path, (before, after, mode) in changes.items():
            if after is not None:
                staged[path] = stage(path, after, mode)
        for path, (before, after, mode) in changes.items():
            check()
            if snapshot(path) != before:
                raise RuntimeError(f'Conflicting file preserved: {path}')
            hook('before', path)
            # Hook simulates a late offline conflict for the regression fixture.
            if snapshot(path) != before:
                raise RuntimeError(f'Conflicting file preserved: {path}')
            if after is None:
                path.unlink()
            else:
                os.replace(staged[path], path)
            published.append(path) # register ownership BEFORE post-write checks
            hook('after', path)
            if snapshot(path) != after:
                raise RuntimeError(f'Publication verification failed: {path}')
    except Exception as cause:
        conflicts = []
        for path in reversed(published):
            before, after, mode = changes[path]
            try:
                if snapshot(path) != after:
                    conflicts.append(str(path)); continue
                if before is None:
                    path.unlink()
                else:
                    temporary = stage(path, before, mode)
                    try:
                        if snapshot(path) != after:
                            conflicts.append(str(path)); continue
                        os.replace(temporary, path)
                    finally:
                        temporary.unlink(missing_ok=True)
                if snapshot(path) != before:
                    conflicts.append(str(path))
            except Exception:
                conflicts.append(str(path))
        raise RuntimeError(f'{cause}; rollback conflicts retained: {conflicts}') from cause
    finally:
        for temporary in staged.values():
            temporary.unlink(missing_ok=True)


def catalogue():
    spec = json.loads(regular(PACKAGE/'package.json'))
    if spec['version'] != 'M6Home1' or spec['originals'] != prototype.HASHES:
        raise RuntimeError('Unsupported package catalogue')
    return spec


def receipt(spec):
    return ('BetterFavoritesHomeInstalled1\n'+spec['version']+'\n'+
            ''.join(spec['patched'][name]+'\n' for name in prototype.HASHES)).encode()


def manage(root, action, payload=None, hook=lambda phase, path: None):
    root, runtime = card(root); spec = catalogue()
    manifest_path = root/'.tmp_update/config/better-favorites-home.json'
    marker = root/'App/BetterFavoritesTest/home-integration.conf'
    binaries = {name: root/'.tmp_update/bin'/name for name in prototype.HASHES}
    if action == 'status':
        manifest = snapshot(manifest_path)
        print(json.dumps(dict(runtime_sha256=runtime, manifest=None if manifest is None else json.loads(manifest),
            binaries={n: prototype.digest(regular(p)) for n,p in binaries.items()},
            receipt_matches=snapshot(marker)==receipt(spec)), indent=2)); return
    prepared_bytes = None
    old_manifest = snapshot(manifest_path)
    if action == 'install':
        if old_manifest is not None:
            previous=json.loads(old_manifest)
            if (previous.get('status')!='uninstalled' or previous.get('version')!=spec['version'] or
                    previous.get('original')!=spec['originals'] or previous.get('patched')!=spec['patched']):
                raise RuntimeError('Existing or conflicting installation/recovery manifest preserved')
            previous_backup=Path(previous['backup'])
            if (previous_backup.is_absolute() or '..' in previous_backup.parts or len(previous_backup.parts)!=3 or
                    previous_backup.parts[:2]!=('.tmp_update','config') or not previous_backup.name.startswith('better-favorites-home-backup-')):
                raise RuntimeError('Unsafe retained backup path')
            directory(root,root/previous_backup)
            for name,sha in spec['originals'].items():
                if prototype.digest(regular(root/previous_backup/name))!=sha:
                    raise RuntimeError('Retained original backup failed verification')
        if snapshot(marker) is not None:
            raise RuntimeError('Existing availability marker preserved')
        code = regular(payload or PACKAGE/'adapter.elf')
        if prototype.digest(code) != spec['payload_sha256']:
            raise RuntimeError('Payload does not match reviewed package hash')
        originals, replacements, modes = {}, {}, {}
        for name, path in binaries.items():
            original = regular(path)
            if prototype.digest(original) != spec['originals'][name]:
                raise RuntimeError(f'Unsupported or conflicting MainUI: {name}')
            patched, _ = prototype.patch(original, code)
            if prototype.digest(patched) != spec['patched'][name]:
                raise RuntimeError('Generated output hash mismatch')
            originals[name], replacements[name] = original, patched
            modes[name] = path.stat().st_mode & 0o777
        stamp = datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S.%fZ')
        backup = root/'.tmp_update/config'/('better-favorites-home-backup-'+stamp)
        backup.mkdir(mode=0o700)
        for name, original in originals.items():
            temp = stage(backup/name, original, modes[name]); os.replace(temp, backup/name)
        # Verify ALL originals and backups before replacing even the first binary.
        for name, path in binaries.items():
            if regular(backup/name)!=originals[name] or regular(path)!=originals[name]:
                raise RuntimeError('Backup verification failed; no installation files replaced')
        manifest = dict(version=spec['version'],status='installed',backup=str(backup.relative_to(root)),
            original=spec['originals'],patched=spec['patched'],modes=modes,runtime_sha256=runtime)
        changes = {path: (originals[n],replacements[n],modes[n]) for n,path in binaries.items()}
        changes[marker] = (None,receipt(spec),0o600)
        prepared = dict(manifest,status='prepared')
        prepared_bytes = (json.dumps(prepared,indent=2)+'\n').encode()
        # A verified journal permits conservative recovery after interruption.
        saved_manifest = stage(backup/'manifest.json',prepared_bytes,0o600)
        os.replace(saved_manifest,backup/'manifest.json')
        if regular(backup/'manifest.json')!=prepared_bytes: raise RuntimeError('Backup manifest verification failed')
        transaction({manifest_path:(old_manifest,prepared_bytes,0o600)},check=lambda: card(root))
        changes[manifest_path] = (prepared_bytes,(json.dumps(manifest,indent=2)+'\n').encode(),0o600)
    else:
        if old_manifest is None: raise RuntimeError('No installed manifest')
        manifest = json.loads(old_manifest)
        if (manifest.get('status') not in ('installed','prepared') or manifest.get('version')!=spec['version'] or
                manifest.get('original')!=spec['originals'] or manifest.get('patched')!=spec['patched']):
            raise RuntimeError('Manifest/package mismatch')
        relative = Path(manifest['backup'])
        if relative.is_absolute() or '..' in relative.parts or len(relative.parts)!=3 or relative.parts[:2]!=('.tmp_update','config') or not relative.name.startswith('better-favorites-home-backup-'):
            raise RuntimeError('Unsafe backup path')
        backup = root/relative; directory(root,backup)
        changes = {}
        for name,path in binaries.items():
            current, original = regular(path),regular(backup/name)
            mode = manifest['modes'][name]
            if not isinstance(mode,int) or not 0<=mode<=0o777: raise RuntimeError('Invalid backup mode')
            allowed = (spec['originals'][name],spec['patched'][name]) if manifest['status']=='prepared' else (spec['patched'][name],)
            if prototype.digest(current) not in allowed or prototype.digest(original)!=spec['originals'][name]:
                raise RuntimeError(f'Changed installed file or backup preserved: {name}')
            changes[path]=(current,original,mode)
        marker_bytes=snapshot(marker)
        allowed_markers=(None,receipt(spec)) if manifest['status']=='prepared' else (receipt(spec),)
        if marker_bytes not in allowed_markers: raise RuntimeError('Changed availability marker preserved')
        if marker_bytes is not None: changes[marker]=(marker_bytes,None,0o600)
        manifest['status']='uninstalled'
        changes[manifest_path]=(old_manifest,(json.dumps(manifest,indent=2)+'\n').encode(),0o600)
    try:
        transaction(changes,check=lambda: card(root),hook=hook)
    except Exception:
        # Keep prepared recovery state if any binary has a rollback conflict.
        if prepared_bytes is not None and all(snapshot(path)==originals[n] for n,path in binaries.items()) and snapshot(marker) is None:
            if snapshot(manifest_path)==prepared_bytes:
                transaction({manifest_path:(prepared_bytes,old_manifest,0o600)},check=lambda: card(root))
        raise
    print(f'{action}: verified; takes effect after reboot. Originals retained: {backup}')


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action',choices=['status','install','uninstall'])
    parser.add_argument('--sd-root',type=Path,required=True)
    parser.add_argument('--payload',type=Path)
    parser.add_argument('--powered-off',action='store_true')
    args=parser.parse_args()
    if args.action!='status' and not args.powered_off: parser.error('Mutations require --powered-off; close all other card writers')
    try: manage(args.sd_root,args.action,args.payload)
    except (OSError,ValueError,KeyError,RuntimeError) as error: parser.exit(1,str(error)+'\n')
