//
// Created by Mxsxll on 22.09.2026.
//

#ifndef ICARUS_BITMAP_H
#define ICARUS_BITMAP_H
#pragma once
#include <stdint.h>

typedef unsigned char byte;
typedef struct Bitmap {
    uint64_t size;
    byte	*data;
}bitmap;

#endif //ICARUS_BITMAP_H
