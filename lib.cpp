
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
#include <unistd.h>
#include <regex>
#include <string>
#include <filesystem>


using namespace std;

#define PORT "58001"
#define BUFFER_SIZE 128
#define NUM_COLORS 4

void create_directories() {
    // Criando o diretório scoreboard
    if (mkdir("SCORES", 0777) == -1) {
        perror("Erro ao criar diretório SCORES");
    } 

    // Criando o diretório show_trials
    if (mkdir("GAMES", 0777) == -1) {
        perror("Erro ao criar diretório GAMES");
    }
}

void create_file(const string& directory, const string& filename) {
    string file_path = directory + "/" + filename;

    ofstream file(file_path);
    
    if (!file) {
        cerr << "Erro ao criar o arquivo: " << file_path << endl;
    }
}

int code_val(const std::string& code) {
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

string rm_spaces(const string& str) {
    string trimmed = regex_replace(str, regex("^\\s+|\\s+$"), "");
    return regex_replace(trimmed, regex("\\s+"), " ");
}


int case_terminal(string &buffer){
    buffer= rm_spaces(buffer);
    if ((buffer.compare("sb"))==0 || (buffer.compare("scoreboard"))==0) {
        buffer = "SSB";
        return 0;
    }
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
    else if ((buffer.compare("st"))==0 || (buffer.compare("show_trials"))==0) {
        buffer = "STR PLID"; 
        return 3;
    }
    else if ((buffer.compare("quit"))==0 ) {
        buffer = "QUT PLID";
        return 4;
    }
    else if ((buffer.compare("exit"))==0 ) {
        buffer = "QUT PLID";
        return 5;
    }
    else if (buffer.size() > 4 && (buffer.substr(0,4).compare("try ")) ==0){
        if (code_val(buffer.substr(4,12)) == true){
            buffer =  "TRY PLID "+ buffer.substr(4,12) +" nT";
            return 6;
        }
    }
    printf("Erro: Mensagem introduzida nao esta de acordo com as normas\n");
    buffer = "";
    return -1;  
} 


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


//////////////////////////////////////////---SERVER---///////////////////////////////////////////////////////////////////////////////////////////

// Função para inicializar o socket
int init_socket_server(struct addrinfo*& infoaddr) {
    int fd_udp = socket(AF_INET, SOCK_DGRAM, 0); 
    if (fd_udp == -1) {
        perror("Erro ao criar socket");
        return -1;
    }

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int errcode_udp = getaddrinfo(NULL, PORT, &hints, &infoaddr);
    if (errcode_udp != 0) {
        perror("Erro ao resolver endereço");
        close(fd_udp);
        return -1;
    }

    return fd_udp; 
}

// Função para vincular o socket ao endereço
int bind_socket_server(int fd_udp, struct addrinfo* infoaddr) {
    int n = bind(fd_udp, infoaddr->ai_addr, infoaddr->ai_addrlen);
    if (n == -1) {
        perror("Erro ao vincular o socket");
        return -1;
    }
    return 0; // Sucesso
}

// Função para receber mensagem
ssize_t receive_message_server(int fd_udp, char* buffer, size_t buffer_size, struct sockaddr_in& addr, socklen_t& addrlen_udp) {
    return recvfrom(fd_udp, buffer, buffer_size, 0, (struct sockaddr*)&addr, &addrlen_udp);
}

// Função para enviar mensagem
int send_message_server(int fd_udp, const char* buffer, size_t length, struct sockaddr_in& addr, socklen_t addrlen_udp) {
    ssize_t n = sendto(fd_udp, buffer, length, 0, (struct sockaddr*)&addr, addrlen_udp);
    if (n == -1) {
        perror("Erro ao enviar mensagem");
        return -1;
    }
    return 0; // Sucesso
}

//gera o codigo de coderes para o jogo
void generate_random_colors(char *result) {
    char colors[] = {'R', 'G', 'B', 'Y', 'O', 'P'};
    size_t num_available_colors = sizeof(colors) / sizeof(colors[0]);

    for (int i = 0; i < NUM_COLORS; i++) {
        int random_index = rand() % num_available_colors; // Escolhe um índice aleatório
        result[i] = colors[random_index];                // Adiciona a cor à sequência
    }
    result[NUM_COLORS] = '\0'; // Adiciona o terminador nulo para tornar a string válida
}



//////////////////////////////////////////---PLAYER---///////////////////////////////////////////////////////////////////////////////////////////



// Função para inicializar o socket
int init_socket_player(const char *hostname, struct addrinfo *&infoaddr){
    int fd_udp = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_udp < 0) {
        perror("Erro ao criar socket");
        return -1;
    }

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int errcode_udp = getaddrinfo(hostname, PORT, &hints, &infoaddr);
    if (errcode_udp != 0)   {
        perror("Erro ao resolver endereço");
        close(fd_udp);
        return -1;
    }

    return fd_udp; 
}

// Função para enviar mensagem???????????????????????????????
int send_socket_udp_player(int fd_udp, const char *message, struct addrinfo *infoaddr)
{
    ssize_t n = sendto(fd_udp, message, strlen(message), 0, infoaddr->ai_addr, infoaddr->ai_addrlen);
    if (n == -1)    {
        perror("Erro ao enviar mensagem");
        return -1;
    }
    return 0; 
}

// Função para receber mensagem
int receive_socket_udp_player(int fd_udp, char *buffer, size_t buffer_size)
{
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    ssize_t n = recvfrom(fd_udp, buffer, buffer_size, 0, (struct sockaddr *)&addr, &addrlen);
    if (n == -1)    {
        perror("Erro ao receber mensagem");
        return -1;
    }

    buffer[n] = '\0'; // Certifica-se de que o buffer termina com '\0' (para strings)
    return n;         // Retorna o número de bytes recebidos
}