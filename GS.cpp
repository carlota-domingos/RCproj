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
#include <ctime>

#define BUFFER_SIZE_GS 1024
#define NUM_COLORS 4
#define NUM_TRIES 8

using namespace std;
class game_player;
vector<game_player> players;

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


string get_str_time(time_t time, int mode) {
    char buffer[20];  // Buffer for the date and time in YYYY-MM-DD HH:MM:SS format
    struct tm *timeinfo;
    if (mode == 0) {
        timeinfo = localtime(&time);
    } else {
        timeinfo = gmtime(&time);
    }
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return string(buffer);
}

class game_file {
public:
    string plid;       // Identificador único do jogador
    int fd_game;       // File descriptor do ficheiro
    time_t time_init;
    string mode;  // Tempo de referência para calcular o tempo das jogadas

    game_file(const string &id, string &mode, const string &code, string &timeout) {
        plid=id;
        time_init=time(0);
        mode= mode;
        string name= "GAMES_"+ id + ".txt";
        //open
        //string firstline = plid + " " + mode + " " + code + " " + timeout + " " + get_str_time(time_init,0) + " "+ string(time_init) + "\n";
    }

    void new_line(string &code, int nb, int nw) {
        time_t game_time = time(0) - time_init;
        string time_str = to_string(game_time);
        string line = "T: " + code + " " + to_string(nb) + " " + to_string(nw) + " " + time_str + "\n";
        //Assuming fd_game is a valid file descriptor and write is a valid function
        write(fd_game, line.c_str(), line.size());
    }
    void finish_game(time_t finishtime, string term, string score, int nT){
        //time_t game_time = finishtime - time_init;
        // string last_line = get_str_time(finishtime,0) + " " + string(game_time)+ "\n";
        string new_fn = get_str_time(finishtime,1)+"_"+term+ ".txt";
        if (term =="W"){
            string score_fn= score+"_"+ plid+"_"+get_str_time(finishtime,1)+".txt";
            //enviar isto e gerar priemira linha i guess
        }
    }
    // Exibe informações do arquivo de jogo
    void display_info() const {
        cout << "Player ID: " << plid << "\n";
        cout << "File Descriptor: " << fd_game << "\n";
        cout << "Time Init: " << time_init << "\n";
    }

} ;
class game_player
{
public:
    string plid;        // Identificador único do jogador
    int time;           // maybe actual time will not be used
    int nT;             // Número de tentativas
    string codigo = ""; // acts to know if a timeout or finished msg has been sent
    bool ativo = false;
    int fd_game;
    int score;

    game_player(const string &id)
        : plid(id), ativo(false) {}

    void start_game(int tempo, string &cores)
    {
        time = tempo;
        ativo = true;
        nT = 0;
        codigo = cores;
        // abrir file e tentar mexer com ele
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

    void finish()
    {
        ativo = false;
        time = 0;
        codigo = "";
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

//NORMAL MODE
int init_game(const string &PLID, int time)
{
    game_player *player = find_player(PLID);
    if (player && player->ativo)
    {
        cout << "Jogador com PLID " << PLID << " já está ativo." << endl;
        return 1;
    } else if( player && !player->ativo)
    {
        cout << "Entrou no caso START NEW GAME com PLID: " << PLID
            << " e tempo_max: " << time << endl;
        char colour_code[NUM_COLORS + 1]; // +1 para o terminador nulo
        generate_random_colors(colour_code);
        string colour_code_str(colour_code);
        player->start_game(time, colour_code_str);
        player->display_info();
        for (auto it = players.begin(); it != players.end(); ++it) {
            cout << it->plid << " " << endl;
        }
        return 0;
    }
    cout << "Entrou no caso START NEW GAME com PLID: " << PLID
         << " e tempo_max: " << time << endl;
    char colour_code[NUM_COLORS + 1]; // +1 para o terminador nulo
    generate_random_colors(colour_code);
    game_player new_player(PLID);
    string colour_code_str(colour_code);
    new_player.start_game(time, colour_code_str);
    players.emplace_back(new_player);
    new_player.display_info();
    for (auto it = players.begin(); it != players.end(); ++it) {
        cout << it->plid << " " << endl;
    }
    // fazer alguma cena com o plid TAA
    // iniciar o timer com o time
    // criar codigo cores e somehow liga lo ao plid  TAA
    // mandar msgm a avisar q ta tudo pronto
    return 0;
}

//DEBUG
int init_game(const string &PLID, int time, string &code)
{
    game_player *player = find_player(PLID);
    if (player && player->ativo)
    {
        cout << "Jogador com PLID " << PLID << " já está ativo." << endl;
        return 1;
    } else if (player && !player->ativo)
    {
        cout << "Entrou no caso START NEW GAME com PLID: " << PLID
            << " e tempo_max: " << time << endl;
        player->start_game(time, code);
        player->display_info();
        return 0;
    }
    cout << "Entrou no caso START NEW GAME com PLID: " << PLID
         << " e tempo_max: " << time << endl;
    game_player new_player(PLID);
    new_player.start_game(time, code);
    players.emplace_back(new_player);
    new_player.display_info();

    // fazer alguma cena com o plid TAA
    // iniciar o timer com o time
    // criar codigo cores e somehow liga lo ao plid  TAA
    // mandar msgm a avisar q ta tudo pronto
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

void process_player(game_player *player, string &code, int nT, string& send_buffer)
{
    if (player)
    {
        if (player->ativo)
        {
            if (player->same_try(nT) || player->same_try(nT - 1))
            {
                cout << "Jogador com PLID " << player->plid << " está ativo." << endl;
                int nW = 0;
                int nB = 0;
                if (player->findtry(code) == 1)
                {
                    cout << "Tentiva duplicada." << endl;
                    send_buffer = "RTR DUP\n";
                    return;
                }
                match_code(player->codigo, code, nW, nB);
                cout << "nW: " << nW << " nB: " << nB << endl;
                if (nT == NUM_TRIES && nB != NUM_COLORS)
                {
                    cout << "Tempo esgotado para o jogador" << endl;
                    send_buffer = "RTR ENT " + player->codigo + "\n";
                    player->finish();
                    return;
                }
                else if (nB == NUM_COLORS)
                {
                    cout << "Jogador com PLID " << player->plid << " acertou no código." << endl;
                    send_buffer = "RTR OK " + to_string(player->nT) + " " + to_string(nB) + " " + to_string(nW) + player->codigo + "\n";
                    player->finish();
                    return;
                }
                player->next_try();
                send_buffer = "RTR OK " + to_string(player->nT) + " " + to_string(nB) + " " + to_string(nW) + "\n";
                return;
                
            }
            else
            {
                cout << "Número de tentativas inválido" << endl;
                send_buffer = "RTR INV\n";
                return;
            }
        }
        else
        {
            printf("jogo nao encontrado");
            if (player->codigo.empty())
            {
                printf("codigo vazio");
                cout << "Jogador com PLID " << player->plid << " não está ativo." << endl;
                send_buffer = "RTR NOK\n";
                return;
            }
            else
            {
                cout << "Jogador com PLID " << player->plid << " não está ativo." << endl;
                cout << "Tempo esgotado para o jogador" << endl;
                send_buffer = "RTR ETM " + player->codigo + "\n";
                player->finish();
                return;
            }
        }
    }
    else
    {
        cout << "Jogador não encontrado." << endl;
        send_buffer = "RTR NOK\n";
        return;
    }
}

int case_player(string &buffer, string &send_buffer)
{
    string PLID;        // Para armazenar o PLID
    cout << "Buffer recebido: '" << buffer << "'" << endl;
    printf("Tamanho do buffer: %zu\n", buffer.size());
    // Caso SCORES
    if (buffer.compare("SSB\n") == 0)
    {
        cout << "Entrou no caso SCORES" << endl;
        send_buffer = "RSS EMPTY\n";
        // tambem nao funciona
    }
    // Caso GAMES
    else if (buffer.substr(0, 4).compare("STR ") == 0)
    {
        regex pattern("^STR (\\d{6})\n$");
        smatch matches;
        if (regex_match(buffer, matches, pattern))
        {
            game_player *player = find_player(PLID);
            if (player)
            {
                if (player->ativo)
                {
                    cout << "Jogador com PLID " << PLID << " está ativo." << endl;
                    send_buffer = "RST OK ";
                    // eventualmente podemos adicionar aqui algo para mandar esta info para o ficheiro games antes de ser apagado
                }
                else
                {
                    cout << "Jogador com PLID " << PLID << " nao está ativo." << endl;
                    send_buffer = "RST FIN ";
                }
                // eventualmente podemos adicionar aqui algo para mandar esta info para o ficheiro games antes de ser apagado
            }
            else
            {
                cout << "Jogador com PLID " << PLID << " não encontrado." << endl;
                send_buffer = "RST NOK\n";
            }
        }
        else
        {
            cout << "Sintaxe do PLID inválida." << endl;
            send_buffer = "RST NOK\n";
        }
    }
    // Caso QUIT da resert as infos sobre este player
    else if (buffer.substr(0, 4).compare("QUT ") == 0)
    {
        regex pattern("^QUT (\\d{6})\n$");
        smatch matches;
        if (regex_match(buffer, matches, pattern))
        {
            PLID = matches[1];
            game_player *player = find_player(PLID);
            if (player)
            {
                if (player->ativo)
                {
                    cout << "Jogador com PLID " << PLID << " está ativo." << endl;
                    send_buffer = "RQT OK " + player->codigo + "\n";
                    player->finish();
                    // eventualmente podemos adicionar aqui algo para mandar esta info para o ficheiro games antes de ser apagado
                }
                else
                {
                    cout << "Jogador com PLID " << PLID << " não está ativo." << endl;
                    send_buffer = "RQT NOK\n";
                }
                cout << "Informações do jogador após reset:" << endl;
                player->display_info();
                // eventualmente podemos adicionar aqui algo para mandar esta info para o ficheiro games antes de ser apagado
            }
            else
            {
                cout << "Jogador com PLID " << PLID << " não encontrado." << endl;
                send_buffer = "RQT NOK\n";
            }
        }
        else
        {
            cout << "Sintaxe do PLID inválida." << endl;
            send_buffer = "RQT ERR\n";
        }
    }
    // Caso DEBUG
    else if (buffer.substr(0, 4).compare("DBG ") == 0)
    {
        regex pattern("^DBG (\\d{6}) (\\d{1,3}) (.*)\n$");
        smatch matches;
        int tempo_max;
        if (regex_match(buffer, matches, pattern))
        {
            string PLID = matches[1];
            string time = matches[2];
            string code = matches[3];
            if (!valid_time(time) || !code_val(code))
            {
                cout << "Tempo ou código inválido" << endl;
                send_buffer = "RDB ERR\n";
            }
            else
            {
                tempo_max = stoi(time);
            }
            if (init_game(PLID, tempo_max, code) == 1)
            {
                send_buffer = "RDB NOK\n";
            }
            else
            {
                send_buffer = "RDB OK\n";
            }
        }
        else
        {
            cout << "Sintaxe Invalida" << endl;
            send_buffer = "RDB ERR\n";
        }
    }
    // Caso START NEW GAME
    else if (buffer.substr(0, 4).compare("SNG ") == 0)
    {
        regex pattern("^SNG (\\d{6}) (\\d{1,3})\n$");
        smatch matches;
        int tempo_max;
        if (regex_match(buffer, matches, pattern))
        {
            printf("Entrou no caso START NEW GAME\n");
            PLID = matches[1];
            string tempo = matches[2];
            if (!valid_time(tempo))
            {
                cout << "Tempo máximo inválido" << endl;
                send_buffer = "RSG ERR\n";
            }
            else
            {
                tempo_max = stoi(tempo);
                if (init_game(PLID, tempo_max) == 1)
                {
                    send_buffer = "RSG NOK\n";
                }
                else
                {
                    send_buffer = "RSG OK\n";
                }
            }
        }
        else
        {
            cout << "PLID ou tempo inválido" << endl;
            send_buffer = "RSG ERR\n";
        }
    }// Caso TRY
    else if (buffer.substr(0, 4).compare("TRY ") == 0)
    {
        regex pattern("^TRY (\\d{6}) ([RGBYOP]) ([RGBYOP]) ([RGBYOP]) ([RGBYOP]) (\\d{1})\\s\n?*$");
        smatch matches;
        if (regex_match(buffer, matches, pattern))
        {
            PLID = matches[1];
            string code = matches[2].str() + matches[3].str()  + matches[4].str()  + matches[5].str() + '\0';
            cout << "Code: " << code << endl;
            string nT = matches[6];
            game_player *player = find_player(PLID);
            process_player(player, code, stoi(nT), send_buffer);
            printf("Entrou no caso TRY\n");
        }
        else
        {
            cout << "sintaxe invalida" << endl;
            send_buffer = "RTR ERR\n";
        }
    }
    else
    {
        cerr << "Mensagem inválida ou Player deu Quit" << endl;
        send_buffer = "ERR\n";
        return -1; // Indica erro
    }
    cout << "ta no final " << endl;
    cout << "Mensagem a enviar: '" << send_buffer << "'" << endl;
    return 0; // Sucesso
}

int validate_args(int argc, char *argv[], const char *&gs_port, bool &verbose)
{
    // Parse command-line arguments
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
        {
            gs_port = argv[++i];
        }
        else if (strcmp(argv[i], "-v") == 0)
        {
            verbose = true;
        }
        else
        {
            cerr << "Usage: " << argv[0] << " [-p GSport] [-v]" << endl;
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    const char *gs_port = "58081"; // Default port number
    bool verbose = false;          // Default verbose mode
    if (validate_args(argc, argv, gs_port, verbose) != 0)
    {
        return 1;
    }
    // create_directories();
    // em comentario para ser mais pratico fazer make por agora
    struct addrinfo *infoaddr = nullptr;
    // struct sockaddr_in addr_udp;
    // struct sockaddr_in addr_tcp;
    // socklen_t addrlen_udp = sizeof(addr_udp);
    // socklen_t addrlen_tcp = sizeof(addr_tcp);

    // Inicializa o servidor UDP
    int fd_udp = init_socket_server(infoaddr, gs_port);
    if (fd_udp < 0)
    {
        return 1;
    }

    if (bind_socket_server(fd_udp, infoaddr) < 0)
    {
        freeaddrinfo(infoaddr);
        close(fd_udp);
        return 1;
    }

    int fd_tcp = init_tcp_server(gs_port);
    if (fd_tcp < 0)
    {
        close(fd_udp);
        return 1;
    }

    fd_set read_fds;
    vector<int> client_fds;
    int max_fd = max(fd_udp, fd_tcp);

    while (true)
    {
        char *buffer = (char *)malloc(BUFFER_SIZE);

        if (buffer == nullptr)
        {
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
        if (activity < 0)
        {
            perror("select");
            free(buffer);
            break;
        }

        if (FD_ISSET(fd_udp, &read_fds))
        {
            string send_string(BUFFER_SIZE, '\0');
            struct sockaddr_in addr_udp;
            socklen_t addrlen_udp = sizeof(addr_udp);
            ssize_t n_udp = receive_message_server(fd_udp, buffer, BUFFER_SIZE, addr_udp, addrlen_udp);
            if (n_udp == -1)
            {
                perror("Erro ao receber mensagem UDP");
                send_string.clear();
                free(buffer);
                break;
            }
            cout << "Mensagem UDP recebida: " << string(buffer, n_udp) << endl;
            buffer_r = string(buffer, n_udp);
            // Processa mensagem UDP
            if (case_player(buffer_r, send_string) != 0)
            {
                cerr << "Erro ao processar o buffer UDP!" << endl;
                send_string = "ERR\n";
            }
            // Envia resposta UDP
            const char* buffer_send = send_string.c_str();
            cout << "Mensagem a enviar(fora do case): " << buffer_send << endl;
            if (send_message_server(fd_udp, buffer_send , BUFFER_SIZE, addr_udp, addrlen_udp) < 0)
            {
                perror("Erro ao enviar mensagem UDP");
                free(buffer);
                send_string.clear();
                break;
            }
            send_string.clear();
            continue;
        }

        if (FD_ISSET(fd_tcp, &read_fds))
        {
            string send_string(BUFFER_SIZE_GS, '\0');
            struct sockaddr_in addr_tcp;
            socklen_t addrlen_tcp = sizeof(addr_tcp);
            int client_fd = accept_connection_tcp_server(fd_tcp, &addr_tcp, &addrlen_tcp);
            if (client_fd >= 0)
            {
                printf("Conneccao aceite com fd: %d\n", client_fd);
                client_fds.push_back(client_fd);
                max_fd = max(max_fd, client_fd); // Update max_fd
            }
            ssize_t n_tcp = read_message_tcp_server(client_fd, buffer, BUFFER_SIZE);
            if (n_tcp == 0)
            { // Cliente desconectou
                cout << "Cliente TCP desconectou." << endl;
                close(client_fd);
            }
            else if (n_tcp == -1)
            {
                perror("Erro ao receber mensagem TCP");
                close(client_fd);
                send_string= "ERR\n";
            }
            else
            {
                cout << "Mensagem TCP recebida: " << string(buffer, n_tcp) << endl;
                buffer_r = string(buffer, n_tcp);
                if (case_player(buffer_r, send_string) != 0)
                {
                    cerr << "Mensagem TCP invalida!" << endl;
                    echo_message_tcp_player(client_fd, "ERR", 3);
                }
                const char* buffer_send = send_string.c_str();
                cout << "Mensagem a enviar: " << buffer_send << endl;
                if (echo_message_tcp_player(client_fd, buffer_send, BUFFER_SIZE_GS) < 0)
                {
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