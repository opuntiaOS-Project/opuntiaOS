# implemented in python to support all platforms
# oneOS
# fat16 in oneOS

from config import format_settings

filename = format_settings['filename']

file_descriptor = {
    'name': filename,
    'size': 0, #(bytes)
}

def open_file():
    size = 0
    with open(filename, "rb") as f:
        byte = f.read(1)
        while byte != b"":
            size+=1
            byte = f.read(1)

    file_descriptor['size'] = size

def write_file(dd, withPos = 0):
    with open(filename, "rb+") as f:
        f.seek(withPos, 0)
        f.write(bytes(x for x in dd))

def merge(res, plus, offset):
    for (id,el) in enumerate(plus):
        res[id + offset] = el
    return res

def test_id():
    result = bytearray(8)
    textCode = 'oneOSToP'
    for (id,el) in enumerate(textCode):
        result[id] = ord(el)
    return result

def bytes_per_sector():
    result = bytearray(2)
    result[0] = format_settings['BytesPerSector'] % 256
    result[1] = format_settings['BytesPerSector'] // 256
    return result

def sectors_per_cluster():
    result = bytearray(1)
    result[0] = format_settings['SectorPerClustor'] % 256
    return result

def number_of_fats():
    result = bytearray(1)
    result[0] = format_settings['NumberOfFATs'] % 256
    return result

def root_entires():
    result = bytearray(2)
    result[0] = format_settings['RootEntires'] % 256
    result[1] = format_settings['RootEntires'] // 256
    return result

def load_kernel():
    result = bytearray(2)
    result[0] = format_settings['ReservedSectors'] % 256
    result[1] = format_settings['ReservedSectors'] // 256
    return result

def reserved_sectors():
    result = bytearray(2)
    result[0] = format_settings['ReservedSectors'] % 256
    result[1] = format_settings['ReservedSectors'] // 256
    return result

def volume_label():
    result = bytearray(11)
    for i in range(11):
        result[i] = 0x20
    textCode = format_settings['VolumeName']
    for (id,el) in enumerate(textCode):
        result[id] = ord(el)
    return result

def system_id():
    result = bytearray(8)
    for i in range(8):
        result[i] = 0x20
    textCode = 'FAT16'
    for (id,el) in enumerate(textCode):
        result[id] = ord(el)
    return result

def header():
    result = bytearray(512)
    result = merge(result, test_id(), 0x3)
    result = merge(result, bytes_per_sector(), 0xB)
    result = merge(result, sectors_per_cluster(), 0xD)
    result = merge(result, reserved_sectors(), 0xE)
    result = merge(result, number_of_fats(), 0x10)
    result = merge(result, root_entires(), 0x11)
    result = merge(result, sectors_per_fat(), 0x16)
    result = merge(result, volume_label(), 0x2B)
    result = merge(result, system_id(), 0x36)
    result[511] = 0x00
    result[510] = 0x00
    print_g(result)
    return result

def fat_size():
    if (format_settings['RootEntires'] * 32) % 512 != 0:
        print("RootEntires error [couldn't fit into sectors]")
        exit(0)
    root_dir_sectors = (format_settings['RootEntires'] * 32) // 512
    load_sectors = format_settings['ReservedSectors']
    data_sectors = file_descriptor['size'] // 512 - root_dir_sectors - load_sectors
    for fat_sectors in range(1, 256):
        free_data_sectors = data_sectors - fat_sectors * format_settings['NumberOfFATs']
        covered_data_sectors = fat_sectors * 512 // 2 - 2
        print(free_data_sectors, covered_data_sectors)
        if free_data_sectors <= format_settings['SectorPerClustor'] * covered_data_sectors:
            return fat_sectors
    return 256

def sectors_per_fat():
    fsize = fat_size()
    result = bytearray(2)
    result[0] = fsize % 256
    result[1] = fsize // 256
    return result

def fat():
    fsize = fat_size()
    print(fsize)
    result = bytearray(512 * fsize)
    result[0] = 0xf8
    result[1] = 0xff
    result[2] = 0xff
    result[3] = 0xff
    root_dir_sectors = (format_settings['RootEntires'] * 32) // 512
    load_sectors = format_settings['ReservedSectors']
    data_sectors = file_descriptor['size'] // 512 - root_dir_sectors - load_sectors
    free_data_sectors = data_sectors - fsize * format_settings['NumberOfFATs']
    covered_data_clusters = fsize * 512 // 2 - 2
    data_clusters = (free_data_sectors - format_settings['SectorPerClustor'] + 1) // format_settings['SectorPerClustor'] + 1
    unused_clusters = covered_data_clusters - data_clusters
    print(covered_data_clusters, data_clusters, unused_clusters)
    if unused_clusters < 0:
        print("Error with clusters")
        exit(0)
    for i in range(unused_clusters):
        result[512 * fsize - 2 * unused_clusters + 2 * i] = 0xff
        result[512 * fsize - 2 * unused_clusters + 2 * i + 1] = 0xff

    print_g(result)
    return result

def root_dir():
    result = bytearray(format_settings['RootEntires'] * 32)
    return result

def print_gh(el):
    if (el >= 10):
        return chr(el - 10 + ord('A'))
    return chr(el + ord('0'))

def print_g(result):
    for (id,el) in enumerate(result):
        if id % 16 == 0:
            print()
        print(print_gh(el//16), end="")
        print(print_gh(el%16), end=" ")


if __name__ == "__main__":
    print("oneOS Formatter")
    print("To change setting change config file")
    open_file()

    pos = 0

    print("writing header")
    print("Header starts at ", pos)
    # writing header
    header_e = header()
    write_file(header_e, pos)
    pos += format_settings['ReservedSectors'] * 512
    print("Kernel starts at ", 512)

    print("writing fats")
    print("Fats start at ", pos)
    # writing fats
    fat_e = fat()
    for i in range(format_settings['NumberOfFATs']):
        write_file(fat_e, pos)
        pos += len(fat_e)

    print("writing root_dir")
    print("Root dir starts at ", pos)
    # writing root_dir
    root_dir_e = root_dir()
    write_file(root_dir_e, pos)
    pos += len(root_dir_e)

    write_file(bytearray(file_descriptor['size'] - pos), pos)

    print("Data starts at ", pos)
    print("Data size is ", file_descriptor['size'] - pos)
    print("Data size is (Mb)", (file_descriptor['size'] - pos) / (1024 * 1024))
