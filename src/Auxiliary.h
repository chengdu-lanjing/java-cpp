/*
 * 本框架版权归"成都蓝景信息技术有限公司所有", 更多细节请参见LICENSE文件
 *
 * 本框架提供以Java思维来开发C++应用程序的能力, 并对本公司相关项目需要用到的JDK和开源框架的API给出类似实现
 *
 * @author 陈涛
 */
#pragma once

#include "Common.h"

namespace com_lanjing_cpp_common {
    struct System {
    public:
        static int64_t currentTimeMillis() {
            struct timeval tv;
            gettimeofday(&tv,NULL);
            return (int64_t)tv.tv_sec * 1000 + (int64_t)tv.tv_usec / 1000;
        }
        template <typename T>
        static void arraycopy(const T *src, T *dst, int length, bool cppElement) {
            if (cppElement) {
                for (int i = 0; i < length; i++) {
                    dst[i] = src[i];
                }
            } else {
                memcpy(dst, src, length * sizeof(T));
            }
        }
        template <typename T>
        static void arraycopy(Arr<T> src, int srcPos, Arr<T> dst, int dstPos, int length) {
            if (length < 0) {
                throw_new(IllegalArgumentException, "length less than zero")
            }
            if (srcPos < 0 || srcPos + length > src.length()) {
                throw_new(IllegalArgumentException, "srcPos out of range");
            }
            if (dstPos < 0 || dstPos + length > dst.length()) {
                throw_new(IllegalArgumentException, "srcPos out of range");
            }
            if (length > 0) {
                arraycopy(
                        src.unsafe() + srcPos,
                        dst.unsafe() + dstPos,
                        length,
                        dst.elementType() == ArrayElementType::CPP
                );
            }
        }
    private:
        System();
    };

    struct Math {
    public:
        template <typename T>
        static T min(const T &a, const T &b) {
            return a < b ? a : b;
        }
        template <typename T>
        static T max(const T &a, const T &b) {
            return a > b ? a : b;
        }
    private:
        Math();
    };

    template <typename T>
    struct Nullable {
        Nullable() : present(true) {}
        Nullable(decltype(nullptr)) : present(true) {}
        Nullable(const T &value) : present(false), value(value) {}
        Nullable(const Nullable<T> &right) : present(right.present), value(right.value) {}
        Nullable<T> &operator = (decltype(nullptr)) {
            this->present = true;
            return *this;
        }
        Nullable<T> &operator = (const T &value) {
            this->present = false;
            this->value = value;
            return *this;
        }
        Nullable<T> &operator = (const Nullable<T> value) {
            if (this != &value) {
                this->present = value.present;
                this->value = value;
            }
            return *this;
        }
        operator const T&() const {
            if (this->present) {
                throw_new(IllegalStateException, "The current object is present");
            }
            return this->value;
        }
        T &operator*() {
            return this->value;
        }
        T *operator->() {
            if (this->present) {
                throw_new(IllegalStateException, "The current object is present");
            }
            return &this->value;
        }
    private:
        bool present;
        T value;

        template <typename X> friend bool operator == (const Nullable<X> &, const Nullable<X> &);
        template <typename X> friend bool operator != (const Nullable<X> &, const Nullable<X> &);
        template <typename X> friend bool operator == (const Nullable<X> &, decltype(nullptr));
        template <typename X> friend bool operator != (const Nullable<X> &, decltype(nullptr));
        template <typename X> friend bool operator == (const Nullable<X> &, const X &);
        template <typename X> friend bool operator != (const Nullable<X> &, const X &);
        template <typename X> friend bool operator == (decltype(nullptr), const Nullable<X> &);
        template <typename X> friend bool operator != (decltype(nullptr), const Nullable<X> &);
        template <typename X> friend bool operator == (const X &, const Nullable<X> &);
        template <typename X> friend bool operator != (const X &, const Nullable<X> &);
    };

    template <typename T> bool operator == (const Nullable<T> &lhs, const Nullable<T> &rhs) {
        if (lhs.present != rhs.present) {
            return false;
        }
        if (lhs.present) {
            return true;
        }
        return lhs.value == rhs.value;
    }
    template <typename T> bool operator != (const Nullable<T> &lhs, const Nullable<T> &rhs) {
        return !operator == (lhs, rhs);
    }
    template <typename T> bool operator == (const Nullable<T> &lhs, decltype(nullptr)) {
        return lhs.present;
    }
    template <typename T> bool operator != (const Nullable<T> &lhs, decltype(nullptr)) {
        return !lhs.present;
    }
    template <typename T> bool operator == (const Nullable<T> &lhs, const T &rhs) {
        return !lhs.present && lhs.value == rhs;
    }
    template <typename T> bool operator != (const Nullable<T> &lhs, const T &rhs) {
        return lhs.present || lhs.value != rhs;
    }
    template <typename T> bool operator == (decltype(nullptr), const Nullable<T> &rhs) {
        return rhs.present;
    }
    template <typename T> bool operator != (decltype(nullptr), const Nullable<T> &rhs) {
        return !rhs.present;
    }
    template <typename T> bool operator == (const T &lhs, const Nullable<T> &rhs) {
        return !rhs.present && lhs == rhs.value;
    }
    template <typename T> bool operator != (const T &lhs, const Nullable<T> &rhs) {
        return rhs.present || lhs != rhs.value;
    }
}
