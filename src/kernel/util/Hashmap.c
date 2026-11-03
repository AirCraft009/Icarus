//
// Created by cocon on 03.11.2026.
//

#include "Hashmap.h"

#include "hashmap.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// credits:
// http://www.cse.yorku.ca/~oz/hash.html
unsigned long hashStr(unsigned char *str){
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

typedef struct HashItem {
    char *key;
    void *value;
}hashItem;

struct Hashmap {
    hashItem *list;
    int size;
    int capacity;
};

hashmap * CreateBaseHashmap() {
    hashmap *hash = (hashmap *)malloc(sizeof(hashmap));
    hash->capacity = INITIAL_CAPACITY;
    hash->size = 0;
    hash->list = calloc(hash->capacity, sizeof(hashItem));

    return hash;
}


hashmap * CreateHashmap(int capacity) {
    hashmap *hash = (hashmap *)malloc(sizeof(hashmap));
    hash->capacity = capacity;
    hash->size = 0;
    hash->list = calloc(capacity, sizeof(hashItem));

    return hash;
}

int sizeUpMap(hashmap *hash) {
    if (hash->size < hash->capacity/2) {
        return 0;
    }

    hash->capacity = hash->capacity * 2;
    void * list = realloc(hash->list, sizeof(hashItem) * hash->capacity);
    if (list == nullptr) {
        return -1;
    }
    hash->list = list;
    return 0;
}

void DestroyHashmap(hashmap *hash) {
    for (int i = 0; i < hash->capacity; i++) {
        free(hash->list[i].key);
    }
    free(hash->list);
    free(hash);
}

/**
 * add an item to the hashmap
 *
 * @param hash hashmap
 * @param key only value
 * @param value ptr is kept
 */
bool hashmapSet(hashmap *hash, char * key, void *value) {
    assert(hash);
    int idx = hashStr(key) % hash->capacity;

    for (int i = 0; i < hash->capacity; i++) {
        if (hash->list[idx].key == NULL || strcmp(hash->list[idx].key, key) == 0) {
            hash->list[idx].key = strdup((char*) key);
            hash->list[idx].value = value;
            hash->size ++;
            if (sizeUpMap(hash) == -1) {
                fprintf(stderr,"Reallocation of internal Hashmap array has failed. Contents may equal null\n");
            }
            return true;
        }
        idx = (idx + 1) % hash->capacity;
    }
    return false;
}


/**
 * remove an item from the hashmap
 * @return was the deletion successful
 */
bool hashmapRemove(hashmap *hash, char * key) {
    assert(hash);
    int idx = hashStr(key) % hash->capacity;
    if (hash->list[idx].key == NULL) {
        return false;
    }
    for (int i = 0; i < hash->capacity; i++) {
        if (hash->list[idx].key != NULL && strcmp(hash->list[idx].key, key) == 0) {
            // don't free value as it's user owned memory
            free(hash->list[idx].key);
            hash->list[idx].key = nullptr;
            hash->size--;
            return true;
        }
        idx = (idx + 1) % hash->capacity;
    }
    return false;
}

void * hashmapGet(hashmap *hash, char * key) {
    int idx = hashStr(key) % hash->capacity;
    for (int i = 0; i < hash->capacity; i++) {
        if (hash->list[idx].key != NULL && strcmp(hash->list[idx].key, key) == 0) {
            return hash->list[idx].value;
        }
        idx = (idx + 1) % hash->capacity;
    }
    return nullptr;
}
