
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sstream>

#include "helpers.h"

extern void myExit(int val);

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

/*
 * ported from:
 * https://stackoverflow.com/questions/779875/what-is-the-function-to-replace-string-in-c
 * 
 * You must delete the result if result is non-NULL
 */
const char *cstr_replace(const char *search, const char *replace, const char *text)
{
	const char *result;	// the return string
	const char *ins;	// the next insert point
	char *tmp;		// varies
	int len_search;		// length of search (the string to remove)
	int len_replace;	// length of replace (the string to replace search with)
	int len_front;		// distance between search and end of last search
	int count;		// number of replacements

	// sanity checks and initialization
	if (!text || !search)
		return NULL;
	len_search = strlen(search);
	if (len_search == 0)
		return NULL; // empty search causes infinite loop during count
	if (!replace)
		replace = "";
	len_replace = strlen(replace);

	// count the number of replacements needed
	ins = text;
	for (count = 0; (tmp = (char*)strstr(ins, search)); ++count)
		ins = tmp + len_search;

	int len_tmp = strlen(text) + (len_replace - len_search) * count + 1;
	tmp = new char[len_tmp];
	memset(tmp, '\0', len_tmp);
	result = (const char*)tmp;

	if (!result)
		return NULL;

	// first time through the loop, all the variable are set correctly
	// from here on,
	//    tmp points to the end of the result string
	//    ins points to the next occurrence of search in text
	//    text points to the remainder of text after "end of search"
	while (count--) {
		ins = strstr(text, search);
		len_front = ins - text;
		tmp = strncpy(tmp, text, len_front) + len_front;
		tmp = strncpy(tmp, replace, len_replace) + len_replace;
		text += len_front + len_search; // move to next "end of search"
	}
	strncpy(tmp, text, strlen(text));
	return result;
}

string str_tolower(string s)
{
	::transform(s.begin(), s.end(), s.begin(), static_cast<int(*)(int)>(::tolower));
	return s;
}

string str_toupper(string s)
{
	::transform(s.begin(), s.end(), s.begin(), static_cast<int(*)(int)>(::toupper));
	return s;
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
	const char* ret = realpath(path.c_str(), buf);
	if (ret == NULL) {
		printf("[%s] Error: path %s not exists.\n", __func__, path.c_str());
		myExit(1);
	}
	return (string)ret;
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

