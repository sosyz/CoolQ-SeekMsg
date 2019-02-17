#include <string>
#include <Windows.h>
#include <time.h>

#include "cqp.h"
#include "SeekMsg.h"
#include "sqlite3.h"
using namespace std;

//取运行目录
string GetExePath(void){
	char szFilePath[MAX_PATH + 1] = { 0 };
	GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
	(strrchr(szFilePath, '\\'))[0] = 0; // 删除文件名，只获得路径字串  
	string path = szFilePath;

	return path;
}

//取随机长度字符串
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

//gbk转UTF-8
string GbkToUtf8(const std::string& strGbk){
	//gbk转unicode
	int len = MultiByteToWideChar(CP_ACP, 0, strGbk.c_str(), -1, NULL, 0);
	wchar_t *strUnicode = new wchar_t[len];
	wmemset(strUnicode, 0, len);
	MultiByteToWideChar(CP_ACP, 0, strGbk.c_str(), -1, strUnicode, len);

	//unicode转UTF-8
	len = WideCharToMultiByte(CP_UTF8, 0, strUnicode, -1, NULL, 0, NULL, NULL);
	char * strUtf8 = new char[len];
	WideCharToMultiByte(CP_UTF8, 0, strUnicode, -1, strUtf8, len, NULL, NULL);

	std::string strTemp(strUtf8);//此时的strTemp是UTF-8编码
	delete[]strUnicode;
	delete[]strUtf8;
	strUnicode = NULL;
	strUtf8 = NULL;
	return strTemp;
}

//UTF-8转gbk
string Utf8ToGbk(const std::string& strUtf8){
	//UTF-8转unicode
	int len = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, NULL, 0);
	wchar_t * strUnicode = new wchar_t[len];//len = 2
	wmemset(strUnicode, 0, len);
	MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, strUnicode, len);

	//unicode转gbk
	len = WideCharToMultiByte(CP_ACP, 0, strUnicode, -1, NULL, 0, NULL, NULL);
	char *strGbk = new char[len];//len=3 本来为2，但是char*后面自动加上了\0
	memset(strGbk, 0, len);
	WideCharToMultiByte(CP_ACP, 0, strUnicode, -1, strGbk, len, NULL, NULL);

	std::string strTemp(strGbk);//此时的strTemp是GBK编码
	delete[]strUnicode;
	delete[]strGbk;
	strUnicode = NULL;
	strGbk = NULL;
	return strTemp;
}

//子文本替换
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

//查询记录
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