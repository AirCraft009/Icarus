#include "multiboot2.h"

#include <stddef.h>
#include <stdint.h>
#include "../shell/shellio.h"

// 20 chars
bitmap *handle_multiboot2 (uint32_t magic, struct multiboot_info *info){
	bitmap *map;
	unsigned long long addr = (unsigned long long) info;
	struct multiboot_tag *tag;
	unsigned long size;

	cursor cur = (cursor) {.x = 0,.y = 0};

	multiboot_memory_map_t *mmap;

	if ((int)magic != MULTIBOOT2_BOOTLOADER_MAGIC)
	{
		mprintf(&cur,"Invalid magic number: 0x%i\n", (unsigned) magic);
		return NULL;
	}

	if (addr & 7)
	{
		mprintf(&cur, "Unaligned mbi: 0x%i\n", addr);
		return NULL;
	}

	size = *(unsigned *) addr;
	mprintf(&cur, "Announced mbi size 0x%d\n", size);
	for (tag = (struct multiboot_tag *) (addr + 8);
	tag->type != MULTIBOOT_TAG_TYPE_END;
	tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag 
                                       + ((tag->size + 7) & ~7)))

	{
		mprintf(&cur, "Tag 0x%i, Size 0x%i\n", tag->type, tag->size);
		switch (tag->type)
		{
			case MULTIBOOT_TAG_TYPE_CMDLINE:
				mprintf(&cur, "Command line = %s\n",
				((struct multiboot_tag_string *) tag)->string);
				break;
			case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
				mprintf(&cur, "Boot loader name = %s\n",
				((struct multiboot_tag_string *) tag)->string);
				break;
			case MULTIBOOT_TAG_TYPE_MODULE:
				mprintf(&cur, "Module at 0x%i-0x%i. Command line %s\n",
				((struct multiboot_tag_module *) tag)->mod_start,
				((struct multiboot_tag_module *) tag)->mod_end,
				((struct multiboot_tag_module *) tag)->cmdline);
				break;
			case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
				mprintf(&cur, "mem_lower = %uKB, mem_upper = %uKB\n",
				((struct multiboot_tag_basic_meminfo *) tag)->mem_lower,
				((struct multiboot_tag_basic_meminfo *) tag)->mem_upper);
				break;
			case MULTIBOOT_TAG_TYPE_BOOTDEV:
				mprintf(&cur, "Boot device 0x%i,%u,%u\n",
				((struct multiboot_tag_bootdev *) tag)->biosdev,
				((struct multiboot_tag_bootdev *) tag)->slice,
				((struct multiboot_tag_bootdev *) tag)->part);
				break;
			case MULTIBOOT_TAG_TYPE_MMAP:
			{
				multiboot_memory_map_t *mmap;

				mprintf(&cur, "mmap\n");
      
				for (mmap = ((struct multiboot_tag_mmap *) tag)->entries;
				(multiboot_uint8_t *) mmap < (multiboot_uint8_t *) tag + tag->size;

				mmap = (multiboot_memory_map_t *) ((unsigned long) mmap +
				((struct multiboot_tag_mmap *) tag)->entry_size))
				mprintf(&cur, " base_addr = 0x%i%i,"
				" length = 0x%i%i, type = 0x%i\n",
				(unsigned) (mmap->addr >> 32),
				(unsigned) (mmap->addr & 0xffffffff),
				(unsigned) (mmap->len >> 32),
				(unsigned) (mmap->len & 0xffffffff),
				(unsigned) mmap->type);
				}
				break;
			case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
			{
				break;
      }
			default:
				break;
		}
    }
	tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag 
                                  + ((tag->size + 7) & ~7));
	mprintf(&cur, "Total mbi size 0x%i\n", (unsigned) tag - addr);

	while(1);

	return map;
}
	