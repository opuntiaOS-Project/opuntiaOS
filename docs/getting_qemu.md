<img src="https://raw.githubusercontent.com/opuntiaOS-Project/opuntiaOS/master/assets/logo/logo_512.png" width="20"> <b> DOCS</b></br></br>

# Getting QEMU

QEMU is an emulator used for running and debugging opuntiaOS. While *x86* and *arm32* targets use the regular QEMU binary, *arm64* target uses a custom QEMU board configuration.

## Getting QEMU for x86 and Arm32

### QEMU for MacOS

```bash
brew install qemu
```

### Tools for Linux (Ubuntu)

```bash
apt install qemu-utils qemu-system-i386 qemu-system-arm
```

## Compiling a custom QEMU for Arm64

```bash
git clone https://github.com/opuntiaOS-Project/qemu.git
cd qemu
./configure --target-list=aarch64-softmmu
make -j{cpus}
export ONEOS_QEMU_AA64=`pwd`/build/qemu-system-aarch64
```
