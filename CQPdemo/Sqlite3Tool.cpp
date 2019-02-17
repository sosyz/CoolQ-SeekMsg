#include "sqlite3.h"
#include "SqliteTool.h"
#include <string>

//打开数据库
bool SQLite3Tool::open(const char* fileName, bool creat){
	int flag = SQLITE_OPEN_READWRITE;
	if (creat) {
		flag = flag | 4;
	}
	int r = sqlite3_open_v2(fileName, &pDb, flag, 0);
	if (r != 0) {
		return false;
	}
	else {
		string erro;
		return exec("PRAGMA synchronous = OFF");
	}
}

//关闭数据库
bool SQLite3Tool::close(){
	int r = sqlite3_close(pDb);
	return r == 0;
}

//读字段双浮点型
double SQLite3Tool::readColumnDouble(sqlite3_stmt* stmt, int index) {

}

//读字段长整数型
int64_t SQLite3Tool::readColumnInt64(sqlite3_stmt* stmt, int index) {
	return sqlite3_column_int64(stmt, index);
}

//读字段整数型
int SQLite3Tool::readColumnInt(sqlite3_stmt* stmt, int index) {
	return sqlite3_column_int(stmt, index);
}

//读字段字符型
string SQLite3Tool::readColumnText(sqlite3_stmt* stmt, int index) {
	return (char *)sqlite3_column_text(stmt, index);
}

//置最大等待时间
bool SQLite3Tool::setBusyTimeout(int time){
	return sqlite3_busy_timeout(pDb, time) == 0;
}

//绑定参数_整数型
bool SQLite3Tool::bindInt(sqlite3_stmt* stmt, int index, int value){
	return sqlite3_bind_int(stmt, index, value) == 0;
}

//绑定参数_长整数型
bool SQLite3Tool::bindInt64(sqlite3_stmt* stmt, int index, int64_t value){
	return sqlite3_bind_int64(stmt, index, value) == 0;
}

//绑定参数_字符型
bool SQLite3Tool::bindText(sqlite3_stmt* stmt, int index, string value){
	return sqlite3_bind_text(stmt, index, value.c_str(), value.length(), 0) == 0;
}

//清除绑定参数
bool SQLite3Tool::claerBindings(sqlite3_stmt* stmt){
	return sqlite3_clear_bindings(stmt) == 0;
}

//取错误代码
int  SQLite3Tool::getErroCode(){
	return sqlite3_errcode(pDb);
}

//取错误文本
string SQLite3Tool::getErroMsg(){
	return sqlite3_errmsg(pDb);
}

//取库版本
string SQLite3Tool::getLibVersion(){
	return sqlite3_libversion();
}

//取SQL语句
string SQLite3Tool::getSqlStr(sqlite3_stmt* stmt){
	return sqlite3_sql(stmt);
}

//置SQL语句
sqlite3_stmt* SQLite3Tool::setSqlStr(string sql){
	sqlite3_stmt* stmt;
	int r = sqlite3_prepare_v2(pDb, sql.c_str(), sql.length(), &stmt, 0);
	if (r != 0) {
		sqlite3_finalize(stmt);
		return 0;
	}
	return stmt;
}

//单步执行
int SQLite3Tool::step(sqlite3_stmt* stmt){
	return sqlite3_step(stmt);
}

//批量执行SQL语句
bool SQLite3Tool::exec(string sqlStr){
	char* erro;
	int r = sqlite3_exec(pDb, sqlStr.c_str(), 0, 0, &erro);
	sqlite3_free(erro);
	return r == 0;
}

//关闭记录集
bool SQLite3Tool::finalize(sqlite3_stmt* stmt){
	return sqlite3_finalize(stmt) == 0;
}

//将记录集重置到第一条记录之前
bool SQLite3Tool::reset(sqlite3_stmt* stmt){
	return sqlite3_reset(stmt) == 0;
}

//取记录行数
int SQLite3Tool::getDataCount(sqlite3_stmt* stmt){
	return sqlite3_data_count(stmt);
}

//取字段数
int SQLite3Tool::getColumnCount(sqlite3_stmt* stmt){
	return sqlite3_column_count(stmt);
}

//枚举视图
sqlite3_stmt* SQLite3Tool::enumView(){
	return setSqlStr("select name,sql from sqlite_master where type='view'");
}

//枚举表
sqlite3_stmt* SQLite3Tool::enumTable(){
	return setSqlStr("select tbl_name,sql from sqlite_master where type='table' and tbl_name<>'sqlite_sequence'");
}

//枚举触发器
sqlite3_stmt* SQLite3Tool::enumTrigger(string tableName){
	return setSqlStr("select name,sql from sqlite_master where type='trigger' and tbl_name='" + tableName + "'");
}

//取字段类型
int SQLite3Tool::getColumnType(sqlite3_stmt* stmt, int index){
	return sqlite3_column_type(stmt, index);
}

//取字段长度
int SQLite3Tool::getColumnBytes(sqlite3_stmt* stmt, int index){
	return sqlite3_column_bytes(stmt, index);
}

//取字段索引
int SQLite3Tool::getColumnIndex(sqlite3_stmt* stmt, string name){
	for (int i = 0; i < getColumnCount(stmt); i++) {
		if (getColumnName(stmt, i) == name) {
			return i;
		}
	}
	return -1;
}

//开始事务
bool SQLite3Tool::transactionBegin(){}

//提交事务
bool SQLite3Tool::transactionCommit(){}

//回滚事务
bool SQLite3Tool::transactionRoolBack(){}

//表是否存在
bool SQLite3Tool::isTableExist(string tableName){
	sqlite3_stmt* stmt = setSqlStr("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND tbl_name='" + tableName + "'");
	step(stmt);
	int num = readColumnInt(stmt, 0);
	finalize(stmt);
	return num != 0;
}

//取字段名
string SQLite3Tool::getColumnName(sqlite3_stmt* stmt, int index){
	return sqlite3_column_name(stmt, index);
}

//取字段类型名
string SQLite3Tool::getColumnDecltype(sqlite3_stmt* stmt, int index){
	return sqlite3_column_decltype(stmt, index);
}

//取字段表名
string SQLite3Tool::getColumnTableName(sqlite3_stmt* stmt, int index){
	return sqlite3_column_table_name(stmt, index);
}

//取字段数据库名
string SQLite3Tool::getColumnDatabaseName(sqlite3_stmt* stmt, int index){
	return sqlite3_column_database_name(stmt, index);
}