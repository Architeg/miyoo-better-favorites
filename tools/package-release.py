#!/usr/bin/env python3
"""Package a clean pinned checkout; never reads device state or credentials.
Fresh output only. Public payload excludes vendor MainUI/runtime and private data.
"""
import argparse, hashlib, io, json, os, shutil, subprocess, tarfile, time, zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
VERSION='1.0.0-rc.2'
SDL_COMMIT='3c68ed01fee7feffd4ea338b1cc5018a455e2be9'
def sha(data):return hashlib.sha256(data).hexdigest()
def command(args,**kwargs):return subprocess.check_output(args,cwd=ROOT,**kwargs)
def write(root,name,data):
 p=root/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data)
def zipdir(root,dest,epoch):
 with zipfile.ZipFile(dest,'x',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
  for p in sorted(root.rglob('*')):
   if not p.is_file():continue
   name=p.relative_to(root).as_posix();entry=zipfile.ZipInfo(name,time.gmtime(max(epoch,315532800))[:6]);entry.compress_type=zipfile.ZIP_DEFLATED
   entry.external_attr=(0o100000|(0o755 if p.suffix in ('.sh','.command') or p.name.startswith('better-favorites-installer-') or p.name=='better-favorites' else 0o644))<<16
   z.writestr(entry,p.read_bytes())
def source_archive(repo, args, destination, epoch):
 import gzip
 proc=subprocess.Popen(['git','-C',str(repo),'archive','--format=tar']+args,stdout=subprocess.PIPE)
 try:
  with destination.open('xb') as raw:
   with gzip.GzipFile(filename='',mode='wb',fileobj=raw,compresslevel=9,mtime=epoch) as gz:shutil.copyfileobj(proc.stdout,gz,1024*1024)
  if proc.wait()!=0:raise RuntimeError('Source archive failed')
 finally:proc.stdout.close()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--legacy-toolchain',type=Path,required=True,help='Isolated official Go1.20.14 for Windows7/8/8.1 and read-only dispatch');p.add_argument('--review-snapshot',action='store_true',help='Explicit uncommitted private review snapshot; identity and matching source inventory included');a=p.parse_args()
 if not a.review_snapshot and command(['git','status','--porcelain','--untracked-files=normal']).strip():raise SystemExit('Commit reviewed source first; package requires a clean pinned checkout.')
 commit=command(['git','rev-parse','HEAD']).decode().strip();epoch=int(command(['git','show','-s','--format=%ct','HEAD']))
 if a.output.exists():raise SystemExit('Choose a fresh output directory; previous candidates are preserved.')
 a.output.mkdir(parents=True);out=a.output
 paths=command(['git','ls-files','-z']).decode().split('\0')
 if a.review_snapshot:
  new=command(['git','ls-files','--others','--exclude-standard','-z']).decode().split('\0')
  allowed_new={'tools/release-installer/uninstall.go','tools/release-installer/uninstall_test.go','tools/build-windows7-test.py','App/BetterFavoritesTest/icon.png','docs/release/rc.2.md','docs/release/rc1-mac-acceptance.md','tests/rc2_presentation_test.py','tests/host_dispatch_test.py','tools/host_packaging.py','tools/package-windows7-test.py','tools/host-dispatch/go.mod','tools/host-dispatch/dispatch.go','tools/host-dispatch/probe_windows.go','tools/host-dispatch/probe_other.go','tools/host-dispatch/dispatch_test.go','packaging/Install-Windows.cmd','packaging/Install-macOS.command','packaging/Install-Linux.sh','docs/release/host-dispatch.md','docs/release/windows-acceptance.md','docs/release/dependency-audit.md'}
  if any(n and n not in allowed_new for n in new):raise SystemExit('Unexpected untracked source; review explicitly before packaging: '+repr(new))
  paths+=new
 source_files={p:sha((ROOT/p).read_bytes()) for p in sorted(set(paths)) if p and (ROOT/p).is_file()}
 snapshot=sha((json.dumps(source_files,sort_keys=True)+'\n').encode())
 write(out,'SOURCE-INVENTORY.json',(json.dumps(dict(base_commit=commit,review_snapshot=a.review_snapshot,source_snapshot_sha256=snapshot,files=source_files),indent=2)+'\n').encode())
 sdl=ROOT/'third_party/sdl2_miyoo'
 if command(['git','-C',str(sdl),'rev-parse','HEAD']).decode().strip()!=SDL_COMMIT:raise SystemExit('SDL source pin mismatch')
 dirty=command(['git','-C',str(sdl),'diff','HEAD','--','sdl2','swiftshader','mini']).strip()
 if dirty:raise SystemExit('Tracked SDL source differs from pin')
 stage=out/'installer';stage.mkdir();base=out/'app-only';base.mkdir()
 inventory=[]
 def payload(name,source,mode=0o644):
  data=source.read_bytes();write(stage,'payload/'+name,data);inventory.append(dict(path=name,sha256=sha(data),mode=mode));return data
 for n in ('config.json','launch.sh','icon.png'):
  data=payload('App/BetterFavoritesTest/'+n,(ROOT/'icon.png' if n=='icon.png' else ROOT/'App/BetterFavoritesTest'/n),0o755 if n.endswith('.sh') else 0o644);write(base,'App/BetterFavoritesTest/'+n,data)
 binary=(ROOT/'build/better-favorites').read_bytes()
 if binary[:7]!=b'\x7fELF\x01\x01\x01' or binary[18:20]!=b'\x28\x00':raise SystemExit('App is not ARM ELF32')
 # Version/commit literals must be in the freshly compiled app.
 if commit.encode() not in binary or VERSION.encode() not in binary:raise SystemExit('Build app using scripts/build.sh at the pinned commit first')
 data=payload('App/BetterFavoritesTest/better-favorites',ROOT/'build/better-favorites',0o755);write(base,'App/BetterFavoritesTest/better-favorites',data)
 libs={ 'libSDL2-2.0.so.0':sdl/'custom/libSDL2-2.0.so.0',
   **{n:sdl/'examples'/n for n in ('libSDL2_image-2.0.so.0','libSDL2_mixer-2.0.so.0','libSDL2_ttf-2.0.so.0','libjson-c.so.5','libpng16.so.16','libz.so.1')},
   **{n:sdl/'prebuilt/mini'/n for n in ('libEGL.so','libGLESv2.so')} }
 expected={r["file"]:r["sha256"] for r in json.loads((ROOT/"docs/release/dependency-hashes.json").read_text())}
 dependencies=[]
 for n,source in libs.items():
  if sha(source.read_bytes())!=expected[n]:raise SystemExit('Working library hash mismatch: '+n)
  data=payload('App/BetterFavoritesTest/'+n,source,0o755);write(base,'App/BetterFavoritesTest/'+n,data);dependencies.append(dict(file=n,sha256=sha(data),size=len(data),source_pin=SDL_COMMIT))
 spec=json.loads((ROOT/'integration/mainui-home/package.json').read_text())
 adapter=ROOT/'build/rc-adapter/adapter.elf'
 if sha(adapter.read_bytes())!=spec['payload_sha256']:raise SystemExit('Adapter differs from accepted exact catalogue')
 for rel in ('integration/mainui-home/package.json','integration/onion-return/hashes.json','integration/onion-return/runtime.patch','integration/onion-return/better_favorites_return.sh'):payload(rel,ROOT/rel)
 payload('integration/mainui-home/adapter.elf',adapter)
 release=dict(version=VERSION,source_commit=commit,source_snapshot_sha256=snapshot,review_snapshot=a.review_snapshot,dependencies=dependencies,toolchain='aemiii91/miyoomini-toolchain@sha256:a864876472a489f63d6223d2c8ad61e12ced679c0b177ae9429e51f3673ef4e7',gates='See docs/release/rc.2.md; not stable/hardware-qualified candidate')
 data=(json.dumps(release,indent=2)+'\n').encode();write(base,'App/BetterFavoritesTest/release.json',data);write(stage,'payload/App/BetterFavoritesTest/release.json',data);inventory.append(dict(path='App/BetterFavoritesTest/release.json',sha256=sha(data),mode=0o644))
 write(stage,'package.json',(json.dumps(dict(format=1,version=VERSION,commit=commit,source_snapshot_sha256=snapshot,review_snapshot=a.review_snapshot,files=inventory),indent=2)+'\n').encode())
 from host_packaging import build_hosts
 build_hosts(stage,a.legacy_toolchain,out)
 for target in (stage,base):
  document_paths=['README.md','CONTRIBUTING.md','CHANGELOG.md','LICENSE','THIRD_PARTY_NOTICES.md','tools/bootstrap/README.md']
  document_paths += [str(p.relative_to(ROOT)) for p in sorted((ROOT/'docs').rglob('*.md'))]
  document_paths += ['docs/release/dependency-hashes.json','docs/release/upstream-sources.json']
  for rel in document_paths:
   data=(ROOT/rel).read_bytes()
   if rel=='README.md' and target==stage:
    data=data.replace(b'src="App/BetterFavoritesTest/icon.png"',b'src="payload/App/BetterFavoritesTest/icon.png"')
   if rel=='docs/roadmap.md':
    for source in ('integration/onion-return/hashes.json','integration/onion-return/manage.py'):
     data=data.replace(('](../'+source+')').encode(),('](https://github.com/Architeg/miyoo-better-favorites/blob/'+commit+'/'+source+')').encode())
   write(target,rel,data)
  write(target,'README.txt',b'Better Favorites 1.0.0-rc.2 - private review candidate, not stable.\nStart with docs/install.md. Uninstall restores integrations and removes owned app/data automatically; computer recovery is retained.\nDesigned for Mini and Mini Plus; hardware tested on Mini Plus.\nUser-confirmed Windows7 SP1 x64/Windows10 x64 and Mac tests are recorded; exact host/tool and remaining gates are in docs/release/rc.2.md.\n')
 # Supply pinned dependency source material, rather than promising a future URL.
 # No vendor MainUI, ROMs, private backups or development logs are in these trees.
 source_archive(sdl,['--prefix=sdl2-miyoo/','HEAD','LICENSE','Makefile','Makefile.mk','sdl2','swiftshader','mini'],out/('sdl2-miyoo-'+SDL_COMMIT+'.tar.gz'),epoch)
 if a.review_snapshot:
  import gzip
  with (out/('better-favorites-'+VERSION+'-source.tar.gz')).open('xb') as raw:
   with gzip.GzipFile(filename='',mode='wb',fileobj=raw,mtime=epoch) as gz:
    with tarfile.open(fileobj=gz,mode='w') as tar:
     for name in source_files:
      data=(ROOT/name).read_bytes()
      if sha(data)!=source_files[name]:raise RuntimeError('Source changed during snapshot: '+name)
      info=tarfile.TarInfo('better-favorites-'+VERSION+'/'+name);info.size=len(data);info.mode=0o755 if (ROOT/name).stat().st_mode&0o111 else 0o644;info.mtime=epoch;tar.addfile(info,io.BytesIO(data))
 else:
  source_archive(ROOT,['--prefix=better-favorites-'+VERSION+'/','HEAD'],out/('better-favorites-'+VERSION+'-source.tar.gz'),epoch)
 notices=out/'licenses';notices.mkdir()
 for rel in ('LICENSE','sdl2/LICENSE.txt','swiftshader/LICENSE.txt','swiftshader/AUTHORS.txt'):
  write(notices,rel.replace('/','-'),(sdl/rel).read_bytes())
 write(notices,'Go-LICENSE',(ROOT/'third_party/notices/Go-BSD.txt').read_bytes())
 for archive in sorted((sdl/'sdl2/dependency').glob('*.tar.gz')):
  if not any(archive.name.startswith(n) for n in ('SDL2_image-','SDL2_mixer-','SDL2_ttf-','json-c-')):continue
  with tarfile.open(archive) as tar:
   for member in tar.getmembers():
    if member.isfile() and Path(member.name).name in ('COPYING','LICENSE','LICENSE.txt'):write(notices,archive.stem+'-'+member.name.replace('/','-'),tar.extractfile(member).read())
 # Preserve pinned supplemental source and exact upstream notices; this does
 # not establish byte correspondence for all prebuilt artifacts.
 supplemental=json.loads((ROOT/'docs/release/upstream-sources.json').read_text())
 for record in supplemental:
  source=ROOT/'third_party/cache'/record['archive']
  notice=ROOT/'third_party/notices'/record['notice_file']
  if sha(source.read_bytes())!=record['sha256'] or sha(notice.read_bytes())!=record['notice_sha256']:
   raise RuntimeError('Supplemental upstream source/notice checksum mismatch')
  write(out,record['archive'],source.read_bytes())
  write(notices,record['notice_file'],notice.read_bytes())
 # Source/license companions must accompany either binary download. License
 # notices flag uncertain prebuilt correspondence; stable publication is gated.
 for target in (stage,base):
  shutil.copytree(notices,target/'licenses')
  write(target,'SOURCE.txt',('Matching project source: better-favorites-'+VERSION+'-source.tar.gz\nDependency source: sdl2-miyoo-'+SDL_COMMIT+'.tar.gz\nDistribute source/license companions with this private candidate. Prebuilt correspondence audit remains a gate.\n'+''.join('Supplemental upstream source: '+r['archive']+'\n' for r in supplemental)).encode())
 for target in (stage,base):
  write(target,'SOURCE-INVENTORY.json',(out/'SOURCE-INVENTORY.json').read_bytes())
  entries=[p for p in sorted(target.rglob('*')) if p.is_file()]
  write(target,'SHA256SUMS', ''.join(sha(p.read_bytes())+'  '+p.relative_to(target).as_posix()+'\n' for p in entries).encode())
 zipdir(base,out/('better-favorites-'+VERSION+'-app-only.zip'),epoch)
 zipdir(stage,out/('better-favorites-'+VERSION+'-installer.zip'),epoch)
 # Separate source/license archives alongside both binaries, checksums external.
 files=[p for p in out.iterdir() if p.is_file()];write(out,'SHA256SUMS', ''.join(sha(p.read_bytes())+'  '+p.name+'\n' for p in sorted(files)).encode())
 print(json.dumps(dict(output=str(out),commit=commit,app_sha256=sha(binary),archives=[p.name for p in files]),indent=2))
if __name__=='__main__':main()
