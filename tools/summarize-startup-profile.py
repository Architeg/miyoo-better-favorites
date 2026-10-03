#!/usr/bin/env python3
"""Keep successful, failed, incomplete and unattributed startup populations separate."""
import argparse,json,statistics
from collections import defaultdict
from pathlib import Path
parser=argparse.ArgumentParser()
parser.add_argument('logs',nargs='+',type=Path)
args=parser.parse_args()
values=defaultdict(list)
for path in args.logs:
    outcome='unattributed' # Orphan/truncated phase lines never become successes.
    launch_ms=None
    launcher_failed=False
    metadata=path.parent/'metadata.txt'
    if metadata.is_file():
        for line in metadata.read_text().splitlines():
            if line.startswith('launcher_exit='):
                launcher_failed=line.split('=',1)[1]!='0'
            if line.startswith('launch_uptime_seconds='):
                launch_ms=float(line.split('=',1)[1])*1000
    for line in path.read_text().splitlines():
        if not line.startswith('BF_PROFILE '):continue
        record=json.loads(line[len('BF_PROFILE '):])
        if record['kind']=='summary':
            outcome=record['outcome']
            if launcher_failed and outcome=='first_presented_frame':outcome='launcher_failure_after_frame'
            values[(outcome,'startup','main_us')].append(record['main_us'])
            if launch_ms is not None:
                for key in ('main_uptime_ms','frame_uptime_ms'):
                    timestamp=record.get(key,-1)
                    if key=='frame_uptime_ms' and outcome!='first_presented_frame':continue
                    if timestamp>=launch_ms:
                        values[(outcome,'launch_to_'+key.replace('_uptime_ms',''),'elapsed_ms')].append(timestamp-launch_ms)
        elif record['kind']=='phase':
            for key in ('total_us','calls','unique'):
                values[(outcome,record['name'],key)].append(record[key])
for (outcome,phase,metric),items in sorted(values.items()):
    print(f'{outcome:24} {phase:32} {metric:9} n={len(items):2} median={statistics.median(items):9g} min={min(items):9g} max={max(items):9g}')
