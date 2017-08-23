#include <signal.h>
#include "doufachang_config.h"
#include "time_helper.h"
#include "doufachang_util.h"
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
#include "conn_node_doufachangsrv.h"
#include "deamon.h"
#include "oper_config.h"
#include "mem_pool.h"
#include "mysql_module.h"
#include "cgi_common.h"

static void cb_signal2(evutil_socket_t fd, short events, void *arg)
{
//	exit(0);
	change_mycat();
	LOG_INFO("%s: fd = %d, events = %d, arg = %p", __FUNCTION__, fd, events, arg);
	
}

static void	doufachang_check()
{
	uint64_t *player_id = (uint64_t *)malloc(sizeof(uint64_t) * DOUFACHANG_MAX_RANK);
	uint64_t *rank = (uint64_t *)malloc(sizeof(uint64_t) * DOUFACHANG_MAX_RANK);
	uint64_t *rank_check = (uint64_t *)malloc(sizeof(uint64_t) * DOUFACHANG_MAX_RANK);	
	for (int i = 0; i < DOUFACHANG_MAX_RANK; ++i)
	{
		rank[i] = i + 1;
		player_id[i] = 0;
		rank_check[i] = 0;
	}
	sg_redis_client.mget_uint64(conn_node_doufachangsrv::doufachang_rank_key,
		DOUFACHANG_MAX_RANK, rank, player_id);

	bool t = false;
	for (int i = 0; i < DOUFACHANG_MAX_RANK; ++i)
	{
		if (t && player_id[i] != 0)
		{
			LOG_ERR("%s: failed, i = %lu, player_id = %lu", __FUNCTION__, i, player_id[i]);
			break;
		}
		if (player_id[i] == 0)
			t = true;
	}
	sg_redis_client.mget_uint64(conn_node_doufachangsrv::doufachang_rank2_key,
		DOUFACHANG_MAX_RANK, player_id, rank_check);

	for (int i = 0; i < DOUFACHANG_MAX_RANK; ++i)
	{
		if (player_id[i] == 0)
			break;
		if (rank_check[i] != rank[i])
		{
			LOG_ERR("%s: failed, i = %lu, player_id = %lu", __FUNCTION__, i, player_id[i]);
			break;			
		}
	}
	
	free(rank);
	free(player_id);
	free(rank_check);
}

static struct event doufachang_event_timer;
static struct timeval doufachang_timeout = {5, 0};
static void cb_doufachang_timer(evutil_socket_t, short, void* /*arg*/)
{
	uint64_t now = time_helper::get_micro_time();
	if (now >= sg_next_copy_rank_time)
	{
		sg_next_copy_rank_time = time_helper::nextOffsetTime(22 * 3600, now);
		LOG_INFO("%s: next rank time = %lu", __FUNCTION__, sg_next_copy_rank_time);
		
		copy_doufachang_rank(conn_node_doufachangsrv::doufachang_rank2_key,
			conn_node_doufachangsrv::doufachang_rank_reward_key, sg_redis_client);
	}
	add_timer(doufachang_timeout, &doufachang_event_timer, NULL);
}

int main(int argc, char **argv)
{
	int ret = 0;
//	FILE *file = NULL;
	std::ifstream file;
	char *line;
	int timeout;
	char *ip;
	int port;
	int i;
	int check = 0;
	struct sockaddr_in sin;
	signal(SIGTERM, SIG_IGN);
	uint64_t now = time_helper::get_micro_time();

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
		if (strcmp(argv[i], "-check") == 0) {
			check = 1;
			break;
		}
	}

	
	uint64_t pid = write_pid_file();		
    LOG_INFO("doufachang_srv run %lu", pid);
	ret = game_event_init();
	if (ret != 0)
		goto done;

	file.open("../server_info.ini", std::ifstream::in);
	if (!file.good()) {
		LOG_ERR("open server_info.ini failed[%d]", errno);				
		ret = -1;
		goto done;
	}

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

	line = get_first_key(file, (char *)"conn_srv_doufachang_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_srv_doufachang_port");
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
	sprintf(conn_node_doufachangsrv::doufachang_key, "doufachang_%u", sg_server_id);
	sprintf(conn_node_doufachangsrv::doufachang_rank_key, "doufachang_rank_%u", sg_server_id);
	sprintf(conn_node_doufachangsrv::doufachang_rank2_key, "doufachang_rank2_%u", sg_server_id);
	sprintf(conn_node_doufachangsrv::doufachang_rank_reward_key, "doufachang_rank_reward_%u", sg_server_id);			
	sprintf(conn_node_doufachangsrv::doufachang_lock_key, "doufachang_lock_%u", sg_server_id);
	sprintf(conn_node_doufachangsrv::doufachang_record_key, "doufachang_record_%u", sg_server_id);		
	sprintf(conn_node_doufachangsrv::server_key, "server_%u", sg_server_id);
	player_doufachang_info__init(&conn_node_doufachangsrv::default_info);
	conn_node_doufachangsrv::default_info.challenge_count = DEFAULT_CHALLENGE_COUNT;

	if (check)
	{
		doufachang_check();
		return (0);
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

	ret = game_add_connect_event((struct sockaddr *)&sin, sizeof(sin), conn_node_doufachangsrv::instance());
	if (ret <= 0)
		goto done;

//	fclose(file);
	file.close();

	//sg_event_timer.ev_callback = cb_on_timer;

	//add_timer(sg_timeout, &sg_event_timer, NULL);

	if (SIG_ERR == signal(SIGPIPE,SIG_IGN)) {
		LOG_ERR("set sigpipe ign failed");		
		return (0);
	}
	add_signal(SIGUSR2, NULL, cb_signal2);

	read_all_config();	

	doufachang_event_timer.ev_callback = cb_doufachang_timer;
	add_timer(doufachang_timeout, &doufachang_event_timer, NULL);

	sg_next_copy_rank_time = time_helper::nextOffsetTime(22 * 3600, now);

	ret = event_base_loop(base, 0);
	LOG_INFO("event_base_loop stoped[%d]", ret);	

	struct timeval tv;
	event_base_gettimeofday_cached(base, &tv);

done:
	LOG_INFO("doufachang_srv stoped[%d]", ret);
	return (ret);
}

