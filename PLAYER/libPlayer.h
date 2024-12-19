#ifndef LIBPLAYER_H_INCLUDED
#define LIBPLAYER_H_INCLUDED
#include <stddef.h>
#include <netdb.h>
#include <cstdio>
#include <arpa/inet.h>
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <filesystem>


using namespace std;

class game_player {
public:
    std::string plid;
    int nT;
    bool active = false;

    game_player(const std::string &id);
    void finish();
    void reset();
    void next_try();
    bool same_try(int server_try) const;
};

int get_msg(string &msg);
int get_file_msg(string &msg, string &file_out);
string rm_spaces(const string& str);
int case_terminal(string &buffer);

int check_active_game(string &, game_player curr_game);
int add_args(string &msg, int code, game_player curr_game);
bool valid_time(const string& str);
int code_val(const string& code);



#endif