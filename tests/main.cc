//          Copyright Jean Pierre Cimalando 2018.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <ring_buffer/ring_buffer.h>
#include <memory>
#include <thread>
#include <cstdio>

std::unique_ptr<Soft_Ring_Buffer> ring_buffer;
size_t initial_capacity = 4;
size_t message_count = 100;
size_t message_size = 100;

void thread1()
{
    Soft_Ring_Buffer &ring_buffer = *::ring_buffer;
    const size_t message_size = ::message_size;
    const size_t message_count = ::message_count;
    std::unique_ptr<uint8_t[]> message_buffer(new uint8_t[message_size]);

    std::printf("begin send\n");
    for (size_t i = 0; i < message_count; ++i) {
        *(size_t *)message_buffer.get() = i;
        size_t oldcap = ring_buffer.capacity();
        ring_buffer.put(message_buffer.get(), message_size);
        size_t newcap = ring_buffer.capacity();
        if (oldcap != newcap)
            std::printf("growth (%zu -> %zu)\n", oldcap, newcap);
    }
    std::printf("sent (%zu)\n", message_count);
}

void thread2()
{
    Soft_Ring_Buffer &ring_buffer = *::ring_buffer;
    const size_t message_size = ::message_size;
    const size_t message_count = ::message_count;
    std::unique_ptr<uint8_t[]> message_buffer(new uint8_t[message_size]);

    std::printf("begin receive\n");
    for (size_t i = 0; i < message_count;) {
        if (!ring_buffer.get(message_buffer.get(), message_size)) {
            std::printf("message not arrived yet (%zu)...\n", i);
        }
        else {
            size_t msg = *(size_t *)message_buffer.get();
            if (msg != i) {
                std::printf("message (%zu) != expected (%zu)\n", msg, i);
                std::terminate();
            }
            ++i;
        }
    }

    std::printf("received (%zu)\n", message_count);
}

int main()
{
    size_t ntimes = 1000;
    for (size_t i = 0; i < ntimes; ++i)
    {
        std::printf("---------------- %4zu/%4zu ----------------\n", i + 1, ntimes);
        ring_buffer.reset(new Soft_Ring_Buffer(initial_capacity));
        std::thread t1(thread1);
        std::thread t2(thread2);
        t1.join();
        t2.join();
    }
    std::printf("-------------------------------------------\n");
    std::printf("success!\n");

    return 0;
}
