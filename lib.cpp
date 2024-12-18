#include <stddef.h>
#include <cstdio>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <random>
#include <unistd.h>
#include <regex>
#include <string>
#include <sstream>
#include <filesystem>


using namespace std;


#define BUFFER_SIZE 128
#define NUM_COLORS 4

void create_directories() {
    // Criando o diretório scoreboard
    if(mkdir("SERVER", 0777) == -1) {
        perror("Erro ao criar diretório SERVER");
    }
    if (mkdir("SERVER/SCORES", 0777) == -1) {
        perror("Erro ao criar diretório SCORES");
    } 

    // Criando o diretório show_trials
    if (mkdir("SERVER/GAMES", 0777) == -1) {
        perror("Erro ao criar diretório GAMES");
    }
}

void create_game_dir(const string &plid) {
    string path = "SERVER/GAMES/" + plid;
    if (mkdir(path.c_str(), 0777) == -1) {
        perror("Erro ao criar diretório do jogo");
    }
}

ofstream create_file(const string& directory, const string& filename) {
    string file_path = directory + "/" + filename;

    ofstream file(file_path);
    
    if (!file) {
        cerr << "Erro ao criar o arquivo: " << file_path << endl;
    }

    return file;
}

int code_val(const string& code) {
    regex pattern("^([RGBYOP]) ([RGBYOP]) ([RGBYOP]) ([RGBYOP])");
    
    return regex_match(code, pattern);
}

bool valid_time(const string& str) {
    try {
        int num = stoi(str);
        return num >= 0 && num <= 600;
    } catch (...) {
        return false;
    }
}

void generate_random_colors(char *result) {
    // Array de cores disponíveis
    char colors[] = {'R', 'G', 'B', 'Y', 'O', 'P'};
    size_t num_available_colors = sizeof(colors) / sizeof(colors[0]);

    // Inicializando gerador de números aleatórios
    std::random_device rd;          // Gerador baseado em hardware (ou fallback para entropia pseudoaleatória)
    std::mt19937 gen(rd());         // Mersenne Twister PRNG
    std::uniform_int_distribution<> dist(0, num_available_colors - 1); // Índices aleatórios no intervalo válido

    // Gerando a sequência aleatória de cores
    for (int i = 0; i < NUM_COLORS; i++) {
        int random_index = dist(gen); // Gera um índice aleatório
        result[i] = colors[random_index];
    }
    result[NUM_COLORS] = '\0'; // Adiciona o terminador nulo para criar uma string válida
}



