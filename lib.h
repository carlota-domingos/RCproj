#ifndef LIB_H_INCLUDED
#define LIB_H_INCLUDED
#include "lib.cpp"
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
//server
void create_directories();
void create_game_dir(const string &plid);
ofstream create_file(const string& directory, const string& filename);

int get_msg(string &msg);//player
int get_file_msg(string &msg, string &file_out);//player

//server
void generate_random_colors(char *result);
int code_val(const string& code);
bool valid_time(const string& str);
string rm_spaces(const string& str);//player


int case_terminal(string &buffer);//player


#endif
