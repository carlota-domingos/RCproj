#include "lib.h"
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
#include <list>
#include <thread>
#include <chrono>
#include <atomic>
#include <ctime>
#include <algorithm>
#include <vector>
#include <fcntl.h>
#include <dirent.h>

using namespace std;

vector<game_player> players;

game_file::game_file(const string &id, const string &mode, const string &code, const string &timeout, time_t time_i)
{
    plid = id;
    if (mode == "P")
    {
     game_mode = "PLAY";
    }
    else
    {
        game_mode = "DEBUG";
    }
    string path = "SERVER/GAMES/" + plid + "/GAME_" + plid + ".txt";
    path_file = path;
    time_init = time_i;
    int fd_game = open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd_game == -1)
    {
        perror("Erro ao abrir o ficheiro de jogo");
        error = 1;
    }
    string firstline = plid + " " + mode + " " + code + " " + timeout + " " + get_str_time(time_init, 0) + " " + to_string(time_init) + "\n";
    if (write(fd_game, firstline.c_str(), firstline.size()) == -1)
    {
        perror("Erro ao escrever no ficheiro de jogo");
        error = 1;
    }
    close(fd_game);
}

void game_file::new_line(const string &code, int nb, int nw, time_t play_time)
{
    cout << time_init << endl;
    cout << play_time << endl;
    time_t game_time = play_time - time_init;
    string time_str = to_string(game_time);
    string line = "T: " + code.substr(0, 4) + " " + to_string(nb) + " " + to_string(nw) + " " + time_str + "\n";
    int fd_game = open(path_file.c_str(), O_WRONLY | O_APPEND);
    if (write(fd_game, line.c_str(), line.size()) == -1)
    {
        perror("Erro ao escrever no ficheiro de jogo");
        error = 1;
    }
    close(fd_game);
}

string game_file::get_code_file(){
    int fd_game = open(path_file.c_str(), O_RDONLY);
    if (fd_game == -1)
    {
        perror("Erro ao abrir o ficheiro de jogo");
        error = 1;
    }
    string code;
    char buffer[50];
    if (read(fd_game, buffer, 50) == -1)
    {
        perror("Erro ao ler o ficheiro de jogo");
        error = 1;
    }
    // get third word of the first line
    int i = 0;
    int count = 0;
    while (count < 2)
    {
        if (buffer[i] == ' ')
        {
            count++;
        }
        i++;
    }
    while (buffer[i] != ' ')
    {
        code += buffer[i];
        i++;
    }
    close(fd_game);
    return code;
}

int game_file::get_nT_file()
{
    // counts the number of lines in the file minus the first line
    int fd_game = open(path_file.c_str(), O_RDONLY);
    if (fd_game == -1)
    {
        perror("Erro ao abrir o ficheiro de jogo");
        error = 1;
    }
    int nT = 0;
    char buffer[50];
    while (read(fd_game, buffer, 50) != 0)
    {
        nT++;
    }
    close(fd_game);
    return nT - 1;
}

string game_file::get_str_time(time_t time, int mode)
{
    char buffer[20];
    struct tm *timeinfo;
    if (mode == 0)
    {
        timeinfo = localtime(&time);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    }
    else
    {
        timeinfo = localtime(&time);
        strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", timeinfo);
    }
    return string(buffer);
}

void game_file::finish_game(time_t finishtime, const string &term, const string &score)
{
    time_t game_time = finishtime - time_init;
    string game_time_str = to_string(game_time);
    string last_line = get_str_time(finishtime, 0) + " " + game_time_str + "\n";
    string new_path = "SERVER/GAMES/" + plid + "/" + get_str_time(finishtime, 1) + "_" + term + ".txt";
    if (term == "W")
    {
        string score_fn = score + "_" + plid + "_" + get_str_time(finishtime, 1) + ".txt";
        string path = "SERVER/SCORES/" + score_fn;
        int score_fd = open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (score_fd == -1)
        {
            perror("Erro ao abrir o ficheiro de scores");
            error = 1;
        }
        string score_line = score + " " + plid + " " + get_code_file() + " " + to_string(get_nT_file()) + " " + game_mode + "\n";
        if (write(score_fd, score_line.c_str(), score_line.size()) == -1)
        {
            perror("Erro ao escrever no ficheiro de scores");
            error = 1;
        }
        close(score_fd);
    }
}

void game_file::display_info() const
{
    cout << "Player ID: " << plid << "\n";
    cout << "Time Init: " << time_init << "\n";
}

ofstream create_file(const string &directory, const string &filename)
{
    string file_path = directory + "/" + filename;

    ofstream file(file_path);

    if (!file)
        cerr << "Erro ao criar o arquivo: " << file_path << endl;
    
    return file;
}

game_player::game_player(const string &id) : plid(id), ativo(false)
{
    create_game_dir(plid);
}

void game_player::start_game(int tempo, string &cores, game_file *gfile, time_t tempo_inicio)
{
    time = tempo;
    tempo_inicio_jogo = tempo_inicio;
    ativo = true;
    nT = 1;
    codigo = cores;
    file = gfile;
}

time_t game_player::get_tempo_inicio_jogo() const
{
    return tempo_inicio_jogo;
}

bool game_player::game_time_act(time_t now)
{
    return (now - tempo_inicio_jogo) < time;
}

int game_player::findtry(string &code)
{
    return 0;
}

void game_player::reset(const string &id)
{
    nT = 1;
    plid = "";
    codigo = "";
}

void game_player::next_try()
{
    nT++;
}

bool game_player::same_try(int server_try) const
{
    return nT == server_try;
}

void game_player::update_codigo(const string &new_code)
{
    codigo = new_code;
}

void game_player::finish(const string &term, time_t time)
{
    ativo = false;
    file->finish_game(time, term, to_string(NUM_TRIES - nT));
    nT = 0;
    codigo = "";
    time = 0;
    tempo_inicio_jogo = 0;
}

void game_player::display_info() const
{
    cout << "Player ID: " << plid << "\n";
    cout << "tempo " << time << "\n";
    cout << "tentativas " << nT << "\n";
    cout << "Code: " << codigo << "\n";
}

string get_termination_type(const string &type)
{
    if (type == "W")
        return "WIN";
    if (type == "F")
        return "FAIL";
    if (type == "T")
        return "TIMEOUT";
    if (type == "Q")
        return "QUIT";
    return "UNKNOWN";
}

void create_directories()
{
    // Criando o diretório scoreboard
    if (mkdir("SERVER", 0777) == -1)
    {
        perror("Erro ao criar diretório SERVER");
    }
    if (mkdir("SERVER/SCORES", 0777) == -1)
    {
        perror("Erro ao criar diretório SCORES");
    }

    // Criando o diretório show_trials
    if (mkdir("SERVER/GAMES", 0777) == -1)
    {
        perror("Erro ao criar diretório GAMES");
    }
}

void create_game_dir(const string &plid)
{
    string path = "SERVER/GAMES/" + plid;
    if (mkdir(path.c_str(), 0777) == -1)
    {
        perror("Erro ao criar diretório do jogo");
    }
}

int code_val(const string &code)
{
    regex pattern("^([RGBYOP]) ([RGBYOP]) ([RGBYOP]) ([RGBYOP])");

    return regex_match(code, pattern);
}

bool valid_time(const string &str)
{
    try
    {
        int num = stoi(str);
        return num >= 0 && num <= 600;
    }
    catch (...)
    {
        return false;
    }
}

void generate_random_colors(char *result)
{
    // Array de cores disponíveis
    char colors[] = {'R', 'G', 'B', 'Y', 'O', 'P'};
    size_t num_available_colors = sizeof(colors) / sizeof(colors[0]);

    // Inicializando gerador de números aleatórios
    std::random_device rd;                                             // Gerador baseado em hardware (ou fallback para entropia pseudoaleatória)
    std::mt19937 gen(rd());                                            // Mersenne Twister PRNG
    std::uniform_int_distribution<> dist(0, num_available_colors - 1); // Índices aleatórios no intervalo válido

    // Gerando a sequência aleatória de cores
    for (int i = 0; i < NUM_COLORS; i++)
    {
        int random_index = dist(gen); // Gera um índice aleatório
        result[i] = colors[random_index];
    }
    result[NUM_COLORS] = '\0'; // Adiciona o terminador nulo para criar uma string válida
}

int FindTopScores(list<string> *list)
{
    struct dirent **filelist;
    int nentries, ifile;
    char fname[512]; // Increased buffer size
    FILE *fp;
    nentries = scandir("SERVER/SCORES/", &filelist, 0, alphasort);
    ifile = 0;
    if (nentries < 0)
    {
        perror("Erro ao ler o diretório de scores");
        return 0;
    }
    else
    {
        while (nentries--)
        {
            if (filelist[nentries]->d_name[0] != '.')
            {
                snprintf(fname, sizeof(fname), "SERVER/SCORES/%s", filelist[nentries]->d_name);
                fp = fopen(fname, "r");
                if (fp != NULL)
                {
                    char mode[10];
                    int score, notries;
                    char PLID[50], colcode[50];
                    if (fscanf(fp, "%d %s %s %d %s", &score, PLID, colcode, &notries, mode) == 5)
                    {
                        string score_entry = to_string(score) + "  " + PLID + "     " + colcode + "        " + to_string(notries) + "       " + mode;
                        list->push_back(score_entry);
                    }
                    fclose(fp);
                    ++ifile;
                }
            }
            free(filelist[nentries]);
            if (ifile == 10)
                break;
        }
        free(filelist);
    }
    return ifile;
}

int FindLastGame(string &PLID_str, char *fname)
{
    const char *PLID = PLID_str.c_str();
    struct dirent **filelist;
    int nentries, found;
    char dirname[20];
    sprintf(dirname, "SERVER/GAMES/%s/", PLID);
    nentries = scandir(dirname, &filelist, 0, alphasort);
    found = 0;

    if (nentries <= 0)
        return (0);
    else
    {
        while (nentries--)
        {
            if (filelist[nentries]->d_name[0] != '.')
            {
                sprintf(fname, "SERVER/GAMES/%s/%s", PLID, filelist[nentries]->d_name);
                found = 1;
            }
            free(filelist[nentries]);
            if (found)
                break;
        }
        free(filelist);
    }
    return (found);
}

int find_play(string &PLID, string &code)
{
    printf("ola find play");
    char *file_name = (char *)malloc(50);
    if (FindLastGame(PLID, file_name) == 0)
    {
        free(file_name);
        return 0;
    }
    FILE *fp = fopen(file_name, "r");
    if (fp == NULL)
    {
        free(file_name);
        return 0;
    }
    char code_buffer[50];
    int nB, nW, time;
    // Skip the first line
    char buffer[256];
    printf("ola find play anteds primeira linha");
    if (fgets(buffer, sizeof(buffer), fp) == NULL)
    {
        fclose(fp);
        free(file_name);
        return 0;
    }
    // Read the remaining lines
    while (fscanf(fp, "T: %s %d %d %d\n", code_buffer, &nB, &nW, &time) != EOF)
    {
        printf("ola find play dentro do while");
        string code_str(code_buffer);
        if (code_str.substr(0, 4).compare(code.substr(0, 4)) == 0)
        {
            fclose(fp);
            free(file_name);
            return 1;
        }
    }
    printf("ola find play fora do while");
    fclose(fp);
    free(file_name);
    return 0;
}

void format_scb(int file_size, string &buffer)
{
    buffer = "-------------------------------- TOP 10 SCORES --------------------------------\n";
    buffer += "                 SCORE PLAYER     CODE    NO TRIALS   MODE\n";
    list<string> score_list;
    FindTopScores(&score_list);
    int score_number = 1;
    string n = " ";
    for (auto it = score_list.begin(); it != score_list.end(); ++it)
    {
        if (score_number == 10)
            n = "";
        buffer += "             " + n + to_string(score_number) + " - " + *it + "\n";
        score_number++;
    }
    file_size = buffer.size();
}

void match_code(const string &code1, const string &code2, int &nW, int &nB)
{
    nW = 0;
    nB = 0;
    cout << "Code 1: " << code1 << endl;
    cout << "Code 2: " << code2 << endl;
    for (int i = 0; i < NUM_COLORS; i++)
    {
        cout << "olaaaa match_code" << endl;
        if (code1[i] == code2[i])
        {
            nB++;
        }
        else
        {
            for (int j = 0; j < NUM_COLORS; j++)
            {
                if (code1[i] == code2[j])
                {
                    nW++;
                    break;
                }
            }
        }
    }
}

string get_str_time(time_t time, int mode)
{
    char buffer[20];
    struct tm *timeinfo;
    if (mode == 0)
    {
        timeinfo = localtime(&time);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    }
    else
    {
        timeinfo = localtime(&time);
        strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", timeinfo);
    }
    return string(buffer);
}

void process_player(game_player *player, string &code, int nT, string &send_buffer, time_t play_time)
{
    
    if (player == nullptr) {
        std::cerr << "Erro: player é nullptr!" << std::endl;
        return;
    }

    if (player->plid.empty()) {
        std::cerr << "Erro: plid está vazio!" << std::endl;
        return;
    }
    else if (player->ativo) {
        if (player->game_time_act(play_time) == false)
        {

            cout << "Jogador com PLID " << player->plid << " não está ativo." << endl;
            cout << "Tempo esgotado para o jogador." << endl;
            send_buffer = "RTR ETM " + player->codigo + "\n";
            player->finish("T", play_time);
            return;
        }
        if (player->same_try(nT) || (player->same_try(nT - 1) && find_play(player->plid, code)))            {
            cout << "ola22222" << endl;
            std::cout << "Jogador com PLID: ";
            for (char c : player->plid) {
                std::cout << "carater: " << c << std::endl;
            }
            std::cout << " está ativo." << std::endl;
            cout << "ola22222" << endl;
            int nW = 0;
            int nB = 0;
            if (find_play(player->plid, code) && player->same_try(nT))
            {
                cout << "Tentiva duplicada." << endl;
                send_buffer = "RTR DUP\n";
                return;
            }
            cout << "ola22222" << endl;
            match_code(player->codigo, code, nW, nB);
            cout << "nW: " << nW << " nB: " << nB << endl;
            if (nT == NUM_TRIES && nB != NUM_COLORS)
            {
                cout << "Número de tentativas esgotado." << endl;
                send_buffer = "RTR ENT " + player->codigo + "\n";
                player->finish("F", play_time);
                return;
            }
            else if (nB == NUM_COLORS)
            {

                cout << "Jogador com PLID " << player->plid << " acertou no código." << endl;
                player->next_try();
                player->file->new_line(code, nB, nW, play_time);
                send_buffer = "RTR OK " + to_string(player->nT) + " " + to_string(nB) + " " + to_string(nW) + " " + player->codigo + "\n";
                player->finish("W", play_time);
                return;
            }
            send_buffer = "RTR OK " + to_string(player->nT) + " " + to_string(nB) + " " + to_string(nW) + "\n";
            if (player->same_try(nT))
            {
                player->next_try();
                player->file->new_line(code, nB, nW, play_time);
            }
            return;
        }
        else
        {
            cout << "Número de tentativas inválido" << endl;
            send_buffer = "RTR INV\n";
            return;
        }
    }
    else {
        printf("jogo nao encontrado");
        if (player->codigo.empty()) {
            printf("codigo vazio");
            cout << "Jogador com PLID " << player->plid << " não está ativo." << endl;
            send_buffer = "RTR NOK\n";
            return;
        }
    }
}



void format_str(string &scorefilename, int file_size, string &buffer, string &code)
{
    buffer = "";
    ifstream file(scorefilename);
    if (file.is_open())
    {
        string line;
        vector<string> lines;
        while (getline(file, line))
        {
            lines.push_back(line);
        }
        file.close();

        if (lines.empty())
            return;

        // Extract initial game information
        stringstream ss(lines[0]);
        string plid, mode, secret_code, timeout, init_date, init_time, init_epoch_str;
        ss >> plid >> mode >> secret_code >> timeout >> init_date >> init_time >> init_epoch_str;

        // Extract game initiation time from filename
        size_t pos = scorefilename.find_last_of('/');
        string filename = (pos == string::npos) ? scorefilename : scorefilename.substr(pos + 1);
        string init_time_str = init_date + " " + init_time; // Combine date and time
        string termination_type = filename.substr(16, 1);   // Extract termination type (W, F, Q, T)

        if (code == "RST OK")
        {
            buffer += "     Active game found for player " + plid + "\n";
            buffer += "Game initiated: " + init_time_str + " with " + timeout + " seconds to be completed\n\n";
            buffer += "     --- Transactions found: " + to_string(lines.size() - 1) + " ---\n\n";

            for (size_t i = 1; i < lines.size(); ++i)
            {
                stringstream ss(lines[i]);
                string trial_code;
                int nb, nw, trial_time;
                ss >> trial_code >> nb >> nw >> trial_time;
                buffer += "Trial: " + trial_code + ", nB: " + to_string(nb) + ", nW: " + to_string(nw) + " at " + to_string(trial_time) + "s\n";
            }

            buffer += "\n  -- " + timeout + " seconds remaining to be completed --\n";
        }
        else if (code == "RST FIN")
        {
            buffer += "Last finalized game for player " + plid + "\n";
            buffer += "Game initiated: " + init_time_str + " with " + timeout + "s to be completed\n";
            buffer += "Mode: " + mode + "  Secret code: " + secret_code + "\n\n";
            buffer += "     --- Transactions found: " + to_string(lines.size() - 2) + " ---\n";

            for (size_t i = 1; i < lines.size() - 1; ++i)
            {
                stringstream ss(lines[i]);
                string trial_code;
                int nb, nw, trial_time;
                ss >> trial_code >> nb >> nw >> trial_time;
                buffer += "Trial: " + trial_code + ", nB: " + to_string(nb) + ", nW: " + to_string(nw) + "   " + to_string(trial_time) + "s\n";
            }

            stringstream ss_last(lines.back());
            string end_date, end_time, duration_str;
            ss_last >> end_date >> end_time >> duration_str;
            buffer += "     Termination: " + get_termination_type(termination_type) + " at " + end_date + " " + end_time + ", Duration: " + duration_str + "s\n";
        }
    }
}

int validate_args(int argc, char *argv[], const char *&gs_port, bool &verbose)
{
    // Parse command-line arguments
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
            gs_port = argv[++i];

        else if (strcmp(argv[i], "-v") == 0)
            verbose = true;
        else
        {
            cerr << "Usage: " << argv[0] << " [-p GSport] [-v]" << endl;
            return 1;
        }
    }
    return 0;
}


game_player *find_player(const string &plid)
{
    cout << "Finding player with PLID: " << plid << endl;

    // Verifica se o vetor está vazio
    if (players.empty()) {
        cout << "Player list is empty. Cannot find any player." << endl;
        return nullptr;
    }

    auto it = find_if(players.begin(), players.end(), [&plid](const game_player &player) {
        return player.plid == plid;
    });

    if (it != players.end())
        return &(*it); // Retorna o ponteiro para o jogador encontrado

    return nullptr; // Retorna nullptr se não encontrar o jogador
}