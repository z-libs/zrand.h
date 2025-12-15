
#define ZRAND_IMPLEMENTATION
#include "zrand.h"

#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>

#define TEST(name) printf("[TEST] %-35s", name);
#define PASS() std::cout << "\033[0;32mPASS\033[0m\n";

void test_cpp_wrappers() 
{
    TEST("Namespace Shortcuts");
    
    z_rand::init();
    
    auto u = z_rand::u32();
    assert(u >= 0);
    
    auto f = z_rand::f64();
    assert(f >= 0.0 && f < 1.0);
    
    bool b = z_rand::chance(0.5);
    (void)b;

    std::string s = z_rand::string(8);
    assert(s.length() == 8);

    std::string uuid = z_rand::uuid();
    assert(uuid.length() == 36);

    PASS();
}

void test_stl_integration() 
{
    TEST("STL Shuffle & Choice");

    std::vector<int> v = {1, 2, 3, 4, 5};
    
    // Copy for verification.
    std::vector<int> original = v;

    z_rand::shuffle(v);

    assert(v.size() == 5);
    // Ensure all elements still exist (just permuted).
    std::sort(v.begin(), v.end());
    assert(v == original);

    // Choice.
    int val = z_rand::choice(v);
    assert(val >= 1 && val <= 5);

    PASS();
}

void test_generator_class() 
{
    TEST("Generator Class (Seedable)");

    z_rand::generator gen1(42);
    z_rand::generator gen2(42);

    // Deterministic check.
    assert(gen1.u32() == gen2.u32());
    assert(gen1.f64() == gen2.f64());
    
    // Range.
    int r = gen1.range(100, 200);
    assert(r >= 100 && r <= 200);

    PASS();
}

int main() 
{
    std::cout << "=> Running tests (zrand.h, cpp).\n";
    test_cpp_wrappers();
    test_stl_integration();
    test_generator_class();
    std::cout << "=> All tests passed successfully.\n";
    return 0;
}

