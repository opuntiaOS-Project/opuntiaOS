# oneOS

## Features
### Kernel
* x86-32 and ARMv7 kernel with pre-emptive multi-threading
* Ext2 FS
* Local sockets
* /dev filesystem
* POSIX signals
* TTY

### Libraries
* Custom LibC (in development)
* Custom LibCXX
* LibFoundation
* LibG & LibUI

### Userland
* Basic userland
* Edit - a simple clone of vim
* Composing windows manager
* Simple gui-app "About"
* Terminal

## How to run
### Installing deps
#### MacOS
* `brew install coreutils qemu e2fsprogs nasm m4 autoconf libtool automake bash gcc@10`
* `brew install --cask osxfuse`
##### x86
* `brew install i686-elf-gcc`
##### arm
* `brew tap ArmMbed/homebrew-formulae`
* `brew install arm-none-eabi-gcc`
#### Linux
* `sudo apt install build-essential curl libmpfr-dev libmpc-dev libgmp-dev e2fsprogs qemu-system-i386 qemu-utils nasm fuseext2`
##### x86
* Build toolchain `./toolchains/scripts/i686-elf-tools.sh`
##### arm
* `apt install gcc-arm-none-eabi`

### Building the OS with GN
#### Generating ninja for x86
* Run `./gn_gen`. Run this only one time.
#### Generating ninja for ARM
* Run `./gn_gen target_cpu=\"aarch32\"`. Run this only one time.
#### Common steps
* `cd out`
* `./all.bash`

### Getting GN
https://gn.googlesource.com/gn/#getting-a-binary

### Images
Progress: February 2021 (preview)
![alt text](https://raw.githubusercontent.com/nimelehin/oneOS/master/images//progress_02_21_preview1.png)
