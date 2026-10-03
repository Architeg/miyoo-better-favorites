#!/usr/bin/env python3
"""Offline, reversible four-run profiling pilot. No change to Onion's app command."""
import argparse,hashlib,json,os,shutil,subprocess,tempfile
from pathlib import Path
REPO=Path(__file__).resolve().parents[1]
M4='aa486abad4e3272f86605945109bf0a0f76159ab373f87c1e57c38abd034b328'
MARKER=b'BetterFavoritesProfilePilot1\n'
def digest(data):return hashlib.sha256(data).hexdigest()
def regular(path):
    if path.is_symlink() or not path.is_file():raise RuntimeError(f'Not a regular file: {path}')
    return path.read_bytes()
def publish(path,data,mode=0o755):
    descriptor,name=tempfile.mkstemp(prefix='.bf-profile-',dir=path.parent)
    temporary=Path(name)
    try:
        with os.fdopen(descriptor,'wb') as output:output.write(data);output.flush();os.fsync(output.fileno())
        temporary.chmod(mode)
        if temporary.read_bytes()!=data:raise RuntimeError('Staged copy mismatch')
        os.replace(temporary,path)
        if regular(path)!=data:raise RuntimeError('Published copy mismatch')
    finally:temporary.unlink(missing_ok=True)
def diagnostic_launcher(original):
    text=original.decode()
    setup='''
# Opt-in four-run pilot. Keep runtime's exact Apps command and ownership context.
if [ -f "$APP_DIR/profile.enabled" ] && [ ! -L "$APP_DIR/profile.enabled" ]; then
    . "$APP_DIR/profile-device-launch.sh"
    bf_profile_begin || echo 'Profile setup failed; ordinary launch continues.' >> "$LOG"
fi
'''
    anchor='} > "$LOG"\n';assert text.count(anchor)==1
    text=text.replace(anchor,anchor+setup,1)
    anchor='cleanup() {\n';assert text.count(anchor)==1
    text=text.replace(anchor,anchor+'    cleanup_status=$?\n',1)
    anchor='\n}\n\ntrap cleanup EXIT';assert text.count(anchor)==1
    text=text.replace(anchor,'\n    if command -v bf_profile_finish >/dev/null 2>&1; then bf_profile_finish "$cleanup_status"; fi'+anchor,1)
    return text.encode()
def protected(card,app):
    names=('settings.conf','browser-state','browser-preferences.conf','config.json','icon.png')
    files=[app/name for name in names if (app/name).is_file()]
    files.extend(p for p in app.glob('*.so*') if p.is_file())
    for name in ('Roms/favourite.json','Roms/recentlist.json','Roms/recentlist-hidden.json','.tmp_update/runtime.sh','.tmp_update/script/better_favorites_return.sh'):
        path=card/name
        if path.is_file():files.append(path)
    # Include installed manifests and original runtime backup; never modify them.
    files.extend(p for p in (card/'.tmp_update/config/better-favorites-return-backup').rglob('*') if p.is_file())
    return {str(p):digest(p.read_bytes()) for p in files}
def verify_manifest(app,backup):
    manifest=json.loads(regular(backup/'manifest.json'))
    if manifest['version']!=1 or manifest['original']['better-favorites']!=M4:raise RuntimeError('Not the accepted M4 backup')
    for name,sha in manifest['original'].items():
        if digest(regular(backup/name))!=sha:raise RuntimeError('Original backup hash mismatch: '+name)
    if manifest['original']['better-favorites']!=M4:raise RuntimeError('Rollback is not accepted M4')
    for name,sha in manifest['installed'].items():
        if digest(regular(app/name))!=sha:raise RuntimeError('Installed file changed: '+name)
    return manifest
parser=argparse.ArgumentParser()
parser.add_argument('operation',choices=('prepare','activate','new-session','restore-session','disable','rollback','status'))
parser.add_argument('--session-id',help='Fresh alphanumeric/hyphen/underscore session ID')
parser.add_argument('--card',type=Path,default=Path('/Volumes/MIYOO'))
parser.add_argument('--binary',type=Path,default=REPO/'build/better-favorites')
parser.add_argument('--output',type=Path,help='New host directory for prepared launcher/helper')
parser.add_argument('--powered-off',action='store_true',help='Operator confirms powered-off device, mounted card')
args=parser.parse_args()
app=args.card/'App/BetterFavoritesTest';backup=app/'.profiling-backup'
original=(REPO/'App/BetterFavoritesTest/launch.sh').read_bytes()
launcher=diagnostic_launcher(original);hook=(REPO/'tools/profile-device-launch.sh').read_bytes()
if args.operation=='prepare':
    if args.output is None:parser.error('--output required for prepare')
    args.output.mkdir(parents=True,exist_ok=False)
    (args.output/'launch.sh').write_bytes(launcher);(args.output/'profile-device-launch.sh').write_bytes(hook)
    subprocess.run(['sh','-n',str(args.output/'launch.sh'),str(args.output/'profile-device-launch.sh')],check=True)
    print('Prepared diagnostic files:',args.output)
else:
    if not app.is_dir():raise RuntimeError('Mounted test app unavailable')
    if args.operation!='status' and not args.powered_off:parser.error('--powered-off required; do not operate on a running device')
    before=protected(args.card,app)
    if args.operation=='activate':
        if backup.exists():raise RuntimeError('Profiling backup already exists; inspect status or rollback')
        if (app/'.profiling-results').exists():raise RuntimeError('Existing profiling evidence retained; use a separately reviewed new pilot, never overwrite it')
        for name in ('profile-device-launch.sh','profile.enabled'):
            if (app/name).exists() or (app/name).is_symlink():raise RuntimeError('Refuse replacing unrelated file: '+name)
        old={name:regular(app/name) for name in ('better-favorites','launch.sh')}
        if digest(old['better-favorites'])!=M4 or old['launch.sh']!=original:raise RuntimeError('Expected accepted M4 binary/launcher not installed')
        binary=regular(args.binary)
        if binary[:6]!=b'\x7fELF\x01\x01' or binary[18:20]!=b'\x28\x00':raise RuntimeError('Diagnostic binary is not ARM32 LE')
        backup.mkdir()
        for name,data in old.items():
            (backup/name).write_bytes(data)
            if regular(backup/name)!=data:raise RuntimeError('Backup verification failed; replacements not started')
        installed={'better-favorites':binary,'launch.sh':launcher,'profile-device-launch.sh':hook,'profile.enabled':MARKER}
        manifest={'version':1,'original':{n:digest(d) for n,d in old.items()},'installed':{n:digest(d) for n,d in installed.items()}}
        (backup/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
        published=[]
        try:
            for name,data in installed.items():
                # Marker last: activation cannot precede complete publication.
                expected=old.get(name)
                if expected is not None and regular(app/name)!=expected:raise RuntimeError('Destination changed before publication')
                if expected is None and ((app/name).exists() or (app/name).is_symlink()):raise RuntimeError('Foreign file appeared')
                published.append(name);publish(app/name,data,0o600 if name=='profile.enabled' else 0o755)
            verify_manifest(app,backup)
            subprocess.run(['sh','-n',str(app/'launch.sh'),str(app/'profile-device-launch.sh')],check=True)
            if before!=protected(args.card,app):raise RuntimeError('Protected files changed')
        except BaseException:
            for name in reversed(published):
                if (app/name).is_file() and not (app/name).is_symlink() and digest((app/name).read_bytes())==manifest['installed'][name]:
                    if name in old:publish(app/name,old[name])
                    else:(app/name).unlink()
            subprocess.run(['sync'],check=True);raise
        subprocess.run(['sync'],check=True);print('Pilot active; verified M4 backup:',backup)
    elif args.operation in ('new-session','restore-session'):
        import re
        session=args.session_id
        if not session or not re.fullmatch(r'[A-Za-z0-9_-]{1,64}',session):
            raise RuntimeError('A safe fresh --session-id is required')
        # Verify current activation and accepted M4 originals before any replacement.
        manifest=json.loads(regular(backup/'manifest.json'))
        for name,sha in manifest['original'].items():
            if digest(regular(backup/name))!=sha:raise RuntimeError('M4 backup mismatch')
        if manifest['original']['better-favorites']!=M4 or manifest['original']['launch.sh']!=digest(original):
            raise RuntimeError('Unexpected M4 originals')
        for name,sha in manifest['installed'].items():
            if digest(regular(app/name))!=sha:raise RuntimeError('Installed file changed: '+name)
        if regular(app/'launch.sh')!=launcher:raise RuntimeError('Unexpected diagnostic launcher')
        session_backup=app/'.profiling-sessions'/session/'backup'
        result_root=app/'.profiling-results'/('session-'+session)
        targets={'profile-device-launch.sh':app/'profile-device-launch.sh',
                 'profile.enabled':app/'profile.enabled','manifest.json':backup/'manifest.json'}
        old={name:regular(path) for name,path in targets.items()}
        if args.operation=='new-session':
            if session_backup.parent.exists() or result_root.exists():raise RuntimeError('Session already exists; never reuse evidence')
            session_backup.mkdir(parents=True)
            # Preserve every current diagnostic file and its manifest, not just replacements.
            for name in ('better-favorites','launch.sh','profile-device-launch.sh','profile.enabled'):
                data=regular(app/name);(session_backup/name).write_bytes(data)
                if regular(session_backup/name)!=data:raise RuntimeError('Session backup mismatch')
            (session_backup/'manifest.json').write_bytes(old['manifest.json'])
            if regular(session_backup/'manifest.json')!=old['manifest.json']:raise RuntimeError('Manifest backup mismatch')
            result_root.mkdir(parents=True) # never removed, including failed activation
            marker=('BetterFavoritesProfileSession2\n'+session+'\n').encode()
            revised=json.loads(old['manifest.json'])
            revised['installed']['profile-device-launch.sh']=digest(hook)
            revised['installed']['profile.enabled']=digest(marker)
            revised['session_id']=session
            new={'profile-device-launch.sh':hook,'manifest.json':(json.dumps(revised,indent=2)+'\n').encode(),'profile.enabled':marker}
        else:
            if manifest.get('session_id')!=session:raise RuntimeError('Only current session can be restored')
            previous=json.loads(regular(session_backup/'manifest.json'))
            if previous['original']!=manifest['original']:raise RuntimeError('Session backup original mismatch')
            for name,sha in previous['installed'].items():
                if digest(regular(session_backup/name))!=sha:raise RuntimeError('Session backup hash mismatch: '+name)
                if name in ('better-favorites','launch.sh') and regular(app/name)!=regular(session_backup/name):
                    raise RuntimeError('Binary/launcher changed since session activation')
            new={name:regular(session_backup/name) for name in targets}
        published=[]
        try:
            for name,data in new.items():
                if regular(targets[name])!=old[name]:raise RuntimeError('Destination changed: '+name)
                published.append(name)
                publish(targets[name],data,0o600 if name in ('manifest.json','profile.enabled') else 0o755)
            verify_manifest(app,backup)
            subprocess.run(['sh','-n',str(app/'launch.sh'),str(app/'profile-device-launch.sh')],check=True)
            if before!=protected(args.card,app):raise RuntimeError('Protected data/integration changed')
        except BaseException:
            for name in reversed(published):
                if targets[name].is_file() and not targets[name].is_symlink() and regular(targets[name])==new[name]:
                    publish(targets[name],old[name],0o600 if name in ('manifest.json','profile.enabled') else 0o755)
            subprocess.run(['sync'],check=True);raise
        subprocess.run(['sync'],check=True)
        print(args.operation,'verified; previous activation backup:',session_backup,'evidence:',result_root)
    elif args.operation=='status':
        if not backup.exists():print('No diagnostic activation; accepted M4 rollback source hash:',M4)
        else:
            manifest=json.loads(regular(backup/'manifest.json'))
            if all(regular(app/name)==regular(backup/name) for name in ('better-favorites','launch.sh')) and not (app/'profile-device-launch.sh').exists():
                assert digest(regular(app/'better-favorites'))==M4
                print('Accepted M4 restored; profiling evidence/backup retained:',backup)
                raise SystemExit(0)
            for name in manifest['original']:
                assert digest(regular(backup/name))==manifest['original'][name]
            for name,sha in manifest['installed'].items():
                if name=='profile.enabled' and not (app/name).exists():print('Profiling disabled');continue
                assert digest(regular(app/name))==sha,name
            print('Verified diagnostic files and original backups:',backup)
    else:
        manifest=json.loads(regular(backup/'manifest.json'))
        if manifest['version']!=1 or manifest['original']['better-favorites']!=M4 or manifest['original']['launch.sh']!=digest(original):raise RuntimeError('Unexpected original checkpoint')
        # Permit our absent marker after disable; verify all other owned hashes.
        for name,sha in manifest['installed'].items():
            if name=='profile.enabled' and not (app/name).exists():continue
            if digest(regular(app/name))!=sha:raise RuntimeError('Refuse overwriting changed file: '+name)
        for name,sha in manifest['original'].items():
            if digest(regular(backup/name))!=sha:raise RuntimeError('Rollback backup mismatch: '+name)
        owned_before={name:regular(app/name) for name in manifest['installed'] if (app/name).exists()}
        changed={}
        try:
            if (app/'profile.enabled').exists():
                (app/'profile.enabled').unlink();changed['profile.enabled']=None
            if args.operation=='rollback':
                for name in ('launch.sh','better-favorites'):
                    data=regular(backup/name);changed[name]=data;publish(app/name,data)
                (app/'profile-device-launch.sh').unlink();changed['profile-device-launch.sh']=None
                assert regular(app/'better-favorites')==regular(backup/'better-favorites')
                assert regular(app/'launch.sh')==regular(backup/'launch.sh')
                subprocess.run(['sh','-n',str(app/'launch.sh')],check=True)
        except BaseException:
            for name,data in reversed(list(changed.items())):
                target=app/name
                if (data is None and not target.exists()) or (target.is_file() and not target.is_symlink() and target.read_bytes()==data):
                    publish(target,owned_before[name],0o600 if name=='profile.enabled' else 0o755)
            subprocess.run(['sync'],check=True);raise
        if before!=protected(args.card,app):raise RuntimeError('Protected files changed')
        subprocess.run(['sync'],check=True);print(args.operation,'verified; backups and evidence retained:',backup)
