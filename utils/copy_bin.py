copyTo = 'one.img'
copyFrom = 'products/kernel.bin'
kernelSize = 0

def open_file():
    global kernelSize
    kernelSize = 0
    # dd = [0, 0]
    dd = []

    with open(copyFrom, "rb") as f:
        byte = f.read(1)
        dd.append(int.from_bytes(byte, byteorder='little'))
        kernelSize += 1
        while byte != b"":
            kernelSize += 1
            byte = f.read(1)
            dd.append(int.from_bytes(byte, byteorder='little'))
    return dd

def open_app(fname):
    dd = []

    with open(fname, "rb") as f:
        byte = f.read(1)
        dd.append(int.from_bytes(byte, byteorder='little'))
        while byte != b"":
            byte = f.read(1)
            dd.append(int.from_bytes(byte, byteorder='little'))
    return dd

def write_file(dd, withPos = 0):
    with open(copyTo, "rb+") as f:
        f.seek(withPos, 0)
        for x in dd:
            f.write(x.to_bytes(1, 'little', signed=False))

def append(res, plus, offset):
    for (id,el) in enumerate(plus):
        res[id+offset] = el
    return res

def get_from(offset, len):
    global dd
    res = 0
    mt = 1
    for i in range(len):
        res += dd[offset + i] * mt
        mt *= 256
    return res

dd = open_file()

e_shoff = get_from(0x20, 4)
e_shentsize = get_from(0x2E, 2)
e_shnum = get_from(0x30, 2)
e_shstrndx = get_from(0x32, 2)
print("Start of the section header table ", e_shoff, e_shentsize, e_shnum, e_shstrndx)

text_size = 0
data_size = 0
bss_size = 0

e_shoff_now = e_shoff
txt_offset = get_from(e_shoff + e_shstrndx * e_shentsize + 0x10, 4)
for i in range(e_shnum + 1):
    if i == e_shstrndx:
        continue
    name = ""
    sh_name = get_from(e_shoff_now + 0x00, 4)
    print(sh_name)
    while dd[txt_offset+sh_name] != 0:
        name += chr(dd[txt_offset+sh_name])
        sh_name+=1
    sh_type = get_from(e_shoff_now + 0x04, 4)
    sh_size = get_from(e_shoff_now + 0x14, 4)
    sh_entsize = get_from(e_shoff_now + 0x24, 4)
    if name == '.text':
        text_size += sh_size
    if name == '.rodata':
        text_size += sh_size
    if name == '.eh_frame':
        text_size += sh_size
    if name == '.data':
        data_size += sh_size
    if name == '.bss':
        bss_size += sh_size
    print(name, sh_type, sh_size, sh_entsize)
    e_shoff_now += e_shentsize


print("text: {0}, data: {1}, bss:{2}".format(text_size, data_size, bss_size))

kernelSize = text_size + data_size + bss_size
kernelSize += 2 # Including offset of size
kernelSizeKb = (kernelSize + 1024 - 1) // 1024
kernelSizeKb1 = kernelSizeKb % 256
kernelSizeKb2 = kernelSizeKb // 256
print("Kernel Size (KB) ", kernelSizeKb)
d_kernel = [0, 0]
d_kernel[0] = kernelSizeKb1
d_kernel[1] = kernelSizeKb2

d_kernel += dd[0x1000:(0x1000 + text_size)]
# print_g(d_kernel)

write_file(d_kernel, 512)
print(kernelSize)
