# $@ = target file
# $< = first dependency
# $^ = all dependencies

# --- Init ------------------------------------------------------------------ #

include makefile.configs
PYTHON3 = python3

C_COMPILE_FLAGS += -ffreestanding
C_INCLUDES += -I./include 
C_DEBUG_FLAGS += -ggdb
C_WARNING_FLAGS += -Werror -Wno-address-of-packed-member

ASM_KERNEL_FLAGS = -f elf

LD_KERNEL_FLAGS = -Ttext ${KERNEL_STAGE3_POSITION} --oformat elf32-i386

# OS RUN CONFIG
KERNEL_STAGE2_POSITION = 0x1000
KERNEL_STAGE3_POSITION = 0xc0000000
QEMU_I386 = qemu-system-i386
QEMU = ${QEMU_I386}
DISK = one.img

GDBPORT = $(shell expr `id -u` % 5000 + 25000)
QEMUGDB = -s

QUIET = @

all: products/os-image.bin ${DISK} install_os

# --- Stage1 ---------------------------------------------------------------- #

products/boot.bin: src/boot/x86/stage1/boot.s
	${ASM} $< -f bin -o $@

# --- Stage2 ---------------------------------------------------------------- #

STAGE2_PATH = src/boot/x86/stage2

STAGE2_C_SRC = $(wildcard \
					src/boot/x86/stage2/*.c \
					src/boot/x86/stage2/*/*.c \
					src/boot/x86/stage2/*/*/*.c \
					src/boot/x86/stage2/*/*/*/*.c )
STAGE2_S_SRC = $(wildcard \
					src/boot/x86/stage2/*.s \
					src/boot/x86/stage2/*/*.s \
					src/boot/x86/stage2/*/*/*.s \
					src/boot/x86/stage2/*/*/*/*.s )

HEADERS = $(wildcard include/*.h)
STAGE2_C_OBJ = ${STAGE2_C_SRC:.c=.o}
STAGE2_S_OBJ = ${STAGE2_S_SRC:.s=.o}
STAGE2_C_FLAGS = ${C_COMPILE_FLAGS} ${C_DEBUG_FLAGS} ${C_WARNING_FLAGS}

products/stage2.bin: products/stage2_entry.o ${STAGE2_C_OBJ} ${STAGE2_S_OBJ}
	@echo "$(notdir $(CURDIR)): LD_S2 $@"
	${QUIET} ${LD} -o $@ -Ttext ${KERNEL_STAGE2_POSITION} $^ --oformat binary

products/stage2_entry.o: src/boot/x86/stage2_entry.s
	@echo "$(notdir $(CURDIR)): S2_ENTRY_ASM $@"
	${QUIET} ${ASM} $< -o $@ ${ASM_KERNEL_FLAGS}

${STAGE2_PATH}/%.o: ${STAGE2_PATH}/%.c
	@echo "$(notdir $(CURDIR)): CC2 $<"
	${QUIET} ${CC}  -c $< -o $@ ${STAGE2_C_FLAGS}

${STAGE2_PATH}/%.o: ${STAGE2_PATH}/%.s
	@echo "$(notdir $(CURDIR)): ASM2 $@"
	${QUIET} ${ASM} $< -o $@ ${ASM_KERNEL_FLAGS}

# --- Kernel ---------------------------------------------------------------- #

KERNEL_PATH = src/kernel

KERNEL_C_SRC = $(wildcard \
					src/kernel/*.c \
					src/kernel/*/*.c \
					src/kernel/*/*/*.c \
					src/kernel/*/*/*/*.c )
KERNEL_S_SRC = $(wildcard \
					src/kernel/*.s \
					src/kernel/*/*.s \
					src/kernel/*/*/*.s \
					src/kernel/*/*/*/*.s )


HEADERS = $(wildcard include/*.h)
KERNEL_C_OBJ = ${KERNEL_C_SRC:.c=.o}
KERNEL_S_OBJ = ${KERNEL_S_SRC:.s=.o}

C_FLAGS = ${C_COMPILE_FLAGS} ${C_DEBUG_FLAGS} ${C_WARNING_FLAGS} ${C_INCLUDES}

products/kernel.bin: products/stage3_entry.o ${KERNEL_C_OBJ} ${KERNEL_S_OBJ}
	@echo "$(notdir $(CURDIR)): LD $@"
	${QUIET} ${LD} -o $@ $^ ${LD_KERNEL_FLAGS}

products/stage3_entry.o: src/boot/x86/stage3_entry.s
	@echo "$(notdir $(CURDIR)): S3_ENTRY_ASM $@"
	${QUIET} ${ASM} $< -o $@ ${ASM_KERNEL_FLAGS}

${KERNEL_PATH}/%.o: ${KERNEL_PATH}/%.c
	@echo "$(notdir $(CURDIR)): CC $@"
	${QUIET} ${CC}  -c $< -o $@ ${C_FLAGS}

${KERNEL_PATH}/%.o: ${KERNEL_PATH}/%.s
	@echo "$(notdir $(CURDIR)): ASM $@"
	${QUIET} ${ASM} $< -o $@ ${ASM_KERNEL_FLAGS}

# --- Lib ------------------------------------------------------------------- #

ARFLAGS = rcs

LIBKERNEL=libs/libkernel.a
LIBKERNEL_PATH=libs/libkernel
LIBKERNEL_SRC=$(wildcard libs/libkernel/*.c)
LIBKERNEL_OBJ=$(patsubst %.c,%.o,$(LIBKERNEL_SRC))

LIBRARIES=$(LIBKERNEL)

${LIBKERNEL_PATH}/%.o: ${LIBKERNEL_PATH}/%.c
	@echo "$(notdir $(CURDIR)): CC $@"
	${QUIET} ${CC}  -c $< -o $@ ${C_FLAGS}

${LIBKERNEL}: ${LIBKERNEL_OBJ}
	${AR} ${ARFLAGS} $@ $^

# --- Apps ------------------------------------------------------------------ #

C_USERLAND_FLAGS = ${C_COMPILE_FLAGS} -I./libs/

APPS_PATH = userland

APPS = 	${APPS_PATH}/init.sys \
		${APPS_PATH}/sec.sys \
		${APPS_PATH}/main.sys

# system apps
${APPS_PATH}/%.sys: ${APPS_PATH}/%.s
	${ASM} $< -f elf -o tmp.o
	${LD} tmp.o -Ttext 0x0 -o $@ --oformat binary
	cp $@ home/

#std compiler
${APPS_PATH}/%.o: ${APPS_PATH}/%.c
	${CC} -c $< -o $@ ${C_USERLAND_FLAGS}

# test with lib
${APPS_PATH}/main.sys: ${APPS_PATH}/main.o ${LIBKERNEL}
	${LD} $^ -Ttext 0x0 -o $@ --oformat binary
	cp $@ home/

# --- Others ---------------------------------------------------------------- #

debug/kernel.dis: products/kernel.bin
	ndisasm -b 32 $< > $@

products/os-image.bin: products/boot.bin products/stage2.bin
	cat $^ > $@

run: products/os-image.bin ${DISK} install_apps
	${QEMU} -m 256M -fda $< -device piix3-ide,id=ide -drive id=disk,file=one.img,if=none -device ide-drive,drive=disk,bus=ide.0 

run-dbg: products/os-image.bin ${DISK} install_os
	${QEMU} -m 256M -fda $< -device piix3-ide,id=ide -drive id=disk,file=one.img,if=none -device ide-drive,drive=disk,bus=ide.0 -S $(QEMUGDB)

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

format_ext2: ${DISK}
	sudo /usr/local/opt/e2fsprogs/sbin/mkfs.ext2 -r 0 ${DISK}

format:
	${PYTHON3} utils/fat16_formatter.py

install_os: ${DISK} products/kernel.bin
	${PYTHON3} utils/copy_bin.py

install_apps: ${APPS}
	${PYTHON3} utils/install_apps.py

sync:
	sudo fuse-ext2 one.img base -o rw+
	sudo mkdir -p base/boot
	sudo cp products/kernel.bin base/boot/
	sudo umount base