#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <Windows.h>

#define NUMBER_OF_RUNS 20
#define NUMBER_OF_SIZES 3

void dot_product_c(
    const double* vector_a,
    const double* vector_b,
    size_t n,
    double* sdot
);

void dot_product_asm(
    const double* vector_a,
    const double* vector_b,
    size_t n,
    double* sdot
);


static LARGE_INTEGER get_timer_value(void)
{
    LARGE_INTEGER timer_value;

    QueryPerformanceCounter(&timer_value);

    return timer_value;
}


static double get_elapsed_seconds(
    LARGE_INTEGER start,
    LARGE_INTEGER end,
    LARGE_INTEGER frequency
)
{
    return (double)(end.QuadPart - start.QuadPart)
        / (double)frequency.QuadPart;
}


static void initialize_vectors(
    double* vector_a,
    double* vector_b,
    size_t n
)
{
    for (size_t i = 0; i < n; i++)
    {
        vector_a[i] = (double)((i % 10) + 1);
        vector_b[i] = (double)((i % 5) + 1);
    }
}


static int results_are_equal(
    double c_result,
    double asm_result
)
{
    const double absolute_difference =
        fabs(c_result - asm_result);

    const double largest_value =
        fmax(fabs(c_result), fabs(asm_result));

    const double tolerance =
        1.0e-9 * fmax(1.0, largest_value);

    return absolute_difference <= tolerance;
}


int main(void)
{
    const unsigned int exponents[NUMBER_OF_SIZES] =
    {
        20,
        24,
        28
    };

    LARGE_INTEGER timer_frequency;

    if (!QueryPerformanceFrequency(&timer_frequency))
    {
        fprintf(stderr, "Error: High-resolution timer is unavailable.\n");
        return EXIT_FAILURE;
    }

#ifdef _DEBUG
    printf("Build configuration: Debug\n");
#else
    printf("Build configuration: Release\n");
#endif

    printf("Number of timing runs: %d\n\n", NUMBER_OF_RUNS);

    for (int size_index = 0;
         size_index < NUMBER_OF_SIZES;
         size_index++)
    {
        const unsigned int exponent = exponents[size_index];

        const size_t n = ((size_t)1) << exponent;

        if (n > SIZE_MAX / sizeof(double))
        {
            fprintf(
                stderr,
                "Vector size 2^%u is too large for size_t.\n",
                exponent
            );

            continue;
        }

        const size_t bytes_per_vector =
            n * sizeof(double);

        double* vector_a =
            (double*)malloc(bytes_per_vector);

        double* vector_b =
            (double*)malloc(bytes_per_vector);

        if (vector_a == NULL || vector_b == NULL)
        {
            fprintf(
                stderr,
                "Unable to allocate memory for n = 2^%u.\n",
                exponent
            );

            fprintf(
                stderr,
                "Required memory: approximately %.2f GiB "
                "for both vectors.\n\n",
                (2.0 * (double)bytes_per_vector)
                / (1024.0 * 1024.0 * 1024.0)
            );

            free(vector_a);
            free(vector_b);

            continue;
        }

        printf("==================================================\n");
        printf("Vector size: 2^%u = %zu elements\n", exponent, n);

        printf(
            "Memory used by both vectors: %.2f GiB\n",
            (2.0 * (double)bytes_per_vector)
            / (1024.0 * 1024.0 * 1024.0)
        );

        printf("Initializing vectors...\n");
        initialize_vectors(vector_a, vector_b, n);

        double c_result = 0.0;
        double asm_result = 0.0;

        dot_product_c(
            vector_a,
            vector_b,
            n,
            &c_result
        );

        dot_product_asm(
            vector_a,
            vector_b,
            n,
            &asm_result
        );

        double total_c_time = 0.0;
        double total_asm_time = 0.0;

        for (int run = 0; run < NUMBER_OF_RUNS; run++)
        {
            LARGE_INTEGER start_time;
            LARGE_INTEGER end_time;

            c_result = 0.0;

            start_time = get_timer_value();

            dot_product_c(
                vector_a,
                vector_b,
                n,
                &c_result
            );

            end_time = get_timer_value();

            total_c_time += get_elapsed_seconds(
                start_time,
                end_time,
                timer_frequency
            );
        }

        for (int run = 0; run < NUMBER_OF_RUNS; run++)
        {
            LARGE_INTEGER start_time;
            LARGE_INTEGER end_time;

            asm_result = 0.0;

            start_time = get_timer_value();

            dot_product_asm(
                vector_a,
                vector_b,
                n,
                &asm_result
            );

            end_time = get_timer_value();

            total_asm_time += get_elapsed_seconds(
                start_time,
                end_time,
                timer_frequency
            );
        }

        const double average_c_time =
            total_c_time / NUMBER_OF_RUNS;

        const double average_asm_time =
            total_asm_time / NUMBER_OF_RUNS;

        printf("\nC result:       %.10f\n", c_result);
        printf("x86-64 result:  %.10f\n", asm_result);

        if (results_are_equal(c_result, asm_result))
        {
            printf("Correctness check: PASSED\n");
            printf("The x86-64 kernel output is correct.\n");
        }
        else
        {
            printf("Correctness check: FAILED\n");

            printf(
                "Absolute difference: %.15f\n",
                fabs(c_result - asm_result)
            );
        }

        printf(
            "\nAverage C kernel time:      %.9f seconds\n",
            average_c_time
        );

        printf(
            "Average x86-64 kernel time: %.9f seconds\n",
            average_asm_time
        );

        if (average_asm_time > 0.0)
        {
            const double speedup =
                average_c_time / average_asm_time;

            printf(
                "Assembly speedup over C:    %.3fx\n",
                speedup
            );
        }

        printf("\n");

        free(vector_a);
        free(vector_b);
    }

    printf("Program finished.\n");

    return EXIT_SUCCESS;
}
