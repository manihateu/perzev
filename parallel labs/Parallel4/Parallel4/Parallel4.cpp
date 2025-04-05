#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <random>

using namespace std;

// Глобальные переменные
int player1_score = 0, player2_score = 0;
int games_played = 0;
mutex mtx; // Мьютекс для синхронизации доступа к общей памяти

// Функция для генерации случайного числа (броска кубика)
int rollDice() {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> dis(1, 6);
    return dis(gen);
}

// Поток для первого игрока
void player1() {
    while (games_played < 10000) {
        int dice_roll = rollDice();
        lock_guard<mutex> lock(mtx); // Блокировка мьютекса
        if (dice_roll > player2_score) player1_score++;
        games_played++;
    }
}

// Поток для второго игрока
void player2() {
    while (games_played < 10000) {
        int dice_roll = rollDice();
        lock_guard<mutex> lock(mtx); // Блокировка мьютекса
        if (dice_roll > player1_score) player2_score++;
        games_played++;
    }
}

// Поток для мониторинга
void monitor() {
    ofstream file("game_results.txt");
    while (games_played < 10000) {
        this_thread::sleep_for(chrono::milliseconds(10)); // Задержка для монитора
        lock_guard<mutex> lock(mtx); // Блокировка мьютекса
        file << "Player 1: " << player1_score << ", Player 2: " << player2_score << endl;
    }
    file.close();
}

int main() {
    thread t1(player1);
    thread t2(player2);
    thread t3(monitor);

    t1.join();
    t2.join();
    t3.join();

    cout << "Final Score - Player 1: " << player1_score << ", Player 2: " << player2_score << endl;
    return 0;
}