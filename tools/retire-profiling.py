#!/usr/bin/env python3
"""Archive verified profiling-owned card files, restore production launch, keep binary."""
import argparse,hashlib,json,os,re,subprocess,tempfile
from pathlib import Path
REPO=Path(__file__).resolve().parents[1]
M4='aa486abad4e3272f86605945109bf0a0f76159ab373f87c1e57c38abd034b328'
def sha(data):return hashlib.sha256(data).hexdigest()
def read(path):
    if path.is_symlink() or not path.is_file():raise RuntimeError('Not a regular file: '+str(path))
    return path.read_bytes()
def retire(card,archive):
    if card.resolve()==archive.resolve() or card.resolve() in archive.resolve().parents:
        raise RuntimeError('Evidence archive must be on the host, outside the card')
    subprocess.run(['sh','-n',str(REPO/'App/BetterFavorites/launch.sh')],check=True)
    app=card/'App/BetterFavoritesTest';backup=app/'.profiling-backup'
    manifest=json.loads(read(backup/'manifest.json'))
    normal=read(REPO/'App/BetterFavorites/launch.sh')
    if manifest['version']!=1 or manifest['original']['better-favorites']!=M4 or manifest['original']['launch.sh']!=sha(normal):
        raise RuntimeError('Unexpected rollback checkpoint')
    for name,digest in manifest['original'].items():
        if sha(read(backup/name))!=digest:raise RuntimeError('Original backup mismatch: '+name)
    for name,digest in manifest['installed'].items():
        if name=='profile.enabled' and not (app/name).exists():continue
        if sha(read(app/name))!=digest:raise RuntimeError('Installed file changed: '+name)
    owned=set()
    for name in ('profile-device-launch.sh','profile.enabled','profile-memory-session.sh','sample-device-memory.sh','idle-20261003-01.log'):
        path=app/name
        if path.exists() or path.is_symlink():
            if name in ('profile-memory-session.sh','sample-device-memory.sh') and read(path)!=read(REPO/'tools'/name):
                raise RuntimeError('Foreign collector: '+name)
            owned.add(path)
    roots=['.profiling-backup','.profiling-sessions','.profiling-results','.profiling-memory']
    dirs=[]
    samples={'conditions.txt','kernel.txt','errors.txt'}
    detail={'process-stat-before.txt','process-stat-after.txt','uptime.txt','meminfo.txt','process-status.txt',
            'process-smaps.txt','memory-source.txt','memory-totals.txt','processes.txt','validity.txt','errors.txt'}
    sessions=set()
    sessionroot=app/'.profiling-sessions'
    if sessionroot.exists():
        for session in sessionroot.iterdir():
            if session.is_symlink() or not session.is_dir() or not re.fullmatch(r'[A-Za-z0-9_-]{1,64}',session.name):
                raise RuntimeError('Foreign session path')
            previous=json.loads(read(session/'backup/manifest.json'))
            if previous['original']!=manifest['original']:raise RuntimeError('Session originals mismatch')
            for name,digest in previous['installed'].items():
                if sha(read(session/'backup'/name))!=digest:raise RuntimeError('Session archive hash mismatch')
            sessions.add(session.name)
    for name in roots:
        root=app/name
        if not root.exists():continue
        if root.is_symlink() or not root.is_dir():raise RuntimeError('Foreign root: '+name)
        dirs.append(root)
        for path in root.rglob('*'):
            if path.is_symlink():raise RuntimeError('Foreign symlink: '+str(path))
            if path.is_dir():dirs.append(path);continue
            rel=path.relative_to(root).parts;allowed=False
            if name=='.profiling-backup':allowed=len(rel)==1 and rel[0] in {'manifest.json',*manifest['original']}
            elif name=='.profiling-sessions':
                allowed=len(rel)==3 and rel[0] in sessions and rel[1]=='backup' and rel[2] in {'manifest.json',*manifest['installed']}
            elif name=='.profiling-results':
                allowed=(len(rel)==2 and re.fullmatch(r'pilot-000[1-4]',rel[0]) is not None or
                         len(rel)==3 and rel[0].startswith('session-') and rel[0][8:] in sessions and re.fullmatch(r'run-000[1-4]',rel[1]) is not None)
                allowed=allowed and rel[-1] in {'metadata.txt','launcher-stat.txt','startup.log'}
            elif name=='.profiling-memory':
                # This retired pass contains exactly this reviewed finite idle trial.
                allowed=rel[0]=='idle-20261003-01-idle' and ((len(rel)==2 and (rel[1] in samples or re.fullmatch(r'inventory-[1-3]\.txt',rel[1]))) or
                    (len(rel)==3 and re.fullmatch(r'(browser|runtime)-[1-3]',rel[1]) and rel[2] in detail))
            if not allowed:raise RuntimeError('Unrecognized file retained; retirement refused: '+str(path))
            owned.add(path)
    # Preserve the unchanged cache binary, all unrelated app files and known Onion data.
    remaining=[p for p in app.iterdir() if p.is_file() and p not in owned and p.name!='launch.sh']
    for name in ('Roms/favourite.json','Roms/recentlist.json','Roms/recentlist-hidden.json','.tmp_update/runtime.sh',
                 '.tmp_update/script/better_favorites_return.sh','.tmp_update/config/active_theme'):
        p=card/name
        if p.is_file():remaining.append(p)
    remaining.extend(p for p in (card/'.tmp_update/config/better-favorites-return-backup').rglob('*') if p.is_file())
    protected={str(p):sha(read(p)) for p in remaining}
    sources=sorted(owned|{app/'launch.sh',app/'better-favorites'})
    originals={p:read(p) for p in sources}
    archive.mkdir(parents=True,exist_ok=False)
    entries={}
    for path,data in originals.items():
        destination=archive/path.relative_to(app);destination.parent.mkdir(parents=True,exist_ok=True)
        destination.write_bytes(data)
        if read(destination)!=data:raise RuntimeError('Archive verification failed; no card changes')
        entries[str(path.relative_to(app))]={'sha256':sha(data),'bytes':len(data)}
    (archive/'production-launch.sh').write_bytes(normal)
    if read(archive/'production-launch.sh')!=normal:raise RuntimeError('Production launcher archive mismatch')
    archive_manifest={'files':entries,'production_launcher_sha256':sha(normal),'cache_binary_sha256':sha(read(app/'better-favorites'))}
    (archive/'archive-manifest.json').write_text(json.dumps(archive_manifest,indent=2)+'\n')
    if json.loads(read(archive/'archive-manifest.json'))!=archive_manifest:raise RuntimeError('Archive manifest verification failed')
    for path,data in originals.items():
        if read(path)!=data:raise RuntimeError('Card changed during archive; no changes made')
    # Atomic same-filesystem launcher publication precedes temporary-file removal.
    fd,name=tempfile.mkstemp(prefix='.bf-retire-',dir=app)
    try:
        with os.fdopen(fd,'wb') as stream:stream.write(normal);stream.flush();os.fsync(stream.fileno())
        os.chmod(name,0o755)
        if read(Path(name))!=normal:raise RuntimeError('Staged launcher mismatch')
        os.replace(name,app/'launch.sh')
    finally:Path(name).unlink(missing_ok=True)
    if read(app/'launch.sh')!=normal:raise RuntimeError('Production launcher mismatch; verified archive retained')
    subprocess.run(['sh','-n',str(app/'launch.sh')],check=True)
    for path in sorted(owned):
        if read(path)!=originals[path]:raise RuntimeError('Foreign change retained: '+str(path))
        path.unlink() # Only a byte-verified, archived owned file.
    for path in sorted(dirs,key=lambda p:len(p.parts),reverse=True):path.rmdir() # Refuse nonempty/foreign content.
    if protected!={str(p):sha(read(p)) for p in remaining}:raise RuntimeError('Protected files changed')
    if not (app/'better-favorites').stat().st_mode&0o111 or not (app/'launch.sh').stat().st_mode&0o111:raise RuntimeError('Executable permission missing')
    if (app/'profile.enabled').exists():raise RuntimeError('Profiling activation remains')
    subprocess.run(['sync'],check=True)
    receipt={'archive':str(archive),'removed':[str(p.relative_to(app)) for p in sorted(owned)],
             'removed_files':len(owned),'removed_bytes':sum(len(originals[p]) for p in owned),
             'launcher_sha256':sha(normal),'binary_sha256':sha(read(app/'better-favorites')),
             'protected_hashes':protected,'syntax_permissions_sync':'passed'}
    (archive/'retirement-verification.json').write_text(json.dumps(receipt,indent=2)+'\n')
    return receipt
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--card',type=Path,default=Path('/Volumes/MIYOO'))
    parser.add_argument('--archive',type=Path,required=True)
    parser.add_argument('--powered-off',action='store_true')
    args=parser.parse_args()
    if not args.powered_off:parser.error('Operator-confirmed power off required')
    result=retire(args.card,args.archive)
    print(json.dumps({k:v for k,v in result.items() if k!='protected_hashes' and k!='removed'},indent=2))
