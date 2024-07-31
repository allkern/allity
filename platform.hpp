#pragma once

#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define be(v) (v)

template <typename T> struct be_t {
    T data;

    be_t(T initial) {
        data = initial;
    }

    operator T() {
        return data;
    }
}
#else
template <class T> inline T be(T v) {
    if constexpr (sizeof(T) == 2) {
        return __builtin_bswap16(v);
    } else if (sizeof(T) == 4) {
        return __builtin_bswap32(v);
    } else {
        return __builtin_bswap64(v);
    }
}

template <typename T> struct be_t {
    T data;

    be_t() = default;
    be_t(T initial) {
        data = initial;
    }

    operator T() const {
        if constexpr (sizeof(T) == 1) {
            return data;
        } else if (sizeof(T) == 2) {
            return __builtin_bswap16(data);
        } else if (sizeof(T) == 4) {
            return __builtin_bswap32(data);
        }

        return __builtin_bswap64(data);
    }
};
#endif