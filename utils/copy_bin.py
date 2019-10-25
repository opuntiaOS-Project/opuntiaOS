copyTo = 'one.img'
copyFrom = 'products/kernel.bin'
kernelSize = 0

def print_gh(el):
    if (el >= 10):
        return chr(el - 10 + ord('A'))
    return chr(el + ord('0'))

def print_g(result):
    for (id,el) in enumerate(result):
        el = int.from_bytes(el, byteorder='little')
        if id % 16 == 0:
            print()
        print(print_gh(el//16), end="")
        print(print_gh(el%16), end=" ")

def open_file():
    global kernelSize
    kernelSize = 0
    dd = [0, 0]
    with open(copyFrom, "rb") as f:
        byte = f.read(1)
        dd.append(byte)
        kernelSize += 1
        while byte != b"":
            kernelSize += 1
            byte = f.read(1)
            dd.append(byte)
    return dd

def write_file(dd, withPos = 0):
    with open(copyTo, "rb+") as f:
        f.seek(withPos, 0)
        for x in dd:
            f.write(x)

def append(res, plus, offset):
    for (id,el) in enumerate(plus):
        res[id+offset] = el
    return res

dd = open_file()
kernelSize += 2 # Including offset of size
kernelSizeKb = (kernelSize + 1024 - 1) // 1024
kernelSizeKb1 = kernelSizeKb % 256
kernelSizeKb2 = kernelSizeKb // 256
print(kernelSizeKb)
kSize1 = kernelSizeKb1.to_bytes(1, 'little', signed=False)
kSize2 = kernelSizeKb2.to_bytes(1, 'little', signed=False)
dd[0] = kSize1
dd[1] = kSize2
write_file(dd)
print(kernelSize)