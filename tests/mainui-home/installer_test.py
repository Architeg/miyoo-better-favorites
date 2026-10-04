#!/usr/bin/env python3
"""Isolated host directories only; optional read-only audited inputs for full installation."""
import importlib.util
import json
import os
from pathlib import Path
import shutil
import sys
import tempfile
import unittest
from unittest import mock
import subprocess
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'integration/mainui-home'))
import manage
import diagnostics
INPUTS=Path(sys.argv.pop(1)) if len(sys.argv)>1 else None
PAYLOAD=Path(sys.argv.pop(1)) if len(sys.argv)>1 else None
class Transactions(unittest.TestCase):
 def test_native_windows_refusal_before_io(self):
  with mock.patch.object(manage.os,'name','nt'):
   with self.assertRaisesRegex(RuntimeError,'Native Windows Python is unsupported'):manage.regular('/unused')
 def test_filesystem_probe(self):
  module=importlib.util.spec_from_file_location('fsprobe',ROOT/'tools/check-m6-filesystem.py');probe=importlib.util.module_from_spec(module);module.loader.exec_module(probe)
  with tempfile.TemporaryDirectory() as tmp:
   self.assertEqual(probe.probe(Path(tmp))['result'],'PASS')
   self.assertEqual(list(Path(tmp).iterdir()),[])
 def test_failure_and_foreign_rollback(self):
  for fail in ('before','after','foreign'):
   with tempfile.TemporaryDirectory() as tmp:
    a,b=Path(tmp)/'a',Path(tmp)/'b';a.write_bytes(b'old-a');b.write_bytes(b'old-b')
    def fault(phase,path):
     if phase=='after' and path==b:
      if fail=='foreign':a.write_bytes(b'foreign')
      raise OSError('injected publication failure')
     if fail=='before' and phase=='before' and path==b:raise OSError('injected failure')
    with self.assertRaises(RuntimeError):manage.transaction({a:(b'old-a',b'new-a',0o700),b:(b'old-b',b'new-b',0o700)},hook=fault)
    self.assertEqual(a.read_bytes(),b'foreign' if fail=='foreign' else b'old-a');self.assertEqual(b.read_bytes(),b'old-b')
    self.assertFalse(list(Path(tmp).glob('*.home-stage-*')))
 def test_fifo_and_conflict(self):
  with tempfile.TemporaryDirectory() as tmp:
   p=Path(tmp)/'fifo';os.mkfifo(p)
   with self.assertRaises(RuntimeError):manage.regular(p)
   p.unlink();p.write_bytes(b'foreign')
   with self.assertRaises(RuntimeError):manage.transaction({p:(b'old',b'new',0o600)})
   self.assertEqual(p.read_bytes(),b'foreign')
@unittest.skipUnless(INPUTS and PAYLOAD,'Supply read-only audited binary directory and payload for full tests')
class Installer(unittest.TestCase):
 def seed(self,root):
  for name in ('.tmp_update/bin','.tmp_update/config','.tmp_update/onionVersion','App/BetterFavorites'): (root/name).mkdir(parents=True,exist_ok=True)
  (root/'.tmp_update/onionVersion/version.txt').write_text('v4.3.1-1\n')
  source=INPUTS.parent
  original=source/'config/better-favorites-return-backup/runtime.sh'
  (root/'.tmp_update/runtime.sh').write_bytes(manage.regular(original if original.exists() else source/'runtime.sh'))
  for name in manage.prototype.HASHES:shutil.copy2(INPUTS/name,root/'.tmp_update/bin'/name)
  for name,data in (('settings.conf',b'personal-generation'),('browser-preferences.conf',b'personal'),('home-entry.conf',b'BetterFavoritesHome1\n1\n')):(root/'App/BetterFavorites'/name).write_bytes(data)
 def test_roundtrip_and_diagnostics(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=Path(tmp)/'card';root.mkdir();self.seed(root)
   before={p:p.read_bytes() for p in root.rglob('*') if p.is_file()}
   manage.manage(root,'install',PAYLOAD)
   marker=root/'App/BetterFavorites/home-integration.conf';self.assertEqual(marker.read_bytes(),manage.receipt(manage.catalogue()))
   for name,h in manage.catalogue()['patched'].items():self.assertEqual(manage.prototype.digest((root/'.tmp_update/bin'/name).read_bytes()),h)
   diagnostics.run(root,'enable');diagnostics.run(root,'disable');self.assertFalse((root/'App/BetterFavorites/home-diagnostics.conf').exists())
   (root/'App/BetterFavorites/home-diagnostics.log').write_text('evidence')
   diagnostics.run(root,'collect',Path(tmp)/'archive');self.assertEqual((Path(tmp)/'archive/home-diagnostics.log').read_text(),'evidence')
   unknown=root/'App/BetterFavorites/home-diagnostics.conf';unknown.write_text('foreign')
   with self.assertRaisesRegex(RuntimeError,'preserved'):diagnostics.run(root,'remove')
   self.assertEqual(unknown.read_text(),'foreign');unknown.unlink()
   diagnostics.run(root,'remove');self.assertEqual((root/'App/BetterFavorites/home-diagnostics.log').read_text(),'evidence')
   manage.manage(root,'uninstall');self.assertFalse(marker.exists())
   for p,data in before.items():self.assertEqual(p.read_bytes(),data)
 def test_conflicts_and_backup(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=Path(tmp);self.seed(root);manage.manage(root,'install',PAYLOAD)
   binary=root/'.tmp_update/bin/MainUI-283-clean';binary.write_bytes(b'foreign')
   with self.assertRaisesRegex(RuntimeError,'preserved'):manage.manage(root,'uninstall')
   self.assertEqual(binary.read_bytes(),b'foreign');self.assertTrue((root/'App/BetterFavorites/home-integration.conf').exists())
 def test_install_and_uninstall_rollback(self):
  for action in ('install','uninstall'):
   with tempfile.TemporaryDirectory() as tmp:
    root=Path(tmp);self.seed(root)
    if action=='uninstall':manage.manage(root,'install',PAYLOAD)
    paths=[root/'.tmp_update/bin'/n for n in manage.prototype.HASHES]
    before={p:p.read_bytes() for p in paths}
    def fault(phase,path):
     if phase=='after' and path==paths[1]:raise OSError('injected failure')
    with self.assertRaises(RuntimeError):manage.manage(root,action,PAYLOAD,hook=fault)
    for p,data in before.items():self.assertEqual(p.read_bytes(),data)
 def test_backup_failure_before_replacement(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=Path(tmp);self.seed(root);paths=[root/'.tmp_update/bin'/n for n in manage.prototype.HASHES];before={p:p.read_bytes() for p in paths}
   real=manage.stage
   def corrupt(path,data,mode):
    staged=real(path,data,mode)
    if path.name=='MainUI-283-clean' and path.parent.name.startswith('better-favorites-home-backup-'):staged.write_bytes(b'bad backup')
    return staged
   with mock.patch.object(manage,'stage',side_effect=corrupt):
    with self.assertRaisesRegex(RuntimeError,'Backup verification'):manage.manage(root,'install',PAYLOAD)
   for p,data in before.items():self.assertEqual(p.read_bytes(),data)
   self.assertFalse((root/'.tmp_update/config/better-favorites-home.json').exists())
 def test_interrupted_install_journal_recovery(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=Path(tmp);self.seed(root);manage.manage(root,'install',PAYLOAD)
   path=root/'.tmp_update/config/better-favorites-home.json';manifest=json.loads(path.read_text());manifest['status']='prepared';path.write_text(json.dumps(manifest))
   backup=root/manifest['backup'];name='MainUI-283-clean';(root/'.tmp_update/bin'/name).write_bytes((backup/name).read_bytes())
   (root/'App/BetterFavorites/home-integration.conf').unlink()
   manage.manage(root,'uninstall')
   for n,h in manage.prototype.HASHES.items():self.assertEqual(manage.prototype.digest((root/'.tmp_update/bin'/n).read_bytes()),h)
 def test_interrupted_uninstall_and_legacy_recovery(self):
  for legacy in (False,True):
   with tempfile.TemporaryDirectory() as tmp:
    root=Path(tmp);self.seed(root);manage.manage(root,'install',PAYLOAD)
    path=root/'.tmp_update/config/better-favorites-home.json';installed=path.read_bytes()
    first=root/'.tmp_update/bin/MainUI-283-clean'
    def interrupt(phase,p):
     if phase=='after' and p==first:raise KeyboardInterrupt('simulated power loss')
    with self.assertRaises(KeyboardInterrupt):manage.manage(root,'uninstall',hook=interrupt)
    self.assertEqual(json.loads(path.read_text())['status'],'uninstalling')
    if legacy:path.write_bytes(installed)
    foreign=root/'.tmp_update/bin/MainUI-354-expert';saved=foreign.read_bytes();foreign.write_bytes(b'foreign')
    before={p:p.read_bytes() for p in root.rglob('*') if p.is_file()}
    with self.assertRaisesRegex(RuntimeError,'preserved'):manage.manage(root,'recover')
    for p,data in before.items():self.assertEqual(p.read_bytes(),data)
    foreign.write_bytes(saved);manage.manage(root,'recover')
    for name,sha in manage.prototype.HASHES.items():self.assertEqual(manage.prototype.digest((root/'.tmp_update/bin'/name).read_bytes()),sha)
    self.assertEqual(json.loads(path.read_text())['status'],'uninstalled')
 def test_preserve_permanent_return_and_availability(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=Path(tmp);self.seed(root);source=INPUTS.parent
   # This optional full fixture uses mounted authority only for READS.
   if manage.prototype.digest(manage.regular(source/'runtime.sh'))!=json.loads(manage.regular(manage.RETURN/'hashes.json'))['patched_sha256']:
    self.skipTest('No audited permanent return integration in input fixture')
   (root/'.tmp_update/runtime.sh').write_bytes(manage.regular(source/'runtime.sh'))
   for folder in ('script','config/better-favorites-return-backup'):
    (root/'.tmp_update'/folder).mkdir(parents=True,exist_ok=True)
   for name in ('script/better_favorites_return.sh','config/better-favorites-return-backup/runtime.sh','config/better-favorites-return-backup/manifest.json'):
    (root/'.tmp_update'/name).write_bytes(manage.regular(source/name))
   protected={p:p.read_bytes() for p in root.rglob('*') if p.is_file() and '.tmp_update/bin' not in str(p)}
   manage.manage(root,'install',PAYLOAD)
   for p,data in protected.items():self.assertEqual(p.read_bytes(),data)
   cpp=Path(tmp)/'availability.cpp';exe=Path(tmp)/'availability'
   cpp.write_text('#include "home_entry_settings.h"\nint main(int c,char** v){return c==3?int(homeEntryStatus(v[1],v[2])):3;}\n')
   subprocess.run(['c++','-std=c++17','-I'+str(ROOT/'include'),str(cpp),str(ROOT/'src/home_entry_settings.cpp'),'-o',str(exe)],check=True)
   args=[str(exe),str(root),str(root/'App/BetterFavorites')];self.assertEqual(subprocess.run(args).returncode,0)
   marker=root/'App/BetterFavorites/home-integration.conf';saved=marker.read_bytes();marker.write_text('foreign');self.assertEqual(subprocess.run(args).returncode,2);marker.write_bytes(saved)
   binary=root/'.tmp_update/bin/MainUI-354-clean';saved=binary.read_bytes();binary.write_bytes(saved+b'foreign');self.assertEqual(subprocess.run(args).returncode,2);binary.write_bytes(saved)
   manage.manage(root,'uninstall');self.assertEqual(subprocess.run(args).returncode,1)
   version=root/'.tmp_update/onionVersion/version.txt';version.write_text('unsupported');self.assertEqual(subprocess.run(args).returncode,2);version.write_text('v4.3.1-1\n')
   for p,data in protected.items():self.assertEqual(p.read_bytes(),data)
 def test_six_file_recovery_without_device_ui(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=Path(tmp)/'card';root.mkdir();self.seed(root);backup=Path(tmp)/'host-backup';backup.mkdir()
   app=root/'App/BetterFavorites'
   for name in ('better-favorites','launch.sh'):(app/name).write_bytes(('old '+name).encode())
   paths=['.tmp_update/bin/'+n for n in manage.prototype.HASHES]+['App/BetterFavorites/better-favorites','App/BetterFavorites/launch.sh']
   records={}
   for relative in paths:
    data=(root/relative).read_bytes();saved=backup/relative;saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes(data)
    records[relative]=dict(sha256=manage.prototype.digest(data),mode=0o700)
   manage.manage(root,'install',PAYLOAD)
   for name in ('better-favorites','launch.sh'):(app/name).write_bytes(('new '+name).encode())
   for relative in paths:records[relative]['deployed_sha256']=manage.prototype.digest((root/relative).read_bytes())
   (backup/'deployment.json').write_text(json.dumps(dict(package=manage.catalogue(),files=records)))
   first=root/paths[0]
   def interrupt(phase,p):
    if phase=='after' and p==first:raise KeyboardInterrupt('interrupted removal')
   with self.assertRaises(KeyboardInterrupt):manage.manage(root,'uninstall',hook=interrupt)
   protected={p:p.read_bytes() for p in app.iterdir() if p.name in ('settings.conf','browser-preferences.conf','home-entry.conf')}
   module=importlib.util.spec_from_file_location('rollback',ROOT/'tools/rollback-m6-device.py');rollback=importlib.util.module_from_spec(module);module.loader.exec_module(rollback)
   rollback.rollback(root,backup)
   for relative in paths:self.assertEqual((root/relative).read_bytes(),(backup/relative).read_bytes())
   for p,data in protected.items():self.assertEqual(p.read_bytes(),data)
 def test_package_runtime_and_backup_refusal(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=Path(tmp);self.seed(root);runtime=root/'.tmp_update/runtime.sh';runtime.write_text('unsupported')
   with self.assertRaisesRegex(RuntimeError,'runtime hash'):manage.manage(root,'install',PAYLOAD)
  with tempfile.TemporaryDirectory() as tmp:
   root=Path(tmp);self.seed(root);manage.manage(root,'install',PAYLOAD)
   manifest=json.loads((root/'.tmp_update/config/better-favorites-home.json').read_text())
   (root/manifest['backup']/'MainUI-354-clean').write_bytes(b'bad backup')
   before=(root/'.tmp_update/bin/MainUI-354-clean').read_bytes()
   with self.assertRaisesRegex(RuntimeError,'backup preserved'):manage.manage(root,'uninstall')
   self.assertEqual((root/'.tmp_update/bin/MainUI-354-clean').read_bytes(),before)
if __name__=='__main__':unittest.main()
