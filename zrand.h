
#ifndef ZRAND_H
#define ZRAND_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* State. */

// You can have multiple independent RNG instances.
typedef struct {
    uint64_t state;
    uint64_t inc;
} zrand_rng;

/* Global / thread-local API. */

// -> So it uses a thread-local instance automatically seeded from OS entropy.

// Re-seeds the thread-local generator from OS entropy.
void zrand_init(void); 

// The core g1eneration.
uint32_t zrand_u32(void);
uint64_t zrand_u64(void);
float    zrand_f32(void); // 0.0f .. 1.0f (exclusive 1.0).
double   zrand_f64(void); // 0.0 .. 1.0 (exclusive 1.0).
bool     zrand_bool(void);

// The utilities.

// Returns integer in [min, max].
int32_t zrand_range(int32_t min, int32_t max);
// Returns float in [min, max).
float   zrand_range_f(float min, float max);
// Returns true "chance" percent of the time (0.0 to 1.0).
bool    zrand_chance(double probability);
// Fills buffer with random bytes.
void    zrand_bytes(void *buf, size_t len);
// Fills buffer with random alphanumeric string (A-Z, a-z, 0-9).
void    zrand_str(char *buf, size_t len);
// Generates a UUID v4 string (37 chars including null: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx).
// Check more here: https://en.wikipedia.org/wiki/Universally_unique_identifier
void    zrand_uuid(char *buf);
// Shuffles an array (Fisher-Yates).
void    zrand_shuffle(void *base, size_t nmemb, size_t size);


/* Instance API. */
// For deterministic behavior (replay systems, proc-gen).

void     zrand_rng_init(zrand_rng *rng, uint64_t seed, uint64_t seq);
uint32_t zrand_rng_u32(zrand_rng *rng);
uint64_t zrand_rng_u64(zrand_rng *rng);
int32_t  zrand_rng_range(zrand_rng *rng, int32_t min, int32_t max);
double   zrand_rng_f64(zrand_rng *rng);

#ifdef __cplusplus
}
#endif

/* ========================================================================= */
/* C++ WRAPPER - [New Addition]                                              */
/* ========================================================================= */
#ifdef __cplusplus
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

namespace z_rand 
{
    /* Global shortcuts. */
    inline void init() { ::zrand_init(); }
    inline uint32_t u32() { return ::zrand_u32(); }
    inline uint64_t u64() { return ::zrand_u64(); }
    inline float f32() { return ::zrand_f32(); }
    inline double f64() { return ::zrand_f64(); }
    inline bool boolean() { return ::zrand_bool(); }
    
    // "Coin toss" or "Loot drop".
    inline bool chance(double prob) { return ::zrand_chance(prob); }
    
    // Inclusive [min, max].
    inline int32_t range(int32_t min, int32_t max) { return ::zrand_range(min, max); }
    inline float range(float min, float max) { return ::zrand_range_f(min, max); }

    inline std::string uuid() 
    {
        char buf[37];
        ::zrand_uuid(buf);
        return std::string(buf);
    }

    inline std::string string(size_t len) 
    {
        std::string s; 
        s.resize(len);
        // zrand_str writes to the internal buffer.
        ::zrand_str(&s[0], len); 
        return s;
    }

    template<typename T>
    inline void shuffle(std::vector<T> &v) 
    {
        if (!v.empty()) ::zrand_shuffle(v.data(), v.size(), sizeof(T));
    }

    /* Generator Instance (for reproducible seeds). */
    class generator 
    {
        zrand_rng rng;
    public:
        // Auto-seed from global entropy.
        generator() 
        { 
            ::zrand_rng_init(&rng, ::zrand_u64(), ::zrand_u64()); 
        }

        // Explicit seed.
        generator(uint64_t seed, uint64_t seq = 1) 
        { 
            ::zrand_rng_init(&rng, seed, seq); 
        }

        uint32_t u32() { return ::zrand_rng_u32(&rng); }
        uint64_t u64() { return ::zrand_rng_u64(&rng); }
        double f64() { return ::zrand_rng_f64(&rng); }
        int32_t range(int32_t min, int32_t max) { return ::zrand_rng_range(&rng, min, max); }
        bool chance(double p) { return f64() < p; }
    };
}
#endif // __cplusplus

// The implementation part, remember including the macro.

#ifdef ZRAND_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <time.h>

// OS Entropy Source, we need it for seeding.
#ifdef _WIN32
    #define _CRT_RAND_S
    #include <stdlib.h>
    static uint64_t zrand__os_seed(void) 
    {
        unsigned int s1, s2;
        rand_s(&s1); rand_s(&s2);
        return ((uint64_t)s1 << 32) | s2;
    }
#else
    #include <stdio.h>
    static uint64_t zrand__os_seed(void) 
    {
        uint64_t seed = 0;
        FILE *f = fopen("/dev/urandom", "rb");
        if (f) 
        {
            size_t n = fread(&seed, sizeof(seed), 1, f);
            fclose(f);
            if (n == 1) return seed;
        }
        // A fallback just in case.
        return (uint64_t)time(NULL) ^ (uintptr_t)&seed;
    }
#endif

/* PCG implementation. */

static uint32_t zrand__pcg32(zrand_rng *rng) 
{
    uint64_t oldstate = rng->state;
    // Advance internal state.
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc | 1);
    // Calculate output function (XSH-RR).
    uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
    uint32_t rot = (uint32_t)(oldstate >> 59u);
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

void zrand_rng_init(zrand_rng *rng, uint64_t seed, uint64_t seq) 
{
    rng->state = 0U;
    rng->inc = (seq << 1u) | 1u;
    zrand__pcg32(rng);
    rng->state += seed;
    zrand__pcg32(rng);
}

/* Global state (thread local). */

// Use C11 thread_local if available, otherwise just global.
// Maybe we could add zthread.h over here.
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
    #include <threads.h>
    #define ZRAND_TLS _Thread_local
#elif defined(_MSC_VER)
    #define ZRAND_TLS __declspec(thread)
#elif defined(__GNUC__)
    #define ZRAND_TLS __thread
#else
    #define ZRAND_TLS
#endif

static ZRAND_TLS zrand_rng zrand_global = {0x853C49E6748FEA9BULL, 0xDA3E39CB94B95BDBULL};
static ZRAND_TLS bool zrand_seeded = false;

static inline zrand_rng* zrand__get(void) 
{
    if (!zrand_seeded) 
    {
        zrand_init();
    }
    return &zrand_global;
}

/* The global API. */

void zrand_init(void) 
{
    uint64_t seed = zrand__os_seed();
    uint64_t seq = (uint64_t)(uintptr_t)&zrand_global; // Use address for sequence diversity
    zrand_rng_init(&zrand_global, seed, seq);
    zrand_seeded = true;
}

uint32_t zrand_u32(void) 
{ 
    return zrand__pcg32(zrand__get()); 
}

uint64_t zrand_u64(void) 
{
    zrand_rng *r = zrand__get();
    return ((uint64_t)zrand__pcg32(r) << 32) | zrand__pcg32(r);
}

float zrand_f32(void) 
{
    return (zrand_u32() >> 8) * (1.0f / 16777216.0f); // 24-bit resolution.
}

double zrand_f64(void) 
{
    return (zrand_u64() >> 11) * (1.0 / 9007199254740992.0); // 53-bit resolution.
}

bool zrand_bool(void) 
{
    return (zrand_u32() & 1);
}

bool zrand_chance(double probability) 
{
    return zrand_f64() < probability;
}

/* Bias-free range. */
int32_t zrand_range(int32_t min, int32_t max) 
{
    if (min >= max) return min;
    uint32_t range = (uint32_t)(max - min) + 1;
    uint32_t x, limit = (uint32_t)-1;
    uint32_t bucket_size = limit / range;
    uint32_t rejection_limit = bucket_size * range;
    
    do 
    {
        x = zrand_u32();
    } while (x >= rejection_limit); // Retry if in the waste zone.
    
    return min + (int32_t)(x / bucket_size);
}

float zrand_range_f(float min, float max) 
{
    return min + zrand_f32() * (max - min);
}

/* Some utilities. */

void zrand_bytes(void *buf, size_t len) 
{
    uint8_t *p = (uint8_t*)buf;
    // Fill in 4-byte chunks for speed.
    while (len >= 4) 
    {
        *(uint32_t*)p = zrand_u32();
        p += 4; len -= 4;
    }
    // Fill remainder.
    if (len > 0) 
    {
        uint32_t rem = zrand_u32();
        uint8_t *r = (uint8_t*)&rem;
        while (len--) *p++ = *r++;
    }
}

static const char ZRAND_ALPHANUM[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

void zrand_str(char *buf, size_t len) 
{
    for (size_t i = 0; i < len; i++) 
    {
        // Simple modulo is fine here, strict uniformity less critical for generic ID strings.
        // But I guess we could modify it if so.
        buf[i] = ZRAND_ALPHANUM[zrand_u32() % (sizeof(ZRAND_ALPHANUM) - 1)];
    }
    buf[len] = '\0';
}

void zrand_uuid(char *buf) 
{
    // 8-4-4-4-12.
    static const char *hex = "0123456789abcdef";
    uint8_t b[16];
    zrand_bytes(b, 16);
    
    // Version 4 adjustments.
    b[6] = (b[6] & 0x0F) | 0x40; // Version 4.
    b[8] = (b[8] & 0x3F) | 0x80; // Variant 1.
    
    // Format: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
    int k = 0;
    for (int i = 0; i < 16; i++) 
    {
        if (i == 4 || i == 6 || i == 8 || i == 10) 
        {
            buf[k++] = '-';
        }
        buf[k++] = hex[(b[i] >> 4) & 0xF];
        buf[k++] = hex[b[i] & 0xF];
    }
    buf[k] = '\0';
}

void zrand_shuffle(void *base, size_t nmemb, size_t size) 
{
    if (nmemb <= 1) return;
    char *arr = (char*)base;
    // Fisher-Yates shuffle.
    // We use a small buffer on stack for swapping.
    char temp[256]; 
    char *swap_buf = (size > sizeof(temp)) ? (char*)malloc(size) : temp;
    if (!swap_buf) return;

    for (size_t i = nmemb - 1; i > 0; i--) 
    {
        size_t j = zrand_range(0, (int32_t)i);
        if (i != j) 
        {
            // We swap arr[i] and arr[j].
            memcpy(swap_buf, arr + i * size, size);
            memcpy(arr + i * size, arr + j * size, size);
            memcpy(arr + j * size, swap_buf, size);
        }
    }
    
    if (swap_buf != temp) free(swap_buf);
}

/* Instance Wrappers (Forwarding) */
uint32_t zrand_rng_u32(zrand_rng *rng) { return zrand__pcg32(rng); }
uint64_t zrand_rng_u64(zrand_rng *rng) { return ((uint64_t)zrand__pcg32(rng) << 32) | zrand__pcg32(rng); }
double zrand_rng_f64(zrand_rng *rng) { return (zrand_rng_u64(rng) >> 11) * (1.0 / 9007199254740992.0); }
int32_t zrand_rng_range(zrand_rng *rng, int32_t min, int32_t max) 
{
    if (min >= max) return min;
    uint32_t range = (uint32_t)(max - min) + 1;
    uint32_t x, bucket = ((uint32_t)-1) / range;
    do 
    { 
        x = zrand_rng_u32(rng); 
    } while (x >= bucket * range);
    return min + (int32_t)(x / bucket);
}

#endif // ZRAND_IMPLEMENTATION
#endif // ZRAND_H