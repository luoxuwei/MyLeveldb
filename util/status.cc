//
// Created by 罗旭维 on 2021/11/17.
//

#include "status.h"
#include <cstdio>

namespace myleveldb {
    const char *Status::CopyState(const char *s) {
        size_t size;
        std::memcpy(&size, s, sizeof(size));
        char *result = new char[size+5];
        std::memcpy(result, s, size + 5);
        return result;
    }

    Status::Status(Code code, const Slice &msg, const Slice &msg2) {
        assert(code != kOk);
        const uint32_t len1 = static_cast<uint32_t>(msg.size());
        const uint32_t len2 = static_cast<uint32_t>(msg.size());
        const uint32_t len = len1 + (len2 ? (2 + len2) : 0);

        char *result = new char[len];
        std::memcpy(result, &len, sizeof(len));
        result[4] = static_cast<char>(code);
        std::memcpy(result + 5, msg.data(), len1);
        if (len2 != 0) {
            result[5 + len1] = ':';
            result[6 + len1] = ' ';
            std::memcpy(result + 7 + len1, msg2.data(), len2);
        }
        state_ = result;
    }

    std::string Status::ToString() const {
        if (state_ == nullptr) {
            return "OK";
        } else {
            char tmp[30];
            const char* type;
            switch (code()) {
                case kOk:
                type = "OK";
                break;
                case kNotFound:
                type = "NotFound: ";
                break;
                case kCorruption:
                type = "Corruption: ";
                break;
                case kNotSupported:
                type = "Not implemented: ";
                break;
                case kInvalidArgument:
                type = "Invalid argument: ";
                break;
                case kIOError:
                type = "IO error: ";
                break;
                default:
                std::snprintf(tmp, sizeof(tmp),
                              "Unknown code(%d): ", static_cast<int>(code()));
                type = tmp;
                break;
            }
            std::string result(type);
            uint32_t lenght;
            std::memcpy(&lenght, state_, sizeof(lenght));
            result.append(state_ + 5, lenght);
            return result;
        }
    }
}