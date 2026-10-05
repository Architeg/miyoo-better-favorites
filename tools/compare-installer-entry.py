#!/usr/bin/env python3
"""Read preserved installer ZIPs; report entry/tool byte identity without executing them."""
import argparse, hashlib, json, zipfile
from pathlib import Path

def inspect(path):
 with zipfile.ZipFile(path) as z:
  if z.testzip() is not None:raise ValueError('Invalid archive')
  names=z.namelist();result={}
  for n in names:
   bname=Path(n).name
   if bname in ('Install-macOS.command','Install-Windows.cmd','HOST-BUILDS.json','package.json') or bname.startswith(('better-favorites-installer-','better-favorites-dispatch-')):
    b=z.read(n);entry={'bytes':len(b),'sha256':hashlib.sha256(b).hexdigest()}
    if bname.endswith(('.command','.cmd')):entry['script']=b.decode('utf-8')
    result[n]=entry
  return {'archive':path.name,'archive_sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'files':result}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('archives',nargs='+',type=Path);a=p.parse_args()
 print(json.dumps([inspect(n) for n in a.archives],indent=2))
