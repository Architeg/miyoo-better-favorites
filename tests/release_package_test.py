#!/usr/bin/env python3
"""Exercise actual extracted release executables using private audited fixtures.
No mounted-card access: all writes are in a fresh host temporary directory.
"""
import argparse,hashlib,json,os,platform,shutil,subprocess,tempfile,zipfile
from pathlib import Path

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def extract(zip_path,dest):
 with zipfile.ZipFile(zip_path) as z:
  for entry in z.infolist():
   parts=Path(entry.filename).parts
   if entry.filename.startswith('/') or '..' in parts or (entry.external_attr>>16)&0o170000==0o120000:raise RuntimeError('Unsafe ZIP member')
  z.extractall(dest)
  for entry in z.infolist():
   if not entry.is_dir() and entry.external_attr>>16: (dest/entry.filename).chmod((entry.external_attr>>16)&0o777)
 for line in (dest/'SHA256SUMS').read_text().splitlines():
  digest,rel=line.split('  ',1);assert sha(dest/rel)==digest,rel

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--release',type=Path,required=True);p.add_argument('--fixtures',type=Path,required=True);a=p.parse_args()
 release=a.release.resolve();fixtures=a.fixtures.resolve()
 for line in (release/'SHA256SUMS').read_text().splitlines():digest,rel=line.split('  ',1);assert sha(release/rel)==digest,rel
 host={'Darwin':'darwin','Linux':'linux','Windows':'windows'}[platform.system()];arch='arm64' if platform.machine().lower() in ('arm64','aarch64') else 'amd64'
 with tempfile.TemporaryDirectory(prefix='bf-release-zip-test-') as td:
  temp=Path(td).resolve();package=temp/'package';base=temp/'app-only';extract(next(release.glob('better-favorites-*-installer.zip')),package);extract(next(release.glob('better-favorites-*-app-only.zip')),base)
  tool=package/('better-favorites-installer-'+host+'-'+arch+('.exe' if host=='windows' else ''));assert tool.is_file()
  # Unix actual ZIP roundtrips execute the public entry script as well as its backend.
  entry=package/('Install-macOS.command' if host=='darwin' else 'Install-Linux.sh') if host!='windows' else tool
  prefix=['sh',str(entry)] if host!='windows' else [str(entry)]
  card=temp/'card';app=card/'App/BetterFavoritesTest';app.mkdir(parents=True)
  for folder in ('.tmp_update/bin','.tmp_update/config','.tmp_update/script','.tmp_update/onionVersion','Roms','Saves'): (card/folder).mkdir(parents=True,exist_ok=True)
  (card/'.tmp_update/onionVersion/version.txt').write_text('v4.3.1-1\n')
  h=json.loads((package/'payload/integration/mainui-home/package.json').read_text());r=json.loads((package/'payload/integration/onion-return/hashes.json').read_text())
  for name in h['originals']:shutil.copy2(fixtures/'.tmp_update/bin'/name,card/'.tmp_update/bin'/name)
  shutil.copy2(fixtures/'.tmp_update/config/better-favorites-return-backup/runtime.sh',card/'.tmp_update/runtime.sh')
  protected={}
  for rel in ('Roms/favourite.json','Roms/recentlist.json','Roms/recentlist-hidden.json','Saves/keep.sav','App/BetterFavoritesTest/settings.conf','App/BetterFavoritesTest/home-entry.conf','App/BetterFavoritesTest/browser-preferences.conf','App/BetterFavoritesTest/browser-state'):
   path=card/rel;path.write_bytes(('personal:'+rel).encode());protected[rel]=path.read_bytes()
  for source in (base/'App/BetterFavoritesTest').rglob('*'):
   if source.is_file():
    destination=app/source.relative_to(base/'App/BetterFavoritesTest');destination.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(source,destination)
  for rel,data in protected.items():assert (card/rel).read_bytes()==data
  arm=(app/'better-favorites').read_bytes();assert arm[:7]==b'\x7fELF\x01\x01\x01' and arm[18:20]==b'\x28\x00'
  if host!='windows':
   assert os.access(app/'better-favorites',os.X_OK) and os.access(app/'launch.sh',os.X_OK)
   subprocess.run(['sh','-n',str(app/'launch.sh')],check=True)
  removed_app=False
  def run(action,more=(),okay=True):
   args=[*prefix,action,'--sd-root',str(card),*map(str,more)]
   if action not in ('status','export-diagnostics'):args+=['--powered-off']
   result=subprocess.run(args,capture_output=True,text=True,timeout=120)
   assert (result.returncode==0)==okay,(args,result.stdout,result.stderr)
   for rel,data in protected.items():
    if rel.startswith('App/') and removed_app:continue
    assert (card/rel).read_bytes()==data,rel
   return result
  recovery=temp/'recovery';run('install',['--home','--return','--recovery',recovery])
  for name,digest in h['patched'].items():assert sha(card/'.tmp_update/bin'/name)==digest
  assert sha(card/'.tmp_update/runtime.sh')==r['patched_sha256']
  assert sha(card/'.tmp_update/script/better_favorites_return.sh')==r['helper_sha256']
  latest=temp/'update';run('install',['--home','--return','--recovery',latest])
  run('export-diagnostics',['--output',temp/'diagnostics.zip']);assert not (app/'home-diagnostics.conf').exists()
  name=next(iter(h['patched']));target=card/'.tmp_update/bin'/name;expected=target.read_bytes();target.write_bytes(b'foreign')
  runtime_before=(card/'.tmp_update/runtime.sh').read_bytes();run('uninstall',['--recovery',latest],False);assert target.read_bytes()==b'foreign';assert (card/'.tmp_update/runtime.sh').read_bytes()==runtime_before
  target.write_bytes(expected);run('remove-integrations',['--recovery',latest])
  for name,digest in h['originals'].items():assert sha(card/'.tmp_update/bin'/name)==digest
  assert sha(card/'.tmp_update/runtime.sh')==r['original_sha256'];assert not (app/'home-integration.conf').exists()
  again=temp/'reinstall';run('install',['--home','--return','--recovery',again]);
  # Simulate unavailable UI / an interrupted restore with an exact stock/patched mixture.
  name=next(iter(h['originals']));shutil.copy2(again/'files/.tmp_update/bin'/name,card/'.tmp_update/bin'/name)
  run('restore',['--recovery',again]);run('restore',['--recovery',again])
  # Replace artificial personal fixtures with app-owned formats for complete removal.
  for name,magic in {'settings.conf':'BetterFavoritesSettings1','home-entry.conf':'BetterFavoritesHome1','browser-preferences.conf':'BetterFavoritesBrowserPreferences1','browser-state':'BetterFavoritesBrowserState1'}.items():
   data=(magic+'\n0\n').encode();(app/name).write_bytes(data);protected['App/BetterFavoritesTest/'+name]=data
  removed_app=True
  run('uninstall',['--recovery',again,'--archive',temp/'uninstall-archive']);assert not app.exists()
  for rel,data in protected.items():
   if not rel.startswith('App/'):assert (card/rel).read_bytes()==data
  assert (temp/'uninstall-archive/recovery/recovery.json').exists()
  run('install',['--home','--return','--recovery',temp/'post-complete-reinstall'])
  target=card/'.tmp_update/bin'/next(iter(h['originals']));target.write_bytes(b'unknown');run('install',['--home','--return','--recovery',temp/'refused'],False);assert target.read_bytes()==b'unknown';assert not (temp/'refused').exists()
  print('Actual ZIPs/native '+host+' executable: app-only, install/update, protected state, diagnostics, integrations-only/stock/reinstall, complete uninstall/host archive/reinstall, repeat/interrupted restore, unknown/foreign refusal: PASS')
if __name__=='__main__':main()
