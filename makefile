# $@ = target file
# $< = first dependency
# $^ = all dependencies

C_SOURCES = $(wildcard kernel/*.c kernel/*/*.c kernel/*/*/*.c)
S_SOURCES = $(wildcard kernel/interrupts/*.s )
HEADERS = $(wildcard kernel/*.h, drivers/*/*.h)
C_OBJ = ${C_SOURCES:.c=.o} 
S_OBJ = ${S_SOURCES:.s=.o} 

all: run

products/kernel.bin: products/kernel_entry.o ${C_OBJ} ${S_OBJ}
	i386-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary

products/kernel_entry.o: kernel/boot/kernel_entry.s
	nasm $< -f elf -o $@

#products/kernel.o: kernel/kernel.c
#	i386-elf-gcc -ffreestanding -c $< -o $@

%.o: %.c ${HEADERS}
	i386-elf-gcc -ffreestanding -c $< -o $@ -I./include

%.o: %.s
	nasm $< -f elf -o $@

debug/kernel.dis: products/kernel.bin
	ndisasm -b 32 $< > $@

products/boot.bin: kernel/boot/boot.s
	nasm $< -f bin -o $@

products/os-image.bin: products/boot.bin products/kernel.bin
	cat $^ > $@

run: products/os-image.bin
	qemu-system-x86_64 -fda $<

mykernel.iso: products/os-image.bin
	mkdir iso
	mkdir iso/boot
	mkdir iso/boot/grub
	cp products/os-image.bin iso/boot/mykernel.bin
	echo 'set timeout=0'                      > iso/boot/grub/grub.cfg
	echo 'set default=0'                     >> iso/boot/grub/grub.cfg
	echo ''                                  >> iso/boot/grub/grub.cfg
	echo 'menuentry "My Operating System" {' >> iso/boot/grub/grub.cfg
	echo '  multiboot /boot/mykernel.bin'    >> iso/boot/grub/grub.cfg
	echo '  boot'                            >> iso/boot/grub/grub.cfg
	echo '}'                                 >> iso/boot/grub/grub.cfg
	i386-elf-grub-mkrescue --output=mykernel.iso iso
	rm -rf iso

clean:
	rm -rf products/*.bin products/*.o debug/*.dis
	rm -rf kernel/*.bin kernel/*.o kernel/*.dis
	rm -rf kernel/*/*.bin kernel/*/*.o
	rm -rf kernel/*/*/*.bin kernel/*/*/*.o
	rm -rf drivers/*/*.bin drivers/*/*.o drivers/*/*.dis
	rm -rf *.bin *.o *.dis