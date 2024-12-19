# Nome do compilador
CXX = g++

# Flags de compilação
CXXFLAGS = -Wall -std=c++11 -I../UdpTcp

# Arquivos fonte
SRC = GS.cpp lib.cpp ../UdpTcp/udpTcp.cpp

# Nome do executável
EXEC = server

gm = GAMES
sm = SCORES

# Regra padrão: compilar o servidor
all: $(EXEC)

# Regra para compilar o executável
$(EXEC): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(SRC)

# Regra para limpar os arquivos compilados
clean:
	rm -f $(EXEC)
	rm -rf $(gm) $(sm)
	rm -f *.txt
	rm -f *.o

# Garante que a regra 'clean' seja chamada corretamente
.PHONY: all clean
