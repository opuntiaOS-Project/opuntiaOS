<img src="https://raw.githubusercontent.com/opuntiaOS-Project/opuntiaOS/master/assets/logo/logo_512.png" width="20"> <b> DOCS</b></br></br>

# Builidng OS

## Prerequisites

The project uses GN Build System, please ensure that you have `gn` binary. If your system doesn't have the build system, please visit [instructions how to get a GN binary](https://github.com/opuntiaOS-Project/opuntiaOS/blob/master/docs/getting_gn.md)

### Getting QEMU

The project uses QEMU as a primary build and test target. Please follow [instructions how to get a QEMU binary](https://github.com/opuntiaOS-Project/opuntiaOS/blob/master/docs/getting_qemu.md).

*Note:* even if you have a `qemu-system-aarch64` installed and you are going to build for *arm64*, please follow the link to compile a custom version QEMU as it is required for this target.

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

opuntiaOS could be compiled both with GNU and LLVM toochains.

*Note* that in GNU world, every host/target combination has its own set of binaries, headers, libraries, etc. So, choose GNU Toolchain based on target architecture you want to compile the OS to.

### GNU Toolchain for MacOS

***x86***

```bash
brew install i686-elf-gcc
```

***x86-64***

```bash
brew install x86_64-elf-gcc
```

***Arm32***

```bash
brew tap opuntiaOS-Project/homebrew-formulae-arm-gcc
brew install opuntiaOS-Project/homebrew-formulae-arm-gcc/arm-none-eabi-gcc
```

***Arm64***

```bash
brew install aarch64-elf-gcc
```

</br>

### LLVM Toolchain for MacOS

```bash
brew install llvm
```

</br>

### GNU Toolchain for Linux (Ubuntu)

***x86***

```bash
./toolchains/scripts/i686-elf-tools.sh
```

***Arm32***

```bash
./toolchains/scripts/arm-none-eabi-tools.sh
```

</br>

### LLVM Toolchain for Linux (Ubuntu)

```bash
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 13
```

Learn more on <https://apt.llvm.org>

### Python Dependencies

```bash
pip intall -r utils/python_requirements.txt
```

## Building OS

Now we are ready to build the OS :)

### Generating Ninja

To generate ninja just run `./gn_gen.sh`. This command creates build directory `out/` and disk image for the OS.

***Note*** you can set some environment variables before running `./gn_gen.sh` to change some parameters, visit [Environment varibles](https://github.com/opuntiaOS-Project/opuntiaOS/blob/master/docs/build.md#environment-variables) to learn more.

#### **Options of ./gn_gen.sh**

* --target_arch|--target_cpu(deprecated) *value*
  * Sets target arch.
  * Possible values:
    * x86 *(default)*
    * x86_64
    * arm32 / arm
    * arm64 / aarch64
* --host *value*
  * Sets toolchain to build the OS.
  * Possible values:
    * gnu *(default)*
    * llvm
* --device_type *value*
  * Configueres OS parts to work in either desktop or mobile mode.
  * Possible values:
    * d / desktop *(default)*
    * m / mobile
* --test_method *value*
  * Possible values:
    * none *(default)*
    * tests
    * bench
* --target_board *value*
  * See "Available targets" table below.
* --dir *value*
  * Creates the build directory with the given name.
* -y|--yes
  * Force yes all operations.
* -h|--help
  * Prints all options of ./gn_gen.sh.

So to build opuntiaOS for Arm with LLVM you have to generate Ninja files with `./gn_gen.sh --target_arch arm32 --host llvm`

#### **Environment variables**

Another option how to configure the project is environment variables.

* `ONEOS_QEMU_SMP`
  * CPU cores count in the system.
* `ONEOS_QEMU_X86`
  * Path to *qemu-system-i386* executable
* `ONEOS_QEMU_X86_64`
  * Path to *qemu-system-x86_64* executable
* `ONEOS_QEMU_ARM`
  * Path to *qemu-system-arm* executable
* `ONEOS_QEMU_AA64`
  * Path to *qemu-system-aarch64* executable
* `LLVM_BIN_PATH`  *(Only with --host llvm)*
  * ***Must be set before `./gn_gen.sh`***
  * Path to LLVM bins.

</br>

### Building

Move to `out/` directory where the OS will be built. There are several scripts:

* `build.sh` - builds OS
* `sync.sh` - synchronise files with the disk image
* `run.sh` - launches QEMU or a real device

The right sequence to run the OS is to build, sync, launch or use `all.sh` which combine these 3 scripts.

### Debugging

There are several scripts which might help you to debug the OS:

* `debug.sh` - launches QEMU in debug mode. Debug analog of `run.sh` (available only when run.sh uses QEMU)
* `dll.sh` - Debug analog of `all.sh`.

Also you can run `gdb` or `lldb` from the `out/` directory, which will automitically load kernel symobols and connect to QEMU in debug mode.

### Available boards

***x86***

* `i386` - regular x86 (i386). (default)

***x86_64***

* `x86_64` - regular x86_64. (default)

***Arm32***

* `vexpress-a15` - QEMU's vexpress-a15 target. (default)


***Arm64***

* `qemu_opun` - QEMU's opuntia target, see [Getting QEMU page](https://github.com/opuntiaOS-Project/opuntiaOS/blob/master/docs/getting_qemu.md). (default)
* `apl` - i-device, see [Target Apl page](https://github.com/opuntiaOS-Project/opuntiaOS/blob/master/docs/target_apl.md).