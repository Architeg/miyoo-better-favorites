#!/usr/bin/env python3
"""Package a clean pinned checkout; never reads device state or credentials.
Fresh output only. Public payload excludes vendor MainUI/runtime and private data.
"""
import argparse, hashlib, io, json, os, shutil, subprocess, tarfile, time, zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
VERSION='1.0.0-rc.7'
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
   entry.external_attr=(0o100000|(0o755 if p.suffix in ('.sh','.command') or p.name.startswith('better-favorites-installer-') or p.suffix=='.desktop' or p.name=='better-favorites' else 0o644))<<16
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
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--legacy-toolchain',type=Path,required=True,help='Isolated official Go1.20.14 for Windows7/8/8.1 native installers');p.add_argument('--review-snapshot',action='store_true',help='Explicit uncommitted private review snapshot; identity and matching source inventory included');p.add_argument('--reuse-rc5-app',action='store_true',help='Retain the exact RC5 ARM bytes for this host-only fix');a=p.parse_args()
 if not a.review_snapshot and command(['git','status','--porcelain','--untracked-files=normal']).strip():raise SystemExit('Commit reviewed source first; package requires a clean pinned checkout.')
 commit=command(['git','rev-parse','HEAD']).decode().strip();epoch=int(command(['git','show','-s','--format=%ct','HEAD']))
 if a.output.exists():raise SystemExit('Choose a fresh output directory; previous candidates are preserved.')
 a.output.mkdir(parents=True);out=a.output
 paths=command(['git','ls-files','-z']).decode().split('\0')
 if a.review_snapshot:
  new=command(['git','ls-files','--others','--exclude-standard','-z']).decode().split('\0')
  allowed_new={'tools/release-installer/receipt_repair.go','tools/release-installer/metadata_cleanup.go','tools/release-installer/recovery_sequence_test.go','packaging/windows-select.ps1','tools/generate-windows-entry.py','tests/windows_entry_test.ps1','tools/release-installer/lifecycle_metadata_test.go','docs/release/rc6-followup.md','tools/compare-installer-entry.py','tools/release-installer/presentation.go','tools/release-installer/presentation_test.go','tools/release-installer/installer_logs.go','tools/release-installer/installer_logs_test.go','tools/release-installer/transport_retry_test.go','tests/rc6_presentation_test.py','packaging/Mac-first-open.html','docs/release/rc.7.md','tools/release-installer/operation_report.go','tools/release-installer/operation_report_test.go','tools/release-installer/testdata/macos-directory-appledouble.bin','tools/release-installer/card_launch.go','tools/release-installer/card_launch_test.go','tests/click_package_test.py','docs/recovery.md','docs/release/rc.7.md','docs/release/notes-rc.3.md','docs/release/dependency-components.json','packaging/Install-Linux.desktop','third_party/notices/SDL_image-2.0.5-COPYING.txt','third_party/notices/SDL_mixer-2.0.4-COPYING.txt','third_party/notices/SDL_ttf-2.0.15-COPYING.txt','tools/release-installer/uninstall.go','tools/release-installer/uninstall_test.go','tools/build-windows7-test.py','App/BetterFavorites/icon.png','docs/release/rc.2.md','docs/release/rc1-mac-acceptance.md','tests/rc2_presentation_test.py','tests/host_dispatch_test.py','tools/host_packaging.py','tools/package-windows7-test.py','tools/host-dispatch/go.mod','tools/host-dispatch/dispatch.go','tools/host-dispatch/probe_windows.go','tools/host-dispatch/probe_other.go','tools/host-dispatch/dispatch_test.go','packaging/Install-Windows.cmd','packaging/Install-macOS.command','packaging/Install-Linux.sh','docs/release/host-dispatch.md','docs/release/windows-acceptance.md','docs/release/dependency-audit.md'}
  if any(n and n not in allowed_new for n in new):raise SystemExit('Unexpected untracked source; review explicitly before packaging: '+repr(new))
  paths+=new
 source_files={p:sha((ROOT/p).read_bytes()) for p in sorted(set(paths)) if p and (ROOT/p).is_file()}
 snapshot=sha((json.dumps(source_files,sort_keys=True)+'\n').encode())
 write(out,'SOURCE-INVENTORY.json',(json.dumps(dict(base_commit=commit,review_snapshot=a.review_snapshot,source_snapshot_sha256=snapshot,files=source_files),indent=2)+'\n').encode())
 sdl=ROOT/'third_party/sdl2_miyoo'
 if command(['git','-C',str(sdl),'rev-parse','HEAD']).decode().strip()!=SDL_COMMIT:raise SystemExit('SDL source pin mismatch')
 dirty=command(['git','-C',str(sdl),'diff','HEAD','--','sdl2','swiftshader']).strip()
 if dirty:raise SystemExit('Tracked SDL source differs from pin')
 stage=out/'installer';stage.mkdir();base=out/'app-payload';base.mkdir()
 inventory=[]
 def payload(name,source,mode=0o644):
  data=source.read_bytes();write(stage,'payload/'+name,data);inventory.append(dict(path=name,sha256=sha(data),mode=mode));return data
 for n in ('config.json','launch.sh','icon.png'):
  data=payload('App/BetterFavorites/'+n,(ROOT/'icon.png' if n=='icon.png' else ROOT/'App/BetterFavorites'/n),0o755 if n.endswith('.sh') else 0o644);write(base,'App/BetterFavorites/'+n,data)
 binary=(ROOT/'build/better-favorites').read_bytes()
 if binary[:7]!=b'\x7fELF\x01\x01\x01' or binary[18:20]!=b'\x28\x00':raise SystemExit('App is not ARM ELF32')
 # Host-only RC6 retains the exact RC5 app, with explicit independent provenance.
 app_commit,app_version=commit,VERSION
 if a.reuse_rc5_app:
  app_commit,app_version='7d1a7452e24627f8d6569e16c97700d144961aec','1.0.0-rc.5'
  if sha(binary)!='1af32c2503681aa65d1302c1c21b587d91f5bb14f23f0697fb1a6f4cf9fe14ad':raise SystemExit('RC5 app bytes differ; refuse reuse')
  for name in source_files:
   if name.startswith(('src/','include/','App/','integration/')) and (ROOT/name).read_bytes()!=command(['git','show',app_commit+':'+name]):raise SystemExit('App/integration source changed: '+name)
 if app_commit.encode() not in binary or app_version.encode() not in binary:raise SystemExit('Build app using scripts/build.sh at the pinned commit first')
 data=payload('App/BetterFavorites/better-favorites',ROOT/'build/better-favorites',0o755);write(base,'App/BetterFavorites/better-favorites',data)
 libs={ 'libSDL2-2.0.so.0':sdl/'custom/libSDL2-2.0.so.0',
   **{n:sdl/'examples'/n for n in ('libSDL2_image-2.0.so.0','libSDL2_mixer-2.0.so.0','libSDL2_ttf-2.0.so.0','libjson-c.so.5','libpng16.so.16','libz.so.1')},
   **{n:sdl/'prebuilt/mini'/n for n in ('libEGL.so','libGLESv2.so')} }
 expected={r["file"]:r["sha256"] for r in json.loads((ROOT/"docs/release/dependency-hashes.json").read_text())}
 dependencies=[]
 for n,source in libs.items():
  if sha(source.read_bytes())!=expected[n]:raise SystemExit('Working library hash mismatch: '+n)
  data=payload('App/BetterFavorites/'+n,source,0o755);write(base,'App/BetterFavorites/'+n,data);dependencies.append(dict(file=n,sha256=sha(data),size=len(data),source_pin=SDL_COMMIT))
 spec=json.loads((ROOT/'integration/mainui-home/package.json').read_text())
 adapter=ROOT/'build/rc-adapter/adapter.elf'
 if sha(adapter.read_bytes())!=spec['payload_sha256']:raise SystemExit('Adapter differs from accepted exact catalogue')
 for rel in ('integration/legacy/rc3-copied-package.json','integration/mainui-home/package.json','integration/mainui-home/legacy-package.json','integration/onion-return/legacy-hashes.json','integration/onion-return/hashes.json','integration/onion-return/runtime.patch','integration/onion-return/better_favorites_return.sh'):payload(rel,ROOT/rel)
 payload('integration/mainui-home/adapter.elf',adapter)
 release=dict(version=VERSION,app_version=app_version,app_source_commit=app_commit,app_sha256=sha(binary),source_commit=commit,source_snapshot_sha256=snapshot,review_snapshot=a.review_snapshot,dependencies=dependencies,toolchain='aemiii91/miyoomini-toolchain@sha256:a864876472a489f63d6223d2c8ad61e12ced679c0b177ae9429e51f3673ef4e7',gates='See docs/release/rc.7.md; not stable/hardware-qualified candidate')
 data=(json.dumps(release,indent=2)+'\n').encode();write(base,'App/BetterFavorites/release.json',data);write(stage,'payload/App/BetterFavorites/release.json',data);inventory.append(dict(path='App/BetterFavorites/release.json',sha256=sha(data),mode=0o644))
 write(stage,'package.json',(json.dumps(dict(format=1,version=VERSION,commit=commit,source_snapshot_sha256=snapshot,review_snapshot=a.review_snapshot,files=inventory),indent=2)+'\n').encode())
 from host_packaging import build_hosts
 build_hosts(stage,a.legacy_toolchain,out)
 for target in (stage,base):
  write(target,'Mac-first-open.html',(ROOT/'packaging/Mac-first-open.html').read_bytes())
  document_paths=['README.md','CONTRIBUTING.md','CHANGELOG.md','LICENSE','THIRD_PARTY_NOTICES.md','tools/bootstrap/README.md','packaging/Mac-first-open.html']
  document_paths += [str(p.relative_to(ROOT)) for p in sorted((ROOT/'docs').rglob('*.md'))]
  document_paths += ['docs/release/dependency-hashes.json','docs/release/dependency-components.json','docs/release/upstream-sources.json']
  for rel in document_paths:
   data=(ROOT/rel).read_bytes()
   if rel=='README.md' and target==stage:
    data=data.replace(b'src="App/BetterFavorites/icon.png"',b'src="payload/App/BetterFavorites/icon.png"')
   if rel=='docs/roadmap.md':
    for source in ('integration/onion-return/hashes.json','integration/onion-return/manage.py'):
     data=data.replace(('](../'+source+')').encode(),('](https://github.com/Architeg/miyoo-better-favorites/blob/'+commit+'/'+source+')').encode())
   write(target,rel,data)
  write(target,'README.txt',b'Better Favorites 1.0.0-rc.7 - private review candidate, not stable.\nStart with docs/install.md. Uninstall restores integrations and removes owned app/data automatically; portable card recovery is primary; computer archive is retained after uninstall.\nDesigned for Mini and Mini Plus; hardware tested on Mini Plus.\nUser-confirmed Windows7 SP1 x64/Windows10 x64 and Mac tests are recorded; exact host/tool and remaining gates are in docs/release/rc.7.md.\n')
 # Supply pinned dependency source material, rather than promising a future URL.
 # No vendor MainUI, ROMs, private backups or development logs are in these trees.
 source_archive(sdl,['--prefix=sdl2-miyoo/','HEAD','LICENSE','Makefile','Makefile.mk','sdl2','swiftshader'],out/('sdl2-miyoo-'+SDL_COMMIT+'.tar.gz'),epoch)
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
 for item in json.loads((ROOT/'third_party/notices/SwiftShader-supplemental.json').read_text()):
  data=(ROOT/'third_party/notices'/item['file']).read_bytes()
  if sha(data)!=item['sha256']:raise RuntimeError('SwiftShader supplemental notice mismatch')
  write(notices,item['file'],data)

 for archive in sorted((sdl/'sdl2/dependency').glob('*.tar.gz')):
  if not any(archive.name.startswith(n) for n in ('SDL2_image-','SDL2_mixer-','SDL2_ttf-','json-c-')):continue
  with tarfile.open(archive) as tar:
   for member in tar.getmembers():
    if member.isfile() and Path(member.name).name.upper().startswith(('COPYING','LICENSE','NOTICE','COPYRIGHT')):write(notices,archive.stem+'-'+member.name.replace('/','-'),tar.extractfile(member).read())
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
  if record['component'].startswith('SDL_'):
   with tarfile.open(source) as tar:
    for member in tar.getmembers():
     if member.isfile() and (Path(member.name).name.upper().startswith(('COPYING','LICENSE','NOTICE','COPYRIGHT')) or Path(member.name).name in ('miniz.h','nanosvg.h','nanosvgrast.h')):
      write(notices,record['component']+'-'+record['version']+'/'+member.name.split('/',1)[1],tar.extractfile(member).read())
 for notice in sorted((sdl/'swiftshader').rglob('*')):
  if notice.is_file() and notice.name.upper().startswith(('COPYING','LICENSE','NOTICE','COPYRIGHT')):
   write(notices,'swiftshader-components/'+notice.relative_to(sdl/'swiftshader').as_posix(),notice.read_bytes())
 # Source/license companions must accompany either binary download. License
 # notices flag uncertain prebuilt correspondence; stable publication is gated.
 for target in (stage,base):
  shutil.copytree(notices,target/'licenses')
  write(target,'SOURCE.txt',('Matching project source: better-favorites-'+VERSION+'-source.tar.gz\nDependency source: sdl2-miyoo-'+SDL_COMMIT+'.tar.gz\nDistribute source/license companions with this private candidate. See docs/release/dependency-audit.md for component-specific evidence/remaining attribution questions. SDK headers/driver libraries are excluded from the dependency source companion.\n'+''.join('Supplemental upstream source: '+r['archive']+'\n' for r in supplemental)).encode())
 for target in (stage,base):
  write(target,'SOURCE-INVENTORY.json',(out/'SOURCE-INVENTORY.json').read_bytes())
  entries=[p for p in sorted(target.rglob('*')) if p.is_file()]
  write(target,'SHA256SUMS', ''.join(sha(p.read_bytes())+'  '+p.relative_to(target).as_posix()+'\n' for p in entries).encode())
 # Full copy-to-card package: computer code stays dormant on-device. The
 # normal launcher derives the card, then stages this verified transport.
 full=out/'full';full.mkdir();shutil.copytree(base/'App',full/'App')
 appdir=full/'App/BetterFavorites';computer=appdir/'computer'
 shutil.copytree(stage,computer)
 for name in ('Install-Windows.cmd','Install-macOS.command','Install-Linux.sh','Install-Linux.desktop','Mac-first-open.html'):
  shutil.copy2(ROOT/'packaging'/name,appdir/name)
  if name.endswith(('.command','.sh','.desktop')):(appdir/name).chmod(0o755)
 transport=[]
 for file in sorted(computer.rglob('*')):
  if file.is_file():transport.append(dict(path='computer/'+file.relative_to(computer).as_posix(),sha256=sha(file.read_bytes()),mode=0o755 if file.stat().st_mode&0o111 else 0o644))
 for name in ('Install-Windows.cmd','Install-macOS.command','Install-Linux.sh','Install-Linux.desktop','Mac-first-open.html'):
  file=appdir/name;transport.append(dict(path=name,sha256=sha(file.read_bytes()),mode=0o755 if file.stat().st_mode&0o111 else 0o644))
 write(computer,'transport.json',(json.dumps(dict(format=1,files=transport),indent=2)+'\n').encode())
 for file in sorted(base.rglob('*')):
  if file.is_file() and not str(file.relative_to(base)).startswith('App/') and file.name!='SHA256SUMS':write(full,file.relative_to(base),file.read_bytes())
 # Metadata supports source/checksum consumers; the executable package lives
 # in App/BetterFavorites/computer, and root Source archives are not apps.
 write(full,'package.json',(stage/'package.json').read_bytes())
 write(full,'SHA256SUMS',''.join(sha(file.read_bytes())+'  '+file.relative_to(full).as_posix()+'\n' for file in sorted(full.rglob('*')) if file.is_file()).encode())
 zipdir(full,out/('better-favorites-'+VERSION+'.zip'),epoch)
 # Separate source/license archives alongside both binaries, checksums external.
 files=[p for p in out.iterdir() if p.is_file()];write(out,'SHA256SUMS', ''.join(sha(p.read_bytes())+'  '+p.name+'\n' for p in sorted(files)).encode())
 print(json.dumps(dict(output=str(out),commit=commit,app_sha256=sha(binary),archives=[p.name for p in files]),indent=2))
if __name__=='__main__':main()
