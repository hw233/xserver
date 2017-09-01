#include "cgi_common.h"
#include "login.pb-c.h"
#include "tea.h"

static int get_server_account_url(uint32_t open_id)
{
	char szUrl[1024] = { 0 };
	char outStr[2048] = { 0 };

	memset(outStr, 0, sizeof(outStr));
	snprintf(szUrl, sizeof(szUrl), "http://192.168.2.114/cgi-bin/cgi_get_account_server?open_id=%u", open_id);

	int ret = 0;
	CURL* curl_obj = NULL;
	json_object *json_obj = NULL;
	do
	{
		curl_obj = curl_easy_init();	
		if (!curl_obj)
		{
//			LOG_ERR("[%s:%d] curl_easy_init failed, open_id:%u", __FUNCTION__, __LINE__, open_id);
			break;
		}

		curl_easy_setopt(curl_obj, CURLOPT_URL, szUrl);
		curl_easy_setopt(curl_obj, CURLOPT_TIMEOUT, 5);
		curl_easy_setopt(curl_obj, CURLOPT_WRITEFUNCTION, &__curl_process_data__);
		curl_easy_setopt(curl_obj, CURLOPT_WRITEDATA, outStr);

		ret = curl_easy_perform(curl_obj);
		if (0 != ret)
		{
//			LOG_ERR("[%s:%d] curl_easy_perform failed, open_id:%u", __FUNCTION__, __LINE__, open_id);
			break;
		}

//		curl_easy_cleanup(curl_obj);

		json_obj = json_tokener_parse(outStr);
		if (!json_obj)
		{
//			LOG_ERR("[%s:%d] json_tokener_parse failed, open_id:%u", __FUNCTION__, __LINE__, open_id);
			break;
		}

		std::map<std::string, std::string>  jsonMap;
		json_object_object_foreach(json_obj, key, val) 
		{
			jsonMap[std::string(key)] = std::string(json_object_to_json_string(val));
		}

		std::map<std::string, std::string>::iterator it;
		std::string szValue;

		it = jsonMap.find("code");
		if (it == jsonMap.end())
		{
			break;
		}

		int code = atoi(szValue.c_str());
		if (code == 0)
		{
			json_object *data_obj = NULL;
			if (json_object_object_get_ex(json_obj, "data", &data_obj))
			{
				char decode_buffer[64*1024];
				int data_len = sg_base64_decode(json_object_get_string(data_obj), 0, decode_buffer);
				//int data_len = json_object_get_string_len(data_obj);
				AccountServerListAnswer *resp = account_server_list_answer__unpack(NULL, data_len, (uint8_t*)decode_buffer);
				if (resp)
				{
					printf("resp.n_serverlist:%lu\n", resp->n_serverlist);
					for (size_t j = 0; j < resp->n_serverlist; ++j)
					{
						printf("server[%u] n_playerlist:%lu, last_login_time:%u\n", resp->serverlist[j]->serverid, resp->serverlist[j]->n_playerlist, resp->serverlist[j]->lastlogintime);
						for (size_t k = 0; k < resp->serverlist[j]->n_playerlist; ++k)
						{
							PlayerBaseInfo* player = resp->serverlist[j]->playerlist[k];
							printf("[%lu]player[%lu][%s] ", k, player->playerid, player->name);
							for (size_t l = 0; l < player->n_attrid; ++l)
							{
								printf("attr[%u:%u] ", player->attrid[l], player->attrval[l]);
							}
							printf("\n");
						}
					}
				}
			}
		}
	} while(0);
	
	if (curl_obj)
	{
		curl_easy_cleanup(curl_obj);
	}
	if (json_obj)
	{
		json_object_put(json_obj);
	}

	return 0;
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("please input param [0]=open_id\n");
		return 0;
	}
	get_server_account_url(atoi(argv[1]));
	return 0;
}
