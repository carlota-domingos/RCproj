#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include "lib.h"
#include <filesystem>

#define BUFFER_SIZE 128
#define NUM_COLORS 4

using namespace std;
class game_player;
vector<game_player> players;
int flag=0; //isto era so porque queria colocar o tcp pronto sem tar a incomodar 
    



class game_player {
public:
    string plid;   // Identificador único do jogador
    int time;
    int nT;             // Número de tentativas
    string codigo;
    bool ativo = false;

    game_player(const string& id)
        : plid(id), ativo(false) {}

    void start_game(int tempo, string& cores) {
        time = tempo;
        ativo = true;
        nT = 0;
        codigo = cores;
    }
    
    // Reseta o estado do jogador
    void reset(const string& id) {
        nT = 0;
        plid = "none";
        codigo = "none";
        
    }

    // Incrementa o número de tentativas
    void next_try() {
        nT++;
    }   

    // Verifica se o número de tentativas coincide com o do servidor
    bool same_try(int server_try) const {
        return nT == server_try;
    }

    // Atualiza o código do jogador
    void update_codigo(const string& new_code) {
        codigo = new_code;
    }

    void finish() {
        ativo = false;
        time = 0;
    }

    // Exibe informações do jogador
    void display_info() const {
        cout << "Player ID: " << plid << "\n";
        cout << "tempo " << time << "\n";
        cout << "tentativas " << nT << "\n";
        cout << "Code: " << codigo << "\n";
    }
};

game_player* find_player(const string& plid) {
    auto it = find_if(players.begin(), players.end(), [&plid](const game_player& player) {
        return player.plid == plid;
    });

    if (it != players.end()) {
        return &(*it); // Retorna o ponteiro para o jogador encontrado
    }
    return nullptr; // Retorna nullptr se não encontrar o jogador
}

int init_game(const string& PLID, int time){
    game_player* player = find_player(PLID);
    if (player && player->ativo) {
        cout << "Jogador com PLID " << PLID << " já está ativo." << endl;
        return 1;
    }
    cout << "Entrou no caso START NEW GAME com PLID: " << PLID 
                << " e tempo_max: " << time << endl;
    char colour_code[NUM_COLORS + 1]; // +1 para o terminador nulo
    generate_random_colors(colour_code);
    game_player new_player(PLID);
    string colour_code_str(colour_code);
    new_player.start_game(time,colour_code_str);
    players.emplace_back(new_player);
    new_player.display_info();

    //fazer alguma cena com o plid TAA
    //iniciar o timer com o time 
    //criar codigo cores e somehow liga lo ao plid  TAA
    //mandar msgm a avisar q ta tudo pronto
    return 0;

}

int init_game(const string& PLID, int time, string& code){
    game_player* player = find_player(PLID);
    if (player && player->ativo) {
        cout << "Jogador com PLID " << PLID << " já está ativo." << endl;
        return 1;
    }
    cout << "Entrou no caso START NEW GAME com PLID: " << PLID 
                << " e tempo_max: " << time << endl;
    game_player new_player(PLID);
    new_player.start_game(time,code);
    players.emplace_back(new_player);
    new_player.display_info();

    //fazer alguma cena com o plid TAA
    //iniciar o timer com o time 
    //criar codigo cores e somehow liga lo ao plid  TAA
    //mandar msgm a avisar q ta tudo pronto
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





int case_player(const char* buffer_received) {
    string buffer(buffer_received); // Converte o buffer recebido em string
    string PLID; // Para armazenar o PLID
    string send_buffer; // Para armazenar a mensagem a enviar
    cout << "Buffer recebido: '" << buffer << "'" << endl;
    printf("Tamanho do buffer: %zu\n", buffer.size());
    // Caso SCORES
    if (buffer.compare("SSB\n") == 0) {
        cout << "Entrou no caso SCORES" << endl;
        //checkar se vazia a diretoria de scores
        //se vazia dar reply com EMPTY
    }
    // Caso GAMES
    else if (buffer.substr(0, 4).compare("STR ") == 0) {
        regex pattern("^STR (\\d{6})\n$");
        smatch matches;
        if (regex_match(buffer, matches, pattern)) {
            game_player* player = find_player(PLID);
            if (player) {
                if (player->ativo) {
                    cout << "Jogador com PLID " << PLID << " está ativo." << endl;
                    send_buffer = "RST OK ";
                    //eventualmente podemos adicionar aqui algo para mandar esta info para o ficheiro games antes de ser apagado
                } else {
                    cout << "Jogador com PLID " << PLID << " nao está ativo." << endl;
                    send buffer = "RST FIN ";
                }
                //eventualmente podemos adicionar aqui algo para mandar esta info para o ficheiro games antes de ser apagado
            } else {
                cout << "Jogador com PLID " << PLID << " não encontrado." << endl;
                send_buffer = "RST NOK\n";
            }
        } else {
            cout << "Sintaxe do PLID inválida." << endl;
            send_buffer = "RST NOK\n";
        }
    
    }
    // Caso QUIT da resert as infos sobre este player
    else if (buffer.substr(0, 4).compare("QUT ") == 0) {
        regex pattern("^QUT (\\d{6})\n$");
        smatch matches;
        if (regex_match(buffer, matches, pattern)) {
            game_player* player = find_player(PLID);
            if (player) {
                if (player->ativo) {
                    cout << "Jogador com PLID " << PLID << " está ativo." << endl;
                    send_buffer = "RQT OK "+ player->codigo + "\n";
                    player->finish(player->plid);
                    //eventualmente podemos adicionar aqui algo para mandar esta info para o ficheiro games antes de ser apagado
                } else {
                    cout << "Jogador com PLID " << PLID << " não está ativo." << endl;
                    send_buffer = "RQT NOK\n"
                }
                cout << "Informações do jogador após reset:" << endl;
                player->display_info();
                //eventualmente podemos adicionar aqui algo para mandar esta info para o ficheiro games antes de ser apagado
            } else {
                cout << "Jogador com PLID " << PLID << " não encontrado." << endl;
                send_buffer = "RQT NOK\n";
            }
        } else {
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
            string PLID  = matches[1];
            string time = matches[2];
            string code = matches[3];
            if (!valid_time(time) || !code_val(code)) {
                cout << "Tempo ou código inválido" << endl;
                send_buffer = "DBG ERR\n";
            } else {
                tempo_max = stoi(time);
            }
            if (init_game(PLID, tempo_max, code) == 1) {
                send_buffer = "DBG NOK\n";
            } else {
                send_buffer = "DBG OK\n";
            }
        } else {
            cout << "Sintaxe Invalida" << endl;
            send_buffer = "DBG ERR\n";
        }
    }
    // Caso TRY
    else if (buffer.substr(0, 4).compare("TRY ") == 0 && buffer.size() == 11) {
        
        PLID = buffer.substr(4, 6);
        cout << "Entrou no caso TRY com PLID: " << PLID << endl;
    }
    // Caso START NEW GAME && buffer.size() == 14
    else if (buffer.substr(0, 4).compare("SNG ") == 0 && (buffer.size() == 14 || buffer.size() == 15)) {
        regex pattern("^SNG (\\d{6}) (\\d{1,3})\n$");
        smatch matches;
        int tempo_max;
        if (regex_match(buffer, matches, pattern)) {
            printf("Entrou no caso START NEW GAME\n");
            PLID  = matches[1];
            string tempo = matches[2];
            if (!valid_time(tempo)) {
                cout << "Tempo máximo inválido" << endl;
                send_buffer = "SNG ERR\n";
            } else {
                tempo_max = stoi(tempo);
            }
        }  else {
            cout << "PLID inválido" << endl;
            send_buffer = "SNG ERR\n";
        }        
        if (init_game(PLID, tempo_max) == 1) {
            send_buffer = "SNG NOK\n";
        } else {
            send_buffer = "SNG OK\n";
        }
    }
    else {
        cerr << "Mensagem inválida ou Player deu Quit" << endl;
        return -1; // Indica erro
    }
    return 0; // Sucesso
}

int validate_args(int argc, char* argv[], const char*& gs_port, bool& verbose) {
    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            gs_port = argv[++i];
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
        } else {
            cerr << "Usage: " << argv[0] << " [-p GSport] [-v]" << endl;
            return 1;
        }
    }
    return 0;
}


int main(int argc, char* argv[]) {
    const char* gs_port = "58081"; // Default port number
    bool verbose = false; // Default verbose mode
    if (validate_args(argc, argv, gs_port, verbose) != 0) {
        return 1;
    }
    create_directories();
    struct addrinfo* infoaddr = nullptr;
    struct sockaddr_in addr_udp;
    struct sockaddr_in addr_tcp;
    socklen_t addrlen_udp = sizeof(addr_udp);
    socklen_t addrlen_tcp = sizeof(addr_tcp);
   
    // Inicializa o servidor UDP
    int fd_udp = init_socket_server(infoaddr, gs_port);
    if (fd_udp < 0) {
        return 1;
    }

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
        char* buffer = (char*)malloc(BUFFER_SIZE);
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
            struct sockaddr_in addr_udp;
            socklen_t addrlen_udp = sizeof(addr_udp);
            ssize_t n_udp = receive_message_server(fd_udp, buffer, BUFFER_SIZE, addr_udp, addrlen_udp);
            if (n_udp == -1) {
                perror("Erro ao receber mensagem UDP");
                free(buffer);
                break;
            }
            cout << "Mensagem UDP recebida: " << string(buffer, n_udp) << endl;
            // Processa mensagem UDP
            if (case_player(buffer) != 0) {
                cerr << "Erro ao processar o buffer UDP!" << endl;
            }
            // Envia resposta UDP
            if (send_message_server(fd_udp, "Resposta UDP", 12, addr_udp, addrlen_udp) < 0) {
                perror("Erro ao enviar mensagem UDP");
                free(buffer);
                break;
            }
            continue;
        }

        if (FD_ISSET(fd_tcp, &read_fds)) {
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
            } else if (n_tcp == -1) {
                perror("Erro ao receber mensagem TCP");
                close(client_fd);
            } else {
                cout << "Mensagem TCP recebida: " << string(buffer, n_tcp) << endl;
                if (case_player(buffer) != 0) {
                    cerr << "Mensagem TCP invalida!" << endl;
                    echo_message_tcp_player(client_fd, "ERR", 3);
                }
                echo_message_tcp_player(client_fd, "Resposta TCP", 12);

            }
            close(client_fd);
        }
        free(buffer);
    }
    
    close(fd_tcp);
    close(fd_udp);

    return 0;
}