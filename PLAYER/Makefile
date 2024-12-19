# Nome do compilador
CXX = g++

# Flags de compilação
CXXFLAGS = -Wall -std=c++11 -I../UdpTcp

# Arquivos fonte
SRC = player.cpp libPlayer.cpp ../UdpTcp/udpTcp.cpp

# Nome do executável
EXEC = player

# Regra padrão: compilar o servidor
all: $(EXEC)

# Regra para compilar o executável
$(EXEC): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(SRC)

# Regra para limpar os arquivos compilados
clean:
	rm -f $(EXEC)
	rm -f *.txt
	rm -f *.o

# Garante que a regra 'clean' seja chamada corretamente
.PHONY: all clean
