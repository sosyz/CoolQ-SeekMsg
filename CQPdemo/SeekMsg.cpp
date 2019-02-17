#include <string>
#include <Windows.h>
#include <time.h>

#include "cqp.h"
#include "SeekMsg.h"
#include "sqlite3.h"
using namespace std;

//ȡ����Ŀ¼
string GetExePath(void){
	char szFilePath[MAX_PATH + 1] = { 0 };
	GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
	(strrchr(szFilePath, '\\'))[0] = 0; // ɾ���ļ�����ֻ���·���ִ�  
	string path = szFilePath;

	return path;
}

//ȡ��������ַ���
char *randstr(char *str, const int len){
	srand((int)time(NULL));
	int i;
	for (i = 0; i < len; ++i)
	{
		switch ((rand() % 3))
		{
		case 1:
			str[i] = 'A' + rand() % 26;
			break;

		case 2:
			str[i] = 'a' + rand() % 26;
			break;
		default:
			str[i] = '0' + rand() % 10;
			break;
		}
	}
	str[++i] = '\0';
	return str;
}

//gbkתUTF-8
string GbkToUtf8(const std::string& strGbk){
	//gbkתunicode
	int len = MultiByteToWideChar(CP_ACP, 0, strGbk.c_str(), -1, NULL, 0);
	wchar_t *strUnicode = new wchar_t[len];
	wmemset(strUnicode, 0, len);
	MultiByteToWideChar(CP_ACP, 0, strGbk.c_str(), -1, strUnicode, len);

	//unicodeתUTF-8
	len = WideCharToMultiByte(CP_UTF8, 0, strUnicode, -1, NULL, 0, NULL, NULL);
	char * strUtf8 = new char[len];
	WideCharToMultiByte(CP_UTF8, 0, strUnicode, -1, strUtf8, len, NULL, NULL);

	std::string strTemp(strUtf8);//��ʱ��strTemp��UTF-8����
	delete[]strUnicode;
	delete[]strUtf8;
	strUnicode = NULL;
	strUtf8 = NULL;
	return strTemp;
}

//UTF-8תgbk
string Utf8ToGbk(const std::string& strUtf8){
	//UTF-8תunicode
	int len = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, NULL, 0);
	wchar_t * strUnicode = new wchar_t[len];//len = 2
	wmemset(strUnicode, 0, len);
	MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, strUnicode, len);

	//unicodeתgbk
	len = WideCharToMultiByte(CP_ACP, 0, strUnicode, -1, NULL, 0, NULL, NULL);
	char *strGbk = new char[len];//len=3 ����Ϊ2������char*�����Զ�������\0
	memset(strGbk, 0, len);
	WideCharToMultiByte(CP_ACP, 0, strUnicode, -1, strGbk, len, NULL, NULL);

	std::string strTemp(strGbk);//��ʱ��strTemp��GBK����
	delete[]strUnicode;
	delete[]strGbk;
	strUnicode = NULL;
	strGbk = NULL;
	return strTemp;
}

//���ı��滻
void string_replace(std::string &strBig, const std::string &strsrc, const std::string &strdst)
{
	std::string::size_type pos = 0;
	std::string::size_type srclen = strsrc.size();
	std::string::size_type dstlen = strdst.size();

	while ((pos = strBig.find(strsrc, pos)) != std::string::npos)
	{
		strBig.replace(pos, srclen, strdst);
		pos += dstlen;
	}
}

//��ѯ��¼
string seekMsg(const string Group, const string QQ, const string Num, const string Time, string& json) {
	string timeStart = "";
	string timeEnd = "";
	/*
	sql::Database log;
	string logFile = GetExePath() + "\\data\\" + to_string(CQ_getLoginQQ(ac)) + "\\logv1.db";
	log.open(GbkToUtf8(logFile));
	sql::Table tbLog(log.getHandle(), "log", definition_tbLog);
	tbLog.open("ID = (SELECT MAX(ID) FROM log)");
	for (int index = 0; index < tbLog.recordCount(); index++)
	{
		if (sql::Record *record = tbLog.getRecord(index))
		{
			sql::Value* value = record->getValue("detail");
			CQ_addLog(ac, CQLOG_DEBUG, "SELECT", Utf8ToGbk(value->toString()).c_str());
		}
	}
	*/


}