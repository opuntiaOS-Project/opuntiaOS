C_COMPILE_FLAGS += -ffreestanding
C_INCLUDES += -I./include 
C_DEBUG_FLAGS += -ggdb
C_WARNING_FLAGS += -Werror -Wno-address-of-packed-member
ASM_KERNEL_FLAGS = -f elf
LD_KERNEL_FLAGS = -T build/x86/kernel_link.ld --oformat elf32-i386

# OS RUN CONFIG
KERNEL_STAGE2_POSITION = 0x1000
KERNEL_STAGE3_POSITION = 0xc0000000