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

#include "helpers.h"
#include "configfile.h"

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

struct GSettings
{
	string videoDbBaseName;
	string videoDb;
	string videoDbTmp1;
	string videoDbTemplate;
	string videoDb_TableVideo;
	string videoDb_TableInfo;
	string videoDb_TableVersion;

	string testLabel;
	bool   testMode;
};

class CSql;

class CMV2Mysql
{
	private:
		bool multiQuery;
		CSql* csql;

		string jsondb;
		string jsonBuf;
		string workDir;
		int epoch;
		bool epochStd;
		string configFileName;
		CConfigFile configFile;

		string VIDEO_DB_TMP_1;

		void printHeader();
		void printCopyright();
		void printHelp();
		void convertDB(string db);
		bool openDB(string db);
		bool parseDB(string db);
		string convertUrl(string url1, string url2);

		int loadSetup(string fname);
		void saveSetup(string fname);
		void setDbFileNames(string xz);

	public:
		CMV2Mysql();
		~CMV2Mysql();
		static CMV2Mysql* getInstance();
		int run(int argc, char *argv[]);
};



#endif // __MV2MYSQL_H__
