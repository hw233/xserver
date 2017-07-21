#include <signal.h>
#include <assert.h>
#include <search.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#include <map>
#include "game_event.h"
#include "listen_node_dbsrv.h"
#include "listen_node_dbsrv.h"
#include "oper_config.h"
#include "deamon.h"
#include "mysql_module.h"

int init_conn_client_map()
{
	return (0);
}

static listen_node_dbsrv server_listener;

static void cb_signal2(evutil_socket_t fd, short events, void *arg)
{
//	exit(0);
	change_mycat();
	LOG_INFO("%s: fd = %d, events = %d, arg = %p", __FUNCTION__, fd, events, arg);
	
}

int main(int argc, char **argv)
{
	int ret = 0;
//	FILE *file;
	std::ifstream file;
	char *line;
	int port;
	std::string szMysqlIp;
	std::string szMysqlDbName;
	std::string szMysqlDbUser;
	std::string szMysqlDbPwd;
	int nMysqlPort = 0;


	signal(SIGTERM, SIG_IGN);
	ret = log4c_init();
	init_mycat();	
	if (ret != 0) {
		printf("log4c_init failed[%d]\n", ret);
		return (ret);
	}
	mycat = log4c_category_get("six13log.log.app.application1");
	if (!mycat) {
		printf("log4c_category_get(\"six13log.log.app.application1\"); failed\n");
		return (0);
	}
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-d") == 0) {
			change_to_deamon();
//			break;
		}
	    else if(strcmp(argv[i], "-t") == 0) { /// test for mem check
			open_err_log_file();
		}
	}

	uint64_t pid = write_pid_file();		
    LOG_INFO("db_srv[%lu] run", pid);
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
	
	line = get_first_key(file, (char *)"db_srv_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no db_srv_port");
		ret = -1;
		goto done;
	}


	ret = game_add_listen_event(port, &server_listener);
	if (ret != 0)
		goto done;


	file.close();

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
	LOG_INFO("db_srv stoped[%d]", ret);
	return (ret);
}
