# $@ = target file
# $< = first dependency
# $^ = all dependencies

all: run

products/kernel.bin: products/kernel_entry.o products/kernel.o
	ld -o $@ -Ttext 0x1000 $^ --oformat binary

products/kernel_entry.o: kernel_entry.s
	nasm $< -f elf64 -o $@

products/kernel.o: kernel/kernel.c
	gcc -ffreestanding -c $< -o $@

debug/kernel.dis: products/kernel.bin
	ndisasm -b 32 $< > $@

products/boot.bin: boot.s
	nasm $< -f bin -o $@

products/os-image.bin: products/boot.bin products/kernel.bin
	cat $^ > $@

run: products/os-image.bin
	qemu-system-i386 -fda $< -curses

clean:
	rm -rf products/*.bin products/*.o debug/*.dis
	rm -rf *.bin *.o *.dis