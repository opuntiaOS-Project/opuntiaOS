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

all: run

products/kernel.bin: products/stage3_entry.o ${C_OBJ} ${S_OBJ}
	i386-elf-ld -Ttext 0xc0000000 -o $@ $^ --oformat elf32-i386

products/stage2.bin: products/stage2_entry.o ${C_OBJ2} ${S_OBJ2}
	i386-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary

products/stage2_entry.o: src/boot/x86/stage2_entry.s
	/usr/local/bin/nasm $< -f elf -o $@

products/stage3_entry.o: src/boot/x86/stage3_entry.s
	/usr/local/bin/nasm $< -f elf -o $@

%.o: %.c ${HEADERS}
	i386-elf-gcc -ffreestanding -c $< -o $@ -I./include

%.o: %.s
	/usr/local/bin/nasm $< -f elf -o $@

debug/kernel.dis: products/kernel.bin
	ndisasm -b 32 $< > $@

products/boot.bin: src/boot/x86/stage1/boot.s
	/usr/local/bin/nasm $< -f bin -o $@

products/os-image.bin: products/boot.bin products/stage2.bin
	cat $^ > $@

run: products/os-image.bin
	make drive
	./qemu/programs/qemu-system-i386 -m 256M -fda $< -device piix3-ide,id=ide -drive id=disk,file=one.img,if=none -device ide-drive,drive=disk,bus=ide.0

debug: products/os-image.bin
	make drive
	./qemu/programs/qemu-system-i386 -serial file:serial.log -d int -fda $< -device piix3-ide,id=ide -drive id=disk,file=one.img,if=none -device ide-drive,drive=disk,bus=ide.0

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

one.img:
	qemu-img create -f raw one.img 1M

drive:
	make products/kernel.bin
	python3 utils/copy_bin.py
