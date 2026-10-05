#!/usr/bin/env python3
"""Actual native copy-to-card menu; temporary fixtures only, never mounted SD.
Not Explorer/Finder/Linux desktop or Miyoo acceptance.
"""
import ctypes,ctypes.util,argparse,hashlib,json,os,platform,shutil,subprocess,tempfile,time,zipfile
from pathlib import Path
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def extract(zp,dest):
 with zipfile.ZipFile(zp) as z:
  assert z.testzip() is None
  for e in z.infolist():
   assert not e.filename.startswith('/') and '..' not in Path(e.filename).parts
   assert ((e.external_attr>>16)&0o170000)!=0o120000
  z.extractall(dest)
  for e in z.infolist():
   if not e.is_dir():(dest/e.filename).chmod((e.external_attr>>16)&0o777)
 for line in (dest/'SHA256SUMS').read_text().splitlines():h,n=line.split('  ',1);assert sha(dest/n)==h,n

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--release',type=Path,required=True);ap.add_argument('--fixtures',type=Path,required=True);a=ap.parse_args();host=platform.system()
 assert host in ('Darwin','Linux'), 'Windows requires native Explorer/cmd qualification separately'
 with tempfile.TemporaryDirectory(prefix='bf-click-test-') as td:
  t=Path(td).resolve();package=t/'package';extract(next(a.release.glob('better-favorites-*.zip')),package)
  if host=='Linux' and ctypes.util.find_library('gio-2.0'):
   # Real GLib desktop Exec parsing, without claiming a graphical terminal test.
   desktop=t/'desktop 日本語';desktop.mkdir();result=desktop/'received'
   script=desktop/'Install-Linux.sh';script.write_text('#!/bin/sh\nprintf "%s" "$0" > "'+str(result)+'"\n')
   entry=desktop/'Install-Linux.desktop';entry.write_text((package/'App/BetterFavorites/Install-Linux.desktop').read_text().replace('Terminal=true','Terminal=false'))
   gio=ctypes.CDLL(ctypes.util.find_library('gio-2.0'));objects=ctypes.CDLL(ctypes.util.find_library('gobject-2.0'))
   gio.g_desktop_app_info_new_from_filename.argtypes=[ctypes.c_char_p];gio.g_desktop_app_info_new_from_filename.restype=ctypes.c_void_p
   gio.g_app_info_launch.argtypes=[ctypes.c_void_p,ctypes.c_void_p,ctypes.c_void_p,ctypes.POINTER(ctypes.c_void_p)];gio.g_app_info_launch.restype=ctypes.c_int
   objects.g_object_unref.argtypes=[ctypes.c_void_p]
   info=gio.g_desktop_app_info_new_from_filename(os.fsencode(str(entry)));assert info,'desktop entry parse failed'
   error=ctypes.c_void_p();assert gio.g_app_info_launch(info,None,None,ctypes.byref(error)),'desktop Exec launch failed';objects.g_object_unref(info)
   for _ in range(100):
    if result.exists():break
    time.sleep(.1)
   assert result.read_text()==str(script), 'desktop path/quoting failed'
  source=package/'App/BetterFavorites';computer=source/'computer';p=json.loads((computer/'package.json').read_text());h=json.loads((computer/'payload/integration/mainui-home/package.json').read_text());r=json.loads((computer/'payload/integration/onion-return/hashes.json').read_text())
  assert p['version']=='1.0.0-rc.7';assert (source/'Install-Windows.cmd').is_file();assert (source/'Install-Linux.desktop').is_file()
  card=t/'SD card 日本語';app=card/'App/BetterFavorites'
  for name in ('App','.tmp_update/bin','.tmp_update/config','.tmp_update/script','.tmp_update/onionVersion','Roms','Saves','Themes'):(card/name).mkdir(parents=True,exist_ok=True)
  (card/'.tmp_update/onionVersion/version.txt').write_text('v4.3.1-1\n')
  for n in h['originals']:shutil.copy2(a.fixtures/'.tmp_update/bin'/n,card/'.tmp_update/bin'/n)
  shutil.copy2(a.fixtures/'.tmp_update/config/better-favorites-return-backup/runtime.sh',card/'.tmp_update/runtime.sh')
  protected={}
  for n in ('Roms/favourite.json','Roms/recentlist.json','Roms/recentlist-hidden.json','Saves/keep.sav','Themes/keep','X-Y-bindings'):
   b=('keep:'+n).encode();(card/n).write_bytes(b);protected[n]=b
  env=dict(os.environ,HOME=str(t/'first-computer'),TMPDIR=str(t/'host-tmp'));Path(env['HOME']).mkdir();Path(env['TMPDIR']).mkdir()
  def copy():
   for n in source.rglob('*'):
    dest=app/n.relative_to(source)
    if n.is_dir():dest.mkdir(parents=True,exist_ok=True)
    else:dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(n,dest)
   # Real Mac-generated sidecar encoding travels to every tested host.
   fixtures=Path(__file__).resolve().parents[1]/'tools/release-installer/testdata'
   (app/'._Install-Linux.desktop').write_bytes((fixtures/'macos-appledouble.bin').read_bytes())
   (app/'._computer').write_bytes((fixtures/'macos-directory-appledouble.bin').read_bytes())
   for d in (app,app/'computer',app/'computer/payload/App/BetterFavorites'):
    (d/'.DS_Store').write_bytes((fixtures/'finder-empty.bin').read_bytes())
   # Host executable bits on SD input are not needed by the click staging route.
   for n in (app/'computer').glob('better-favorites-*'):n.chmod(0o644)
  def unchanged():
   for n,b in protected.items():assert (card/n).read_bytes()==b,n
  def run(stdin='',more=(),ok=True):
   entry=app/('Install-macOS.command' if host=='Darwin' else 'Install-Linux.sh')
   result=subprocess.run(['/bin/sh',str(entry),*more],input=stdin,text=True,capture_output=True,env=env,timeout=180)
   assert (result.returncode==0)==ok,(result.stdout,result.stderr)
   unchanged();return result
  def stock():
   for n,v in h['originals'].items():assert sha(card/'.tmp_update/bin'/n)==v,n
   assert sha(card/'.tmp_update/runtime.sh')==r['original_sha256']
  # Accepted earlier *entry style*, corrected current backend and full current
  # payload: execute the extracted host wrapper directly, then use copied-card
  # entry below. Both call the same transaction/recovery implementation.
  stock()
  standalone=computer/('Install-macOS.command' if host=='Darwin' else 'Install-Linux.sh')
  recovery=t/'direct-recovery'
  controlled=subprocess.run(['/bin/sh',str(standalone),'install','--sd-root',str(card),'--package',str(computer),'--recovery',str(recovery),'--powered-off'],env=env,capture_output=True,text=True,timeout=180)
  assert controlled.returncode==0,(controlled.stdout,controlled.stderr)
  for n,v in h['patched'].items():assert sha(card/'.tmp_update/bin'/n)==v
  assert sha(card/'.tmp_update/runtime.sh')==r['patched_sha256']
  controlled=subprocess.run(['/bin/sh',str(standalone),'uninstall','--sd-root',str(card),'--package',str(computer),'--recovery',str(recovery),'--archive',str(t/'direct-archive'),'--powered-off'],env=env,capture_output=True,text=True,timeout=180)
  assert controlled.returncode==0,(controlled.stdout,controlled.stderr)
  assert not app.exists();stock();unchanged()
  copy();stock();run('0\n');assert not (card/'.tmp_update/config/better-favorites-installation.json').exists()
  result=run('1\ny\n');assert 'Installing Better Favorites' in result.stdout and 'Preparing recovery' in result.stdout
  result=run('3\n');assert '[3] Export diagnostics' in result.stdout
  exports=list(Path(env['HOME']).glob('better-favorites-diagnostics-*.zip'));assert len(exports)==1
  with zipfile.ZipFile(exports[0]) as z:
   assert 'report.json' in z.namelist() and any(n.startswith('installer/session-') for n in z.namelist())
  for n,v in h['patched'].items():assert sha(card/'.tmp_update/bin'/n)==v
  assert sha(card/'.tmp_update/runtime.sh')==r['patched_sha256']
  assert not (app/'settings.conf').exists() and not (app/'home-entry.conf').exists()
  assert (app/'welcome-pending').read_bytes()==b'BetterFavoritesWelcome1\n'
  index=json.loads((card/'.tmp_update/config/better-favorites-installation.json').read_text());assert sha(card/index['recovery']/'recovery.json')==index['sha256']
  for s in json.loads((card/index['recovery']/'recovery.json').read_text())['files']:
   if s['stock_sha256']!='absent':assert sha(card/index['recovery']/'files'/s['path'])==s['stock_sha256']
  prefs={'settings.conf':b'BetterFavoritesSettings1\n1\n'+b'0'*32+b'\n','home-entry.conf':b'BetterFavoritesHome1\n1\n','browser-preferences.conf':b'BetterFavoritesBrowserPreferences1\n0\n0\n1\n','browser-state':b'BetterFavoritesBrowserState1\n0\n'}
  for n,b in prefs.items():(app/n).write_bytes(b)
  (app/'._welcome-pending').write_bytes((Path(__file__).resolve().parents[1]/'tools/release-installer/testdata/macos-appledouble.bin').read_bytes())
  (app/'welcome-pending').unlink();copy();run('1\ny\n')
  for n,b in prefs.items():assert (app/n).read_bytes()==b,n
  assert not (app/'welcome-pending').exists()
  metadata=app/'._Install-Linux.desktop';valid=metadata.read_bytes();metadata.write_bytes(b'unknown metadata');run('1\ny\n',ok=False);assert metadata.read_bytes()==b'unknown metadata';metadata.write_bytes(valid)
  bad=app/'foreign.txt';bad.write_text('foreign');run('1\ny\n',ok=False);assert bad.read_text()=='foreign';bad.unlink()
  payload=app/'computer/payload/App/BetterFavorites/icon.png';original=payload.read_bytes();payload.write_bytes(b'tampered');before=(card/'.tmp_update/runtime.sh').read_bytes();run('1\ny\n',ok=False);assert (card/'.tmp_update/runtime.sh').read_bytes()==before;payload.write_bytes(original)
  # Relocation + another computer HOME: no original computer recovery exists.
  moved=t/'Other mount – новый';card.rename(moved);card=moved;app=card/'App/BetterFavorites';env['HOME']=str(t/'second-computer');Path(env['HOME']).mkdir()
  run('2\ny\n');assert not app.exists();stock()
  assert not any(n.name.startswith('better-favorites-') for n in (card/'.tmp_update/config').iterdir())
  archives=list((Path(env['HOME'])/'BetterFavorites-Recovery').glob('uninstall-*'));assert len(archives)==1
  assert json.loads((archives[0]/'uninstall-result.json').read_text())['complete']
  copy();run('1\ny\n');assert not (app/'settings.conf').exists();assert not (app/'home-entry.conf').exists();assert (app/'welcome-pending').exists()
  # Missing portable recovery is an honest failure; nothing is removed.
  index=json.loads((card/'.tmp_update/config/better-favorites-installation.json').read_text());recovery=card/index['recovery'];kept=recovery.with_name('kept-test-recovery');recovery.rename(kept);run('2\ny\n',ok=False);assert app.exists();kept.rename(recovery)
  run('2\ny\n');stock();assert not app.exists()
  # Both unsupported integrations offer app-only only on a genuine version mismatch.
  (card/'.tmp_update/onionVersion/version.txt').write_text('v4.4.0\n');copy();run('1\ny\ny\n');stock();assert not (app/'home-integration.conf').exists();assert not (card/'.tmp_update/script/better_favorites_return.sh').exists();run('2\ny\n');assert not app.exists();stock()
  assert not list(Path(env['TMPDIR']).glob('better-favorites-*')), 'private execution stage remains'
  print('PASS: actual '+host+' full ZIP menu, copy overlap, Unicode paths, source non-executable bits, cancel, both integrations/default OFF, update preferences, integrity/foreign refusal, moved card/new computer, portable uninstall/reinstall, missing recovery, unsupported-version app-only, temporary stage cleanup. Not native click/device acceptance.')
if __name__=='__main__':main()
