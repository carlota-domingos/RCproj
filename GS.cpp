nao tou a conseguir por coisas no git, vou te mandar 
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
class game_player;
std::vector<game_player> players;
int flag=0; //isto era so porque queria colocar o tcp pronto sem tar a incomodar 
    



class game_player {
public:
    std::string plid;   // Identificador único do jogador
    int time;
    int nT;             // Número de tentativas
    std::string codigo;
    

    game_player(const std::string& id, int time, int nT, const std::string& colors)
        : plid(id), time(time), nT(nT), codigo(colors)  {}

    // Reseta o estado do jogador
    void reset(const std::string& id) {
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
    void update_codigo(const std::string& new_code) {
        codigo = new_code;
    }

    // Exibe informações do jogador
    void display_info() const {
        std::cout << "Player ID: " << plid << "\n";
        std::cout << "tempo " << time << "\n";
        std::cout << "tentativas " << nT << "\n";
        std::cout << "Code: " << codigo << "\n";
    }
};

int init_game(const std::string& PLID, int time){
    //checkar e ver se o plid ja esta ativo ou nao 
    //ver return dependendo do caso do erro
    char colour_code[NUM_COLORS + 1]; // +1 para o terminador nulo
    generate_random_colors(colour_code);
    game_player new_player(PLID, time, 0, std::string(colour_code));
    players.emplace_back(new_player);
    new_player.display_info();
    //fazer alguma cena com o plid TAA
    //iniciar o timer com o time 
    //criar codigo cores e somehow liga lo ao plid  TAA
    //mandar msgm a avisar q ta tudo pronto
    return 0;

}
/* 
int get_thread(const std::string& PLID) {
    for (size_t i = 0; i < threads_ativas; i++) {
        if (players[i].plid == PLID) {
            return players[i].thread; // Retorna o identificador da thread
        }
    }
    return -1; // Retorna -1 se o jogador não for encontrado
}
*/

game_player* find_player(const std::string& plid) {
    auto it = std::find_if(players.begin(), players.end(), [&plid](const game_player& player) {
        return player.plid == plid;
    });

    if (it != players.end()) {
        return &(*it); // Retorna o ponteiro para o jogador encontrado
    }
    return nullptr; // Retorna nullptr se não encontrar o jogador
}



int case_player(const char* buffer_received) {
    std::string buffer(buffer_received); // Converte o buffer recebido em std::string
    std::string PLID; // Para armazenar o PLID
    std::string send_buffer; // Para armazenar a mensagem a enviar
    std::cout << "Buffer recebido: '" << buffer << "'" << std::endl;
    printf("Buffer recebido aaaaaa: %s\n", buffer.c_str());
    printf("parou");
    printf("%s\n", buffer.substr(0, 4).c_str());
    printf("Tamanho do buffer: %zu\n", buffer.size());
    // Caso SCORES
    if (buffer.compare("SSB") == 0) {
        std::cout << "Entrou no caso SCORES" << std::endl;
        flag=2; //isto era so porque queria colocar o tcp pronto sem tar a incomodar 
    
    }
    // Caso GAMES
    else if (buffer.substr(0, 4).compare("STR ") == 0 && buffer.size() == 11) {
        PLID = buffer.substr(4, 6); 
        std::cout << "Entrou no caso GAMES com PLID: " << PLID << std::endl;
        flag=2; //isto era so porque queria colocar o tcp pronto sem tar a incomodar 
    
    }
    // Caso QUIT da resert as infos sobre este player
    else if (buffer.substr(0, 4).compare("QUT ") == 0 && buffer.size() == 11) {
        PLID = buffer.substr(4, 6);
        game_player* player = find_player(PLID);

        if (player) {
            std::cout << "Jogador encontrado: " << player->plid << std::endl;

            // Chamando reset para o jogador encontrado
            player->reset(player->plid);

            std::cout << "Informações do jogador após reset:" << std::endl;
            player->display_info();

            //eventualmente podemos adicionar aqui algo para mandar esta info para o ficheiro games antes de ser apagado
        } else {
            std::cout << "Jogador com PLID " << PLID << " não encontrado." << std::endl;
        }

        std::cout << "Jogador terminou o jogo " << PLID << std::endl;
    }
    // Caso DEBUG 
    else if (buffer.substr(0, 4).compare("DBG ") == 0 && (buffer.size() == 14 || buffer.size() == 15)) {
        PLID = buffer.substr(4, 6);
        std::cout << "Entrou no caso DEBUG com PLID: " << PLID << std::endl;
    }
    // Caso TRY
    else if (buffer.substr(0, 4).compare("TRY ") == 0 && buffer.size() == 11) {
        PLID = buffer.substr(4, 6);
        std::cout << "Entrou no caso TRY com PLID: " << PLID << std::endl;
    }
    // Caso START NEW GAME && buffer.size() == 14
    else if (buffer.substr(0, 4).compare("SNG ") == 0 && (buffer.size() == 14 || buffer.size() == 15)) {
        PLID = buffer.substr(4, 6);
        int tempo_max = std::stoi(buffer.substr(11, 3)); 
        if (tempo_max > 600) {
            std::cout << " escede tempo_max permitido (menor de 600) " << std::endl;
            //podemos por esta msgm a enviar em uml
            return -1;
        }     
        
        std::cout << "Entrou no caso START NEW GAME com PLID: " << PLID 
                  << " e tempo_max: " << tempo_max << std::endl;
        init_game(PLID, tempo_max);
    }
    else {
        std::cerr << "Mensagem inválida ou Player deu Quit" << std::endl;
        return -1; // Indica erro
    }
    return 0; // Sucesso
}



int main() {
    struct addrinfo* infoaddr = nullptr;
    struct sockaddr_in addr_udp;
    struct sockaddr_in addr_tcp;
    socklen_t addrlen_udp = sizeof(addr_udp);
    socklen_t addrlen_tcp = sizeof(addr_tcp);
    char buffer[BUFFER_SIZE];

    // Inicializa o servidor UDP
    int fd_udp = init_socket_server(infoaddr);
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
            int fd_tcp = init_tcp_server(PORT);
            if (fd_tcp < 0) {
                freeaddrinfo(infoaddr);
                close(fd_udp);
                return 1;
            }

            std::cout << "Servidor TCP e UDP inicializados com sucesso. Aguardando conexões..." << std::endl;

            // Aceitar conexão TCP
            if ((accept_connection_tcp_server(fd_tcp, &addr_tcp, &addrlen_tcp)) < 0) {
                freeaddrinfo(infoaddr);
                close(fd_udp);
                close(fd_tcp);
                return 1;
            }

            std::cout << "Cliente TCP conectado." << std::endl;

            // Aceitar conexão TCP
            if ((accept_connection_tcp_server(fd_tcp, &addr_tcp, &addrlen_tcp)) < 0) {
                freeaddrinfo(infoaddr);
                close(fd_udp);
                close(fd_tcp);
                return 1;
            }

            std::cout << "Cliente TCP conectado." << std::endl;

            // Recebe mensagem TCP
            ssize_t n_tcp = read_message_tcp_server(fd_tcp, buffer, BUFFER_SIZE);
            if (n_tcp == 0) { // Cliente desconectou
                std::cout << "Cliente TCP desconectou." << std::endl;
                break;
            } else if (n_tcp == -1) {
                perror("Erro ao receber mensagem TCP");
                break;
            }

            std::cout << "Mensagem TCP recebida: " << std::string(buffer, n_tcp) << std::endl;

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

            std::cout << "Mensagem UDP recebida: " << std::string(buffer, n_udp) << std::endl;

            // Processa mensagem UDP
            if (case_player(buffer) != 0) {
                std::cerr << "Erro ao processar o buffer UDP!" << std::endl;
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
    return 0;
}