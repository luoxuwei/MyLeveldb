//
// Created by 罗旭维 on 2021/11/17.
//

#include <cstdint>

char* EncodeVarint32(char* dst, uint32_t v) {
    uint8_t *ptr = reinterpret_cast<uint8_t*>(dst);
    static const int B = 128;
    if (v < (1<<7)) {
        *(ptr++) = v;
    } else if (v < (1<<14)) {
        *(ptr++) = v | B;
        *(ptr++) = v>>7;
    } else if (v < (1<<21)) {
        *(ptr++) = v | B;
        *(ptr++) = (v>>7) | B;
        *(ptr++) = v>>14;
    } else if (v < (1<<28)) {
        *(ptr++) = v | B;
        *(ptr++) = (v>>7) | B;
        *(ptr++) = (v>>14) | B;
        *(ptr++) = v>>21;
    } else {
        *(ptr++) = v | B;
        *(ptr++) = (v >> 7) | B;
        *(ptr++) = (v >> 14) | B;
        *(ptr++) = (v >> 21) | B;
        *(ptr++) = v >> 28;
    }
    return reinterpret_cast<char*>(ptr);
}


char* EncodeVarint64(char* dst, uint64_t v) {
    static const int B = 128;
    uint8_t* ptr = reinterpret_cast<uint8_t*>(dst);
    while (v >= B) {
        *(ptr++) = v | B;
        v >>= 7;
    }
    *(ptr++) = static_cast<uint8_t>(v);
    return reinterpret_cast<char*>(ptr);
}