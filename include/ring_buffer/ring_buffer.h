//          Copyright Jean Pierre Cimalando 2018.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <memory>
#include <atomic>
#include <cstdint>
#include <cstddef>

template <class RB>
class Basic_Ring_Buffer {
public:
    // read operations
    template <class T> bool get(T &x);
    template <class T> bool get(T *x, size_t n);
    template <class T> bool peek(T &x);
    template <class T> bool peek(T *x, size_t n);
    // write operations
    template <class T> bool put(const T &x);
    template <class T> bool put(const T *x, size_t n);
};

//------------------------------------------------------------------------------
class Ring_Buffer final :
    private Basic_Ring_Buffer<Ring_Buffer> {
public:
    // initialization and cleanup
    explicit Ring_Buffer(size_t capacity);
    ~Ring_Buffer();
    // attributes
    size_t capacity() const;
    // read operations
    size_t size_used() const;
    bool discard(size_t len);
    using Basic_Ring_Buffer::get;
    using Basic_Ring_Buffer::peek;
    // write operations
    size_t size_free() const;
    using Basic_Ring_Buffer::put;

private:
    size_t cap_{0};
    std::atomic<size_t> rp_{0}, wp_{0};
    std::unique_ptr<uint8_t[]> rbdata_ {};
    friend class Basic_Ring_Buffer;
    friend class Soft_Ring_Buffer;
    bool getbytes_(void *data, size_t len);
    bool peekbytes_(void *data, size_t len) const;
    bool putbytes_(const void *data, size_t len);
    bool getbytes_ex_(void *data, size_t len, bool advp);
    bool putbytes_ex_(const void *data, size_t len);
};

//------------------------------------------------------------------------------
#include <shared_mutex>
#if defined(RING_BUFFER_NO_STD_SHARED_MUTEX)
#    include <boost/thread/shared_mutex.hpp>
#endif

class Soft_Ring_Buffer final :
    private Basic_Ring_Buffer<Soft_Ring_Buffer> {
public:
    // initialization and cleanup
    explicit Soft_Ring_Buffer(size_t capacity);
    ~Soft_Ring_Buffer();
    // attributes
    size_t capacity() const;
    // read operations
    size_t size_used() const;
    bool discard(size_t len);
    using Basic_Ring_Buffer::get;
    using Basic_Ring_Buffer::peek;
    // write operations
    size_t size_free() const;
    using Basic_Ring_Buffer::put;

private:
#if !defined(RING_BUFFER_NO_STD_SHARED_MUTEX)
    typedef std::shared_mutex mutex_type;
#else
    typedef boost::shared_mutex mutex_type;
#endif

private:
    Ring_Buffer rb_;
    mutable mutex_type shmutex_;
    friend class Basic_Ring_Buffer;
    bool getbytes_(void *data, size_t len);
    bool peekbytes_(void *data, size_t len) const;
    bool putbytes_(const void *data, size_t len);
    void grow_(size_t newcap);
};

//------------------------------------------------------------------------------
#include "ring_buffer/ring_buffer.tcc"
