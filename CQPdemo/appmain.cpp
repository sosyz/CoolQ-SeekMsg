#include "stdafx.h"
#include "cqp.h"
#include "appmain.h"
#include "SeekMsg.h"
#include "sqlite3.h"

#include <string>
#include <time.h>

using namespace std;
int ac = -1; //AuthCode ���ÿ�Q�ķ���ʱ��Ҫ�õ�
string appDirectory;
string cfgDirectory;
string rootKey = "";
sqlite3* pDB;
//define database object

bool AllisNum(string str)
{
	for (int i = 0; i < str.size(); i++)
	{
		int tmp = (int)str[i];
		if (tmp >= 48 && tmp <= 57)
		{
			continue;
		}
		else
		{
			return false;
		}
	}
	return true;
}

bool writeConfig(int64_t QQ, string name, string value, bool isAdd = false) {
	sqlite3_stmt* stmt = NULL;
	string sql = "";
	string data = "";

	//�ж��Ƿ�׷������
	if (isAdd) {
		sql = "SELECT value FROM config WHERE `qq` = ? AND `name` = ?";
		sqlite3_prepare_v2(pDB, sql.c_str(), sql.length(), &stmt, 0);
		sqlite3_bind_int64(stmt, 1, QQ);
		sqlite3_bind_text(stmt, 2, name.c_str(), name.length(), 0);
		data = (char *)sqlite3_column_text(stmt, 0);
		data = data + value;
		sqlite3_finalize(stmt);
	}
	else {
		data = value;
	}
	sql = "SELECT COUNT(`value`) FROM config WHERE `qq` = ? AND `name` = ?";
	sqlite3_prepare_v2(pDB, sql.c_str(), sql.length(), &stmt, 0);
	sqlite3_bind_int64(stmt, 1, QQ);
	sqlite3_bind_text(stmt, 2, name.c_str(), name.length(), 0);
	sqlite3_step(stmt);
	int count = sqlite3_column_int(stmt,0);
	sqlite3_finalize(stmt);
	if (count == 1) {
		sql = "UPDATE config SET value = ? WHERE `qq` = ? AND `name` = ?";
	}
	else {
		sql = "INSERT INTO config(`value`,`qq`,`name`) VALUES(?,?,?)";
	}
	sqlite3_prepare_v2(pDB, sql.c_str(), sql.length(), &stmt, 0);
	sqlite3_bind_text(stmt, 1, data.c_str(), data.length(), 0);
	sqlite3_bind_int64(stmt, 2, QQ);
	sqlite3_bind_text(stmt, 3, name.c_str(), name.length(), 0);
	int s = sqlite3_step(stmt);
	if (s != SQLITE_DONE)
	{
		CQ_addLog(ac, CQLOG_DEBUG, "WRITERETURN", to_string(s).c_str());
	}
	sqlite3_finalize(stmt);
	return s == SQLITE_DONE;
}

string readConfig(const int64_t QQ, const string name) {
	sqlite3_stmt* stmt;
	string sql = "";
	string data = ""; 

	sql = "SELECT `value` FROM config WHERE `qq` = ? AND `name` = ?";
	sqlite3_prepare_v2(pDB, sql.c_str(), sql.length(), &stmt, 0);
	sqlite3_bind_int64(stmt, 1, QQ);
	sqlite3_bind_text(stmt, 2, name.c_str(), name.length(), 0);
	int s = sqlite3_step(stmt);
	if (s == SQLITE_ROW) {
		data = (char *)sqlite3_column_text(stmt, 0);
	}
	else {
		CQ_addLog(ac, CQLOG_DEBUG, "READRETURN", to_string(s).c_str());
	}
	sqlite3_finalize(stmt);
	CQ_addLog(ac, CQLOG_DEBUG, "SELECTDATA", data.c_str());
	return data;
}

bool delectConfig(const int64_t QQ, const string name) {
	char* error = 0;
	string sql = "DELETE FROM config WHERE `qq` = " + to_string(QQ) + " AND `name` = '" + name + "';";
	if (sqlite3_exec(pDB, sql.c_str(), NULL, NULL, &error) != SQLITE_OK) {
		string content = "FAILED TO DELETE DATA, ERRORMSG:";
		content.append(error);
		CQ_addLog(ac, CQLOG_DEBUG, "ERREO", content.c_str());
		return false;
	}
	else {
		return true;
	}
}

bool execSQL(const string sql) {
	char* error = 0;
	if (sqlite3_exec(pDB, sql.c_str(), 0, 0, &error) != SQLITE_OK) {
		string content = "FAILED TO EXEC, ERRORMSG:";
		content.append(error);
		CQ_addLog(ac, CQLOG_DEBUG, "ERREO", content.c_str());
		return false;
	}
	else {
		return true;
	}
}

/* 
* ����Ӧ�õ�ApiVer��Appid������󽫲������
*/
CQEVENT(const char*, AppInfo, 0)() {
	return CQAPPINFO;
}


/* 
* ����Ӧ��AuthCode����Q��ȡӦ����Ϣ��������ܸ�Ӧ�ã���������������������AuthCode��
* ��Ҫ�ڱ��������������κδ��룬���ⷢ���쳣���������ִ�г�ʼ����������Startup�¼���ִ�У�Type=1001����
*/
CQEVENT(int32_t, Initialize, 4)(int32_t AuthCode) {
	ac = AuthCode;
	return 0;
}


/*
* Type=1001 ��Q����
* ���۱�Ӧ���Ƿ����ã������������ڿ�Q������ִ��һ�Σ���������ִ��Ӧ�ó�ʼ�����롣
* ��Ǳ�Ҫ����������������ش��ڡ���������Ӳ˵������û��ֶ��򿪴��ڣ�
*/
CQEVENT(int32_t, __eventStartup, 0)() {
	appDirectory = CQ_getAppDirectory(ac);
	cfgDirectory = appDirectory + "config.db";
	return 0;
}


/*
* Type=1002 ��Q�˳�
* ���۱�Ӧ���Ƿ����ã������������ڿ�Q�˳�ǰִ��һ�Σ���������ִ�в���رմ��롣
* ������������Ϻ󣬿�Q���ܿ�رգ��벻Ҫ��ͨ���̵߳ȷ�ʽִ���������롣
*/
CQEVENT(int32_t, __eventExit, 0)() {

	return 0;
}

/*
* Type=1003 Ӧ���ѱ�����
* ��Ӧ�ñ����ú󣬽��յ����¼���
* �����Q����ʱӦ���ѱ����ã�����_eventStartup(Type=1001,��Q����)�����ú󣬱�����Ҳ��������һ�Ρ�
* ��Ǳ�Ҫ����������������ش��ڡ���������Ӳ˵������û��ֶ��򿪴��ڣ�
*/
CQEVENT(int32_t, __eventEnable, 0)() {
	
	string sql = "";
	sqlite3_stmt* stmt;
	string content = "";
	sqlite3_open_v2(GbkToUtf8(cfgDirectory).c_str() , &pDB, SQLITE_OPEN_READWRITE| SQLITE_OPEN_CREATE, 0);
	sql = "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND tbl_name='config'";
	sqlite3_prepare_v2(pDB, sql.c_str(), sql.length(), &stmt, 0);
	sqlite3_step(stmt);
	int num = sqlite3_column_int(stmt, 0);
	sqlite3_finalize(stmt);
	if (num == 0) {
		sql = "CREATE TABLE \"config\" (\"id\" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,\"qq\" integer,\"name\" TEXT,\"value\" TEXT);";
		char* error;
		if (sqlite3_exec(pDB, sql.c_str(), 0, 0, &error) != SQLITE_OK) {
			content = "FAILED TO CREATE TABLE, ERRORMSG:";
			content.append(error);
			CQ_addLog(ac, CQLOG_DEBUG, "ERREO", content.c_str());
			CQ_addLog(ac, CQLOG_ERROR, "��ʼ��", "��ʼ��ʧ�ܣ�����޷��������У���ϸ������Ϣ������");
		}
		else {
			char temp[9] = { 0 };
			rootKey = randstr(temp, 8);
			content = "��⵽�ǵ�һ�����У����������˽�ķ���\"#";
			content.append(rootKey);
			content.append("\"��������");
			CQ_addLog(ac, CQLOG_WARNING, "��ʼ��", content.c_str());
		}
		sqlite3_free(error);
	}
	return 0;
}


/*
* Type=1004 Ӧ�ý���ͣ��
* ��Ӧ�ñ�ͣ��ǰ�����յ����¼���
* �����Q����ʱӦ���ѱ�ͣ�ã��򱾺���*����*�����á�
* ���۱�Ӧ���Ƿ����ã���Q�ر�ǰ��������*����*�����á�
*/
CQEVENT(int32_t, __eventDisable, 0)() {
	sqlite3_close(pDB);
	return 0;
}


/*
* Type=21 ˽����Ϣ
* subType �����ͣ�11/���Ժ��� 1/��������״̬ 2/����Ⱥ 3/����������
*/
CQEVENT(int32_t, __eventPrivateMsg, 24)(int32_t subType, int32_t msgId, int64_t fromQQ, const char *fromMsg, int32_t font) {

	//���Ҫ�ظ���Ϣ������ÿ�Q�������ͣ��������� return EVENT_BLOCK - �ضϱ�����Ϣ�����ټ�������  ע�⣺Ӧ�����ȼ�����Ϊ"���"(10000)ʱ������ʹ�ñ�����ֵ
	//������ظ���Ϣ������֮���Ӧ��/�������������� return EVENT_IGNORE - ���Ա�����Ϣ
	string msg = fromMsg;
	if (rootKey != "") {
		CQ_addLog(ac, CQLOG_DEBUG, "ROOTKEY", rootKey.c_str());
		string_replace(msg, "\r", "");
		string_replace(msg, "\n", "");
		string_replace(msg, " ", "");
		string_replace(msg, "��", "");
		if (msg == "#" + rootKey) {
			writeConfig(0, "tempAdmin", to_string(fromQQ));
			writeConfig(0, "rootState", "1");
			CQ_sendPrivateMsg(ac, fromQQ, "���ѽ�������ģʽ���Ƿ񽫴�QQ����Ϊ�����ߣ�����\"��\"��QQ��");
			return EVENT_BLOCK;
		}

		int rootState = strtol(readConfig(0, "rootState").c_str(), NULL, 0);
		CQ_addLog(ac, CQLOG_DEBUG, "rootState", to_string(rootState).c_str());
		if (rootState != 0 && msg == "�˳�") {
			execSQL("DELETE FROM config WHERE qq = 0");
			CQ_sendPrivateMsg(ac, fromQQ, "���˳�����ģʽ");
			return EVENT_BLOCK;
		}

		if (readConfig(0, "tempAdmin") == to_string(fromQQ)) {
			CQ_addLog(ac, CQLOG_DEBUG, "if", "true");
			switch (rootState)
			{
			case 1:
				writeConfig(0, "rootState", "2");
				if (msg == "��") {
					writeConfig(0, "admin", to_string(fromQQ));
					string outMsg;
					outMsg.append("�ѽ�");
					outMsg.append(to_string(fromQQ));
					outMsg.append("��Ϊ����Ա��������һ������");
					CQ_sendPrivateMsg(ac, fromQQ, outMsg.c_str());
					CQ_sendPrivateMsg(ac, fromQQ, "��ѡ���ѯȨ��ģʽ��\n[1]���������˽��в�ѯ\n[2]���ظ���ͨ��ѯȨ�޵���");
				}else if(strtol(msg.c_str(), NULL, 0)) {
					writeConfig(0, "admin", msg);
					string outMsg;
					outMsg.append("�ѽ�");
					outMsg.append(msg);
					outMsg.append("��Ϊ����Ա��������һ������");

					CQ_sendPrivateMsg(ac, fromQQ, outMsg.c_str());
					CQ_sendPrivateMsg(ac, fromQQ, "��ѡ���ѯȨ��ģʽ��\n[1]���������˽��в�ѯ\n[2]���ظ���ͨ��ѯȨ�޵���");
				}
				else {
					CQ_sendPrivateMsg(ac, fromQQ, "����Ļظ��������·��ͣ����߷���\"�˳�\"�˳����ã���Ҫע����ǣ��˲�����ʹ���޷�ʹ�ñ����");
				}
				break;
			case 2:
				switch (strtol(msg.c_str(), NULL, 0))
				{
				case 1:
					delectConfig(0, "rootState");
					writeConfig(0, "mode", "1");
					CQ_sendPrivateMsg(ac, fromQQ, "���óɹ������������˽��в�ѯ����ʼ�����õ��˽���");
					break;
				case 2:
					delectConfig(0, "rootState");
					writeConfig(0, "mode", "2");
					writeConfig(0, "useList", "");
					rootKey = "";
					CQ_sendPrivateMsg(ac, fromQQ, "���óɹ������ظ���ͨ��ѯȨ�޵ģ������Է���\"��ͨ[QQ]\"������ӣ���\"��ͨ10000\"����ʼ�����õ��˽���");
					break;
				default:
					CQ_sendPrivateMsg(ac, fromQQ, "����Ļظ��������·��ͣ����߷���\"�˳�\"�˳����ã���Ҫע����ǣ��˲�����ʹ���޷�ʹ�ñ����");
				}
			}
			return EVENT_BLOCK;
		}
	}
	
	if (msg == "�����ѯ") {
		int mode = strtol(readConfig(0, "mode").c_str(), NULL, 0);
		switch (mode)
		{
		case 1:
			writeConfig(fromQQ, "userState", "1");
			CQ_sendPrivateMsg(ac, fromQQ, "���ѿ�����ѯ���ܣ��뷢����Ҫ��ѯ��Ⱥ��");
			break;
		case 2:{string list = readConfig(0, "userList");
		if (list.find("@" + to_string(fromQQ) + "@") == string::npos) {
			CQ_sendPrivateMsg(ac, fromQQ, "��Ǹ����û�в�ѯȨ�ޣ��޷�ʹ�ò�ѯ����");
		}
		else {
			writeConfig(fromQQ, "userState", "1");
			CQ_sendPrivateMsg(ac, fromQQ, "���ѿ�����ѯ���ܣ��뷢����Ҫ��ѯ��Ⱥ��");
		}
		break; }
		default:
			break;
		}
		return EVENT_BLOCK;
	}
	else if (msg.substr(0, 6) == "��ͨ" || msg.substr(0, 6) == "�Ƴ�") {
		if (stoll(readConfig(0, "admin")) == fromQQ) {
			string addQQ = msg.substr(6);
			string outMsg;
			if (stoll(addQQ) >= 10000) {
				if (msg.substr(0, 6) == "��ͨ") {
					writeConfig(0, "userLisr", addQQ, true);
				}
				else {
					string userList = readConfig(0, "userList");
					string_replace(userList, "@" + addQQ + "@", "");
					writeConfig(0, "userLisr", userList);
				}
				outMsg.append("�ѽ�" + addQQ + msg.substr(0, 6) + "�б������Է���\"");
				outMsg.append(msg.substr(0, 6) == "��ͨ" ? "�Ƴ�" : "��ͨ");
				outMsg.append(addQQ);
				outMsg.append("\"������");
				outMsg.append(msg.substr(0, 6) == "��ͨ" ? "�Ƴ�" : "��ͨ");
				outMsg.append("Ȩ��");
				CQ_sendPrivateMsg(ac, fromQQ, addQQ.c_str());
				return EVENT_BLOCK;
			}
			else {
				outMsg.append("�յ�һ������Ϊ�������Ϣ[");
				outMsg.append(msg);
				outMsg.append("]����Ϊ������������");
				CQ_addLog(ac, CQLOG_DEBUG, "ִ��", outMsg.c_str());
				return EVENT_IGNORE;
			}
		}
	}
	return EVENT_IGNORE;
}


/*
* Type=2 Ⱥ��Ϣ
*/
CQEVENT(int32_t, __eventGroupMsg, 36)(int32_t subType, int32_t msgId, int64_t fromGroup, int64_t fromQQ, const char *fromAnonymous, const char *msg, int32_t font) {

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}

/*
* �˵������� .json �ļ������ò˵���Ŀ��������
* �����ʹ�ò˵������� .json ���˴�ɾ�����ò˵�
*/
CQEVENT(int32_t, __menuA, 0)() {
	MessageBoxA(NULL, "����menuA�����������봰�ڣ����߽�������������", "" ,0);
	return 0;
}

CQEVENT(int32_t, __menuB, 0)() {
	MessageBoxA(NULL, "����menuB�����������봰�ڣ����߽�������������", "" ,0);
	return 0;
}