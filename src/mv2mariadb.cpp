/*
	mv2mariadb - convert MediathekView db to mysql
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

#define PROGVERSION "0.3.0"
#define DBVERSION "3.0"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>

#include <json/json.h>
#include <mysqld_error.h>

#include <iostream>
#include <fstream>
#include <climits>

#include "mv2mariadb.h"
#include "helpers.h"
#include "filehelpers.h"

#define DB_1 "-1.json"
#define DB_2 "-2.json"

CMV2Mysql::CMV2Mysql()
: configFile('\t')
{
	progName	= "mv2mariadb";
	progCopyright	= "Copyright (C) 2015-2017, M. Liebmann 'micha-bbg'";
	progVersion	= "v"PROGVERSION;

	epoch  = 180; /* 1/2 year*/
	epochStd = false;
	debugPrint = false;

	mysqlCon = NULL;
}

CMV2Mysql::~CMV2Mysql()
{
	videoInfo.clear();
	if (mysqlCon != NULL) {
		mysql_close(mysqlCon);
	}
}

CMV2Mysql* CMV2Mysql::getInstance()
{
	static CMV2Mysql* instance = NULL;
	if (!instance)
		instance = new CMV2Mysql();
	return instance;
}

void CMV2Mysql::printHeader()
{
	printf("%s %s\n", progName, progVersion);
}

void CMV2Mysql::printCopyright()
{
	printf("%s\n", progCopyright);
}

void CMV2Mysql::printHelp()
{
	printHeader();
	printCopyright();
	printf("	-f | --file	   => Json file to parse\n");
	printf("	-e | --epoch	   => Use not older entrys than 'epoch' days\n");
	printf("			      (default 180 days)\n");
	printf("	-s | --epoch-std   => Value of 'epoch' in hours (for debugging)\n");
	printf("	-d | --debug-print => Print debug info\n");
	printf("	-h | --help	   => Display this help screen and exit\n");
	printf("	-v | --version	   => Display versions info and exit\n");
}

int CMV2Mysql::loadSetup(string fname)
{
	int erg = 0;
	if (!configFile.loadConfig(fname.c_str()))
		/* file not exist */
		erg = 1;

	g_settings.testLabel		= configFile.getString("testLabel",            "_TEST");
	g_settings.testMode		= configFile.getBool  ("testMode",             true);

	g_settings.videoDbBaseName	= configFile.getString("videoDbBaseName",      "mediathek_1");
	g_settings.videoDb		= configFile.getString("videoDb",              g_settings.videoDbBaseName);
	g_settings.videoDbTmp1		= configFile.getString("videoDbTmp1",          g_settings.videoDbBaseName + "_tmp1");
	g_settings.videoDbTemplate	= configFile.getString("videoDbTemplate",      g_settings.videoDbBaseName + "_template");
	g_settings.videoDb_TableVideo	= configFile.getString("videoDb_TableVideo",   "video");
	g_settings.videoDb_TableInfo	= configFile.getString("videoDb_TableInfo",    "channelinfo");
	g_settings.videoDb_TableVersion	= configFile.getString("videoDb_TableVersion", "version");


	VIDEO_DB			= g_settings.videoDb;
	VIDEO_DB_TMP_1			= g_settings.videoDbTmp1;
	VIDEO_DB_TEMPLATE		= g_settings.videoDbTemplate;
	if (g_settings.testMode) {
		VIDEO_DB		+= g_settings.testLabel;
		VIDEO_DB_TMP_1		+= g_settings.testLabel;
		VIDEO_DB_TEMPLATE	+= g_settings.testLabel;
	}
	VIDEO_TABLE			= g_settings.videoDb_TableVideo;
	INFO_TABLE			= g_settings.videoDb_TableInfo;
	VERSION_TABLE			= g_settings.videoDb_TableVersion;

	if (erg)
		configFile.setModifiedFlag(true);
	return erg;
}

void CMV2Mysql::saveSetup(string fname)
{
	configFile.setString("testLabel",            g_settings.testLabel);
	configFile.setBool  ("testMode",             g_settings.testMode);

	configFile.setString("videoDbBaseName",      g_settings.videoDbBaseName);
	configFile.setString("videoDb",              g_settings.videoDb);
	configFile.setString("videoDbTmp1",          g_settings.videoDbTmp1);
	configFile.setString("videoDbTemplate",      g_settings.videoDbTemplate);
	configFile.setString("videoDb_TableVideo",   g_settings.videoDb_TableVideo);
	configFile.setString("videoDb_TableInfo",    g_settings.videoDb_TableInfo);
	configFile.setString("videoDb_TableVersion", g_settings.videoDb_TableVersion);

	if (configFile.getModifiedFlag())
		configFile.saveConfig(fname.c_str());
}

int CMV2Mysql::run(int argc, char *argv[])
{
	if (argc < 2) {
		printHeader();
		printf("\tType '%s --help' to print help screen\n", progName);
		return 0;
	}

	/* set name for configFileName */
	string arg0         = (string)argv[0];
	string path0        = getPathName(arg0);
	configFileName      = getRealPath(path0) + "/" + getBaseName(arg0)+ ".conf";

	int loadSettingsErg = loadSetup(configFileName);

	if (loadSettingsErg) {
		configFile.setModifiedFlag(true);
		saveSetup(configFileName);
	}

	int noParam       = 0;
	int requiredParam = 1;
//	int optionalParam = 2;
	static struct option long_options[] = {
		{"help",	noParam,       NULL, 'h'},
		{"version",	noParam,       NULL, 'v'},
		{"file",	requiredParam, NULL, 'f'},
		{"epoch",	requiredParam, NULL, 'e'},
		{"epoch-std",   noParam,       NULL, 's'},
		{"debug-print", noParam,       NULL, 'd'},
		{NULL,		0, NULL, 0}
	};
	int c, opt;
	while ((opt = getopt_long(argc, argv, "h?vf:e:sd", long_options, &c)) >= 0) {
		switch (opt) {
			case 'h':
			case '?':
				printHelp();
				return (opt == '?') ? -1 : 0;
			case 'v':
				printHeader();
				printCopyright();
				return 0;
			case 'f':
				jsondb = string(optarg);
				break;
			case 'e':
				epoch = atoi(optarg);
				break;
			case 's':
				epochStd = true;
				break;
			case 'd':
				debugPrint = true;
				break;
			default:
				break;
		}
	}

	if (!openDB(jsondb))
		return 1;

	connectMysql();
	parseDB(jsondb);

	return 0;
}

void CMV2Mysql::convertDB(string db)
{
	FILE* f1 = fopen(db.c_str(), "r");
	if (f1 == NULL) {
		printf("#### [%s:%d] error opening db file: %s\n", __func__, __LINE__, db.c_str());
		return;
	}
	FILE* f2 = fopen((db + DB_1).c_str(), "w+");

	printf("[%s] convert json db...", progName); fflush(stdout);

	char buf[8192];
	string line;
	string listSearch = "\"Filmliste\":";
	string xSearch    = "\"X\":";

	int c = 32;
	while (c != EOF) {
		int count = 0;
		memset(buf, 0, sizeof(buf));
		c = 32;
		while ((c != ',') && (c != EOF)) {
			c = fgetc(f1);
			if (c != EOF)
				buf[count] = c;
			count++;
		}

		string tmpLine;
		line = buf;
		size_t pos = line.find(listSearch);
		if (pos != string::npos) {
			tmpLine = "\n\"Filmliste\":";
			line.replace(pos, listSearch.length(), tmpLine);
		}
		pos = line.find(xSearch);
		if (pos != string::npos) {
			tmpLine = "\n\"X\":";
			line.replace(pos, xSearch.length(), tmpLine);
		}

		fputs(line.c_str(), f2);
	}

	fclose(f1);
	fputs("\n", f2);
	fclose(f2);

	printf("done.\n"); fflush(stdout);
}

bool CMV2Mysql::openDB(string db)
{
	convertDB(db);

	FILE* f = fopen((db + DB_1).c_str(), "r");
	if (f == NULL) {
		printf("#### [%s:%d] error opening db file: %s\n", __func__, __LINE__, db.c_str());
		return false;
	}

	printf("[%s] read json db...", progName); fflush(stdout);
	fseek(f, 0, SEEK_END);
	size_t fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	jsonBuf = "";
	char buf[0xFFFF];
	string line;
	string listSearch = "\"Filmliste\"";
	string xSearch    = "\"X\"";
	int count = 1;
	while(fgets(buf, sizeof(buf), f) != NULL) {
		string tmpLine;
		line = buf;
		line = trim(line);
		if (line.find(listSearch) == 0) {
			tmpLine = "{\"Filmliste\"";
			line.replace(0, listSearch.length(), tmpLine);
			size_t pos = line.find_last_of("]");
			line.replace(pos, 1, "]}");
			count++;
		}
		else if (line.find(xSearch) == 0) {
			tmpLine = "{\"X\"";
			line.replace(0, xSearch.length(), tmpLine);
			size_t pos = line.find_last_of("]");
			line.replace(pos, 1, "]}");
			count++;
		}
		jsonBuf += line + "\n";
	}	
	size_t lPos = jsonBuf.find_first_of("{");
	jsonBuf.replace(lPos, 1, "[");
	lPos = jsonBuf.find_last_of("}");
	jsonBuf.replace(lPos, 1, "\n]");

	fclose(f);

	f = fopen((db + DB_2).c_str(), "w+");
	fwrite(jsonBuf.c_str(), jsonBuf.length(), 1, f);
	fclose(f);
	jsonBuf.clear();

	printf("done (%u Bytes)\n", (uint32_t)fsize); fflush(stdout);

	return true;
}

bool CMV2Mysql::parseDB(string db)
{
	printf("[%s] parse json db & write temporary database...", progName); fflush(stdout);

	Json::Value root;
	Json::Reader reader;
	bool parsedSuccess = false;

	ifstream jsonData((db + DB_2).c_str(), ifstream::binary);
	if (!jsonData.is_open()) {
		printf("\n[%s:%d] Failed to open json db %s\n", __func__, __LINE__, (db + DB_2).c_str());
		return false;
	}

	parsedSuccess = reader.parse(jsonData, root, false);
	jsonData.close();

	if(!parsedSuccess) {
		printf("\nFailed to parse JSON\n");
		printf("[%s:%d] %s\n", __func__, __LINE__, reader.getFormattedErrorMessages().c_str());
		return false;
	}

	Json::Value data;

	string cName = "";
	string tName = "";
	int cCount = 0;
	uint32_t entrys = 0;
	TVideoInfoEntry videoInfoEntry;
	videoInfoEntry.lastest = INT_MIN;
	videoInfoEntry.oldest = INT_MAX;
	time_t nowTime = time(NULL);
	struct timeval t1;
	double nowDTms;
	string vMultiQuery = "";
	string vQuery = "";
	uint32_t writeLen = 0;
	bool writeStart = true;

	createVideoDB_fromTemplate(VIDEO_DB_TMP_1);

	string sqlBuff = "";
	if (mysql_set_server_option(mysqlCon, MYSQL_OPTION_MULTI_STATEMENTS_OFF) != 0)
		show_error();

	gettimeofday(&t1, NULL);
	nowDTms = (double)t1.tv_sec*1000ULL + ((double)t1.tv_usec)/1000ULL;
	if (debugPrint) {
		printf("\e[?25l"); /* cursor off */
		printf("\n");
	}

	for (unsigned int i = 0; i < root.size(); ++i) {

		if (i == 0) {		/* head 1 */
			data = root[i].get("Filmliste", "");
			mvVersion = data[3].asString();
		}
		else if (i == 1) {	/* head 2 */
			data = root[i].get("Filmliste", "");
		}
		else {			/* data   */
			data = root[i].get("X", "");
			TVideoEntry videoEntry;
			videoEntry.channel		= data[0].asString();
			if ((videoEntry.channel != "") && (videoEntry.channel != cName)) {
				if (cName != "") {
					videoInfo.push_back(videoInfoEntry);
				}
				cName = videoEntry.channel;
				cCount = 0;
				videoInfoEntry.lastest = INT_MIN;
				videoInfoEntry.oldest = INT_MAX;
			}

			videoEntry.theme		= data[1].asString();
			if (videoEntry.theme != "") {
				tName = videoEntry.theme;
			}
			else
				 videoEntry.theme	= tName;

			videoEntry.title		= data[2].asString();
			videoEntry.duration		= duration2time(data[5].asString());
			videoEntry.size_mb		= atoi(data[6].asCString());
			videoEntry.description		= data[7].asString();
			videoEntry.url			= data[8].asString();
			videoEntry.website		= data[9].asString();
			videoEntry.subtitle		= data[10].asString();
			videoEntry.url_rtmp		= convertUrl(videoEntry.url, data[11].asString());
			videoEntry.url_small		= convertUrl(videoEntry.url, data[12].asString());
			videoEntry.url_rtmp_small	= convertUrl(videoEntry.url, data[13].asString());
			videoEntry.url_hd		= convertUrl(videoEntry.url, data[14].asString());
			videoEntry.url_rtmp_hd		= convertUrl(videoEntry.url, data[15].asString());

			videoEntry.date_unix		= atoi((data[16].asCString()));
			if ((videoEntry.date_unix == 0) && (data[3].asString() != "") && (data[4].asString() != "")) {
				videoEntry.date_unix = str2time("%d.%m.%Y %H:%M:%S", data[3].asString() + " " + data[4].asString());
			}
			if (videoEntry.date_unix > 0) {
				time_t maxDiff = (24*3600) * epoch; /* Not older than 'epoch' days (default 180) */
				if (epochStd)
					maxDiff /= 24;
				if (videoEntry.date_unix < (nowTime - maxDiff))
					continue;
			}

			videoEntry.url_history		= data[17].asString();
			videoEntry.geo			= data[18].asString();
			videoEntry.new_entry		= data[19].asString();

			videoEntry.channel		= cName;
			cCount++;
			videoInfoEntry.channel		= cName;
			videoInfoEntry.count		= cCount;
			videoInfoEntry.lastest		= max(videoEntry.date_unix, videoInfoEntry.lastest);
			if (videoEntry.date_unix != 0)
				videoInfoEntry.oldest	= min(videoEntry.date_unix, videoInfoEntry.oldest);

			entrys++;
			if (debugPrint) {
				if ((entrys % 32) == 0)
					printf("[%s-debug] Processed entries: %d\r", progName, entrys);
				if ((entrys % 32*8) == 0)
					fflush(stdout);
			}
			vQuery = createVideoTableQuery(entrys, writeStart, &videoEntry);
			writeStart = false;

			uint32_t maxWriteLen = 1048576;		/* 1MB */
			if ((writeLen + vQuery.length()) >= maxWriteLen) {
				sqlBuff += ";\n";
				if (mysql_real_query(mysqlCon, sqlBuff.c_str(), sqlBuff.length()) != 0)
					show_error();
				vQuery = createVideoTableQuery(entrys, true, &videoEntry);
				sqlBuff = "";
				writeLen = 0;
			}
			sqlBuff += vQuery;
			writeLen = sqlBuff.length();
		}
	}

	if (!sqlBuff.empty()) {
		if (mysql_real_query(mysqlCon, sqlBuff.c_str(), sqlBuff.length()) != 0)
			show_error();
		sqlBuff.clear();
	}
	if (mysql_set_server_option(mysqlCon, MYSQL_OPTION_MULTI_STATEMENTS_ON) != 0)
		show_error();
	if (debugPrint) {
		printf("\e[?25h"); /* cursor on */
		printf("\n");
	}

	videoInfo.push_back(videoInfoEntry);
	string itq = createInfoTableQuery(entrys);
	executeMultiQueryString(itq);

	if (!debugPrint)
		printf("\n");

	if (entrys < 1000) {
		printf("\n[%s] Video list too small (%d entrys), no transfer to the database.\n", progName, entrys); fflush(stdout);
		return false;
	}

	copyDB();

	printf("[%s] done (%u (%d %s) / %u entrys)\n", progName, entrys, epoch, (epochStd)?"std":"days", (uint32_t)(root.size()-2));
	gettimeofday(&t1, NULL);
	double workDTms = (double)t1.tv_sec*1000ULL + ((double)t1.tv_usec)/1000ULL;
	double workDTus = (double)t1.tv_sec*1000000ULL + ((double)t1.tv_usec);
	int32_t workTime = (int32_t)((workDTms - nowDTms) / 1000);
	double entryTime_us = (workDTus - nowDTms*1000) / entrys;
	printf("[%s] duration: %d sec (%.03f msec/entry)\n", progName, workTime, entryTime_us/1000);
	fflush(stdout);

	return true;
}

string CMV2Mysql::convertUrl(string url1, string url2)
{
	/* format url_small / url_rtmp_small etc:
		55|xxx.yyy
		 |    |
		 |    ----------  replace string
		 ---------------  replace pos in videoEntry.url (url1) */

	string ret = "";
	size_t pos = url2.find_first_of("|");
	if (pos != string::npos) {
		int pos1 = atoi(url2.substr(0, pos).c_str());
		ret = url1.substr(0, pos1) + url2.substr(pos+1);
	}
	else
		ret = url2;

	return ret;
}

void CMV2Mysql::show_error()
{
	printf("\nError(%d) [%s] \"%s\"\n", mysql_errno(mysqlCon),
					    mysql_sqlstate(mysqlCon),
					    mysql_error(mysqlCon));
	mysql_close(mysqlCon);
	mysqlCon = NULL;
	exit(-1);
}

bool CMV2Mysql::connectMysql()
{
	FILE* f = fopen("pw_conv.txt", "r");
	if (f == NULL) {
		printf("#### [%s:%d] error opening pw file: %s\n", __func__, __LINE__, "pw_conv.txt");
		return false;
	}
	char buf[256];
	fgets(buf, sizeof(buf), f);
	fclose(f);
	string pw = buf;
	vector<string> v = split(pw, ':');
	sqlUser = v[0];
	sqlPW   = v[1];

	mysqlCon = mysql_init(NULL);

	int maxAllowedPacketDef = 4194304;			// default
	int maxAllowedPacket = maxAllowedPacketDef*64;
//	int maxAllowedPacket = maxAllowedPacketDef*256;		// max value
	if (mysql_optionsv(mysqlCon, MYSQL_OPT_MAX_ALLOWED_PACKET, (const void*)(&maxAllowedPacket)) != 0)
		show_error();

	unsigned long flags = 0;
	flags |= CLIENT_MULTI_STATEMENTS;
//	flags |= CLIENT_COMPRESS;
	if (!mysql_real_connect(mysqlCon, "127.0.0.1", sqlUser.c_str(), sqlPW.c_str(), NULL, 3306, NULL, flags))
		show_error();

	if (mysql_set_character_set(mysqlCon, "utf8") != 0)
		show_error();

	return true;
}

bool CMV2Mysql::executeMultiQueryString(string query)
{
	int status = mysql_real_query(mysqlCon, query.c_str(), query.length());
	if (status)
		show_error();
	
	bool ret = true;
	/* process each statement result */
	do {
		/* did current statement return data? */
		MYSQL_RES *result = mysql_store_result(mysqlCon);
		if (result) {
			/* yes; free the result set */
			mysql_free_result(result);
		}
		else {	/* no result set or error */
			if (mysql_field_count(mysqlCon) != 0) {
				/* some error occurred */
				printf("Could not retrieve result set\n");
				ret = false;
				break;
			}
		}
		/* more results? -1 = no, >0 = error, 0 = yes (keep looping) */
		if ((status = mysql_next_result(mysqlCon)) > 0) {
			printf("Could not execute statement\n");
			ret = false;
		}
	} while (status == 0);

	return ret;
}

bool CMV2Mysql::createVideoDB_fromTemplate(string name)
{
	string query = "DROP DATABASE IF EXISTS `" + name +"`;";
	query += "CREATE DATABASE IF NOT EXISTS `" + name + "` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;";
	query += "CREATE TABLE " + name + "." + VIDEO_TABLE + " LIKE " + VIDEO_DB_TEMPLATE + "." + VIDEO_TABLE + ";";
	query += "CREATE TABLE " + name + "." + INFO_TABLE + " LIKE " + VIDEO_DB_TEMPLATE + "." + INFO_TABLE + ";";
	query += "CREATE TABLE " + name + "." + VERSION_TABLE + " LIKE " + VIDEO_DB_TEMPLATE + "." + VERSION_TABLE + ";";
	query += "USE `" + name + "`;";

	return executeMultiQueryString(query);
}

string CMV2Mysql::createVideoTableQuery(int count, bool startRow, TVideoEntry* videoEntry)
{
	string entry = "";
	if (startRow) {
		entry += "INSERT INTO " + VIDEO_TABLE + " VALUES ";
	}
	else {
		entry += ",";
	}
	entry += "(";
	entry += checkInt(count) + ",";
	entry += checkString(videoEntry->channel, 256) + ",";
	entry += checkString(videoEntry->theme, 256) + ",";
	entry += checkString(videoEntry->title, 1024) + ",";
	entry += checkInt(videoEntry->duration) + ",";
	entry += checkInt(videoEntry->size_mb) + ",";
	entry += checkString(videoEntry->description, 8192) + ",";
	entry += checkString(videoEntry->url, 1024) + ",";
	entry += checkString(videoEntry->website, 1024) + ",";
	entry += checkString(videoEntry->subtitle, 1024) + ",";
	entry += checkString(videoEntry->url_rtmp, 1024) + ",";
	entry += checkString(videoEntry->url_small, 1024) + ",";
	entry += checkString(videoEntry->url_rtmp_small, 1024) + ",";
	entry += checkString(videoEntry->url_hd, 1024) + ",";
	entry += checkString(videoEntry->url_rtmp_hd, 1024) + ",";
	entry += checkInt(videoEntry->date_unix) + ",";
	entry += checkString(videoEntry->url_history, 1024) + ",";
	entry += checkString(videoEntry->geo, 1024) + ",";
	entry += checkInt(0) + ",";
	entry += checkString(videoEntry->new_entry, 1024);
	entry += ")";

	return entry;
}

string CMV2Mysql::createInfoTableQuery(int size)
{
	string entry = "";
	for (size_t i = 0; i < videoInfo.size(); ++i) {
		entry += "INSERT INTO " + INFO_TABLE + " (channel, count, lastest, oldest) VALUES (";
		entry += checkString(videoInfo[i].channel, 256) + ", ";
		entry += checkInt(videoInfo[i].count) + ", ";
		entry += checkInt(videoInfo[i].lastest) + ", ";
		entry += checkInt(videoInfo[i].oldest);
		entry += ");";
	}
	videoInfo.clear();

	struct stat st;
	stat(jsondb.c_str(), &st);

	entry += "INSERT INTO " + VERSION_TABLE + " (version, vdate, mvversion, mvdate, mventrys) VALUES (";
	string tmpStr = (string)DBVERSION;
	entry += checkString(tmpStr, 256) + ", ";
	entry += checkInt(time(0)) + ", ";
	entry += checkString(mvVersion, 256) + ", ";
	entry += checkInt(st.st_mtime) + ", ";
	entry += checkInt(size);
	entry += ");";

	return entry;
}

bool CMV2Mysql::copyDB()
{
	time_t startTime = time(0);
	printf("[%s] copy database...", progName); fflush(stdout);

	createVideoDB_fromTemplate(VIDEO_DB);
	string query = "";
	query += "INSERT INTO " + VIDEO_DB + "." + VIDEO_TABLE + " SELECT * FROM " + VIDEO_DB_TMP_1 + "." + VIDEO_TABLE + ";";
	query += "INSERT INTO " + VIDEO_DB + "." + INFO_TABLE + " SELECT * FROM " + VIDEO_DB_TMP_1 + "." + INFO_TABLE + ";";
	query += "INSERT INTO " + VIDEO_DB + "." + VERSION_TABLE + " SELECT * FROM " + VIDEO_DB_TMP_1 + "." + VERSION_TABLE + ";";
	bool ret = executeMultiQueryString(query);

	time_t endTime = time(0);
	printf("done (%ld sec)\n", endTime-startTime); fflush(stdout);
	
	return ret;
}

int main(int argc, char *argv[])
{
	return CMV2Mysql::getInstance()->run(argc, argv);
}
