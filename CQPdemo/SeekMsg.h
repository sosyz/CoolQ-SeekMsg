#pragma once

#include <string>
using namespace std;

//ȡ����Ŀ¼
string GetExePath(void);

//ȡ��������ַ���
char *randstr(char *str, const int len);

//gbkתUTF-8
string GbkToUtf8(const std::string& strGbk);

//UTF-8תgbk
string Utf8ToGbk(const std::string& strUtf8);

//��ѯ��¼
string seekMsg(const string Group, const string QQ, const string Num, const string Time, string& json);

//���ı��滻
void string_replace(std::string &strBig, const std::string &strsrc, const std::string &strdst);