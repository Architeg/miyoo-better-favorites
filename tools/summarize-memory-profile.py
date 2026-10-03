#!/usr/bin/env python3
"""Compare only identity-verified one-shot samples, one condition/role per invocation."""
import argparse,statistics,sys
from collections import defaultdict
from pathlib import Path
parser=argparse.ArgumentParser()
parser.add_argument('samples',nargs='+',type=Path)
args=parser.parse_args()
values=defaultdict(list)
accepted=0
for directory in args.samples:
    try:
        validity=dict(line.split('=',1) for line in (directory/'validity.txt').read_text().splitlines())
        before=validity.get('starttime_before','')
        if validity.get('valid')!='1' or not before.isdecimal() or before!=validity.get('starttime_after'):
            raise ValueError(validity.get('reason','unverified identity'))
        observed=defaultdict(list)
        for line in (directory/'process-status.txt').read_text().splitlines():
            if line.startswith(('VmRSS:','VmSize:')):
                key,value,*_=line.split();observed[key].append(int(value))
        totals=directory/'memory-totals.txt'
        if totals.exists():
            for line in totals.read_text().splitlines():
                key,value,*_=line.split();observed[key].append(int(value))
        for key,items in observed.items():values[key].extend(items)
        accepted+=1
    except (OSError,ValueError) as error:
        print(f'EXCLUDED {directory}: {error}',file=sys.stderr)
print(f'valid_samples={accepted} (missing metrics are unavailable, never zero)')
for key,items in sorted(values.items()):
    print(f'{key:16} n={len(items)} median={statistics.median(items):g} min={min(items)} max={max(items)} kB')
