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

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>

#include <mysqld_error.h>

#include <iostream>
#include <fstream>
#include <climits>

#include "sql.h"
#include "helpers.h"
#include "filehelpers.h"

extern GSettings		g_settings;
extern const char*		g_progName;
extern const char*		g_progCopyright;
extern const char*		g_progVersion;
extern const char*		g_dbVersion;
extern string			g_jsondb;
extern string			g_templateDBFile;
extern string			g_mvVersion;
extern bool			g_debugPrint;
extern vector<TVideoInfoEntry>	g_videoInfo;

CSql::CSql()
{
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

	multiQuery = true;
	mysqlCon = NULL;
}

CSql::~CSql()
{
	if (mysqlCon != NULL) {
		mysql_close(mysqlCon);
	}
}

CSql* CSql::getInstance()
{
	static CSql* instance = NULL;
	if (!instance)
		instance = new CSql();
	return instance;
}

void CSql::show_error(const char* func, int line)
{
	printf("\n[%s:%d] Error(%d) [%s] \"%s\"\n",
	       				    func, line,
					    mysql_errno(mysqlCon),
					    mysql_sqlstate(mysqlCon),
					    mysql_error(mysqlCon));
	mysql_close(mysqlCon);
	mysqlCon = NULL;
	exit(-1);
}

bool CSql::connectMysql()
{
	FILE* f = fopen("pw_conv.txt", "r");
	if (f == NULL) {
		printf("#### [%s:%d] error opening pw file: %s\n", __func__, __LINE__, "pw_conv.txt");
		exit(1);
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
		show_error(__func__, __LINE__);

	unsigned long flags = 0;
	if (multiQuery)
		flags |= CLIENT_MULTI_STATEMENTS;
//	flags |= CLIENT_COMPRESS;
	if (!mysql_real_connect(mysqlCon, "127.0.0.1", sqlUser.c_str(), sqlPW.c_str(), NULL, 3306, NULL, flags))
		show_error(__func__, __LINE__);

	if (mysql_set_character_set(mysqlCon, "utf8") != 0)
		show_error(__func__, __LINE__);

	return true;
}

string CSql::createVideoTableQuery(int count, bool startRow, TVideoEntry* videoEntry)
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

string CSql::createInfoTableQuery(int size)
{
	string entry = "";
	for (size_t i = 0; i < g_videoInfo.size(); ++i) {
		entry += "INSERT INTO " + INFO_TABLE + " (channel, count, lastest, oldest) VALUES (";
		entry += checkString(g_videoInfo[i].channel, 256) + ", ";
		entry += checkInt(g_videoInfo[i].count) + ", ";
		entry += checkInt(g_videoInfo[i].lastest) + ", ";
		entry += checkInt(g_videoInfo[i].oldest);
		entry += ");";
	}
	g_videoInfo.clear();

	struct stat st;
	stat(g_jsondb.c_str(), &st);

	entry += "INSERT INTO " + VERSION_TABLE + " (version, vdate, mvversion, mvdate, mventrys, progname, progversion) VALUES (";
	string tmpStr = (string)g_dbVersion;
	entry += checkString(tmpStr, 256) + ", ";
	entry += checkInt(time(0)) + ", ";
	entry += checkString(g_mvVersion, 256) + ", ";
	entry += checkInt(st.st_mtime) + ", ";
	entry += checkInt(size) + ", ";
	tmpStr = (string)g_progName;
	entry += checkString(tmpStr, 256) + ", ";
	tmpStr = (string)g_progVersion;
	entry += checkString(tmpStr, 256);
	entry += ");";

	return entry;
}

bool CSql::executeSingleQueryString(string query)
{
	bool ret = true;

	if (mysql_real_query(mysqlCon, query.c_str(), query.length()) != 0)
		show_error(__func__, __LINE__);

	return ret;
}

bool CSql::executeMultiQueryString(string query)
{
	if (!multiQuery) {
		printf("[%s:%d] No multiple statement execution support.\n", __func__, __LINE__);
		exit(1);
	}
	if (mysql_set_server_option(mysqlCon, MYSQL_OPTION_MULTI_STATEMENTS_ON) != 0)
		show_error(__func__, __LINE__);

	int status = mysql_real_query(mysqlCon, query.c_str(), query.length());
	if (status)
		show_error(__func__, __LINE__);
	
	bool ret = true;
	/* process each statement result */
	do {
		ret = true;
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
//			printf("Could not execute statement\n");
//			ret = false;
		}
	} while (status == 0);

	return ret;
}

bool CSql::createVideoDbFromTemplate(string name)
{
	string query = "DROP DATABASE IF EXISTS `" + name +"`;";
	query += "CREATE DATABASE IF NOT EXISTS `" + name + "` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;";
	query += "CREATE TABLE " + name + "." + VIDEO_TABLE + " LIKE " + VIDEO_DB_TEMPLATE + "." + VIDEO_TABLE + ";";
	query += "CREATE TABLE " + name + "." + INFO_TABLE + " LIKE " + VIDEO_DB_TEMPLATE + "." + INFO_TABLE + ";";
	query += "CREATE TABLE " + name + "." + VERSION_TABLE + " LIKE " + VIDEO_DB_TEMPLATE + "." + VERSION_TABLE + ";";
	query += "USE `" + name + "`;";

	return executeMultiQueryString(query);
}

void CSql::checkTemplateDB()
{
	if (multiQuery) {
		if (mysql_set_server_option(mysqlCon, MYSQL_OPTION_MULTI_STATEMENTS_OFF) != 0)
			show_error(__func__, __LINE__);
	}

	string sql = "SHOW DATABASES;";
	if (!executeSingleQueryString(sql))
		show_error(__func__, __LINE__);

	MYSQL_RES* result = mysql_store_result(mysqlCon);
	MYSQL_ROW row;
	bool dbExists = false;
	while ((row = mysql_fetch_row(result)))
	{
		if (VIDEO_DB_TEMPLATE == (string)row[0]) {
			if (g_debugPrint)
				printf("[%s-debug] check i.o., database [%s] exists.\n", g_progName, VIDEO_DB_TEMPLATE.c_str());
			
			dbExists = true;
			break;
		}
	}
	if (!dbExists) {
		bool ret = createTemplateDB(true);
		if (g_debugPrint && ret)
			printf("[%s-debug] database [%s] successfully created.\n", g_progName, VIDEO_DB_TEMPLATE.c_str());
	}

	mysql_free_result(result);

	if (multiQuery) {
		if (mysql_set_server_option(mysqlCon, MYSQL_OPTION_MULTI_STATEMENTS_ON) != 0)
			show_error(__func__, __LINE__);
	}
}

bool CSql::createTemplateDB(bool quiet/* = false*/)
{
	size_t size = 0;
	const char* buf = NULL;
	if (file_exists(g_templateDBFile.c_str())) {
		FILE* f = fopen(g_templateDBFile.c_str(), "r");
		if (f != NULL) {
			size = file_size(g_templateDBFile.c_str());
			buf = new char[size];
			size = fread((void*)buf, size, 1, f);
			fclose(f);
		}

	}
	if (size == 0) {
		printf("\n[%s] error read database template [%s]\n", g_progName, g_templateDBFile.c_str());
		if (buf != NULL)
			delete [] buf;
		exit(1);
	}
	string sql = (string)buf;
	delete [] buf;

	string search = "@@@db_template@@@";
	sql = str_replace(search, VIDEO_DB_TEMPLATE, sql);
	search = "@@@tab_channelinfo@@@";
	sql = str_replace(search, INFO_TABLE, sql);
	search = "@@@tab_version@@@";
	sql = str_replace(search, VERSION_TABLE, sql);
	search = "@@@tab_video@@@";
	sql = str_replace(search, VIDEO_TABLE, sql);

	bool ret = executeMultiQueryString(sql);
	if (!quiet)
		printf("\n[%s] database [%s] successfully created or updated.\n", g_progName, VIDEO_DB_TEMPLATE.c_str());
	return ret;
}

bool CSql::renameDB()
{
	struct timeval t1;
	gettimeofday(&t1, NULL);
	double nowDTms = (double)t1.tv_sec*1000ULL + ((double)t1.tv_usec)/1000ULL;
	printf("[%s] rename temporary database...", g_progName); fflush(stdout);

	string query = "";
	query += "START TRANSACTION;";
	query += "SET autocommit = 0;";
	query += "DROP DATABASE IF EXISTS `" + VIDEO_DB +"`;";
	query += "CREATE DATABASE IF NOT EXISTS `" + VIDEO_DB + "` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;";
	query += "RENAME TABLE " + VIDEO_DB_TMP_1 + "." + VIDEO_TABLE + " TO " + VIDEO_DB + "." + VIDEO_TABLE + ";";
	query += "RENAME TABLE " + VIDEO_DB_TMP_1 + "." + INFO_TABLE + " TO " + VIDEO_DB + "." + INFO_TABLE + ";";
	query += "RENAME TABLE " + VIDEO_DB_TMP_1 + "." + VERSION_TABLE + " TO " + VIDEO_DB + "." + VERSION_TABLE + ";";
	query += "DROP DATABASE IF EXISTS `" + VIDEO_DB_TMP_1 +"`;";
	query += "COMMIT;";
	bool ret = executeMultiQueryString(query);

	gettimeofday(&t1, NULL);
	double workDTms = (double)t1.tv_sec*1000ULL + ((double)t1.tv_usec)/1000ULL;
	printf("done (%.02f sec)\n", (workDTms-nowDTms)/1000); fflush(stdout);

	return ret;
}

void CSql::setServerMultiStatementsOff()
{
	if (mysql_set_server_option(mysqlCon, MYSQL_OPTION_MULTI_STATEMENTS_OFF) != 0)
		show_error(__func__, __LINE__);
}

void CSql::setServerMultiStatementsOn()
{
	if (mysql_set_server_option(mysqlCon, MYSQL_OPTION_MULTI_STATEMENTS_ON) != 0)
		show_error(__func__, __LINE__);
}
