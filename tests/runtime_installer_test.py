#!/usr/bin/env python3
"""Installer fault/conflict tests on temporary cards. Reference is read-only."""
import argparse
import importlib.util
from pathlib import Path
import sys
import tempfile
from unittest.mock import patch

sys.dont_write_bytecode = True
repo = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('return_manager', repo / 'integration/onion-return/manage.py')
manager = importlib.util.module_from_spec(spec)
spec.loader.exec_module(manager)
parser = argparse.ArgumentParser()
parser.add_argument('--reference', type=Path, required=True)
args = parser.parse_args()
original = args.reference.read_bytes()

def fixture(base):
    root = Path(base)
    for folder in ('.tmp_update/config', '.tmp_update/script', '.tmp_update/onionVersion', 'Roms'):
        (root / folder).mkdir(parents=True)
    runtime = root / '.tmp_update/runtime.sh'
    runtime.write_bytes(original)
    runtime.chmod(0o755)
    (root / '.tmp_update/onionVersion/version.txt').write_text('v4.3.1-1')
    for name in ('favourite.json', 'recentlist.json', 'recentlist-hidden.json'):
        (root / 'Roms' / name).write_bytes(b'untouched history fixture\n')
    return root, manager.paths(root)

def unchanged_history(root):
    for p in (root / 'Roms').iterdir():
        assert p.read_bytes() == b'untouched history fixture\n'

def refuses(call):
    try:
        call()
    except (RuntimeError, OSError):
        return
    raise AssertionError('Unsafe operation was accepted')

with tempfile.TemporaryDirectory(prefix='better-favorites-installer-test-') as base:
    base = Path(base)
    root, (runtime, helper, backup) = fixture(base / 'normal')
    manager.manage(root, 'install')
    assert not (root / '.tmp_update/config/.betterFavoritesReturn').exists()
    assert not (root / 'App/BetterFavoritesTest/settings.conf').exists()
    assert (backup / 'runtime.sh').read_bytes() == original
    assert runtime.stat().st_mode & 0o111
    helper.write_bytes(b'unrelated helper change')
    refuses(lambda: manager.manage(root, 'uninstall'))
    assert helper.read_bytes() == b'unrelated helper change'
    helper.write_bytes((manager.PACKAGE / 'better_favorites_return.sh').read_bytes())
    manager.manage(root, 'uninstall')
    assert runtime.read_bytes() == original and not helper.exists()
    assert (backup / 'runtime.sh').read_bytes() == original
    # Cleanup can be resumed after original runtime restoration succeeds but
    # writing the completion manifest fails.
    root2, (rt2, helper2, backup2) = fixture(base / 'retry-cleanup')
    manager.manage(root2, 'install')
    real_atomic2 = manager.atomic
    def fail_manifest(path, data, mode=0o700):
        if path == backup2 / 'manifest.json':
            raise OSError('Injected completion manifest failure')
        real_atomic2(path, data, mode)
    with patch.object(manager, 'atomic', fail_manifest):
        refuses(lambda: manager.manage(root2, 'uninstall'))
    assert rt2.read_bytes() == original and not helper2.exists()
    manager.manage(root2, 'uninstall')
    unchanged_history(root)
    unchanged_history(root2)

    # Installation and removal never activate/deactivate or rewrite a preference.
    root3, (rt3, helper3, backup3) = fixture(base / 'existing-preference')
    preference = root3 / 'App/BetterFavoritesTest/settings.conf'
    preference.parent.mkdir(parents=True)
    enabled = b'BetterFavoritesSettings1\n1\n' + b'a' * 32 + b'\n'
    preference.write_bytes(enabled)
    manager.manage(root3, 'install')
    assert preference.read_bytes() == enabled
    manager.manage(root3, 'uninstall')
    assert preference.read_bytes() == enabled
    unchanged_history(root3)

    for problem in ('wrong-hash', 'wrong-version', 'existing-helper', 'bad-backup', 'publish-failure', 'concurrent-change'):
        root, (runtime, helper, backup) = fixture(base / problem)
        if problem == 'wrong-hash': runtime.write_bytes(b'unrelated runtime')
        if problem == 'wrong-version': (root / '.tmp_update/onionVersion/version.txt').write_text('v4.4')
        if problem == 'existing-helper': helper.write_bytes(b'unrelated helper')
        before = runtime.read_bytes()
        real_atomic = manager.atomic
        def injected_atomic(path, data, mode=0o700):
            if problem == 'publish-failure' and path == runtime:
                raise OSError('Injected runtime publication failure')
            real_atomic(path, data, mode)
            if problem == 'concurrent-change' and path == helper:
                runtime.write_bytes(b'foreign runtime replacement')
        def broken_copy(source, destination):
            Path(destination).write_bytes(b'incomplete backup')
            raise OSError('Injected backup failure')
        if problem == 'bad-backup':
            with patch.object(manager.shutil, 'copy2', broken_copy):
                refuses(lambda: manager.manage(root, 'install'))
        else:
            with patch.object(manager, 'atomic', injected_atomic):
                refuses(lambda: manager.manage(root, 'install'))
        assert runtime.read_bytes() == (b'foreign runtime replacement' if problem == 'concurrent-change' else before)
        if problem == 'existing-helper': assert helper.read_bytes() == b'unrelated helper'
        else: assert not helper.exists()
        assert not (root / 'App/BetterFavoritesTest/settings.conf').exists()
        unchanged_history(root)
print('Installer version/hash gating, verified backup, default-off settings separation, rollback and foreign-change preservation: PASS')
