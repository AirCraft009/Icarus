#include "kernel/util/hash_table.h"
#include <stdlib.h>

#include "kernel/Memory/alloc.h"

#define HASH_MIN_CAPACITY   16
#define HASH_MAX_USAGE_CAP  90   // keeps at least one empty slot so probing always terminates

// murmur3 64-bit finalizer: spreads aligned / sequential keys evenly
static inline uint64_t hash_mix(uint64_t key)
{
    key ^= key >> 33;
    key *= 0xff51afd7ed558ccdULL;
    key ^= key >> 33;
    key *= 0xc4ceb9fe1a85ec53ULL;
    key ^= key >> 33;
    return key;
}

static uint64_t round_up_pow2(uint64_t value)
{
    if (value <= HASH_MIN_CAPACITY)
        return HASH_MIN_CAPACITY;

    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    return value + 1;
}

static inline uint64_t max_entries(const hash_table_t *hash_table)
{
    return (uint64_t)(hash_table->capacity * hash_table->max_usage) / 100;
}

// Index of the slot holding key, or of the empty slot where it would go.
static inline uint64_t find_slot(const hash_entry_t *entries, uint64_t capacity, uint64_t key)
{
    uint64_t mask = capacity - 1;
    uint64_t index = hash_mix(key) & mask;

    while (entries[index].key != HASH_EMPTY_KEY && entries[index].key != key)
        index = (index + 1) & mask;

    return index;
}

static bool grow(hash_table_t *hash_table)
{
    uint64_t new_capacity = hash_table->capacity * 2;
    hash_entry_t *new_entries = Icalloc(new_capacity, sizeof(hash_entry_t));
    if (!new_entries)
        return false;

    for (uint64_t index = 0; index < hash_table->capacity; index++) {
        hash_entry_t entry = hash_table->table[index];
        if (entry.key == HASH_EMPTY_KEY)
            continue;
        new_entries[find_slot(new_entries, new_capacity, entry.key)] = entry;
    }

    free(hash_table->table);
    hash_table->table = new_entries;
    hash_table->capacity = new_capacity;
    return true;
}

bool hash_table_init(hash_table_t *hash_table, uint64_t initial_capacity, uint8_t max_usage)
{
    if (max_usage <= 0)
        max_usage = RECOMMENDED_EFFICIENCY;
    if (max_usage > HASH_MAX_USAGE_CAP)
        max_usage = HASH_MAX_USAGE_CAP;

    hash_table->capacity = round_up_pow2(initial_capacity);
    hash_table->size = 0;
    hash_table->max_usage = max_usage;
    hash_table->table = Icalloc(hash_table->capacity, sizeof(hash_entry_t));
    return hash_table->table != NULL;
}

void hash_table_destroy(hash_table_t *hash_table)
{
    free(hash_table->table);
    hash_table->table = NULL;
    hash_table->size = 0;
    hash_table->capacity = 0;
}

bool hash_table_put(hash_table_t *hash_table, uint64_t key, uint64_t value)
{
    if (key == HASH_EMPTY_KEY)
        return false;

    uint64_t index = find_slot(hash_table->table, hash_table->capacity, key);

    if (hash_table->table[index].key == key) {
        hash_table->table[index].value = value;
        return true;
    }

    if (hash_table->size + 1 > max_entries(hash_table)) {
        if (!grow(hash_table))
            return false;
        index = find_slot(hash_table->table, hash_table->capacity, key);
    }

    hash_table->table[index].key = key;
    hash_table->table[index].value = value;
    hash_table->size++;
    return true;
}

bool hash_table_get(const hash_table_t *hash_table, uint64_t key, uint64_t *out_value)
{
    if (key == HASH_EMPTY_KEY)
        return false;

    uint64_t index = find_slot(hash_table->table, hash_table->capacity, key);
    if (hash_table->table[index].key != key)
        return false;

    *out_value = hash_table->table[index].value;
    return true;
}

// Backward-shift deletion: no tombstones, probe chains stay short.
bool hash_table_remove(hash_table_t *hash_table, uint64_t key)
{
    if (key == HASH_EMPTY_KEY)
        return false;

    hash_entry_t *entries = hash_table->table;
    uint64_t mask = hash_table->capacity - 1;
    uint64_t hole = find_slot(entries, hash_table->capacity, key);

    if (entries[hole].key != key)
        return false;

    uint64_t next = (hole + 1) & mask;
    while (entries[next].key != HASH_EMPTY_KEY) {
        uint64_t home = hash_mix(entries[next].key) & mask;

        // the entry may fill the hole if the hole lies between its home slot and its current slot
        if (((next - home) & mask) >= ((next - hole) & mask)) {
            entries[hole] = entries[next];
            hole = next;
        }
        next = (next + 1) & mask;
    }

    entries[hole].key = HASH_EMPTY_KEY;
    entries[hole].value = 0;
    hash_table->size--;
    return true;
}
