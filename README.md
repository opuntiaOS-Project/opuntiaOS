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

## How to run
* Install dependencies (i686 cross-compiler, mkfs.ext2, fuse-ext2, qemu-i386) and setup right paths in `makefile.configs`
* Run `make format_ext2`. Run this only one time, to format your virtual drive
* Run `make && make sync && make run`

### Images
Progress: November 2020
![alt text](https://raw.githubusercontent.com/nimelehin/oneOS/master/images/progress_11_20.png)
