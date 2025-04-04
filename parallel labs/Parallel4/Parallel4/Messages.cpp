#include <iostream>
#include <fstream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <random>

using namespace std;

// Очередь сообщений
queue<pair<int, int>> message_queue; // (игрок, результат)
mutex mtx;
condition_variable cv;
bool game_finished = false; // Флаг завершения игры

// Глобальные счетчики
int player1_score = 0, player2_score = 0;

// Функция для генерации случайного числа (броска кубика)
int rollDice() {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> dis(1, 6);
    return dis(gen);
}

// Поток для первого игрока
void player1() {
    for (int i = 0; i < 10000; ++i) {
        int dice_roll = rollDice();
        unique_lock<mutex> lock(mtx);
        message_queue.push({ 1, dice_roll });
        cv.notify_all(); // Уведомляем монитор
    }
}

// Поток для второго игрока
void player2() {
    for (int i = 0; i < 10000; ++i) {
        int dice_roll = rollDice();
        unique_lock<mutex> lock(mtx);
        message_queue.push({ 2, dice_roll });
        cv.notify_all(); // Уведомляем монитор
    }
}

// Поток для мониторинга
void monitor() {
    ofstream file("game_results.txt");
    int games_played = 0;

    while (games_played < 10000) {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [] { return !message_queue.empty() || game_finished; });

        while (!message_queue.empty()) {
            pair<int, int> message = message_queue.front();
            message_queue.pop();

            int player = message.first;
            int result = message.second;

            if (player == 1 && result > player2_score) player1_score++;
            if (player == 2 && result > player1_score) player2_score++;
            games_played++;

            file << "Player 1: " << player1_score << ", Player 2: " << player2_score << endl;
        }
    }

    file.close();
    game_finished = true; // Устанавливаем флаг завершения игры
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