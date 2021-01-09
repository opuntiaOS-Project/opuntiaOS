kernel: products/os-image.bin products/kernel.bin

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
	${QUIET} ${CC} -c $< -o $@ -Os ${C_FLAGS}

${KERNEL_PATH}/%.o: ${KERNEL_PATH}/%.s
	@echo "$(notdir $(CURDIR)): ASM $@"
	${QUIET} ${ASM} $< -o $@ ${ASM_KERNEL_FLAGS}


products/os-image.bin: products/boot.bin products/stage2.bin
	@mkdir -p products
	cat $^ > $@