#include <iostream>
#include <vector>
#include <random>
#include <omp.h>
#include <chrono>
#include <functional> // Для std::function
#include <iomanip>    // Для форматирования вывода

// Функция для генерации случайной матрицы
std::vector<std::vector<double>> generateRandomMatrix(int size) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    std::vector<std::vector<double>> matrix(size, std::vector<double>(size));
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            matrix[i][j] = dis(gen);
        }
    }
    return matrix;
}

// Функция для генерации случайного вектора
std::vector<double> generateRandomVector(int size) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    std::vector<double> vector(size);
    for (int i = 0; i < size; ++i) {
        vector[i] = dis(gen);
    }
    return vector;
}

// Последовательное умножение вектора на матрицу
std::vector<double> sequentialMatrixVectorMultiplication(const std::vector<std::vector<double>>& matrix,
    const std::vector<double>& vector) {
    int size = matrix.size();
    std::vector<double> result(size, 0.0);

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            result[i] += matrix[i][j] * vector[j];
        }
    }

    return result;
}

// Параллельное умножение вектора на матрицу
std::vector<double> parallelMatrixVectorMultiplication(const std::vector<std::vector<double>>& matrix,
    const std::vector<double>& vector, int numThreads) {
    int size = matrix.size();
    std::vector<double> result(size, 0.0);

#pragma omp parallel num_threads(numThreads)
    {
#pragma omp for
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                result[i] += matrix[i][j] * vector[j];
            }
        }
    }

    return result;
}

// Функция для измерения времени выполнения
double measureTime(std::function<void()> func) {
    auto startTime = std::chrono::high_resolution_clock::now();
    func();
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    return duration.count();
}

int main() {
    // Размеры матриц и векторов для тестирования
    const int sizes[] = { 1000, 2000, 4000, 60000 };
    const int threadCounts[] = { 1, 2, 4, 8 };

    // Таблица для хранения результатов
    std::vector<std::vector<double>> results(sizes, std::vector<double>(threadCounts));

    for (size_t s = 0; s < sizeof(sizes) / sizeof(sizes[0]); ++s) {
        int size = sizes[s];

        // Генерация матрицы и вектора
        std::vector<std::vector<double>> matrix = generateRandomMatrix(size);
        std::vector<double> vector = generateRandomVector(size);

        std::cout << "\nРазмер объектов: " << size << std::endl;

        for (size_t t = 0; t < sizeof(threadCounts) / sizeof(threadCounts[0]); ++t) {
            int numThreads = threadCounts[t];

            if (numThreads == 1) {
                // Последовательное умножение
                auto sequentialFunc = [&]() {
                    sequentialMatrixVectorMultiplication(matrix, vector);
                    };
                double time = measureTime(sequentialFunc);
                results[s][t] = time;
                std::cout << "Последовательное умножение (время): " << time << " мс" << std::endl;
            }
            else {
                // Параллельное умножение
                auto parallelFunc = [&]() {
                    parallelMatrixVectorMultiplication(matrix, vector, numThreads);
                    };
                double time = measureTime(parallelFunc);
                results[s][t] = time;
                std::cout << "Параллельное умножение (" << numThreads << " потоков, время): " << time << " мс" << std::endl;
            }
        }
    }

    // Вывод результатов в виде таблицы
    std::cout << "\nТаблица результатов:\n";
    std::cout << std::setw(20) << "Размер объектов"
        << std::setw(25) << "Последовательный (время)"
        << std::setw(25) << "2 процесса (время)"
        << std::setw(25) << "4 процесса (время)"
        << std::setw(25) << "8 процесса (время)"
        << std::endl;

    for (size_t s = 0; s < sizeof(sizes) / sizeof(sizes[0]); ++s) {
        std::cout << std::setw(20) << sizes[s];
        for (size_t t = 0; t < sizeof(threadCounts) / sizeof(threadCounts[0]); ++t) {
            std::cout << std::setw(25) << results[s][t];
        }
        std::cout << std::endl;
    }

    return 0;
}