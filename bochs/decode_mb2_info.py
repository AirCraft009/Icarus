#!/usr/bin/env python3
import struct, sys

MMAP_TYPES = {1: "available", 3: "ACPI reclaimable", 4: "NVS", 5: "defective"}
FB_TYPES = {0: "indexed", 1: "direct RGB", 2: "EGA text"}
TAG_NAMES = {
    1: "boot command line", 2: "bootloader name", 3: "module",
    4: "basic meminfo", 5: "BIOS boot device", 6: "memory map",
    7: "VBE info", 8: "framebuffer", 9: "ELF sections", 10: "APM table",
    11: "EFI32 system table", 12: "EFI64 system table", 13: "SMBIOS",
    14: "ACPI RSDP (old)", 15: "ACPI RDSP (new)", 16: "network",
    17: "EFI memory map", 18: "EFI boot services not terminated",
    19: "EFI32 image handle", 20: "EFI64 image handle",
    21: "image load base address",
}

def cstr(b):
    return b.split(b"\0", 1)[0].decode("utf-8", "replace")

def dump(data):
    total, reserved = struct.unpack_from("<II", data, 0)
    print(f"total_size: {total}  reserved: {reserved}")
    if total > len(data):
        print(f"warning: dump is only {len(data)} bytes")
    pos = 8
    while pos + 8 <= min(total, len(data)):
        ttype, size = struct.unpack_from("<II", data, pos)
        p = data[pos + 8 : pos + size]
        print(f"\n[{pos:#06x}] tag {ttype}: {TAG_NAMES.get(ttype, 'unknown')} (size {size})")
        if ttype == 0:
            break
        elif ttype in (1, 2):
            print(f"  {cstr(p)!r}")
        elif ttype == 3:
            s, e = struct.unpack_from("<II", p)
            print(f"  start={s:#x} end={e:#x} cmdline={cstr(p[8:])!r}")
        elif ttype == 4:
            lo, hi = struct.unpack_from("<II", p)
            print(f"  mem_lower={lo} KiB  mem_upper={hi} KiB")
        elif ttype == 5:
            dev, part, sub = struct.unpack_from("<III", p)
            print(f"  biosdev={dev:#x} partition={part:#x} sub_partition={sub:#x}")
        elif ttype == 6:
            esz, ever = struct.unpack_from("<II", p)
            print(f"  entry_size={esz} entry_version={ever}")
            for off in range(8, len(p) - esz + 1, esz):
                base, length, mtype, _ = struct.unpack_from("<QQII", p, off)
                print(f"  {base:#018x} - {base + length:#018x}  "
                      f"{length:>12} B  {MMAP_TYPES.get(mtype, f'reserved({mtype})')}")
        elif ttype == 8:
            addr, pitch, w, h, bpp, fbt = struct.unpack_from("<QIIIBB", p)
            print(f"  addr={addr:#x} pitch={pitch} {w}x{h}x{bpp} type={FB_TYPES.get(fbt, fbt)}")
            if fbt == 1:
                rp, rs, gp, gs, bp, bs = struct.unpack_from("<6B", p, 22)
                print(f"  red: pos={rp} size={rs}  green: pos={gp} size={gs}  blue: pos={bp} size={bs}")
        elif ttype == 9:
            num, entsize, shndx, _ = struct.unpack_from("<4H", p)
            print(f"  num={num} entsize={entsize} shndx={shndx}")
        elif ttype in (11, 12, 19, 20):
            fmt = "<I" if ttype in (11, 19) else "<Q"
            print(f"  pointer={struct.unpack_from(fmt, p)[0]:#x}")
        elif ttype == 13:
            print(f"  SMBIOS v{p[0]}.{p[1]}, {len(p) - 8} bytes of tables")
        elif ttype in (14, 15):
            print(f"  RSDP copy, {len(p)} bytes, signature={p[:8]!r}")
        elif ttype == 17:
            dsz, dver = struct.unpack_from("<II", p)
            print(f"  descriptor_size={dsz} version={dver} count={(len(p) - 8) // dsz if dsz else 0}")
        elif ttype == 21:
            print(f"  load_base_addr={struct.unpack_from('<I', p)[0]:#x}")
        pos += (size + 7) & ~7

if __name__ == "__main__":
    dump(open(sys.argv[1], "rb").read())