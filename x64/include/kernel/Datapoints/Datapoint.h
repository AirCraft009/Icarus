//
// Created by Mxsxll on 12.11.2026.
//

#ifndef ICARUS_DATAPOINT_H
#define ICARUS_DATAPOINT_H
#include <stdint.h>

#define UNIFIED_MAPPING_LAYER_HEAP 0x0000

typedef uint64_t exit_code_t;

typedef exit_code_t (io_write) (uint8_t *data, uint64_t len, uint64_t offset);
typedef exit_code_t (io_read) (uint8_t *input, uint64_t len, uint64_t offset);
typedef exit_code_t (io_close) (uint8_t *close_data, uint64_t len);


typedef struct {
    io_write * write_function;
    io_read * read_function;
    io_close * close;
}data_functions;

typedef struct {
    uint64_t ref_count;
    uint64_t time_open;
} datapoint_bookkeeping;

typedef struct DATAPOINT_REGISTRY_ENTRY {
    uint64_t data_id;
    data_functions io_functions;
    datapoint_bookkeeping bookkeeping;
} datapoint_reg_entry_t;





#endif //ICARUS_DATAPOINT_H
