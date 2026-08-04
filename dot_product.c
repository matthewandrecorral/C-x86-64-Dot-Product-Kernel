#include <stddef.h>

/*
    Calculates the dot product of vector A and vector B.

    sdot = A[0] * B[0]
         + A[1] * B[1]
         + ...
         + A[n - 1] * B[n - 1]
*/
void dot_product_c(
    const double* vector_a,
    const double* vector_b,
    size_t n,
    double* sdot
)
{
    double sum = 0.0;

    for (size_t i = 0; i < n; i++)
    {
        sum += vector_a[i] * vector_b[i];
    }

    *sdot = sum;
}