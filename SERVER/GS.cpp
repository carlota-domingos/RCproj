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

using namespace std;


int init_game(string &PLID, int time, time_t play_time){
    game_player *player = find_player(PLID);
    PLID = PLID.substr(0, 6);
    if (player && player->ativo) {
        //cout << "Jogador com PLID " << PLID << " já está ativo." << endl;
        return 1;
    }
    else if (player && !player->ativo) {
        //cout << "Entrou no caso START NEW GAME com PLID: " << PLID << " e tempo_max: " << time << endl;

        char colour_code[NUM_COLORS + 1]; 
        generate_random_colors(colour_code);
        string colour_code_str(colour_code);
        game_file *file = new game_file(PLID, "P", colour_code_str, to_string(time), play_time);
        player->start_game(time, colour_code_str, file, play_time);
        player->display_info();
        return 0;
    }
    // cout << "Entrou no caso START NEW GAME com PLID: " << PLID << " e tempo_max: " << time << endl;
    char colour_code[NUM_COLORS + 1]; 

    generate_random_colors(colour_code);
    string colour_code_str(colour_code);
    game_player new_player(PLID);
    game_file *file = new game_file(PLID, "P", colour_code, to_string(time), play_time);
    //cout << "tempo inicio jogo: " << play_time << endl;
    new_player.start_game(time, colour_code_str, file, play_time);
    players.emplace_back(new_player);
    //new_player.display_info();
    return 0;
}

int init_game_debug(string &PLID, int time, string &code, time_t play_time){
    game_player *player = find_player(PLID);
    PLID = PLID.substr(0, 6);
    if (player && player->ativo) {
        //cout << "Jogador com PLID " << PLID << " já está ativo." << endl;
        return 1;
    }
    else if (player && !player->ativo)  {
        //cout << "Entrou no caso START NEW GAME com PLID: " << PLID << " e tempo_max: " << time << endl;
        game_file *file = new game_file(PLID, "D", code, to_string(time), play_time);
        player->start_game(time, code, file, play_time);
        //player->display_info();
        return 0;
    }
    //cout << "Entrou no caso START NEW GAME com PLID: " << PLID << " e tempo_max: " << time << endl;
    game_player new_player(PLID);
    game_file *file = new game_file(PLID, "D", code, to_string(time), play_time);
    new_player.start_game(time, code, file, play_time);
    players.emplace_back(new_player);
    //new_player.display_info();
    return 0;
}

int case_player(string &buffer, string &send_buffer, time_t play_time, string &args_verbose) {
    string PLID; // Para armazenar o PLID
    string rqstype;
    //cout << "Buffer recebido: '" << buffer << "'" << endl;
    //// printf("Tamanho do buffer: %zu\n", buffer.size());
    // Caso SCORES
    if (buffer.compare("SSB\n") == 0) {
        struct dirent **filelist;
        int nentries;
        nentries = scandir("SCORES/", &filelist, 0, alphasort);

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
            send_buffer = "RSS OK";
            string filename;
            string file_buffer;
            format_scb(file_buffer);
            int file_size = file_buffer.size();
            filename = "SCORES_" + get_str_time(play_time, 1) + ".txt";
            send_buffer += filename + " " + to_string(file_size) + " " + file_buffer + "\n";
            //create file with scores
        }
        rqstype = "Scoreboard";
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
                // cout << "ficheiro do jogo: " << gamefilename << endl;
                if (player->ativo)
                {
                    if (player->game_time_act(play_time) == false) {
                        // cout << "Tempo esgotado para o jogador." << endl;
                        send_buffer = "RST FIN ";
                        player->finish("T", play_time);
                        FindLastGame(PLID, gamefilename_c);
                        gamefilename = string(gamefilename_c);
                        // cout << "ficheiro do jogo: " << gamefilename << endl;
                        string code = "RST FIN";
                        format_str(gamefilename,  file_buffer, code);
                        file_size = file_buffer.size();
                        send_buffer += filename + " " + to_string(file_size) + " " + file_buffer + "\n";
                    } else{

                        //cout << "Jogador com PLID " << PLID << " está ativo." << endl;
                        send_buffer = "RST ACT ";
                        string code = "RST ACT";
                        format_str(gamefilename,  file_buffer, code);
                        file_size = file_buffer.size();
                        send_buffer += filename + " " + to_string(file_size) + " " + file_buffer + "\n";
                    }
                } else  {
                    // cout << "Jogador com PLID " << PLID << " nao está ativo." << endl;
                    send_buffer = "RST FIN ";
                    string code = "RST FIN";
                    format_str(gamefilename, file_buffer, code);
                    file_size = file_buffer.size();
                    send_buffer += filename + " " + to_string(file_size) + " " + file_buffer +"\n";
                }
            } else {
                // cout << "Jogador com PLID " << PLID << " não encontrado." << endl;
                send_buffer = "RST NOK\n";
            }
        }
        else {
            // cout << "Sintaxe do PLID inválida." << endl;
            send_buffer = "RST NOK\n";
        }
        rqstype = "Show Trials";
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
                    //cout << "Jogador com PLID " << PLID << " está ativo." << endl;
                    send_buffer = "RQT OK " + player->codigo + "\n";
                    player->finish("Q", play_time);
                    player->reset(PLID);
                }
                else {
                    // cout << "Jogador com PLID " << PLID << " não está ativo." << endl;
                    send_buffer = "RQT NOK\n";
                }
                //cout << "Informações do jogador após reset:" << endl;
                //player->display_info();
                
            }
            else {
                // cout << "Jogador com PLID " << PLID << " não encontrado." << endl;
                send_buffer = "RQT NOK\n";
            }
        }
        else {
            // cout << "Sintaxe do PLID inválida." << endl;
            send_buffer = "RQT ERR\n";
        }
        rqstype = "Quit Game";
    }
    // Caso DEBUG
    else if (buffer.substr(0, 4).compare("DBG ") == 0) {
        regex pattern("^DBG (\\d{6}) (\\d{1,3}) (.*)\n$");
        smatch matches;
        int tempo_max;
        if (regex_match(buffer, matches, pattern)) {
            PLID = matches[1];
            string time = matches[2];
            string code = matches[3];
            if (!valid_time(time) || !code_val(code)) {
                // cout << "Tempo ou código inválido" << endl;
                send_buffer = "RDB ERR\n";
            }
            else 
                tempo_max = stoi(time);
            string code_n = code.substr(0, 4);
            if (init_game_debug(PLID, tempo_max, code_n, play_time) == 1) 
                send_buffer = "RDB NOK\n";
            else
                send_buffer = "RDB OK\n";
            
        }
        else {
            // cout << "Sintaxe Invalida" << endl;
            send_buffer = "RDB ERR\n";
        }
        rqstype = "Start new Game Debug";
    }
    // Caso START NEW GAME
    else if (buffer.substr(0, 4).compare("SNG ") == 0) {
        regex pattern("^SNG (\\d{6}) (\\d{1,3})\n$");
        smatch matches;
        int tempo_max;
        if (regex_match(buffer, matches, pattern)) {
            // printf("Entrou no caso START NEW GAME\n");
            PLID = matches[1];
            string tempo = matches[2];
            if (!valid_time(tempo)) {
                // cout << "Tempo máximo inválido" << endl;
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
            // cout << "PLID ou tempo inválido" << endl;
            send_buffer = "RSG ERR\n";
        }
        rqstype = "Start new Game";
    } // Caso TRY
    else if (buffer.substr(0, 4).compare("TRY ") == 0)  {
        regex pattern("^TRY (\\d{6}) ([RGBYOP]) ([RGBYOP]) ([RGBYOP]) ([RGBYOP]) (\\d{1})\\s\n?*$");
        smatch matches;
        if (regex_match(buffer, matches, pattern)) {
            PLID = matches[1];
            string code = matches[2].str() + matches[3].str() + matches[4].str() + matches[5].str() + '\0';
            //cout << "Code: " << code << endl;
            string nT = matches[6];
            game_player *player = find_player(PLID);
            process_player(player, code, stoi(nT), send_buffer, play_time);
            //// printf("Entrou no caso TRY\n");
        }
        else {
            // cout << "sintaxe invalida" << endl;
            send_buffer = "RTR ERR\n";
        }
        rqstype = "Try";
    }
    else {
        cerr << "Mensagem inválida ou Player deu Quit" << endl;
        send_buffer = "ERR\n";
        return -1; 
    }
    //cout << "ta no final " << endl;
    //cout << "Mensagem a enviar: '" << send_buffer << "'" << endl;
    if(rqstype != "Scoreboard")
        args_verbose = "[Player ID: "+ PLID + "] [Type of Request: " + rqstype + "]";
    else 
        args_verbose = "[Type of Request: " + rqstype + "]";
    return 0; 
}

int main(int argc, char *argv[]) {
    char *gs_port = (char *)malloc(6 * sizeof(char)); // Allocate memory for the port
    if (!gs_port) {
        cerr << "Memory allocation failed" << endl;
        return 1;
    }
    strcpy(gs_port, "58081"); // Copy the default port value

    bool verbose = false;          
    if (validate_args(argc, argv, gs_port, verbose) != 0) {
        free(gs_port); // Free allocated memory before returning
        return 1;
    }
    create_directories();

    // Inicializa o servidor UDP
    int fd_udp = init_socket_server(gs_port);
    if (fd_udp < 0)
        return 1;
    else 
        cout << "Servidor UDP inicializado na porta " << gs_port << endl;

    int fd_tcp = init_tcp_server(gs_port);
    if (fd_tcp < 0) {
        close(fd_udp);
        return 1;
    }
    else 
        cout << "Servidor TCP inicializado na porta " << gs_port << endl;

       

    int max_fd;
    fd_set activefds;

    while (true) {
        char *buffer = (char *)malloc(BUFFER_SIZE);
        if (buffer == nullptr) {
            cerr << "Memory allocation failed" << endl;
            close(fd_tcp);
            close(fd_udp);
            exit(1);
        }

        string buffer_r(BUFFER_SIZE_GS, '\0');

        FD_ZERO(&activefds);
        FD_SET(fd_udp, &activefds);
        FD_SET(fd_tcp, &activefds);

        fd_set readfds = activefds;

        max_fd = fd_udp > fd_tcp ? fd_udp : fd_tcp;
        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select");
            free(buffer);
            break;
        }
        string args_verbose(BUFFER_SIZE_GS, '\0');
        string verbose_str(BUFFER_SIZE_GS, '\0');
        if (FD_ISSET(fd_udp, &readfds)) {
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
            cout << "Mensagem recebida: " << buffer << endl;

            time_t now = time(0);
            buffer_r = string(buffer, n_udp);
            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(addr_udp.sin_addr), ip_str, INET_ADDRSTRLEN);
            
            if (case_player(buffer_r, send_string, now,args_verbose) != 0) {
                cerr << "Erro ao processar o buffer UDP!" << endl;
                send_string = "ERR\n";
            } else if (verbose) {
                verbose_str = "[VERBOSE] " + args_verbose;
                string ip_addr = string(ip_str);
                if (ip_addr == "127.0.0.1")
                    verbose_str += " [Ip Address: localhost]";
                else
                    verbose_str += " [Ip Address: " + ip_addr+ "]\n";
                cout << verbose_str << endl;
            }
            args_verbose.clear();
            verbose_str.clear();
            
            
            const char *buffer_send = send_string.c_str();
            if (send_message_server(fd_udp, buffer_send, BUFFER_SIZE, addr_udp, addrlen_udp) < 0) {
                perror("Erro ao enviar mensagem UDP");
                free(buffer);
                send_string.clear();
                break;
            }
            printf("Mensagem enviada: %s\n", buffer_send);
            send_string.clear();
            continue;
        }

        if (FD_ISSET(fd_tcp, &readfds)) {
            struct sockaddr_in addr_tcp;
            socklen_t addrlen_tcp = sizeof(addr_tcp);
            int client_fd = accept_connection_tcp_server(fd_tcp, &addr_tcp, &addrlen_tcp);
            if (client_fd >= 0) {
            //     pid_t pid = fork();
                
                // if (pid < 0) { 
                    // perror("Erro ao criar processo filho");
                    // close(client_fd);
                    // continue;
                    
                // } else if (pid == 0) { // Processo filho
                    //close(fd_tcp); 
                    char *buffer = (char *)malloc(BUFFER_SIZE);
                    if (!buffer) {
                        cerr << "Falha na alocação de memória no processo filho" << endl;
                        close(client_fd);
                        exit(1);
                    }
                    ssize_t n_tcp = read_message_tcp_server(client_fd, buffer, BUFFER_SIZE);
                    if (n_tcp <= 0) {
                        if (n_tcp == 0) {
                            cout << "Cliente TCP desconectou." << endl;
                        } else {
                            perror("Erro ao receber mensagem TCP");
                        }
                        free(buffer);
                        close(client_fd);
                        exit(1);
                    }
                    cout << "Mensagem recebida: " << buffer << endl;
                    string buffer_r(buffer, n_tcp);
                    string send_string(BUFFER_SIZE_GS, '\0');
                    time_t now = time(0);

                    char ip_str[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &(addr_tcp.sin_addr), ip_str, INET_ADDRSTRLEN);
                    
                    if (case_player(buffer_r, send_string, now, args_verbose) != 0) {
                        cerr << "Mensagem TCP inválida!" << endl;
                        send_string = "ERR\n";
                    } else if (verbose) {
                        verbose_str = "[VERBOSE] "+ args_verbose;
                        string ip_addr = string(ip_str);
                        if (ip_addr == "127.0.0.1")
                            verbose_str += " [Ip Address: localhost]";
                        else
                            verbose_str += " [Ip Address: " + string(ip_str)+ "]\n";
                        cout << verbose_str << endl;
                    }
                    args_verbose.clear();
                    verbose_str.clear();
                    
                    
                    const char *buffer_send = send_string.c_str();
                    if (send_message_tcp_server(client_fd, buffer_send, BUFFER_SIZE_GS) < 0) {
                        perror("Erro ao enviar mensagem TCP");
                    }
                    free(buffer);
                    close(client_fd);
                    send_string.clear();
                // } else {
                //     close(client_fd); 
                // }
            }
        }
        free(buffer);
    }
    free(gs_port);
    close(fd_tcp);
    close(fd_udp);

    return 0;
}
