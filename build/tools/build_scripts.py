# Copyright 2021 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
import os
QEMU_PATH_VAR = "qemu_exec"
QEMU_PATH_ENV_VAR = ""
QEMU_SMP_VAR = "qemu_smp"
QEMU_SMP_ENV_VAR = "ONEOS_QEMU_SMP"
QEMU_STD_PATH = ""
qemu_run_cmd = ""
arch = sys.argv[1]
base = sys.argv[2]
out = sys.argv[3]

if arch == "x86":
    QEMU_PATH_ENV_VAR = "ONEOS_QEMU_X86"
    QEMU_STD_PATH = "qemu-system-i386"
    qemu_run_cmd = "${2} -m 256M --drive file={1}/os-image.bin,format=raw,index=0,if=floppy -device piix3-ide,id=ide -drive id=disk,format=raw,file={1}/one.img,if=none -device ide-hd,drive=disk,bus=ide.0 -serial mon:stdio -rtc base=utc -vga std".format(
        base, out, QEMU_PATH_VAR)
if arch == "aarch32":
    QEMU_PATH_ENV_VAR = "ONEOS_QEMU_ARM"
    QEMU_STD_PATH = "qemu-system-arm"
    qemu_run_cmd = "${2} -M vexpress-a15 -cpu cortex-a15 -kernel {1}/base/boot/kernel.bin  -smp ${3} -serial mon:stdio -vga std -drive id=disk,if=sd,format=raw,file={1}/one.img".format(
        base, out, QEMU_PATH_VAR, QEMU_SMP_VAR)

if base[-1] == '/':
    base = base[:-1]

if out[-1] == '/':
    out = out[:-1]

sync = open("{0}/sync.sh".format(out), "w")
sync.write(
    """#!/bin/bash
GREEN='\\033[0;32m'
RED='\\033[0;31m'
NC='\\033[0m'
ERROR="${{RED}}[ERROR]${{NC}}"
SUCCESS="${{GREEN}}[SUCCESS]${{NC}}"

mkdir -p {0}/mountpoint
sudo fuse-ext2 {1}/one.img {0}/mountpoint -o rw+
if [ $? -ne 0 ]; then echo -e "${{ERROR}} Can't mount one.img to {0}/mountpoint" && exit 1; fi
sudo mkdir -p {0}/mountpoint/boot
sudo mkdir -p {0}/mountpoint/proc
sudo mkdir -p {0}/mountpoint/dev
sudo mkdir -p {0}/mountpoint/tmp
sudo cp -r {0}/base/* {0}/mountpoint/
sudo cp -r {1}/base/* {0}/mountpoint/

sudo chmod -R 644 {0}/mountpoint/proc
sudo chmod -R 644 {0}/mountpoint/dev
sudo chmod -R 666 {0}/mountpoint/tmp
sudo chmod -R 755 {0}/mountpoint/bin
sudo chmod -R 700 {0}/mountpoint/home
sudo chmod 777 {0}/mountpoint/home
sudo chmod -R 755 {0}/mountpoint/System
sudo chmod -R 755 {0}/mountpoint/Applications

sudo chown -R 0 {0}/mountpoint/home/root
sudo chown -R 0 {0}/mountpoint/bin/sudo
sudo chmod 4755 {0}/mountpoint/bin/sudo

sudo chown -R 10 {0}/mountpoint/home/user

sudo umount {0}/mountpoint
if [ $? -ne 0 ]; then echo -e "${{ERROR}} Can't umount {0}/mountpoint" && exit 1; fi
echo -e "${{SUCCESS}} Sync"
""".format(base, out))
sync.close()


build = open("{0}/build.sh".format(out), "w")
build.write(
    """#!/bin/bash
GREEN='\\033[0;32m'
RED='\\033[0;31m'
NC='\\033[0m'
ERROR="${{RED}}[ERROR]${{NC}}"
SUCCESS="${{GREEN}}[SUCCESS]${{NC}}"

ninja
if [ $? -ne 0 ]; then echo -e "${{ERROR}} Can't build for arch: {0}" && exit 1; fi
echo -e "${{SUCCESS}} Build for arch: {0}"
""".format(arch))
build.close()

run = open("{0}/run.sh".format(out), "w")
run.write(
    """#!/bin/bash
{2}="{3}"
{4}=1
[[ -z "${1}" ]] && {2}='{3}' || {2}="${1}"
[[ -z "${5}" ]] && {4}=1 || {4}="${5}"
{0}
if [ $? -ne 0 ]; then echo -e "${{ERROR}} Run command failed" && exit 1; fi""".format(qemu_run_cmd, QEMU_PATH_ENV_VAR, QEMU_PATH_VAR, QEMU_STD_PATH, QEMU_SMP_VAR, QEMU_SMP_ENV_VAR)
)
run.close()

debug = open("{0}/debug.sh".format(out), "w")
debug.write(
    """#!/bin/bash
{2}="{3}"
{4}=1
[[ -z "${1}" ]] && {2}='{3}' || {2}="${1}"
[[ -z "${5}" ]] && {4}=1 || {4}="${5}"
{0} -s -S
if [ $? -ne 0 ]; then echo -e "${{ERROR}} Debug Run command failed" && exit 1; fi""".format(qemu_run_cmd, QEMU_PATH_ENV_VAR, QEMU_PATH_VAR, QEMU_STD_PATH, QEMU_SMP_VAR, QEMU_SMP_ENV_VAR)
)
debug.close()

allf = open("{0}/all.sh".format(out), "w")
allf.write(
    """#!/bin/bash
GREEN='\\033[0;32m'
RED='\\033[0;31m'
NC='\\033[0m'
ERROR="${RED}[ERROR]${NC}"
SUCCESS="${GREEN}[SUCCESS]${NC}"

./build.sh
if [ $? -ne 0 ]; then echo -e "${ERROR} All command failed" && exit 1; fi
./sync.sh
if [ $? -ne 0 ]; then echo -e "${ERROR} All command failed" && exit 1; fi
./run.sh
if [ $? -ne 0 ]; then echo -e "${ERROR} All command failed" && exit 1; fi
""")
allf.close()

allf = open("{0}/run_tester.sh".format(out), "w")
allf.write(
    """#!/bin/bash
GREEN='\\033[0;32m'
RED='\\033[0;31m'
NC='\\033[0m'
ERROR="${{RED}}[ERROR]${{NC}}"
SUCCESS="${{GREEN}}[SUCCESS]${{NC}}"

./build.sh
if [ $? -ne 0 ]; then echo -e "${{ERROR}} All command failed" && exit 1; fi
./sync.sh
if [ $? -ne 0 ]; then echo -e "${{ERROR}} All command failed" && exit 1; fi
{2}="{3}"
[[ -z "${1}" ]] && {2}='{3}' || {2}="${1}"
{0} --nographic
if [ $? -ne 0 ]; then echo -e "${{ERROR}} All command failed" && exit 1; fi
""".format(qemu_run_cmd, QEMU_PATH_ENV_VAR, QEMU_PATH_VAR, QEMU_STD_PATH))
allf.close()

allf = open("{0}/dll.sh".format(out), "w")
allf.write(
    """#!/bin/bash
GREEN='\\033[0;32m'
RED='\\033[0;31m'
NC='\\033[0m'
ERROR="${RED}[ERROR]${NC}"
SUCCESS="${GREEN}[SUCCESS]${NC}"

./build.sh
if [ $? -ne 0 ]; then echo -e "${ERROR} Debug All command failed" && exit 1; fi
./sync.sh
if [ $? -ne 0 ]; then echo -e "${ERROR} Debug All command failed" && exit 1; fi
./debug.sh
if [ $? -ne 0 ]; then echo -e "${ERROR} Debug All command failed" && exit 1; fi
""")
allf.close()

gdbinit = open("{0}/.gdbinit".format(out), "w")
gdbinit.write(
    """file {0}/base/boot/kernel.bin
target remote :1234""".format(out))
gdbinit.close()
