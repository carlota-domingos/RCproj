#ifndef LIBPLAYER_H_INCLUDED
#define LIBPLAYER_H_INCLUDED
#include "libPlayer.cpp"
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



int get_msg(string &msg);
int get_file_msg(string &msg, string &file_out);

string rm_spaces(const string& str);

int case_terminal(string &buffer);

int check_active_game(string &sendmsg);
int add_args(string &msg, int code);



#endif