# $@ = target file
# $< = first dependency
# $^ = all dependencies

C_SOURCES = $(wildcard src/kernel/*.c src/kernel/*/*.c src/kernel/*/*/*.c src/kernel/*/*/*/*.c)
S_SOURCES = $(wildcard src/kernel/*.s src/kernel/*/*.s src/kernel/*/*/*.s src/kernel/*/*/*/*.s)
C_SOURCES2 = $(wildcard src/boot/x86/stage2/*.c src/boot/x86/stage2/*/*.c src/boot/x86/stage2/*/*/*.c src/boot/x86/stage2/*/*/*/*.c)
S_SOURCES2 = $(wildcard src/boot/x86/stage2/*.s src/boot/x86/stage2/*/*.s src/boot/x86/stage2/*/*/*.s src/boot/x86/stage2/*/*/*/*.s)

HEADERS = $(wildcard include/*.h)
C_OBJ = ${C_SOURCES:.c=.o}
S_OBJ = ${S_SOURCES:.s=.o}
C_OBJ2 = ${C_SOURCES2:.c=.o}
S_OBJ2 = ${S_SOURCES2:.s=.o}

# BUILDING CONFIG
I386_ELF_GCC = i386-elf-gcc
I386_LD = i386-elf-ld
NASM = /usr/local/bin/nasm
PYTHON3 = python3

# OS RUN CONFIG
KERNEL_STAGE2_POSITION = 0x1000
KERNEL_STAGE3_POSITION = 0xc0000000
QEMU_I386 = qemu-system-i386
QEMU = ${QEMU_I386}
DISK = one.img

GDBPORT = $(shell expr `id -u` % 5000 + 25000)
QEMUGDB = -s

all: run

products/kernel.bin: products/stage3_entry.o ${C_OBJ} ${S_OBJ}
	${I386_LD} -Ttext ${KERNEL_STAGE3_POSITION} -o $@ $^ --oformat elf32-i386

products/stage2.bin: products/stage2_entry.o ${C_OBJ2} ${S_OBJ2}
	${I386_LD} -o $@ -Ttext ${KERNEL_STAGE2_POSITION} $^ --oformat binary

products/stage2_entry.o: src/boot/x86/stage2_entry.s
	${NASM} $< -f elf -o $@

products/stage3_entry.o: src/boot/x86/stage3_entry.s
	${NASM} $< -f elf -o $@

%.o: %.c ${HEADERS}
	${I386_ELF_GCC} -ggdb -ffreestanding -c $< -o $@ -I./include -Werror -Wno-address-of-packed-member

%.o: %.s
	${NASM} $< -f elf -o $@

debug/kernel.dis: products/kernel.bin
	ndisasm -b 32 $< > $@

products/boot.bin: src/boot/x86/stage1/boot.s
	${NASM} $< -f bin -o $@

products/os-image.bin: products/boot.bin products/stage2.bin
	cat $^ > $@

run: products/os-image.bin ${DISK}
	make drive
	${QEMU} -m 256M -fda $< -device piix3-ide,id=ide -drive id=disk,file=one.img,if=none -device ide-drive,drive=disk,bus=ide.0 

run-dbg: products/os-image.bin ${DISK}
	make drive
	${QEMU} -m 256M -fda $< -device piix3-ide,id=ide -drive id=disk,file=one.img,if=none -device ide-drive,drive=disk,bus=ide.0 -S $(QEMUGDB)


run-no-ide: products/os-image.bin
	make drive
	${QEMU} -m 256M -fda $< -hda one.img -hdb one2.img

debug: products/os-image.bin
	make drive
	${QEMU} -serial file:serial.log -d int -fda $< -device piix3-ide,id=ide -drive id=disk,file=one.img,if=none -device ide-drive,drive=disk,bus=ide.0

clean:
	rm -rf products/*.bin products/*.o debug/*.dis
	rm -rf src/kernel/*.bin src/kernel/*.o src/kernel/*.dis
	rm -rf src/kernel/*/*.bin src/kernel/*/*.o
	rm -rf src/kernel/*/*/*.bin src/kernel/*/*/*.o
	rm -rf src/kernel/*/*/*/*.bin src/kernel/*/*/*/*.o

	rm -rf src/boot/x86/stage2/*.bin src/boot/x86/stage2/*.o src/boot/x86/stage2/*.dis
	rm -rf src/boot/x86/stage2/*/*.bin src/boot/x86/stage2/*/*.o
	rm -rf src/boot/x86/stage2/*/*/*.bin src/boot/x86/stage2/*/*/*.o
	rm -rf src/boot/x86/stage2/*/*/*/*.bin src/boot/x86/stage2/*/*/*/*.o

${DISK}:
	qemu-img create -f raw ${DISK} 1M

format:
	${PYTHON3} utils/fat16_formatter.py

drive: products/kernel.bin
	${PYTHON3} utils/copy_bin.py

install_apps:
	${PYTHON3} utils/install_apps.py
