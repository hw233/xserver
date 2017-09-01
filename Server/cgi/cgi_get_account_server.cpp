#include "cgi_common.h"
#include "account_help.h"
#include "login.pb-c.h"
#include "player_db.pb-c.h"
#include "attr_id.h"
#include "tea.h"

static const char* scg_user_conf = "./user.conf";
static const uint32_t sg_init_head[] =
{
	105900001,
	105900003,
	105900001,
	105900003,
	105900003,
};

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
	char *lenstr = NULL;
	char sql[1024];
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;
	unsigned long *lengths = NULL;
//	int var = 0;
//	while(var == 0)
//	{
//		sleep(1);
//	}
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

		uint32_t open_id = strtoul(pOpenId, NULL, 0);

		sprintf(sql, "select distinct(server_id) from account_server where open_id = %u", open_id);
		res = query(sql, 0, NULL);
		if (res == NULL)
		{
			printf("{\"code\":4, \"reason\":\"%s\"}", mysql_error());
			break;
		}

		std::vector<uint32_t> server_ids;
		while(true)
		{
			row = fetch_row(res);
			if (row == NULL)
			{
				break;
			}

			server_ids.push_back(atoi(row[0]));
		}
		free_query(res);

		AccountServerListAnswer *resp = (AccountServerListAnswer*)malloc(sizeof(AccountServerListAnswer));
		account_server_list_answer__init(resp);
		std::vector<MysqlDBInfo> server_dbs;
		for (std::vector<uint32_t>::iterator iter = server_ids.begin(); iter != server_ids.end(); ++iter)
		{
			uint32_t server_id = *iter;
			sprintf(sql, "select mysql_host, mysql_port, mysql_user, mysql_pwd, mysql_dbname from servers where server_id = %u", server_id);
			res = query(sql, 0, NULL);
			if (res == NULL)
			{
				continue;
			}

			row = fetch_row(res);
			if (row == NULL)
			{
				free_query(res);
				continue;
			}

			lengths = mysql_fetch_lengths(res);

			MysqlDBInfo info;
			info.server_id = server_id;
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

			free_query(res);
		}

		close_db();

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

			sprintf(sql, "SELECT player_id, job, player_name, lv, comm_data, UNIX_TIMESTAMP(logout_time) from player where open_id = %u", open_id);
			res = query(sql, 1, NULL);
			if (res == NULL)
			{
				close_db();
				continue;
			}

			AccountServerData *pServer = (AccountServerData*)malloc(sizeof(AccountServerData));
			account_server_data__init(pServer);

			pServer->serverid = iter->server_id;
			uint64_t last_login_time = 0;
			while(true)
			{
				row = fetch_row(res);
				if (row == NULL)
				{
					break;
				}

				lengths = mysql_fetch_lengths(res);
				uint64_t logout_time = (row[5] == NULL ? 0 : strtoull(row[5], NULL, 0));
				if (logout_time > last_login_time)
				{
					last_login_time = logout_time;
				}
				uint32_t job = atoi(row[1]);

				PlayerBaseInfo *base_info = (PlayerBaseInfo*)malloc(sizeof(PlayerBaseInfo));
				player_base_info__init(base_info);
				base_info->name = (char*)malloc(lengths[2] + 1);
				base_info->attrid = (uint32_t*)malloc(3 * sizeof(uint32_t));
				base_info->attrval = (uint32_t*)malloc(3 * sizeof(uint32_t));
				pServer->n_playerlist++;
				pServer->playerlist = (PlayerBaseInfo**)realloc(pServer->playerlist, pServer->n_playerlist * sizeof(PlayerBaseInfo*));
				pServer->playerlist[pServer->n_playerlist - 1] = base_info;


				base_info->playerid = strtoull(row[0], NULL, 0);
				memcpy(base_info->name, row[2], lengths[2]);
				base_info->name[lengths[2]] = '\0';
				base_info->attrid[base_info->n_attrid] = PLAYER_ATTR_JOB;
				base_info->attrval[base_info->n_attrval] = job;
				base_info->n_attrid++;
				base_info->n_attrval++;
				base_info->attrid[base_info->n_attrid] = PLAYER_ATTR_LEVEL;
				base_info->attrval[base_info->n_attrval] = atoi(row[3]);
				base_info->n_attrid++;
				base_info->n_attrval++;
				base_info->attrid[base_info->n_attrid] = PLAYER_ATTR_HEAD;
				base_info->attrval[base_info->n_attrval] = (job > 5 ? sg_init_head[0] : sg_init_head[job - 1]);
				base_info->n_attrid++;
				base_info->n_attrval++;

				PlayerDBInfo *db_info = player_dbinfo__unpack(NULL, lengths[4], (uint8_t*)row[4]);
				if (!db_info)
				{
					continue;
				}

				base_info->attrval[2] = db_info->head_icon;
			}

			if (pServer->n_playerlist == 0)
			{
				account_server_data__free_unpacked(pServer, NULL);
			}
			else
			{
				pServer->lastlogintime = last_login_time;
				resp->n_serverlist++;
				resp->serverlist = (AccountServerData**)realloc(resp->serverlist, resp->n_serverlist * sizeof(AccountServerData*));
				resp->serverlist[resp->n_serverlist - 1] = pServer;
			}

			free_query(res);
			close_db();
		}

		uint8_t data_buffer[64*1024];
		int data_len = account_server_list_answer__pack(resp, data_buffer);
		//json_object *json_obj = json_object_new_object();
		char encode_buffer[128*1024];
		sg_base64_encode(data_buffer, data_len, (char*)encode_buffer);
		//json_object_object_add(json_obj, "code", json_object_new_int(0));
		//json_object_object_add(json_obj, "reason", json_object_new_string("OK"));
		//json_object_object_add(json_obj, "data", json_object_new_string(encode_buffer));
		printf("{\"code\":0, \"reason\":\"OK\", \"data\":\"%s\"}", encode_buffer);
		//printf("{\"code\":0, \"reason\":\"OK\", \"data\":\"\"}");
		//printf("%s", json_object_to_json_string(json_obj));

		account_server_list_answer__free_unpacked(resp, NULL);
	} while(0);

	fflush(stdout);
	return 0;
}

