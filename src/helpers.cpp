
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <iostream>
#include <sstream>

#include "helpers.h"

time_t duration2time(string t)
{
	struct tm tm;
	memset(&tm, 0, sizeof(tm));
	strptime(("1970-01-01 " + t).c_str(), "%F %T", &tm);
	time_t ret = mktime(&tm) + 3600;

	return ret;
}

time_t str2time(string format, string t)
{
	struct tm tm;
	memset(&tm, 0, sizeof(tm));
	strptime(t.c_str(), format.c_str(), &tm);
	time_t ret = mktime(&tm) + 3600;

//printf("\n>>> sec: %d, min: %d, hour: %d, mday: %d, mon: %d, year: %d, wday: %d, yday: %d, isdst: %d\n", tm.tm_sec, tm.tm_min, tm.tm_hour, tm.tm_mday, tm.tm_mon, tm.tm_year, tm.tm_wday, tm.tm_yday, tm.tm_isdst);
//printf(">>> %ld\n", ret);

	return ret;
}

string trim(string &str, const string &trimChars /*= " \n\r\t"*/)
{
	string result = str.erase(str.find_last_not_of(trimChars) + 1);
	return result.erase(0, result.find_first_not_of(trimChars));
}

vector<string> split(const string &s, char delim)
{
	vector<string> vec;
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim))
		vec.push_back(item);
	return vec;
}

string to_string(int i)
{
	stringstream s;
	s << i;
	return s.str();
}

string to_string(unsigned int i)
{
	stringstream s;
	s << i;
	return s.str();
}

string to_string(long i)
{
	stringstream s;
	s << i;
	return s.str();
}

string to_string(unsigned long i)
{
	stringstream s;
	s << i;
	return s.str();
}

string to_string(long long i)
{
	stringstream s;
	s << i;
	return s.str();
}

string to_string(unsigned long long i)
{
	stringstream s;
	s << i;
	return s.str();
}

string& str_replace(const string &search, const string &replace, string &text)
{
	if (search.empty() || text.empty())
		return text;

	size_t searchLen = search.length();
	while (1) {
		size_t pos = text.find(search);
		if (pos == string::npos)
			break;
		text.replace(pos, searchLen, replace);
	}
	return text;
}

string _getPathName(string &path, string sep)
{
	size_t pos = path.find_last_of(sep);
	if (pos == string::npos)
		return path;
	return path.substr(0, pos);
}

string _getBaseName(string &path, string sep)
{
	size_t pos = path.find_last_of(sep);
	if (pos == string::npos)
		return path;
	if (path.length() == pos +1)
		return "";
	return path.substr(pos+1);
}

string getPathName(string &path)
{
	return _getPathName(path, "/");
}

string getBaseName(string &path)
{
	return _getBaseName(path, "/");
}

string getFileName(string &file)
{
	return _getPathName(file, ".");
}

string getFileExt(string &file)
{
	return _getBaseName(file, ".");
}

string getRealPath(string &path)
{
	char buf[PATH_MAX];
	return (string)realpath(path.c_str(), buf);
}


off_t file_size(const char *filename)
{
	struct stat stat_buf;
	if(::stat(filename, &stat_buf) == 0)
	{
		return stat_buf.st_size;
	} else
	{
		return 0;
	}
}

bool file_exists(const char *filename)
{
	struct stat stat_buf;
	if(::stat(filename, &stat_buf) == 0)
	{
		return true;
	} else
	{
		return false;
	}
}

