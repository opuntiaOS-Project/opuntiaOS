# $@ = target file
# $< = first dependency
# $^ = all dependencies

# --- Init ------------------------------------------------------------------ #

PLATFORM = x86
MAKEFILES = build/${PLATFORM}
PYTHON3 = python3

include ${MAKEFILES}/${PLATFORM}.configs
include build/makefile.configs

include ${MAKEFILES}/consts.make

GDBPORT = $(shell expr `id -u` % 5000 + 25000)
QEMUGDB = -s
CONNECTION_COMPILER = utils/ConnectionCompiler

QUIET = @

all: kernel apps
run: run_${PLATFORM}
debug: debug_${PLATFORM}

include ${MAKEFILES}/kernel.make

# --- Lib ------------------------------------------------------------------- #

CXX_LIB_FLAGS = ${C_COMPILE_FLAGS} ${C_WARNING_FLAGS} -std=c++17 -fno-sized-deallocation -fno-rtti -fno-exceptions  -I./libs/libcxx -I./libs -D__oneOS__

ARFLAGS = rcs

LIB_PATH = ${BASE_DIR}/lib

LIBC = $(LIB_PATH)/libc.a
LIBC_PATH = libs/libc
LIBC_C_SRC=$(shell find libs/libc -name "*.c" -not -path "libs/libc/private/*")
LIBC_S_SRC=$(shell find libs/libc -name "*.s" -not -path "libs/libc/private/*")
LIBC_C_OBJ=$(patsubst %.c,%.o,$(LIBC_C_SRC))
LIBC_S_OBJ=$(patsubst %.s,%.o,$(LIBC_S_SRC))
LIBC_OBJ = $(LIBC_S_OBJ) $(LIBC_C_OBJ)

LIBCXX = $(LIB_PATH)/libcxx.a
LIBCXX_PATH = libs/libcxx
LIBCXX_SRC=$(shell find $(LIBCXX_PATH) -name "*.cpp" -not -path " -not -path "libs/libcxx/private/*"/*")
LIBCXX_OBJ=$(patsubst %.cpp,%.o,$(LIBCXX_SRC))

LIBUI = $(LIB_PATH)/libui.a
LIBUI_PATH = libs/libui
LIBUI_SRC=$(shell find $(LIBUI_PATH) -name "*.cpp")
LIBUI_OBJ=$(patsubst %.cpp,%.o,$(LIBUI_SRC))

LIBG = $(LIB_PATH)/libg.a
LIBG_PATH = libs/libg
LIBG_SRC=$(shell find $(LIBG_PATH) -name "*.cpp")
LIBG_OBJ=$(patsubst %.cpp,%.o,$(LIBG_SRC))

LIBFOUNDATION = $(LIB_PATH)/libfoundation.a
LIBFOUNDATION_PATH = libs/libfoundation
LIBFOUNDATION_C_SRC=$(shell find $(LIBFOUNDATION_PATH) -name "*.c")
LIBFOUNDATION_CPP_SRC=$(shell find $(LIBFOUNDATION_PATH) -name "*.cpp")
LIBFOUNDATION_C_OBJ=$(patsubst %.c,%.o,$(LIBFOUNDATION_C_SRC))
LIBFOUNDATION_CPP_OBJ=$(patsubst %.cpp,%.o,$(LIBFOUNDATION_CPP_SRC))
LIBFOUNDATION_OBJ = $(LIBFOUNDATION_C_OBJ) $(LIBFOUNDATION_CPP_OBJ) 

LIBRARIES_ALL = $(LIBC) $(LIBCXX) $(LIBUI) $(LIBG) $(LIBFOUNDATION)
LIBRARIES = $(LIBC)

# LIB C

${LIBC_PATH}/%.o: ${LIBC_PATH}/%.s
	@mkdir -p $(LIB_PATH)
	@echo "$(notdir $(CURDIR)): ASM $@"
	${QUIET} ${ASM} $< -o $@ ${ASM_KERNEL_FLAGS}

${LIBC_PATH}/%.o: ${LIBC_PATH}/%.c
	@mkdir -p $(LIB_PATH)
	@echo "$(notdir $(CURDIR)): CC $@"
	${QUIET} ${CC} -c $< -o $@ -Os ${C_FLAGS} -I./${LIBC_PATH}

${LIBC}: ${LIBC_OBJ} libs/libc/private/_init.o
	@echo "$(notdir $(CURDIR)): [AR] $@"
	${QUIET} ${AR} ${ARFLAGS} $@ $^

# LIB CXX

${LIBCXX_PATH}/%.o: ${LIBCXX_PATH}/%.cpp
	@mkdir -p $(LIB_PATH)
	@echo "$(notdir $(CURDIR)): C++ $@"
	${QUIET} ${C++} -c $< -o $@ -Os ${CXX_LIB_FLAGS} -I./libs -I./libs/libcxx

${LIBCXX}: ${LIBCXX_OBJ} $(LIBC_OBJ) libs/libcxx/private/_init.o
	@echo "$(notdir $(CURDIR)): [AR] $(LIBC_OBJ)"
	${QUIET} ${AR} -rcs $@ $^ 

# LIB Graphics

${LIBG_PATH}/%.o: ${LIBG_PATH}/%.cpp
	@mkdir -p $(LIB_PATH)
	@echo "$(notdir $(CURDIR)): C++ $@"
	${QUIET} ${C++} -c $< -o $@ -Os ${CXX_LIB_FLAGS} -I./libs -I./libs/libcxx

${LIBG}: ${LIBG_OBJ} ${LIBCXX_OBJ} $(LIBC_OBJ)
	@echo "$(notdir $(CURDIR)): [AR] $@"
	${QUIET} ${AR} -rcs $@ $^

# LIB Foundation

${LIBFOUNDATION_PATH}/%.o: ${LIBFOUNDATION_PATH}/%.c
	@mkdir -p $(LIB_PATH)
	@echo "$(notdir $(CURDIR)): CC $@"
	${QUIET} ${CC} -c $< -o $@ -Os ${C_FLAGS} -I./libs/libc

${LIBFOUNDATION_PATH}/%.o: ${LIBFOUNDATION_PATH}/%.cpp
	@mkdir -p $(LIB_PATH)
	@echo "$(notdir $(CURDIR)): C++ $@"
	${QUIET} ${C++} -c $< -o $@ -Os ${CXX_LIB_FLAGS} -I./libs -I./libs/libcxx

${LIBFOUNDATION}: ${LIBFOUNDATION_OBJ} ${LIBCXX_OBJ} $(LIBC_OBJ)
	@echo "$(notdir $(CURDIR)): [AR] $@"
	${QUIET} ${AR} -rcs $@ $^

# LIB UI

${LIBUI_PATH}/%.o: ${LIBUI_PATH}/%.cpp servers/window_server/WSConnection.h
	@mkdir -p $(LIB_PATH)
	@echo "$(notdir $(CURDIR)): C++ $@"
	${QUIET} ${C++} -c $< -o $@ -Os ${CXX_LIB_FLAGS} -I./libs -I./libs/libcxx

${LIBUI}: ${LIBUI_OBJ} ${LIBG_OBJ} ${LIBFOUNDATION_OBJ} ${LIBCXX_OBJ} $(LIBC_OBJ)
	@echo "$(notdir $(CURDIR)): [AR] $@"
	${QUIET} ${AR} -rcs $@ $^

# --- CRTs ------------------------------------------------------------------- #

CRTS = libs/crt0.o \

libs/crt0.o: libs/crt0.s
	${QUIET} $(ASM) $< -f elf -o $@

# --- Servers ---------------------------------------------------------------- #

SERVERS_PATH = ${BASE_DIR}/bin
WINDOW_SERVER = $(SERVERS_PATH)/window_server
WINDOW_SERVER_PATH = servers/window_server
WINDOW_SERVER_SRC=$(shell find $(WINDOW_SERVER_PATH) -name "*.cpp")
WINDOW_SERVER_OBJ=$(patsubst %.cpp,%.o,$(WINDOW_SERVER_SRC))
WINDOW_SERVER_IPC=$(WINDOW_SERVER_PATH)/WSConnection.h
WINDOW_SERVER_DEPENDENCIES = ${BASE_DIR}/lib/libcxx.a ${BASE_DIR}/lib/libg.a ${BASE_DIR}/lib/libfoundation.a

SERVERS = $(WINDOW_SERVER)

servers/window_server/WSConnection.h: servers/window_server/ws_connection.ipc
	@echo "$(notdir $(CURDIR)): CON $@"
	${QUIET} ${PYTHON3} ${CONNECTION_COMPILER} $< $@

${WINDOW_SERVER_PATH}/%.o: ${WINDOW_SERVER_PATH}/%.cpp ${WINDOW_SERVER_IPC}
	@mkdir -p $(WINDOW_SERVER_PATH)
	@echo "$(notdir $(CURDIR)): C++ $@"
	${QUIET} ${C++} -c $< -o $@ -Os -std=c++17 -fno-sized-deallocation -fno-rtti -fno-exceptions ${C_FLAGS} -I./${LIBCXX_PATH} -I./libs/

$(WINDOW_SERVER): ${WINDOW_SERVER_IPC} $(WINDOW_SERVER_OBJ) $(CRTS) ${WINDOW_SERVER_DEPENDENCIES}
	@echo "Window Server [LD]  $@"
	$(LD) $(CRTS) $(WINDOW_SERVER_OBJ) -o $@ ${WINDOW_SERVER_DEPENDENCIES} -Map ws.map
	
# --- Apps ------------------------------------------------------------------ #

C_USERLAND_FLAGS = ${C_COMPILE_FLAGS} -I./libs/libc -D__oneOS__
CXX_USERLAND_FLAGS = ${C_COMPILE_FLAGS} -std=c++17 -fno-sized-deallocation -fno-rtti -fno-exceptions  -I./libs/libcxx -I./libs -D__oneOS__

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
$(1)_CPP_SOURCES = $$(wildcard ${APPS_PATH}/$($(1)_NAME)/*.cpp) \
				$$(wildcard ${APPS_PATH}/$($(1)_NAME)/*/*.cpp) \
			   	$$(wildcard ${APPS_PATH}/$($(1)_NAME)/*/*/*.cpp)
$(1)_INC_LIBSLIST = $$(addprefix -I./libs/lib, $$($(1)_LIBS))
$(1)_SRC_LIBSLIST = $$(addprefix $(LIB_PATH)/lib, $$(addsuffix .a, $$($(1)_LIBS)))

$(1)_C_OBJECTS = $$(patsubst %.c,%.o,$$($(1)_C_SOURCES))
$(1)_CPP_OBJECTS = $$(patsubst %.cpp,%.o,$$($(1)_CPP_SOURCES))
$(1)_S_OBJECTS = $$(patsubst %.s,%.o,$$($(1)_S_SOURCES))

# system apps
$$($(1)_BINARY): $$($(1)_C_OBJECTS) $$($(1)_CPP_OBJECTS) $$($(1)_S_OBJECTS) $$(CRTS) $$($(1)_SRC_LIBSLIST)
	@mkdir -p $(BASE_DIR)/$($(1)_INSTALL_PATH)
	@echo "$($(1)_NAME) [LD]  $$@"
	$(QUIET) $$(LD) $$(CRTS) $$($(1)_C_OBJECTS) $$($(1)_CPP_OBJECTS) $$($(1)_S_OBJECTS) -o $$@ $$($(1)_SRC_LIBSLIST)

# std compiler
${APPS_PATH}/$($(1)_NAME)/%.o: ${APPS_PATH}/$($(1)_NAME)/%.c
	@echo "$($(1)_NAME) [CC]  $$@"
	$(QUIET) $$(CC) -c $$< -o $$@ $$(C_USERLAND_FLAGS) $$($(1)_INC_LIBSLIST)

# std compiler
${APPS_PATH}/$($(1)_NAME)/%.o: ${APPS_PATH}/$($(1)_NAME)/%.cpp
	@echo "$($(1)_NAME) [C++] $$@"
	$(QUIET) $$(C++) -c $$< -o $$@ $$(CXX_USERLAND_FLAGS) $$($(1)_INC_LIBSLIST)

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

apps: $(APPS_BINARIES) $(SERVERS)
	echo $(CAT_LIBSLIST)

run_x86: products/os-image.bin ${DISK}
	${QEMU} -m 256M -fda $< -device piix3-ide,id=ide -drive id=disk,file=${DISK},if=none -device ide-drive,drive=disk,bus=ide.0 -serial mon:stdio -rtc base=utc -vga std

debug_x86: products/os-image.bin ${DISK}
	${QEMU} -m 256M -fda $< -device piix3-ide,id=ide -drive id=disk,file=${DISK},if=none -device ide-drive,drive=disk,bus=ide.0 -S $(QEMUGDB) -serial mon:stdio -rtc base=utc -vga std

run_aarch32: ${DISK}
	${QEMU} -M vexpress-a15 -cpu cortex-a15 -kernel products/kernel.bin -serial mon:stdio -vga std -sd ${DISK}

debug_aarch32: ${DISK}
	${QEMU} -M vexpress-a15 -cpu cortex-a15 -kernel products/kernel.bin -serial mon:stdio -vga std -sd ${DISK} -S $(QEMUGDB)

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
	rm -rf libs/*/*/*.o
	rm -rf libs/*/*.o
	rm -rf libs/*.o
	rm -rf servers/*/*.o
	rm -rf servers/*.o

	rm -rf base/lib/*
	rm -rf base/bin/*

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