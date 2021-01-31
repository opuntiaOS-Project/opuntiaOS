# Copyright 2021 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

QEMU_PATH = "/Users/nikitamelehin/Develop/qemu-4.2.1/i386-softmmu/qemu-system-i386"

import sys
arch = sys.argv[1]
base = sys.argv[2]
out = sys.argv[3]

if base[-1] == '/':
    base = base[:-1]

if out[-1] == '/':
    out = out[:-1]

sync = open("{0}/sync.sh".format(out), "w")
sync.write(
"""#!/bin/bash
sudo mkdir -p {0}/base/dev
sudo mkdir -p {0}/base/proc

sudo mkdir -p {0}/mountpoint
sudo fuse-ext2 {1}/one.img {0}/mountpoint -o rw+
sudo mkdir -p {0}/mountpoint/boot
sudo mkdir -p {0}/mountpoint/proc
sudo cp -r {0}/base/* {0}/mountpoint/
sudo cp -r {1}/base/* {0}/mountpoint/
sudo umount {0}/mountpoint""".format(base, out)) 
sync.close()


build = open("{0}/build.sh".format(out), "w")
build.write(
"""#!/bin/bash
ninja""") 
build.close()

run = open("{0}/run.sh".format(out), "w")
run.write(
"""#!/bin/bash
{2} -m 256M -fda {1}/os-image.bin -device piix3-ide,id=ide -drive id=disk,file={1}/one.img,if=none -device ide-drive,drive=disk,bus=ide.0 -serial mon:stdio -rtc base=utc -vga std""".format(base, out, QEMU_PATH)) 
run.close()

allf = open("{0}/all.sh".format(out), "w")
allf.write(
"""#!/bin/bash
./build.sh
./sync.sh
./run.sh
""") 
allf.close()