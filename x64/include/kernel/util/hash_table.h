//
// Created by cocon on 03.11.2026.
//

#ifndef ICARUS_HASHMAP_H
#define ICARUS_HASHMAP_H
#include <stdint.h>
// hashmap implementation by me

#include <stdbool.h>
#include <stdint.h>

#define RECOMMENDED_EFFICIENCY 70

// Key value that marks an empty slot. It can never be stored as a real key.
#define HASH_EMPTY_KEY 0

typedef struct HASH_ENTRY {
    uint64_t key;
    uint64_t value;
} hash_entry_t;

typedef struct HASH_TABLE {
    uint64_t size;
    uint64_t capacity;        // always a power of two
    hash_entry_t *table;
    uint8_t max_usage;          // percent of capacity full; rec 70
} hash_table_t;

// Creates a table with room for at least initial_capacity slots.
// max_usage is clamped to (0, 90]. Returns false on allocation failure.
bool hash_table_init(hash_table_t *hash_table, uint64_t initial_capacity, uint8_t max_usage);

void hash_table_destroy(hash_table_t *hash_table);

// Inserts or overwrites. Returns false if key == HASH_EMPTY_KEY or on allocation failure.
bool hash_table_put(hash_table_t *hash_table, uint64_t key, uint64_t value);

// Returns true and writes *out_value if the key exists.
bool hash_table_get(const hash_table_t *hash_table, uint64_t key, uint64_t *out_value);

// Returns true if the key existed and was removed.
bool hash_table_remove(hash_table_t *hash_table, uint64_t key);

#endif //ICARUS_HASHMAP_H
