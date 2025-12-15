
#define ZRAND_IMPLEMENTATION
#define ZRAND_SHORT_NAMES
#include "zrand.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

#define TEST(name) printf("[TEST] %-35s", name);
#define PASS() printf(" \033[0;32mPASS\033[0m\n")

void test_basic_gen(void) 
{
    TEST("Core Generation (u32, f32, bool)");
    
    // Explicit init (optional, but good for testing).
    rand_init();

    // Sanity checks.
    uint32_t u = rand_u32();
    assert(u <= UINT32_MAX);
    
    float f = rand_f32();
    assert(f >= 0.0f && f < 1.0f);
    
    bool b = rand_bool();
    assert(b == true || b == false);

    PASS();
}

void test_range(void) 
{
    TEST("Range (Integer & Float)");

    // Test boundaries.
    for (int i = 0; i < 1000; i++) 
    {
        int32_t r = rand_range(10, 20);
        assert(r >= 10 && r <= 20);
        
        float rf = rand_range_f(5.0f, 10.0f);
        assert(rf >= 5.0f && rf < 10.0f);
    }
    
    // Single value range.
    assert(rand_range(5, 5) == 5);

    PASS();
}

void test_utilities(void) 
{
    TEST("UUID, String, Choice");

    // UUID.
    char uuid[37];
    rand_uuid(uuid);
    assert(strlen(uuid) == 36);
    assert(uuid[14] == '4'); // Version 4.
    // Variant bits: 8, 9, a, or b (binary 10xx).
    char v = uuid[19];
    assert(v == '8' || v == '9' || v == 'a' || v == 'b');

    // Random string.
    char str[11];
    rand_str(str, 10);
    assert(strlen(str) == 10);

    // Choice.
    int nums[] = {10, 20, 30, 40};
    int *picked = (int*)rand_choice(nums, 4, sizeof(int));
    assert(picked != NULL);
    assert(*picked == 10 || *picked == 20 || *picked == 30 || *picked == 40);

    PASS();
}

void test_determinism(void) 
{
    TEST("RNG Determinism (Seeding)");

    zrand_rng rng1, rng2;
    uint64_t seed = 12345ULL;
    uint64_t seq = 1ULL;

    zrand_rng_init(&rng1, seed, seq);
    zrand_rng_init(&rng2, seed, seq);

    // Sequence must be identical.
    for (int i = 0; i < 100; i++) 
    {
        assert(zrand_rng_u32(&rng1) == zrand_rng_u32(&rng2));
    }

    PASS();
}

int main(void) 
{
    printf("=> Running tests (zrand.h, main).\n");
    test_basic_gen();
    test_range();
    test_utilities();
    test_determinism();
    printf("=> All tests passed successfully.\n");
    return 0;
}

