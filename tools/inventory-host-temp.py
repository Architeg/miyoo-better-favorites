#!/usr/bin/env python3
"""Read-only Better Favorites inventory in macOS /private/tmp. Never follows links."""
from pathlib import Path
import os, stat
ROOT = Path('/private/tmp')
ACTIVE = {'better-favorites-menu-removal.patch', 'better-favorites-host-temp-inventory.md',
          'better-favorites-menu-removal-report.md'}
REFERENCES = {'better-favorites-runtime-reference.sh', 'better-favorites-runtime-patched.sh'}
def purpose(path, mode):
    name = path.name
    if name in ACTIVE: return 'Active review artifact; preserve'
    if name in REFERENCES: return 'Pinned Onion runtime fixture; preserve for checks'
    if name.startswith('better-favorites-reference-'): return 'Current visual reference; preserve during review'
    if name in {'better-favorites-menu-state-test', 'better-favorites-removal-test'} or name.startswith(('better-favorites-menu-removal-', 'better-favorites-menu-ui-', 'better-favorites-refine-removal', 'better-favorites-local-tools-', 'better-favorites-menu-docs-final', 'better-favorites-final-review')):
        return 'Current local implementation/check artifact; preserve until review completes'
    if name.startswith('better-favorites-menu-final-'): return 'Older generated host test binary; disposable'
    if name in {'better-favorites-menu-functions.txt', 'better-favorites-menu-lifecycle.txt'}: return 'One-shot implementation fragment now covered by repository source/tests; disposable'
    if name in {'better-favorites-menu-state-test', 'better-favorites-removal-test'} or name.startswith(('better-favorites-menu-removal-', 'better-favorites-menu-ui-', 'better-favorites-refine-removal', 'better-favorites-local-tools-', 'better-favorites-menu-docs-final', 'better-favorites-final-review')):
        return 'Current local implementation/check artifact; preserve until review completes'
    if name.startswith('better-favorites-menu-final-'): return 'Older generated host test binary; disposable'
    if name in {'better-favorites-menu-functions.txt', 'better-favorites-menu-lifecycle.txt'}: return 'One-shot implementation fragment now covered by repository source/tests; disposable'
    if stat.S_ISDIR(mode): return 'Isolated host test/request fixture; inspect before disposing'
    if name.endswith('.patch'): return 'Older review patch; disposable after confirming checkpoint coverage'
    if 'audit' in name or 'rejected-paths' in name: return 'Earlier path/history audit; disposable'
    if 'test' in name or name.endswith('.o'): return 'Generated host test binary/object or one-shot test fragment; disposable'
    if name.endswith('.py'): return 'One-shot edit/deploy script with checkpoint-specific paths/hashes; archival/disposable'
    if 'manifest' in name or 'guards' in name or 'backup-path' in name: return 'Past deployment metadata; disposable after backup review'
    return 'Historical work artifact; review before disposing'
def size(path):
    details = path.lstat()
    if not stat.S_ISDIR(details.st_mode): return details.st_size
    total = 0
    for base, dirs, files in os.walk(path, followlinks=False):
        for name in files: total += (Path(base) / name).lstat().st_size
        dirs[:] = [name for name in dirs if not (Path(base) / name).is_symlink()]
    return total
print('# Better Favorites host temporary files\n')
print('Read-only /private/tmp inventory. Logical bytes; links are not followed. Nothing deleted.\n')
print('Reusable tests and lifecycle tools live in tests/ and integration/onion-return/manage.py;')
print('this reusable inventory tool lives in tools/inventory-host-temp.py. Temporary edit/deploy')
print('scripts are checkpoint-specific fragments, not supported general deployment tools.\n')
print('| File/directory | Bytes | Purpose / disposition |')
print('| --- | ---: | --- |')
for path in sorted(ROOT.iterdir()):
    if not path.name.lower().startswith(('better-favorites', 'betterfavorites')): continue
    try:
        details = path.lstat()
        print(f'| {path} | {size(path):,} | {purpose(path, details.st_mode)} |')
    except OSError as error:
        print(f'| {path} | unavailable | {error} |')
