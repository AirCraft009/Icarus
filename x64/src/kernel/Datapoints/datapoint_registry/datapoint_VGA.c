//
// Created by cocon on 28.11.2026.
//

#include <stddef.h>

#include "kernel/Datapoints/Datapoint.h"
#include "kernel/Datapoints/datapoint_registry.h"
#include "kernel/util/shellio.h"

exit_code_t vga_write (void * writeable, uint8_t * data, uint64_t length, uint64_t offset);
exit_code_t vga_read (void * readable, uint8_t * data, uint64_t length, uint64_t offset);
exit_code_t vga_close (void * closeable, uint8_t * data, uint64_t length);

data_functions vga_io_funcs =
{
    vga_write,
    vga_read,
    vga_close,
};

exit_code_t vga_write (void * writeable, uint8_t * data, uint64_t length, uint64_t offset) {
    write_str_len(data, offset, length);
    return 0;
}
exit_code_t vga_read (void * readable, uint8_t * data, uint64_t length, uint64_t offset) {
    return 0;
}
exit_code_t vga_close (void * closeable, uint8_t * data, uint64_t length) {
    return 0;
}

uint64_t create_vga_point() {
    uint64_t data_id = datapoint_register(&vga_io_funcs, NULL);
    return data_id;
}
