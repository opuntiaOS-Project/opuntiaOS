<img src="https://raw.githubusercontent.com/opuntiaOS-Project/opuntiaOS/master/assets/logo/logo_512.png" width="20"> <b> DOCS</b></br></br>

# Bootloaders

The boot process of Prickly Pear Kernel could be divided into 2 parts: bootloader(prekernel) and kernel. These parts are tight together and share the same ABI to prepare the required environment for the kernel.
 
### Kernel boot requirements
 
* VM is on.
* Filled *boot args*.
* All *kernel data* required for a kernel bootup (e.g. the kernel itself, boot args and vm translation tables) should be a continuous region of memory (both physical and virtual). This region is described with `boot_args->vaddr`, `boot_args->paddr` and `boot_args->kernel_data_size`.
* After *kernel data* **4mb** should be mapped for a successful kernel boot process. This part is used for kernel structs (e.g physical allocation table) which are required before the kernel can set up its own vm translation tables.
* Initial debug devices should be mapped (UART and/or framebuffer).
* For 64-bit kernels all RAM should be mapped to itself.
 
### Prekernel
 
The kernel is an ELF file but there are some use cases when it is needed a raw binary image or an UEFI-capable image. In order to prepare the required environment for a kernel bootup there is a part called *prekernel*. It is a position independent raw binary which contains code to boot the kernel.

Raw image consists of prekernel, kernel and devtree.
 
### Bootloaders
 
An alternative to *prekernel* part is a standalone bootloader which could prepare the required environment for the kernel.

## Dirs

* [arm32](https://github.com/opuntiaOS-Project/opuntiaOS/tree/master/boot/arm32) Arm32 bootloader.
* [aarch64](https://github.com/opuntiaOS-Project/opuntiaOS/tree/master/boot/aarch64) Aarch64 bootloader.
* [libboot](https://github.com/opuntiaOS-Project/opuntiaOS/tree/master/boot/libboot) Bootloader library.
* [x86](https://github.com/opuntiaOS-Project/opuntiaOS/tree/master/boot/x86) x86 bootloader.
