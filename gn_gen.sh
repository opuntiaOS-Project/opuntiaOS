#!/bin/bash

# pass --target_cpu arm32 as arg to build for arm

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'
ERROR="${RED}[ERROR]${NC}"
SUCCESS="${GREEN}[SUCCESS]${NC}"

for arg in "$@"; do
    if [[ $arg == "--help" ]]; then
        echo "--target_cpu
  Sets target arch
  Possible values:
    x86 (default)
    x86_64
    arm32 / arm
    arm64 / aarch64

--host
  Sets toolchain to build the OS
  Possible values:
    gnu (default)
    llvm

--device_type
  Configueres OS parts to work in either desktop or mobile mode.
  Possible values:
    d / desktop (default)
    m / mobile

--test_method
  Possible values:
    none (default)
    tests
    bench

--target_board
    Possible values (arm64):
        qemu-virt (default)
        apl

--help
  Prints all options of ./gn_gen.sh"
        exit
    fi
done

argline=""
for arg in "$@"; do
    if [[ $arg == "--"* ]]; then
        argline+="${arg:2}="
    else
        argline+="\"$arg\""
    fi
done

gn_args="--args='$argline'"
gn gen out --args=$argline
if [ $? -ne 0 ]; then echo -e "${ERROR} Can't do gn gen" && exit 1; fi
ninja -C out scripts
if [ $? -ne 0 ]; then echo -e "${ERROR} Can't do ninja -C out scripts" && exit 1; fi
chmod +x out/build.sh
chmod +x out/run.sh
chmod +x out/sync.sh
chmod +x out/all.sh
chmod +x out/run_tester.sh
chmod +x out/debug.sh
chmod +x out/dll.sh

#building one.img
IMAGE_SIZE=64M
qemu-img create -f raw out/one.img $IMAGE_SIZE
if [ $? -ne 0 ]; then echo -e "${ERROR} Can't create an out/one.img" && exit 1; fi
MKFS="" # Provide path here
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    MKFS=mkfs
elif [[ "$OSTYPE" == "darwin"* ]]; then
    MKFS=/usr/local/opt/e2fsprogs/sbin/mkfs.ext2
else
    echo "Please provide path to MKFS in gn_gen.sh"
fi
$MKFS -t ext2 -r 0 -b 1024 out/one.img
if [ $? -ne 0 ]; then echo -e "${ERROR} Can't create an out/one.img" && exit 1; fi
echo -e "${SUCCESS} Generated files with args: $*"
