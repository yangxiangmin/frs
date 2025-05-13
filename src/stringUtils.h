#ifndef __UDC_UTILS_H__
#define __UDC_UTILS_H__
#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
using namespace std;

#include "json/json.h"  // JSONCPP
using namespace Json;


int lockFile(string fileName);
void unlockFile(int fd);

void generate_token(const char *seed, char *output);
string md5(string str);


string trim(string str);
string ltrim(string str);
string rtrim(string str);

string int2str(int i);
int str2int(string s);

void split(std::string src, std::string token, vector<std::string>& vect);
void split_ex(std::string src, std::string token, vector<std::string>& vect);

string& replace_all(string& str,const string& old_value,const string& new_value);
void dump_hex(const char *title, char *buff, int len);
string Replace(const char * data);

string str2Upper(string str);
string str2Lower(string str);

bool strIsDigit(string str);

int string2json(string str, Value& data);

string GetCurrentDTstring();
string GetCurrentDTstring2();

string gbk_utf8(string& strdata);
string utf8_gbk(string& strdata);

string code_convert(const char *from_charset, const char *to_charset, const string& sourceStr);

string calculate_file_md5(string file_path);
string cmd_system(const char* command);

int havBackQuote(const char * data);
string ReplaceBackQuote2DoubleQuote(const char * data);
string ReplaceDoubleQuote2BackQuote(const char * data);

bool IsIpAddress(const char* address);

string json_format_string(Json::Value root, bool is_format);
string json_unformat_string(Json::Value data);

#endif /*  __UDC_UTILS_H__ */
