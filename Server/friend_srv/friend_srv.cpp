#include <signal.h>
#include <assert.h>
#include <search.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
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
#include <string.h>
#include "game_event.h"
#include "conn_node_friendsrv.h"
#include "deamon.h"
#include "oper_config.h"
#include "mem_pool.h"
#include "time_helper.h"
#include "wanyaogu.pb-c.h"
#include "send_mail.h"
#include "app_data_statis.h"
#include "friend_config.h"
#include "friend_util.h"



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

static struct event friendsrv_event_timer;
static struct timeval friendsrv_timeout;

static void debug_log()
{
	CAutoRedisReply autoR;		
	redisReply *r = sg_redis_client.hgetall_bin(conn_node_friendsrv::server_wyk_key, autoR);
	if (!r || r->type != REDIS_REPLY_ARRAY)
	{
		LOG_ERR("%s: hgetall failed", __FUNCTION__);
	}
	else
	{
		uint32_t index = 0;
		uint64_t player_id;
		while (index + 1 < r->elements)
		{
			struct redisReply *key = r->element[index];
			struct redisReply *val = r->element[index + 1];
			index += 2;
			if (key->type != REDIS_REPLY_STRING || val->type != REDIS_REPLY_STRING)
				continue;
			player_id = strtoull(key->str, NULL, 10);
			if (player_id == 0)
				continue;
			
			ListWanyaokaAnswer *answer = NULL;
			answer = list_wanyaoka_answer__unpack(NULL, val->len, (const uint8_t *)val->str);
			if (answer)
			{
				LOG_DEBUG("%s: player[%lu] have %lu wanyaoka", __FUNCTION__, player_id, answer->n_wanyaoka_id);
				list_wanyaoka_answer__free_unpacked(answer, NULL);
			}
		}
	}
}

static void	send_wyk_reward()
{
	CAutoRedisReply autoR;			
	redisReply *r = sg_redis_client.hgetall_bin(conn_node_friendsrv::server_wyk_key, autoR);
	if (!r || r->type != REDIS_REPLY_ARRAY)
	{
		LOG_ERR("%s: hgetall failed", __FUNCTION__);
	}
	else
	{
		uint32_t index = 0;
		uint64_t player_id;
		while (index + 1 < r->elements)
		{
			struct redisReply *key = r->element[index];
			struct redisReply *val = r->element[index + 1];
			index += 2;
			if (key->type != REDIS_REPLY_STRING || val->type != REDIS_REPLY_STRING)
				continue;
			player_id = strtoull(key->str, NULL, 10);
			if (player_id == 0)
				continue;
			
			ListWanyaokaAnswer *answer = NULL;
			answer = list_wanyaoka_answer__unpack(NULL, val->len, (const uint8_t *)val->str);
			if (!answer)
				continue;

			for (std::map<uint64_t, struct RandomCardRewardTable*>::iterator ite = wanyaoka_reward_config.begin();
				 ite != wanyaoka_reward_config.end(); ++ite)
			{
				if (answer->n_wanyaoka_id == ite->second->CardNum)
				{
					std::map<uint32_t, uint32_t> attachs;
					attachs[ite->second->RewardID] = ite->second->RewardNum;
					send_mail(&conn_node_friendsrv::connecter, player_id, MAIL_ID_WANYAOKA, NULL, NULL, NULL, NULL, &attachs, MAGIC_TYPE_WANYAOKA_REWARD);
					break;
				}
			}
			
			list_wanyaoka_answer__free_unpacked(answer, NULL);
		}
	}

	int ret = sg_redis_client.del(conn_node_friendsrv::server_wyk_key);
	if (ret != 0)
	{
		LOG_ERR("%s: del ret %d", __FUNCTION__, ret);
	}
	else
	{
		LOG_INFO("%s: del ret %d", __FUNCTION__, ret);
	}
}

static void	set_next_reward_time()
{
	static int next_week_days[] = {1, 7, 6, 5, 4, 3, 2};
	
	time_t times = time_helper::get_micro_time() / 1000000;
	struct tm *tm = localtime(&times);
	uint32_t next_times = (times / (3600 * 24) + next_week_days[tm->tm_wday]) * (3600 * 24) + time_helper::timezone_offset();
	
	LOG_INFO("%s: times = %lu, wday = %d, zoneoffset = %d, nexttime = %u", __FUNCTION__, times, tm->tm_wday, time_helper::timezone_offset(), next_times);

	sg_redis_client.hset_bin("wyk_reward_time", sg_str_server_id, (char *)&next_times, sizeof(uint32_t));
}

static void cb_friendsrv_timer(evutil_socket_t, short, void* /*arg*/)
{
	uint32_t now;
	int data_len = MAX_GLOBAL_SEND_BUF;
	int ret = sg_redis_client.hget_bin("wyk_reward_time", sg_str_server_id, (char *)conn_node_base::global_send_buf, &data_len);
	if (ret != 0)
	{
		set_next_reward_time();
		goto done;
	}

	now = time_helper::get_micro_time() / 1000000;
	if (now >= *(uint32_t *)conn_node_base::global_send_buf)
	{
		send_wyk_reward();
		set_next_reward_time();
		goto done;
	}

	debug_log();

done:	
	add_timer(friendsrv_timeout, &friendsrv_event_timer, NULL);	
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
	int timeout;
	char *ip;
	int port;
	int i;
	struct sockaddr_in sin;
	signal(SIGTERM, SIG_IGN);

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
    LOG_INFO("friend_srv run %lu", pid);
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
	
	line = get_first_key(file, (char *)"conn_srv_friend_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_srv_friend_port");
		ret = -1;
		goto done;
	}

	line = get_first_key(file, (char *)"game_srv_id");
	line = get_value(line);
	sg_server_id = (uint64_t )strtoull(line, NULL, 0);
	if (sg_server_id <= 0) {
		LOG_ERR("config file wrong, no game_srv_id");
		ret = -1;
		goto done;
	}
	strcpy(sg_str_server_id, line);

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	ret = evutil_inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr);
	if (ret != 1) {
		LOG_ERR("evutil_inet_pton failed[%d]", ret);		
		return (-1);		
	}

	ret = game_add_connect_event((struct sockaddr *)&sin, sizeof(sin), &conn_node_friendsrv::connecter);
	if (ret <= 0)
		goto done;

	file.close();

	if (SIG_ERR == signal(SIGPIPE,SIG_IGN)) {
		LOG_ERR("set sigpipe ign failed");		
		return (0);
	}
	add_signal(SIGUSR2, NULL, cb_signal2);

	read_all_excel_data();
	init_redis_keys(sg_server_id);
	rebuild_watch_info();

	friendsrv_timeout.tv_sec = 10;
	friendsrv_event_timer.ev_callback = cb_friendsrv_timer;
	add_timer(friendsrv_timeout, &friendsrv_event_timer, NULL);
	
	ret = event_base_loop(base, 0);
	LOG_INFO("event_base_loop stoped[%d]", ret);	

	struct timeval tv;
	event_base_gettimeofday_cached(base, &tv);

done:
	LOG_INFO("friend_srv stoped[%d]", ret);
	return (ret);
}

