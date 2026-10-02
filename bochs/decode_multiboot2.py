#!/usr/bin/env python3
import struct, sys

MAGIC = 0xE85250D6
ARCH = {0: "i386", 4: "MIPS32"}
TAGS = {0: "end", 1: "information request", 2: "address", 3: "entry address",
        4: "console flags", 5: "framebuffer", 6: "module alignment",
        7: "EFI boot services", 8: "EFI i386 entry", 9: "EFI amd64 entry",
        10: "relocatable"}

def find_header(data):
    for off in range(0, min(len(data), 32768) - 16 + 1, 8):
        if struct.unpack_from("<I", data, off)[0] == MAGIC:
            return off
    raise SystemExit("no multiboot2 magic found in first 32 KiB")

def dump(data, base=0):
    off = base
    magic, arch, length, csum = struct.unpack_from("<4I", data, off)
    ok = (magic + arch + length + csum) & 0xFFFFFFFF == 0
    print(f"offset      : {off:#x}")
    print(f"magic       : {magic:#010x}")
    print(f"architecture: {arch} ({ARCH.get(arch, '?')})")
    print(f"length      : {length}")
    print(f"checksum    : {csum:#010x} ({'OK' if ok else 'BAD'})")

    end = off + length
    pos = off + 16
    while pos + 8 <= end:
        ttype, flags, size = struct.unpack_from("<HHI", data, pos)
        payload = data[pos + 8 : pos + size]
        name = TAGS.get(ttype, "unknown")
        opt = "optional" if flags & 1 else "required"
        print(f"\ntag {ttype}: {name} ({opt}, size {size})")
        if ttype == 1:
            n = len(payload) // 4
            print("  requests:", [hex(x) for x in struct.unpack(f"<{n}I", payload[:n*4])])
        elif ttype == 2:
            for k, v in zip(("header_addr", "load_addr", "load_end_addr", "bss_end_addr"),
                            struct.unpack("<4I", payload[:16])):
                print(f"  {k}: {v:#x}")
        elif ttype in (3, 8, 9):
            print(f"  entry_addr: {struct.unpack('<I', payload[:4])[0]:#x}")
        elif ttype == 4:
            print(f"  console_flags: {struct.unpack('<I', payload[:4])[0]:#x}")
        elif ttype == 5:
            w, h, d = struct.unpack("<3I", payload[:12])
            print(f"  width={w} height={h} depth={d}")
        elif ttype == 6:
            pass
        elif ttype == 10:
            mn, mx, align, pref = struct.unpack("<4I", payload[:16])
            print(f"  min={mn:#x} max={mx:#x} align={align:#x} preference={pref}")
        if ttype == 0:
            break
        pos += (size + 7) & ~7

if __name__ == "__main__":
    data = open(sys.argv[1], "rb").read()
    dump(data, find_header(data))