#!/bin/bash

version="gcc-arm-none-eabi-10-2020-q4"
arch="x86_64" # x86_64 or aarch64

wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/${version}-major-${arch}-linux.tar.bz2
sudo tar xjf ${version}.bz2 -C /usr/share/
sudo ln -s /usr/share/${version}/bin/arm-none-eabi-gcc /usr/bin/arm-none-eabi-gcc
sudo ln -s /usr/share/${version}/bin/arm-none-eabi-g++ /usr/bin/arm-none-eabi-g++
sudo ln -s /usr/share/${version}/bin/arm-none-eabi-ar /usr/bin/arm-none-eabi-ar
sudo ln -s /usr/share/${version}/bin/arm-none-eabi-as /usr/bin/arm-none-eabi-as
sudo ln -s /usr/share/${version}/bin/arm-none-eabi-gdb /usr/bin/arm-none-eabi-gdb