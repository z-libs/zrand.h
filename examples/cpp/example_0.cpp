
// We could use our containers, but as we are introducing
// concepts, I believe it's just better to work with the 
// standard.
#include <iostream>
#include <vector>
#include <string>

#define ZRAND_IMPLEMENTATION
#include "zrand.h"

int main() 
{
    // -> No need to manually seed, it happens lazily on first use.
    std::cout << "d6 Roll: " << z_rand::range(1, 6) << "\n";
    std::cout << "Chance:  " << (z_rand::chance(0.5) ? "Hit" : "Miss") << "\n";

    // -> In C++, this returns a std::string automatically.
    std::string id = z_rand::uuid();
    std::cout << "UUID:    " << id << "\n";

    // -> The C++ wrapper detects std::vector and handles the size/pointer logic.
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    z_rand::shuffle(numbers);

    std::cout << "Shuffle: ";
    for (int n : numbers) std::cout << n << " ";
    std::cout << "\n";

    // -> Useful for game replays or procedural generation.
    z_rand::generator gen(12345); // Fixed seed.
    std::cout << "Replay:  " << gen.range(1, 100) << "\n";

    return 0;
}
