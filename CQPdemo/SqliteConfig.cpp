#include "SqliteConfig.h"
#include "SeekMsg.h"
#include "SqliteTool.h"

#include <string>

using namespace std;

SQLite3Tool config;

int loadConfig(string appDirectory) {
	const string configFile = appDirectory + "config.db";
	config.open(GbkToUtf8(configFile).c_str());
	if (!config.isTableExist("config")) {
		if (!config.exec("CREATE TABLE \"config\" (\"id\" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,\"qq\" integer,\"name\" TEXT,\"value\" TEXT);")) {
		}
		else {
			return CODE_FIRST;
		}
	}
	return CODE_NONE;
}

void closeConfig(){
	config.close();
}

bool writeConfig(int64_t QQ, string name, string value, bool isAdd) {
	sqlite3_stmt* stmt = config.setSqlStr("SELECT value FROM config WHERE `qq` = ? AND `name` = ?");
	config.bindInt64(stmt, 1, QQ);
	config.bindText(stmt, 2, name);
	config.step(stmt);
	string data = config.readColumnText(stmt, 0);
	int count = config.getDataCount(stmt);
	config.finalize(stmt);
	if (count == 1) {
		stmt = config.setSqlStr("UPDATE config SET value = ? WHERE `qq` = ? AND `name` = ?");
	}
	else {
		stmt = config.setSqlStr("INSERT INTO config(`value`,`qq`,`name`) VALUES(?,?,?)");
	}
	if (isAdd) {
		data = value + data;
	}
	config.bindText(stmt, 1, data);
	config.bindInt64(stmt, 2, QQ);
	config.bindText(stmt, 3, name);
	bool b = config.step(stmt) == 100;
	config.finalize(stmt);
	return b;
}

string readConfig(const int64_t QQ, const string name) {
	sqlite3_stmt* stmt = config.setSqlStr("SELECT count(value) FROM config WHERE `qq` = ? AND `name` = ?");
	config.bindInt64(stmt, 1, QQ);
	config.bindText(stmt, 2, name);
	config.step(stmt);
	string value = config.readColumnText(stmt, 0);
	return value;
}

bool delectConfig(const int64_t QQ, const string name) {
	string err;
	return config.exec("DELETE FROM config WHERE `qq` = " + to_string(QQ) + " AND `name` = '" + name + "';");
}

bool execSQL(const string SQL) {
	string err;
	return config.exec(SQL);
}
