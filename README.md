# oneOS

## Features
### Kernel
* x86 (32-bit) kernel with pre-emptive multi-threading
* Ext2 FS
* Local sockets
* /dev and /proc filesystems
* POSIX signals
* TTY

### Libraries
* Custom LibC (in development)

### Userland
* Basic ls-like application
* Edit - a clone of vim

## How to run
* Install dependencies (i686 cross-compiler, mkfs.ext2, fuse-ext2, qemu-i386) and setup right paths in `makefile.configs`
* Run `make format_ext2`. Run this only one time, to format your virtual drive
* Run `make && make sync && make run`
