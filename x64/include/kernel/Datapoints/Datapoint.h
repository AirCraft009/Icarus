//
// Created by Mxsxll on 12.11.2026.
//

#ifndef ICARUS_DATAPOINT_H
#define ICARUS_DATAPOINT_H
#include <stdint.h>



typedef int exit_code_t;

typedef exit_code_t (io_write) (void * writable, uint8_t *data, uint64_t len, uint64_t offset);
typedef exit_code_t (io_read) (void * readable, uint8_t *input, uint64_t len, uint64_t offset);
typedef exit_code_t (io_close) (void * closeable, uint8_t *close_data, uint64_t len);


typedef struct {
    io_write * write;
    io_read * read;
    io_close * close;
    // uint8_t * extended_function_count;
    // void *extended_functions[];
}data_functions;

typedef struct {
    uint64_t ref_count;
    uint64_t time_open;
} datapoint_bookkeeping;

typedef struct DATAPOINT_REGISTRY_ENTRY {
    uint64_t data_id;
    void * data_point;
    datapoint_bookkeeping bookkeeping;
    data_functions *io_functions;
} datapoint_reg_entry_t;

uint64_t datapoint_register(data_functions *funcs, void * data_pointer);
bool datapoint_remove(uint64_t data_id);
int Datapoint_init();
datapoint_reg_entry_t * datapoint_get(uint64_t data_id);
exit_code_t datapoint_close(uint64_t data_id, uint8_t * buffer, uint64_t buffer_size);
exit_code_t datapoint_read(uint64_t data_id, uint8_t * buffer, uint64_t buffer_size, uint64_t offset);
exit_code_t datapoint_write(uint64_t data_id, uint8_t * buffer, uint64_t buffer_size, uint64_t offset);
exit_code_t datapoint_close(uint64_t data_id, uint8_t * buffer, uint64_t buffer_size);

#endif //ICARUS_DATAPOINT_H
