#!/bin/bash

# configure
BINUTILSURL="https://ftp.gnu.org/gnu/binutils/binutils-2.38.tar.gz"
GCCURL="https://ftp.gnu.org/gnu/gcc/gcc-12.1.0/gcc-12.1.0.tar.gz"
QEMUURL="https://download.qemu.org/qemu-7.0.0.tar.xz"
GDBURL="https://ftp.gnu.org/gnu/gdb/gdb-12.1.tar.gz"
OPT="-j8"

GCCTARGET="aarch64-none-elf"
QEMUTARGET="aarch64-softmmu"

# by default it is built and installed into the local repo
# change PREFIX to chose another directory

PREFIX="$PWD/cross"
SOURCES="$PREFIX/src"
BUILD="$PREFIX/build"
BIN="$PREFIX/out"

BASE=$PWD

mkdir $PREFIX
mkdir $SOURCES
mkdir $BUILD

PATH="$PATH:$BIN/bin"

cd $SOURCES

wget $BINUTILSURL -O binutils.tar.gz
wget $GCCURL -O gcc.tar.gz
wget $QEMUURL -O qemu.tar.xz
wget $GDBURL -O gdb.tar.gz

tar xvf binutils.tar.gz
tar xvf gcc.tar.gz
tar xvf qemu.tar.xz
tar xvf gdb.tar.gz

# build binutils
cd $BUILD
mkdir binutils
cd binutils
$SOURCES/binutils*/configure --target=$GCCTARGET --prefix=$BIN
make $OPT
make install

# build gcc
cd $BUILD
mkdir gcc
cd gcc
$SOURCES/gcc*/configure --target=$GCCTARGET --prefix=$BIN
make $OPT
make install

# build qemu
cd $BUILD
mkdir qemu
cd qemu
$SOURCES/qemu*/configure --target-list=$QEMUTARGET --prefix=$BIN
make $OPT
make install

# build gdb
cd $BUILD
mkdir gdb
cd gdb
$SOURCES/gdb*/configure --target=$GCCTARGET --prefix=$BIN
make $OPT
make install

cd $BASE
echo "PATH=\$PATH:$BIN/bin" > activateccenv
