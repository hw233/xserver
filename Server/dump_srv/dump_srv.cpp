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
#include "conn_node_dumpsrv.h"
#include "deamon.h"
#include "oper_config.h"
#include "mem_pool.h"
#include "mysql_module.h"
#include "cgi_common.h"
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
};

extern void ldb_loop();
extern void luaLdbLineHook(lua_State *lua, lua_Debug *ar);
static void cb_signal_int(evutil_socket_t fd, short events, void *arg)
{
	sighandler_t old = signal(SIGINT, SIG_IGN);
	ldb_loop();
	signal(SIGINT, old);
}

static void cb_signal2(evutil_socket_t fd, short events, void *arg)
{
//	exit(0);
	change_mycat();
	LOG_INFO("%s: fd = %d, events = %d, arg = %p", __FUNCTION__, fd, events, arg);
	
}

lua_State *L = NULL;

int debug_init()
{
	printf("debug pid %d\n", getpid());
	add_signal(SIGINT, NULL, cb_signal_int);
	lua_sethook(L, luaLdbLineHook, LUA_MASKLINE, 100000);	
	return (0);
}

int lua_init()
{
	//开启lua状态机
    L = luaL_newstate();
	if (!L)
	{
		return (-1);
	}
	luaL_openlibs(L);
	if (luaL_loadfile(L, "lua/main.lua") || lua_pcall(L, 0,0,0))
	{
		LOG_ERR("[%s:%d] do lua error %s", __FUNCTION__, __LINE__, lua_tostring(L,-1));
		printf("[%s:%d] do lua error %s\n", __FUNCTION__, __LINE__, lua_tostring(L,-1));
		return (-2);
	}

	lua_getglobal(L, "dispatch_message");
	if (lua_type(L, -1) != LUA_TFUNCTION)
	{
		LOG_ERR("get dispatch message failed");
		return (-3);
	}
	lua_rawsetp(L, LUA_REGISTRYINDEX, &L);
	return (0);
}

int main(int argc, char **argv)
{
	int ret = 0;
//	FILE *file = NULL;
	std::ifstream file;
	char *line;
	int port;
	int i;
	bool debug = true;
	struct sockaddr_in sin;
	signal(SIGTERM, SIG_IGN);

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
			debug = false;
			break;
		}
	}

	uint64_t pid = write_pid_file();		
    LOG_INFO("dump_srv run %lu", pid);
	ret = game_event_init();
	if (ret != 0)
		goto done;

	file.open("../server_info.ini", std::ifstream::in);
	if (!file.good()) {
		LOG_ERR("open server_info.ini failed[%d]", errno);				
		ret = -1;
		goto done;
	}

	line = get_first_key(file, (char *)"conn_srv_dump_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_srv_dump_port");
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

	ret = game_add_connect_event((struct sockaddr *)&sin, sizeof(sin), conn_node_dumpsrv::instance());
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

	if (lua_init() != 0)
	{
		LOG_ERR("init lua failed");
		goto done;
	}
	if (debug)
		debug_init();
	
	ret = event_base_loop(base, 0);
	LOG_INFO("event_base_loop stoped[%d]", ret);	

	if (L)
	{
		lua_close(L);	
	}
	struct timeval tv;
	event_base_gettimeofday_cached(base, &tv);

done:
	LOG_INFO("dump_srv stoped[%d]", ret);
	return (ret);
}

