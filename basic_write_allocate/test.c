#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <emmintrin.h>

// I use defines here so there's never a possibility of speculation
// accidentally seeing the wrong instruction and having some
// side effect

typedef struct {
    __m128i vec_val;
    int int_val;
    char data[64 - (sizeof(__m128i) + sizeof(int))];
} cache_line;

cache_line barrier1[32];

__attribute__ ((aligned(64)))
cache_line dummy_lines[64];

cache_line barrier2[32];

__attribute__ ((aligned(64)))
cache_line large_buffer[1000000/32];

__attribute__ ((aligned(64)))
cache_line large_nontemporal_buffer[1000000/32];

volatile int dump_to_me;

void shuffle() {
    for (int i = 0; i < 64; i++) {
        dummy_lines[i].int_val = i;
    }
    for (int i = 0; i < 64; i++) {
        int left = 64 - i;
        int swap = rand() % left;
        cache_line temp = dummy_lines[i];
        dummy_lines[i] = dummy_lines[swap];
        dummy_lines[swap] = temp;
    }
}

void iterate() {
    int next = 0;
    for (int i = 0; i < 32; i++) {
        __asm volatile("" : "+r" (next) :: "memory");
        next = dummy_lines[next].int_val;
    }
}

void force_load(void *a) {
    __asm volatile("mov (%0), %%edi"
                   :
                   : "r" (a)
                   : "edi", "memory");
}

void force_store(cache_line *a) {
    *(volatile int *)a = 0;
}

void force_nt_store(cache_line *a) {
    __m128i zeros = {0, 0}; // chosen to use zeroing idiom;
    // do 4 stores to hit whole cache line
    __asm volatile("movntdq %0, (%1)\n\t"
                   "movntdq %0, 16(%1)\n\t"
                   "movntdq %0, 32(%1)\n\t"
                   "movntdq %0, 48(%1)"
                   :
                   : "x" (zeros), "r" (&a->vec_val)
                   : "memory");
}

void mfence() {
   __asm volatile("mfence" ::: "memory");
}

uint64_t rdtscp() {
    // thanks https://stackoverflow.com/questions/9887839/how-to-count-clock-cycles-with-rdtsc-in-gcc-x86
    // since I'm lazy
    unsigned hi, lo;
    __asm volatile ("rdtscp" : "=a"(lo), "=d"(hi) :: "memory", "rcx");
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

#ifndef STORES
#define STORES 0
#endif

#ifndef NT_STORES
#define NT_STORES 0
#endif

__attribute__ ((noinline))
uint64_t run_timer_loop(void) {

    shuffle();

    for (int i = 0; i < STORES; i++) {
        force_store(&large_buffer[i]);
    }

    for (int i = 0; i < NT_STORES; i++) {
        force_nt_store(&large_buffer[i]);
    }

    uint64_t start = rdtscp();
    iterate();
    volatile uint64_t end = rdtscp();

    mfence();
    uint64_t diff = end > start ? end - start : 0;
    return diff;
}

// never will the compiler inline this!
volatile uint64_t (*loop_var)(void);

int main()
{
    srand(rdtscp());
    uint64_t n_loop = 10000;
    uint64_t total = 0;
    loop_var = (void *)&run_timer_loop;
    for (int i = 0; i < n_loop; i++) {
        total += loop_var();
    }
    printf("%d\n", (int)(total / n_loop));
    return 0;
}
