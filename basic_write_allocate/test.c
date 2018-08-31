#include <stdint.h>
#include <stdio.h>

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
cache_line dummy_line[32]; // many to circle through conflicts

cache_line barrier2[32];

__attribute__ ((aligned(64)))
cache_line large_buffer[1000000/32];

__attribute__ ((aligned(64)))
cache_line large_nontemporal_buffer[1000000/32];


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
uint64_t run_timer_loop(int ind) {

    ind %= 32;

    force_load(&dummy_line[ind]);

    for (int i = 0; i < STORES; i++) {
        force_store(&large_buffer[i]);
    }

    for (int i = 0; i < NT_STORES; i++) {
        force_nt_store(&large_buffer[i]);
    }

    uint64_t start = rdtscp();
    force_load(&dummy_line[ind]);
    volatile uint64_t end = rdtscp();

    mfence();
    uint64_t diff = end > start ? end - start : 0;
    return diff;
}

// never will the compiler inline this!
volatile uint64_t (*loop_var)(int);

int main()
{
    uint64_t n_loop = 10000;
    uint64_t total = 0;
    loop_var = (void *)&run_timer_loop;
    for (int i = 0; i < n_loop; i++) {
        total += loop_var(i);
    }
    printf("%d\n", (int)(total / n_loop));
    return 0;
}
