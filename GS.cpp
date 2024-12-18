#include "lib.h"
#include "udpTcp.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <filesystem>
#include <ctime>
#include <list>
#include <algorithm>
#include <vector>
#include <fcntl.h>
#include <dirent.h>
#include <regex>

#define BUFFER_SIZE_GS 1024
#define NUM_COLORS 4
#define NUM_TRIES 8

using namespace std;
class game_player;
vector<game_player> players;

int FindTopScores(list<string> *list) {
    struct dirent **filelist;
    int nentries, ifile;
    char fname[512]; // Increased buffer size
    FILE *fp;
    nentries = scandir("SERVER/SCORES/", &filelist, 0, alphasort);
    ifile = 0;
    if (nentries < 0) {
        perror("Erro ao ler o diretório de scores");
        return 0;
    } else {
        while (nentries--) {
            if (filelist[nentries]->d_name[0] != '.') {
                snprintf(fname, sizeof(fname), "SERVER/SCORES/%s", filelist[nentries]->d_name);
                fp = fopen(fname, "r");
                if (fp != NULL) {
                    char mode[10];
                    int score, notries;
                    char PLID[50], colcode[50];
                    if (fscanf(fp, "%d %s %s %d %s", &score, PLID, colcode, &notries, mode) == 5) {
                        string score_entry = to_string(score) + "  " + PLID + "     " + colcode + "        " + to_string(notries) + "       " + mode;
                        list->push_back(score_entry);
                    }
                    fclose(fp);
                    ++ifile;
                }
            }
            free(filelist[nentries]);
            if (ifile == 10) break;
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
    if (fgets(buffer, sizeof(buffer), fp) == NULL)
    {
        fclose(fp);
        free(file_name);
        return 0;
    }
    // Read the remaining lines
    while (fscanf(fp, "T: %s %d %d %d\n", code_buffer, &nB, &nW, &time) != EOF)
    {
        string code_str(code_buffer);
        if (code_str.substr(0, 4).compare(code.substr(0, 4)) == 0)
        {
            fclose(fp);
            free(file_name);
            return 1;
        }
    }
    fclose(fp);
    free(file_name);
    return 0;
}

void match_code(const string &code1, const string &code2, int &nW, int &nB)
{

    nW = 0;
    nB = 0;
    cout << "Code 1: " << code1 << endl;
    cout << "Code 2: " << code2 << endl;
    for (int i = 0; i < NUM_COLORS; i++)
    {
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
    char buffer[20]; // Buffer for the date and time in YYYY-MM-DD HH:MM:SS format
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

void format_scb(int file_size, string &buffer)
{
    buffer = "-------------------------------- TOP 10 SCORES --------------------------------\n";
    buffer += "                 SCORE PLAYER     CODE    NO TRIALS   MODE\n";
    list<string> score_list;
    FindTopScores(&score_list);
    int score_number = 1;
    string n = " ";
    for (auto it = score_list.begin(); it != score_list.end(); ++it) {
        if (score_number == 10) n = "";
        buffer += "             " + n +to_string(score_number) + " - " + *it + "\n";
        score_number++;
    }
    file_size = buffer.size();
    
}

class game_file
{
public:
    string plid; // Identificador único do jogador
    string path_file;
    time_t time_init; // Tempo de referência para calcular o tempo das jogadas
    string game_mode; // Modo do jogo
    int error = 0;

    game_file(const string &id, const string &mode, const string &code, const string &timeout, time_t time_i)
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

    void new_line(const string &code, int nb, int nw, time_t play_time)
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

    string get_code_file()
    {
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

    int get_nT_file()
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
        return nT-1;
    }

    void finish_game(time_t finishtime, const string &term, const string &score)
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
        // change file name to new file name
        if (rename(path_file.c_str(), new_path.c_str()) == -1)
        {
            perror("Erro ao renomear o ficheiro de jogo");
            error = 1;
        }
        path_file = new_path;
        // Add the last line to the new file
        int fd_game = open(new_path.c_str(), O_WRONLY | O_APPEND);
        if (write(fd_game, last_line.c_str(), last_line.size()) == -1)
        {
            perror("Erro ao escrever no ficheiro de jogo");
            error = 1;
        }
        close(fd_game);
    }


    // Exibe informações do arquivo de jogo
    void display_info() const
    {
        cout << "Player ID: " << plid << "\n";
        cout << "Time Init: " << time_init << "\n";
    }
};

class game_player
{
public:
    string plid;        // Identificador único do jogador
    int time;           // maybe actual time will not be used
    int nT;             // Número de tentativas
    string codigo = ""; // acts to know if a timeout or finished msg has been sent
    bool ativo = false;
    int score;
    game_file *file;
    time_t tempo_inicio_jogo;


    game_player(const string &id)
        : plid(id), ativo(false)
    {
        create_game_dir(plid);
    }

    void start_game(int tempo, string &cores, game_file *gfile, time_t tempo_inicio)
    {
        time = tempo;
        tempo_inicio_jogo = tempo_inicio;
        cout<< "tempo inicio jogo: " << tempo_inicio_jogo << endl;
        ativo = true;
        nT = 1;
        codigo = cores;
        file = gfile;
    }

    time_t get_tempo_inicio_jogo() const {
        return tempo_inicio_jogo;
    }
    
    // verifica se o jogo já acabou 
    bool game_time_act(time_t now) {
        return (now - tempo_inicio_jogo) < time;
    }
    
    int findtry(string &code)
    {
        return 0;
    }

    // Reseta o estado do jogador
    void reset(const string &id)
    {
        nT = 0;
        plid = "none";
        codigo = "none";
    }

    // Incrementa o número de tentativas
    void next_try()
    {
        nT++;
    }

    // Verifica se o número de tentativas coincide com o do servidor
    bool same_try(int server_try) const
    {
        return nT == server_try;
    }

    // Atualiza o código do jogador
    void update_codigo(const string &new_code)
    {
        codigo = new_code;
    }

    void finish(const string &term, time_t time)
    {
        ativo = false;
        file->finish_game(time, term, to_string(NUM_TRIES - nT));
        nT = 0;
        codigo = "";
        time = 0;
        tempo_inicio_jogo = 0;
    }

    // Exibe informações do jogador
    void display_info() const
    {
        cout << "Player ID: " << plid << "\n";
        cout << "tempo " << time << "\n";
        cout << "tentativas " << nT << "\n";
        cout << "Code: " << codigo << "\n";
    }
};

game_player *find_player(const string &plid)
{
    auto it = find_if(players.begin(), players.end(), [&plid](const game_player &player)
                      { return player.plid == plid; });

    if (it != players.end())
    {
        return &(*it); // Retorna o ponteiro para o jogador encontrado
    }
    return nullptr; // Retorna nullptr se não encontrar o jogador
}

int init_game(const string &PLID, int time, time_t play_time)
{
    game_player *player = find_player(PLID);
    if (player && player->ativo) {
        cout << "Jogador com PLID " << PLID << " já está ativo." << endl;
        return 1;
    }
    else if (player && !player->ativo) {
        cout << "Entrou no caso START NEW GAME com PLID: " << PLID << " e tempo_max: " << time << endl;

        char colour_code[NUM_COLORS + 1]; // +1 para o terminador nulo
        generate_random_colors(colour_code);
        string colour_code_str(colour_code);
        cout << "tempo inicio jogo: " << play_time << endl;
        game_file *file = new game_file(PLID, "P", colour_code_str, to_string(time), play_time);
        player->start_game(time, colour_code_str, file, play_time);
        player->display_info();
        for (auto it = players.begin(); it != players.end(); ++it)
        {
            cout << it->plid << " " << endl;
        }
        return 0;
    }
    cout << "Entrou no caso START NEW GAME com PLID: " << PLID << " e tempo_max: " << time << endl;
    char colour_code[NUM_COLORS + 1]; // +1 para o terminador nulo

    generate_random_colors(colour_code);
    game_player new_player(PLID);
    
    cout << "tempo inicio jogo: " << play_time << endl;
    game_file *file = new game_file(PLID, "P", colour_code, to_string(time), play_time);
    string colour_code_str(colour_code);
    new_player.start_game(time, colour_code_str, file, play_time);
    players.emplace_back(new_player);
    new_player.display_info();
    for (auto it = players.begin(); it != players.end(); ++it)
    {
        cout << it->plid << " " << endl;
    }
    return 0;
}

int init_game_debug(const string &PLID, int time, string &code, time_t play_time)
{
    game_player *player = find_player(PLID);
    if (player && player->ativo)
    {
        cout << "Jogador com PLID " << PLID << " já está ativo." << endl;
        return 1;
    }
    else if (player && !player->ativo)
    {
        cout << "Entrou no caso START NEW GAME com PLID: " << PLID << " e tempo_max: " << time << endl;
        game_file *file = new game_file(PLID, "P", code, to_string(time), play_time);
        player->start_game(time, code, file, play_time);
        player->display_info();
        return 0;
    }
    cout << "Entrou no caso START NEW GAME com PLID: " << PLID << " e tempo_max: " << time << endl;
    game_player new_player(PLID);
    game_file *file = new game_file(PLID, "P", code, to_string(time), play_time);
    new_player.start_game(time, code, file, play_time);
    players.emplace_back(new_player);
    new_player.display_info();
    return 0;
}
/*
int get_thread(const string& PLID) {
    for (size_t i = 0; i < threads_ativas; i++) {
        if (players[i].plid == PLID) {
            return players[i].thread; // Retorna o identificador da thread
        }
    }
    return -1; // Retorna -1 se o jogador não for encontrado
}
*/
void process_player(game_player *player, string &code, int nT, string &send_buffer, time_t play_time)
{
    if (player) {
        if (player->ativo){
            if (player->game_time_act(play_time) == false) {
                cout << "Jogador com PLID " << player->plid << " não está ativo." << endl;
                cout << "Tempo esgotado para o jogador." << endl;
                send_buffer = "RTR ETM " + player->codigo + "\n";
                player->finish("T", play_time);
                return;
            }
            
            if (player->same_try(nT) || (player->same_try(nT - 1) && find_play(player->plid, code))) {
                cout << "Jogador com PLID " << player->plid << " está ativo." << endl;
                int nW = 0;
                int nB = 0;
                if (find_play(player->plid, code) && player->same_try(nT)) {
                    cout << "Tentiva duplicada." << endl;
                    send_buffer = "RTR DUP\n";
                    return;
                }
                match_code(player->codigo, code, nW, nB);
                cout << "nW: " << nW << " nB: " << nB << endl;
                if (nT == NUM_TRIES && nB != NUM_COLORS) {
                    cout << "Número de tentativas esgotado." << endl;
                    send_buffer = "RTR ENT " + player->codigo + "\n";
                    player->finish("F", play_time);
                    return;
                }
                else if (nB == NUM_COLORS) {
                    cout << "Jogador com PLID " << player->plid << " acertou no código." << endl;
                    player->next_try();
                    player->file->new_line(code, nB, nW, play_time);
                    send_buffer = "RTR OK " + to_string(player->nT) + " " + to_string(nB) + " " + to_string(nW) + " " + player->codigo + "\n";
                    player->finish("W", play_time);
                    return;
                }
                send_buffer = "RTR OK " + to_string(player->nT) + " " + to_string(nB) + " " + to_string(nW) + "\n";
                if (player->same_try(nT)) {
                    player->next_try();
                    player->file->new_line(code, nB, nW, play_time);
                }
                return;
            }
            else {
                cout << "Número de tentativas inválido" << endl;
                send_buffer = "RTR INV\n";
                return;
            }
        }
        else {
            printf("jogo nao encontrado");
            if (player->codigo.empty())
            {
                printf("codigo vazio");
                cout << "Jogador com PLID " << player->plid << " não está ativo." << endl;
                send_buffer = "RTR NOK\n";
                return;
            }
           
        }
    }
    else {
        cout << "Jogador não encontrado." << endl;
        send_buffer = "RTR NOK\n";
        return;
    }
}


string get_termination_type(const string &type) {
    if (type == "W") return "WIN";
    if (type == "F") return "FAIL";
    if (type == "T") return "TIMEOUT";
    if (type == "Q") return "QUIT";
    return "UNKNOWN";
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

        if (lines.empty()) return;

        // Extract initial game information
        stringstream ss(lines[0]);
        string score, plid, secret_code, notries, mode;
        ss >> score >> plid >> secret_code >> notries >> mode;

        // Extract game initiation time from filename
        size_t pos = scorefilename.find_last_of('/');
        string filename = (pos == string::npos) ? scorefilename : scorefilename.substr(pos + 1);
        string init_time_str = filename.substr(0, 15); // Extract YYYYMMDD_HHMMSS
        string termination_type = filename.substr(16, 1); // Extract termination type (W, T, etc.)

        if (code == "RST OK")
        {
            buffer += "     Active game found for player " + plid + "\n";
            buffer += "Game initiated: " + init_time_str + " with " + notries + " seconds to be completed\n\n";
            buffer += "     --- Transactions found: " + to_string(lines.size() - 2) + " ---\n\n";

            for (size_t i = 1; i < lines.size() - 1; ++i)
            {
                stringstream ss(lines[i]);
                string trial_code;
                int nb, nw, trial_time;
                ss >> trial_code >> nb >> nw >> trial_time;
                buffer += "Trial: " + trial_code + ", nB: " + to_string(nb) + ", nW: " + to_string(nw) + " at " + to_string(trial_time) + "s\n";
            }

            buffer += "\n  -- " + notries + " seconds remaining to be completed --\n";
        }
        else if (code == "RST FIN")
        {
            buffer += "Last finalized game for player " + plid + "\n";
            buffer += "Game initiated: " + init_time_str + " with " + notries + "s to be completed\n";
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
            string end_time_str, duration_str;
            ss_last >> end_time_str >> duration_str;
            buffer += "     Termination: " + get_termination_type(termination_type) + " at " + end_time_str + ", Duration: " + duration_str + "s\n";
        }
    }
}


int case_player(string &buffer, string &send_buffer, time_t play_time) {
    string PLID; // Para armazenar o PLID
    cout << "Buffer recebido: '" << buffer << "'" << endl;
    printf("Tamanho do buffer: %zu\n", buffer.size());
    // Caso SCORES
    if (buffer.compare("SSB\n") == 0) {
        struct dirent **filelist;
        int nentries;
        nentries = scandir("SERVER/SCORES/", &filelist, 0, alphasort);

        bool is_empty = true;
        if (nentries < 0) {
            perror("Erro ao ler o diretório de scores");
        } else {
            while (nentries--) {
                if (strcmp(filelist[nentries]->d_name, ".") != 0 && strcmp(filelist[nentries]->d_name, "..") != 0) {
                    is_empty = false;
                    break;
                }
                free(filelist[nentries]);
            }
            free(filelist);
        }
        if (is_empty) {
            send_buffer = "RSS EMPTY\n";
        } else {
            send_buffer = "RSS OK ";
            string filename;
            int file_size= 0;
            string file_buffer;
            format_scb(file_size, file_buffer);
            filename = "SCORES_" + get_str_time(play_time, 1) + ".txt";
            send_buffer += filename + " " + to_string(file_size) + "\n" + file_buffer;
            //create file with scores
        }
        // tambem nao funciona
    }
    // Caso GAMES
    else if (buffer.substr(0, 4).compare("STR ") == 0) {
        regex pattern("^STR (\\d{6})\n$");
        smatch matches;
        if (regex_match(buffer, matches, pattern)) {
            PLID = matches[1];
            game_player *player = find_player(PLID);
            string gamefilename;
            string filename = "STATE_" + PLID + ".txt";
            int file_size=0;
            string file_buffer;
            if (player) {
                char gamefilename_c[256];
                FindLastGame(PLID, gamefilename_c);
                gamefilename = string(gamefilename_c);
                if (player->ativo)
                {
                    if (player->game_time_act(play_time) == false) {
                        cout << "Tempo esgotado para o jogador." << endl;
                        send_buffer = "RTR FIN ";
                        player->finish("T", play_time);
                        string code = "RST FIN";
                        format_str(gamefilename, file_size, file_buffer, code);
                    } else{

                        cout << "Jogador com PLID " << PLID << " está ativo." << endl;
                        send_buffer = "RST OK ";
                        string code = "RST OK";
                        format_str(gamefilename, file_size, file_buffer, code);
                    }
                } else  {
                    cout << "Jogador com PLID " << PLID << " nao está ativo." << endl;
                    send_buffer = "RST FIN ";
                    string code = "RST FIN";
                    format_str(gamefilename, file_size, file_buffer, code);
                }
            } else {
                cout << "Jogador com PLID " << PLID << " não encontrado." << endl;
                send_buffer = "RST NOK\n";
            }
        }
        else {
            cout << "Sintaxe do PLID inválida." << endl;
            send_buffer = "RST NOK\n";
        }
    }
    // Caso QUIT da resert as infos sobre este player
    else if (buffer.substr(0, 4).compare("QUT ") == 0) {
        regex pattern("^QUT (\\d{6})\n$");
        smatch matches;
        if (regex_match(buffer, matches, pattern))
        {
            PLID = matches[1];
            game_player *player = find_player(PLID);
            if (player) {
                if (player->ativo) {
                    cout << "Jogador com PLID " << PLID << " está ativo." << endl;
                    send_buffer = "RQT OK " + player->codigo + "\n";
                    player->finish("Q", play_time);
                    // eventualmente podemos adicionar aqui algo para mandar esta info para o ficheiro games antes de ser apagado
                }
                else {
                    cout << "Jogador com PLID " << PLID << " não está ativo." << endl;
                    send_buffer = "RQT NOK\n";
                }
                cout << "Informações do jogador após reset:" << endl;
                player->display_info();
                // eventualmente podemos adicionar aqui algo para mandar esta info para o ficheiro games antes de ser apagado
            }
            else {
                cout << "Jogador com PLID " << PLID << " não encontrado." << endl;
                send_buffer = "RQT NOK\n";
            }
        }
        else {
            cout << "Sintaxe do PLID inválida." << endl;
            send_buffer = "RQT ERR\n";
        }
    }
    // Caso DEBUG
    else if (buffer.substr(0, 4).compare("DBG ") == 0) {
        regex pattern("^DBG (\\d{6}) (\\d{1,3}) (.*)\n$");
        smatch matches;
        int tempo_max;
        if (regex_match(buffer, matches, pattern)) {
            string PLID = matches[1];
            string time = matches[2];
            string code = matches[3];
            if (!valid_time(time) || !code_val(code)) {
                cout << "Tempo ou código inválido" << endl;
                send_buffer = "RDB ERR\n";
            }
            else 
                tempo_max = stoi(time);
            
            if (init_game_debug(PLID, tempo_max, code, play_time) == 1) 
                send_buffer = "RDB NOK\n";
            
            else
                send_buffer = "RDB OK\n";
            
        }
        else {
            cout << "Sintaxe Invalida" << endl;
            send_buffer = "RDB ERR\n";
        }
    }
    // Caso START NEW GAME
    else if (buffer.substr(0, 4).compare("SNG ") == 0) {
        regex pattern("^SNG (\\d{6}) (\\d{1,3})\n$");
        smatch matches;
        int tempo_max;
        if (regex_match(buffer, matches, pattern)) {
            printf("Entrou no caso START NEW GAME\n");
            PLID = matches[1];
            string tempo = matches[2];
            if (!valid_time(tempo)) {
                cout << "Tempo máximo inválido" << endl;
                send_buffer = "RSG ERR\n";
            }
            else  {
                tempo_max = stoi(tempo);
                if (init_game(PLID, tempo_max, play_time) == 1)         
                    send_buffer = "RSG NOK\n";
                else                
                    send_buffer = "RSG OK\n";
                
            }
        }
        else {
            cout << "PLID ou tempo inválido" << endl;
            send_buffer = "RSG ERR\n";
        }
    } // Caso TRY
    else if (buffer.substr(0, 4).compare("TRY ") == 0)  {
        regex pattern("^TRY (\\d{6}) ([RGBYOP]) ([RGBYOP]) ([RGBYOP]) ([RGBYOP]) (\\d{1})\\s\n?*$");
        smatch matches;
        if (regex_match(buffer, matches, pattern)) {
            PLID = matches[1];
            string code = matches[2].str() + matches[3].str() + matches[4].str() + matches[5].str() + '\0';
            cout << "Code: " << code << endl;
            string nT = matches[6];
            game_player *player = find_player(PLID);
            process_player(player, code, stoi(nT), send_buffer, play_time);
            printf("Entrou no caso TRY\n");
        }
        else {
            cout << "sintaxe invalida" << endl;
            send_buffer = "RTR ERR\n";
        }
    }
    else {
        cerr << "Mensagem inválida ou Player deu Quit" << endl;
        send_buffer = "ERR\n";
        return -1; // Indica erro
    }
    cout << "ta no final " << endl;
    cout << "Mensagem a enviar: '" << send_buffer << "'" << endl;
    return 0; // Sucesso
}

int validate_args(int argc, char *argv[], const char *&gs_port, bool &verbose) {
    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
            gs_port = argv[++i];
        
        else if (strcmp(argv[i], "-v") == 0)
            verbose = true;
        else {
            cerr << "Usage: " << argv[0] << " [-p GSport] [-v]" << endl;
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]){
    const char *gs_port = "58081"; // Default port number
    bool verbose = false;          // Default verbose mode
    if (validate_args(argc, argv, gs_port, verbose) != 0)
        return 1;
    
    create_directories();
    struct addrinfo *infoaddr = nullptr;

    // Inicializa o servidor UDP
    int fd_udp = init_socket_server(infoaddr, gs_port);
    if (fd_udp < 0)
        return 1;

    if (bind_socket_server(fd_udp, infoaddr) < 0) {
        freeaddrinfo(infoaddr);
        close(fd_udp);
        return 1;
    }

    int fd_tcp = init_tcp_server(gs_port);
    if (fd_tcp < 0) {
        close(fd_udp);
        return 1;
    }

    fd_set read_fds;
    vector<int> client_fds;
    int max_fd = max(fd_udp, fd_tcp);

    while (true) {

        char *buffer = (char *)malloc(BUFFER_SIZE);

        if (buffer == nullptr) {
            // Handle allocation failure
            cerr << "Memory allocation failed" << endl;
            close(fd_tcp);
            close(fd_udp);
            exit(1);
        }
        string buffer_r(BUFFER_SIZE_GS, '\0');
        FD_ZERO(&read_fds);
        FD_SET(fd_udp, &read_fds);
        FD_SET(fd_tcp, &read_fds);

        max_fd = fd_udp > fd_tcp ? fd_udp : fd_tcp;
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select");
            free(buffer);
            break;
        }
        if (FD_ISSET(fd_udp, &read_fds)) {
            string send_string(BUFFER_SIZE, '\0');
            struct sockaddr_in addr_udp;
            socklen_t addrlen_udp = sizeof(addr_udp);
            ssize_t n_udp = receive_message_server(fd_udp, buffer, BUFFER_SIZE, addr_udp, addrlen_udp);
            if (n_udp == -1) {
                perror("Erro ao receber mensagem UDP");
                send_string.clear();
                free(buffer);
                break;
            }
            time_t now = time(0);
            cout << "Mensagem UDP recebida: " << string(buffer, n_udp) << endl;
            buffer_r = string(buffer, n_udp);
            // Processa mensagem UDP
            if (case_player(buffer_r, send_string, now) != 0) {
                cerr << "Erro ao processar o buffer UDP!" << endl;
                send_string = "ERR\n";
            }
            // Envia resposta UDP
            const char *buffer_send = send_string.c_str();
            cout << "Mensagem a enviar(fora do case): " << buffer_send << endl;
            if (send_message_server(fd_udp, buffer_send, BUFFER_SIZE, addr_udp, addrlen_udp) < 0) {
                perror("Erro ao enviar mensagem UDP");
                free(buffer);
                send_string.clear();
                break;
            }
            send_string.clear();
            continue;
        }

        if (FD_ISSET(fd_tcp, &read_fds)) {
            string send_string(BUFFER_SIZE_GS, '\0');
            struct sockaddr_in addr_tcp;
            socklen_t addrlen_tcp = sizeof(addr_tcp);
            int client_fd = accept_connection_tcp_server(fd_tcp, &addr_tcp, &addrlen_tcp);
            if (client_fd >= 0) {
                printf("Conneccao aceite com fd: %d\n", client_fd);
                client_fds.push_back(client_fd);
                max_fd = max(max_fd, client_fd); // Update max_fd
            }
            ssize_t n_tcp = read_message_tcp_server(client_fd, buffer, BUFFER_SIZE);
            if (n_tcp == 0) { // Cliente desconectou
                cout << "Cliente TCP desconectou." << endl;
                close(client_fd);
            }
            else if (n_tcp == -1) {
                perror("Erro ao receber mensagem TCP");
                close(client_fd);
                send_string = "ERR\n";
            }
            else {
                time_t now = time(0);
                cout << "Mensagem TCP recebida: " << string(buffer, n_tcp) << endl;
                buffer_r = string(buffer, n_tcp);
                if (case_player(buffer_r, send_string, now) != 0) {
                    cerr << "Mensagem TCP invalida!" << endl;
                    send_message_tcp_server(client_fd, "ERR", 3);
                }
                const char *buffer_send = send_string.c_str();
                cout << "Mensagem a enviar: " << buffer_send << endl;
                if (send_message_tcp_server(client_fd, buffer_send, BUFFER_SIZE_GS) < 0) {
                    perror("Erro ao enviar mensagem UDP");
                    free(buffer);
                    send_string.clear();
                    break;
                }
                send_string.clear();
            }
            close(client_fd);
            send_string.clear();
        }
        buffer_r.clear();
        free(buffer);
    }
    close(fd_tcp);
    close(fd_udp);

    return 0;
}