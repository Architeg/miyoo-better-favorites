"""Pinned native host builds, dependency verification and entry scripts."""
import hashlib, json, os, shutil, struct, subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def inspect_linux(path):
 data=path.read_bytes()
 if data[:6]!=b"\x7fELF\x02\x01":raise RuntimeError("Not ELF64 little-endian: "+str(path))
 offset=struct.unpack_from('<Q',data,32)[0];size,count=struct.unpack_from('<HH',data,54)
 kinds=[struct.unpack_from('<I',data,offset+i*size)[0] for i in range(count)]
 if 2 in kinds or 3 in kinds:raise RuntimeError("Linux installer has dynamic/interpreter dependency: "+str(path))
 return {"format":"ELF64", "PT_INTERP":False,"PT_DYNAMIC":False,"dynamic_libraries":[],"linkage":"static"}

def inspect_mac(path):
 data=path.read_bytes()
 if data[:4]!=b"\xcf\xfa\xed\xfe":raise RuntimeError("Not Mach-O64: "+str(path))
 count=struct.unpack_from('<I',data,16)[0];offset=32;minimum=None;libraries=[]
 for _ in range(count):
  cmd,size=struct.unpack_from('<II',data,offset)
  if cmd==0x32: minimum=struct.unpack_from('<I',data,offset+12)[0]
  if cmd==0x24: minimum=struct.unpack_from('<I',data,offset+8)[0]
  if cmd in (0xc,0x80000018,0x8000001f):
   start=struct.unpack_from('<I',data,offset+8)[0]+offset;libraries.append(data[start:offset+size].split(b'\0')[0].decode())
  offset+=size
 if minimum is None or minimum>(12<<16):raise RuntimeError("Mach-O deployment target exceeds Monterey: "+str(path))
 return {"format":"Mach-O64","minimum_macos":"%d.%d.%d"%(minimum>>16,(minimum>>8)&255,minimum&255),"dynamic_libraries":libraries}

def build_hosts(stage,legacy_toolchain,out):
 version=subprocess.check_output(['go','version'],text=True).strip()
 if not version.startswith('go version go1.26.2 '):raise RuntimeError('Pin normal toolchain Go1.26.2; Go1.27 raises macOS minimum')
 legacy=legacy_toolchain.resolve();oldgo=legacy/'bin/go'
 if not subprocess.check_output([str(oldgo),'version'],text=True).startswith('go version go1.20.14 '):raise RuntimeError('Exact Go1.20.14 legacy toolchain required')
 # The legacy copy has go1.20 in isolation. Normal go.mod remains untouched.
 legacyout=out/'legacy-build'
 subprocess.run(['python3',str(ROOT/'tools/build-windows7-test.py'),'--toolchain',str(legacy),'--output',str(legacyout)],check=True)
 records=[]
 for arch in ('amd64','386'):
  name='better-favorites-installer-windows7-'+arch+'.exe';shutil.copy2(legacyout/name,stage/name)
  records.append(dict(file=name,os='windows',arch=arch,versions='7/8/8.1',toolchain='go1.20.14'))
 for goos,arch in (('windows','386'),('windows','amd64'),('windows','arm64'),('darwin','arm64'),('darwin','amd64'),('linux','amd64'),('linux','arm64')):
  name='better-favorites-installer-'+goos+'-'+arch+('.exe' if goos=='windows' else '')
  env=dict(os.environ,GOOS=goos,GOARCH=arch,CGO_ENABLED='0',GOAMD64='v1',GOARM64='v8.0',GO386='sse2',GOTOOLCHAIN='local',GOWORK='off')
  subprocess.run(['go','build','-trimpath','-buildvcs=false','-ldflags=-s -w','-o',str((stage/name).resolve()),'.'],cwd=ROOT/'tools/release-installer',env=env,check=True)
  info=inspect_linux(stage/name) if goos=='linux' else inspect_mac(stage/name) if goos=='darwin' else {"format":"PE"}
  records.append(dict(file=name,os=goos,arch=arch,toolchain='go1.26.2',minimum=('Windows10' if goos=='windows' else 'macOS12' if goos=='darwin' else 'kernel3.2 (ARM64 3.7)'),**info))
 subprocess.run(['python3',str(ROOT/'tools/generate-windows-entry.py'),'--check'],check=True)
 for name in ('Install-Windows.cmd','Install-macOS.command','Install-Linux.sh'):
  shutil.copy2(ROOT/'packaging'/name,stage/name)
  if not name.endswith('.cmd'):(stage/name).chmod(0o755)
 for record in records:record['sha256']=hashlib.sha256((stage/record['file']).read_bytes()).hexdigest()
 (stage/'HOST-SHA256SUMS').write_text(''.join(record['sha256']+'  '+record['file']+'\n' for record in records))
 (stage/'HOST-BUILDS.json').write_text(json.dumps(dict(format=1,normal_toolchain=version,legacy_toolchain='go1.20.14',payload='One package.json and payload/ tree for every host; no host-specific Miyoo paths',execution='Cross-build/dependency metadata, not native acceptance',files=records),indent=2)+'\n')
 return records
