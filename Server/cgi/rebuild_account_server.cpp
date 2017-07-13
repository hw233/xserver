#include "cgi_common.h"
#include "player_db.pb-c.h"

static const char* scg_user_conf = "./user.conf";

struct MysqlDBInfo
{
	uint32_t server_id;
	char host[128];
	uint32_t port;
	char user[128];
	char pwd[128];
	char db_name[128];
};

int main(void)
{
	int result = 0;
	char sql[1024];
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;
	unsigned long *lengths = NULL;
	do
	{
		result = init_mysql(scg_user_conf);
		if (0 != result)
		{
			break;
		}

		sprintf(sql, "select mysql_host, mysql_port, mysql_user, mysql_pwd, mysql_dbname, server_id from servers ");
		res = query(sql, 0, NULL);
		if (res == NULL)
		{
			printf("{\"code\":4, \"reason\":\"%s\"}", mysql_error());
			break;
		}

		std::vector<MysqlDBInfo> server_dbs;
		while(true)
		{
			row = fetch_row(res);
			if (row == NULL)
			{
				break;
			}

			lengths = mysql_fetch_lengths(res);

			MysqlDBInfo info;
			info.server_id = atoi(row[5]);
			memcpy(info.host, row[0], lengths[0]);
			info.host[lengths[0]] = '\0';
			info.port = atoi(row[1]);
			memcpy(info.user, row[2], lengths[2]);
			info.user[lengths[2]] = '\0';
			memcpy(info.pwd, row[3], lengths[3]);
			info.pwd[lengths[3]] = '\0';
			memcpy(info.db_name, row[4], lengths[4]);
			info.db_name[lengths[4]] = '\0';
			server_dbs.push_back(info);
		}
		free_query(res);
		close_db();

		std::map<uint32_t, std::vector<uint32_t> > server_accounts;
		for (std::vector<MysqlDBInfo>::iterator iter = server_dbs.begin(); iter != server_dbs.end(); ++iter)
		{
			if (strcmp(iter->host, "127.0.0.1") != 0)
			{
				continue;
			}
			if (init_db(iter->host, iter->port, iter->db_name, iter->user, iter->pwd) != 0)
			{
				continue;
			}

			sprintf(sql, "SELECT distinct(open_id) from player");
			res = query(sql, 1, NULL);
			if (res == NULL)
			{
				close_db();
				continue;
			}

			server_accounts[iter->server_id].clear();
			while(true)
			{
				row = fetch_row(res);
				if (row == NULL)
				{
					break;
				}

				lengths = mysql_fetch_lengths(res);
				server_accounts[iter->server_id].push_back(atoi(row[0]));
			}

			free_query(res);
			close_db();
		}

		result = init_mysql(scg_user_conf);
		if (0 != result)
		{
			break;
		}

		for (std::map<uint32_t, std::vector<uint32_t> >::iterator iter_map = server_accounts.begin(); iter_map != server_accounts.end(); ++iter_map)
		{
			for (std::vector<uint32_t>::iterator iter_vec = iter_map->second.begin(); iter_vec != iter_map->second.end(); ++iter_vec)
			{
				sprintf(sql, "insert into account_server(server_id, open_id) values (%u, %u)", iter_map->first, *iter_vec);
				query(sql, 1, NULL);
			}
		}

		close_db();
	} while(0);

	return 0;
}

