#pragma once

#include "sqlite3.h"
#include <string>

using namespace std;

class SQLite3Tool
{
public:
	//ȡ�������
	int getErroCode();
	//����ִ��
	int step(sqlite3_stmt* stmt);
	//ȡ��¼����
	int getDataCount(sqlite3_stmt* stmt);
	//ȡ�ֶ���
	int getColumnCount(sqlite3_stmt* stmt);
	//���ֶ�������
	int readColumnInt(sqlite3_stmt* stmt, int index);
	//ȡ�ֶ�����
	int getColumnType(sqlite3_stmt* stmt, int index);
	//ȡ�ֶγ���
	int getColumnBytes(sqlite3_stmt* stmt, int index);
	//ȡ�ֶ�����
	int getColumnIndex(sqlite3_stmt* stmt, string name);

	//���ֶ�˫������
	double readColumnDouble(sqlite3_stmt* stmt, int index);
	//���ֶγ�������
	int64_t readColumnInt64(sqlite3_stmt* stmt, int index);


	//�ر����ݿ�
	bool close();
	//��ʼ����
	bool transactionBegin();
	//�ύ����
	bool transactionCommit();
	//�ع�����
	bool transactionRoolBack();
	//�����ȴ�ʱ��
	bool setBusyTimeout(int time);
	//����¼�����õ���һ����¼֮ǰ
	bool reset(sqlite3_stmt* stmt);
	//�رռ�¼��
	bool finalize(sqlite3_stmt* stmt);
	//���Ƿ����
	bool isTableExist(string tableName);
	//����󶨲���
	bool claerBindings(sqlite3_stmt* stmt);
	//����ִ��SQL���
	bool exec(string sqlStr);
	//�����ݿ�
	bool open(const char* fileName, bool creat = false);
	//�󶨲���_������
	bool bindInt(sqlite3_stmt* stmt, int index, int value);
	//�󶨲���_�ַ���
	bool bindText(sqlite3_stmt* stmt, int index, string value);
	//�󶨲���_��������
	bool bindInt64(sqlite3_stmt* stmt, int index, int64_t value);



	//ȡ�����ı�
	string getErroMsg();
	//ȡ��汾
	string getLibVersion();
	//ȡSQL���
	string getSqlStr(sqlite3_stmt* stmt);
	//ȡ�ֶ���
	string getColumnName(sqlite3_stmt* stmt, int index);
	//���ֶ��ַ���
	string readColumnText(sqlite3_stmt* stmt, int index);
	//ȡ�ֶ�������
	string getColumnDecltype(sqlite3_stmt* stmt, int index);
	//ȡ�ֶα���
	string getColumnTableName(sqlite3_stmt* stmt, int index);
	//ȡ�ֶ����ݿ���
	string getColumnDatabaseName(sqlite3_stmt* stmt, int index);


	//ö����ͼ
	sqlite3_stmt* enumView();
	//ö�ٱ�
	sqlite3_stmt* enumTable();
	//��SQL���
	sqlite3_stmt* setSqlStr(string sql);
	//ö�ٴ�����
	sqlite3_stmt* enumTrigger(string tableName);
private:
	sqlite3* pDb;
};