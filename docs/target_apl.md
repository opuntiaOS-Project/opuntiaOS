<img src="https://raw.githubusercontent.com/opuntiaOS-Project/opuntiaOS/master/assets/logo/logo_512.png" width="20"> <b> DOCS</b></br></br>

# Target Apl

## Introduction

opuntiaOS gets support for `apl` target allowing to run the OS on a real i-device.

## How to boot

* Step 1: The process of compiling the OS is similar to other targets. You have to provide `apl` as a `target_board` to GN Build System. More information you can find [on this page](https://github.com/opuntiaOS-Project/opuntiaOS/blob/master/docs/build.md).

* Step 2: To get access to the bootloader you have to jailbreak your i-device. The checkra1n environment is used. You can get the binary at [http://checkra.in/](http://checkra.in/).

* Step 3: After configuring GN you have to provide path to the checkra1n binary with a $CHECKRAIN env variable.

* Step 4: Run `./all.sh` from `out/` dir. This will compile the OS, prepare ramdisk and compile a custom version of pongoOS which is used as a bootloader.


Note: to turn off opuntiaOS, [force reset](https://support.apple.com/en-gb/guide/iphone/iph8903c3ee6/15.0/ios/15.0) your device.

## Limitations

Support for `apl` target at a early stage of development. Currently only devices with A7-A9x are supported. We have also plans to get support for AIC and touchscreen. If you want to contribute, you are always welcome. 


***NOTE: Please ensure you have a backup of your device before applying the jailbreak. While the data loss is unlikely, we are not responsible if something goes wrong. Use it on your own risk.*** 