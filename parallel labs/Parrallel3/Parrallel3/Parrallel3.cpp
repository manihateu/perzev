#include <iostream>
#include <vector>
#include <cmath>
#include <omp.h>

using namespace std;

// Функция для вывода матрицы
void printMatrix(const vector<vector<double>>& matrix) {
    setlocale(LC_ALL, "Ru");
    for (const auto& row : matrix) {
        for (double elem : row) {
            cout << elem << "\t";
        }
        cout << endl;
    }
}

// UL-разложение с выбором главного элемента по столбцу
void ulDecomposition(vector<vector<double>>& A, vector<vector<double>>& U, vector<vector<double>>& L, int n) {
    // Инициализация матриц U и L
    U = A; // Копируем исходную матрицу в U
    L.assign(n, vector<double>(n, 0.0)); // L заполняется нулями

    for (int i = 0; i < n; ++i) {
        L[i][i] = 1.0; // Диагональные элементы L равны 1
    }

    // Основной цикл разложения
    for (int k = 0; k < n; ++k) {
        // Выбор главного элемента по столбцу
        int maxRow = k;
        double maxValue = abs(U[k][k]);
        for (int i = k + 1; i < n; ++i) {
            if (abs(U[i][k]) > maxValue) {
                maxValue = abs(U[i][k]);
                maxRow = i;
            }
        }

        // Меняем строки местами в U
        if (maxRow != k) {
            swap(U[k], U[maxRow]);
            swap(L[k], L[maxRow]);
            // Также меняем элементы ниже диагонали в L
            for (int i = 0; i < k; ++i) {
                swap(L[k][i], L[maxRow][i]);
            }
        }

        // Проверка на нулевой диагональный элемент
        if (U[k][k] == 0.0) {
            cerr << "Матрица вырождена, разложение невозможно." << endl;
            return;
        }

        // Обновление элементов матрицы L и U
#pragma omp parallel for
        for (int i = k + 1; i < n; ++i) {
            L[i][k] = U[i][k] / U[k][k];
            for (int j = k; j < n; ++j) {
                U[i][j] -= L[i][k] * U[k][j];
            }
        }
    }
}

int main() {
    int n;
    cout << "Введите размер матрицы: ";
    cin >> n;
    setlocale(LC_ALL, "Ru");
    vector<vector<double>> A(n, vector<double>(n));
    cout << "Введите элементы матрицы:" << endl;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            cin >> A[i][j];
        }
    }

    vector<vector<double>> U, L;

    // Выполняем UL-разложение
    ulDecomposition(A, U, L, n);

    cout << "Матрица U:" << endl;
    printMatrix(U);

    cout << "Матрица L:" << endl;
    printMatrix(L);

    return 0;
}