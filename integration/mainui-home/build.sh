#!/bin/sh
# Execute inside the existing Docker toolchain; outputs must be host build files.
set -eu
out=${1:?output directory required}
mkdir -p "$out"
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
CXX=${CROSS_COMPILE:-arm-linux-gnueabihf-}g++
CC=${CROSS_COMPILE:-arm-linux-gnueabihf-}gcc
flags='-Os -marm -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=hard -ffreestanding -fno-rtti -fno-stack-protector -fno-pic -fno-pie -ffunction-sections -fdata-sections -Wall -Wextra -Werror'
$CXX $flags -fexceptions -funwind-tables -c "$root/integration/mainui-home/adapter.cpp" -o "$out/adapter.o"
$CC -marm -c "$root/integration/mainui-home/hooks.S" -o "$out/hooks.o"
$CXX -nostdlib -no-pie -Wl,--build-id=none,-T,"$root/integration/mainui-home/adapter.ld",-Map,"$out/adapter.map" "$out/adapter.o" "$out/hooks.o" -o "$out/adapter.elf"
${CROSS_COMPILE:-arm-linux-gnueabihf-}readelf -lSWu "$out/adapter.elf" > "$out/adapter-layout.txt"
${CROSS_COMPILE:-arm-linux-gnueabihf-}objdump -d "$out/adapter.elf" > "$out/adapter-disassembly.txt"
