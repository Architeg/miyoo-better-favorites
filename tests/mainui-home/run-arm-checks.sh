#!/bin/bash
# ONLY in an isolated host Docker toolchain container, without the card mounted.
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
out=${1:?directory of generated exact-binary host prototypes}
out=$(cd "$out" && pwd)
[ -f /.dockerenv ] || { echo 'Run only in an isolated Docker container.' >&2; exit 1; }
# Refuse a card/bind mount at or above the fixture root, or a symlink alias.
python3 - <<'PYGUARD'
from pathlib import Path
root = Path('/mnt/SDCARD').resolve()
assert str(root) == '/mnt/SDCARD', 'fixture root must not be a symlink alias'
for line in Path('/proc/self/mountinfo').read_text().splitlines():
    mounted = Path(line.split()[4])
    assert mounted == Path('/') or not (root == mounted or mounted in root.parents), 'mounted fixture root forbidden'
PYGUARD
command -v qemu-arm >/dev/null
source /root/setup-env.sh
arm-linux-gnueabihf-g++ -std=c++17 -O0 -marm -Wall -Wextra -Werror \
  -Wl,-Ttext-segment=0x400000,--export-dynamic \
  "$root/tests/mainui-home/arm_harness.cpp" "$root/tests/mainui-home/raw_syscall.S" \
  -ldl -o "$out/arm-harness"
mkdir -p /mnt/SDCARD/App/BetterFavoritesTest /mnt/SDCARD/miyoo/app
for variant in MainUI-283-clean MainUI-283-expert MainUI-354-clean MainUI-354-expert; do
  cp "$out/arm-harness" /mnt/SDCARD/App/BetterFavoritesTest/better-favorites
  touch /mnt/SDCARD/App/BetterFavoritesTest/launch.sh
  address=$(python3 -c 'import json,sys; print(hex(json.load(open(sys.argv[1]))["symbols"]["bf_syscall3"]))' "$out/$variant/byte-map.json")
  (cd /mnt/SDCARD/miyoo/app
   BF_MAINUI_HOME_TEST_ISOLATED=1 qemu-arm -L /opt/miyoomini-toolchain/arm-linux-gnueabihf/sysroot \
    "$out/arm-harness" "$out/$variant/$variant" "$address" "$out/$variant/generated-command.sh") \
    > "$out/$variant/arm-test.log" 2>&1
  chmod 700 "$out/$variant/$variant" # host fixture only: permit target loader inspection
  qemu-arm -E LD_TRACE_LOADED_OBJECTS=1 -L /opt/miyoomini-toolchain/arm-linux-gnueabihf/sysroot \
    "$out/$variant/$variant" > "$out/$variant/loader-trace.txt"
  echo "$variant: ARM native dispatch, failure and loader fixtures passed"
done
