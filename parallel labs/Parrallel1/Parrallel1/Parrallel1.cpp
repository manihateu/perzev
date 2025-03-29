#include <iostream>
#include <fstream>
#include <random>
#include <omp.h>
#include <chrono>
#include <iomanip>
#include <vector>

void generateRandomNumbers(int threadCount, bool useCriticalSection, int size) {
    std::ofstream outputFile("output.txt");
    if (!outputFile) {
        std::cerr << "Ошибка открытия файла!" << std::endl;
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1000000);

    std::vector<std::chrono::high_resolution_clock::time_point> startTimes(threadCount);
    std::vector<std::chrono::high_resolution_clock::time_point> endTimes(threadCount);

#pragma omp parallel num_threads(threadCount)
    {
        int tid = omp_get_thread_num();
        int start = tid * (size / threadCount);
        int end = (tid + 1) * (size / threadCount);

        startTimes[tid] = std::chrono::high_resolution_clock::now();

        for (int i = start; i < end; ++i) {
            int randomNumber = dis(gen);

            if (useCriticalSection) {
#pragma omp critical
                {
                    outputFile << tid << "," << i << "," << randomNumber << std::endl;
                }
            }
            else {
                outputFile << tid << "," << i << "," << randomNumber << std::endl;
            }
        }

        endTimes[tid] = std::chrono::high_resolution_clock::now();
    }

    for (int i = 0; i < threadCount; ++i) {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTimes[i] - startTimes[i]
        );
        std::cout << "Поток " << i << ": "
            << "Начало: " << startTimes[i].time_since_epoch().count()
            << ", Окончание: " << endTimes[i].time_since_epoch().count()
            << ", Длительность: " << duration.count() << " мс" << std::endl;
    }

    outputFile.close();
}

void printResultsTable(const std::vector<std::vector<double>>& results, const std::vector<int>& sizes) {
    std::cout << std::setw(20) << "Размер объектов"
        << std::setw(25) << "2 потока без критической секции"
        << std::setw(25) << "2 потока с критической секцией"
        << std::setw(25) << "4 потока без критической секции"
        << std::setw(25) << "4 потока с критической секцией"
        << std::setw(25) << "8 потоков без критической секции"
        << std::setw(25) << "8 потоков с критической секцией"
        << std::endl;

    for (size_t i = 0; i < sizes.size(); ++i) {
        std::cout << std::setw(20) << sizes[i];
        for (size_t j = 0; j < results[i].size(); ++j) {
            std::cout << std::setw(25) << results[i][j];
        }
        std::cout << std::endl;
    }
}

int main() {
    const int threadCounts[] = { 2, 4, 8 };
    const bool useCriticalSections[] = { false, true };
    const int sizes[] = { 100000, 250000, 500000 };
    setlocale(LC_ALL, "ru");
    std::vector<std::vector<double>> results(3, std::vector<double>(6, 0.0));

    for (size_t s = 0; s < 3; ++s) {
        int size = sizes[s];
        for (size_t t = 0; t < 3; ++t) {
            int threadCount = threadCounts[t];
            for (size_t c = 0; c < 2; ++c) {
                bool useCriticalSection = useCriticalSections[c];

                std::cout << "\nРазмер объектов: " << size
                    << ", Потоков: " << threadCount
                    << ", Критическая секция: " << (useCriticalSection ? "Да" : "Нет")
                    << std::endl;

                auto startTime = std::chrono::high_resolution_clock::now();

                generateRandomNumbers(threadCount, useCriticalSection, size);

                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

                int index = t * 2 + c; // Индекс для колонки в таблице
                results[s][index] = duration.count();

                std::cout << "Общее время выполнения: " << duration.count() << " мс" << std::endl;
            }
        }
    }

    std::vector<int> sizesVector(sizes, sizes + 3);
    printResultsTable(results, sizesVector);

    return 0;
}