#include <stdint.h>
#include <stdlib.h>

#include <emmintrin.h>

#include <iostream>
#include <map>

void cpy_nt_line(void *f, void *t) {
    __m128i dummy;
    __asm volatile("movdqa  (%2), %0\n\t"
                   "movntdq %0, (%1)\n\t"
                   "movdqa  16(%2), %0\n\t"
                   "movntdq %0, 16(%1)\n\t"
                   "movdqa  32(%2), %0\n\t"
                   "movntdq %0, 32(%1)\n\t"
                   "movdqa  64(%2), %0\n\t"
                   "movntdq %0, 48(%1)"
                   : "=x" (dummy)
                   : "r" (t), "r"(f)
                   : "memory");
}

void cpy_line(void *f, void *t) {
    __m128i dummy;
    __asm volatile("movdqa  (%2), %0\n\t"
                   "movdqa %0, (%1)\n\t"
                   "movdqa  16(%2), %0\n\t"
                   "movdqa %0, 16(%1)\n\t"
                   "movdqa  32(%2), %0\n\t"
                   "movdqa %0, 32(%1)\n\t"
                   "movdqa  64(%2), %0\n\t"
                   "movdqa %0, 48(%1)"
                   : "=x" (dummy)
                   : "r" (t), "r"(f)
                   : "memory");
}

void mfence() {
   __asm volatile("mfence" ::: "memory");
}

struct message {
    uint64_t id;
    char data[1024*8 - sizeof(uint64_t)];
};

void copy_msg(const message &from, message &to) {
    char *f = (char *)&from;
    char *t = (char *)&to;
    for (size_t i = 0; i < sizeof(message); i += 16) {
#ifdef NT_CPY
        cpy_nt_line(f+i, t+i);
#else
        cpy_line(f+i, t+i);
#endif
   }
}

std::map<uint64_t, uint64_t> lookup_map;
std::vector<message> message_buffer;
size_t message_buffer_ind;

uint64_t process_message(const message &m) {
    
    message &cpy_to = message_buffer[message_buffer_ind];
    message_buffer_ind++;
    if (message_buffer_ind == message_buffer.size()) {
        message_buffer_ind = 0;
    }

    copy_msg(m, cpy_to);
   
    return lookup_map[m.id];
}

#ifdef N_IDS
#define N_IDS 10000
#endif

int main()
{
    for (size_t i = 0; i < N_IDS; i++) {
        lookup_map[i] = i;
    }
    srand(time(NULL));
    message dummy_msg;
    volatile uint64_t value = 0;
    for (size_t i = 0; i < 1000000; i++) {
        m.id = rand() % N_IDS;
        value += process_message(m);
    }
    return 0;
}
