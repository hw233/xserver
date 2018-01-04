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
#include <pthread.h>
#include "game_event.h"
#include "msgid.h"
#include "time_helper.h"
#include "listen_node_client.h"
#include "listen_node_gamesrv.h"
#include "listen_node_raidsrv.h"
#include "listen_node_login.h"
#include "listen_node_friend.h"
#include "listen_node_dump.h"
#include "listen_node_mail.h"
#include "listen_node_guild.h"
#include "listen_node_rank.h"
#include "listen_node_doufachang.h"
#include "listen_node_trade.h"
#include "listen_node_activity.h"
#include "../proto/login.pb-c.h"
#include "oper_config.h"
#include "deamon.h"
#include <evhttp.h>
//#include "event-internal.h"
#include "flow_record.h"

void generic_request_handler(struct evhttp_request *req, void *arg)
{
	struct evbuffer *returnbuffer = evbuffer_new();

	evbuffer_add_printf(returnbuffer, "Thanks for the request!");
	evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
	evbuffer_free(returnbuffer);
	return;
}
extern struct event_base *event_global_current_base_;
void init_http_server()
{
	short          http_port = 8081;
	const char          *http_addr = "0.0.0.0";
	struct evhttp *http_server = NULL;

//	base = event_init();
	event_global_current_base_ = base;
	http_server = evhttp_start(http_addr, http_port);
	evhttp_set_gencb(http_server, generic_request_handler, NULL);
}

int init_conn_client_map()
{
	return (0);
}

static void cb_signal(evutil_socket_t fd, short events, void *arg)
{
	if (conn_node_gamesrv::server_node)
		shutdown(conn_node_gamesrv::server_node->fd, SHUT_WR);
//	evutil_closesocket(conn_node_server::server_node->fd);
	conn_node_gamesrv::server_node = NULL;
	LOG_DEBUG("%s: fd = %d, events = %d, arg = %p", __FUNCTION__, fd, events, arg);
	//连接服断开，存流量数据到数据库
#ifdef FLOW_MONITOR
	save_flow_record_to_mysql();
#endif
	
}

static void cb_signal2(evutil_socket_t fd, short events, void *arg)
{
//	exit(0);
	change_mycat();
	LOG_INFO("%s: fd = %d, events = %d, arg = %p", __FUNCTION__, fd, events, arg);
	
}

static struct event connsrv_event_timer;
static struct timeval connsrv_timeout;
void cb_connsrv_timer(evutil_socket_t, short, void* /*arg*/)
{
	add_timer(connsrv_timeout, &connsrv_event_timer, NULL);

	HeartbeatNotify nty;
	heartbeat_notify__init(&nty);
	uint64_t times = time_helper::get_micro_time();
	nty.curtime = times / 1000 / 1000;

	PROTO_HEAD* head = conn_node_base::get_send_buf(MSG_ID_HEARTBEAT_NOTIFY, 0);
	size_t size = heartbeat_notify__pack(&nty, conn_node_base::get_send_data());	
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);

	conn_node_client *client;		
	for (std::map<evutil_socket_t, conn_node_client *>::iterator it = conn_node_client::map_fd_nodes.begin();
		 it != conn_node_client::map_fd_nodes.end(); ++it)
	{
		client = it->second;
		if (!client) {// || client->player_id == 0) {
			continue;
		}
//		LOG_DEBUG("%s %d: broadcast to playerid[%lu] openid[%u] fd[%u]", __FUNCTION__, __LINE__, client->player_id, client->open_id, client->fd);		
		
		if (client->send_one_msg(head, 1) != (int)(ENDION_FUNC_4(head->len))) {		
//			LOG_ERR("%s: broadcast to client[%u] failed err[%d]", __FUNCTION__, client->player_id, errno);
			continue;
		}
	}
}

static listen_node_client client_listener;   //客户端连接
static listen_node_gamesrv gamesrv_listener;   //game_srv连接
static listen_node_raidsrv raidsrv_listener;   //raid_srv连接
static listen_node_login login_listener; //login_srv连接
static listen_node_friend friend_listener; //friend_srv连接
static listen_node_dump dump_listener; //dump_srv连接
static listen_node_mail mail_listener; //mail_srv连接
static listen_node_guild guild_listener; //guild_srv连接
static listen_node_rank rank_listener; //rank_srv连接
static listen_node_doufachang doufachang_listener; //doufachang_srv连接
static listen_node_trade trade_listener; //trade_srv连接
static listen_node_activity activity_listener; //activity_srv连接

uint32_t sg_server_id;

int main(int argc, char **argv)
{
	int ret = 0;
	FILE *file=NULL;
	char *line;
	int port;

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
	LOG_INFO("%s %d: conn_srv run %lu",	__FUNCTION__, __LINE__, pid);
	if (init_conn_client_map() != 0) {
		LOG_ERR("init client map failed");
		goto done;
	}

	ret = game_event_init();
	if (ret != 0)
		goto done;

	file = fopen("../server_info.ini", "r");
	if (!file) {
		LOG_ERR("open server_info.ini failed[%d]", errno);				
		ret = -1;
		goto done;
	}

	line = get_first_key(file, (char *)"game_srv_id");
	sg_server_id = (uint64_t )strtoul(get_value(line), NULL, 0);

	line = get_first_key(file, (char *)"conn_srv_client_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_srv_client_port");
		ret = -1;
		goto done;
	}

	ret = game_add_listen_event(port, &client_listener, "client");
	if (ret != 0)
		goto done;

	line = get_first_key(file, (char *)"conn_srv_game_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_srv_game_port");
		ret = -1;
		goto done;
	}

	add_signal(SIGUSR1, NULL, cb_signal);
	add_signal(SIGUSR2, NULL, cb_signal2);		
	
	ret = game_add_listen_event(port, &gamesrv_listener, "gamesrv");
	if (ret != 0)
		goto done;

	line = get_first_key(file, (char *)"conn_srv_raid_port");
	if (line)
	{
		port = atoi(get_value(line));
		if (port > 0) {
			game_add_listen_event(port, &raidsrv_listener, "raidsrv");		
		}
	}

	//listen login_srv_port
	line = get_first_key(file, (char *)"conn_srv_login_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_srv_login_port");
		ret = -1;
		goto done;
	}
	
	ret = game_add_listen_event(port, &login_listener, "loginsrv");
	if (ret != 0)
		goto done;

	//listen friend_srv_port
	line = get_first_key(file, (char *)"conn_srv_friend_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_srv_friend_port");
		ret = -1;
		goto done;
	}
	
	ret = game_add_listen_event(port, &friend_listener, "friendsrv");
	if (ret != 0)
		goto done;
	
	//listen dump_srv_port
	line = get_first_key(file, (char *)"conn_srv_dump_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_srv_dump_port");
		ret = -1;
		goto done;
	}
	ret = game_add_listen_event(port, &dump_listener, "dumpsrv");
	if (ret != 0)
		goto done;

	//listen doufachang_srv_port
	line = get_first_key(file, (char *)"conn_srv_doufachang_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_srv_doufachang_port");
		ret = -1;
		goto done;
	}
	ret = game_add_listen_event(port, &doufachang_listener, "doufachangsrv");
	if (ret != 0)
		goto done;

	//listen mail_srv_port
	line = get_first_key(file, (char *)"conn_srv_mail_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_srv_mail_port");
		ret = -1;
		goto done;
	}

	ret = game_add_listen_event(port, &mail_listener, "mailsrv");
	if (ret != 0)
		goto done;

	//listen guild_srv_port
	line = get_first_key(file, (char *)"conn_srv_guild_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_srv_guild_port");
		ret = -1;
		goto done;
	}

	ret = game_add_listen_event(port, &guild_listener, "guildsrv");
	if (ret != 0)
		goto done;

	//listen rank_srv_port
	line = get_first_key(file, (char *)"conn_srv_rank_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_srv_rank_port");
		ret = -1;
		goto done;
	}

	ret = game_add_listen_event(port, &rank_listener, "ranksrv");
	if (ret != 0)
		goto done;
	
	//listen trade_srv_port
	line = get_first_key(file, (char *)"conn_srv_trade_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_srv_trade_port");
		ret = -1;
		goto done;
	}

	ret = game_add_listen_event(port, &trade_listener, "tradesrv");
	if (ret != 0)
		goto done;

	//listen activity_srv_port
	line = get_first_key(file, (char *)"conn_srv_activity_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_srv_activity_port");
		ret = -1;
		goto done;
	}

	ret = game_add_listen_event(port, &activity_listener, "activitysrv");
	if (ret != 0)
		goto done;

	if (SIG_ERR == signal(SIGPIPE,SIG_IGN)) {
		LOG_ERR("set sigpipe ign failed");		
		return (0);
	}

//	init_http_server();
	connsrv_timeout.tv_sec = 5;
	connsrv_event_timer.ev_callback = cb_connsrv_timer;
	add_timer(connsrv_timeout, &connsrv_event_timer, NULL);
	
	ret = event_base_loop(base, 0);
	LOG_INFO("event_base_loop stoped[%d]", ret);	

	struct timeval tv;
	event_base_gettimeofday_cached(base, &tv);

done:
	LOG_INFO("conn_srv stoped[%d]", ret);
	if (file)
		fclose(file);
	return (ret);
}

