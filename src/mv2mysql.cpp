/*
	mv2mysql - convert MediathekView db to mysql
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

#define PROGVERSION "0.2.0"
#define DBVERSION "1.1"

#define DEBUG_PRINT

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>

#include <json/json.h>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <mysql_error.h>

#include <iostream>
#include <climits>

#include "mv2mysql.h"
#include "helpers.h"

CMV2Mysql::CMV2Mysql()
{
	progName	= "mv2mysql";
	progCopyright	= "Copyright (C) 2015-2017, M. Liebmann 'micha-bbg'";
	progVersion	= "v"PROGVERSION;

	fulldb = false;
	diffdb = false;

	mysqlCon = NULL;
}

CMV2Mysql::~CMV2Mysql()
{
	if (mysqlCon != NULL)
		delete mysqlCon;
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
	printf("	-h | --help	  => Display this help screen and exit\n");
	printf("	-v | --version	  => Display versions info and exit\n");
}

int CMV2Mysql::run(int argc, char *argv[])
{
	if (argc < 2) {
		printHeader();
		printf("\tType '%s --help' to print help screen\n", progName);
		return 0;
	}
	int noParam       = 0;
	int requiredParam = 1;
//	int optionalParam = 2;
	static struct option long_options[] = {
		{"help",	noParam,       NULL, 'h'},
		{"version",	noParam,       NULL, 'v'},
		{"fulldb",	requiredParam, NULL, 'f'},
		{"diffdb",	requiredParam, NULL, 'd'},
		{NULL,		0, NULL, 0}
	};
	int c, opt;
	while ((opt = getopt_long(argc, argv, "h?vf:d:", long_options, &c)) >= 0) {
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
				if (diffdb == false) {
					fulldb = true;
					jsondb = string(optarg);
				}
				break;
			case 'd': 
				if (fulldb == false) {
					diffdb = true;
					jsondb = string(optarg);
				}
				break;
			default:
				break;
		}
	}

	if (!openDB(jsondb, fulldb))
		return 1;

	bool parseIO = parseDB(fulldb);
	if (parseIO) {
		connectMysql();
		writeMysql(fulldb);
	}

	return 0;
}

void CMV2Mysql::convert_db(string db)
{
	FILE* f1 = fopen(db.c_str(), "r");
	if (f1 == NULL) {
		printf("#### [%s:%d] error opening db file: %s\n", __func__, __LINE__, db.c_str());
		return;
	}
	FILE* f2 = fopen((db + ".tmp").c_str(), "w+");

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

bool CMV2Mysql::openDB(string db, bool /*is_fulldb*/)
{
	convert_db(db);

	FILE* f = fopen((db + ".tmp").c_str(), "r");
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

#if 1
/* ################################################ */
f = fopen((db + ".xxx").c_str(), "w+");
fwrite(jsonBuf.c_str(), jsonBuf.length(), 1, f);
fclose(f);
/* ################################################ */
#endif

	printf("done (%u Bytes)\n", (uint32_t)fsize); fflush(stdout);

	return true;
}

bool CMV2Mysql::parseDB(bool /*is_fulldb*/)
{
	printf("[%s] parse json db...", progName); fflush(stdout);

	Json::Value root;
	Json::Reader reader;

	bool parsedSuccess = reader.parse(jsonBuf, root, true);
	if(!parsedSuccess) {
		printf("\nFailed to parse JSON\n");
		printf("[%s:%d] %s\n", __func__, __LINE__, reader.getFormattedErrorMessages().c_str());
		return false;
	}

	Json::Value data;

	string cName = "";
	string tName = "";
	int cCount = 0;
	TVideoInfoEntry videoInfoEntry;
	videoInfoEntry.lastest = INT_MIN;
	videoInfoEntry.oldest = INT_MAX;
	time_t nowTime = time(NULL);
	int days = 1000;
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
			/*
			## Example: ##
			full		166107 entrys
			  1 day		  1893 entrys
			  7 days	  9323 entrys
			 14 days	 14844 entrys
			 31 days	 23262 entrys
			 60 days	 35565 entrys
			 90 days	 47236 entrys
			120 days	 56320 entrys
			180 days	 77016 entrys
			*/
			if (videoEntry.date_unix > 0) {
				days = 180;
				time_t maxDiff = (24*3600) * days; /* Not older than 180 days */
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

			videoList.push_back(videoEntry);
		}
	}
	videoInfo.push_back(videoInfoEntry);

	printf("done (%u (%d days) / %u entrys)\n", (uint32_t)(videoList.size()), days, (uint32_t)(root.size()-2)); fflush(stdout);
	jsonBuf.clear();

	if (videoList.size() < 1000) {
		printf("[%s] Video list too small (%d entrys), no transfer to the database.", progName, (uint32_t)(videoList.size())); fflush(stdout);
		return false;
	}

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

	mysqlDriver = sql::mysql::get_mysql_driver_instance();
	mysqlCon = mysqlDriver->connect("tcp://localhost:3306", v[0].c_str(), v[1].c_str());

	return true;
}

bool CMV2Mysql::writeMysql(bool is_fulldb)
{
	sql::Statement *stmt;
	sql::PreparedStatement  *prep_stmt;
	time_t startTime = time(0);
	stmt = mysqlCon->createStatement();

	if (is_fulldb) {
		printf("[%s] write temporary mysql db...", progName); fflush(stdout);
		createVideoDB_fromTemplate(VIDEO_DB_TMP_1);
		stmt->execute("USE " VIDEO_DB_TMP_1";");
	}
	else {
		printf("[%s] update mysql db...", progName); fflush(stdout);
		stmt->execute("USE " VIDEO_DB";");
	}

	prep_stmt = mysqlCon->prepareStatement("INSERT INTO " VIDEO_TABLE " \
		(channel, theme, title, duration, size_mb, description, url, website, subtitle, \
		url_rtmp, url_small, url_rtmp_small, url_hd, url_rtmp_hd, date_unix, url_history, geo, new_entry) \
		VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
	for (unsigned int i = 0; i < videoList.size(); ++i) {
		prep_stmt->setString(1,  checkString(videoList[i].channel, 256));
		prep_stmt->setString(2,  checkString(videoList[i].theme, 256));
		prep_stmt->setString(3,  checkString(videoList[i].title, 1024));
		prep_stmt->setInt(   4,  checkInt(videoList[i].duration));
		prep_stmt->setInt(   5,  checkInt(videoList[i].size_mb));
		prep_stmt->setString(6,  checkString(videoList[i].description, 8192));
		prep_stmt->setString(7,  checkString(videoList[i].url, 1024));
		prep_stmt->setString(8,  checkString(videoList[i].website, 1024));
		prep_stmt->setString(9,  checkString(videoList[i].subtitle, 1024));
		prep_stmt->setString(10, checkString(videoList[i].url_rtmp, 1024));
		prep_stmt->setString(11, checkString(videoList[i].url_small, 1024));
		prep_stmt->setString(12, checkString(videoList[i].url_rtmp_small, 1024));
		prep_stmt->setString(13, checkString(videoList[i].url_hd, 1024));
		prep_stmt->setString(14, checkString(videoList[i].url_rtmp_hd, 1024));
		prep_stmt->setInt(   15, checkInt(videoList[i].date_unix));
		prep_stmt->setString(16, checkString(videoList[i].url_history, 1024));
		prep_stmt->setString(17, checkString(videoList[i].geo, 1024));
		prep_stmt->setString(18, checkString(videoList[i].new_entry, 1024));
		prep_stmt->execute();
	}
	delete prep_stmt;

	struct stat st;
	stat(jsondb.c_str(), &st);

	if (is_fulldb) {
		prep_stmt = mysqlCon->prepareStatement("INSERT INTO " INFO_TABLE " (channel, count, lastest, oldest) VALUES (?, ?, ?, ?)");
		for (unsigned int i = 0; i < videoInfo.size(); ++i) {
			prep_stmt->setString(1, videoInfo[i].channel);
			prep_stmt->setInt(   2, videoInfo[i].count);
			prep_stmt->setInt(   3, videoInfo[i].lastest);
			prep_stmt->setInt(   4, videoInfo[i].oldest);
			prep_stmt->execute();
		}
		delete prep_stmt;

		prep_stmt = mysqlCon->prepareStatement("INSERT INTO " VERSION_TABLE " (version, vdate, mvversion, mvdate, mventrys) VALUES (?, ?, ?, ?, ?)");
		prep_stmt->setString(1, DBVERSION);
		prep_stmt->setInt(   2, time(0));
		prep_stmt->setString(3, mvVersion);
		prep_stmt->setInt(   4, st.st_mtime);
		prep_stmt->setInt(   5, videoList.size());
		prep_stmt->execute();
		delete prep_stmt;

		time_t endTime = time(0);
		printf("done (%ld sec)\n", endTime-startTime); fflush(stdout);
		startTime = time(0);
		printf("[%s] copy mysql db...", progName); fflush(stdout);

		createVideoDB_fromTemplate(VIDEO_DB);
		stmt->execute("USE " VIDEO_DB";");
		stmt->execute("INSERT INTO "VIDEO_DB"."VIDEO_TABLE" SELECT * FROM "VIDEO_DB_TMP_1"."VIDEO_TABLE";");
		stmt->execute("INSERT INTO "VIDEO_DB"."INFO_TABLE" SELECT * FROM "VIDEO_DB_TMP_1"."INFO_TABLE";");
		stmt->execute("INSERT INTO "VIDEO_DB"."VERSION_TABLE" SELECT * FROM "VIDEO_DB_TMP_1"."VERSION_TABLE";");

		delete stmt;
		endTime = time(0);
		printf("done (%ld sec)\n", endTime-startTime); fflush(stdout);
	}
	else {
		/* Update INFO_TABLE & VERSION_TABLE */

		uint32_t tmp1 = (uint32_t)time(0);
		string vdate = to_string(tmp1);
		tmp1 = (uint32_t)st.st_mtime;
		string mvdate = to_string(tmp1);
		tmp1 = getMysqlTableSize(VIDEO_DB, VIDEO_TABLE);
		string mventrys = to_string(tmp1);

		string query = "UPDATE " VERSION_TABLE \
					" SET vdate='" + vdate + \
					"', mvversion='" + mvVersion + "', mvdate='" + mvdate + \
					"', mventrys='" + mventrys + "' WHERE id=1;";
//printf("\n \nquery: %s\n \n", query.c_str());
		prep_stmt = mysqlCon->prepareStatement(query);
		prep_stmt->execute();
		delete prep_stmt;

		time_t endTime = time(0);
		printf("done (%ld sec)\n", endTime-startTime); fflush(stdout);
	}

	videoList.clear();
	return true;
}

uint32_t CMV2Mysql::getMysqlTableSize(string db, string table)
{
	try {
		sql::Statement *stmt;
		sql::ResultSet *result;

		stmt = mysqlCon->createStatement();
		stmt->execute("USE " + db + ";");
		string query = "SELECT id FROM " + table + ";";
		result = stmt->executeQuery(query);
		uint32_t ret = (uint32_t)result->rowsCount();

		delete result;
		delete stmt;
		return ret;

	} catch (sql::SQLException &e) {
		printf("\n[%s:%d] # ERR: %s (MySQL error code: %d, SQLState: %s)\n \n",
		__func__, __LINE__, 
		e.what(), 
		e.getErrorCode(), 
		e.getSQLState().c_str());
	}
	return 0;
}

bool CMV2Mysql::createVideoDB_fromTemplate(string name)
{
	sql::Statement *stmt;

	stmt = mysqlCon->createStatement();
	stmt->execute("DROP DATABASE IF EXISTS " + name +";");
	stmt->execute("CREATE DATABASE IF NOT EXISTS " + name + " DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;");
	stmt->execute("CREATE TABLE " + name + "."VIDEO_TABLE" LIKE "VIDEO_DB_TEMPLATE"."VIDEO_TABLE";");
	stmt->execute("CREATE TABLE " + name + "."INFO_TABLE" LIKE "VIDEO_DB_TEMPLATE"."INFO_TABLE";");
	stmt->execute("CREATE TABLE " + name + "."VERSION_TABLE" LIKE "VIDEO_DB_TEMPLATE"."VERSION_TABLE";");

	delete stmt;
	return true;
}


int main(int argc, char *argv[])
{
	return CMV2Mysql::getInstance()->run(argc, argv);
}
