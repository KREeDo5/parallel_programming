#include <iostream>
#include <omp.h>
#include <cmath>
#include <iomanip>
#include <Windows.h>

using namespace std;

const long long ITERATIONS = 1e8;

double calculate_pi_for()
{
    double pi = 0.0;
    for (long long i = 0; i < ITERATIONS; i++)
    {
        double term = pow(-1, i) / (2.0 * i + 1);
        pi += term;
    }
    return pi * 4;
}

double calculate_pi_parallel_for_incorrect()
{
    double pi = 0.0;
#pragma omp parallel for
    for (long long i = 0; i < ITERATIONS; i++)
    {
        double term = pow(-1, i) / (2.0 * i + 1);
        pi += term;
    }
    return pi * 4;
}

double calculate_pi_parallel_for_atomic() {
    double pi = 0.0;
#pragma omp parallel for
    for (long long i = 0; i < ITERATIONS; i++)
    {
        double term = pow(-1, i) / (2.0 * i + 1);
#pragma omp atomic
        pi += term;
    }
    return pi * 4;
}

double calculate_pi_parallel_for_reduction()
{
    double pi = 0.0;
#pragma omp parallel for reduction(+:pi)
    for (long long i = 0; i < ITERATIONS; i++)
    {
        double term = pow(-1, i) / (2.0 * i + 1);
        pi += term;
    }
    return pi * 4;
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    cout << setprecision(15);

    double start_time = omp_get_wtime();
    double pi_for = calculate_pi_for();
    double time_for = omp_get_wtime() - start_time;
    cout << "For (синхронно): " << pi_for << "\nВремя выполнения: " << time_for << " сек\n\n\n";

    start_time = omp_get_wtime();
    double pi_parallel_for_incorrect = calculate_pi_parallel_for_incorrect();
    double time_parallel_for_incorrect = omp_get_wtime() - start_time;
    cout << "For (неверно): " << pi_parallel_for_incorrect << "\nВремя выполнения: " << time_parallel_for_incorrect << " сек\n\n\n";

    start_time = omp_get_wtime();
    double pi_parallel_for_atomic = calculate_pi_parallel_for_atomic();
    double time_parallel_for_atomic = omp_get_wtime() - start_time;
    cout << "Parallel for и atomic: " << pi_parallel_for_atomic << "\nВремя выполнения: " << time_parallel_for_atomic << " сек\n\n\n";

    start_time = omp_get_wtime();
    double pi_parallel_for_reduction = calculate_pi_parallel_for_reduction();
    double time_parallel_for_reduction = omp_get_wtime() - start_time;
    cout << "Reduction: " << pi_parallel_for_reduction << "\nВремя выполнения: " << time_parallel_for_reduction << " сек\n";

    system("pause");

    return 0;
}
