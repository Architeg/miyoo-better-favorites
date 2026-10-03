"""Private fixtures only: aggregation/fallback/refusal and short-lived launcher wrapper."""
from pathlib import Path
import tempfile,subprocess,os,json,threading
repo=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='better-favorites-profile-tests.') as directory:
    root=Path(directory);proc=root/'proc';proc.mkdir()
    (proc/'uptime').write_text('123.45 234.56\n');(proc/'meminfo').write_text('MemFree: 1024 kB\n')
    pid=proc/'123';pid.mkdir();(pid/'status').write_text('Name:\truntime\nPPid:\t1\nVmRSS:\t55 kB\n')
    (pid/'stat').write_text('123 (runtime (fixture)) S '+' '.join(['1']*18+['500'])+'\n');(pid/'comm').write_text('runtime\n')
    (pid/'smaps').write_text('Rss: 12 kB\nPss: 7 kB\nPrivate_Dirty: 3 kB\nRss: 5 kB\nPss: 2 kB\n')
    env=dict(os.environ,BETTER_FAVORITES_PROC_ROOT=str(proc))
    def sample(name):
        output=root/name
        subprocess.run(['sh',str(repo/'tools/sample-device-memory.sh'),'123',str(output)],env=env,check=True,capture_output=True)
        return output
    output=sample('smaps');assert 'smaps\n'==(output/'memory-source.txt').read_text()
    totals=(output/'memory-totals.txt').read_text();assert 'Pss: 9 kB' in totals and 'Rss: 17 kB' in totals
    (pid/'smaps_rollup').write_text('Rss: 20 kB\nPss: 11 kB\n');output=sample('rollup')
    assert (output/'memory-source.txt').read_text()=='smaps_rollup\n'
    (pid/'smaps_rollup').unlink();(pid/'smaps').unlink();output=sample('unavailable')
    assert 'unavailable' in (output/'memory-source.txt').read_text();assert not (output/'memory-totals.txt').exists()
    assert subprocess.run(['sh',str(repo/'tools/sample-device-memory.sh'),'123',str(output)],env=env,capture_output=True).returncode!=0
    assert subprocess.run(['sh',str(repo/'tools/sample-device-memory.sh'),'first',str(root/'bad')],env=env,capture_output=True).returncode!=0
    # FIFO rendezvous mutates identity after before-stat, during metric collection.
    baseline=(pid/'stat').read_text()
    invalid=[]
    for name,mutation in [('reused',lambda:(pid/'stat').write_text(baseline.replace('500','999'))),
                          ('exited',lambda:(pid/'stat').unlink())]:
        (pid/'stat').write_text(baseline)
        (proc/'meminfo').unlink();os.mkfifo(proc/'meminfo')
        def change():
            with (proc/'meminfo').open('w') as writer:
                mutation();writer.write('MemFree: 1024 kB\n')
        worker=threading.Thread(target=change);worker.start()
        output=root/name
        result=subprocess.run(['sh',str(repo/'tools/sample-device-memory.sh'),'123',str(output)],env=env,capture_output=True)
        worker.join(timeout=5);assert not worker.is_alive()
        assert result.returncode!=0
        validity=(output/'validity.txt').read_text();assert 'valid=0' in validity
        assert ('pid-identity-changed' if name=='reused' else 'process-exited-or-unreadable-after') in validity
        assert (output/'process-stat-before.txt').read_text()==baseline
        assert (output/'uptime.txt').is_file() and (output/'process-status.txt').is_file()
        invalid.append(output)
        (proc/'meminfo').unlink();(proc/'meminfo').write_text('MemFree: 1024 kB\n')
    (pid/'stat').write_text('malformed fixture\n')
    output=root/'malformed'
    result=subprocess.run(['sh',str(repo/'tools/sample-device-memory.sh'),'123',str(output)],env=env,capture_output=True)
    assert result.returncode!=0 and 'valid=0' in (output/'validity.txt').read_text();invalid.append(output)
    (pid/'stat').write_text(baseline)
    output=root/'zombie';(pid/'stat').write_text(baseline.replace(') S ',') Z '))
    result=subprocess.run(['sh',str(repo/'tools/sample-device-memory.sh'),'123',str(output)],env=env,capture_output=True)
    assert result.returncode!=0 and 'reason=process-exited' in (output/'validity.txt').read_text();invalid.append(output)
    (pid/'stat').write_text(baseline)
    report=subprocess.run(['python3',str(repo/'tools/summarize-memory-profile.py'),str(root/'smaps'),*map(str,invalid)],text=True,capture_output=True,check=True)
    assert 'valid_samples=1' in report.stdout and 'median=9' in report.stdout
    assert report.stderr.count('EXCLUDED')==4
    app=root/'app';app.mkdir();(app/'better-favorites').write_text('fixture');(app/'better-favorites').chmod(0o700)
    (app/'launch.sh').write_text('#!/bin/sh\nprintf "BF_PROFILE fixture\\n" > "$(dirname "$0")/better-favorites.log"\nexit 0\n')
    card=root/'card';(card/'.tmp_update/config').mkdir(parents=True);(card/'Roms').mkdir()
    (card/'.tmp_update/config/active_theme').write_text('fixture-theme\n');(card/'Roms/favourite.json').write_text('{}\n')
    env['BETTER_FAVORITES_SD_ROOT']=str(card)
    (app/'profile.enabled').write_text('BetterFavoritesProfilePilot1\n')
    # Hashing must not warm executable/corpus during startup preparation.
    hash_bin=root/'hash-bin';hash_bin.mkdir();trace=root/'hash-trace'
    hash_command=hash_bin/'sha256sum'
    hash_command.write_text('#!/bin/sh\nprintf "%s\\n" "$1" >> "$HASH_TRACE"\n')
    hash_command.chmod(0o700)
    env.update(PATH=str(hash_bin)+os.pathsep+env['PATH'],HASH_TRACE=str(trace))
    script='. "$HOOK"; bf_profile_begin; [ ! -s "$HASH_TRACE" ] || exit 99; printf "BF_PROFILE fixture\n" > "$LOG"; bf_profile_finish 0'
    env.update(APP_DIR=str(app),LOG=str(app/'better-favorites.log'),HOOK=str(repo/'tools/profile-device-launch.sh'))
    for run in range(1,6):
        trace.write_text('')
        subprocess.run(['sh','-c',script],env=env,check=True,capture_output=True)
        assert len(trace.read_text().splitlines())==(3 if run<=4 else 0)
    assert len(list((app/'.profiling-results').iterdir()))==4
    output=app/'.profiling-results/pilot-0001'
    assert (output/'startup.log').read_text()=='BF_PROFILE fixture\n'
    assert 'launcher_exit=0' in (output/'metadata.txt').read_text()
    assert 'hash_timing=after-binary-exit-and-launcher-cleanup' in (output/'metadata.txt').read_text()
    assert (card/'Roms/favourite.json').read_text()=='{}\n'
    # A fresh marker gets distinct bounded archives, preserving all four pilot runs.
    pilot_bytes={p:p.read_bytes() for p in (app/'.profiling-results').rglob('*') if p.is_file()}
    (app/'profile.enabled').write_text('BetterFavoritesProfileSession2\ncorrected-fixture\n')
    for run in range(1,6):
        trace.write_text('')
        subprocess.run(['sh','-c',script],env=env,check=True,capture_output=True)
        assert len(trace.read_text().splitlines())==(3 if run<=4 else 0)
    assert len(list((app/'.profiling-results/session-corrected-fixture').iterdir()))==4
    assert all(p.read_bytes()==data for p,data in pilot_bytes.items())
    # Finite Terminal observer: exact executable resolution, no first-PID fallback.
    observer_app=root/'observer-app';observer_app.mkdir()
    (observer_app/'sample-device-memory.sh').write_bytes((repo/'tools/sample-device-memory.sh').read_bytes())
    runtime=proc/'222';runtime.mkdir()
    for name in ('comm','status','stat'):(runtime/name).write_bytes((pid/name).read_bytes())
    (runtime/'comm').write_text('runtime.sh\n')
    (pid/'status').write_text('Name:\truntime\nPPid:\t222\nVmRSS:\t55 kB\n')
    (pid/'exe').symlink_to(observer_app/'better-favorites')
    observer_env=dict(env,BETTER_FAVORITES_APP_DIR=str(observer_app),BETTER_FAVORITES_SAMPLE_DELAY='0')
    collector=['sh',str(repo/'tools/profile-memory-session.sh'),'fixture','idle']
    subprocess.run(collector,env=observer_env,check=True,capture_output=True)
    evidence=observer_app/'.profiling-memory/fixture-idle'
    assert len(list(evidence.glob('browser-*')))==3 and len(list(evidence.glob('runtime-*')))==3
    assert subprocess.run(collector,env=observer_env,capture_output=True).returncode!=0
    ambiguous=proc/'333';ambiguous.mkdir()
    for name in ('comm','status','stat'):(ambiguous/name).write_bytes((pid/name).read_bytes())
    (ambiguous/'exe').symlink_to(observer_app/'better-favorites')
    result=subprocess.run(['sh',str(repo/'tools/profile-memory-session.sh'),'ambiguous','idle'],env=observer_env,capture_output=True)
    assert result.returncode!=0
    evidence=observer_app/'.profiling-memory/ambiguous-idle'
    assert 'target_candidates=2' in (evidence/'conditions.txt').read_text()
    assert not list(evidence.glob('browser-*')) and (evidence/'inventory-1.txt').is_file()
    records=[{'kind':'summary','outcome':'first_presented_frame','main_us':1000,'main_uptime_ms':123460,'frame_uptime_ms':123470},
             {'kind':'phase','name':'decode','calls':3,'total_us':21,'unique':2}]
    (output/'startup.log').write_text(''.join('BF_PROFILE '+json.dumps(record)+'\n' for record in records))
    summary=subprocess.check_output(['python3',str(repo/'tools/summarize-startup-profile.py'),str(output/'startup.log')],text=True)
    assert 'launch_to_main' in summary and 'launch_to_frame' in summary and 'median=       20' in summary
    records[0]['outcome']='incomplete_startup'
    (output/'startup.log').write_text(''.join('BF_PROFILE '+json.dumps(record)+'\n' for record in records))
    summary=subprocess.check_output(['python3',str(repo/'tools/summarize-startup-profile.py'),str(output/'startup.log')],text=True)
    assert 'launch_to_frame' not in summary
    # Successful phase distributions must exclude failures, including sequential runs.
    records=[]
    for outcome,duration in [('first_presented_frame',10),('incomplete_startup',9000),('presentation_error',8000),('first_presented_frame',20)]:
        records += [{'kind':'summary','outcome':outcome,'main_us':duration},
                    {'kind':'phase','name':'decode','calls':1,'total_us':duration,'unique':1}]
    (output/'startup.log').write_text(''.join('BF_PROFILE '+json.dumps(record)+'\n' for record in records))
    summary=subprocess.check_output(['python3',str(repo/'tools/summarize-startup-profile.py'),str(output/'startup.log')],text=True)
    success=[line for line in summary.splitlines() if line.startswith('first_presented_frame') and 'decode' in line and 'total_us' in line]
    assert len(success)==1 and 'n= 2' in success[0] and 'median=       15' in success[0]
    assert any(line.startswith('incomplete_startup') and 'median=     9000' in line for line in summary.splitlines())
    (output/'metadata.txt').write_text('launcher_exit=139\n')
    summary=subprocess.check_output(['python3',str(repo/'tools/summarize-startup-profile.py'),str(output/'startup.log')],text=True)
    assert 'launcher_failure_after_frame' in summary
    assert not any(line.startswith('first_presented_frame') for line in summary.splitlines())
print('Profiling tools: separate success/failure populations, PID reuse/exit/zombie invalidation, smaps fallbacks, exclusion and four-run hook: PASS')
