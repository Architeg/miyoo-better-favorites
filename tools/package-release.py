#!/usr/bin/env python3
"""Package a clean pinned checkout; never reads device state or credentials.
Fresh output only. Public payload excludes vendor MainUI/runtime and private data.
"""
import argparse, hashlib, io, json, os, shutil, subprocess, tarfile, time, zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
VERSION='1.0.0-rc.1'
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
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
 if command(['git','status','--porcelain','--untracked-files=normal']).strip():raise SystemExit('Commit reviewed source first; package requires a clean pinned checkout.')
 commit=command(['git','rev-parse','HEAD']).decode().strip();epoch=int(command(['git','show','-s','--format=%ct','HEAD']))
 if a.output.exists():raise SystemExit('Choose a fresh output directory; previous candidates are preserved.')
 a.output.mkdir(parents=True);out=a.output
 sdl=ROOT/'third_party/sdl2_miyoo'
 if command(['git','-C',str(sdl),'rev-parse','HEAD']).decode().strip()!=SDL_COMMIT:raise SystemExit('SDL source pin mismatch')
 dirty=command(['git','-C',str(sdl),'diff','HEAD','--','sdl2','swiftshader','mini']).strip()
 if dirty:raise SystemExit('Tracked SDL source differs from pin')
 stage=out/'installer';stage.mkdir();base=out/'app-only';base.mkdir()
 inventory=[]
 def payload(name,source,mode=0o644):
  data=source.read_bytes();write(stage,'payload/'+name,data);inventory.append(dict(path=name,sha256=sha(data),mode=mode));return data
 for n in ('config.json','launch.sh'):
  data=payload('App/BetterFavoritesTest/'+n,ROOT/'App/BetterFavoritesTest'/n,0o755 if n.endswith('.sh') else 0o644);write(base,'App/BetterFavoritesTest/'+n,data)
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
 release=dict(version=VERSION,source_commit=commit,dependencies=dependencies,toolchain='aemiii91/miyoomini-toolchain@sha256:a864876472a489f63d6223d2c8ad61e12ced679c0b177ae9429e51f3673ef4e7',gates='See RELEASE-GATES.md; not stable/hardware-qualified candidate')
 data=(json.dumps(release,indent=2)+'\n').encode();write(base,'App/BetterFavoritesTest/release.json',data);write(stage,'payload/App/BetterFavoritesTest/release.json',data);inventory.append(dict(path='App/BetterFavoritesTest/release.json',sha256=sha(data),mode=0o644))
 write(stage,'package.json',(json.dumps(dict(format=1,version=VERSION,commit=commit,files=inventory),indent=2)+'\n').encode())
 for goos,arch in (('windows','amd64'),('darwin','arm64'),('darwin','amd64'),('linux','amd64'),('linux','arm64')):
  name='better-favorites-installer-'+goos+'-'+arch+('.exe' if goos=='windows' else '')
  subprocess.run(['go','build','-trimpath','-buildvcs=false','-ldflags=-s -w','-o',str((stage/name).absolute()),'.'],cwd=ROOT/'tools/release-installer',env=dict(os.environ,GOOS=goos,GOARCH=arch,CGO_ENABLED='0'),check=True)
 write(stage,'Install-Windows.cmd',b'@echo off\r\ncd /d "%~dp0"\r\n"%~dp0better-favorites-installer-windows-amd64.exe" %*\r\npause\r\n')
 write(stage,'Install-macOS.command',b'#!/bin/sh\ncd "$(dirname "$0")" || exit 1\ncase "$(uname -m)" in arm64) arch=arm64;; *) arch=amd64;; esac\nexec "./better-favorites-installer-darwin-$arch" "$@"\n')
 write(stage,'Install-Linux.sh',b'#!/bin/sh\ncd "$(dirname "$0")" || exit 1\ncase "$(uname -m)" in aarch64) arch=arm64;; x86_64) arch=amd64;; *) echo "Unsupported host architecture" >&2; exit 1;; esac\nexec "./better-favorites-installer-linux-$arch" "$@"\n')
 for target in (stage,base):
  for rel in ('LICENSE','THIRD_PARTY_NOTICES.md','docs/install.md','docs/uninstall.md','docs/compatibility.md','docs/user-guide.md','docs/release/rc.1.md'):
   write(target,rel,(ROOT/rel).read_bytes())
  write(target,'README.txt',b'Better Favorites 1.0.0-rc.1 - release candidate, not stable.\nStart with docs/install.md. Restore optional integrations BEFORE deleting the app.\nDesigned for Mini and Mini Plus; hardware tested on Mini Plus.\nCandidate ZIP roundtrip and native Windows execution gates remain; see docs/release/rc.1.md.\n')
 # Supply pinned dependency source material, rather than promising a future URL.
 # No vendor MainUI, ROMs, private backups or development logs are in these trees.
 source=command(['git','-C',str(sdl),'archive','--format=tar','--prefix=sdl2-miyoo/','HEAD','LICENSE','Makefile','Makefile.mk','sdl2','swiftshader','mini'])
 import gzip
 write(out,'sdl2-miyoo-'+SDL_COMMIT+'.tar.gz',gzip.compress(source,compresslevel=9,mtime=epoch))
 own=command(['git','archive','--format=tar','--prefix=better-favorites-'+VERSION+'/','HEAD'])
 write(out,'better-favorites-'+VERSION+'-source.tar.gz',gzip.compress(own,compresslevel=9,mtime=epoch))
 notices=out/'licenses';notices.mkdir()
 for rel in ('LICENSE','sdl2/LICENSE.txt','swiftshader/LICENSE.txt','swiftshader/AUTHORS.txt'):
  write(notices,rel.replace('/','-'),(sdl/rel).read_bytes())
 go=Path(command(['go','env','GOROOT']).decode().strip());write(notices,'Go-LICENSE',(go/'LICENSE').read_bytes())
 for archive in sorted((sdl/'sdl2/dependency').glob('*.tar.gz')):
  if not any(archive.name.startswith(n) for n in ('SDL2_image-','SDL2_mixer-','SDL2_ttf-','json-c-')):continue
  with tarfile.open(archive) as tar:
   for member in tar.getmembers():
    if member.isfile() and Path(member.name).name in ('COPYING','LICENSE','LICENSE.txt'):write(notices,archive.stem+'-'+member.name.replace('/','-'),tar.extractfile(member).read())
 # Source/license companions must accompany either binary download. License
 # notices flag uncertain prebuilt correspondence; stable publication is gated.
 for target in (stage,base):
  shutil.copytree(notices,target/'licenses')
  write(target,'SOURCE.txt',('Matching project source: better-favorites-'+VERSION+'-source.tar.gz\nDependency source: sdl2-miyoo-'+SDL_COMMIT+'.tar.gz\nDistribute source/license companions with this private candidate. Prebuilt correspondence audit remains a gate.\n').encode())
 zipdir(base,out/('better-favorites-'+VERSION+'-app-only.zip'),epoch)
 zipdir(stage,out/('better-favorites-'+VERSION+'-installer.zip'),epoch)
 # Separate source/license archives alongside both binaries, checksums external.
 files=[p for p in out.iterdir() if p.is_file()];write(out,'SHA256SUMS', ''.join(sha(p.read_bytes())+'  '+p.name+'\n' for p in sorted(files)).encode())
 print(json.dumps(dict(output=str(out),commit=commit,app_sha256=sha(binary),archives=[p.name for p in files]),indent=2))
if __name__=='__main__':main()
