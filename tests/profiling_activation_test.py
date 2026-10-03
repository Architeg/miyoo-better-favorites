#!/usr/bin/env python3
"""Offline private-card fixtures: own-file publication, protected data and rollback."""
from pathlib import Path
import tempfile,subprocess,hashlib,os,json
repo=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='better-favorites-profile-activation.') as folder:
    root=Path(folder)
    old=b'\x7fELF\x01\x01'+b'\0'*12+b'\x28\x00'+b'accepted fixture'
    new=old+b'diagnostic'
    tool=root/'manage.py'
    source=(repo/'tools/manage-profiling.py').read_text()
    # Private fixtures do not need a global card sync; real installer retains sync.
    source=source.replace("subprocess.run(['sync'],check=True)","subprocess.run(['true'],check=True)")
    source=source.replace('REPO=Path(__file__).resolve().parents[1]','REPO=Path('+repr(str(repo))+')')
    source=source.replace("M4='aa486abad4e3272f86605945109bf0a0f76159ab373f87c1e57c38abd034b328'","M4="+repr(hashlib.sha256(old).hexdigest()))
    tool.write_text(source);binary=root/'new-arm';binary.write_bytes(new)
    def card(name):
        card=root/name;app=card/'App/BetterFavoritesTest';app.mkdir(parents=True)
        (app/'better-favorites').write_bytes(old);(app/'launch.sh').write_bytes((repo/'App/BetterFavoritesTest/launch.sh').read_bytes())
        for name in ('settings.conf','browser-state','browser-preferences.conf','config.json'):(app/name).write_text(name+' untouched')
        (card/'Roms').mkdir();(card/'Roms/favourite.json').write_text('favorites untouched')
        (card/'.tmp_update/config/better-favorites-return-backup').mkdir(parents=True);(card/'.tmp_update/config/better-favorites-return-backup/manifest.json').write_text('integration untouched')
        return card,app
    def command(operation,card,powered=True):
        return ['python3',str(tool),operation,'--card',str(card),'--binary',str(binary)]+(['--powered-off'] if powered else [])
    mounted,app=card('normal')
    assert subprocess.run(command('activate',mounted,False),capture_output=True).returncode!=0
    protected={p:p.read_bytes() for p in mounted.rglob('*') if p.is_file() and p.name not in ('better-favorites','launch.sh')}
    subprocess.run(command('activate',mounted),check=True,capture_output=True)
    assert (app/'better-favorites').read_bytes()==new
    assert (app/'.profiling-backup/better-favorites').read_bytes()==old
    assert all(p.read_bytes()==data for p,data in protected.items())
    subprocess.run(command('status',mounted,False),check=True,capture_output=True)
    # New session leaves pilot evidence and accepted originals byte-identical.
    pilot=app/'.profiling-results/pilot-0001';pilot.mkdir(parents=True)
    (pilot/'startup.log').write_text('preserved pilot')
    originals={p:p.read_bytes() for p in (app/'.profiling-backup').iterdir() if p.name!='manifest.json'}
    prior={n:(app/n).read_bytes() for n in ('better-favorites','launch.sh','profile-device-launch.sh','profile.enabled')}
    prior_manifest=(app/'.profiling-backup/manifest.json').read_bytes()
    session_cmd=command('new-session',mounted)+['--session-id','corrected-fixture']
    subprocess.run(session_cmd,check=True,capture_output=True)
    assert (app/'.profiling-results/session-corrected-fixture').is_dir()
    assert (app/'profile.enabled').read_text()=='BetterFavoritesProfileSession2\ncorrected-fixture\n'
    assert all(p.read_bytes()==data for p,data in originals.items())
    assert (pilot/'startup.log').read_text()=='preserved pilot'
    assert subprocess.run(session_cmd,capture_output=True).returncode!=0
    subprocess.run(command('restore-session',mounted)+['--session-id','corrected-fixture'],check=True,capture_output=True)
    assert all((app/n).read_bytes()==data for n,data in prior.items())
    assert (app/'.profiling-backup/manifest.json').read_bytes()==prior_manifest
    assert (app/'.profiling-results/session-corrected-fixture').is_dir()
    subprocess.run(command('disable',mounted),check=True,capture_output=True)
    assert not (app/'profile.enabled').exists()
    foreign=b'foreign';(app/'profile-device-launch.sh').write_bytes(foreign)
    assert subprocess.run(command('rollback',mounted),capture_output=True).returncode!=0
    assert (app/'profile-device-launch.sh').read_bytes()==foreign
    (app/'profile-device-launch.sh').write_bytes((repo/'tools/profile-device-launch.sh').read_bytes())
    subprocess.run(command('rollback',mounted),check=True,capture_output=True)
    assert (app/'better-favorites').read_bytes()==old and all(p.read_bytes()==data for p,data in protected.items())
    subprocess.run(command('status',mounted,False),check=True,capture_output=True)
    # Fail one atomic publication; subsequent rollback must restore only own files.
    for operation in ('activate','rollback','new-session'):
        mounted,app=card(operation+'-failure')
        if operation in ('rollback','new-session'):subprocess.run(command('activate',mounted),check=True,capture_output=True)
        original_manifest=(app/'.profiling-backup/manifest.json').read_bytes() if operation!='activate' else None
        before={p.name:p.read_bytes() for p in app.iterdir() if p.is_file()}
        target=(app/'.profiling-backup/manifest.json') if operation=='new-session' else app/('launch.sh' if operation=='activate' else 'better-favorites')
        code='''import os,runpy,sys
original=os.replace
target=sys.argv[1]
failed=False
def replace(source,destination):
 global failed
 if str(destination)==target and not failed:
  failed=True;raise OSError('fixture publication failure')
 return original(source,destination)
os.replace=replace
sys.argv=sys.argv[2:]
runpy.run_path(sys.argv[0],run_name='__main__')
'''
        cmd=command(operation,mounted)
        if operation=='new-session':cmd+=['--session-id','failed-session']
        result=subprocess.run(['python3','-c',code,str(target),*cmd[1:]],capture_output=True)
        assert result.returncode!=0
        assert all((app/name).read_bytes()==data for name,data in before.items())
        if operation!='activate':assert (app/'.profiling-backup/manifest.json').read_bytes()==original_manifest
        if operation=='activate':assert not (app/'profile.enabled').exists() and not (app/'profile-device-launch.sh').exists()
print('Profiling activation: backup/hash guards, power-state guard, protected files, disable, conflict refusal and failed publication/rollback: PASS')
