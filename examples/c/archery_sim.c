
#include <stdio.h>
#include <string.h>

#define ZMATH_IMPLEMENTATION
#include "zmath.h"

#define ZRAND_SHORT_NAMES
#define ZRAND_IMPLEMENTATION
#include "zrand.h"

#define SAMPLES 100000
#define HIST_WIDTH 40
#define HIST_HEIGHT 20

void draw_histogram(const char* title, int *buckets, int bucket_count, int max_val) 
{
    printf("\n** %s **\n", title);
    
    for (int y = HIST_HEIGHT; y > 0; y--) 
    {
        printf("%2d| ", y);
        for (int x = 0; x < bucket_count; x++) 
        {
            int bar_height = (int)((float)buckets[x] / max_val * HIST_HEIGHT);
            if (bar_height >= y) 
            {
                printf("#");
            }
            else 
            {
                printf(" ");
            }
        }
        printf("\n");
    }
    printf("   +");
    for (int i = 0; i<bucket_count; i++)
    {
        printf("-");
    }
    printf("\n    Low                                 High\n");
}

void test_uniform_distribution() 
{
    int buckets[HIST_WIDTH] = {0};
    int max_count = 0;

    for (int i = 0; i < SAMPLES; i++) 
    {
        // Generates number between -3.0 and 3.0 uniformly.
        float shot = rand_range_f(-3.0f, 3.0f);
        
        int idx = (int)((shot + 3.0f) / 6.0f * HIST_WIDTH);
        if (idx >= 0 && idx < HIST_WIDTH) 
        {
            buckets[idx]++;
            if (buckets[idx] > max_count) 
            {
                max_count = buckets[idx];
            }
        }
    }
    
    draw_histogram("Uniform (Standard Random)", buckets, HIST_WIDTH, max_count);
    printf("Result: Flat distribution. Unrealistic for skill-based events.\n");
}

void test_gaussian_distribution() 
{
    int buckets[HIST_WIDTH] = {0};
    int max_count = 0;

    for (int i = 0; i < SAMPLES; i++) 
    {
        double shot = rand_gaussian(0.0, 1.0);
        
        // Note: Gaussian can technically generate numbers outside -3..3 but really rarely.
        int idx = (int)((shot + 3.0) / 6.0 * HIST_WIDTH);
        if (idx >= 0 && idx < HIST_WIDTH) 
        {
            buckets[idx]++;
            if (buckets[idx] > max_count) max_count = buckets[idx];
        }
    }

    draw_histogram("Gaussian (Normal Distribution)", buckets, HIST_WIDTH, max_count);
    printf("Result: Bell Curve. Most shots hit near center. Feels natural.\n");
}

int main(void) 
{
    zrand_init();

    printf("Simulating %d shots...\n", SAMPLES);

    test_uniform_distribution();
    test_gaussian_distribution();

    return 0;
}
