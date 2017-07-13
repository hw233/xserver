#include "cgi_common.h"
#include "account_help.h"

static const char* scg_user_conf = "./user.conf";

int main(void)
{
	int result = 0;
	char *lenstr = NULL;
	uint64_t effect = 0;
	char sql[1024];
	do
	{
		result = init_mysql(scg_user_conf);
		if (0 != result)
		{
			printf("{\"code\":1, \"reason\":\"init db failed\"}");
			break;
		}

		printf("Content-Type:text/html\n\n");
		lenstr = getenv("QUERY_STRING");
		if (lenstr == NULL || lenstr[0] == '0' || lenstr[0] == '\0')
		{
			printf("{\"code\":2, \"reason\":\"not input param\"}");
			break;
		}

		parse_post_data(lenstr, strlen(lenstr) + 1);
		char *pOpenId = get_post_value(const_cast<char*>("open_id"));
		if (!pOpenId)
		{
			printf("{\"code\":3, \"reason\":\"not key open_id\"}");
			break;
		}

		char *pServerId = get_post_value(const_cast<char*>("server_id"));
		if (!pServerId)
		{
			printf("{\"code\":4, \"reason\":\"not key server_id\"}");
			break;
		}

		uint64_t open_id = strtoull(pOpenId, NULL, 0);
		uint32_t server_id = strtoul(pServerId, NULL, 0);

		sprintf(sql, "insert into account_server(open_id, server_id) values (%lu, %u)", open_id, server_id);
		query(sql, 0, &effect);
		if (effect != 1)
		{
			printf("{\"code\":5, \"reason\":\"%s\"}", mysql_error());
			break;
		}

		printf("{\"code\":0, \"reason\":\"OK\"}");
	} while(0);

	fflush(stdout);
	return 0;
}

