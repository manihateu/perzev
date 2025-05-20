#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <omp.h>
#include <chrono>

using namespace std;

struct GameResult {
    int game_id;
    int player1_roll;
    int player2_roll;
    int winner; // 0 - ничья, 1 - игрок 1, 2 - игрок 2
};

// Общая память
vector<GameResult> shared_memory_results;
bool shared_memory_ready = false;
int shared_memory_counter = 0;

// Монитор
bool monitor_ready = false;
vector<GameResult> monitor_results;

// Сообщения
vector<GameResult> final_results_msg;
vector<bool> player1_done_msg;
vector<bool> player2_done_msg;

int roll_dice(mt19937& gen) {
    uniform_int_distribution<> dis(1, 6);
    return dis(gen);
}

void play_with_shared_memory(int num_games, ofstream& out_file) {
#pragma omp parallel num_threads(3)
    {
        int thread_id = omp_get_thread_num();

        if (thread_id == 0 || thread_id == 1) {
            mt19937 gen(chrono::system_clock::now().time_since_epoch().count() + thread_id);

            for (int i = 0; i < num_games; ++i) {
                int roll = roll_dice(gen);

#pragma omp critical
                {
                    if (thread_id == 0) {
                        shared_memory_results[i].player1_roll = roll;
                    }
                    else {
                        shared_memory_results[i].player2_roll = roll;
                    }
                    shared_memory_counter++;

                    if (shared_memory_counter == 2) {
                        shared_memory_ready = true;
                        shared_memory_counter = 0;
                    }
                }

                while (shared_memory_ready) {}
            }
        }
        else if (thread_id == 2) {
            for (int i = 0; i < num_games; ++i) {
                while (!shared_memory_ready) {}

                if (shared_memory_results[i].player1_roll > shared_memory_results[i].player2_roll) {
                    shared_memory_results[i].winner = 1;
                }
                else if (shared_memory_results[i].player1_roll < shared_memory_results[i].player2_roll) {
                    shared_memory_results[i].winner = 2;
                }
                else {
                    shared_memory_results[i].winner = 0;
                }

                out_file << "Game " << i + 1 << ": Player1=" << shared_memory_results[i].player1_roll
                    << ", Player2=" << shared_memory_results[i].player2_roll
                    << ", Winner=" << shared_memory_results[i].winner << endl;

#pragma omp critical
                {
                    shared_memory_ready = false;
                }
            }
        }
    }
}

void play_with_messages(int num_games, ofstream& out_file) {
    final_results_msg.resize(num_games);
    player1_done_msg.resize(num_games, false);
    player2_done_msg.resize(num_games, false);

#pragma omp parallel num_threads(3)
    {
        int thread_id = omp_get_thread_num();

        if (thread_id == 0 || thread_id == 1) {
            mt19937 gen(chrono::system_clock::now().time_since_epoch().count() + thread_id);

            for (int i = 0; i < num_games; ++i) {
                int roll = roll_dice(gen);

#pragma omp critical
                {
                    if (thread_id == 0) {
                        final_results_msg[i].game_id = i;
                        final_results_msg[i].player1_roll = roll;
                        player1_done_msg[i] = true;
                    }
                    else {
                        final_results_msg[i].game_id = i;
                        final_results_msg[i].player2_roll = roll;
                        player2_done_msg[i] = true;
                    }
                }
            }
        }
        else if (thread_id == 2) {
            for (int i = 0; i < num_games; ++i) {
                bool ready = false;
                while (!ready) {
#pragma omp critical
                    {
                        ready = player1_done_msg[i] && player2_done_msg[i];
                    }
                }

                if (final_results_msg[i].player1_roll > final_results_msg[i].player2_roll) {
                    final_results_msg[i].winner = 1;
                }
                else if (final_results_msg[i].player1_roll < final_results_msg[i].player2_roll) {
                    final_results_msg[i].winner = 2;
                }
                else {
                    final_results_msg[i].winner = 0;
                }

                out_file << "Game " << i + 1 << ": Player1=" << final_results_msg[i].player1_roll
                    << ", Player2=" << final_results_msg[i].player2_roll
                    << ", Winner=" << final_results_msg[i].winner << endl;
            }
        }
    }
}

void play_with_monitor(int num_games, ofstream& out_file) {
#pragma omp parallel num_threads(3)
    {
        int thread_id = omp_get_thread_num();

        if (thread_id == 0 || thread_id == 1) {
            mt19937 gen(chrono::system_clock::now().time_since_epoch().count() + thread_id);

            for (int i = 0; i < num_games; ++i) {
                int roll = roll_dice(gen);

#pragma omp critical
                {
                    if (monitor_results.size() <= i) {
                        monitor_results.resize(i + 1);
                        monitor_results[i].game_id = i;
                    }

                    if (thread_id == 0) {
                        monitor_results[i].player1_roll = roll;
                    }
                    else {
                        monitor_results[i].player2_roll = roll;
                    }

                    if (monitor_results[i].player1_roll != 0 && monitor_results[i].player2_roll != 0) {
                        monitor_ready = true;
                    }
                }

                while (monitor_ready) {}
            }
        }
        else if (thread_id == 2) {
            for (int i = 0; i < num_games; ++i) {
                while (!monitor_ready) {}

                if (monitor_results[i].player1_roll > monitor_results[i].player2_roll) {
                    monitor_results[i].winner = 1;
                }
                else if (monitor_results[i].player1_roll < monitor_results[i].player2_roll) {
                    monitor_results[i].winner = 2;
                }
                else {
                    monitor_results[i].winner = 0;
                }

                out_file << "Game " << i + 1 << ": Player1=" << monitor_results[i].player1_roll
                    << ", Player2=" << monitor_results[i].player2_roll
                    << ", Winner=" << monitor_results[i].winner << endl;

#pragma omp critical
                {
                    monitor_ready = false;
                }
            }
        }
    }
}

int main() {
    const int num_games = 10000;
    int choice;

    cout << "Выберите вариант организации взаимодействия нитей:\n";
    cout << "1. Общая память\n";
    cout << "2. Сообщения\n";
    cout << "3. Монитор\n";
    cout << "Ваш выбор: ";
    cin >> choice;

    ofstream out_file("dice_game_results.txt");
    if (!out_file) {
        cerr << "Не удалось открыть файл для записи результатов.\n";
        return 1;
    }

    shared_memory_results.resize(num_games);
    monitor_results.reserve(num_games);

    auto start = chrono::high_resolution_clock::now();

    switch (choice) {
    case 1:
        play_with_shared_memory(num_games, out_file);
        break;
    case 2:
        play_with_messages(num_games, out_file);
        break;
    case 3:
        play_with_monitor(num_games, out_file);
        break;
    default:
        cout << "Неверный выбор.\n";
        return 1;
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    int player1_wins = 0, player2_wins = 0, draws = 0;

    if (choice == 1) {
        for (const auto& result : shared_memory_results) {
            if (result.winner == 1) player1_wins++;
            else if (result.winner == 2) player2_wins++;
            else draws++;
        }
    }
    else if (choice == 2) {
        for (const auto& result : final_results_msg) {
            if (result.winner == 1) player1_wins++;
            else if (result.winner == 2) player2_wins++;
            else draws++;
        }
    }
    else {
        for (const auto& result : monitor_results) {
            if (result.winner == 1) player1_wins++;
            else if (result.winner == 2) player2_wins++;
            else draws++;
        }
    }

    cout << "\nРезультаты после " << num_games << " игр:\n";
    cout << "Игрок 1 победил: " << player1_wins << " раз\n";
    cout << "Игрок 2 победил: " << player2_wins << " раз\n";
    cout << "Ничьих: " << draws << " раз\n";
    cout << "Время выполнения: " << duration.count() << " секунд\n";

    out_file.close();
    return 0;
}