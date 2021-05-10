#!/bin/bash

# i686-elf-tools.sh
# v1.3

# Define Global Variables

BINUTILS_VERSION=2.35
GCC_VERSION=11.1.0
GDB_VERSION=9.2

BUILD_TARGET="i686-elf"

set -e

ALL_PRODUCTS=true

# Parse Commandline Options

if [ $# -eq 0 ]; then
    BUILD_BINUTILS=true
    BUILD_GCC=true
    BUILD_GDB=true
    ZIP=true
    
    args="binutils gcc gdb zip"
else
    args=$@
fi

while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    binutils)               BUILD_BINUTILS=true;   ALL_PRODUCTS=false; shift ;;
    gcc)                    BUILD_GCC=true;        ALL_PRODUCTS=false; shift ;;
    gdb)                    BUILD_GDB=true;        ALL_PRODUCTS=false; shift ;;
    win)                    WINDOWS_ONLY=true;                         shift ;;
    linux)                  LINUX_ONLY=true;                           shift ;;
    zip)                    ZIP=true;              ALL_PRODUCTS=false; shift ;;
    env)                    ENV_ONLY=true;                             shift ;;
    -64)                    x64=true;                                  shift ;;
    -bv|--binutils-version) BINUTILS_VERSION="$2";                     shift; shift ;;
    -gv|--gcc-version)      GCC_VERSION="$2";                          shift; shift ;;
    -dv|--gdb-version)      GDB_VERSION="$2";                          shift; shift ;;
    *)                                                                 shift ;;
esac
done

if [[ $x64 == true ]]; then
    BUILD_TARGET="x86_64-elf"
fi

BUILD_DIR="$HOME/build-${BUILD_TARGET}"
export PATH="/opt/mxe/usr/bin:$BUILD_DIR/linux/output/bin:$BUILD_DIR/windows/output/bin:$PATH"

echo "BUILD_TARGET     = ${BUILD_TARGET}"
echo "BUILD_DIR        = ${BUILD_DIR}"
echo "BUILD_BINUTILS   = ${BUILD_BINUTILS}"
echo "BUILD_GCC        = ${BUILD_GCC}"
echo "BUILD_GDB        = ${BUILD_GDB}"
echo "ZIP              = ${ZIP}"
echo "WIN              = ${WINDOWS_ONLY}"
echo "LINUX            = ${LINUX_ONLY}"
echo "ENV              = ${ENV_ONLY}"
echo "x64              = ${x64}"
echo "BINUTILS_VERSION = ${BINUTILS_VERSION}"
echo "GCC_VERSION      = ${GCC_VERSION}"
echo "GDB_VERSION      = ${GDB_VERSION}"
echo "PATH             = ${PATH}"

function main {

    installPackages
    installMXE
    
    if [[ $ENV_ONLY == true ]]; then
        echoColor "Successfully installed build environment. Exiting as 'env' only was specified"
        return
    fi
    
    downloadSources
    
    if [[ $WINDOWS_ONLY == true ]]; then
        echoColor "Skipping compiling Linux as 'win' was specified in commandline args '$args'"
    else    
        compileAll "linux"
    fi
    
    if [[ $LINUX_ONLY == true ]]; then
        echoColor "Skipping compiling Windows as 'linux' was specified in commandline args '$args'"
    else    
        compileAll "windows"
    fi
        
    finalize
}

function installPackages {
    
    echoColor "Installing packages"

    sudo -E apt-get -qq install git \
        autoconf automake autopoint bash bison bzip2 flex gettext\
        g++ gperf intltool libffi-dev libgdk-pixbuf2.0-dev \
        libtool libltdl-dev libssl-dev libxml-parser-perl make \
        openssl p7zip-full patch perl pkg-config python ruby scons \
        sed unzip wget xz-utils libtool-bin texinfo g++-multilib lzip -y
}

# MXE

function installMXE {

    echoColor "Installing MXE"

    if [ ! -d "/opt/mxe/usr/bin" ]
    then
        echoColor "    Cloning MXE and compiling mingw32.static GCC"
        cd /opt
        sudo -E git clone https://github.com/mxe/mxe.git
        cd mxe
        sudo make gcc
    else
       echoColor "    MXE is already installed. You'd better make sure that you've previously made MXE's gcc! (/opt/mxe/usr/bin/i686-w64-mingw32.static-gcc)"
    fi
}

# Downloads

function downloadSources {
    mkdir -p $BUILD_DIR
    cd $BUILD_DIR
    
    echoColor "Downloading all sources"
    
    if [[ $BUILD_BINUTILS == true || $ALL_PRODUCTS == true ]]; then
        downloadAndExtract "binutils" $BINUTILS_VERSION
    else
        echoColor "    Skipping binutils as 'binutils' was ommitted from commandline args '$args'"
    fi
    
    if [[ $BUILD_GCC == true || $ALL_PRODUCTS == true ]]; then
        downloadAndExtract "gcc" $GCC_VERSION "http://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz"
        
        echoColor "        Downloading GCC prerequisites"
        
        # Automatically download GMP, MPC and MPFR. These will be placed into the right directories.
        # You can also download these separately, and specify their locations as arguments to ./configure
        
        if [[ $WINDOWS_ONLY != true ]]; then
            echoColor "            Linux"
            cd ./linux/gcc-$GCC_VERSION
            ./contrib/download_prerequisites
        fi
        
        cd $BUILD_DIR
        
        if [[ $LINUX_ONLY != true ]]; then
        echoColor "            Windows"
            cd ./windows/gcc-$GCC_VERSION
            ./contrib/download_prerequisites
        fi
        
        cd $BUILD_DIR
    else
        echoColor "    Skipping gcc as 'gcc' was ommitted from commandline args '$args'"
    fi
    
    if [[ $BUILD_GDB == true || $ALL_PRODUCTS == true ]]; then
        downloadAndExtract "gdb" $GDB_VERSION
    else
       echoColor "    Skipping gdb as 'gdb' was ommitted from commandline args '$args'" 
    fi
}

function downloadAndExtract {
    name=$1
    version=$2
    override=$3
    
    echoColor "    Processing $name"
    
    if [ ! -f $name-$version.tar.gz ]
    then
        echoColor "        Downloading $name-$version.tar.gz"
        
        if [ -z $3 ]
        then
            wget -q http://ftp.gnu.org/gnu/$name/$name-$version.tar.gz
        else
            wget -q $override
        fi
    else
        echoColor "        $name-$version.tar.gz already exists"
    fi

    if [[ $WINDOWS_ONLY == true ]]; then
        echoColor "        Skipping extracting Linux as 'win' was specified in commandline args '$args'"
    else
        mkdir -p linux
        cd linux
        
        if [ ! -d $name-$version ]
        then
            echoColor "        [linux]   Extracting $name-$version.tar.gz"
            tar -xf ../$name-$version.tar.gz
        else
            echoColor "        [linux]   Folder $name-$version already exists"
        fi
        
        cd ..
    fi
    
    if [[ $LINUX_ONLY == true ]]; then
        echoColor "        Skipping extracting Linux as 'win' was specified in commandline args '$args'"
    else
        mkdir -p windows
        cd windows
        
        if [ ! -d $name-$version ]
        then
            echoColor "        [windows] Extracting $name-$version.tar.gz"
            tar -xf ../$name-$version.tar.gz
        else
            echoColor "        [windows] Folder $name-$version already exists"        
        fi
        
        cd ..
    fi
}

function compileAll {

    echoColor "Compiling all $1"
    
    cd $1
    
    mkdir -p output

    compileBinutils $1
    compileGCC $1
    compileGDB $1
    
    cd ..
}

function compileBinutils {    
    if [[ $BUILD_BINUTILS == true || $ALL_PRODUCTS == true ]]; then
        echoColor "    Compiling binutils [$1]"
    
        mkdir -p build-binutils-$BINUTILS_VERSION
        cd build-binutils-$BINUTILS_VERSION
        
        configureArgs="--target=${BUILD_TARGET} --with-sysroot --disable-nls --disable-werror --prefix=$BUILD_DIR/$1/output"
        
        if [ $1 == "windows" ]
        then
            configureArgs="--host=i686-w64-mingw32.static $configureArgs"
        fi
        
        # Configure
        echoColor "        Configuring binutils (binutils_configure.log)"
        ../binutils-$BINUTILS_VERSION/configure $configureArgs >> binutils_configure.log
        
        # Make
        echoColor "        Making (binutils_make.log)"
        make >> binutils_make.log
        
        # Install
        echoColor "        Installing (binutils_install.log)"
        sudo make install >> binutils_install.log
        cd ..
    else
        echoColor "    Skipping binutils [$1] as 'binutils' was ommitted from commandline args '$args'"
    fi
}

function compileGCC {
    if [[ $BUILD_GCC == true || $ALL_PRODUCTS == true ]]; then
    
        echoColor "    Compiling gcc [$1]"

        mkdir -p build-gcc-$GCC_VERSION
        cd build-gcc-$GCC_VERSION
        
        configureArgs="--target=${BUILD_TARGET} --disable-nls --enable-languages=c,c++ --without-headers --prefix=$BUILD_DIR/$1/output"
        
        if [ $1 == "windows" ]
        then
            configureArgs="--host=i686-w64-mingw32.static $configureArgs"
        fi
        
        if [[ $x64 == true ]]; then
        
            # https://wiki.osdev.org/Libgcc_without_red_zone#Preparations
            
            echoColor "        Installing config/i386/t-x86_64-elf"
            echo -e "# Add libgcc multilib variant without red-zone requirement\n\nMULTILIB_OPTIONS += mno-red-zone\nMULTILIB_DIRNAMES += no-red-zone" > ../gcc-$GCC_VERSION/gcc/config/i386/t-x86_64-elf
            
            echoColor "        Patching gcc/config.gcc"
            sed -i '/x86_64-\*-elf\*)/a \\ttmake_file="${tmake_file} i386/t-x86_64-elf" # include the new multilib configuration' ../gcc-$GCC_VERSION/gcc/config.gcc
        fi
        
        # Configure
        echoColor "        Configuring gcc (gcc_configure.log)"
        ../gcc-$GCC_VERSION/configure $configureArgs >> gcc_configure.log
        
        # Make GCC
        echoColor "        Making gcc (gcc_make.log)"
        make all-gcc >> gcc_make.log
        
        # Install GCC
        echoColor "        Installing gcc (gcc_install.log)"
        sudo make install-gcc >> gcc_install.log
        
        # Make libgcc
        echoColor "        Making libgcc (libgcc_make.log)"
        make all-target-libgcc >> libgcc_make.log
        
        # Install libgcc
        echoColor "        Installing libgcc (libgcc_install.log)"
        sudo make install-target-libgcc >> libgcc_install.log
                
        if [[ $x64 == true ]]; then
        
            if [ $1 == "windows" ]
            then
                # no-red-zone doesn't appear to get installed by make install-target-libgcc for some reason. Manually install it ourselves
                
                cd "${BUILD_TARGET}/no-red-zone/libgcc"
                sudo make install >> ../../../libgcc_install_noredzone.log
                
                cd ../../..
            fi
        
            if [[ ! -d "../output/lib/gcc/x86_64-elf/$GCC_VERSION/no-red-zone" ]]; then
                echoError "ERROR: no-red-zone was not created. x64 patching failed"
                exit 1
            else
                echoColor "            Successfully compiled for no-red-zone"
            fi
        fi
        
        cd ..
    else
        echoColor "    Skipping gcc [$1] as 'gcc' was ommitted from commandline args '$args'"
    fi
}

function compileGDB {
    if [[ $BUILD_GDB == true || $ALL_PRODUCTS == true ]]; then

        echoColor "    Compiling gdb [$1]"
    
        configureArgs="--target=${BUILD_TARGET} --disable-nls --disable-werror --prefix=$BUILD_DIR/$1/output"
        
        if [ $1 == "windows" ]
        then
            configureArgs="--host=i686-w64-mingw32.static $configureArgs"
        fi
    
        mkdir -p build-gdb-$GDB_VERSION
        cd build-gdb-$GDB_VERSION
        
        # Configure        
        echoColor "        Configuring (gdb_configure.log)"
        ../gdb-$GDB_VERSION/configure $configureArgs >> gdb_configure.log
        
        # Make
        echoColor "        Making (gdb_make.log)"
        make >> gdb_make.log
        
        # Install
        echoColor "        Installing (gdb_install.log)"
        sudo make install >> gdb_install.log
        cd ..
    else
        echoColor "    Skipping gdb [$1] as 'gdb' was ommitted from commandline args '$args'"
    fi
}

function finalize {
    if [[ $ZIP == true || $ALL_PRODUCTS == true ]]; then
        echo "Zipping everything up!"
        
        if [[ -d "$BUILD_DIR/windows/output" ]]; then
            cd $BUILD_DIR/windows/output
            zip -r "${BUILD_DIR}/${BUILD_TARGET}-tools-windows.zip" *
        fi
        
        if [[ -d "$BUILD_DIR/linux/output" ]]; then
            cd $BUILD_DIR/linux/output
            zip -r "${BUILD_DIR}/${BUILD_TARGET}-tools-linux.zip" *
        fi
        
        echo -e "\e[92mZipped everything to $BUILD_DIR/${BUILD_TARGET}-tools-[windows | linux].zip\e[39m"
    else
        echoColor "    Skipping zipping 'zip' was ommitted from commandline args '$args'"
    fi
}

function echoColor {
    echo -e "\e[96m$1\e[39m"
}

function echoError {
    echo -e "\e[31m$1\e[39m"
}

main
