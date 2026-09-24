#include "multiboot2.h"

#include <stddef.h>
#include <stdint.h>
#include "../shell/shellio.h"
#include "MMU/memory_mapping.h"

/**
 * handles parsing the info struct passed by GRUB multiboot2
 *	- checks for errors in the boot up process
 *	- prints utility to the screen
 *	- initiates bitmap
 */
mem_map *handle_multiboot2 (uint32_t magic, boot_info *info){
	unsigned long long addr = (unsigned long long) info;
	struct multiboot_tag *tag;
	unsigned long size;

	cursor cur = (cursor) {.x = 0,.y = 0};

	multiboot_memory_map_t *mmap;

	if ((int)magic != MULTIBOOT2_BOOTLOADER_MAGIC)
	{
		cons_mprintf("Invalid magic number: 0x%i\n", (unsigned) magic);
		return NULL;
	}

	if (addr & 7)
	{
		cons_mprintf( "Unaligned mbi: 0x%i\n", addr);
		return NULL;
	}

	size = *(unsigned *) addr;
	cons_mprintf( "Announced mbi size 0x%d\n", size);
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
				init_mmap(tag);
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

		return NULL;
	}