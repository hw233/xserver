#include <signal.h>
#include <assert.h>
#include <search.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#include <map>
#include <pthread.h>
#include <string.h>
#include "game_event.h"
#include "conn_node_loginsrv.h"
#include "deamon.h"
#include "oper_config.h"
#include "mem_pool.h"
#include "mysql_module.h"
#include "cgi_common.h"
#include "login_config.h"

static void cb_signal2(evutil_socket_t fd, short events, void *arg)
{
//	exit(0);
	change_mycat();
	LOG_INFO("%s: fd = %d, events = %d, arg = %p", __FUNCTION__, fd, events, arg);
	
}

int init_conn_client_map()
{
	return (0);
}

void* server_callback(void* lparam);

//const char* const filterPlayerConfig="./filter.open_id.file.name";

#if 0
static void load_filter_config() {
	std::ifstream inFile;

	inFile.open(filterPlayerConfig);
	if (!inFile.good())
		return;

	char buffer[1024];
	while (inFile.getline(buffer, sizeof(buffer))) {
		if (strlen(buffer) == 0)
			continue;

		if (strstr(buffer, "//") || strstr(buffer, "#"))
			continue;

		uint32_t open_id = strtoul(buffer, NULL, 0);

		filterPlayerList.insert(open_id);
	}
	
	
	inFile.close();
}
#endif

int main(int argc, char **argv)
{
	int ret = 0;
//	FILE *file = NULL;
	std::ifstream file;
	char *line;
	int port;
	int i;
	struct sockaddr_in sin;
	signal(SIGTERM, SIG_IGN);

	std::string szMysqlIp;
	std::string szMysqlDbName;
	std::string szMysqlDbUser;
	std::string szMysqlDbPwd;
	int nMysqlPort = 0;
//	pthread_t thread=0;
	
	ret = log4c_init();
	if (ret != 0) {
		printf("log4c_init failed[%d]\n", ret);
		return (ret);
	}
//	mycat = log4c_category_get("six13log.log.app.application1");
	init_mycat();	
	if (!mycat) {
		printf("log4c_category_get(\"six13log.log.app.application1\"); failed\n");
		return (0);
	}
	for (i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-d") == 0) {
			change_to_deamon();
//			break;
		}
	    else if(strcmp(argv[i], "-t") == 0) { /// test for mem check
			open_err_log_file();
		}
	}

//	load_filter_config();

	uint64_t pid = write_pid_file();		
    LOG_INFO("login_srv run %lu", pid);
	if (init_conn_client_map() != 0) {
		LOG_ERR("init client map failed");
		goto done;
	}

	ret = game_event_init();
	if (ret != 0)
		goto done;

	file.open("../server_info.ini", std::ifstream::in);
	if (!file.good()) {
		LOG_ERR("open server_info.ini failed[%d]", errno);				
		ret = -1;
		goto done;
	}

	line = get_first_key(file, (char *)"mysql_host");
	if (!line) {
		LOG_ERR("[%s : %d]: get config failed, key: mysql_host", __FUNCTION__, __LINE__);
		return -1;
	}
	szMysqlIp = (get_value(line));

	line = get_first_key(file, (char *)"mysql_port");
	if (!line) {
		LOG_ERR("[%s : %d]: get config failed, key: mysql_port", __FUNCTION__, __LINE__);
		return -1;
	}
	nMysqlPort = atoi(get_value(line));

	line = get_first_key(file, (char *)"mysql_db_name");
	if (!line) {
		LOG_ERR("[%s : %d]: get config failed, key: mysql_db_name", __FUNCTION__, __LINE__);
		return -1;
	}
	szMysqlDbName = (get_value(line));

	line = get_first_key(file, (char *)"mysql_db_user");
	if (!line) {
		LOG_ERR("[%s : %d]: get config failed, key: mysql_db_user", __FUNCTION__, __LINE__);
		return -1;
	}
	szMysqlDbUser = (get_value(line));

	line = get_first_key(file, (char *)"mysql_db_pwd");
	if (!line) {
		LOG_ERR("[%s : %d]: get config failed, key: mysql_db_pwd", __FUNCTION__, __LINE__);
		return -1;
	}
	szMysqlDbPwd = (get_value(line));

	ret = init_db(const_cast<char*>(szMysqlIp.c_str()), nMysqlPort,  const_cast<char*>(szMysqlDbName.c_str())
		, const_cast<char*>(szMysqlDbUser.c_str()), const_cast<char*>(szMysqlDbPwd.c_str()));
	if (0 != ret) {
		LOG_ERR("[%s : %d]: init db failed, ip: %s, port: %u, abname: %s, user name: %s, pwd: %s",
			__FUNCTION__, __LINE__, szMysqlIp.c_str(), nMysqlPort, szMysqlDbName.c_str(), szMysqlDbUser.c_str(), szMysqlDbPwd.c_str());
		return -1;
	}	

	line = get_first_key(file, (char *)"conn_srv_login_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_srv_login_port");
		ret = -1;
		goto done;
	}

	line = get_first_key(file, (char *)"game_srv_id");
	sg_server_id = (uint64_t )strtoull(get_value(line), NULL, 0);
	if (sg_server_id <= 0) {
		LOG_ERR("config file wrong, no game_srv_id");
		ret = -1;
		goto done;
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	ret = evutil_inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr);
	if (ret != 1) {
		LOG_ERR("evutil_inet_pton failed[%d]", ret);		
		return (-1);		
	}

	//ret = pthread_create(&thread, NULL, &server_callback, NULL);
	//if (0 != ret) {
	//	LOG_ERR("[%s : %d]: create thread failed");
	//	return -1;
	//}

	ret = game_add_connect_event((struct sockaddr *)&sin, sizeof(sin), &conn_node_loginsrv::connecter);
	if (ret <= 0)
		goto done;

//	fclose(file);
	file.close();

	read_all_excel_data();
	//sg_event_timer.ev_callback = cb_on_timer;

	//add_timer(sg_timeout, &sg_event_timer, NULL);

	if (SIG_ERR == signal(SIGPIPE,SIG_IGN)) {
		LOG_ERR("set sigpipe ign failed");		
		return (0);
	}
	add_signal(SIGUSR2, NULL, cb_signal2);		
	
	ret = event_base_loop(base, 0);
	LOG_INFO("event_base_loop stoped[%d]", ret);	

	struct timeval tv;
	event_base_gettimeofday_cached(base, &tv);

done:
	LOG_INFO("login_srv stoped[%d]", ret);
	return (ret);
}

void get_cur_server_max_conn() {
	
	get_server_max_conn(sg_server_id, sg_max_conn, sg_server_key);

	LOG_INFO("[%s: %d]: get cur server max conn success! server_id: %u, max_conn: %u", __FUNCTION__, __LINE__, sg_server_id, sg_max_conn);
}

struct curlReadData 
{
	char*		data;
	uint32_t	pos;

	curlReadData() 
		: data(NULL)
		, pos(0)
	{ }
};

static inline size_t __filter_curl_process_data__(void *buffer, size_t size, size_t nmemb, void *userdata) {
	curlReadData* curlData = (curlReadData*)userdata;

	memcpy(curlData->data + curlData->pos, buffer, size*nmemb);
	curlData->pos += size*nmemb;

	return size * nmemb;
}

void get_server_filter_list() {
	char szUrl[1024] = { 0 };
	curlReadData curlRead;	
	static char read_buffer[1024*1024*10]; 
	curlRead.data = read_buffer;

	if (sg_server_key=="") {
		snprintf(szUrl, sizeof(szUrl), "http://ghzlist.ycatgame.com:8080/newweb/game_manage/web/disable_account.php");
	} else {
		snprintf(szUrl, sizeof(szUrl), "http://ghzlist.ycatgame.com:8080/newweb/game_manage/web/disable_account.php?key=%s", sg_server_key.c_str());
	}

	CURL* curl_obj = curl_easy_init();	
	if (!curl_obj) {
		printf ("{\"code\": 1, \"reason\": \"call curl_easy_init failed\"}");
		return;	
	}

	curl_easy_setopt(curl_obj, CURLOPT_URL, szUrl);
	curl_easy_setopt(curl_obj, CURLOPT_TIMEOUT, 5);
	curl_easy_setopt(curl_obj, CURLOPT_WRITEFUNCTION, &__filter_curl_process_data__);
	curl_easy_setopt(curl_obj, CURLOPT_WRITEDATA, &curlRead);

	int ret = curl_easy_perform(curl_obj);
	if (0 != ret) {
		LOG_ERR ("[%s : %d]: curl perform failed!", __FUNCTION__, __LINE__);
		curl_easy_cleanup(curl_obj);
		return;	
	}

	curl_easy_cleanup(curl_obj);
	read_buffer[curlRead.pos] = '\0';

	json_object *new_obj = json_tokener_parse(read_buffer);
	if (!new_obj ) {
		LOG_ERR ("[%s : %d]: prase json failed!", __FUNCTION__, __LINE__);
		return;
	}

	json_type  jtype = json_object_get_type(new_obj);
	if (json_type_array != jtype) {
		LOG_ERR ("[%s : %d]: json type not array type, type: %d", __FUNCTION__, __LINE__, jtype);
		return;
	}

	sg_filter_list.clear();
	int len = json_object_array_length(new_obj);
	if (len <= 0) {
		LOG_ERR ("[%s : %d]: json  array size<=0, len: %d", __FUNCTION__, __LINE__, len);
		return;
	}

	sg_filter_map_locker = 1;
	for (int i=0; i<len; ++i) {
		json_object *obj = json_object_array_get_idx(new_obj, i);
		if (!obj)
			break;

		std::map<std::string, std::string>  jsonMap;
		json_object_object_foreach(obj, key, val) {
			jsonMap[std::string(key)] = std::string(json_object_to_json_string(val));
		}

		std::map<std::string, std::string>::iterator it;

		it = jsonMap.find("openId");
		if (it == jsonMap.end()) {
			continue;
		}

		std::string szTmpPort;
		szTmpPort.assign(it->second.c_str()+1, it->second.length()-2);
		uint32_t open_id = strtoul(szTmpPort.c_str(), NULL, 0);

		it = jsonMap.find("day");
		if (it==jsonMap.end()) 
			continue;

		szTmpPort.assign(it->second.c_str()+1, it->second.length()-2);
		uint32_t day = strtoul(szTmpPort.c_str(), NULL, 0);

		it = jsonMap.find("updateTime");
		if (it==jsonMap.end()) 
			continue;

		szTmpPort.assign(it->second.c_str()+1, it->second.length()-2);
		uint32_t ts = strtoul(szTmpPort.c_str(), NULL, 0);

		sg_filter_list[open_id] = std::make_pair(day, ts);
	}
	sg_filter_map_locker = 0;

	json_object_put(new_obj);
}

void* server_callback(void* lparam) {
	while (true) {
		get_cur_server_max_conn();
		
		get_server_filter_list();
		
		usleep(30000000);
	}

	return NULL;
}

