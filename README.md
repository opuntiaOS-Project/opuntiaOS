<p align="center">
<img src="https://raw.githubusercontent.com/nimelehin/oneOS/master/assets/banner/banner_opac.png" width="300">
</p>

## Screenshot
Progress: March 2021
![alt text](https://raw.githubusercontent.com/nimelehin/oneOS/master/assets/images/progress_21_03.png)

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
|  | MacOS | Linux |
| ------------- | ------------- | ------------- |
| common  | `brew install --cask osxfuse`<br><br>`brew install coreutils qemu e2fsprogs nasm m4 autoconf libtool automake bash gcc@10 ninja`  | `apt install build-essential curl libmpfr-dev libmpc-dev libgmp-dev e2fsprogs qemu-system-i386 qemu-utils nasm fuseext2 ninja` |
| building x86  | `brew install i686-elf-gcc`  | `./toolchains/scripts/i686-elf-tools.sh` |
| building arm  | `brew tap ArmMbed/homebrew-formulae`<br>`brew install arm-none-eabi-gcc` | `apt install gcc-arm-none-eabi` |

### Building the OS with GN
Note: If you don't have a GN Build system, [learn more how to get it](https://github.com/nimelehin/oneOS#getting-gn)
#### Generating ninja
| building x86 | building arm |
| ------------- | ------------- |
| `./gn_gen` | `./gn_gen --target_cpu aarch32` |
#### Common steps
* `cd out`
* `./all.bash`

### Getting GN
https://gn.googlesource.com/gn/#getting-a-binary
#### Pre-compiled binaries
| MacOS | Linux |
| ------------- | ------------- |
| [amd64](https://chrome-infra-packages.appspot.com/dl/gn/gn/mac-amd64/+/latest)<br>[arm64](https://chrome-infra-packages.appspot.com/dl/gn/gn/mac-arm64/+/latest) | [amd64](https://chrome-infra-packages.appspot.com/dl/gn/gn/linux-amd64/+/latest)<br>[arm64](https://chrome-infra-packages.appspot.com/dl/gn/gn/linux-arm64/+/latest) |
