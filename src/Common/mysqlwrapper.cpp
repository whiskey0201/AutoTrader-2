
#ifdef WIN32
#include <winsock2.h>
#endif
#include "mysqlwrapper.h"

mysql_db::mysql_db(){
	mysql_init(&sqlCon);
}

mysql_db::~mysql_db(){
	mysql_close(&sqlCon);
}

int mysql_db::mysql_open(const char * host, const char * user, const char * password, const char * database, unsigned int port)
{
	char nvalue = 1;
	mysql_options(&sqlCon, MYSQL_OPT_RECONNECT, (char *)&nvalue);
	mysql_options(&sqlCon, MYSQL_SET_CHARSET_NAME, "utf8");
	if (!mysql_real_connect(&sqlCon, host, user, password, database, port, NULL, 0))
		return -1;
	return 1;
}

int mysql_db::mysql_noResult_query(const char * sql)
{
	if (mysql_query(&sqlCon, sql) != 0)
		return -1;
	return (int)mysql_affected_rows(&sqlCon);
}

int mysql_db::mysql_select_query(const char * sql, std::map<int, std::vector<std::string>> & map_results)
{

	if (mysql_query(&sqlCon, sql) != 0)
		return -1;

	if (!(m_pResult = mysql_use_result(&sqlCon)))
		return -1;
	int i = 0;

	int count = mysql_num_fields(m_pResult);
	while (m_Row = mysql_fetch_row(m_pResult))
	{
		std::vector<std::string> vVal;
		for (int j = 0; j<count; j++)
			vVal.push_back(m_Row[j]);
		map_results[i++] = vVal;
	}
	mysql_free_result(m_pResult);
	return i;
}

int mysql_db::mysql_select_SingleLine_query(const char * sql, std::vector<std::string> & v_results)
{
	if (mysql_query(&sqlCon, sql) != 0)
		return -1;
	if (!(m_pResult = mysql_use_result(&sqlCon)))
		return -1;
	int i = 0;

	int count = mysql_num_fields(m_pResult);
	while (m_Row = mysql_fetch_row(m_pResult))
	{
		for (int j = 0; j<count; j++)
			v_results.push_back(m_Row[j]);
	}
	mysql_free_result(m_pResult);
	return i;
}

std::string mysql_db::mysql_lasterror()
{
	return mysql_error(&sqlCon);
}