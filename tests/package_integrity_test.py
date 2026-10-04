#!/usr/bin/env python3
"""Verify the one user ZIP, independent of private firmware or mounted cards."""
import argparse,hashlib,json,stat,subprocess,zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sha=lambda b:hashlib.sha256(b).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--release',type=Path,required=True);a=p.parse_args()
 users=list(a.release.glob('*.zip'));assert len(users)==1 and users[0].name=='better-favorites-1.0.0-rc.5.zip',users
 for line in (a.release/'SHA256SUMS').read_text().splitlines():h,n=line.split('  ',1);assert sha((a.release/n).read_bytes())==h,n
 with zipfile.ZipFile(users[0]) as z:
  assert z.testzip() is None;names=set(z.namelist());prefix='App/BetterFavorites/';computer=prefix+'computer/'
  assert not any('App/BetterFavoritesTest/' in n for n in names)
  for line in z.read('SHA256SUMS').decode().splitlines():h,n=line.split('  ',1);assert sha(z.read(n))==h,n
  manifest=json.loads(z.read(computer+'transport.json'))
  for f in manifest['files']:assert sha(z.read(prefix+f['path']))==f['sha256'],f['path']
  package=json.loads(z.read(computer+'package.json'))
  for f in package['files']:assert sha(z.read(computer+'payload/'+f['path']))==f['sha256'],f['path']
  assert z.read(prefix+'icon.png')==(ROOT/'icon.png').read_bytes()
  for n in ['settings.conf','browser-state','browser-preferences.conf','home-entry.conf','home-diagnostics.conf','better-favorites.log','welcome-pending']:assert prefix+n not in names,n
  arm=z.read(prefix+'better-favorites');assert arm[:7]==b'\x7fELF\x01\x01\x01' and arm[18:20]==b'\x28\x00'
  assert package['commit'].encode() in arm and package['version'].encode() in arm
  for n in ['better-favorites','launch.sh','Install-macOS.command','Install-Linux.sh','Install-Linux.desktop']:assert z.getinfo(prefix+n).external_attr>>16 & stat.S_IXUSR,n
  for entry in json.loads((ROOT/'docs/release/dependency-hashes.json').read_text()):assert sha(z.read(prefix+entry['file']))==entry['sha256'],entry['file']
  for entry in json.loads((ROOT/'third_party/notices/SwiftShader-supplemental.json').read_text()):assert sha(z.read('licenses/'+entry['file']))==entry['sha256'] and sha(z.read(computer+'licenses/'+entry['file']))==entry['sha256']
  assert json.loads(z.read(computer+'payload/integration/mainui-home/package.json'))['originals']==json.loads(z.read(computer+'payload/integration/mainui-home/legacy-package.json'))['originals']
  assert '/mnt/SDCARD/App/BetterFavorites/icon.png' in z.read(prefix+'config.json').decode()
  assert b'App/BetterFavoritesTest' not in z.read(computer+'payload/integration/onion-return/better_favorites_return.sh')
  assert b'/mnt/SDCARD/App/BetterFavorites' in z.read(computer+'payload/integration/mainui-home/adapter.elf')
  subprocess.run(['sh','-n'],input=z.read(prefix+'launch.sh'),check=True)
 print('PASS: one user ZIP, CRC/checksums, transport/payload manifests, source identity, ARM format, permissions, icon, unchanged libraries, no personal state, supplemental notices and new command paths')
if __name__=='__main__':main()
