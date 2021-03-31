<img src="https://raw.githubusercontent.com/nimelehin/oneOS/master/assets/logo/logo_blue_512.png" width="20"> <b> DOCS</b></br></br>

# Builidng OS

## Prerequisites

The project uses GN Build System, please ensure that you have `gn` binary. If your system doesn't have the build system, please visit [instructions how to get a binary](https://github.com/nimelehin/oneOS/blob/master/docs/getting_gn.md)

### Tools for MacOS

```bash
brew install --cask osxfuse

brew install coreutils qemu e2fsprogs nasm m4 autoconf libtool automake bash gcc@10 ninja

# install fuse-ext2
```

`fuse-ext2` is not available as homebrew formula, download [sources](https://github.com/alperakcan/fuse-ext2) and compile it following [instructions](https://apple.stackexchange.com/questions/226981/how-do-i-install-fuse-ext2-to-use-with-osxfuse).

*Notes*:

* Make sure you have `xcode-tools` installed.

</br>

### Tools for Linux (Ubuntu)

```bash
apt install build-essential curl libmpfr-dev libmpc-dev libgmp-dev e2fsprogs qemu-system-i386 qemu-utils nasm fuseext2 ninja
```

</br>

## Cross-compiler

oneOS supports compilation both with GNU toolchains and LLVM. 

*Note* that in GNU world, every host/target combination has its own set of binaries, headers, libraries, etc. So, choose GNU Toolchain based on target architecture you want to compile the OS to.

### GNU Toolchain for MacOS

***x86***

```bash
brew install i686-elf-gcc
```

***Aarch32***

```bash
brew tap cesarvandevelde/homebrew-formulae
brew install cesarvandevelde/homebrew-formulae/arm-none-eabi-gcc
```

</br>

### LLVM Toolchain for MacOS

***Currently LLVM toolchain uses some GCC binutils. You must install GNU Toolchain to compile with LLVM.***

```bash
brew install llvm
```

</br>

### GNU Toolchain for Linux (Ubuntu)

***x86***

```bash
./toolchains/scripts/i686-elf-tools.sh
```

***Aarch32***

```bash
./toolchains/scripts/arm-none-eabi-tools.sh
```

</br>

### LLVM Toolchain for Linux (Ubuntu)

***Currently LLVM toolchain uses some GCC binutils. You must install GNU Toolchain to compile with LLVM.***

```bash
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 11
```

Learn more on <https://apt.llvm.org>

## Building OS

Now we are ready to build the OS :)

### Generating Ninja

To generate ninja just run `./gn_gen.sh`. This command creates build directory `out/` and disk image for the OS.

***Note*** you can set some env variables before running `./gn_gen.sh` to change some parameters, go to [Env varibles for Ninja]() to learn more.

**Options**

* --target_cpu *value*
  * Allows to choose target arch
  * Possible values:
    * x86 *(default)*
    * aarch32
* --host *value*
  * Choose toolchain to build the OS
  * Possible values:
    * gnu *(default)*
    * llvm
* --LLVM_BIN_PATH *value*
  * Provide path to LLVM bins
  * Possible values:
    * `path`

So to build oneOS for Arm with LLVM you have to generate Ninja files with `./gn_gen.sh --target_cpu aarch32 --host llvm`

### Building

Move to `out/` directory where the OS will be built. There are several scripts:

* `build.sh` - builds OS
* `sync.sh` - synchronise files with the disk image
* `run.sh` - launches QEMU

The right sequence to run the OS is to build, sync, launch or use `all.sh` which combine these 3 scripts.

### Debugging

There are several scripts which might help you to debug the OS:

* `debug.sh` - launches QEMU in debug mode. Debug analog of `run.sh`
* `dll.sh` - Debug analog of `all.sh`.

Also you can run `gdb` or `lldb` from the `out/` directory, which will automitically load kernel symobols and connect to QEMU in debug mode.
