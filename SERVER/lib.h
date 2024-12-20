#ifndef LIB_H
#define LIB_H
#include <string>
#include <vector>
#include <list>
#include <ctime>
#include <iostream>
#include <fstream>

using namespace std;

#define BUFFER_SIZE_GS 1024
#define BUFFER_SIZE 128
#define NUM_COLORS 4
#define NUM_TRIES 8


class game_file {
public:
    string plid;        // Identificador único do jogador.
    string path_file;   // Caminho do arquivo associado ao jogo do jogador.
    time_t time_init;   // Tempo de início do jogo usado como referência para calcular a duração das jogadas.
    string game_mode;   // Modo do jogo

    game_file(const string &id, const string &mode, const string &code, const string &timeout, time_t time_i);
    void new_line(const string &code, int nb, int nw, time_t play_time);
    string get_code_file();
    int get_nT_file();
    string get_str_time(time_t time, int mode);
    void finish_game(time_t finishtime, const string &term, const string &score);
   
};

class game_player {
public:
    string plid;              // Identificador único do jogador.
    int time;                 // Tempo total do jogo definido pelo jogador.
    int nT;                   // Número de tentativas feitas pelo jogador contadas pelo servidor
    string codigo = "";       // codigo do jogo atual do jogador 
    bool ativo = false;       // Indica se o jogador está ativo no jogo (true caso ativo)
    int score;                // Pontuação atual do jogador.  
    game_file *file;          // Ponteiro para o objeto game_file associado ao jogador (gerencia dados do jogo).  
    time_t tempo_inicio_jogo; // Timestamp do início do jogo para o jogador.

    game_player(const string &id);
    void start_game(int tempo, string &cores, game_file *gfile, time_t tempo_inicio);
    time_t get_tempo_inicio_jogo() const;
    bool game_time_act(time_t now);
    void reset(const string &id);
    void next_try();
    void add_spaces(string &str);
    bool same_try(int server_try) const;
    void update_codigo(const string &new_code);
    void finish(const string &term, time_t time);
    

};

extern vector<game_player> players;

ofstream create_file(const string& directory, const string& filename);
void create_directories();
void create_game_dir(const string &plid);
int code_val(string& code);
bool valid_time(const string& str);
void generate_random_colors(char *result);
int FindTopScores(list<string> *list);
int FindLastGame(string &PLID_str, char *fname);
int find_play(string &PLID, string &code);
void format_scb(string &buffer);
void match_code(const string &code1, const string &code2, int &nW, int &nB);
string get_str_time(time_t time, int mode);
void process_player(game_player *player, string &code, int nT, string &send_buffer, time_t play_time);
string get_termination_type(const string &type);
void format_str(string &scorefilename,  string &buffer, string &code);
int validate_args(int argc, char *argv[], char *&gs_port, bool &verbose);
game_player *find_player(const string &plid);


#endif 