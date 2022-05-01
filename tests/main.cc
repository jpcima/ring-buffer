//          Copyright Jean Pierre Cimalando 2018-2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// SPDX-License-Identifier: BSL-1.0

#include <ring_buffer/ring_buffer.h>
#include <string>
#include <memory>
#include <thread>
#include <chrono>
#include <cstdio>
#include <cstring>
namespace kro = std::chrono;

namespace {
size_t initial_capacity = 1024;
size_t message_count = 100;
size_t message_size = 100;
size_t num_tries = 1000;
double message_timeout = 5.0;
}

struct Basic_Test {
    Basic_Test(std::string name) : name_{std::move(name)} {}
    virtual ~Basic_Test() = default;
    virtual void perform() = 0;
    const std::string name_;
};

template <class RB>
struct Test : public Basic_Test {
    Test(std::string name) : Basic_Test{std::move(name)} {}
    void perform() override;
};

template <class RB>
void Test<RB>::perform()
{
    std::printf(">>> Test start: %s <<<\n", name_.c_str());

    RB ring_buffer{initial_capacity};

    if (::message_size < sizeof(size_t)) {
        std::printf("message too small (need at least %lu bytes)\n", (unsigned long)sizeof(size_t));
        std::terminate();
    }

    auto thread1_run = [&ring_buffer]() {
        const size_t message_size = ::message_size;
        const size_t message_count = ::message_count;
        std::unique_ptr<uint8_t[]> message_buffer(new uint8_t[message_size]);

        std::printf("begin send\n");

        kro::steady_clock::time_point start = kro::steady_clock::now();
        kro::steady_clock::time_point time_of_warning;
        bool is_first_warning = true;

        for (size_t i = 0; i < message_count;) {
            std::memcpy(message_buffer.get(), &i, sizeof(size_t));
            size_t oldcap = ring_buffer.capacity();
            if (!ring_buffer.put(message_buffer.get(), message_size)) {
                if (RB::can_extend()) {
                    std::printf("buffer did not extend\n"); // must never happen
                    std::terminate();
                }
                else {
                    kro::steady_clock::time_point now = kro::steady_clock::now();
                    double elapsed = kro::duration<double>(now - start).count();
                    if (is_first_warning || kro::duration<double>(now - time_of_warning).count() > 1) {
                        std::printf("message not sent yet (%zu)...\n", i);
                        time_of_warning = now;
                        is_first_warning = false;
                    }
                    if (elapsed > ::message_timeout) {
                        std::printf("message sending timeout\n");
                        std::terminate();
                    }
                }
            }
            else {
                size_t newcap = ring_buffer.capacity();
                if (oldcap != newcap)
                    std::printf("growth (%zu -> %zu)\n", oldcap, newcap);
                ++i;
                start = kro::steady_clock::now();
                is_first_warning = true;
            }
        }
        std::printf("sent (%zu)\n", message_count);
    };

    auto thread2_run = [&ring_buffer]() {
        const size_t message_size = ::message_size;
        const size_t message_count = ::message_count;
        std::unique_ptr<uint8_t[]> message_buffer(new uint8_t[message_size]);

        std::printf("begin receive\n");

        kro::steady_clock::time_point start = kro::steady_clock::now();
        kro::steady_clock::time_point time_of_warning;
        bool is_first_warning = true;

        for (size_t i = 0; i < message_count;) {
            if (!ring_buffer.get(message_buffer.get(), message_size)) {
                kro::steady_clock::time_point now = kro::steady_clock::now();
                double elapsed = kro::duration<double>(now - start).count();
                if (is_first_warning || kro::duration<double>(now - time_of_warning).count() > 1) {
                    std::printf("message not arrived yet (%zu)...\n", i);
                    time_of_warning = now;
                    is_first_warning = false;
                }
                if (elapsed > ::message_timeout) {
                    std::printf("message receiving timeout\n");
                    std::terminate();
                }
            }
            else {
                size_t msg;
                std::memcpy(&msg, message_buffer.get(), sizeof(size_t));
                if (msg != i) {
                    std::printf("message (%zu) != expected (%zu)\n", msg, i);
                    std::terminate();
                }
                ++i;
                start = kro::steady_clock::now();
                is_first_warning = true;
            }
        }

        std::printf("received (%zu)\n", message_count);
    };

    for (size_t i = 0; i < num_tries; ++i)
    {
        std::printf("---------------- %4zu/%4zu ----------------\n", i + 1, num_tries);
        std::thread t1(thread1_run);
        std::thread t2(thread2_run);
        t1.join();
        t2.join();
    }
    std::printf("-------------------------------------------\n");
    std::printf("success!\n");

    std::printf("<<< Test end: %s >>>\n", name_.c_str());
}

int main()
{
    std::unique_ptr<Basic_Test> test;

    test.reset(new Test<Ring_Buffer>{"Hard"});
    test->perform();

    std::putchar('\n');

    test.reset(new Test<Soft_Ring_Buffer>{"Soft"});
    test->perform();

    return 0;
}
