physical fram allocation broken.

the physical allocator hands out the same addr twice and this leads to two threads being in the same frame\

things to check
frame_alloc,
elf-loader,
bitmap
