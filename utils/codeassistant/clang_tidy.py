import subprocess
import os
import sys

def run_clang_tidy(src, flags):
    cmd = ['clang-tidy', "--use-color"]
    cmd.extend(src)
    cmd.extend(["--"])
    cmd.extend(flags)
    result = subprocess.run(cmd, stdout=subprocess.PIPE)
    return result.stdout

def run_clang_tidy_kernel(src):
    print(run_clang_tidy(src, ["-I", "kernel/include", "-m32"]).decode("ascii"))

def get_kernel_files():
    res = []
    platforms = ['x86', 'aarch32']
    ignore_platforms = []

    for platform in platforms:
        if "x86" != platform:
            ignore_platforms.append(platform)

    def is_file_type(name, ending):
        if len(name) <= len(ending):
            return False
        return (name[-len(ending)-1::] == '.'+ending)

    def is_file_blocked(name):
        for platform in ignore_platforms:
            if (name.find(platform) != -1):
                return True
        return False

    for path, subdirs, files in os.walk("kernel/kernel"):
        for name in files:
            # It runs from out dir, at least it should
            file = path + "/" + name
            if not is_file_type(file, 'c'):
                continue
            if is_file_blocked(file):
                continue
            res.append(file)
    return res

kernel_files = get_kernel_files()
run_clang_tidy_kernel(kernel_files)