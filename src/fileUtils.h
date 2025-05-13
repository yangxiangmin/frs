#ifndef __FILE_UTILES_H__
#define __FILE_UTILES_H__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h> 
#include <unistd.h> 
#include <string>  
#include <fstream>  
#include <sstream>  
#include <vector>  
#include <string>  
#include <iostream>
using namespace std;

// load file's content into buffer 
int get_file_contents(const char *file, char *data, unsigned long len);

// save content into file
int put_file_contents(const char *file, const char *data, unsigned long len);


// get size of file content
unsigned long get_file_size(const char *file);

// check if file exist
bool is_file_exist(const char *file);  
bool file_exists(const string& name);

int file_rename(string oldName, string newName);

bool make_dir(const char *folder);

string get_file_dir(string full_path_file);
string get_file_name(string full_path_file);  
string get_file_ext(string file);

int write_to_file(string file, string str_write);
string read_from_file(string file);
int write_to_binary_file(string file, const char* data, size_t length);

// get app owner path
string get_owner_path();

int append_to_file(string file, const char* str_write);
int append_to_file2(string file, const char* str_write, size_t length);

int get_dir_list(string dirpath, vector<string>& list);
int get_files_list(string dirpath, vector<string>& dirlist);
int get_all_files(string dirpath, vector<string>& dirlist);

void make_dir_multi_level(const char *folder);


#endif /* __FILE_UTILES_H__ */
