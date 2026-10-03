#!/usr/bin/env python3
"""Build separate bootstrap assets; never overwrite candidates or alter offline installers."""
import argparse,hashlib,json,os,subprocess
from pathlib import Path
root=Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);p.add_argument('--legacy-toolchain',type=Path,required=True);a=p.parse_args()
if a.output.exists():raise SystemExit('Fresh output required')
modern=subprocess.check_output(['go','version'],text=True).strip();old=a.legacy_toolchain.resolve()/'bin/go'
assert modern.startswith('go version go1.26.2 ')
assert subprocess.check_output([str(old),'version'],text=True).startswith('go version go1.20.14 ')
a.output.mkdir(parents=True);out=a.output.resolve();records=[]
for target,arch in [('windows','386'),('darwin','amd64'),('darwin','arm64'),('linux','amd64'),('linux','arm64')]:
 name=f'better-favorites-bootstrap-{target}-{arch}'+('.exe' if target=='windows' else '')
 env=dict(os.environ,CGO_ENABLED='0',GOOS=target,GOARCH=arch,GOWORK='off',GOTOOLCHAIN='local',GOAMD64='v1',GOARM64='v8.0',GO386='sse2')
 go='go'
 if target=='windows':go=str(old);env['GOROOT']=str(a.legacy_toolchain.resolve())
 subprocess.run([go,'build','-trimpath','-buildvcs=false','-ldflags=-s -w','-o',str(out/name),'.'],cwd=root,env=env,check=True)
 records.append({'file':name,'sha256':hashlib.sha256((out/name).read_bytes()).hexdigest(),'toolchain':'go1.20.14' if target=='windows' else 'go1.26.2'})
(out/'BOOTSTRAP-NOTICES.txt').write_bytes((root.parents[1]/'LICENSE').read_bytes()+b'\nGo runtime/standard library:\n'+(root.parents[1]/'third_party/notices/Go-BSD.txt').read_bytes())
(out/'BOOTSTRAP-SHA256SUMS').write_text(''.join(r['sha256']+'  '+r['file']+'\n' for r in records))
(out/'BUILD-REPORT.json').write_text(json.dumps({'files':records,'acceptance':'Cross-build only; native Windows TLS/bootstrap execution and public-asset roundtrips pending'},indent=2)+'\n')
