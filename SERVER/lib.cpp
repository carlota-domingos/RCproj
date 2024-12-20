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
//-------------------------------------------------------------CLASSE GAME_FILE------------------------------------------------------

//Função construtora da classe game_file
// Inicializa um arquivo de jogo com os detalhes fornecidos e grava a linha inicial no arquivo.
game_file::game_file(const string &id, const string &mode, const string &code, const string &timeout, time_t time_i){
    plid = id;
    if (mode == "P")
        game_mode = "PLAY";
    else
        game_mode = "DEBUG";
    string path = "SERVER/GAME_" + plid + ".txt";
    path_file = path;
    time_init = time_i;
    int fd_game = open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd_game == -1) {
        perror("Erro ao abrir o ficheiro de jogo");
        
    }
    string firstline = plid + " " + mode + " " + code + " " + timeout + " " + get_str_time(time_init, 0) + " " + to_string(time_init) + "\n";
    if (write(fd_game, firstline.c_str(), firstline.size()) == -1) {
        perror("Erro ao escrever no ficheiro de jogo");
        
    }
    close(fd_game);
}

// Função que grava uma nova linha no arquivo de jogo com informações sobre o código e jogadas
void game_file::new_line(const string &code, int nb, int nw, time_t play_time) {
    time_t game_time = play_time - time_init;
    string time_str = to_string(game_time);
    string line = "T: " + code.substr(0, 4) + " " + to_string(nb) + " " + to_string(nw) + " " + time_str + "\n";
    int fd_game = open(path_file.c_str(), O_WRONLY | O_APPEND);
    if (write(fd_game, line.c_str(), line.size()) == -1)
    {
        perror("Erro ao escrever no ficheiro de jogo");
        
    }
    close(fd_game);
}

// Função que lê o arquivo de jogo e retorna o código inicial registrado
string game_file::get_code_file() {
    int fd_game = open(path_file.c_str(), O_RDONLY);
    if (fd_game == -1) {
        perror("Erro ao abrir o ficheiro de jogo");
        
    }
    string code;
    char buffer[50];
    if (read(fd_game, buffer, 50) == -1) {
        perror("Erro ao ler o ficheiro de jogo");
        
    }
    int i = 0;
    int count = 0;
    while (count < 2) {
        if (buffer[i] == ' ')
            count++;
        i++;
    }
    while (buffer[i] != ' ') {
        code += buffer[i];
        i++;
    }
    close(fd_game);
    return code;
}

// Função que retorna o número de tentativas registradas no arquivo de jogo
int game_file::get_nT_file() {
    int fd_game = open(path_file.c_str(), O_RDONLY);
    if (fd_game == -1) {
        perror("Erro ao abrir o ficheiro de jogo");
        
    }
    int nT = 0;
    char buffer[50];
    while (read(fd_game, buffer, 50) != 0)
        nT++;
    close(fd_game);
    return nT - 1;
}

// Função que formata o tempo como string, dependendo do modo especificado
string game_file::get_str_time(time_t time, int mode) {
    char buffer[20];
    struct tm *timeinfo;
    if (mode == 0) {
        timeinfo = localtime(&time);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    }
    else {
        timeinfo = localtime(&time);
        strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", timeinfo);
    }
    return string(buffer);
}

// Função que finaliza o jogo, renomeia o arquivo e registaa o resultado no sistema
void game_file::finish_game(time_t finishtime, const string &term, const string &score) {
    time_t game_time = finishtime - time_init;
    string game_time_str = to_string(game_time);
    string last_line = get_str_time(finishtime, 0) + " " + game_time_str + "\n";
    string new_path = "SERVER/GAMES/" + plid + "/" + get_str_time(finishtime, 1) + "_" + term + ".txt";
    create_game_dir(plid);
    if (rename(path_file.c_str(), new_path.c_str()) == -1) {
        perror("Erro ao renomear o ficheiro de jogo");
        
    }
    path_file = new_path;
    int fd_game = open(path_file.c_str(), O_WRONLY | O_APPEND);
    if (write(fd_game, last_line.c_str(), last_line.size()) == -1) {
        perror("Erro ao escrever no ficheiro de jogo");
        
    }
    if (term == "W") {
        string score_fn = score + "_" + plid + "_" + get_str_time(finishtime, 1) + ".txt";
        string path = "SERVER/SCORES/" + score_fn;
        struct stat st;
        if (stat("SERVER/SCORES", &st) != 0) {
            perror("Directory SCORES does not exist");
            
            return;
        }

        int score_fd = open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (score_fd == -1) {
            perror("Error opening score file");
            
            return;
        }

        string score_line = score + " " + plid + " " + get_code_file() + " " + to_string(get_nT_file()) + " " + game_mode + "\n";
        if (write(score_fd, score_line.c_str(), score_line.size()) == -1) {
            perror("Error writing to score file");
            
            close(score_fd);
            return;
        }
        close(score_fd);
    }
}


//---------------------------------------------------------------CLASSE GAME_PLAYER-------------------------------------------------------

// Construtor da classe game_player
game_player::game_player(const string &id) : plid(id), ativo(false){}

// Função que inicia o jogo com informações fornecidas pelo jogador
void game_player::start_game(int tempo, string &cores, game_file *gfile, time_t tempo_inicio) {
    time = tempo;
    tempo_inicio_jogo = tempo_inicio;
    ativo = true;
    nT = 1;
    codigo = cores;
    file = gfile;
}

// Função que retorna o tempo de início do jogo
time_t game_player::get_tempo_inicio_jogo() const {
    return tempo_inicio_jogo;
}

// Função que verifica se o tempo de jogo ainda está dentro do limite
bool game_player::game_time_act(time_t now) {
    return (now - tempo_inicio_jogo) < time;
}

// Função que reseta as informações do jogo
void game_player::reset(const string &id) {
    nT = 1;
    codigo = "";
}

// Função que incrementa o número de tentativas do jogador
void game_player::next_try() {
    nT++;
}

// Função que adiciona espaços ao código de cores
void game_player::add_spaces(string &str) {
    for (int i = 0; i<3 ; i++) {
        str= str + codigo[i]+ " ";
    }
    str = str + codigo[3];
}

// Função que verifica se o servidor e o jogador estão na mesma tentativa
bool game_player::same_try(int server_try) const{
    return nT == server_try;
}

// Função que atualiza o código do jogo
void game_player::update_codigo(const string &new_code){
    codigo = new_code;
}

// Função que termina o jogo do jogador e grava os dados no arquivo
void game_player::finish(const string &term, time_t time) {
    ativo = false;
    file->finish_game(time, term, to_string((NUM_TRIES - nT+1)*10));
    nT = 0;
    codigo = "";
    time = 0;
    tempo_inicio_jogo = 0;
}


//-------------------------------------------------------------------OUTRAS----------------------------------------------------------------

// Função que cria ficheiros
ofstream create_file(const string &directory, const string &filename){
    string file_path = directory + "/" + filename;
    ofstream file(file_path);
    if (!file)
        cerr << "Erro ao criar o arquivo: " << file_path << endl;    
    return file;
}

// Função que retorna por extenso o código de terminação
string get_termination_type(const string &type){
    if (type == "W")
        return "Win";
    if (type == "F")
        return "Fail";
    if (type == "T")
        return "Timeout";
    if (type == "Q")
        return "Quit";
    return "UNKNOWN";
}

// Função que cria diretorias
void create_directories(){

    if (mkdir("SERVER/SCORES", 0777) == -1) {
        perror("Erro ao criar diretório SCORES");
    }
    // Criar o diretório show_trials
    if (mkdir("SERVER/GAMES", 0777) == -1) {
        perror("Erro ao criar diretório GAMES");
    }
}

// Função que cria a diretoria Games do player
void create_game_dir(const string &plid){
    string path = "SERVER/GAMES/" + plid;
    mkdir(path.c_str(), 0777);
}

// Função que verifica se o código recebido é valido
int code_val(string &code) {
    regex pattern("^([RGBYOP]) ([RGBYOP]) ([RGBYOP]) ([RGBYOP])");
    if (regex_match(code, pattern)){
        code.erase(remove(code.begin(), code.end(), ' '), code.end());
    }
    code=code.substr(0, 4) + '\0';
    return regex_match(code, pattern);
}

// Função que verifica se o tempo recebido é valido
bool valid_time(const string &str){
    try {
        int num = stoi(str);
        return num >= 0 && num <= 600;
    }
    catch (...) {
        return false;
    }
}

// Função que gera um código de cores aleatório
void generate_random_colors(char *result) {
    char colors[] = {'R', 'G', 'B', 'Y', 'O', 'P'};
    size_t num_available_colors = sizeof(colors) / sizeof(colors[0]);

    random_device rd;                                             
    mt19937 gen(rd());                                            
    uniform_int_distribution<> dist(0, num_available_colors - 1); 

    for (int i = 0; i < NUM_COLORS; i++)  {
        int random_index = dist(gen); 
        result[i] = colors[random_index];
    }
    result[NUM_COLORS] = '\0'; 
}

// Função que encontra os melhores scores 
int FindTopScores(list<string> *list) {
    struct dirent **filelist;
    int nentries, ifile;
    char fname[512]; 
    FILE *fp;
    nentries = scandir("SERVER/SCORES/", &filelist, 0, alphasort);
    ifile = 0;
    if (nentries < 0) {
        perror("Erro ao ler o diretório de scores");
        return 0;
    }
    else {
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
            if (ifile == 10)
                break;
        }
        free(filelist);
    } 
    return ifile;
}

// Função que encontra o último jogo por meio do arquivo
int FindLastGame(string &PLID_str, char *fname){
    const char *PLID = PLID_str.c_str();
    struct dirent **filelist;
    int nentries, found;
    char filename[30];
    char dirname[50];

    sprintf(filename, "GAME_%s.txt", PLID);
    nentries = scandir("SERVER", &filelist, 0, alphasort);
    found = 0;

    if (nentries > 0) {
        while (nentries--) {
            if (strcmp(filelist[nentries]->d_name, filename) == 0)
            {
                sprintf(fname, "SERVER/%s", filelist[nentries]->d_name);
                found = 1;
            }
            free(filelist[nentries]);
            if (found)
                break;
        }
        free(filelist);
    } 
    if (!found) {
        sprintf(dirname, "SERVER/GAMES/%s/", PLID);
        nentries = scandir(dirname, &filelist, 0, alphasort);
        found = 0;
        if (nentries <= 0)
            return (0);
        else {
            while (nentries--) {
                if (filelist[nentries]->d_name[0] != '.') {
                    sprintf(fname, "SERVER/GAMES/%s/%s", PLID, filelist[nentries]->d_name);
                    found = 1;
                }
                free(filelist[nentries]);
                if (found)
                    break;
            }
            free(filelist);
        }
    }
    return (found);
}

// Função que encontra uma jogada (para verificação de duplicados)
int find_play(string &PLID, string &code) {
    char *file_name = (char *)malloc(50);
    if (FindLastGame(PLID, file_name) == 0) {
        free(file_name);
        return 0;
    }
    FILE *fp = fopen(file_name, "r");
    if (fp == NULL) {
        free(file_name);
        return 0;
    }
    char code_buffer[50];
    int nB, nW, time;
    char buffer[256];
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        fclose(fp);
        free(file_name);
        return 0;
    }
    while (fscanf(fp, "T: %s %d %d %d\n", code_buffer, &nB, &nW, &time) != EOF) {
        string code_str(code_buffer);
        if (code_str.substr(0, 4).compare(code.substr(0, 4)) == 0) {
            fclose(fp);
            free(file_name);
            return 1;
        }
    }
    fclose(fp);
    free(file_name);
    return 0;
}


// Função que formata a score board
void format_scb(string &buffer)
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
}


// Função que compara códigos e retorna o número de pretos (nB) e brancos (nW)
void match_code(const string &code1, const string &code2, int &nW, int &nB)
{
    nW = 0;
    nB = 0;
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

// Função que formata o tempo para arquivos
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

// Função que processa a jogada do jogador
void process_player(game_player *player, string &code, int nT, string &send_buffer, time_t play_time)
{
    
    if (player == nullptr) {
        cerr << "Erro: player é nullptr!" << endl;
        return;
    }

    if (player->plid.empty()) {
        cerr << "Erro: plid está vazio!" << endl;
        return;
    }
    // Se o jogador não está ativo, então não pode jogar
    else if (player->ativo) {
        // Verifica o timer do jogo
        if (player->game_time_act(play_time) == false)
        {
            string code_final;
            player-> add_spaces(code_final);
            send_buffer = "RTR ETM " + code_final + "\n";
            player->finish("T", play_time);
            return;
        }
        // verifica se o numero da jogada é valida 
        if (player->same_try(nT) || (player->same_try(nT - 1) && find_play(player->plid, code)))            {
            int nW = 0;
            int nB = 0;
            //caso duplicado
            if (find_play(player->plid, code) && player->same_try(nT))
            {
                send_buffer = "RTR DUP\n";
                return;
            }
            match_code(player->codigo, code, nW, nB);
            // caso fique sem tentativas
            if (nT == NUM_TRIES && nB != NUM_COLORS)  {
                string code_final;
                player-> add_spaces(code_final);
                send_buffer = "RTR ENT " + code_final + "\n";
                player->finish("F", play_time);
                return;
            }
            // caso ganhe o jogo
            else if (nB == NUM_COLORS)
            {
                player->file->new_line(code, nB, nW, play_time);              
                send_buffer = "RTR OK " + to_string(player->nT) + " " + to_string(nB) + " " + to_string(nW)+ "\n";
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
        //numero invalido de tentativas
        else
        {
            send_buffer = "RTR INV\n";
            return;
        }
    }
    //jogador existe no arquivo mas nao tem um jogo ativo
    else {
        if (player->codigo.empty()) {
            send_buffer = "RTR NOK\n";
            return;
        }
    }
}

// Função que formata o arquivo de jogo para ser enviado ao jogador
void format_str(string &scorefilename, string &buffer, string &code){
    buffer = "";
    ifstream file(scorefilename);
    if (file.is_open())  {
        string line;
        vector<string> lines;
        while (getline(file, line))
        {
            lines.push_back(line);
        }
        file.close();

        if (lines.empty())
            return;

        stringstream ss(lines[0]);
        string plid, mode, secret_code, timeout, init_date, init_time, init_epoch_str;
        ss >> plid >> mode >> secret_code >> timeout >> init_date >> init_time >> init_epoch_str;
        if (mode == "P")
            mode = "PLAY";
        else
            mode = "DEBUG";
        if (code == "RST ACT")
        {
            buffer += "     Active game found for player " + plid + "\n";
            buffer += "     --- Transactions found: " + to_string(lines.size() - 1) + " ---\n\n";

            for (size_t i = 1; i < lines.size(); ++i)
            {
                stringstream ss(lines[i]);
                string trial_code, trial;
                int nb, nw, trial_time;
                ss >> trial >> trial_code >> nb >> nw >> trial_time;
                buffer += "Trial: " + trial_code + ", nB: " + to_string(nb) + ", nW: " + to_string(nw) + " at " + to_string(trial_time) + "s\n";
            }
            int timeleft = stoi(timeout) - (time(nullptr) - stoi(init_epoch_str));
            buffer += "\n  -- " + to_string(timeleft) + " seconds remaining to be completed --\n";
        }
        else if (code == "RST FIN")
        {
            buffer += "Last finalized game for player " + plid + "\n";
            buffer += "Game initiated: " + init_time + " with " + timeout + "s to be completed\n";
            buffer += "Mode: " + mode + "  Secret code: " + secret_code + "\n\n";
            buffer += "     --- Transactions found: " + to_string(lines.size() - 2) + " ---\n";

            string termination_type;
            stringstream ss_filename(scorefilename);
            string part;
            while (ss_filename >> part) {
                if (part.find(".txt") != string::npos) {
                    termination_type = part[part.size() - 5]; // Extrai a terminacao (W, F, Q, T)
                }
            }
            
            for (size_t i = 1; i < lines.size()-1; ++i)
            {
                stringstream ss(lines[i]);
                string trial_code, trial;
                int nb, nw, trial_time;
                ss >> trial >> trial_code >> nb >> nw >> trial_time;
                buffer += "Trial: " + trial_code + ", nB: " + to_string(nb) + ", nW: " + to_string(nw) + " at " + to_string(trial_time) + "s\n";
            }

            // Extrai a a data e hora de terminacao e a duraçãoo do jogo
            stringstream ss_end(lines.back());
            string end_date, end_time, duration_str;
            ss_end >> end_date >> end_time >> duration_str;

            buffer += "     Termination: " + get_termination_type(termination_type) + " at " + end_date + " " + end_time + ", Duration: " + duration_str + "s\n";

            // formata o nome do ficheiro para o novo nome
            string new_filename = "SERVER/GAMES/"+ plid +"/" + end_date + " " + end_time + " " + termination_type + ".txt";
            ofstream new_file(new_filename);
            if (new_file.is_open())
            {
                new_file << buffer;
                new_file.close();
            }
        }
    }
}


// Função que valida os argumentos recebidos
int validate_args(int argc, char *argv[], char *&gs_port, bool &verbose){
    
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

// Função que encontra um jogador na base de dados
game_player *find_player(const string &plid) {
    if (players.empty()) {
        return nullptr;
    }

    auto it = find_if(players.begin(), players.end(), [&plid](const game_player &player) {
        return player.plid == plid;
    });

    if (it != players.end())
        return &(*it); 

    return nullptr; 
}