
#ifndef __helpers_h__
#define __helpers_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include <string>
#include <vector>

using namespace std;

time_t duration2time(string t);
time_t str2time(string format, string t);

string trim(string &str, const string &trimChars = " \n\r\t");

vector<string> split(const string &s, char delim);

string to_string(int);
string to_string(unsigned int);
string to_string(long);
string to_string(unsigned long);
string to_string(long long);
string to_string(unsigned long long);

string& str_replace(const string &search, const string &replace, string &text);

string getPathName(string &path);
string getBaseName(string &path);
string getFileName(string &file);
string getFileExt(string &file);
string getRealPath(string &path);

off_t file_size(const char *filename);
bool file_exists(const char *filename);

#endif // __helpers_h__
