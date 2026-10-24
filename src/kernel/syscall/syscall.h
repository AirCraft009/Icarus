//
// Created by cocon on 23.10.2026.
//

#ifndef ICARUS_SYSCALL_H
#define ICARUS_SYSCALL_H
#pragma once
#include <stdint.h>

int syscall_init();
uint64_t syscall_handler(uint64_t number, uint64_t first, uint64_t second,uint64_t third, uint64_t fourth, uint64_t fifth);

#endif //ICARUS_SYSCALL_H
