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
LD_KERNEL_FLAGS = -T link.ld --oformat elf32-i386

# FS CONFIG
BASE_DIR = base

# OS RUN CONFIG
KERNEL_STAGE2_POSITION = 0x1000
KERNEL_STAGE3_POSITION = 0xc0000000
QEMU_I386 = qemu-system-i386
QEMU = ${QEMU_I386}
DISK = one.img

GDBPORT = $(shell expr `id -u` % 5000 + 25000)
QEMUGDB = -s

QUIET = @

all: products/os-image.bin products/kernel.bin apps

# --- Stage1 ---------------------------------------------------------------- #

products/boot.bin: src/boot/x86/stage1/boot.s
	@mkdir -p products
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

STAGE2_HEADERS = $(wildcard include/*.h)
STAGE2_C_OBJ = ${STAGE2_C_SRC:.c=.o}
STAGE2_S_OBJ = ${STAGE2_S_SRC:.s=.o}
STAGE2_C_FLAGS = ${C_COMPILE_FLAGS} ${C_DEBUG_FLAGS} ${C_WARNING_FLAGS}

products/stage2.bin: products/stage2_entry.o ${STAGE2_C_OBJ} ${STAGE2_S_OBJ}
	@mkdir -p products
	@echo "$(notdir $(CURDIR)): LD_S2 $@"
	${QUIET} ${LD} -o $@ -Ttext ${KERNEL_STAGE2_POSITION} $^ --oformat binary

products/stage2_entry.o: src/boot/x86/stage2_entry.s
	@mkdir -p products
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


KERNEL_HEADERS = $(shell find include -name "*.h")
KERNEL_C_OBJ = ${KERNEL_C_SRC:.c=.o}
KERNEL_S_OBJ = ${KERNEL_S_SRC:.s=.o}

C_FLAGS = ${C_COMPILE_FLAGS} ${C_DEBUG_FLAGS} ${C_WARNING_FLAGS} ${C_INCLUDES}

products/kernel.bin: products/stage3_entry.o ${KERNEL_C_OBJ} ${KERNEL_S_OBJ}
	@mkdir -p products
	@echo "$(notdir $(CURDIR)): LD $@"
	${QUIET} ${LD} -o $@ $^ ${LD_KERNEL_FLAGS}

products/stage3_entry.o: src/boot/x86/stage3_entry.s
	@mkdir -p products
	@echo "$(notdir $(CURDIR)): S3_ENTRY_ASM $@"
	${QUIET} ${ASM} $< -o $@ ${ASM_KERNEL_FLAGS}

${KERNEL_PATH}/%.o: ${KERNEL_PATH}/%.c ${KERNEL_HEADERS}
	@echo "$(notdir $(CURDIR)): CC $@"
	${QUIET} ${CC} -c $< -o $@ ${C_FLAGS}

${KERNEL_PATH}/%.o: ${KERNEL_PATH}/%.s
	@echo "$(notdir $(CURDIR)): ASM $@"
	${QUIET} ${ASM} $< -o $@ ${ASM_KERNEL_FLAGS}

# --- Lib ------------------------------------------------------------------- #

ARFLAGS = rcs

LIB_PATH = ${BASE_DIR}/lib
LIBSYSTEM = $(LIB_PATH)/libsystem.a
LIBSYSTEM_PATH = libs/libsystem
LIBSYSTEM_SRC=$(wildcard libs/libsystem/*.c)
LIBSYSTEM_OBJ=$(patsubst %.c,%.o,$(LIBSYSTEM_SRC))

LIBRARIES = $(LIBSYSTEM)

${LIBSYSTEM_PATH}/%.o: ${LIBSYSTEM_PATH}/%.c
	@mkdir -p $(LIB_PATH)
	@echo "$(notdir $(CURDIR)): CC $@"
	${QUIET} ${CC} -c $< -o $@ ${C_FLAGS}

${LIBSYSTEM}: ${LIBSYSTEM_OBJ}
	${AR} ${ARFLAGS} $@ $^


# --- CRTs ------------------------------------------------------------------- #

CRTS = libs/crt0.o \

libs/crt0.o: libs/crt0.s
	$(ASM) $< -f elf -o $@
	
# --- Apps ------------------------------------------------------------------ #

C_USERLAND_FLAGS = ${C_COMPILE_FLAGS} -I./libs/

APPS_PATH = userland

define APP_TEMPLATE
$(1)_BINARY = $(BASE_DIR)/$($(1)_INSTALL_PATH)/$($(1)_NAME)
$(1)_ELF  = $(APPS_PATH)/$($(1)_NAME).exec
$(1)_C_SOURCES =  $$(wildcard ${APPS_PATH}/$($(1)_NAME)/*.c) \
				$$(wildcard ${APPS_PATH}/$($(1)_NAME)/*/*.c) \
			   	$$(wildcard ${APPS_PATH}/$($(1)_NAME)/*/*/*.c)
$(1)_S_SOURCES = $$(wildcard ${APPS_PATH}/$($(1)_NAME)/*.s) \
				$$(wildcard ${APPS_PATH}/$($(1)_NAME)/*/*.s) \
			   	$$(wildcard ${APPS_PATH}/$($(1)_NAME)/*/*/*.s)

$(1)_C_OBJECTS = $$(patsubst %.c,%.o,$$($(1)_C_SOURCES))
$(1)_S_OBJECTS = $$(patsubst %.s,%.o,$$($(1)_S_SOURCES))

# system apps
$$($(1)_BINARY): $$($(1)_C_OBJECTS) $$($(1)_S_OBJECTS) $$(CRTS) $$(LIBRARIES)
	@mkdir -p $(BASE_DIR)/$($(1)_INSTALL_PATH)
	@echo "$($(1)_NAME) [LD]  $$@"
	$(QUIET) $$(LD) $$(CRTS) $$($(1)_C_OBJECTS) $$($(1)_S_OBJECTS) -Ttext 0x0 -o $$@ --oformat binary $$(LIBRARIES)

# std compiler
${APPS_PATH}/$($(1)_NAME)/%.o: ${APPS_PATH}/$($(1)_NAME)/%.c
	@echo "$($(1)_NAME) [CC]  $$@"
	$(QUIET) $$(CC) -c $$< -o $$@ $$(C_USERLAND_FLAGS)

# std compiler
${APPS_PATH}/$($(1)_NAME)/%.o: ${APPS_PATH}/$($(1)_NAME)/%.s
	@echo "$($(1)_NAME) [ASM] $$@"
	$(QUIET) $$(ASM) $$< -f elf -o $$@

APPS_BINARIES += $$($(1)_BINARY)
endef

# For now we build them as binaries
include ${APPS_PATH}/*/.info.mk
$(foreach app, $(APPS), $(eval $(call APP_TEMPLATE,$(app))))

# --- Others ---------------------------------------------------------------- #

apps: $(APPS_BINARIES)

debug/kernel.dis: products/kernel.bin
	ndisasm -b 32 $< > $@

products/os-image.bin: products/boot.bin products/stage2.bin
	@mkdir -p products
	cat $^ > $@

run: products/os-image.bin ${DISK}
	${QEMU} -m 256M -fda $< -device piix3-ide,id=ide -drive id=disk,file=one.img,if=none -device ide-drive,drive=disk,bus=ide.0 -serial stdio

debug: products/os-image.bin ${DISK}
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

	rm -rf userland/*/*.o
	rm -rf libs/*/*.o
	rm -rf libs/*.o

${DISK}:
	qemu-img create -f raw ${DISK} 16M

format_ext2: ${DISK}
	sudo ${MKFS} -t ext2 -r 0 -b 1024 ${DISK}

base_env:
	sudo mkdir -p base/dev
	sudo mkdir -p base/proc

sync: base_env products/kernel.bin
	sudo mkdir -p mountpoint
	sudo ${MOUNT_EXT2} one.img mountpoint -o rw+
	sudo mkdir -p mountpoint/boot
	sudo mkdir -p mountpoint/proc
	sudo cp -r base/* mountpoint/
	sudo cp products/kernel.bin mountpoint/boot/
	sudo umount mountpoint

# was used with fat16
format_fat16:
	${PYTHON3} utils/fat16_formatter.py

# was used with fat16
old_install_os: ${DISK} products/kernel.bin
	${PYTHON3} utils/copy_bin.py

# was used with fat16
old_install_apps: ${APPS}
	${PYTHON3} utils/install_apps.py