#include <signal.h>
#include <assert.h>
//#include "event-internal.h"
#include <search.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#include <map>
#include "game_event.h"
#include "cmd_impl.h"
#include "conn_node_test.h"
#include "oper_config.h"
#include "deamon.h"

//static uint8_t send_buf[1024];
//static PROTO_HEAD *head = (PROTO_HEAD *)&send_buf[0];
//static uint8_t *send_data = &send_buf[sizeof(PROTO_HEAD)];
int init_conn_client_map()
{
	return (0);
}

conn_node_test *conn_node_test::conn_test;

static void test_timer(evutil_socket_t fd, short events, void *arg)
{
	return;
/*	
	struct timeval t1 = {3, 0};
	struct event *event_timer = (struct event *)arg;	
	add_timer(t1, event_timer, arg);
	printf("%s %d\n", __FUNCTION__, __LINE__);
	head->len = htons(30);
	for (int i = 0; i < 30; ++i) {
		head->data[i] = 'a' + i;
	}

	if (conn_node_test::conn_test)
		conn_node_test::conn_test->send_one_msg(head, 1);
*/		
}
/*
void send_login_request(int argc, char *argv[])
{
	LoginReq req;
	size_t size;
	if (argc != 3)
		return;
	login_req__init(&req);
	req.name = argv[1];
	req.password = argv[2];
	head->msg_id = CS__MESSAGE__ID__LOGIN_REQUEST;
	size = login_req__pack(&req, send_data);
	head->len = htons(size + sizeof(PROTO_HEAD));
	if (conn_node_test::conn_test)
		conn_node_test::conn_test->send_one_msg(head, 1);
}
void do_cmd(int argc, char *argv[])
{
	if (argc == 0)
		return;
	if (strcmp(argv[0], "login") == 0) {
		send_login_request(argc, argv);
	}
		
}
*/

void parse_cmd(char *line, int *argc, char *argv[])
{
	int n = 0;
	argv[0] = line;
	while (*line) {
		switch (*line)
		{
			case ' ':
			case '	':
			case '\n':				
				*line = '\0';
				if (argv[n][0] != '\0')
					++n;
				++line;
				argv[n] = line;
				break;
			default:
				++line;
		}
	}
	*argc = n;
}

cmd_impl cmd_impl__;

void *cmd_func(void *)
{
	char line[256];
	int argc;
	char *argv[10];
	printf("%s\n", __FUNCTION__);
	while (fgets(line, sizeof(line), stdin)) {
		parse_cmd(line, &argc, argv);
		cmd_impl__.do_cmd_main(argc, argv);
//		for (i = 0; i < argc; ++i)
//			printf("[%d]: [%s]\n", i, argv[i]);
	}
	return NULL;
}

int main(int argc, char **argv)
{
	static struct event event_timer;
	int i;
	int ret = 0;
	FILE *file;
	char *line;
	int port;
	struct sockaddr_in sin;
	struct timeval t1 = {3, 0};	
	
	ret = log4c_init();
	if (ret != 0) {
		printf("log4c_init failed[%d]\n", ret);
		return (ret);
	}
	mycat = log4c_category_get("six13log.log.app.application1");
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

    LOG_INFO("test run");
	if (init_conn_client_map() != 0) {
		log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "init client map failed");
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
	line = get_first_key(file, (char *)"conn_srv_client_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "config file wrong, no conn_srv_listen_port");
		ret = -1;
		goto done;
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	ret = evutil_inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr);
	if (ret != 1) {
		log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "evutil_inet_pton failed[%d]", ret);		
		return (-1);		
	}

	conn_node_test::conn_test = new conn_node_test();
	ret = game_add_connect_event((struct sockaddr *)&sin, sizeof(sin), conn_node_test::conn_test);
	if (ret <= 0)
		goto done;

	fclose(file);
	
	event_timer.ev_callback = test_timer;
	add_timer(t1, &event_timer, &event_timer);
//	event_timer.ev_arg = &event_timer;

	pthread_t thread;	
	pthread_create(&thread, NULL, cmd_func, NULL);

	ret = event_base_loop(base, 0);
	log4c_category_log(mycat, LOG4C_PRIORITY_INFO, "event_base_loop stoped[%d]", ret);	

	struct timeval tv;
	event_base_gettimeofday_cached(base, &tv);

done:
	log4c_category_log(mycat, LOG4C_PRIORITY_INFO, "test stoped[%d]", ret);
	return (ret);
}
