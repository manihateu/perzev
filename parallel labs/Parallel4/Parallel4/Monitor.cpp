#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>

using namespace std;

// Монитор
class Monitor {
private:
    int player1_score = 0, player2_score = 0;
    int games_played = 0;
    mutex mtx;
    condition_variable cv;
    bool finished = false;

public:
    void play(int player, int result) {
        unique_lock<mutex> lock(mtx);
        if (player == 1 && result > player2_score) player1_score++;
        if (player == 2 && result > player1_score) player2_score++;
        games_played++;
        if (games_played >= 10000) finished = true;
        cv.notify_all();
    }

    bool isGameFinished() {
        unique_lock<mutex> lock(mtx);
        return finished;
    }

    pair<int, int> getScores() {
        unique_lock<mutex> lock(mtx);
        return { player1_score, player2_score };
    }
};

Monitor monitor;

// Функция для генерации случайного числа (броска кубика)
int rollDice() {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> dis(1, 6);
    return dis(gen);
}

// Поток для первого игрока
void player1() {
    while (!monitor.isGameFinished()) {
        int dice_roll = rollDice();
        monitor.play(1, dice_roll);
    }
}

// Поток для второго игрока
void player2() {
    while (!monitor.isGameFinished()) {
        int dice_roll = rollDice();
        monitor.play(2, dice_roll);
    }
}

// Поток для мониторинга
void monitorThread() {
    ofstream file("game_results.txt");
    while (!monitor.isGameFinished()) {
        this_thread::sleep_for(chrono::milliseconds(10));
        pair<int, int> scores = monitor.getScores(); // Явное извлечение пары
        int score1 = scores.first;
        int score2 = scores.second;
        file << "Player 1: " << score1 << ", Player 2: " << score2 << endl;
    }
    file.close();
}

int main() {
    thread t1(player1);
    thread t2(player2);
    thread t3(monitorThread);

    t1.join();
    t2.join();
    t3.join();

    pair<int, int> final_scores = monitor.getScores(); // Явное извлечение финальных результатов
    int score1 = final_scores.first;
    int score2 = final_scores.second;

    cout << "Final Score - Player 1: " << score1 << ", Player 2: " << score2 << endl;
    return 0;
}