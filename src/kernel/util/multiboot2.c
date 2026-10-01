#include "kernel/util/multiboot2.h"

#include <stdint.h>
#include "kernel/util/shellio.h"
#include "../Memory/memory_mapping.h"
#include "kernel/Memory/DMA/ACPI.h"
#include "kernel/util/mem_utils.h"

static struct RSDP_t *find_rsdp(void) {
	uint16_t ebda_seg = *(volatile uint16_t *)0x40E;
	uintptr_t ranges[2][2] = {
		{ (uintptr_t)ebda_seg << 4, ((uintptr_t)ebda_seg << 4) + 1024 },
		{ 0xE0000, 0x100000 },
	};
	for (int r = 0; r < 2; r++)
		for (uintptr_t p = ranges[r][0]; p < ranges[r][1]; p += 16)
			if (Imemcmp((void *)p, "RSD PTR ", 8) == 0) {
				// verify checksum: first 20 bytes sum to 0 mod 256
				uint8_t sum = 0;
				for (int i = 0; i < 20; i++) sum += ((uint8_t *)p)[i];
				if (sum == 0) return (struct RSDP_t *)p;
			}
	return 0;
}

extern char _kernel_end_phys[];

/**
 * handles parsing the info struct passed by GRUB multiboot2
 *	- checks for errors in the boot up process
 *	- prints utility to the screen
 *	- initiates bitmap
 */
int handle_multiboot2 (uint32_t magic, boot_info *info, page_map_l4_entry *pml4){
	unsigned long long addr = (unsigned long long) info;
	struct multiboot_tag *tag;
	unsigned long size;
	struct RSDP_t *rsdp_old = (struct RSDP_t *) 0;
	struct XSDP_t *rsdp_new = (struct XSDP_t *) 0;

	cursor cur = (cursor) {.x = 0,.y = 0};

	multiboot_memory_map_t *mmap;


	if ((int)magic != MULTIBOOT2_BOOTLOADER_MAGIC)
	{
		cons_mprintf("Invalid magic number: 0x%i\n", (unsigned) magic);
		return -1;
	}

	if (addr & 7)
	{
		cons_mprintf( "Unaligned mbi: 0x%i\n", addr);
		return -1;
	}

	size = *(unsigned *) addr;
	cons_mprintf( "Announced mbi size 0x%d\n", size);
	cons_mprintf("MBI: %x - %x (size %x)\n", (unsigned) addr, (unsigned)(addr + size), size);
	cons_mprintf("kernel_end: %x\n", &_kernel_end_phys);
	for (tag = (struct multiboot_tag *) (addr + 8);
	tag->type != MULTIBOOT_TAG_TYPE_END;
	tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag
                                       + ((tag->size + 7) & ~7))) {
		//cons_mprintf( "Tag 0x%i, Size 0x%i\n", tag->type, tag->size);
		switch (tag->type)
		{
			case MULTIBOOT_TAG_TYPE_CMDLINE:
				cons_mprintf( "Command line = %s\n",
				((struct multiboot_tag_string *) tag)->string);
				break;
			case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
				cons_mprintf( "Boot loader name = %s\n",
				((struct multiboot_tag_string *) tag)->string);
				break;
			case MULTIBOOT_TAG_TYPE_MODULE:
				cons_mprintf( "Module at 0x%i-0x%i. Command line %s\n",
				((struct multiboot_tag_module *) tag)->mod_start,
				((struct multiboot_tag_module *) tag)->mod_end,
				((struct multiboot_tag_module *) tag)->cmdline);
				break;
			case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
				cons_mprintf( "mem_lower = %uKB, mem_upper = %uKB\n",
				((struct multiboot_tag_basic_meminfo *) tag)->mem_lower,
				((struct multiboot_tag_basic_meminfo *) tag)->mem_upper);
				break;
			case MULTIBOOT_TAG_TYPE_BOOTDEV:
				cons_mprintf( "Boot device 0x%i,%u,%u\n",
				((struct multiboot_tag_bootdev *) tag)->biosdev,
				((struct multiboot_tag_bootdev *) tag)->slice,
				((struct multiboot_tag_bootdev *) tag)->part);
				break;
			case MULTIBOOT_TAG_TYPE_MMAP:
				if (handle_mb2_mmap(tag, pml4) != 0)
					return -1;
				break;

			case MULTIBOOT_TAG_TYPE_ACPI_OLD:
				rsdp_old = (struct RSDP_t *) (tag + 1);
				// acpi 1.0 (https://edc.intel.com/content/www/us/en/publications/specification-nuc12dcm-nuc12edb/acpi/)
				// rsdp struct ( https://wiki.osdev.org/RSDP#Fields)
				break;

			case MULTIBOOT_TAG_TYPE_ACPI_NEW:
				rsdp_new = (struct XSDP_t *) (tag + 1);
				break;

			case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
				break;
			default:
			break;
			}
		}
		tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag
									  + ((tag->size + 7) & ~7));
		cons_mprintf( "Total mbi size 0x%i\n", (unsigned) tag - addr);

		if (rsdp_new != 0) {
			handle_new_acpi(rsdp_new);
		}
		else if (rsdp_old != 0) {
			handle_old_acpi(rsdp_old);
		}
		else {
			cons_mprintf( "No RSDP found by GRUB (trying manual scan)\n");
			if (find_rsdp() != 0) {
				cons_mprintf("manual scan found ACPI\n");
				return 0;
			};
			cons_mprintf( "No RSDP found (ACPI devices are non functional)\n");
			return 0;
		}

		return 0;
	}