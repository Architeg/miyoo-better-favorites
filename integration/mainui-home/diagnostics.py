#!/usr/bin/env python3
"""Optional offline M6 diagnostics activation and immutable host collection."""
import argparse
import json
from pathlib import Path
import manage

ON=b'BetterFavoritesHomeDiagnostics1\n1\n'
NAMES=('home-diagnostics.log','better-favorites.log')

def run(root,action,output=None):
    manage.require_posix()
    root=root.absolute()
    manage.directory(root,root/'App/BetterFavorites')
    app=root/'App/BetterFavorites';marker=app/'home-diagnostics.conf'
    current=manage.snapshot(marker)
    if action=='collect':
        if output is None: raise RuntimeError('--output requires a fresh host archive directory')
        output=output.absolute()
        if root==output or root in output.parents: raise RuntimeError('Archive must be on the host, outside the card')
        output.mkdir(parents=True,exist_ok=False)
        records={}
        for name in NAMES+('home-entry.conf','home-integration.conf'):
            data=manage.snapshot(app/name)
            if data is None: records[name]='absent';continue
            path=output/name
            path.write_bytes(data)
            if manage.regular(path)!=data: raise RuntimeError('Archive copy verification failed')
            records[name]=dict(size=len(data),sha256=manage.prototype.digest(data))
        for name in ('better-favorites','launch.sh'):
            data=manage.snapshot(app/name)
            records[name]='absent' if data is None else dict(size=len(data),sha256=manage.prototype.digest(data),archived=False)
        (output/'collection.json').write_text(json.dumps(records,indent=2)+'\n')
        print(f'Verified evidence retained: {output}');return
    if current not in (None,ON): raise RuntimeError('Unrecognized diagnostic marker preserved')
    if action=='enable' and current is None:
        # Destination-filesystem temp + guarded atomic rename; no SD hard links.
        manage.transaction({marker:(None,ON,0o600)})
    elif action in ('disable','remove') and current==ON:
        manage.transaction({marker:(ON,None,0o600)})
    print(f'Diagnostics {action}; preferences, binaries, logs and runtime unchanged')

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action',choices=['enable','collect','disable','remove'])
    parser.add_argument('--sd-root',type=Path,required=True)
    parser.add_argument('--output',type=Path)
    parser.add_argument('--powered-off',action='store_true')
    args=parser.parse_args()
    if args.action!='collect' and not args.powered_off: parser.error('Mutations require --powered-off and no other card writer')
    try: run(args.sd_root,args.action,args.output)
    except (OSError,ValueError,RuntimeError) as error: parser.exit(1,str(error)+'\n')
