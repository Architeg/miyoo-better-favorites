#!/usr/bin/env python3
"""Mac/offline rollback of the first M6 deployment; no working MainUI is required."""
import argparse
import datetime
import hashlib
import json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'integration/mainui-home'))
import manage
import diagnostics

def rollback(root,backup):
    root,_=manage.card(root)
    meta=json.loads(manage.regular(backup/'deployment.json'))
    package=manage.catalogue()
    if meta['package']!=package:raise RuntimeError('Deployment catalogue differs; preserve files and use the matching reviewed package')
    expected={'.tmp_update/bin/'+n for n in manage.prototype.HASHES}|{'App/BetterFavoritesTest/better-favorites','App/BetterFavoritesTest/launch.sh'}
    if set(meta['files'])!=expected:raise RuntimeError('Unexpected backup file set')
    originals={}
    # Verify all six backups AND target hashes before any rollback writes.
    for relative,spec in meta['files'].items():
        saved=manage.regular(backup/relative)
        if manage.prototype.digest(saved)!=spec['sha256']:raise RuntimeError(f'Backup changed: {relative}')
        current=manage.prototype.digest(manage.regular(root/relative))
        if current not in (spec['sha256'],spec['deployed_sha256']):raise RuntimeError(f'Conflicting installed file preserved: {relative}')
        originals[relative]=saved
    stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S.%fZ')
    diagnostics.run(root,'collect',backup/('rollback-evidence-'+stamp))
    diagnostics.run(root,'disable')
    manifest=root/'.tmp_update/config/better-favorites-home.json'
    state=json.loads(manage.regular(manifest))
    if state['status'] in ('installed','prepared'):manage.manage(root,'uninstall')
    elif state['status']!='uninstalled':raise RuntimeError('Unknown integration state preserved')
    changes={}
    for relative in ('App/BetterFavoritesTest/better-favorites','App/BetterFavoritesTest/launch.sh'):
        path=root/relative;before=manage.regular(path)
        if before!=originals[relative]:changes[path]=(before,originals[relative],meta['files'][relative]['mode'])
    manage.transaction(changes,check=lambda:manage.card(root))
    for relative,data in originals.items():
        if manage.regular(root/relative)!=data:raise RuntimeError(f'Final rollback verification failed: {relative}')
    print('All six originals verified. Saved preferences, history and return integration retained. Run sync, then reboot.')
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--sd-root',type=Path,required=True);p.add_argument('--backup',type=Path,required=True);p.add_argument('--powered-off',action='store_true');a=p.parse_args()
    if not a.powered_off:p.error('Power off and mount the card first; --powered-off is required')
    try:rollback(a.sd_root,a.backup)
    except (OSError,ValueError,KeyError,RuntimeError) as error:p.exit(1,str(error)+'\n')
