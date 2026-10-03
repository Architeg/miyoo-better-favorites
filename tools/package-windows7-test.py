#!/usr/bin/env python3
"""Fresh offline Desktop test archive; preserve every previous candidate."""
import argparse,hashlib,json,sys,tempfile,zipfile
from pathlib import Path
from importlib.machinery import SourceFileLoader
pack=SourceFileLoader('release_pack',str(Path(__file__).with_name('package-release.py'))).load_module()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--release',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
 if a.output.exists():raise SystemExit('Preserve previous test ZIP; choose a fresh destination')
 with tempfile.TemporaryDirectory(prefix='bf-win7-bundle-') as td:
  parent=Path(td);root=parent/'BetterFavorites-Test';root.mkdir()
  installer=next(a.release.glob('better-favorites-*-installer.zip'))
  with zipfile.ZipFile(installer) as z:
   for entry in z.infolist():
    if '..' in Path(entry.filename).parts or entry.filename.startswith('/'):raise SystemExit('Unsafe member')
   z.extractall(root)
  for line in (root/'SHA256SUMS').read_text().splitlines():
   digest,rel=line.split('  ',1)
   if hashlib.sha256((root/rel).read_bytes()).hexdigest()!=digest:raise SystemExit('Checksum mismatch '+rel)
  (root/'Install-Windows7.cmd').write_bytes(b'@echo off\r\ncall "%~dp0Install-Windows.cmd" %*\r\nexit /b %errorlevel%\r\n')
  (root/'TEST-WINDOWS7.txt').write_text('Better Favorites RC2 host compatibility test - NOT Windows execution acceptance.\nExtract with Explorer to C:\\BetterFavorites-Test. No shared folder/network/developer tools required.\nOpen bundled PowerShell there; one-line commands:\n.\\Install-Windows7.cmd install\n.\\Install-Windows7.cmd export-diagnostics\n.\\Install-Windows7.cmd uninstall\nThe alias uses the same native version/architecture dispatcher as Install-Windows.cmd.\nWindows7/8/8.1 select Go1.20.14 x86/x64; newer Windows select modern native executables.\nPower off Miyoo for mutations; prompts ask for local SD drive, optional patches, recovery if needed.\nUninstall restores/automatically verifies stock, archives recovery outside SD and removes owned app/data.\nUnknown/modified paths fail safely. Keep every printed recovery/archive.\nTest install/Home OFF-ON/game-return/export/uninstall/stock boot/reinstall.\nRecord Windows version/bitness, shell bitness, SD reader/filesystem, exact hashes and failures.\nNo Get-FileHash/Expand-Archive or PowerShell upgrade needed. Optional verification: certutil -hashfile FILE SHA256.\n')
  entries=[f for f in sorted(root.rglob('*')) if f.is_file() and f.name!='SHA256SUMS']
  (root/'SHA256SUMS').write_text(''.join(hashlib.sha256(f.read_bytes()).hexdigest()+'  '+f.relative_to(root).as_posix()+'\n' for f in entries))
  epoch=int(__import__('subprocess').check_output(['git','show','-s','--format=%ct','HEAD'],cwd=pack.ROOT))
  pack.zipdir(parent,a.output,epoch)
 digest=hashlib.sha256(a.output.read_bytes()).hexdigest();a.output.with_suffix(a.output.suffix+'.sha256').write_text(digest+'  '+a.output.name+'\n');print(digest+'  '+str(a.output))
if __name__=='__main__':main()
