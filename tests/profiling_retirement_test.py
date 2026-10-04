#!/usr/bin/env python3
"""Private card fixtures only: archive first, exact production launcher, owned cleanup."""
from pathlib import Path
import tempfile,hashlib,json,importlib.util,subprocess
repo=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('retire',repo/'tools/retire-profiling.py')
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
with tempfile.TemporaryDirectory(prefix='better-favorites-retirement.') as folder:
    root=Path(folder);module.M4=module.sha(b'accepted M4 fixture')
    original_run=module.subprocess.run
    module.subprocess.run=lambda args,**kwargs: original_run(['true'] if args==['sync'] else args,**kwargs)
    def fixture(name):
        card=root/name;app=card/'App/BetterFavoritesTest';app.mkdir(parents=True)
        normal=(repo/'App/BetterFavorites/launch.sh').read_bytes()
        (app/'launch.sh').write_bytes(b'#!/bin/sh\n# diagnostic fixture\nexit 0\n')
        (app/'better-favorites').write_bytes(b'cache fixture');(app/'better-favorites').chmod(0o755)
        for n in ['profile-device-launch.sh','profile-memory-session.sh','sample-device-memory.sh']:(app/n).write_bytes((repo/'tools'/n).read_bytes())
        backup=app/'.profiling-backup';backup.mkdir();(backup/'better-favorites').write_bytes(b'accepted M4 fixture');(backup/'launch.sh').write_bytes(normal)
        manifest={'version':1,'original':{'better-favorites':module.M4,'launch.sh':module.sha(normal)},'installed':{n:module.sha((app/n).read_bytes()) for n in ['better-favorites','launch.sh','profile-device-launch.sh']}}
        (backup/'manifest.json').write_text(json.dumps(manifest))
        run=app/'.profiling-results/pilot-0001';run.mkdir(parents=True)
        for n in ['metadata.txt','startup.log','launcher-stat.txt']:(run/n).write_text(n+' retained')
        (app/'settings.conf').write_text('saved preference');(app/'browser-state').write_text('saved state')
        (card/'Roms').mkdir();(card/'Roms/favourite.json').write_text('favorite untouched')
        return card,app
    for refusal in ['foreign','changed-hook','archive-failure']:
        card,app=fixture(refusal)
        if refusal=='foreign':(app/'.profiling-results/unrelated.txt').write_text('do not delete')
        if refusal=='changed-hook':(app/'profile-device-launch.sh').write_text('unrelated replacement')
        archive=root/(refusal+'-archive')
        if refusal=='archive-failure':archive.mkdir()
        before={str(p):p.read_bytes() for p in card.rglob('*') if p.is_file()}
        try:module.retire(card,archive)
        except (RuntimeError,FileExistsError):pass
        else:raise AssertionError('Expected refusal')
        assert before=={str(p):p.read_bytes() for p in card.rglob('*') if p.is_file()}
    card,app=fixture('success');archive=root/'verified-archive'
    originals={p.relative_to(app):p.read_bytes() for p in app.rglob('*') if p.is_file()}
    result=module.retire(card,archive)
    assert (app/'launch.sh').read_bytes()==(repo/'App/BetterFavorites/launch.sh').read_bytes()
    assert (app/'better-favorites').read_bytes()==originals[Path('better-favorites')]
    assert (app/'settings.conf').read_text()=='saved preference' and (app/'browser-state').read_text()=='saved state'
    for name in result['removed']:assert (archive/name).read_bytes()==originals[Path(name)] and not (app/name).exists()
    assert not (app/'.profiling-results').exists() and not (app/'.profiling-backup').exists()
    assert (archive/'retirement-verification.json').is_file()
print('Profiling retirement: verified archive, exact launcher, kept binary/data and foreign/failure refusal: PASS')
