#!/usr/bin/env python3
"""Probe installer operations in a NEW private directory on the candidate mount.

No existing files are read/replaced. This checks operations, not power-loss atomicity
or full WSL/FAT qualification. Native Windows Python is explicitly unsupported.
"""
import argparse
import json
import os
from pathlib import Path
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'integration/mainui-home'))
import manage


def probe(parent):
    manage.require_posix()
    parent=parent.absolute();manage.directory(parent,parent)
    with tempfile.TemporaryDirectory(prefix='bf-m6-filesystem-check-',dir=parent) as folder:
        root=Path(folder);a,b=root/'a',root/'b'
        for p,data in ((a,b'original-a'),(b,b'original-b')):
            temp=manage.stage(p,data,0o700);os.replace(temp,p)
            if manage.regular(p)!=data or not os.access(p,os.X_OK):
                raise RuntimeError('Byte/execute permission check failed')
        before={a:manage.regular(a),b:manage.regular(b)}
        changes={a:(before[a],b'new-a',0o700),b:(before[b],b'new-b',0o700)}
        def fail(phase,path):
            if phase=='after' and path==b:raise OSError('probe interruption')
        try:manage.transaction(changes,hook=fail)
        except RuntimeError:pass
        else:raise RuntimeError('Failure injection did not run')
        if any(manage.regular(p)!=data for p,data in before.items()):raise RuntimeError('Rollback check failed')
        def foreign(phase,path):
            if phase=='after' and path==b:
                a.write_bytes(b'foreign-fixture');raise OSError('probe foreign change')
        try:manage.transaction(changes,hook=foreign)
        except RuntimeError:pass
        else:raise RuntimeError('Conflict injection did not run')
        if manage.regular(a)!=b'foreign-fixture' or manage.regular(b)!=before[b]:raise RuntimeError('Foreign preservation check failed')
        # Own disposable fixture only, not an actual Onion command or user file.
        a.write_bytes(before[a]);manage.transaction(changes)
        if any(manage.regular(p)!=changes[p][1] for p in changes):raise RuntimeError('Publication check failed')
        return dict(platform=sys.platform,python=sys.version.split()[0],mount_parent=str(parent),
                    operations='regular descriptor read, O_NOFOLLOW/O_NONBLOCK, fchmod/execute, fsync, same-filesystem replace, readback, rollback, foreign preservation',
                    result='PASS',limits='No crash/power-loss or physical Windows/WSL/FAT qualification')


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--scratch-parent',type=Path,required=True)
    p.add_argument('--allow-test-write',action='store_true',help='authorize a fresh disposable directory only')
    a=p.parse_args()
    if not a.allow_test_write:p.error('Probe requires --allow-test-write; power off card and close other writers')
    try:print(json.dumps(probe(a.scratch_parent),indent=2))
    except (OSError,RuntimeError) as error:p.exit(1,str(error)+'\n')
