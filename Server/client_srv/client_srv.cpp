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
#include "conn_node_clientsrv.h"
#include "deamon.h"
#include "oper_config.h"
#include "mem_pool.h"
#include "mysql_module.h"
#include "cgi_common.h"
std::vector<conn_node_clientsrv *> vector_person;
uint64_t timer_client_loop_count;
#define run_with_period_client(_ms_) if (timer_client_loop_count % _ms_ == 0)
uint32_t sg_person_num;
uint32_t sg_clientsrv_tick_time;
uint32_t sg_move_or_useskill;
uint32_t sg_player_split;
uint32_t sg_move_type;
uint32_t sg_useskill_type;
uint32_t sg_move_min_time;
uint32_t sg_move_max_time;
static void cb_signal2(evutil_socket_t fd, short events, void *arg)
{
//	exit(0);
	change_mycat();
	LOG_INFO("%s: fd = %d, events = %d, arg = %p", __FUNCTION__, fd, events, arg);
	
}
struct timeval clientsrv_timeout;
struct event sg_clientsrv_timer;

void client_on_timer(evutil_socket_t, short, void* /*arg*/)
{
	add_timer(clientsrv_timeout, &sg_clientsrv_timer, NULL);
	//玩家移动
	if (vector_person.size() != 0)
	{
		for (std::vector<conn_node_clientsrv *>::iterator itr = vector_person.begin(); itr != vector_person.end(); itr++)
		{
			if (sg_move_or_useskill == 0)
			{
				if (sg_move_type == 0)
				{
//					run_with_period_client(1)
					{
						(*itr)->send_move_request();
					}
				}
				else
				{
					run_with_period_client((*itr)->interval_time)
					{
						(*itr)->send_move_request();
					}
				}
				
			}
			else
			{
				if (sg_useskill_type == 0)
				{
					run_with_period_client(100)
					{
					 
						(*itr)->handle_player_useskill_request();
						
					}
					
				}
				else
				{
					run_with_period_client((*itr)->interval_time)
					{
						
						
						(*itr)->handle_player_useskill_request();
						
					}
				}
				
			}
		}
	}

	timer_client_loop_count++;
}


int main(int argc, char **argv)
{
	int ret = 0;
	FILE *file = NULL;
//	std::ifstream file;
	char *line;
	char *ip;
	int port;
	int i;
	struct sockaddr_in sin;
	signal(SIGTERM, SIG_IGN);
	uint32_t openid = 1;
	
/*	std::string szMysqlIp;
	std::string szMysqlDbName;
	std::string szMysqlDbUser;
	std::string szMysqlDbPwd;
	int nMysqlPort = 0;*/
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
			break;
		}
	}

//	load_filter_config();

	uint64_t pid = write_pid_file();		
    LOG_INFO("client_srv run %lu", pid);
/*	if (init_conn_client_map() != 0) {
		LOG_ERR("init client map failed");
		goto done;
	}
*/
	ret = game_event_init();
	if (ret != 0)
		goto done;

	file = fopen("client_info.ini","r");
	if (!file) {
		LOG_ERR("open client_info.ini failed[%d]", errno);				
		ret = -1;
		goto done;
	}
/*
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
*/
	if(!file)
	{
		LOG_ERR("open client_info.ini fail");
		return -1;
	}

	line = get_first_key(file,(char*)"person_num");
	if(line)
	{
		sg_person_num = atoi(get_value(line));
	}

	line = get_first_key(file,(char*)"clientsrv_tick_time");
	if(line)
	{
		sg_clientsrv_tick_time = atoi(get_value(line));
	}

	line = get_first_key(file,(char*)"move_or_useskill");
	if(line)
	{
		sg_move_or_useskill = atoi(get_value(line));
	}

	line = get_first_key(file,(char*)"player_split");
	if (line)
	{
		sg_player_split = atoi(get_value(line));
	}

	line = get_first_key(file,(char*)"move_type");
	if (line)
	{
		sg_move_type = atoi(get_value(line));
	}

	line = get_first_key(file,(char*)"useskill_type");
	if (line)
	{
		sg_useskill_type = atoi(get_value(line));
	}

	line = get_first_key(file,(char*)"move_min_time");
	if (line)
	{
		sg_move_min_time = atoi(get_value(line));
	}

	line = get_first_key(file,(char*)"move_max_time");
	if (line)
	{
		sg_move_max_time = atoi(get_value(line));
	}

	line = get_first_key(file, (char *)"conn_srv_client_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_srv_client_port");
		ret = -1;
		goto done;
	}

	line = get_first_key(file, (char*)"client_ip");
	ip = get_value(line);
	if (!ip)
	{
		LOG_ERR("config file wrong, client_ip");
		ret = -1;
		goto done;
	}
	

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	ret = evutil_inet_pton(AF_INET, ip, &sin.sin_addr);
	if (ret != 1) {
		LOG_ERR("evutil_inet_pton failed[%d]", ret);		
		return (-1);		
	}

	//ret = pthread_create(&thread, NULL, &server_callback, NULL);
	//if (0 != ret) {
	//	LOG_ERR("[%s : %d]: create thread failed");
	//	return -1;
	//}
	vector_person.clear();
	srand(time(NULL));
	while (openid <= sg_person_num)
	{
		conn_node_clientsrv* connectera = new conn_node_clientsrv();
		connectera->open_id = openid;
		connectera->interval_time = rand()%(sg_move_max_time - sg_move_min_time + 1) + sg_move_min_time;
		ret = game_add_connect_event((struct sockaddr *)&sin, sizeof(sin), connectera);
		if (ret <= 0)
			goto done;


		if (SIG_ERR == signal(SIGPIPE, SIG_IGN)) {
			LOG_ERR("set sigpipe ign failed");
			return (0);
		}
		add_signal(SIGUSR2, NULL, cb_signal2);
		
		connectera->send_login_request(openid);
		vector_person.push_back(connectera);
		openid++;
		
	}
	
	sg_clientsrv_timer.ev_callback = client_on_timer;
	//clientsrv_timeout.tv_sec = 3;
	clientsrv_timeout.tv_usec = sg_clientsrv_tick_time;
	add_timer(clientsrv_timeout, &sg_clientsrv_timer, NULL);
	ret = event_base_loop(base, 0);
	LOG_INFO("event_base_loop stoped[%d]", ret);	

	struct timeval tv;
	event_base_gettimeofday_cached(base, &tv);
	fclose(file);
done:
	LOG_INFO("client_srv stoped[%d]", ret);
	return (ret);
}

