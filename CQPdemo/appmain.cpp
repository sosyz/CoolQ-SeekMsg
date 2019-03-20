#include "stdafx.h"
#include "cqp.h"
#include "appmain.h"
#include "SeekMsg.h"
#include "sqlite3/sqlite3.h"

#include <string>
#include <time.h>
#include<regex>

using namespace std;
int ac = -1; //AuthCode 调用酷Q的方法时需要用到
string appDirectory;
string cfgDirectory;
string rootKey = "";
sqlite3* pDB;

//对于字符串为0时无法判断
bool AllisNum(string str)
{
	return strtol(str.c_str(), NULL, NULL) != 0;
}

bool writeConfig(int64_t QQ, string name, string value, bool isAdd = false) {
	sqlite3_stmt* stmt = NULL;
	string sql = "";
	string data = "";

	//判断是否追加内容
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
* 返回应用的ApiVer、Appid，打包后将不会调用
*/
CQEVENT(const char*, AppInfo, 0)() {
	return CQAPPINFO;
}


/* 
* 接收应用AuthCode，酷Q读取应用信息后，如果接受该应用，将会调用这个函数并传递AuthCode。
* 不要在本函数处理其他任何代码，以免发生异常情况。如需执行初始化代码请在Startup事件中执行（Type=1001）。
*/
CQEVENT(int32_t, Initialize, 4)(int32_t AuthCode) {
	ac = AuthCode;
	return 0;
}


/*
* Type=1001 酷Q启动
* 无论本应用是否被启用，本函数都会在酷Q启动后执行一次，请在这里执行应用初始化代码。
* 如非必要，不建议在这里加载窗口。（可以添加菜单，让用户手动打开窗口）
*/
CQEVENT(int32_t, __eventStartup, 0)() {
	appDirectory = CQ_getAppDirectory(ac);
	setAuthCode(ac);
	cfgDirectory = appDirectory + "config.db";
	CQ_addLog(ac, CQLOG_DEBUG, "runName", GetRunningPath().c_str());
	return 0;
}


/*
* Type=1002 酷Q退出
* 无论本应用是否被启用，本函数都会在酷Q退出前执行一次，请在这里执行插件关闭代码。
* 本函数调用完毕后，酷Q将很快关闭，请不要再通过线程等方式执行其他代码。
*/
CQEVENT(int32_t, __eventExit, 0)() {

	return 0;
}

/*
* Type=1003 应用已被启用
* 当应用被启用后，将收到此事件。
* 如果酷Q载入时应用已被启用，则在_eventStartup(Type=1001,酷Q启动)被调用后，本函数也将被调用一次。
* 如非必要，不建议在这里加载窗口。（可以添加菜单，让用户手动打开窗口）
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
			CQ_addLog(ac, CQLOG_ERROR, "初始化", "初始化失败，插件无法正常运行，详细错误信息见调试");
		}
		else {
			char temp[9] = { 0 };
			rootKey = randstr(temp, 8);
			content = "检测到是第一次运行，请向机器人私聊发送\"#";
			content.append(rootKey);
			content.append("\"开启设置");
			CQ_addLog(ac, CQLOG_WARNING, "初始化", content.c_str());
		}
		sqlite3_free(error);
	}
	return 0;
}


/*
* Type=1004 应用将被停用
* 当应用被停用前，将收到此事件。
* 如果酷Q载入时应用已被停用，则本函数*不会*被调用。
* 无论本应用是否被启用，酷Q关闭前本函数都*不会*被调用。
*/
CQEVENT(int32_t, __eventDisable, 0)() {
	sqlite3_close(pDB);
	return 0;
}


/*
* Type=21 私聊消息
* subType 子类型，11/来自好友 1/来自在线状态 2/来自群 3/来自讨论组
*/
CQEVENT(int32_t, __eventPrivateMsg, 24)(int32_t subType, int32_t msgId, int64_t fromQQ, const char *fromMsg, int32_t font) {

	//如果要回复消息，请调用酷Q方法发送，并且这里 return EVENT_BLOCK - 截断本条消息，不再继续处理  注意：应用优先级设置为"最高"(10000)时，不得使用本返回值
	//如果不回复消息，交由之后的应用/过滤器处理，这里 return EVENT_IGNORE - 忽略本条消息
	string msg = fromMsg;
	int qqState = strtol(readConfig(fromQQ, "userState").c_str(), NULL, NULL);

	if (qqState != 0) {
		if (msg == "退出查询") {
			string sql = "";
			sql = "DELETE FROM config WHERE `qq` = ";
			sql.append(to_string(fromQQ));
			sql.append(" AND `name` IN ('userState','commandGroup','commandQQ','commandTime','commandNum');");
			execSQL(sql);
			CQ_sendPrivateMsg(ac, fromQQ, "您已退出查询功能");
		}
		
		switch (qqState)
		{
		case 1:
			if (AllisNum(msg)) {
				writeConfig(fromQQ, "userState", "2");
				writeConfig(fromQQ, "commandGroup", msg);
				CQ_sendPrivateMsg(ac, fromQQ, "请发送您要查询的QQ(不限制填发送\"-1\")");
			}
			else {
				CQ_sendPrivateMsg(ac, fromQQ, "错误的参数，原因：非纯数字，请发送QQ（仅数字部分）或发送\"退出\"退出查询");
			}
			break;
		case 2:
			if (AllisNum(msg)) {
				writeConfig(fromQQ, "userState", "3");
				writeConfig(fromQQ, "commandQQ", msg);
				CQ_sendPrivateMsg(ac, fromQQ, "请发送您要查询的数量");
			}
			else {
				CQ_sendPrivateMsg(ac, fromQQ, "错误的参数，原因：非纯数字，请发送QQ（仅数字部分）或发送\"退出\"退出查询");
			}
			break;
		case 3:
			if (AllisNum(msg)) {
				writeConfig(fromQQ, "userState", "4");
				writeConfig(fromQQ, "commandNum", msg);
				CQ_sendPrivateMsg(ac, fromQQ, "请发送您要查询的时间(支持)\"昨天\"、\"今天\"、\"XXXX年X月X日XX时XX分(到或-)XXXX年X月X日XX时XX分\"");
			}
			else {
				CQ_sendPrivateMsg(ac, fromQQ, "错误的参数，原因：非纯数字，请发送QQ（仅数字部分）或发送\"退出\"退出查询");
			} 
			break;
		case 4:{
			sqlite3_stmt* stmt;
			string sql = "";
			string tempGroup;
			string tempNum;
			string tempQQ;
			string start;
			string end;
			sql = "SELECT value FROM config WHERE name IN ('commandGroup', 'commandQQ', 'commandNum') AND `qq` = ? ORDER BY name ASC";
			sqlite3_prepare_v2(pDB, sql.c_str(), sql.length(), &stmt, 0);
			sqlite3_bind_int64(stmt, 1, fromQQ);
			int s = sqlite3_step(stmt);
			if (s == SQLITE_ROW) {
				tempGroup = (char *)sqlite3_column_text(stmt, 0);
				sqlite3_step(stmt);
				tempNum = (char *)sqlite3_column_text(stmt, 0);
				sqlite3_step(stmt);
				tempQQ = (char *)sqlite3_column_text(stmt, 0);
			}
			else {
				CQ_addLog(ac, CQLOG_DEBUG, "READRETURN", to_string(s).c_str());
			}
			sqlite3_finalize(stmt);

			/*
			int tm_sec; // 秒 – 取值区间为[0,59] 

			int tm_min; // 分 - 取值区间为[0,59] 

			int tm_hour;// 时 - 取值区间为[0,23] 

			int tm_mday; // 一个月中的日期 - 取值区间为[1,31] 

			int tm_mon;// 月份（从一月开始，0代表一月） - 取值区间为[0,11]

			int tm_year; // 年份，其值等于实际年份减去1900 

			int tm_wday; // 星期 – 取值区间为[0,6]，其中0代表星期天，1代表星期一，以此类推 

			int tm_yday; // 从每年的1月1日开始的天数 – 取值区间为[0,365]，其中0代表1月1日，1代表1月2日，以此类推 

			int tm_isdst; // 夏令时标识符，实行夏令时的时候，tm_isdst为正。不实行夏令时的进候，tm_isdst为0；不了解情况时，tm_isdst()为负。
			*/
			tm _time;
			int year, month, day, hour, minute, second;
			sscanf_s(msg.c_str(), "%d/%d/%d/ %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
			tm t = GetDayTime(time(NULL));

			//添加时间处理
			if (msg == "今天") {


				_time.tm_year = t.tm_year;
				_time.tm_mon = t.tm_mon;
				_time.tm_mday = t.tm_mday;
				_time.tm_hour = 0;
				_time.tm_min = 0;
				_time.tm_sec = 0;
				_time.tm_isdst = 0;

				start = to_string(mktime(&_time));
				_time.tm_mday = t.tm_mday + 1;
				end = to_string(mktime(&_time));
			}
			else if (msg == "昨天") {
				_time.tm_year = t.tm_year;
				_time.tm_mon = t.tm_mon;
				_time.tm_mday = t.tm_mday - 1;
				_time.tm_hour = 0;
				_time.tm_min = 0;
				_time.tm_sec = 0;
				_time.tm_isdst = 0;

				start = to_string(mktime(&_time));
				_time.tm_mday = t.tm_mday;
				end = to_string(mktime(&_time));
			}
			else {
				smatch result;
				regex pattern("(.+)\\s*(到|-)\\s*(.+)");
				if (regex_match(msg, result, pattern)) {
					string time_;
					time_ = result[1];
					sscanf_s(time_.c_str(), "%d/%d/%d/ %d:%d:%d", &year, &month, &day, &hour, &minute, &second);

					if (year != 0 && month != 0 && day != 0) {
						_time.tm_year = year - 1900;
						_time.tm_mon = month - 1;
						_time.tm_mday = day;
						
						if (hour == 0)hour = 0;
						if (minute == 0)minute = 0;
						if (second == 0)second = 0;

						_time.tm_hour = hour;
						_time.tm_min = minute;
						_time.tm_sec = second;
						_time.tm_isdst = 0;

						start = to_string(mktime(&_time));

						time_ = result[2];
						sscanf_s(time_.c_str(), "%d/%d/%d/ %d:%d:%d", &year, &month, &day, &hour, &minute, &second);

						_time.tm_year = year - 1900;
						_time.tm_mon = month - 1;
						_time.tm_mday = day;

						if (hour == 0)hour = 0;
						if (minute == 0)minute = 0;
						if (second == 0)second = 0;

						_time.tm_hour = hour;
						_time.tm_min = minute;
						_time.tm_sec = second;
						_time.tm_isdst = 0;

						end = to_string(mktime(&_time));

						seekMsg(ac, tempGroup, tempQQ, tempNum, start, end);
					}
					else {
						CQ_sendPrivateMsg(ac, fromQQ, "错误的参数，原因：时间格式错误，请参考提示输入，例如\"2019/1/1 [00:00]到2019/1/2 [00:00]\"查询2019年1月1日到2019年1月2日的记录\
\"[]\"包括的内容意为可留空");
					}
				}
			}
			
			break;
		}
		default:
			CQ_sendPrivateMsg(ac, fromQQ, "功能发生错误，请发送\"退出查询\"来退出功能");
			break;
		}

	}else if (rootKey != "") {
		string_replace(msg, "\r", "");
		string_replace(msg, "\n", "");
		string_replace(msg, " ", "");
		string_replace(msg, "　", "");
		if (msg == "#" + rootKey) {
			writeConfig(0, "tempAdmin", to_string(fromQQ));
			writeConfig(0, "rootState", "1");
			CQ_sendPrivateMsg(ac, fromQQ, "您已进入设置模式，是否将此QQ号设为管理者？发送\"是\"或QQ号");
			return EVENT_BLOCK;
		}

		int rootState = strtol(readConfig(0, "rootState").c_str(), NULL, NULL);
		CQ_addLog(ac, CQLOG_DEBUG, "rootState", to_string(rootState).c_str());
		if (rootState != 0 && msg == "退出") {
			execSQL("DELETE FROM config WHERE qq = 0");
			CQ_sendPrivateMsg(ac, fromQQ, "已退出设置模式");
			return EVENT_BLOCK;
		}

		if (readConfig(0, "tempAdmin") == to_string(fromQQ)) {
			CQ_addLog(ac, CQLOG_DEBUG, "if", "true");
			switch (rootState)
			{
			case 1:
				writeConfig(0, "rootState", "2");
				if (msg == "是") {
					writeConfig(0, "admin", to_string(fromQQ));
					string outMsg;
					outMsg.append("已将");
					outMsg.append(to_string(fromQQ));
					outMsg.append("设为管理员，进入下一项设置");
					CQ_sendPrivateMsg(ac, fromQQ, outMsg.c_str());
					CQ_sendPrivateMsg(ac, fromQQ, "请选择查询权限模式：\n[1]允许所有人进行查询\n[2]仅回复开通查询权限的人");
				}else if(strtol(msg.c_str(), NULL, NULL)) {
					writeConfig(0, "admin", msg);
					string outMsg;
					outMsg.append("已将");
					outMsg.append(msg);
					outMsg.append("设为管理员，进入下一项设置");

					CQ_sendPrivateMsg(ac, fromQQ, outMsg.c_str());
					CQ_sendPrivateMsg(ac, fromQQ, "请选择查询权限模式：\n[1]允许所有人进行查询\n[2]仅回复开通查询权限的人");
				}
				else {
					CQ_sendPrivateMsg(ac, fromQQ, "错误的回复，请重新发送，或者发送\"退出\"退出设置，需要注意的是，此操作会使您无法使用本插件");
				}
				break;
			case 2:
				switch (strtol(msg.c_str(), NULL, NULL))
				{
				case 1:
					delectConfig(0, "rootState");
					writeConfig(0, "mode", "1");
					CQ_sendPrivateMsg(ac, fromQQ, "设置成功，允许所有人进行查询，初始化设置到此结束");
					break;
				case 2:
					delectConfig(0, "rootState");
					delectConfig(0, "tempAdmin");
					writeConfig(0, "mode", "2");
					writeConfig(0, "useList", "");
					rootKey = "";
					//设置结束
					CQ_sendPrivateMsg(ac, fromQQ, "设置成功，仅回复开通查询权限的，您可以发送\"开通[QQ]\"进行添加，如\"开通10000\"，初始化设置到此结束");
					break;
				default:
					CQ_sendPrivateMsg(ac, fromQQ, "错误的回复，请重新发送，或者发送\"退出\"退出设置，需要注意的是，此操作会使您无法使用本插件");
				}
			}
			return EVENT_BLOCK;
		}
	}
	
	if (msg == "进入查询") {
		int mode = strtol(readConfig(0, "mode").c_str(), NULL, NULL);
		CQ_addLog(ac, CQLOG_DEBUG, "SELECTDATA", to_string(mode).c_str());
		switch (mode)
		{
		case 1:
			writeConfig(fromQQ, "userState", "1");
			CQ_sendPrivateMsg(ac, fromQQ, "您已开启查询功能，请发送您要查询的群号");
			break;
		case 2:{string list = readConfig(0, "userList");
		if (list.find("@" + to_string(fromQQ) + "@") == string::npos) {
			CQ_sendPrivateMsg(ac, fromQQ, "抱歉，您没有查询权限，无法使用查询功能");
		}
		else {
			writeConfig(fromQQ, "userState", "1");
			CQ_sendPrivateMsg(ac, fromQQ, "您已开启查询功能，请发送您要查询的群号");
		}
		break; }
		default:
			break;
		}
		return EVENT_BLOCK;
	}
	else if (msg.substr(0, 6) == "开通" || msg.substr(0, 6) == "移除") {
		if (stoll(readConfig(0, "admin")) == fromQQ) {
			string addQQ = msg.substr(6);
			string outMsg;
			if (stoll(addQQ) >= 10000) {
				if (msg.substr(0, 6) == "开通") {
					writeConfig(0, "userLisr", addQQ, true);
				}
				else {
					string userList = readConfig(0, "userList");
					string_replace(userList, "@" + addQQ + "@", "");
					writeConfig(0, "userLisr", userList);
				}
				outMsg.append("已将" + addQQ + msg.substr(0, 6) + "列表，您可以发送\"");
				outMsg.append(msg.substr(0, 6) == "开通" ? "移除" : "开通");
				outMsg.append(addQQ);
				outMsg.append("\"来进行");
				outMsg.append(msg.substr(0, 6) == "开通" ? "移除" : "开通");
				outMsg.append("权限");
				CQ_sendPrivateMsg(ac, fromQQ, addQQ.c_str());
				return EVENT_BLOCK;
			}
			else {
				outMsg.append("收到一条可能为命令的消息[");
				outMsg.append(msg);
				outMsg.append("]，但为错误的命令，忽略");
				CQ_addLog(ac, CQLOG_DEBUG, "执行", outMsg.c_str());
				return EVENT_IGNORE;
			}
		}
	}
	return EVENT_IGNORE;
}


/*
* Type=2 群消息
*/
CQEVENT(int32_t, __eventGroupMsg, 36)(int32_t subType, int32_t msgId, int64_t fromGroup, int64_t fromQQ, const char *fromAnonymous, const char *msg, int32_t font) {

	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}

/*
* 菜单，可在 .json 文件中设置菜单数目、函数名
* 如果不使用菜单，请在 .json 及此处删除无用菜单
*/
CQEVENT(int32_t, __menuA, 0)() {
	MessageBoxA(NULL, "这是menuA，在这里载入窗口，或者进行其他工作。", "" ,0);
	return 0;
}

CQEVENT(int32_t, __menuB, 0)() {
	MessageBoxA(NULL, "这是menuB，在这里载入窗口，或者进行其他工作。", "" ,0);
	return 0;
}