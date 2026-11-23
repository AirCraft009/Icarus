//
// Created by Mxsxll on 23.11.2026.
//
// contains some default datafunctions for memory and similar

#include "string.h"
#include "kernel/Datapoints/Datapoint.h"
#include "kernel/Memory/alloc.h"

exit_code_t mem_write (void * writeable, uint8_t * data, uint64_t length, uint64_t offset);
exit_code_t mem_read (void * readable, uint8_t * data, uint64_t length, uint64_t offset);
exit_code_t mem_close (void * closeable, uint8_t * data, uint64_t length);

data_functions mem_io_func =
{
    mem_write,
    mem_read,
    mem_close,
};

exit_code_t mem_write (void * writeable, uint8_t * data, uint64_t length, uint64_t offset) {
    memcpy (writeable + offset, data, length);
    return 0;
}

exit_code_t mem_read (void * readable, uint8_t * data, uint64_t length, uint64_t offset) {
    memcpy (readable + offset, data, length);
    return 0;
}

exit_code_t mem_close (void * closeable, uint8_t * data, uint64_t length) {
    // don't free this is for general memory access
}

uint64_t create_mem_point(void * mem) {
    uint64_t data_id = datapoint_register(&mem_io_func, mem);
    return data_id;
}