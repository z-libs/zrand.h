
#include <stdio.h>
#define ZRAND_IMPLEMENTATION
#include "zrand.h"

int main(void) 
{
    printf("=> Z-Rand Dice Roller\n");
    // No explicit init needed, auto-seeds from OS entropy on first use.
    
    // -> Basic range (1-6).
    printf("Rolling d6 10 times: ");
    for(int i=0; i<10; i++) 
    {
        printf("%d ", zrand_range(1, 6));
    }
    printf("\n");

    // -> Bias check (coin flip).
    int heads = 0, tails = 0;
    int iterations = 1000000;
    for(int i=0; i<iterations; i++) 
    {
        if (zrand_bool()) heads++; else tails++;
    }
    printf("Coin Flip (%d runs): Heads=%.2f%%, Tails=%.2f%%\n", 
           iterations, 
           (heads / (double)iterations) * 100.0, 
           (tails / (double)iterations) * 100.0);

    // -> Loot drop (float chance).
    printf("\nLoot Drop (5%% chance):\n");
    for(int i=1; i<=50; i++) 
    {
        if (zrand_chance(0.05)) 
        {
            printf("  Attempt %d: LEGENDARY ITEM DROP!\n", i);
        }
    }
    
    return 0;
}
