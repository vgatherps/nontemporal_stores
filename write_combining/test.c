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

#ifndef BYTES
#define BYTES 16
#endif

void force_nt_store(cache_line *a) {

    // special case for 8 bytes
#if BYTES == 8
    __asm volatile("pxor %%mm0, %%mm0\n\t"
                   "movntq %%mm0, (%0)\n\t"
                   :
                   : "r" (&a->vec_val)
                   : "memory");
#else
    __m128i zeros = {0, 0};
    __asm volatile("movntdq %0, (%1)\n\t"
#if BYTES > 16
                   "movntdq %0, 16(%1)\n\t"
#endif
#if BYTES > 32
                   "movntdq %0, 32(%1)\n\t"
#endif
#if BYTES > 48
                   "movntdq %0, 48(%1)"
#endif
                   :
                   : "x" (zeros), "r" (&a->vec_val)
                   : "memory");
#endif
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

__attribute__ ((noinline))
uint64_t run_timer_loop(void) {

    mfence();

    uint64_t start = rdtscp();

    for (int i = 0; i < 32; i++) {
        force_nt_store(&large_buffer[i]);
    }

    mfence();

    volatile uint64_t end = rdtscp();

    uint64_t diff = end > start ? end - start : 0;
    return diff;
}

// never will the compiler inline this!
volatile uint64_t (*loop_var)(void);

int main()
{
    uint64_t n_loop = 10000;
    uint64_t total = 0;
    loop_var = (void *)&run_timer_loop;
    for (int i = 0; i < n_loop; i++) {
        total += loop_var();
    }
    printf("%d\n", (int)(total / n_loop));
    return 0;
}
