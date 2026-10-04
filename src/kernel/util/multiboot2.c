#include "kernel/util/Multiboot2.h"

#include <stddef.h>
#include <stdint.h>
#include "../../../include/kernel/util/shellio.h"
#include "../Memory/memory_mapping.h"

#include "kernel/Memory/DMA/ACPI.h"
#include "kernel/util/mem_utils.h"


extern char _kernel_end_phys[];
static struct RSDP_t rsdp = {'\0'};
static struct XSDP_t rsdp_new = {'\0'};


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

	struct multiboot_tag *mmap = 0;

	if ((int)magic != MULTIBOOT2_BOOTLOADER_MAGIC)
	{
		Iprintf("Invalid magic number: 0x%i\n", (unsigned) magic);
		return -1;
	}

	if (addr & 7)
	{
		Iprintf( "Unaligned mbi: 0x%i\n", addr);
		return -1;
	}

	size = *(unsigned *) addr;
	Iprintf( "Announced mbi size 0x%d\n", size);
	for (tag = (struct multiboot_tag *) (addr + 8);
	tag->type != MULTIBOOT_TAG_TYPE_END;
	tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag
                                       + ((tag->size + 7) & ~7))) {
		//cons_mprintf( "Tag 0x%i, Size 0x%i\n", tag->type, tag->size);
		switch (tag->type)
		{
			case MULTIBOOT_TAG_TYPE_CMDLINE:
				Iprintf( "Command line = %s\n",
				((struct multiboot_tag_string *) tag)->string);
				break;
			case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
				Iprintf( "Boot loader name = %s\n",
				((struct multiboot_tag_string *) tag)->string);
				break;
			case MULTIBOOT_TAG_TYPE_MODULE:
				Iprintf( "Module at 0x%i-0x%i. Command line %s\n",
				((struct multiboot_tag_module *) tag)->mod_start,
				((struct multiboot_tag_module *) tag)->mod_end,
				((struct multiboot_tag_module *) tag)->cmdline);
				break;
			case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
				Iprintf( "mem_lower = %uKB, mem_upper = %uKB\n",
				((struct multiboot_tag_basic_meminfo *) tag)->mem_lower,
				((struct multiboot_tag_basic_meminfo *) tag)->mem_upper);
				break;
			case MULTIBOOT_TAG_TYPE_BOOTDEV:
				Iprintf( "Boot device 0x%i,%u,%u\n",
				((struct multiboot_tag_bootdev *) tag)->biosdev,
				((struct multiboot_tag_bootdev *) tag)->slice,
				((struct multiboot_tag_bootdev *) tag)->part);
				break;
			case MULTIBOOT_TAG_TYPE_MMAP:
				mmap = tag;
				break;

			case MULTIBOOT_TAG_TYPE_ACPI_OLD:
				/*
				 * copy acpi tag into rsdp bc mb2 info gets overwritten by the bitmap,
				 * however we need the Direct kernel mapping to correctly access and parse the struct
				 * if we don't want a #PF (I've met him he's not very nice )
				 */
				Imemccpy(&rsdp, tag + 1 , sizeof(struct RSDP_t));
				// acpi 1.0 (https://edc.intel.com/content/www/us/en/publications/specification-nuc12dcm-nuc12edb/acpi/)
				// rsdp & xsdp structs ( https://wiki.osdev.org/RSDP#Fields)
				break;

			case MULTIBOOT_TAG_TYPE_ACPI_NEW:
				// same as above
				Imemccpy(&rsdp_new , tag + 1, sizeof(struct XSDP_t));
				break;

			case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
				break;
			default:
			break;
			}
		}
		tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag
									  + ((tag->size + 7) & ~7));
		Iprintf( "Total mbi size 0x%i\n", (unsigned) tag - addr);


	if (mmap == 0) {
		Iprintf( "MB2 MMAP tag not found. stopping execution!\n");
		return -1;
	}

	if (handle_mb2_mmap(mmap, pml4)) {
		Iprintf("Error while parsing MB2 mmap tag and building physical mem alloc\n");
		return -1;
	}

	//handle_ACPI();

	return 0;
}

void handle_ACPI() {
	if (Istrncmp(rsdp_new.Signature, "\0", 1) != 0) {
		handle_new_acpi(&rsdp_new);
	}
	else if (Istrncmp(rsdp.Signature, "\0", 1) != 0) {
		handle_old_acpi(&rsdp);
	}
	else {
		Iprintf( "No RSDP found (ACPI devices are non functional)\n");
	}
}