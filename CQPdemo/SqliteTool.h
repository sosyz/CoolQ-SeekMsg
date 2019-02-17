#pragma once

#include "sqlite3.h"
#include <string>

using namespace std;

class SQLite3Tool
{
public:
	//取错误代码
	int getErroCode();
	//单步执行
	int step(sqlite3_stmt* stmt);
	//取记录行数
	int getDataCount(sqlite3_stmt* stmt);
	//取字段数
	int getColumnCount(sqlite3_stmt* stmt);
	//读字段整数型
	int readColumnInt(sqlite3_stmt* stmt, int index);
	//取字段类型
	int getColumnType(sqlite3_stmt* stmt, int index);
	//取字段长度
	int getColumnBytes(sqlite3_stmt* stmt, int index);
	//取字段索引
	int getColumnIndex(sqlite3_stmt* stmt, string name);

	//读字段双浮点型
	double readColumnDouble(sqlite3_stmt* stmt, int index);
	//读字段长整数型
	int64_t readColumnInt64(sqlite3_stmt* stmt, int index);


	//关闭数据库
	bool close();
	//开始事务
	bool transactionBegin();
	//提交事务
	bool transactionCommit();
	//回滚事务
	bool transactionRoolBack();
	//置最大等待时间
	bool setBusyTimeout(int time);
	//将记录集重置到第一条记录之前
	bool reset(sqlite3_stmt* stmt);
	//关闭记录集
	bool finalize(sqlite3_stmt* stmt);
	//表是否存在
	bool isTableExist(string tableName);
	//清除绑定参数
	bool claerBindings(sqlite3_stmt* stmt);
	//批量执行SQL语句
	bool exec(string sqlStr);
	//打开数据库
	bool open(const char* fileName, bool creat = false);
	//绑定参数_整数型
	bool bindInt(sqlite3_stmt* stmt, int index, int value);
	//绑定参数_字符型
	bool bindText(sqlite3_stmt* stmt, int index, string value);
	//绑定参数_长整数型
	bool bindInt64(sqlite3_stmt* stmt, int index, int64_t value);



	//取错误文本
	string getErroMsg();
	//取库版本
	string getLibVersion();
	//取SQL语句
	string getSqlStr(sqlite3_stmt* stmt);
	//取字段名
	string getColumnName(sqlite3_stmt* stmt, int index);
	//读字段字符型
	string readColumnText(sqlite3_stmt* stmt, int index);
	//取字段类型名
	string getColumnDecltype(sqlite3_stmt* stmt, int index);
	//取字段表名
	string getColumnTableName(sqlite3_stmt* stmt, int index);
	//取字段数据库名
	string getColumnDatabaseName(sqlite3_stmt* stmt, int index);


	//枚举视图
	sqlite3_stmt* enumView();
	//枚举表
	sqlite3_stmt* enumTable();
	//置SQL语句
	sqlite3_stmt* setSqlStr(string sql);
	//枚举触发器
	sqlite3_stmt* enumTrigger(string tableName);
private:
	sqlite3* pDb;
};