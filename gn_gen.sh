#!/bin/bash -e

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'
ERROR="${RED}[ERROR]${NC}"
SUCCESS="${GREEN}[SUCCESS]${NC}"
OUTDIR="out"
FORCEY="NO"
GNARGS=""

function main() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                print_help
                ;;
            -y|--yes)
                FORCEY="YES"
                shift
                ;;
            --dir)
                OUTDIR="$2"
                shift
                shift
                ;;
            # All other flags are passed to GN.
            --*)
                GNARGS+="${1:2}="
                shift
                ;;
            *)
                GNARGS+="\"$1\""
                shift
                ;;
        esac
    done

    if [ -d "$OUTDIR" ]; then
        if [[ $FORCEY != "YES" ]]; then
            read -p "Are you sure you want to overwrite all files in ${OUTDIR} (y/N): " -n 1 -r
            echo
            if [[ $REPLY =~ ^[Yy]$ ]]; then
                rm -rf $OUTDIR
            else
                echo "Exiting..."
                exit
            fi
        fi
    fi

    gn gen $OUTDIR --args=$GNARGS
    if [ $? -ne 0 ]; then echo -e "${ERROR} Can't do gn gen" && exit 1; fi
    ninja -C $OUTDIR scripts
    if [ $? -ne 0 ]; then echo -e "${ERROR} Can't do ninja -C ${OUTDIR} scripts" && exit 1; fi
    chmod +x $OUTDIR/build.sh
    chmod +x $OUTDIR/run.sh
    chmod +x $OUTDIR/sync.sh
    chmod +x $OUTDIR/all.sh
    chmod +x $OUTDIR/run_tester.sh
    chmod +x $OUTDIR/debug.sh
    chmod +x $OUTDIR/dll.sh

    #building one.img
    IMAGE_SIZE=64M
    qemu-img create -f raw $OUTDIR/one.img $IMAGE_SIZE
    if [ $? -ne 0 ]; then echo -e "${ERROR} Can't create an one.img" && exit 1; fi
    MKFS="" # Provide path here
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        MKFS=mkfs
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        MKFS=/usr/local/opt/e2fsprogs/sbin/mkfs.ext2
    else
        echo "Please provide path to MKFS in MKFSBIN env variable"
    fi
    $MKFS -t ext2 -r 0 -b 1024 $OUTDIR/one.img
    if [ $? -ne 0 ]; then echo -e "${ERROR} Can't create an one.img" && exit 1; fi
    echo -e "${SUCCESS} Generated files with args: $*"

}

function print_help() {
    echo "--target_cpu {arch}
    Sets target arch.
    Possible values:
        x86 (default)
        x86_64
        arm32 / arm
        arm64 / aarch64

--host {host}
    Sets toolchain to build the OS.
    Possible values:
        gnu (default)
        llvm

--device_type {type}
    Configueres OS parts to work in either desktop or mobile mode.
    Possible values:
        d / desktop (default)
        m / mobile

--test_method {method}
    Possible values:
        none (default)
        tests
        bench

--target_board {board}
    Possible values (arm64):
        qemu-virt (default)
        apl

--dir {name}
    Creates the build directory with the given {name}.

-y|--yes
    Force yes all operations.

-h|--help
    Prints all options of ./gn_gen.sh"
        exit
}

main "$@"; exit