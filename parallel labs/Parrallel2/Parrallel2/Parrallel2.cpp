#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <omp.h>
#include <iomanip>

// Константы для размерности вектора и матрицы
const int MAX_SIZE = 60000;

// Функция для генерации случайного числа
double random_double() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0.0, 1.0);
    return dis(gen);
}

// Функция для создания случайного вектора
std::vector<double> create_random_vector(int size) {
    std::vector<double> vec(size);
    for (int i = 0; i < size; ++i) {
        vec[i] = random_double();
    }
    return vec;
}

// Функция для создания случайной квадратной матрицы
std::vector<std::vector<double>> create_random_matrix(int size) {
    std::vector<std::vector<double>> matrix(size, std::vector<double>(size));
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            matrix[i][j] = random_double();
        }
    }
    return matrix;
}

// Умножение вектора на матрицу в один поток
std::vector<double> multiply_vector_matrix_single_thread(const std::vector<double>& vec,
    const std::vector<std::vector<double>>& matrix) {
    int size = vec.size();
    std::vector<double> result(size, 0.0);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            result[i] += vec[j] * matrix[j][i];
        }
    }
    return result;
}

// Умножение вектора на матрицу с использованием OpenMP
std::vector<double> multiply_vector_matrix_openmp(const std::vector<double>& vec,
    const std::vector<std::vector<double>>& matrix,
    int num_threads) {
    int size = vec.size();
    std::vector<double> result(size, 0.0);

#pragma omp parallel for num_threads(num_threads)
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            result[i] += vec[j] * matrix[j][i];
        }
    }

    return result;
}

// Вывод результатов в таблицу
void print_results_table(int size, int num_threads, double single_thread_time, double openmp_time) {
    std::cout << std::setw(10) << size
        << std::setw(15) << num_threads
        << std::setw(20) << single_thread_time << " ms"
        << std::setw(20) << openmp_time << " ms"
        << std::setw(20) << (single_thread_time / openmp_time) << "x speedup"
        << std::endl;
}

int main() {
    // Размеры для экспериментов
    std::vector<int> sizes = { 1000, 5000, 10000, 20000, 30000, 40000, 50000 };
    std::vector<int> thread_counts = { 1, 2, 4, 8 };

    // Заголовок таблицы
    std::cout << std::setw(10) << "Size"
        << std::setw(15) << "Threads"
        << std::setw(20) << "Single-thread Time"
        << std::setw(20) << "OpenMP Time"
        << std::setw(20) << "Speedup"
        << std::endl;

    for (int size : sizes) {
        // Создаем случайный вектор и матрицу
        std::vector<double> vec = create_random_vector(size);
        std::vector<std::vector<double>> matrix = create_random_matrix(size);

        // Однопоточное умножение
        auto start = std::chrono::high_resolution_clock::now();
        std::vector<double> single_thread_result = multiply_vector_matrix_single_thread(vec, matrix);
        auto end = std::chrono::high_resolution_clock::now();
        double single_thread_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        for (int num_threads : thread_counts) {
            // Многопоточное умножение
            start = std::chrono::high_resolution_clock::now();
            std::vector<double> openmp_result = multiply_vector_matrix_openmp(vec, matrix, num_threads);
            end = std::chrono::high_resolution_clock::now();
            double openmp_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            // Проверка результатов
            bool results_match = true;
            for (int i = 0; i < size && results_match; ++i) {
                if (std::abs(single_thread_result[i] - openmp_result[i]) > 1e-6) {
                    results_match = false;
                }
            }

            if (!results_match) {
                std::cerr << "Results do not match for size = " << size << " and threads = " << num_threads << std::endl;
            }

            // Вывод результатов
            print_results_table(size, num_threads, single_thread_time, openmp_time);
        }
    }

    return 0;
}