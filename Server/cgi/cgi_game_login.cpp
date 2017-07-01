#include <stdlib.h>
#include "cgi_common.h"
#include "account_help.h"


static const char* scg_appkey = "PzN4xoVvh3QOg79yfibUKmDHcswRIu8p";
static const char* scg_user_conf = "/data/release/config/user.conf";

int main(void)
{
	//int result = 1;
	char *lenstr = NULL;

	uint32_t outOpenId = 0;

//	int var = 0;
//	while(var == 0)
//	{
//		sleep(1);
//	}

	do 
	{
		/*result = init_mysql(scg_user_conf);
		if (0 != result)
		{
			printf("{\"code\":1, \"reason\":\"init db failed\"}");
			break;
		}*/

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

		char *pChannel = get_post_value(const_cast<char*>("channel"));
		if (!pChannel)
		{
			printf("{\"code\":4, \"reason\":\"not key channel\"}");
			break;
		}
		uint32_t nChannel = strtoul(pChannel, NULL, 10);

		if (nChannel == 1)
		{
			outOpenId = strtoul(pOpenId, NULL, 10);
			if (outOpenId == 0)
			{
				printf("{\"code\":5, \"reason\":\"open_id invalid\"}");
				break;
			}
		}
		else
		{
			printf("{\"code\":6, \"reason\":\"channel error\"}");
			break;
		}

		std::ostringstream os;
		os << "open_id:" << outOpenId << ",channel:" << nChannel << ",scg_appkey:" << scg_appkey;
		std::string szKey = create_md5_val(os.str());
		printf("{\"code\":0, \"reason\":\"OK\", \"open_id\":\"%u\", \"key\":\"%s\"}", outOpenId, szKey.c_str());

	} while (0);

	fflush(stdout);
	return 0;
}

