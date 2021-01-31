#!/bin/bash
gn gen out
ninja -C out scripts
chmod +x out/build.sh
chmod +x out/run.sh
chmod +x out/sync.sh
chmod +x out/all.sh

#building one.img
IMAGE_SIZE=16M
qemu-img create -f raw out/one.img $IMAGE_SIZE
MKFS="" # Provide path here
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    MKFS=mkfs
elif [[ "$OSTYPE" == "darwin"* ]]; then
    MKFS=/usr/local/opt/e2fsprogs/sbin/mkfs.ext2
else
    echo "Please provide path to MKFS in gn_gen.sh"
fi
sudo $MKFS -t ext2 -r 0 -b 1024 out/one.img