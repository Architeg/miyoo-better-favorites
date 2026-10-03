#!/usr/bin/env python3
"""Isolated Go 1.20.14 compatibility build; never changes release go.mod/toolchain."""
import argparse,json,os,shutil,subprocess,tempfile,hashlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--toolchain',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
 go=a.toolchain.resolve()/'bin/go'
 version=subprocess.check_output([str(go),'version'],text=True).strip()
 if not version.startswith('go version go1.20.14 '):raise SystemExit('Exact official Go 1.20.14 required')
 if a.output.exists():raise SystemExit('Fresh output required')
 a.output.mkdir(parents=True);out=a.output.resolve();e=dict(os.environ,GOROOT=str(a.toolchain.resolve()),CGO_ENABLED='0',GOWORK='off',GOPROXY='off',GOSUMDB='off')
 with tempfile.TemporaryDirectory(prefix='bf-win7-source-') as tmp:
  source=Path(tmp)
  for f in (ROOT/'tools/release-installer').glob('*.go'):shutil.copy2(f,source/f.name)
  (source/'go.mod').write_text('module better-favorites/release-installer\n\ngo 1.20\n')
  with (out/'host-tests.log').open('wb') as log:subprocess.run([str(go),'test','./...'],cwd=source,env=e,stdout=log,stderr=subprocess.STDOUT,check=True)
  for arch in ('amd64','386'):
   env=dict(e,GOOS='windows',GOARCH=arch)
   subprocess.run([str(go),'build','-trimpath','-buildvcs=false','-ldflags=-s -w','-o',str(out/('better-favorites-installer-windows7-'+arch+'.exe')),'.'],cwd=source,env=env,check=True)
   subprocess.run([str(go),'test','-c','-o',str(out/('windows7-'+arch+'-tests.exe')),'.'],cwd=source,env=env,check=True)
 report={'toolchain':version,'normal_module':'go 1.24 unchanged; normal build Go 1.26.2','compatibility_change':'zeroing loop replaces Go 1.21 clear builtin; equivalent ARM ELF output tested','execution':'Mac host tests only; Windows 7 execution and SD reader qualification pending','sha256':{f.name:hashlib.sha256(f.read_bytes()).hexdigest() for f in out.glob('*.exe')}}
 (out/'BUILD-REPORT.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
if __name__=='__main__':main()
