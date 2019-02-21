#include <string>
#include <Windows.h>
#include <time.h>
#include <regex>
#include <iostream>

#include "cqp.h"
#include "SeekMsg.h"
#include "sqlite3.h"
#include "Unpack.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"


using namespace rapidjson;
using namespace std;

struct msgInfo
{
	string Group;
	string QQ;
	string Msg;
};

tm GetDayTime(time_t time)
{
	struct tm * ptimeDetail;
	struct tm timeDetail;
	ptimeDetail = localtime(&time);
	memcpy(&timeDetail, ptimeDetail, sizeof(tm));
	return timeDetail;
}

unsigned char ToHex(unsigned char x)
{
	return  x > 9 ? x + 55 : x + 48;
}

std::string UrlEncode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		if (isalnum((unsigned char)str[i]) ||
			(str[i] == '-') ||
			(str[i] == '_') ||
			(str[i] == '.') ||
			(str[i] == '~'))
			strTemp += str[i];
		else if (str[i] == ' ')
			strTemp += "+";
		else
		{
			strTemp += '%';
			strTemp += ToHex((unsigned char)str[i] >> 4);
			strTemp += ToHex((unsigned char)str[i] % 16);
		}
	}
	return strTemp;
}


static bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

//base64解码
string base64_decode(string const& encoded_string) {
	static const string base64_chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	string ret;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] = (unsigned char)base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = (unsigned char)base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

	return ret;
}

string GetRunningPath() {
	char* str = 0;
	_get_pgmptr(&str);
	string fileName = str;
	fileName.substr(0, fileName.find_last_of("/\\"));
	return fileName;
}

//取随机长度字符串
char *randstr(char *str, const int len){
	int i;
	srand(time(NULL));
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
void string_replace(std::string &strBig, const std::string &strsrc, const std::string &strdst){
	std::string::size_type pos = 0;
	std::string::size_type srclen = strsrc.size();
	std::string::size_type dstlen = strdst.size();

	while ((pos = strBig.find(strsrc, pos)) != std::string::npos)
	{
		strBig.replace(pos, srclen, strdst);
		pos += dstlen;
	}
}

//取消息类型
int getType(string type) {
	return type == "[↓]群组消息" ? 1 : 2;
}

//根据名取CQ码值
string getValue(string CQStr, char name) {
	smatch result;
	regex pattern(name + "=([^(,|\\])]*)");
	if (regex_match(CQStr, result, pattern)) {
		return result[1];
	}
}

msgInfo getInfo(string detail) {
	smatch result;
	msgInfo info;
	regex pattern("群:\\s*(\\d+)[\\s\\S]*?QQ:\\s*(\\d+)\\s*\\n{0,}([\\s\\S]*)");
	if (regex_match(detail, result, pattern)) {
		info.Group = result[1];
		info.QQ = result[2];
		info.Msg = result[3];
	}
	return info;
}

string getName(int32_t ac, string group, string qq) {
	Unpack u(base64_decode(CQ_getGroupMemberInfoV2(ac, strtol(group.c_str(), NULL, NULL), strtol(qq.c_str(), NULL, NULL), false)));
	u.getLong();
	u.getLong();
	string name = u.getstring();
	string nick = u.getstring();
	if (nick != "")
	{
		return nick;
	}
	else if (name != "") {
		return name;
	}
	else {
		Unpack u(base64_decode(CQ_getStrangerInfo(ac, strtol(qq.c_str(), NULL, NULL), false)));
		u.getLong();
		return u.getstring();
	}


}

string msgToHtml(msgInfo info) {
	return "";
}

//查询记录
string seekMsg(int32_t ac, string Group, string QQ, string Num, string timeStart,string timeEnd) {
	sqlite3* pDB;
	sqlite3_stmt* stmt;
	StringBuffer jsonBuffer;
	Writer<StringBuffer> writer(jsonBuffer);
	string sql = "";
	string logFile = GetRunningPath() + "\\data\\" + to_string(CQ_getLoginQQ(ac)) + "\\logv1.db";
	string detail = "";
	string name = "";
	string msg = "";

	sqlite3_open_v2(logFile.c_str(), &pDB, SQLITE_OPEN_READWRITE, 0);
	
	sql = "SELECT name,detail,time FROM log WHERE detail LIKE ? AND time BETWEEN ? AND ? AND name != '[↓]入群请求(申请)' ORDER BY time ACS";

	detail = "群: " + Group + " %QQ: ";
	detail += QQ;
	detail += "%";

	sqlite3_prepare_v2(pDB, sql.c_str(), sql.length(), &stmt, 0);
	sqlite3_bind_text(stmt, 1, detail.c_str(), detail.length(), 0);
	sqlite3_bind_int64(stmt, 2, strtol(timeStart.c_str(), NULL, NULL));
	sqlite3_bind_int64(stmt, 2, strtol(timeEnd.c_str(), NULL, NULL));
	writer.StartObject();
	writer.Key("version");
	writer.String("1.0.0");
	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		int type = getType(GbkToUtf8((char *)sqlite3_column_text(stmt, 0)));
		msgInfo info = getInfo((char *)sqlite3_column_text(stmt, 1));
		writer.StartArray();
		writer.Key("data");
		switch (type)
		{
		case 1:
			name = GbkToUtf8(getName(ac, info.Group, info.QQ));
			name = UrlEncode(name);
			msg = msgToHtml(info);
			msg = UrlEncode(msg);
			
			writer.Key("type");
			writer.Int(1);
			writer.Key("qq");
			writer.String(info.QQ.c_str());
			writer.Key("name");
			writer.String(name.c_str());
			writer.Key("time");
			writer.String((char *)sqlite3_column_text(stmt, 2));
			writer.Key("msg");
			writer.String(msg.c_str());
			break;
		case 2:
			name = GbkToUtf8(getName(ac, info.Group, info.QQ));
			name = UrlEncode(name);
			msg = msgToHtml(info);
			msg = UrlEncode(msg);

			writer.Key("type");
			writer.Int(2);
			writer.Key("qq");
			writer.String(info.QQ.c_str());
			writer.Key("name");
			writer.String(name.c_str());
			writer.Key("time");
			writer.String((char *)sqlite3_column_text(stmt, 2));
			writer.Key("msg");
			writer.String(msg.c_str());
			break;
		case 3:{
			if (info.Msg == "操作者QQ: " + info.QQ) {
				name = "你";
			}
			else {
				name = getName(ac, info.Group, info.QQ);
			}
			name = GbkToUtf8(name);

			writer.Key("type");
			writer.Int(3);
			writer.Key("qq");
			string tip = "<span><a href=\"mqqapi://card/show_pslcard?src_type=internal&amp;source=sharecard&amp;version=1&amp;uin=";
			tip += info.QQ + "\" target=\"_blank\">";
			tip += name;
			tip += "</a> 加入本群</span>";
			writer.String(tip.c_str());
		}
		default:
			break;
		}
		writer.EndArray();
	}
	writer.EndObject();
	string json = jsonBuffer.GetString();
	int s = sqlite3_step(stmt);
	return "";
}