#!/usr/bin/env bash
# !!!!!!!!!!!! THIS FILE WAS MADE WITH THE HELP OF AI !!!!!!!!!!!!!!
# Build script for the Icarus kernel (replaces the kernel part of CMakeLists.txt).
#
# Usage:
#   ./build.sh          build the kernel ELF
#   ./build.sh iso      build the kernel and the bootable ISO
#   ./build.sh clean    remove the build directory
#
# Environment variables:
#   ICARUS_CC   compiler to use (default: x86_64-icarus-gcc)
#   ICARUS_NASM NASM executable to use (default: nasm)
#   BUILD_DIR   output directory (default: <project root>/build)

set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="$PROJECT_ROOT/x64/src/kernel"
LIBC_DIR="$PROJECT_ROOT/x64/src/icarus-libc"
USER_DIR="$PROJECT_ROOT/x64/src/userland"
INCLUDE_DIR="$PROJECT_ROOT/x64/include"
LIBC_INCLUDE_DIR="$INCLUDE_DIR/icarus-libc"
BUILD_DIR="${BUILD_DIR:-$PROJECT_ROOT/build}"
OBJECT_DIR="$BUILD_DIR/obj"
LINKER_SCRIPT="$SOURCE_DIR/linker.ld"
GRUB_CONFIG="$PROJECT_ROOT/grub.cfg"
INIT_RAM_FS="$PROJECT_ROOT/root-fs"

KERNEL_ELF="$BUILD_DIR/Icarus_kernel.elf"
ISO_DIR="$BUILD_DIR/iso"
ISO_FILE="$BUILD_DIR/icarus.iso"
TAR_FILE="$BUILD_DIR/iso/boot/initramfs.tar"
BUILT_TAR_FILE="$INIT_RAM_FS/initramfs.tar"

COMPILER="${ICARUS_CC:-x86_64-icarus-gcc}"
ASSEMBLER_NASM="${ICARUS_NASM:-nasm}"

# Flags for C sources (same as target_compile_options in CMake).
C_FLAGS=(
    -m64
    -ffreestanding
    -fno-pie
    -fno-pic
    -fno-stack-protector
    -fno-asynchronous-unwind-tables
    -mno-red-zone
    -mgeneral-regs-only
    -Wall -Wextra
    -mcmodel=kernel
    -I"$INCLUDE_DIR/"
    -I"$LIBC_INCLUDE_DIR/"
)

# Assembly sources (.S) only get the basics, like in the CMake file.
ASM_FLAGS=(
    -m64
    -I"$INCLUDE_DIR/"
    -I"$LIBC_INCLUDE_DIR/"
)

# NASM sources (.asm). The trailing slash on the include path is required by NASM.
NASM_FLAGS=(
    -f elf64
    -I"$INCLUDE_DIR/"
    -I"$LIBC_INCLUDE_DIR/"
)

# Flags for linking (same as target_link_options in CMake).
LINK_FLAGS=(
    -m64
    -nostdlib
    -static
    -no-pie
    -fno-pie
    -fno-pic
    -z max-page-size=0x1000
    -T "$LINKER_SCRIPT"
)

build_kernel() {
    local object_files=()
    rm -rf "$BUILD_DIR"
    mkdir -p "$OBJECT_DIR"

    echo "building kernel src"
    while IFS= read -r -d '' source_file; do
        local relative_path="${source_file#"$SOURCE_DIR"/}"
        local object_file="$OBJECT_DIR/${relative_path}.o"
        mkdir -p "$(dirname "$object_file")"

        echo "CC  $relative_path"
        case "$source_file" in
            *.c) "$COMPILER" "${C_FLAGS[@]}" -c "$source_file" -o "$object_file" ;;
            *.S) "$COMPILER" "${ASM_FLAGS[@]}" -c "$source_file" -o "$object_file" ;;
            *.asm) "$ASSEMBLER_NASM" "${NASM_FLAGS[@]}" "$source_file" -o "$object_file" ;;
        esac
        object_files+=("$object_file")
    done < <(find "$SOURCE_DIR" \( -name '*.c' -o -name '*.S' -o -name '*.asm' \) -print0 | sort -z)

    if [ "${#object_files[@]}" -eq 0 ]; then
        echo "error: no .c, .S or .asm files found in $SOURCE_DIR" >&2
        exit 1
    fi

    echo "building LIBC for the kernel as util"
    while IFS= read -r -d '' source_file; do
        local relative_path="${source_file#"$LIBC_DIR"/}"
        local object_file="$OBJECT_DIR/${relative_path}.o"
        mkdir -p "$(dirname "$object_file")"

        echo "CC  $relative_path"
        case "$source_file" in
            *.c) "$COMPILER" "${C_FLAGS[@]}" -c "$source_file" -o "$object_file" ;;
            *.S) "$COMPILER" "${ASM_FLAGS[@]}" -c "$source_file" -o "$object_file" ;;
            *.asm) "$ASSEMBLER_NASM" "${NASM_FLAGS[@]}" "$source_file" -o "$object_file" ;;
        esac
        object_files+=("$object_file")
    done < <(find "$LIBC_DIR" \( -name '*.c' -o -name '*.S' -o -name '*.asm' \) -print0 | sort -z)

    if [ "${#object_files[@]}" -eq 0 ]; then
        echo "error: no .c, .S or .asm files found in $LIBC_DIR" >&2
        exit 1
    fi

    echo "LD  $KERNEL_ELF"
    "$COMPILER" "${LINK_FLAGS[@]}" "${object_files[@]}" -o "$KERNEL_ELF"
}

build_user() {
    mkdir -p "$INIT_RAM_FS"

    echo "building user programs for initramfs"
    while IFS= read -r -d '' source_file; do
        local relative_path="${source_file#"$USER_DIR"/}"
        local binary_file="$INIT_RAM_FS/bin/${relative_path}.o"
        mkdir -p "$(dirname "$binary_file")"

        echo "CC  $relative_path"
        case "$source_file" in
            *.c) "$COMPILER" -static "$source_file" -o "$binary_file" ;;
            *.S) "$COMPILER" -static "$source_file" -o "$binary_file" ;;
        esac
    done < <(find "$USER_DIR" \( -name '*main.c' -o -name '*main.S' \) -print0 | sort -z)
}

stage_iso() {
    mkdir -p "$ISO_DIR/boot/grub"
    cp "$KERNEL_ELF" "$ISO_DIR/boot/Icarus_kernel.elf"
    cp "$GRUB_CONFIG" "$ISO_DIR/boot/grub/grub.cfg"
}

build_iso() {
    echo "ISO $ISO_FILE"
    grub-mkrescue -o "$ISO_FILE" "$ISO_DIR"
}

build_tar(){
  stage_iso
  build_user
  echo "$INIT_RAM_FS"
  tar --format=ustar --owner=0 --group=0 --numeric-owner -cvf "$INIT_RAM_FS/initramfs.tar" -C "$INIT_RAM_FS/src" .
  echo "TAR $TAR_FILE"
  cp "$BUILT_TAR_FILE" "$TAR_FILE"
}

clean() {
    rm -rf "$BUILD_DIR"
}

case "${1:-kernel}" in
    kernel)       build_kernel; stage_iso ;;
    iso)          build_kernel; stage_iso; build_iso ;;
    full_iso)     build_kernel; build_tar; build_iso;;
    clean)        clean ;;
    *)
        echo "usage: $0 [kernel|iso|clean]" >&2
        exit 1
        ;;
esac