//          Copyright Jean Pierre Cimalando 2018.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "ring_buffer/ring_buffer.h"
#include <algorithm>
#include <cassert>

Ring_Buffer::Ring_Buffer(size_t capacity)
    : cap_(capacity + 1),
      rbdata_(new uint8_t[capacity + 1])
{
}

Ring_Buffer::~Ring_Buffer()
{
}

size_t Ring_Buffer::size_used() const
{
    const size_t rp = rp_, wp = wp_, cap = cap_;
    return wp + ((wp < rp) ? cap : 0) - rp;
}

bool Ring_Buffer::discard(size_t len)
{
    return getbytes_ex_(nullptr, len, true);
}

size_t Ring_Buffer::size_free() const
{
    const size_t rp = rp_, wp = wp_, cap = cap_;
    return rp + ((rp <= wp) ? cap : 0) - wp - 1;
}

bool Ring_Buffer::getbytes_(void *data, size_t len)
{
    return getbytes_ex_(data, len, true);
}

bool Ring_Buffer::getbytes_ex_(void *data, size_t len, bool advp)
{
    if (size_used() < len)
        return false;

    const size_t rp = rp_, cap = cap_;
    const uint8_t *src = rbdata_.get();
    uint8_t *dst = (uint8_t *)data;

    if (data) {
        const size_t taillen = std::min(len, cap - rp);
        std::atomic_thread_fence(std::memory_order_acquire);
        std::copy_n(&src[rp], taillen, dst);
        std::copy_n(src, len - taillen, dst + taillen);
    }

    if (advp)
        rp_ = (rp + len < cap) ? (rp + len) : (rp + len - cap);
    return true;
}

bool Ring_Buffer::peekbytes_(void *data, size_t len) const
{
    Ring_Buffer *ncthis = const_cast<Ring_Buffer *>(this);
    return ncthis->getbytes_ex_(data, len, false);
}

bool Ring_Buffer::putbytes_(const void *data, size_t len)
{
    if (size_free() < len)
        return false;

    const size_t wp = wp_, cap = cap_;
    const uint8_t *src = (const uint8_t *)data;
    uint8_t *dst = rbdata_.get();

    const size_t taillen = std::min(len, cap - wp);
    std::copy_n(src, taillen, &dst[wp]);
    std::copy_n(src + taillen, len - taillen, dst);
    std::atomic_thread_fence(std::memory_order_release);

    wp_ = (wp + len < cap) ? (wp + len) : (wp + len - cap);
    return true;
}

//------------------------------------------------------------------------------
size_t Soft_Ring_Buffer::size_used() const
{
    std::shared_lock<mutex_type> lock(shmutex_);
    return rb_.size_used();
}

bool Soft_Ring_Buffer::discard(size_t len)
{
    std::shared_lock<mutex_type> lock(shmutex_);
    return rb_.discard(len);
}

size_t Soft_Ring_Buffer::size_free() const
{
    std::shared_lock<mutex_type> lock(shmutex_);
    return rb_.size_free();
}

bool Soft_Ring_Buffer::getbytes_(void *data, size_t len)
{
    std::shared_lock<mutex_type> lock(shmutex_);
    return rb_.getbytes_(data, len);
}

bool Soft_Ring_Buffer::peekbytes_(void *data, size_t len) const
{
    std::shared_lock<mutex_type> lock(shmutex_);
    return rb_.peekbytes_(data, len);
}

bool Soft_Ring_Buffer::putbytes_(const void *data, size_t len)
{
    bool good;
    size_t oldcap = rb_.capacity();
    size_t atleastcap = size_used() + len;
    if (atleastcap <= oldcap) {
        std::shared_lock<mutex_type> lock(shmutex_);
        good = rb_.putbytes_(data, len);
    }
    else {
        std::unique_lock<mutex_type> lock(shmutex_);
        grow_(atleastcap);
        good = rb_.putbytes_(data, len);
    }
    assert(good);
    return true;
}

void Soft_Ring_Buffer::grow_(size_t atleastcap)
{
    size_t oldcap = rb_.capacity();
    size_t newcap = (oldcap < 16) ? 16 : oldcap;
    while (newcap < atleastcap) {
        if (newcap > std::numeric_limits<size_t>::max() / 3)
            throw std::bad_alloc();
        newcap = newcap * 3 / 2;
    }

    size_t len = rb_.size_used();
    std::unique_ptr<uint8_t[]> newdata(new uint8_t[newcap + 1]);

    {
        const size_t rp = rb_.rp_;
        const uint8_t *src = rb_.rbdata_.get();
        uint8_t *dst = newdata.get();

        const size_t taillen = std::min(len, oldcap + 1 - rp);
        std::copy_n(&src[rp], taillen, dst);
        std::copy_n(src, len - taillen, dst + taillen);
    }

    rb_.cap_ = newcap + 1;
    rb_.rp_ = 0;
    rb_.wp_ = len;
    rb_.rbdata_ = std::move(newdata);
}
