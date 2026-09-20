//
// Created by cocon on 21.09.2026.
//

#include "string.h"

long strlen(char *str) {
    long len = 0;
    while (str) {
        len++;
        str++;
    }
    return len;
}
