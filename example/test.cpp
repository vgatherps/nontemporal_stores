#include <stdint.h>
#include <stdlib.h>

#include <emmintrin.h>

#include <chrono>
#include <iostream>
#include <map>
#include <vector>

void cpy_nt_line(void *f, void *t) {
    __m128i dummy = {0, 0};
    __asm volatile(
                   "movdqa (%2), %0\n\t"
                   "movntdq %0, (%1)\n\t"
                   "movdqa 16(%2), %0\n\t"
                   "movntdq %0, 16(%1)\n\t"
                   "movdqa 32(%2), %0\n\t"
                   "movntdq %0, 32(%1)\n\t"
                   "movdqa 48(%2), %0\n\t"
                   "movntdq %0, 48(%1)"
                   : "=x" (dummy)
                   : "r" (t), "r" (f)
                   : "memory");
}

void cpy_line(void *f, void *t) {
    __m128i dummy = {0, 0};
    __asm volatile(
                   "movdqa (%2), %0\n\t"
                   "movdqa %0, (%1)\n\t"
                   "movdqa 16(%2), %0\n\t"
                   "movdqa %0, 16(%1)\n\t"
                   "movdqa 32(%2), %0\n\t"
                   "movdqa %0, 32(%1)\n\t"
                   "movdqa 48(%2), %0\n\t"
                   "movdqa %0, 48(%1)"
                   : "=x" (dummy)
                   : "r" (t), "r" (f)
                   : "memory");
}

struct message {
    uint64_t id;
    char data[1024*8 - sizeof(uint64_t)];
};

void cpy_message(const message &from, message &to) {
    char *f = (char *)&from;
    char *t = (char *)&to;
    for (size_t i = 0; i < sizeof(message); i += 64) {
#ifdef NT_COPY
        cpy_nt_line(f+i, t+i);
#else
        cpy_line(f+i, t+i);
#endif
    }
}

void mfence() {
   __asm volatile("mfence" ::: "memory");
}

std::map<uint64_t, uint64_t> lookup_map;
std::vector<message> message_buffer;
size_t message_buffer_ind = 0;

uint64_t process_message(const message &m) {
    
    message &cpy_to = message_buffer[message_buffer_ind];
    message_buffer_ind++;
    if (message_buffer_ind == message_buffer.size()) {
        message_buffer_ind = 0;
    }

    cpy_message(m, cpy_to);
   
    return lookup_map[m.id];
}

#ifndef N_IDS
#define N_IDS 100000
#endif

#ifndef BUF
#define BUF 1
#endif

int main()
{
    srand(time(NULL));
    for (size_t i = 0; i < N_IDS; i++) {
        lookup_map[i] = i;
    }
    for (size_t i = 0; i < (1024*1024*BUF)/sizeof(message); i++) {
        message_buffer.push_back({});
    }
    volatile uint64_t total = 0;
    message m;
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < 100000; i++) {
        m.id = rand() % N_IDS;
        total += process_message(m);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto spent = std::chrono::duration_cast<std::chrono::milliseconds>(end -start).count();
    std::cout << spent << std::endl;
    __asm volatile ("" : "+m" (total) :: "memory");
    return 0;
}
