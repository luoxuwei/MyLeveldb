//
// Created by 罗旭维 on 2021/11/17.
//

#ifndef MYLEVELDB_CODING_H
#define MYLEVELDB_CODING_H
#include <cstdint>

char* EncodeVarint32(char* dst, uint32_t value);
char* EncodeVarint64(char* dst, uint64_t value);

#endif //MYLEVELDB_CODING_H
