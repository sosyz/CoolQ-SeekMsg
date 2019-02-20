#pragma once

using namespace std;

//取运行目录
string GetRunningPath(void);

//取随机长度字符串
char *randstr(char *str, const int len);

//gbk转UTF-8
string GbkToUtf8(const std::string& strGbk);

//UTF-8转gbk
string Utf8ToGbk(const std::string& strUtf8);

//查询记录
string seekMsg(int ac, string Group, string QQ, string Num, string timeStaat, string timeEnd);

//子文本替换
void string_replace(std::string &strBig, const std::string &strsrc, const std::string &strdst);