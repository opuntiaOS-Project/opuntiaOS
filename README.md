# oneOS

## Features
### Kernel
* x86 (32-bit) kernel with pre-emptive multi-threading
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
* Basic ls-like application
* Edit - a clone of vim
* Composing windows manager
* Simple gui-app "About"
* Terminal

## How to run
### Installing deps
#### MacOS
* `brew install coreutils qemu e2fsprogs nasm m4 autoconf libtool automake bash gcc@10`
* `brew install --cask osxfuse`
* `brew install i686-elf-gcc`
#### Linux
* `sudo apt install build-essential curl libmpfr-dev libmpc-dev libgmp-dev e2fsprogs qemu-system-i386 qemu-utils nasm fuseext2`
* Build toolchain `./toolchains/scripts/i686-elf-tools.sh`

### Building the OS with GN
* Run `./gn_gen`. Run this only one time.
* `cd out`
* `./all.bash`
### Building the OS with Make
* Run `make format_ext2`. Run this only one time, to format your virtual drive
* Run `make && make sync && make run`

### Getting GN
https://gn.googlesource.com/gn/#getting-a-binary


### Images
Progress: November 2020
![alt text](https://raw.githubusercontent.com/nimelehin/oneOS/master/images/progress_11_20.png)
