C_COMPILE_FLAGS += -ffreestanding -fno-builtin -march=armv7-a -fno-pie -no-pie
C_INCLUDES += -I./include 
C_DEBUG_FLAGS += -ggdb
C_WARNING_FLAGS += -Werror -Wno-address-of-packed-member
ASM_KERNEL_FLAGS = -march=armv7-a -mcpu=cortex-a15
LD_KERNEL_FLAGS = -T link.ld -nostdlib -nostdinc -nodefaultlibs -nostartfiles -lgcc