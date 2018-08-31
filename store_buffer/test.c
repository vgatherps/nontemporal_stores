#include <stdint.h>
#include <stdio.h>

#include <emmintrin.h>

// I use defines here so there's never a possibility of speculation
// accidentally seeing the wrong instruction and having some
// side effect

#ifndef NT_LINES
#define NT_LINES 1
#endif

#ifndef LINES
#define LINES 1
#endif

typedef struct {
    __m128i vec_val;
    int int_val;
    char data[64 - (sizeof(__m128i) + sizeof(int))];
} cache_line;

__attribute__ ((aligned(64)))
cache_line large_buffer[1024];

__attribute__ ((aligned(64)))
cache_line large_nontemporal_buffer[1024];

__attribute__ ((aligned(64)))
cache_line large_prestore_buffer[1024];

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

void clflush(void *a) {
    __asm volatile("clflush (%0)"
                   :
                   : "r" (a)
                   : "memory");
}

void sfence() {
   __asm volatile("sfence" ::: "memory");
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

#ifndef STORES_BEFORE
#define STORES_BEFORE 0
#endif

#ifndef NT_STORES_BEFORE
#define NT_STORES_BEFORE 0
#endif

#ifndef NT_LINES
#define NT_LINES 0
#endif

#ifndef LINES
#define LINES 0
#endif

__attribute__ ((noinline))
uint64_t run_timer_loop(void) {

    for (int i = 0; i < STORES_BEFORE; i++) {
        force_store(&large_prestore_buffer[i]);
    }

    for (int i = 0; i < NT_STORES_BEFORE; i++) {
        force_nt_store(&large_prestore_buffer[i]);
    }

    for (int i = 0; i < LINES; i++) {
#ifndef LINES_IN_CACHE
        clflush(&large_buffer[i]);
#else
        force_load(&large_buffer[i]);
#endif
    }

    for (int i = 0; i < LINES; i++) {
#ifndef NT_LINES_IN_CACHE
        clflush(&large_nontemporal_buffer[i]);
#else
        force_load(&large_nontemporal_buffer[i]);
#endif
    }

    // mfence is a serializing instruction, stronger than say mfence. Defined in the full code
    mfence();

    uint64_t start = rdtscp();

    for (int i = 0; i < NT_LINES; i++) {
        force_nt_store(&large_nontemporal_buffer[i]);
    }

#ifdef FENCE
    sfence();
#endif

    for (int i = 0; i < LINES; i++) {
        force_store(&large_buffer[i]);
    }

#ifdef FENCE_END
    mfence();
#endif

    // while rdtscp 'waits' for all preceding instructions to complete,
    // it does not wait for stores to complete as in be visible to
    // all other cores or the L3, but for the instruction to be retired
    // this makes it easy to benchmark the visible effects of nontemporal stores
    // on our stalls
    volatile uint64_t end = rdtscp();

    mfence();
    uint64_t diff = end > start ? end - start : 0;
    return diff;
}

// never will the compiler inline this!
volatile uint64_t (*loop_var)(void);

int main()
{
    uint64_t n_loop = 1000;
    uint64_t total = 0;
    loop_var = (void *)&run_timer_loop;
    for (int i = 0; i < n_loop; i++) {
        total += loop_var();
    }
    printf("%d\n", (int)(total / n_loop));
    return 0;
}
