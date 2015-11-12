#include <iostream>
//#include <algorithm>
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
