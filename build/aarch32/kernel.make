kernel: products/kernel.bin

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

products/kernel.bin: _start.o ${KERNEL_C_OBJ} ${KERNEL_S_OBJ}
	@mkdir -p products
	@echo "$(notdir $(CURDIR)): LD $@"
	${QUIET} ${LD} -o $@ $^ ${LD_KERNEL_FLAGS}

_start.o: src/boot/aarch32/_start.s
	@mkdir -p products
	@echo "$(notdir $(CURDIR)): S3_ENTRY_ASM $@"
	${QUIET} ${ASM} $< -o $@ ${ASM_KERNEL_FLAGS}

${KERNEL_PATH}/%.o: ${KERNEL_PATH}/%.c ${KERNEL_HEADERS}
	@echo "$(notdir $(CURDIR)): CC $@"
	${QUIET} ${CC} -c $< -o $@ ${C_FLAGS}

${KERNEL_PATH}/%.o: ${KERNEL_PATH}/%.s
	@echo "$(notdir $(CURDIR)): ASM $@"
	${QUIET} ${ASM} $< -o $@ ${ASM_KERNEL_FLAGS}