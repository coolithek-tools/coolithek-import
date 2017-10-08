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

/* TODO: Separate class for shared sql functions */

bool CSql::databaseExists(string db)
{
	bool ret = false;
	string sql = "SHOW DATABASES LIKE '"+ db +"';";
	executeSingleQueryString(sql);

	MYSQL_RES* result = mysql_store_result(mysqlCon);
	MYSQL_ROW row;
	if (mysql_num_fields(result) > 0) {
		row = mysql_fetch_row(result);
		if ((row != NULL) && (row[0] != NULL) && (row[0] == db))
			ret = true;
	}
	mysql_free_result(result);

	return ret;
}

uint32_t CSql::getTableEntries(string db, string table)
{
	string tmpUsedDb = getUsedDatabase();
	setUsedDatabase(db);
	uint32_t ret = 0;
        string query = "SELECT COUNT(id) AS anz FROM " + table + ";";
	executeSingleQueryString(query);
	MYSQL_RES* result = mysql_store_result(mysqlCon);
	MYSQL_ROW row;
	if (mysql_num_fields(result) > 0) {
		row = mysql_fetch_row(result);
		if ((row != NULL) && (row[0] != NULL))
			ret = atoi(row[0]);
	}
	mysql_free_result(result);
	
	if ((!tmpUsedDb.empty()) && (tmpUsedDb != db))
		setUsedDatabase(tmpUsedDb);

	return ret;
}

uint32_t CSql::getLastIndex(string db, string table)
{
	string tmpUsedDb = getUsedDatabase();
	setUsedDatabase(db);
	uint32_t ret = 0;
	string query = "SELECT MAX(id) FROM " + table;
	executeSingleQueryString(query);
	MYSQL_RES* result = mysql_store_result(mysqlCon);
	MYSQL_ROW row;
	if (mysql_num_fields(result) > 0) {
		row = mysql_fetch_row(result);
		if ((row != NULL) && (row[0] != NULL))
			ret = atoi(row[0]);
	}
	mysql_free_result(result);
	
	if ((!tmpUsedDb.empty()) && (tmpUsedDb != db))
		setUsedDatabase(tmpUsedDb);

	return ret;
}

string CSql::getUsedDatabase()
{
	string ret_s = "";
	string query = "SELECT DATABASE();";
	executeSingleQueryString(query);
	MYSQL_RES* result = mysql_store_result(mysqlCon);
	MYSQL_ROW row;
	if (mysql_num_fields(result) > 0) {
		row = mysql_fetch_row(result);
		if ((row != NULL) && (row[0] != NULL))
			ret_s = (string)row[0];
	}
	mysql_free_result(result);

	return ret_s;
}

void CSql::setUsedDatabase(string db)
{
	if (!db.empty() && databaseExists(db)) {
		string query = "USE " + db + ";";
		executeSingleQueryString(query);
	}
}

bool CSql::copyDatabase(string fromDB, string toDB, string characterSet, bool noData/*=false*/)
{
	return intCopyOrRenameDatabase(fromDB, toDB, characterSet, db_mode_copy, noData);
}

bool CSql::renameDatabase(string fromDB, string toDB, string characterSet)
{
	return intCopyOrRenameDatabase(fromDB, toDB, characterSet, db_mode_rename);
}

bool CSql::intCopyOrRenameDatabase(string fromDB, string toDB, string characterSet, int mode, bool noData/*=false*/)
{
	if (multiQuery)
		setServerMultiStatementsOff();

	string oldUsedDB = getUsedDatabase();
	setUsedDatabase(fromDB);
	string query = "SHOW TABLES;";
	executeSingleQueryString(query);

	vector<string> tablesList;
	MYSQL_RES* result = mysql_store_result(mysqlCon);
	MYSQL_ROW row;
	while ((row = mysql_fetch_row(result))) {
		tablesList.push_back((string)row[0]);
	}
	mysql_free_result(result);
	if (multiQuery)
		setServerMultiStatementsOn();

	query = "";
	query += "START TRANSACTION;";
	query += "SET autocommit = 0;";
	query += "DROP DATABASE IF EXISTS `" + toDB + "`;";
	query += "CREATE DATABASE IF NOT EXISTS `" + toDB + "` " + characterSet + ";";
	/* copy tables */
	if (mode == db_mode_copy) {
		for (size_t i = 0; i < tablesList.size(); i++) {
			query += "CREATE TABLE " + toDB + "." + tablesList[i] + " LIKE " + fromDB + "." + tablesList[i] + ";";
			if (!noData)
				query += "INSERT INTO " + toDB + "." + tablesList[i] + " SELECT * FROM " + fromDB + "." + tablesList[i] + ";";
		}
	}
	/* rename (move) tables */
	else if (mode == db_mode_rename) {
		for (size_t i = 0; i < tablesList.size(); i++) {
			query += "RENAME TABLE " + fromDB + "." + tablesList[i] + " TO " + toDB + "." + tablesList[i] + ";";
		}
		query += "DROP DATABASE IF EXISTS `" + fromDB + "`;";
	}
	query += "COMMIT;";
	query += "SET autocommit = 1;";
	bool ret = executeMultiQueryString(query);

	setUsedDatabase(oldUsedDB);
	return ret;
}
