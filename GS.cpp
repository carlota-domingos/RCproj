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
    

    game_player(const string& id, int time, int nT, const string& colors)
        : plid(id), time(time), nT(nT), codigo(colors)  {}

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

    // Exibe informações do jogador
    void display_info() const {
        cout << "Player ID: " << plid << "\n";
        cout << "tempo " << time << "\n";
        cout << "tentativas " << nT << "\n";
        cout << "Code: " << codigo << "\n";
    }
};

int init_game(const string& PLID, int time){
    //checkar e ver se o plid ja esta ativo ou nao 
    //ver return dependendo do caso do erro
    char colour_code[NUM_COLORS + 1]; // +1 para o terminador nulo
    generate_random_colors(colour_code);
    game_player new_player(PLID, time, 0, string(colour_code));
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

game_player* find_player(const string& plid) {
    auto it = find_if(players.begin(), players.end(), [&plid](const game_player& player) {
        return player.plid == plid;
    });

    if (it != players.end()) {
        return &(*it); // Retorna o ponteiro para o jogador encontrado
    }
    return nullptr; // Retorna nullptr se não encontrar o jogador
}



int case_player(const char* buffer_received) {
    string buffer(buffer_received); // Converte o buffer recebido em string
    string PLID; // Para armazenar o PLID
    string send_buffer; // Para armazenar a mensagem a enviar
    cout << "Buffer recebido: '" << buffer << "'" << endl;
    printf("Buffer recebido aaaaaa: %s\n", buffer.c_str());
    printf("parou");
    printf("%s\n", buffer.substr(0, 4).c_str());
    printf("Tamanho do buffer: %zu\n", buffer.size());
    // Caso SCORES
    if (buffer.compare("SSB") == 0) {
        cout << "Entrou no caso SCORES" << endl;
        flag=2; //isto era so porque queria colocar o tcp pronto sem tar a incomodar 
    
    }
    // Caso GAMES
    else if (buffer.substr(0, 4).compare("STR ") == 0 && buffer.size() == 11) {
        PLID = buffer.substr(4, 6); 
        cout << "Entrou no caso GAMES com PLID: " << PLID << endl;
        flag=2; //isto era so porque queria colocar o tcp pronto sem tar a incomodar 
    
    }
    // Caso QUIT da resert as infos sobre este player
    else if (buffer.substr(0, 4).compare("QUT ") == 0 && buffer.size() == 11) {
        PLID = buffer.substr(4, 6);
        game_player* player = find_player(PLID);

        if (player) {
            cout << "Jogador encontrado: " << player->plid << endl;

            // Chamando reset para o jogador encontrado
            player->reset(player->plid);

            cout << "Informações do jogador após reset:" << endl;
            player->display_info();

            //eventualmente podemos adicionar aqui algo para mandar esta info para o ficheiro games antes de ser apagado
        } else {
            cout << "Jogador com PLID " << PLID << " não encontrado." << endl;
        }

        cout << "Jogador terminou o jogo " << PLID << endl;
    }
    // Caso DEBUG 
    else if (buffer.substr(0, 4).compare("DBG ") == 0 && (buffer.size() == 14 || buffer.size() == 15)) {
        PLID = buffer.substr(4, 6);
        cout << "Entrou no caso DEBUG com PLID: " << PLID << endl;
    }
    // Caso TRY
    else if (buffer.substr(0, 4).compare("TRY ") == 0 && buffer.size() == 11) {
        PLID = buffer.substr(4, 6);
        cout << "Entrou no caso TRY com PLID: " << PLID << endl;
    }
    // Caso START NEW GAME && buffer.size() == 14
    else if (buffer.substr(0, 4).compare("SNG ") == 0 && (buffer.size() == 14 || buffer.size() == 15)) {
        PLID = buffer.substr(4, 6);
        int tempo_max = stoi(buffer.substr(11, 3)); 
        if (tempo_max > 600) {
            cout << " escede tempo_max permitido (menor de 600) " << endl;
            //podemos por esta msgm a enviar em uml
            return -1;
        }     
        
        cout << "Entrou no caso START NEW GAME com PLID: " << PLID 
                  << " e tempo_max: " << tempo_max << endl;
        init_game(PLID, tempo_max);
    }
    else {
        cerr << "Mensagem inválida ou Player deu Quit" << endl;
        return -1; // Indica erro
    }
    return 0; // Sucesso
}



int main(int argc, char* argv[]) {
    const char* gs_port = "58081"; // Default port number
    bool verbose = false; // Default verbose mode

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
    struct addrinfo* infoaddr = nullptr;
    struct sockaddr_in addr_udp;
    struct sockaddr_in addr_tcp;
    socklen_t addrlen_udp = sizeof(addr_udp);
    socklen_t addrlen_tcp = sizeof(addr_tcp);
    char buffer[BUFFER_SIZE];

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

    

    // Loop de recepção e envio de mensagens UDP e TCP
    while (true) {
        // Recebe mensagem UDP

        if (flag ==2){
            // Inicializa o servidor TCP 
            int fd_tcp = init_tcp_server(gs_port);
            if (fd_tcp < 0) {
                freeaddrinfo(infoaddr);
                close(fd_udp);
                return 1;
            }

            cout << "Servidor TCP e UDP inicializados com sucesso. Aguardando conexões..." << endl;

            // Aceitar conexão TCP
            if ((accept_connection_tcp_server(fd_tcp, &addr_tcp, &addrlen_tcp)) < 0) {
                freeaddrinfo(infoaddr);
                close(fd_udp);
                close(fd_tcp);
                return 1;
            }

            cout << "Cliente TCP conectado." << endl;

            // Aceitar conexão TCP
            if ((accept_connection_tcp_server(fd_tcp, &addr_tcp, &addrlen_tcp)) < 0) {
                freeaddrinfo(infoaddr);
                close(fd_udp);
                close(fd_tcp);
                return 1;
            }

            cout << "Cliente TCP conectado." << endl;

            // Recebe mensagem TCP
            ssize_t n_tcp = read_message_tcp_server(fd_tcp, buffer, BUFFER_SIZE);
            if (n_tcp == 0) { // Cliente desconectou
                cout << "Cliente TCP desconectou." << endl;
                break;
            } else if (n_tcp == -1) {
                perror("Erro ao receber mensagem TCP");
                break;
            }

            cout << "Mensagem TCP recebida: " << string(buffer, n_tcp) << endl;

            // Envia resposta TCP
            echo_message_tcp_player(fd_tcp, "Resposta TCP", 12);
            close(fd_tcp);
            flag =0;

        }
        else{ 
            ssize_t n_udp = receive_message_server(fd_udp, buffer, BUFFER_SIZE, addr_udp, addrlen_udp);
            if (n_udp == -1) {
                perror("Erro ao receber mensagem UDP");
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
                break;
            }

        }        

        
    }

    // Liberação de recursos
    freeaddrinfo(infoaddr);
    close(fd_udp);  
    return 0;
}