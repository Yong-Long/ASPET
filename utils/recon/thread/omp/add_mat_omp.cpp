#include <iostream>

#if defined(_OPENMP)
#include <omp.h>
#endif

#define NRW (1 << 18)

int main(void)
{
    long A[NRW];
    long B[NRW];
    long C[NRW];

#pragma omp parallel for
    for (auto irw = 0; irw < NRW; irw++)
    {
        C[irw] = 0;
        A[irw] = irw;
        B[irw] = NRW;  //(NRW - irw);
    }

#pragma omp parallel for
    for (auto irw = 0; irw < NRW; irw++)
    {
        C[irw] = A[irw] + B[irw];
    }

    for (auto irw = 0; irw < 10; irw++)
    {
        std::cout << C[irw] << ", ";
    }
    std::cout << " ...\n"
              << (NRW) << "\n";

    return 0;
}