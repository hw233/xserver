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
#include "conn_node_ranksrv.h"
#include "deamon.h"
#include "oper_config.h"
#include "mem_pool.h"
#include "redis_client.h"

static void cb_signal2(evutil_socket_t fd, short events, void *arg)
{
//	exit(0);
	change_mycat();
	LOG_INFO("%s: fd = %d, events = %d, arg = %p", __FUNCTION__, fd, events, arg);
	
}

extern CRedisClient sg_redis_client;
extern uint32_t sg_server_id;
extern struct event sg_clear_timer_event;
extern struct timeval sg_clear_timer_val;
extern void cb_clear_timeout(evutil_socket_t, short, void* /*arg*/);
extern void init_redis_keys(uint32_t server_id);

int main(int argc, char **argv)
{
	int ret = 0;
	std::ifstream file;
	char *line;
	int port;
	int i;
	struct sockaddr_in sin;
	signal(SIGTERM, SIG_IGN);
/*
	std::string szMysqlIp;
	std::string szMysqlDbName;
	std::string szMysqlDbUser;
	std::string szMysqlDbPwd;
	int nMysqlPort = 0;
*/	

	int timeout;
	char *ip;
	ret = log4c_init();
	if (ret != 0) {
		printf("log4c_init failed[%d]\n", ret);
		return (ret);
	}

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

	uint64_t pid = write_pid_file();		
    LOG_INFO("rank_srv run %lu", pid);

	ret = game_event_init();
	if (ret != 0)
		goto done;

	file.open("../server_info.ini", std::ifstream::in);
	if (!file.good()) {
		LOG_ERR("open server_info.ini failed[%d]", errno);				
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
	
	line = get_first_key(file, (char *)"conn_redis_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_redis_port");
		ret = -1;
		goto done;
	}
	line = get_first_key(file, (char *)"conn_redis_timeout");
	timeout = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_redis_timeout");
		ret = -1;
		goto done;
	}
	line = get_first_key(file, (char *)"conn_redis_ip");
	ip = get_value(line);
	if (!ip) {
		LOG_ERR("config file wrong, no conn_redis_ip");
		ret = -1;
		goto done;
	}
	if (sg_redis_client.connect(ip, port, timeout) != 0)
	{
		LOG_ERR("connect redis %s %d faild", ip, port);
		ret = -1;
		goto done;
	}

	line = get_first_key(file, (char *)"conn_srv_rank_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_srv_rank_port");
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

	ret = game_add_connect_event((struct sockaddr *)&sin, sizeof(sin), &conn_node_ranksrv::connecter);
	if (ret <= 0)
		goto done;

	file.close();

	if (SIG_ERR == signal(SIGPIPE,SIG_IGN)) {
		LOG_ERR("set sigpipe ign failed");		
		return (0);
	}
	add_signal(SIGUSR2, NULL, cb_signal2);		

	init_redis_keys(sg_server_id);

	sg_clear_timer_val.tv_sec = 3600;
	sg_clear_timer_event.ev_callback = cb_clear_timeout;
	add_timer(sg_clear_timer_val, &sg_clear_timer_event, NULL);
	
	ret = event_base_loop(base, 0);
	LOG_INFO("event_base_loop stoped[%d]", ret);	

	struct timeval tv;
	event_base_gettimeofday_cached(base, &tv);

done:
	LOG_INFO("rank_srv stoped[%d]", ret);
	return (ret);
}

