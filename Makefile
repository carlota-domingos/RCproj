# Nome do compilador
CXX = g++

# Flags de compilação
CXXFLAGS = -Wall -std=c++11

# Arquivos fonte
SRC = GS.cpp lib.cpp

# Nome do executável
EXEC = server 

gm = SERVER/GAMES 
sc = SERVER/SCORES 

# Regra padrão: compilar o servidor
all: $(EXEC)

# Regra para compilar o executável
$(EXEC): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(SRC)

# Regra para limpar os arquivos compilados
clean:
	rm -f $(EXEC)
	rm -rf $(gm) $(sc)

# Garante que a regra 'clean' seja chamada corretamente
.PHONY: all clean
