//
// Created by cocon on 03.11.2026.
//

#include "src/kernel/Processes/process_management.h"


#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "kernel/Memory/alloc.h"
#include "kernel/util/mem_utils.h"

#define  INITIAL_CAPACITY 40

// credits:
// http://www.cse.yorku.ca/~oz/hash.html
unsigned long hashStr(unsigned char *str){
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}
typedef struct PROCESS_ITEM {
    char *key;
    void *value;
}proc_item;

typedef struct PROC_MAP {
    proc_item *list;
    int size;
    int capacity;
    int key_size;           // size of each key in bytes
} proc_map;

proc_map * CreateProcTable(int key_size) {
    proc_map *hash = (proc_map *)Imalloc(sizeof(proc_map));
    hash->capacity = INITIAL_CAPACITY;
    hash->size = 0;
    hash->key_size = key_size;
    hash->list = Icalloc(hash->capacity,sizeof(proc_item));

    return hash;
}


proc_map * Createproc_map(int capacity, int key_size) {
    proc_map *hash = (proc_map *)Imalloc(sizeof(proc_map));
    hash->capacity = capacity;
    hash->size = 0;
    hash->key_size = key_size;
    hash->list = Icalloc(capacity, sizeof(proc_item));

    return hash;
}

int sizeUpMap(proc_map *hash) {
    if (hash->size < hash->capacity/2) {
        return 0;
    }

    hash->capacity = hash->capacity * 2;
    void * list = Irealloc(hash->list, sizeof(proc_item) * hash->capacity);
    if (list == NULL) {
        return -1;
    }
    hash->list = list;
    return 0;
}

void Destroyproc_map(proc_map *hash) {
    for (int i = 0; i < hash->capacity; i++) {
        free(hash->list[i].key);
    }
    free(hash->list);
    free(hash);
}

/**
 * add an item to the proc_map
 *
 * @param hash proc_map
 * @param key only value
 * @param value ptr is kept
 */
bool proc_mapSet(proc_map *hash, uint8_t * key, void *value) {
    assert(hash);
    int idx = hashStr(key) % hash->capacity;

    for (int i = 0; i < hash->capacity; i++) {
        if (hash->list[idx].key == NULL || Istrncmp(hash->list[idx].key, key, hash->key_size) == 0) {
            hash->list[idx].key = strndup(key, hash->key_size);
            hash->list[idx].value = value;
            hash->size ++;
            if (sizeUpMap(hash) == -1) {
                fprintf(stderr,"Reallocation of internal proc_map array has failed. Contents may equal null\n");
            }
            return true;
        }
        idx = (idx + 1) % hash->capacity;
    }
    return false;
}


/**
 * remove an item from the proc_map
 * @return was the deletion successful
 */
bool proc_mapRemove(proc_map *hash, char * key) {
    assert(hash);
    int idx = hashStr(key) % hash->capacity;
    if (hash->list[idx].key == NULL) {
        return false;
    }
    for (int i = 0; i < hash->capacity; i++) {
        if (hash->list[idx].key != NULL && strcmp(hash->list[idx].key, key) == 0) {
            // don't free value as it's user owned memory
            free(hash->list[idx].key);
            hash->list[idx].key = NULL;
            hash->size--;
            return true;
        }
        idx = (idx + 1) % hash->capacity;
    }
    return false;
}

void * proc_mapGet(proc_map *hash, char * key) {
    int idx = hashStr(key) % hash->capacity;
    for (int i = 0; i < hash->capacity; i++) {
        if (hash->list[idx].key != NULL && strcmp(hash->list[idx].key, key) == 0) {
            return hash->list[idx].value;
        }
        idx = (idx + 1) % hash->capacity;
    }
    return NULL;
}

