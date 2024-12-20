# Nome do compilador
CXX = g++

# Flags de compilação
CXXFLAGS = -Wall -std=c++11 -I./UdpTcp

# Diretórios
PLAYER_DIR = PLAYER
SERVER_DIR = SERVER
UDPTCP_DIR = UdpTcp
SCORES_DIR = SERVER/SCORES
GAMES_DIR = SERVER/GAMES
# Arquivos fonte para os executáveis
PLAYER_SRC = $(PLAYER_DIR)/player.cpp $(PLAYER_DIR)/libPlayer.cpp $(UDPTCP_DIR)/udpTcp.cpp
SERVER_SRC = $(SERVER_DIR)/GS.cpp $(SERVER_DIR)/lib.cpp $(UDPTCP_DIR)/udpTcp.cpp

# Nomes dos executáveis
PLAYER_EXEC = player
SERVER_EXEC = server

# Regra padrão: compilar tudo
all: $(PLAYER_EXEC) $(SERVER_EXEC)

# Regra para compilar o player
$(PLAYER_EXEC): $(PLAYER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Regra para compilar o servidor
$(SERVER_EXEC): $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Regra para limpar os arquivos compilados
clean:
	rm -f $(PLAYER_EXEC) $(SERVER_EXEC)
	rm -f $(PLAYER_DIR)/*.o $(SERVER_DIR)/*.o $(UDPTCP_DIR)/*.o
	rm -f $(PLAYER_DIR)/*.txt $(SERVER_DIR)/*.txt $(GAMES_DIR)/*.txt $(SCORES_DIR)/*.txt
	rm -fr $(GAMES_DIR) $(SCORES_DIR)


# Garante que a regra 'clean' seja chamada corretamente
.PHONY: all clean run_player run_server
