//
// Created by cocon on 29.10.2026.
//

#ifndef ICARUS_ELF_PARSE_H
#define ICARUS_ELF_PARSE_H
#include <stdint.h>

#include "kernel/Processes/Process.h"

#define EI_NIDENT 16
//access for e_ident
#define EI_MAG0 0x0
#define EI_MAG1 0x1
#define EI_MAG2 0x2
#define EI_MAG3 0x3
#define EI_CLASS 0x4
#define EI_DATA 0x5
#define EI_VERSION 0x6
#define EI_OSABI 0x7
#define EI_ABIVERSION 0x8
#define EI_PAD 0x9

// validation
#define ELF_MAGIC "\177ELF"
#define EM_X86_64 0x3E

// e_types
typedef enum {
    ET_NONE,
    ET_REL,
    ET_EXEC,
    ET_DYN,
}e_type_t;

// program header

// types
typedef enum {
    PT_NULL,
    PT_LOAD,
    PT_DYNAMIC,
    PT_INTERP,
    PT_NOTE,
    PT_SHLIB,
    PT_PHDR,
    PT_TLS,
    PT_LOOS,
    PT_HIOS,
    PT_LOPROC,
    PT_HIPROC,
} ep_type_t;


#define P_FLAG_EXECUTE 0x1
#define P_FLAG_WRITE 0x2
#define P_FLAG_READ 0x4


typedef struct E_IDENT{
    uint8_t ei_mag0;
    uint8_t ei_mag1;
    uint8_t ei_mag2;
    uint8_t ei_mag3;
    uint8_t ei_class;
    uint8_t ei_data;
    uint8_t ei_version;
    uint8_t ei_osabi;
    uint8_t ei_abi_version;
    uint8_t ei_pad[7];
} e_ident_t __attribute__((packed));


typedef struct {
    union {
        unsigned char e_ident_c[EI_NIDENT];
        e_ident_t e_ident;
    };
    uint16_t      e_type;
    uint16_t      e_machine;
    uint32_t      e_version;
    uint64_t      e_entry;
    uint64_t      e_phoff;
    uint64_t      e_shoff;
    uint32_t      e_flags;
    uint16_t      e_ehsize;
    uint16_t      e_phentsize;
    uint16_t      e_phnum;
    uint16_t      e_shentsize;
    uint16_t      e_shnum;
    uint16_t      e_shstrndx;
} elf_header_t;

typedef struct {
    uint32_t	p_type;
    uint32_t	p_flags;
    uint64_t	p_offset;
    uint64_t	p_vaddr;
    uint64_t	p_paddr;
    uint64_t	p_filesz;
    uint64_t	p_memsz;
    uint64_t	p_align;
} elf_program_header_t;

typedef struct ELF_FILE_HEADERS {
    elf_header_t header;
    elf_program_header_t program_headers[];
}elf_file_t;

int load_elf(elf_header_t * elf_file, PID_t current_pid, bool is_privileged, uint16_t priority);
int load_init_elf(elf_header_t * elf_file, PID_t current_pid, bool is_privileged, uint16_t priority);

#endif //ICARUS_ELF_PARSE_H
