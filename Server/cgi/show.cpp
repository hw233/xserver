#include <stdlib.h>
#include "cgi_common.h"
#include "account_help.h"
#include "mysql_module.h"




int main(void)
{

	char *lenstr = NULL;
	
	int ret = 0;
	char *line;
	int port;
	std::ifstream file;
	file.open("../../server_info.ini", std::ifstream::in);
	if (!file.good()) {
		return -1;
	}

	unsigned long *lengths;
	std::string szMysqlIp;
	std::string szMysqlDbName;
	std::string szMysqlDbUser;
	std::string szMysqlDbPwd;
	int nMysqlPort = 0;

	line = get_first_key(file, (char *)"mysql_host");
	if (!line) {
		return -1;
	}
	szMysqlIp = (get_value(line));

	line = get_first_key(file, (char *)"mysql_port");
	if (!line) {
		return -1;
	}
	nMysqlPort = atoi(get_value(line));

	line = get_first_key(file, (char *)"mysql_db_name");
	if (!line) {
		return -1;
	}
	szMysqlDbName = (get_value(line));

	line = get_first_key(file, (char *)"mysql_db_user");
	if (!line) {
		return -1;
	}
	szMysqlDbUser = (get_value(line));

	line = get_first_key(file, (char *)"mysql_db_pwd");
	if (!line) {
		return -1;
	}
	szMysqlDbPwd = (get_value(line));

	ret = init_db(const_cast<char*>(szMysqlIp.c_str()), nMysqlPort, const_cast<char*>(szMysqlDbName.c_str())
		, const_cast<char*>(szMysqlDbUser.c_str()), const_cast<char*>(szMysqlDbPwd.c_str()));
	if (0 != ret) {
		return -1;
	}
	file.close();

	do
	{
		printf("Content-Type:application/octet-stream\n\n");
		lenstr = getenv("QUERY_STRING");
		if (lenstr == NULL || lenstr[0] == '0' || lenstr[0] == '\0')
		{
			return 0;
		}

		parse_post_data(lenstr, strlen(lenstr) + 1);
		char *pKey = get_post_value(const_cast<char*>("key"));
		if (!pKey)
		{
			return 0;
		}

		MYSQL_RES *res = NULL;
		MYSQL_ROW row;

		char sql[1024 * 64];
		uint32_t key = strtoul(pKey, NULL, 0);
		sprintf(sql, "SELECT type,data from showitem where id = %u", key);

		res = query(sql, 1, NULL);
		if (!res) {
			return 1;
		}
		row = fetch_row(res);
		if (!row) {
			free_query(res);
			return 1;
		}
		lengths = mysql_fetch_lengths(res);
		printf("{\"type\":%s, \"len\":\"%lu\"}", row[0], lengths[1]);
		for (int i = 0; i < lengths[1]; ++i)
		{
			putchar(row[1][i]);
		}
		free_query(res);
	} while (0);

	fflush(stdout);
	return 0;
}

