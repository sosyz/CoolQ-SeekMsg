#pragma once

using namespace std;

//ȡ����Ŀ¼
string GetRunningPath(void);

//ȡ��������ַ���
char *randstr(char *str, const int len);

//gbkתUTF-8
string GbkToUtf8(const std::string& strGbk);

//UTF-8תgbk
string Utf8ToGbk(const std::string& strUtf8);

//��ѯ��¼
string seekMsg(int ac, string Group, string QQ, string Num, string timeStaat, string timeEnd);

//���ı��滻
void string_replace(std::string &strBig, const std::string &strsrc, const std::string &strdst);