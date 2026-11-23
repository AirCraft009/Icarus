//
// Created by Mxsxll on 12.11.2026.
//

#include "kernel/Datapoints/Datapoint.h"

#include "string.h"
#include "kernel/Memory/alloc.h"
#include "kernel/util/hash_table.h"

hash_table_t * map_table;
uint64_t DATA_ID = 1;

int Datapoint_init() {
    map_table = (hash_table_t *) Imalloc(sizeof(hash_table_t));
    if (!hash_table_init(map_table, 100, 70)) return -1;
    return 0;
}

uint64_t datapoint_register(data_functions *funcs, void * data_pointer) {
    datapoint_reg_entry_t *entry = (datapoint_reg_entry_t *) Imalloc(sizeof(datapoint_reg_entry_t));
    if (!entry) return 0;
    entry->data_id = DATA_ID++;
    //TODO: set time
    entry->io_functions = funcs;
    entry->data_point = data_pointer;
    if (!hash_table_put(map_table, entry->data_id, (uint64_t) entry))
        return 0;
    return entry->data_id;
}

bool datapoint_remove(uint64_t data_id) {
    return hash_table_remove(map_table, data_id);
}

datapoint_reg_entry_t * datapoint_get(uint64_t data_id) {
    datapoint_reg_entry_t * entry = NULL;
    if (!hash_table_get(map_table, data_id,(uint64_t *) entry))
        return NULL;
    return entry;
}

exit_code_t datapoint_read(uint64_t data_id, uint8_t * buffer, uint64_t buffer_size, uint64_t offset) {
    datapoint_reg_entry_t * entry = datapoint_get(data_id);
    return entry->io_functions->read
    (
        entry->data_point,
        buffer,
        buffer_size,
        offset
    );
}




