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

// Class do jogador e funções 
class game_player{
public:
    std::string plid;
    int nT;
    bool active = false;

    game_player(const std::string &id) : plid(id), nT(1) {}

    void finish(){
        active = false;
    }

    void reset() {
        nT = 1;
        active = true;
    }

    void next_try(){
        nT++;
    }

    bool same_try(int server_try) const{
        return nT == server_try;
    }
};

//funcao para ler do terminal
int get_msg(string &msg) {
    char c;
    int i = 0;
    char msgbuffer[BUFFER_SIZE];
    while ((c = getchar()) != EOF && c != '\n' && i < BUFFER_SIZE - 1) {
        msgbuffer[i] = c;
        i++;
    }
    msgbuffer[i] = '\0';  
    msg = string(msgbuffer);
    return 0;
}

//Função para criar ficheiros
ofstream create_file(const string& directory, const string& filename) {
    string file_path = directory + "/" + filename;
    ofstream file(file_path);
    
    if (!file)
        cerr << "Erro ao criar o arquivo: " << file_path << endl;

    return file;
}

//Função salvar uma mensagem num ficheiro
int get_file_msg(string &msg, string &file_out) {
    istringstream stream(msg);
    string curr;
    string filename;
    string size;
    int count = 0;

    while (stream >> curr) {
        if (count == 2) 
            break;
        count++;
    }

    if (!curr.empty()) {
        try {
            filename = curr;
            stream >> size;
            int num_chars = stoi(size);
            size_t pos = msg.find(size);
            file_out = msg.substr(pos + size.length() + 1, num_chars);
        } 
        catch (const invalid_argument& e) {
            cerr << "Invalid arguments size " << size << endl;
            return -1;
        }
        catch (const out_of_range& e) {
            cerr << "size out of range: " << size << endl;
            return -1;
        }
    } else {
        cout << "Format not correct" << endl;
        return -1;
    }

    ofstream file = create_file(".", filename);
    if (!file) 
        return -1;

    file << file_out;
    file.close();
    return 0;
}

// Função que verifica se o tempo dado é valido
bool valid_time(const string& str) {
    try {
        int num = stoi(str);
        return num >= 0 && num <= 600;
    } catch (...) {
        return false;
    }
}

//Função que remove espaços
string rm_spaces(const string& str) {
    string trimmed = regex_replace(str, regex("^\\s+|\\s+$"), "");
    return regex_replace(trimmed, regex("\\s+"), " ");
}


// Função que verifica se o codigo dado é valido
int code_val(const string& code) {
    regex pattern("^([RGBYOP]) ([RGBYOP]) ([RGBYOP]) ([RGBYOP])");
    return regex_match(code, pattern);
}

// Função para processar o buffer recebido e converter num comando específico
int case_terminal(string &buffer){
    buffer= rm_spaces(buffer);
    //Caso scoreboard
    if ((buffer.compare("sb"))==0 || (buffer.compare("scoreboard"))==0) {
        buffer = "SSB";
        return 0;
    }
    //Caso debug
    else if (buffer.size() > 6 && buffer.substr(0,6).compare("debug ")==0){
        regex pattern("^debug (\\d{6}) (\\d{1,3}) (.*)$");
        smatch matches;
        if (regex_match(buffer, matches, pattern)) {
            string plid  = matches[1];
            string time = matches[2];
            string code = matches[3];
            if (valid_time(time) && code_val(code)) {
                buffer ="DBG "+ plid + " " + time + " " + code;
                return 1;
            }
        }
    } 
    //Caso start novo jogo
    else if (buffer.size() > 6 && buffer.substr(0,6).compare("start ")==0){
        regex pattern("^start (\\d{6}) (\\d{1,3})$");
        smatch matches;
        if (regex_match(buffer, matches, pattern)) {
            string plid  = matches[1];
            string time = matches[2];
            if (valid_time(time)) {
                buffer= "SNG "+ plid +" "+ time;
                return 2;
            } 
        }
    }
    //Caso show trials
    else if ((buffer.compare("st"))==0 || (buffer.compare("show_trials"))==0) {
        buffer = "STR PLID"; 
        return 3;
    }
    //Caso quit
    else if ((buffer.compare("quit"))==0 ) {
        buffer = "QUT PLID";
        return 4;
    }
    //Caso exit
    else if ((buffer.compare("exit"))==0 ) {
        buffer = "QUT PLID";
        return 5;
    }
    //Caso try
    else if (buffer.size() >= 11 && (buffer.substr(0,4).compare("try ")) ==0){
        if (code_val(buffer.substr(4,7)) == true){
            buffer =  "TRY PLID "+ buffer.substr(4,12) +" nT";
            return 6;
        }
    }

    printf("Erro: Mensagem introduzida nao esta de acordo com as normas\n");
    buffer = "";
    return -1;  
} 

// Função que verifica se há algum jogo ativo de momento
int check_active_game(string &sendmsg, game_player curr_game){
    if (curr_game.active == true) {
        printf("Erro: Existe outro jogo ativo neste momento\n");
        sendmsg = "";
        return 1;
    }
    return 0;
}

// Função que complementa as mensagens com informacões do jogador ativo
int add_args(string &msg, int code, game_player curr_game){
    if (code <= 2)
        return 0;
    else if (code == 6) {
        if (curr_game.active == false) {
            printf("Erro: não existe um jogo ativo de momento\n");
            msg = "";
            return -1;
        }
        size_t pos = msg.find("nT");
        if (pos != string::npos)
            msg.replace(pos, 2, std::to_string(curr_game.nT));
        
    }

    size_t pos = msg.find("PLID");
    if (pos != string::npos)
        msg.replace(pos, 4, curr_game.plid);
    
    return 0;
}


