/*
	mv2mariadb - convert MediathekView db to mariadb
	Copyright (C) 2015-2017, M. Liebmann 'micha-bbg'

	License: GPL

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	General Public License for more details.

	You should have received a copy of the GNU General Public
	License along with this program; if not, write to the
	Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
	Boston, MA  02110-1301, USA.
*/

#ifndef __MV2MYSQL_H__
#define __MV2MYSQL_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <string>

#include <mysql.h>

#include "db.h"
#include "helpers.h"

using namespace std;

typedef struct VideoEntry
{
	string channel;
	string theme;
	string title;
	int    duration;
	int    size_mb;
	string description;
	string url;
	string website;
	string subtitle;
	string url_rtmp;
	string url_small;
	string url_rtmp_small;
	string url_hd;
	string url_rtmp_hd;
	int    date_unix;
	string url_history;
	string geo;
	string new_entry;
} TVideoEntry;

typedef struct VideoInfoEntry
{
	string channel;
	int    count;
	int    lastest;
	int    oldest;
} TVideoInfoEntry;

class CMV2Mysql
{
	private:
		const char* progName;
		const char* progCopyright;
		const char* progVersion;

		string jsondb;
		string jsonBuf;
		int epoch;
		bool epochStd;
		bool debugPrint;
		string sqlUser;
		string sqlPW;

		string mvVersion;

		vector<TVideoInfoEntry> videoInfo;

		MYSQL *mysqlCon;

		void printHeader();
		void printCopyright();
		void printHelp();
		void convertDB(string db);
		bool openDB(string db);
		bool parseDB(string db);
		string convertUrl(string url1, string url2);

		void show_error();
		bool connectMysql();
		string createVideoTableQuery(int count, bool startRow, TVideoEntry* videoEntry);
		string createInfoTableQuery(int size);
		bool copyDB();
		bool executeMultiQueryString(string query);
		bool createVideoDB_fromTemplate(string name);
		char checkStringBuff[0xFFFF];
		inline string checkString(string& str, int size) {
			size_t size_ = ((size_t)size > (sizeof(checkStringBuff)-1)) ? sizeof(checkStringBuff)-1 : size;
			string str2 = (str.length() > size_) ? str.substr(0, size_) : str;
//			memset(checkStringBuff, 0, sizeof(checkStringBuff));
			memset(checkStringBuff, 0, size_+1);
			mysql_real_escape_string(mysqlCon, checkStringBuff, str2.c_str(), str2.length());
			str = (string)checkStringBuff;
			return "'" + str + "'";
		}
		inline string checkInt(int i) { return to_string(i); }

	public:
		CMV2Mysql();
		~CMV2Mysql();
		static CMV2Mysql* getInstance();
		int run(int argc, char *argv[]);
};



#endif // __MV2MYSQL_H__
