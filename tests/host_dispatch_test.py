#!/usr/bin/env python3
"""Execute Unix wrappers with mocked OS probes; not other-OS acceptance."""
import argparse, hashlib, json, os, shutil, struct, subprocess, sys, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def check_package(root):
 sys.path.insert(0,str(ROOT/'tools'))
 from host_packaging import inspect_linux,inspect_mac
 report=json.loads((root/'HOST-BUILDS.json').read_text())
 assert len(report['files'])==10
 expected={'windows7':{'386','amd64'},'windows':{'386','amd64','arm64'},'darwin':{'amd64','arm64'},'linux':{'amd64','arm64'}}
 seen={k:set() for k in expected}
 for item in report['files']:
  path=root/item['file'];data=path.read_bytes();assert hashlib.sha256(data).hexdigest()==item['sha256']
  if item.get('role'):assert item['toolchain']=='go1.20.14';continue
  group='windows7' if 'windows7-' in path.name else item['os'];seen[group].add(item['arch'])
  if item['os']=='linux':
   assert inspect_linux(path)['dynamic_libraries']==[]
   assert struct.unpack_from('<H',data,18)[0]==(62 if item['arch']=='amd64' else 183)
   # Focused rejection: convert one program header into PT_INTERP.
   bad=bytearray(data);offset=struct.unpack_from('<Q',bad,32)[0];struct.pack_into('<I',bad,offset,3)
   with tempfile.TemporaryDirectory() as tmp:
    fixture=Path(tmp)/'dynamic';fixture.write_bytes(bad)
    try:inspect_linux(fixture)
    except RuntimeError:pass
    else:raise AssertionError('Accepted dynamically loaded Linux executable')
  elif item['os']=='darwin':
   assert inspect_mac(path)['minimum_macos']=='12.0.0'
   assert struct.unpack_from('<I',data,4)[0]==(0x1000007 if item['arch']=='amd64' else 0x100000c)
  else:
   offset=struct.unpack_from('<I',data,60)[0];assert data[offset:offset+4]==b'PE\0\0'
   assert struct.unpack_from('<H',data,offset+4)[0]=={'386':0x14c,'amd64':0x8664,'arm64':0xaa64}[item['arch']]
 assert seen==expected
 print('Packaged 10 host executables: hashes, architectures, Monterey load minima and static Linux linkage PASS')

def main():
 parser=argparse.ArgumentParser();parser.add_argument('--package',type=Path);a=parser.parse_args()
 if a.package:check_package(a.package)
 with tempfile.TemporaryDirectory(prefix='bf-dispatch-tests-') as tmp:
  t=Path(tmp); mock=t/'probes';mock.mkdir(); package=t/'package';package.mkdir()
  probe="""#!/bin/sh
case "$1" in
-s) echo "$BF_SYSTEM";; -m) echo "$BF_MACHINE";; -r) echo "$BF_KERNEL";;
-productVersion) echo "$BF_VERSION";;
-n) case "$2" in hw.optional.arm64) [ "$BF_ARM" != absent ] || exit 1; echo "$BF_ARM";; sysctl.proc_translated) [ "$BF_TRANSLATED" != absent ] || exit 1; echo "$BF_TRANSLATED";; hw.cputype) echo "$BF_CPUTYPE";; *) exit 1;; esac;;
*) exit 1;; esac
"""
  for n in ('uname','sysctl','sw_vers'):(mock/n).write_text(probe);(mock/n).chmod(0o755)
  for host,name in [('darwin','Install-macOS.command'),('linux','Install-Linux.sh')]:
   script=(ROOT/'packaging'/name).read_text()
   # Only the test copy substitutes absolute system probe paths.
   for original in ('/usr/bin/uname','/usr/bin/sw_vers','/usr/sbin/sysctl'):script=script.replace(original,str(mock/Path(original).name))
   (package/name).write_text(script);(package/name).chmod(0o755)
   for arch in ('amd64','arm64'):
    p=package/('better-favorites-installer-'+host+'-'+arch)
    p.write_text('#!/bin/sh\nprintf "%s\\n" "$0" "$@" > "$BF_RESULT"\n[ "$BF_CHILD_STATUS" != 1 ] || echo "helper reason: unknown/modified app input preserved: App/BetterFavorites/._Install-Linux.desktop" >&2\nexit "$BF_CHILD_STATUS"\n');p.chmod(0o755)
  defaults=dict(BF_SYSTEM='Darwin',BF_MACHINE='arm64',BF_VERSION='12.7.6',BF_ARM='1',BF_TRANSLATED='0',BF_CPUTYPE='16777228',BF_KERNEL='6.1.0',BF_CHILD_STATUS='0',BF_RESULT=str(t/'result'))
  cases=[
   ('Install-macOS.command',{},'darwin-arm64'),
   ('Install-macOS.command',dict(BF_MACHINE='x86_64',BF_TRANSLATED='1'),'darwin-arm64'),
   ('Install-macOS.command',dict(BF_MACHINE='x86_64',BF_ARM='0',BF_TRANSLATED='absent'),'darwin-amd64'),
   ('Install-macOS.command',dict(BF_MACHINE='x86_64',BF_ARM='absent',BF_TRANSLATED='absent',BF_CPUTYPE='16777223'),'darwin-amd64'),
   ('Install-macOS.command',dict(BF_MACHINE='x86_64',BF_TRANSLATED='1',BF_ARM='0'),'darwin-arm64'),
   ('Install-macOS.command',dict(BF_MACHINE='x86_64',BF_TRANSLATED='1',BF_ARM='absent'),'darwin-arm64'),
   ('Install-macOS.command',dict(BF_VERSION='26.0'),'darwin-arm64'),
   ('Install-macOS.command',dict(BF_VERSION='11.7.10'),None),
   ('Install-macOS.command',dict(BF_VERSION='unknown'),None),
   ('Install-macOS.command',dict(BF_ARM='absent'),None),
   ('Install-macOS.command',dict(BF_MACHINE='x86_64',BF_ARM='1',BF_TRANSLATED='absent'),None),
   ('Install-macOS.command',dict(BF_MACHINE='ppc'),None),
   ('Install-macOS.command',dict(BF_SYSTEM='Linux'),None),
   ('Install-Linux.sh',dict(BF_SYSTEM='Linux',BF_MACHINE='x86_64',BF_KERNEL='3.2.0'),'linux-amd64'),
   ('Install-Linux.sh',dict(BF_SYSTEM='Linux',BF_MACHINE='aarch64',BF_KERNEL='3.7.0'),'linux-arm64'),
   ('Install-Linux.sh',dict(BF_SYSTEM='Linux',BF_MACHINE='arm64',BF_KERNEL='6.12.9-generic'),'linux-arm64'),
   ('Install-Linux.sh',dict(BF_SYSTEM='Linux',BF_MACHINE='x86_64',BF_KERNEL='6.1.0+deb12'),'linux-amd64'),
   ('Install-Linux.sh',dict(BF_SYSTEM='Linux',BF_MACHINE='aarch64',BF_KERNEL='3.2.0'),None),
   ('Install-Linux.sh',dict(BF_SYSTEM='Linux',BF_MACHINE='x86_64',BF_KERNEL='2.6.32'),None),
   ('Install-Linux.sh',dict(BF_SYSTEM='Linux',BF_MACHINE='i686'),None),
   ('Install-Linux.sh',dict(BF_SYSTEM='Linux',BF_MACHINE='x86_64',BF_KERNEL='unknown'),None),
   ('Install-Linux.sh',dict(BF_SYSTEM='Linux',BF_MACHINE='x86_64',BF_KERNEL='3'),None),
   ('Install-Linux.sh',{},None),
  ]
  for script,over,expected in cases:
   result=t/'result'
   if result.exists():result.unlink()
   e={**os.environ,**defaults,**over};e['PATH']=str(mock)+os.pathsep+os.environ['PATH']
   p=subprocess.run([str(package/script),'install','--sd-root','SD path with spaces'],env=e,capture_output=True,text=True,timeout=10)
   assert (p.returncode==0)==(expected is not None),(script,over,p.stderr)
   if expected is None:assert not result.exists(),over
   else:
    lines=result.read_text().splitlines();assert lines[0].endswith(expected);assert lines[1:]==['install','--sd-root','SD path with spaces']
  for name in ('Install-macOS.command','Install-Linux.sh'):
   e={**os.environ,**defaults,'BF_CHILD_STATUS':'17'};e['PATH']=str(mock)+os.pathsep+os.environ['PATH']
   if name.endswith('.sh'):e.update(BF_SYSTEM='Linux',BF_MACHINE='x86_64')
   p=subprocess.run([str(package/name),'uninstall'],env=e,capture_output=True,timeout=10);assert p.returncode==17
  # Copied-card wrapper branch: staged mock verifies args/status and cleanup.
  copied=t/'card 日本語'/'App'/'BetterFavorites';copied.mkdir(parents=True);(copied/'computer').mkdir();hosttmp=t/'hosttmp';hosttmp.mkdir()
  for name,host in [('Install-macOS.command','darwin'),('Install-Linux.sh','linux')]:
   shutil.copy2(package/name,copied/name)
   for arch in ('amd64','arm64'):
    dest=copied/'computer'/('better-favorites-installer-'+host+'-'+arch);shutil.copy2(package/dest.name,dest);dest.chmod(0o644)
   e={**os.environ,**defaults,'BF_CHILD_STATUS':'17','TMPDIR':str(hosttmp),'HOME':str(t/'home')};Path(e['HOME']).mkdir(exist_ok=True);e['PATH']=str(mock)+os.pathsep+os.environ['PATH']
   if host=='linux':e.update(BF_SYSTEM='Linux',BF_MACHINE='x86_64')
   child=subprocess.run([str(copied/name),'uninstall'],env=e,capture_output=True,timeout=10);assert child.returncode==17,(name,child.stderr)
   assert (t/'result').read_text().splitlines()[1:]==['--card-launcher',str(copied),'uninstall']
   assert not list(hosttmp.iterdir()),'bootstrap stage remains'
   if host=='darwin':
    approved=list((Path(e['HOME'])/'Library/Caches/BetterFavorites').glob('*/BetterFavorites-Installer'))
    assert len(approved)==1 and approved[0].read_bytes()==(copied/'computer/better-favorites-installer-darwin-arm64').read_bytes()
    identity=approved[0].stat().st_ino
    retry=subprocess.run([str(copied/name),'install'],env=e,input='r\nx\n',text=True,capture_output=True,timeout=10)
    assert retry.returncode==17 and 'Installation failed' in retry.stdout and 'retry' not in retry.stdout and 'Open Anyway' not in retry.stdout
    for status in ('1','2','143'):
     ordinary=subprocess.run([str(copied/name),'install'],env={**e,'BF_CHILD_STATUS':status},input='r\nx\n',text=True,capture_output=True,timeout=10)
     assert ordinary.returncode==int(status) and 'Installation failed' in ordinary.stdout and 'retry' not in ordinary.stdout
     if status=='1':assert 'helper reason: unknown/modified app input preserved' in ordinary.stderr
    security=subprocess.run([str(copied/name),'install'],env={**e,'BF_CHILD_STATUS':'137'},input='r\nx\n',text=True,capture_output=True,timeout=10)
    assert security.returncode==137 and 'possible SIGKILL' in security.stdout and 'does not establish' in security.stdout and 'file is retained' in security.stdout and approved[0].stat().st_ino==identity
    assert (t/'result').read_text().splitlines()[1:]==['--card-launcher',str(copied),'install']
    approved[0].write_bytes(b'foreign cache bytes');(t/'result').unlink()
    refused=subprocess.run([str(copied/name),'uninstall'],env=e,capture_output=True,timeout=10)
    assert refused.returncode!=0 and not (t/'result').exists() and approved[0].read_bytes()==b'foreign cache bytes'

  p=package/'better-favorites-installer-darwin-arm64';p.unlink()
  e=dict(os.environ,**defaults);assert subprocess.run([str(package/'Install-macOS.command'),'install'],env=e,capture_output=True).returncode!=0
  print('Unix dispatch: 23 simulated OS/version/native/Rosetta/kernel combinations, unsupported-before-invocation, arguments and failure status PASS; not native acceptance')
if __name__=='__main__':main()
