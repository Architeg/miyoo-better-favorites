#!/usr/bin/env python3
"""Run the real launcher with a child stub, isolated paths and no device libraries."""
from pathlib import Path
import tempfile, subprocess, os
repo=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='better-favorites-launcher-test-') as folder:
    app=Path(folder)
    active=app/'active-command'
    active.write_text('fixture app command')
    prepared=app/'prepared'
    subprocess.run(['python3',str(repo/'tools/manage-profiling.py'),'prepare','--output',str(prepared)],check=True,capture_output=True)
    (app/'profile-device-launch.sh').write_bytes((prepared/'profile-device-launch.sh').read_bytes())
    (app/'profile.enabled').write_text('BetterFavoritesProfilePilot1\n')
    (app/'Roms').mkdir();(app/'Roms/favourite.json').write_text('{}\n')
    (app/'proc').mkdir();(app/'proc/uptime').write_text('12.34 34.56\n')
    launcher=(prepared/'launch.sh').read_text()
    launcher=launcher.replace('ACTIVE=/mnt/SDCARD/.tmp_update/cmd_to_run.sh', 'ACTIVE="'+str(active)+'"')
    launcher=launcher.replace('LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so '+chr(92)+'\n','')
    (app/'launch.sh').write_text(launcher)
    stub=app/'better-favorites'
    stub.write_text("""#!/bin/sh
[ "$BETTER_FAVORITES_RETURN_DIR" = "$TEST_RETURN_DIR" ] || exit 97
printf '%s\\n' "$*" >> "$TEST_TRACE"
case "$1" in
    --rotate-log) : > "$BETTER_FAVORITES_LOG"; exit 0 ;;
    --log-event) printf '%s\\n' "$2" >> "$BETTER_FAVORITES_LOG"; exit 0 ;;
    --publish-handoff|--publish-switcher-handoff)
        [ -f "$TEST_CLEANUP" ] || exit 99
        exit "$TEST_PUBLISH_EXIT" ;;
    --cancel-handoff)
        rm -f "$2/app-command.sh" "$2/request.sh" "$2/recent.json" "$2/switcher.request"
        exit 0 ;;
esac
printf 'SDL/audio cleanup complete\\n' > "$TEST_CLEANUP"
exit "$TEST_BINARY_EXIT"
""")
    stub.chmod(0o700)
    (app/'home-diagnostics.conf').write_text('BetterFavoritesHomeDiagnostics1\n1\n')
    (app/'home-diagnostics.log').write_text('M6Home1 variant=MainUI-354-clean attempt=fixture event=publication reason=committed\n')
    for exitcode,publication,expected,operation in [(20,0,0,'--publish-handoff'),(21,0,0,'--publish-switcher-handoff'),(21,1,1,'--publish-switcher-handoff'),(0,0,0,None),(1,0,1,None),(139,0,139,None)]:
        if (app/'.profiling-results').exists():__import__('shutil').rmtree(app/'.profiling-results')
        trace=app/'trace'; cleanup=app/'cleanup'
        trace.unlink(missing_ok=True);cleanup.unlink(missing_ok=True)
        env=dict(os.environ,TEST_TRACE=str(trace),TEST_CLEANUP=str(cleanup),TEST_BINARY_EXIT=str(exitcode),TEST_PUBLISH_EXIT=str(publication),BETTER_FAVORITES_PROC_ROOT=str(app/'proc'),BETTER_FAVORITES_SD_ROOT=str(app),BETTER_FAVORITES_RETURN_DIR=str(app/'runtime-context'),TEST_RETURN_DIR=str(app/'runtime-context'))
        result=subprocess.run(['sh',str(app/'launch.sh')],env=env,capture_output=True,text=True)
        assert result.returncode==expected,(exitcode,result.stdout,result.stderr)
        diagnostic_log=(app/'better-favorites.log').read_text()
        assert 'event=entry candidate_attempt=fixture' in diagnostic_log,diagnostic_log
        assert 'event=exit committed=' in diagnostic_log,diagnostic_log
        assert (app/'.profiling-results/pilot-0001/startup.log').is_file()
        lines=trace.read_text().splitlines()
        publishers=[line.split()[0] for line in lines if line.startswith('--publish')]
        assert publishers==([operation] if operation else []),lines
        assert any(line.startswith('--cancel-handoff') for line in lines),lines
        for line in lines:
            if line.startswith('--cancel-handoff'): assert not Path(line.split()[1]).exists()
print('Launcher exit 20/21 dispatch after cleanup, B/abnormal exit and failed publication: PASS')
