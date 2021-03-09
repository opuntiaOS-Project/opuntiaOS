# The tool checks the compatability of linkern and libc bits/ structs

import sys
import os


def list_of_files_relative(opath):
    res = []

    def is_file_type(name, ending):
        if len(name) <= len(ending):
            return False
        return (name[-len(ending)-1::] == '.'+ending)

    for path, subdirs, files in os.walk(opath):
        for name in files:
            # It runs from out dir, at least it should
            file = path + "/" + name
            if is_file_type(file, 'h'):
                res.append(file[len(opath):])
    return res


def process_file(file, res_map):
    def accept_line(line):
        if line.endswith("\n"):
            line = line[:-1]

        if (len(line) == 0):
            return False

        block = [
            "#include",
            "#ifndef",
            "#endif",
            "#define _KERNEL_LIBKERN",
            "#define _LIBC",
        ]

        for b in block:
            if line.startswith(b):
                return False

        return True

    with open(file) as ofile:
        for line in ofile:
            if line.endswith("\n"):
                line = line[:-1]
            if accept_line(line):
                if line in res_map:
                    res_map[line] += 1
                else:
                    res_map[line] = 1


def create_map_of_lines(files):
    res_map = {}
    for f in files:
        process_file(f, res_map)
    return res_map


def check_files(pbase, pslave, files):
    filesbase = [pbase+x for x in files]
    filesslave = [pslave+x for x in files]
    libkern_map = create_map_of_lines(filesbase)
    libc_map = create_map_of_lines(filesslave)

    for i, x in libkern_map.items():
        if i in libc_map:
            if x != libc_map[i]:
                return False
        else:
            print("Can't find {0} in LibC".format(i))
            return False

    return True


libkern_files = list_of_files_relative("kernel/include/libkern/bits")
libc_files = list_of_files_relative("libs/libc/include/bits")

if len(libkern_files) != len(libc_files):
    print("Note: LibC and LibKern might not be compatible, taking LibKern as base")

if check_files("kernel/include/libkern/bits", "libs/libc/include/bits", libkern_files):
    print("OK")
else:
    print("Failed")
    exit(1)
