//
// Created by 罗旭维 on 2021/11/17.
//

#ifndef MYLEVELDB_SLICE_H
#define MYLEVELDB_SLICE_H
#include <cassert>
#include <cstddef>
#include <string>
#include <cstring>

namespace leveldb {
    class Slice {
    public:
        Slice(): data_(""), size_(0) {}
        Slice(const char *d, size_t n) : data_(d), size_(n) {}
        Slice(const std::string &s) : data_(s.c_str()), size_(s.size()) {}
        Slice(const char *s) : data_(s), size_(strlen(s)) {}

        Slice(const Slice &) = default;
        Slice& operator=(const Slice&) = default;

        const char *data() const {return data_;}
        size_t size() const {return size_;}

        bool empty() const {return size_ == 0;}

        char operator[](size_t n) const {
            assert(n < size());
            return data_[n];
        }

        void clear() {
            data_ = "";
            size_ = 0;
        }

        void remove_prefix(size_t n) {
            assert(n <= size());
            data_ += n;
            size_ -= n;
        }

        std::string ToString() const { return std::string(data_, size_);}
        int compare(const Slice& b) const;

        bool start_with(const Slice& x) const {
            return ((size_ >= x.size_) && (memcmp(data_, x.data_, x.size_)));
        }

    private:
        const char* data_;
        size_t size_;
    };

    inline bool operator==(const Slice &x, const Slice &y) {
        return ((x.size() == y.size()) && (memcmp(x.data(), y.data(), x.size()) == 0));
    }

    inline bool operator!=(const Slice& x, const Slice& y) { return !(x == y); }

    inline int Slice::compare(const Slice& b) const {
        const size_t min_len = (size_ < b.size_) ? size_ : b.size_;
        int r = memcmp(data_, b.data_, min_len);
        if (r == 0) {
            if (size_ < b.size_)
                r = -1;
            else if (size_ > b.size_)
                r = +1;
        }
        return r;
    }
}

#endif //MYLEVELDB_SLICE_H
