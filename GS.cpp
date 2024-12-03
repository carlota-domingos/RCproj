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


#define PORT "58001"
#define BUFFER_SIZE 128
#define NUM_COLORS 4
class game_player;
std::vector<game_player> players;




class game_player {
public:
    std::string plid;   // Identificador único do jogador
    int nT;             // Número de tentativas
    std::string codigo;

    game_player(const std::string& id, int time, const std::string& colors)
        : plid(id), nT(time), codigo(colors) {}

    // Reseta o estado do jogador
    void reset() {
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
        std::cout << "Attempts: " << nT << "\n";
        std::cout << "Code: " << codigo << "\n";
    }
};

int init_game(const std::string& PLID, int time){
    //checkar e ver se o plid ja esta ativo ou nao 
    //ver return dependendo do caso do erro
    char colour_code[NUM_COLORS + 1]; // +1 para o terminador nulo
    generate_random_colors(colour_code);
    game_player new_player(PLID, time, std::string(colour_code));
    players.emplace_back(new_player);
    new_player.display_info();
    //fazer alguma cena com o plid TAA
    //iniciar o timer com o time 
    //criar codigo cores e somehow liga lo ao plid  TAA
    //mandar msgm a avisar q ta tudo pronto
    return 0;

}

int case_player(const char* buffer_received) {
    std::string buffer(buffer_received); // Converte o buffer recebido em std::string
    std::string PLID; // Para armazenar o PLID
    std::string send_buffer; // Para armazenar a mensagem a enviar
    std::cout << "Buffer recebido: '" << buffer << "'" << std::endl;

    // Caso SCORES
    if (buffer.compare("SSB") == 0) {
        std::cout << "Entrou no caso SCORES" << std::endl;
    }
    // Caso GAMES
    else if (buffer.substr(0, 4).compare("STR ") == 0 && buffer.size() == 10) {
        PLID = buffer.substr(4, 6); 
        std::cout << "Entrou no caso GAMES com PLID: " << PLID << std::endl;
    }
    // Caso QUIT
    else if (buffer.substr(0, 4).compare("QUT ") == 0 && buffer.size() == 10) {
        PLID = buffer.substr(4, 6);
        std::cout << "Entrou no caso QUIT com PLID: " << PLID << std::endl;
    }
    // Caso DEBUG 
    else if (buffer.substr(0, 4).compare("DBG ") == 0 && buffer.size() == 14) {
        PLID = buffer.substr(4, 6);
        std::cout << "Entrou no caso DEBUG com PLID: " << PLID << std::endl;
    }
    // Caso TRY
    else if (buffer.substr(0, 4).compare("TRY ") == 0 && buffer.size() == 10) {
        PLID = buffer.substr(4, 6);
        std::cout << "Entrou no caso TRY com PLID: " << PLID << std::endl;
    }
    // Caso START NEW GAME
    else if (buffer.substr(0, 4).compare("SNG ") == 0 && buffer.size() == 14) {
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





// Função principal
int main() {
    
    struct addrinfo* infoaddr = nullptr;
    struct sockaddr_in addr{};
    char buffer[BUFFER_SIZE];
    socklen_t addrlen_udp = sizeof(addr);

    create_directories();
    //create_file("SCORES", "scoreboard.txt");

    // Inicializa o socket
    int fd_udp = init_socket_server(infoaddr);
    if (fd_udp < 0) {
        return 1;
    }

    // Vincula o socket ao endereço
    if (bind_socket_server(fd_udp, infoaddr) < 0) {
        freeaddrinfo(infoaddr);
        close(fd_udp);
        return 1;
    }


    
    // Loop de recepção e envio de mensagens durante o jogo apenas
    while (1) {
        ssize_t n = receive_message_server(fd_udp, buffer, BUFFER_SIZE, addr, addrlen_udp);
        if (n == -1) {
            freeaddrinfo(infoaddr);
            close(fd_udp);
            return 1; 
        }

        write(1, "received: ", 10);
        write(1, buffer, n);
        write(1, "\n", 1);
        
        if (case_player(buffer) != 0) {
            std::cerr << "Erro ao processar o buffer!" << std::endl;
        }

        // Envia a mensagem de volta para o endereço de onde foi recebida
        if (send_message_server(fd_udp, "tudo tudo ok", 12, addr, addrlen_udp) < 0) {
            freeaddrinfo(infoaddr);
            close(fd_udp);
            return 1; // Erro ao enviar mensagem
        }
    }

    // Liberação de recursos
    freeaddrinfo(infoaddr);
    close(fd_udp);
    return 0;
}
