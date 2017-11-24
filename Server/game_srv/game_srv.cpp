#include <signal.h>
#include <assert.h>
#include <time.h>
#include <limits.h>
#include <search.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <list>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#include <evhttp.h>
#include <map>
#include <dlfcn.h>
#include <python2.7/Python.h>
#include "game_event.h"
#include "conn_node_gamesrv.h"
#include "conn_node_dbsrv.h"
#include "oper_config.h"
#include "mem_pool.h"
// #include "scene.h"
// #include "game_config.h"
// #include "player_manager.h"
// #include "raid_manager.h"
// #include "zhenying_raid_manager.h"
// #include "sight_space_manager.h"
// #include "monster_manager.h"
// #include "skill_manager.h"
// #include "buff_manager.h"
// #include "sight_space_manager.h"
#include "cgi_common.h"
#include "time_helper.h"
// #include "test_timer.h"
// #include "unit.h"
// #include "team.h"
// #include "collect.h"
// #include "pvp_match_manager.h"
// #include "guild_battle_manager.h"
// #include "guild_wait_raid_manager.h"
// #include "chengjie.h"
// #include "global_shared_data.h"
// #include "team.h"
#include "app_data_statis.h"

//static int count_mem_used(int player_num)
//{
//	int ret = 0;
//	ret = count_init_comm_pool_size(sizeof(player_data), player_num);
//	return (ret);
//}

uint64_t  sg_server_id;
uint32_t sg_gm_cmd_open;
uint32_t sg_server_open_time;

typedef int (*install_func)(int argc, char **argv);
typedef int (*reload_func)();
typedef int (*uninstall_func)();
typedef void (*http_request_func)(struct evhttp_request *req, void *arg);
typedef void (*cb_timer_func)();
typedef int (*game_recv_func)(evutil_socket_t fd, conn_node_gamesrv *node);
typedef int (*db_recv_func)(evutil_socket_t fd, conn_node_dbsrv *node);

static http_request_func g_http_request_func;
static cb_timer_func g_cb_timer_func;
game_recv_func g_game_recv_func;
db_recv_func g_db_recv_func;

// typedef struct _so_entry
// {
// 	char *so_name;
// 	void *so_handle;
// } so_entry;
//std::list<so_entry> so_list;

extern int init_signals();

void* report_callback(void* lparam);

static int install_so(int argc, char **argv);
static int reload_so();
static void uninstall_so();
static void generic_request_handler(struct evhttp_request *req, void *arg)
{
	std::map<std::string, std::string> param_map;
	char *param = strchr(req->uri, '?');
	
	if (param)
	{
		*param = '\0';
		++param;
		uri_parse(param, param_map);
	}
    if (strcmp(req->uri, "/reload") == 0)
	{
		uninstall_so();
		reload_so();
		struct evbuffer *returnbuffer = evbuffer_new();

		evbuffer_add_printf(returnbuffer, "reload gamesrv so success!\n");
		evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
		evbuffer_free(returnbuffer);
		return;		
	}

	// if (strcmp(req->uri, "/player_pos") == 0)
	// {
	// 	std::map<std::string, std::string>::iterator iter = param_map.find("player_id");
	// 	if (iter == param_map.end())
	// 		return;
	// 	uint64_t player_id = strtoul(iter->second.c_str(), NULL, 0);
	// 	player_struct *player = player_manager::get_player_by_id(player_id);
	// 	if (player)
	// 	{
	// 		struct position *pos = player->get_pos();
	// 		struct evbuffer *returnbuffer = evbuffer_new();			
	// 		evbuffer_add_printf(returnbuffer, "player[%s] at pos[%.1f][%.1f]", player->data->name, pos->pos_x, pos->pos_z);
	// 		evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);			
	// 		evbuffer_free(returnbuffer);			
	// 	}
	// }
	// if (strcmp(req->uri, "/add_item") == 0)
	// {
	// 	struct evbuffer *returnbuffer = evbuffer_new();			
	// 	do
	// 	{
	// 		std::map<std::string, std::string>::iterator iter = param_map.find("player_id");
	// 		if (iter == param_map.end())
	// 		{
	// 			evbuffer_add_printf(returnbuffer, "not arg player_id");
	// 			break;
	// 		}
	// 		uint64_t player_id = strtoul(iter->second.c_str(), NULL, 0);

	// 		iter = param_map.find("item_id");
	// 		if (iter == param_map.end())
	// 		{
	// 			evbuffer_add_printf(returnbuffer, "not arg item_id");
	// 			break;
	// 		}
	// 		uint32_t item_id = strtoul(iter->second.c_str(), NULL, 0);

	// 		iter = param_map.find("item_num");
	// 		if (iter == param_map.end())
	// 		{
	// 			evbuffer_add_printf(returnbuffer, "not arg item_num");
	// 			break;
	// 		}
	// 		uint32_t item_num = strtoul(iter->second.c_str(), NULL, 0);

	// 		player_struct *player = player_manager::get_player_by_id(player_id);
	// 		if (player)
	// 		{
	// 			int ret = player->add_item(item_id, item_num, MAGIC_TYPE_GM);
	// 			evbuffer_add_printf(returnbuffer, "player[%s] add item [%u][%u] success, ret:%d.\n", player->data->name, item_id, item_num, ret);
	// 		}
	// 		else
	// 		{
	// 			evbuffer_add_printf(returnbuffer, "player[%lu] not online.\n", player_id);
	// 		}
	// 	} while(0);
	// 	evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);			
	// 	evbuffer_free(returnbuffer);			
	// }
	// if (strcmp(req->uri, "/del_item") == 0)
	// {
	// 	struct evbuffer *returnbuffer = evbuffer_new();			
	// 	do
	// 	{
	// 		std::map<std::string, std::string>::iterator iter = param_map.find("player_id");
	// 		if (iter == param_map.end())
	// 		{
	// 			evbuffer_add_printf(returnbuffer, "not arg player_id");
	// 			break;
	// 		}
	// 		uint64_t player_id = strtoul(iter->second.c_str(), NULL, 0);

	// 		iter = param_map.find("item_id");
	// 		if (iter == param_map.end())
	// 		{
	// 			evbuffer_add_printf(returnbuffer, "not arg item_id");
	// 			break;
	// 		}
	// 		uint32_t item_id = strtoul(iter->second.c_str(), NULL, 0);

	// 		iter = param_map.find("item_num");
	// 		if (iter == param_map.end())
	// 		{
	// 			evbuffer_add_printf(returnbuffer, "not arg item_num");
	// 			break;
	// 		}
	// 		uint32_t item_num = strtoul(iter->second.c_str(), NULL, 0);

	// 		player_struct *player = player_manager::get_player_by_id(player_id);
	// 		if (player)
	// 		{
	// 			int ret = player->del_item_by_id(item_id, item_num, MAGIC_TYPE_GM);
	// 			evbuffer_add_printf(returnbuffer, "player[%s] del item [%u][%u] success, ret:%d.\n", player->data->name, item_id, item_num, ret);
	// 		}
	// 		else
	// 		{
	// 			evbuffer_add_printf(returnbuffer, "player[%lu] not online.\n", player_id);
	// 		}
	// 	} while(0);
	// 	evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);			
	// 	evbuffer_free(returnbuffer);			
	// }
	// if (strcmp(req->uri, "/drop_item") == 0)
	// {
	// 	struct evbuffer *returnbuffer = evbuffer_new();			
	// 	do
	// 	{
	// 		std::map<std::string, std::string>::iterator iter = param_map.find("drop_id");
	// 		if (iter == param_map.end())
	// 		{
	// 			evbuffer_add_printf(returnbuffer, "not arg drop_id");
	// 			break;
	// 		}
	// 		uint32_t drop_id = strtoul(iter->second.c_str(), NULL, 0);

	// 		std::map<uint32_t, uint32_t> item_list;
	// 		int ret = get_drop_item(drop_id, item_list);
	// 		evbuffer_add_printf(returnbuffer, "get drop id %u, ret:%d, item_list:<br>", drop_id, ret);
	// 		for (std::map<uint32_t, uint32_t>::iterator iter = item_list.begin(); iter != item_list.end(); ++iter)
	// 		{
	// 			evbuffer_add_printf(returnbuffer, "[%u:%u]<br>", iter->first, iter->second);
	// 		}
	// 	} while(0);
	// 	evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);			
	// 	evbuffer_free(returnbuffer);			
	// }
	// if (strcmp(req->uri, "/add_exp") == 0)
	// {
	// 	struct evbuffer *returnbuffer = evbuffer_new();			
	// 	do
	// 	{
	// 		std::map<std::string, std::string>::iterator iter = param_map.find("player_id");
	// 		if (iter == param_map.end())
	// 		{
	// 			evbuffer_add_printf(returnbuffer, "not arg player_id");
	// 			break;
	// 		}
	// 		uint64_t player_id = strtoul(iter->second.c_str(), NULL, 0);

	// 		iter = param_map.find("exp");
	// 		if (iter == param_map.end())
	// 		{
	// 			evbuffer_add_printf(returnbuffer, "not arg exp");
	// 			break;
	// 		}
	// 		uint32_t exp = strtoul(iter->second.c_str(), NULL, 0);

	// 		player_struct *player = player_manager::get_player_by_id(player_id);
	// 		if (player)
	// 		{
	// 			int ret = player->add_exp(exp, MAGIC_TYPE_GM);
	// 			evbuffer_add_printf(returnbuffer, "player[%s] add exp [%u] success, ret:%d.\n", player->data->name, exp, ret);
	// 		}
	// 		else
	// 		{
	// 			evbuffer_add_printf(returnbuffer, "player[%lu] not online.\n", player_id);
	// 		}
	// 	} while(0);
	// 	evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);			
	// 	evbuffer_free(returnbuffer);			
	// }

	// if (strcmp(req->uri, "/addso") == 0)
	// {
	// 	struct evbuffer *returnbuffer = evbuffer_new();
	// 	std::map<std::string, std::string>::iterator iter = param_map.find("name");		
	// 	so_entry entry;
	// 	entry.so_name = strdup(iter->second.c_str());
	// 	entry.so_handle = dlopen(entry.so_name, RTLD_NOW | RTLD_GLOBAL);
	// 	if (!entry.so_handle)
	// 	{
	// 		LOG_ERR("addso %s fail = %s", entry.so_name, dlerror());
	// 		evbuffer_add_printf(returnbuffer, "load %s so fail!", entry.so_name);
	// 		evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
	// 		evbuffer_free(returnbuffer);			
	// 		free(entry.so_name);
	// 		return;
	// 	}
	// 	install_func t = (install_func)dlsym(entry.so_handle, "install");
	// 	if (!t)
	// 	{
	// 		LOG_ERR("dlsym install failed");
	// 		evbuffer_add_printf(returnbuffer, "install %s so fail!", entry.so_name);
	// 		evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
	// 		evbuffer_free(returnbuffer);			
	// 		free(entry.so_name);
	// 		dlclose(entry.so_handle);			
	// 		return;
	// 	}
	// 	t();
	// 	so_list.push_back(entry);
	// 	evbuffer_add_printf(returnbuffer, "load %s so success[%p]!", entry.so_name, entry.so_handle);
	// 	evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
	// 	evbuffer_free(returnbuffer);
	// }
	// if (strcmp(req->uri, "/delso") == 0)
	// {
	// 	struct evbuffer *returnbuffer = evbuffer_new();
	// 	std::map<std::string, std::string>::iterator name = param_map.find("name");		
	// 	std::list<so_entry>::iterator ite;
	// 	for (ite = so_list.begin(); ite != so_list.end(); ++ite)
	// 	{
	// 		if (strcmp(ite->so_name, name->second.c_str()) == 0)
	// 		{
	// 			evbuffer_add_printf(returnbuffer, "unload %s so success[%p]!", ite->so_name, ite->so_handle);
	// 			evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);

	// 			install_func t = (install_func)dlsym(ite->so_handle, "uninstall");
	// 			if (!t)
	// 			{
	// 				LOG_ERR("uninstall %s %p fail: err = %s\n", ite->so_name, ite->so_handle, dlerror());
	// 			}
	// 			else
	// 			{
	// 				t();
	// 			}
	// 			dlclose(ite->so_handle);
	// 			so_list.erase(ite);
	// 			break;
	// 		}
	// 	}
	// 	evbuffer_free(returnbuffer);				
	// }
	// if (strcmp(req->uri, "/listso") == 0)
	// {
	// 	struct evbuffer *returnbuffer = evbuffer_new();			
	// 	std::list<so_entry>::iterator ite;
	// 	for (ite = so_list.begin(); ite != so_list.end(); ++ite)
	// 	{
	// 		evbuffer_add_printf(returnbuffer, "%s %p<br><br>\n", ite->so_name, ite->so_handle);
	// 		evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);			
	// 	}
	// 	evbuffer_free(returnbuffer);			
	// }
	
    if (strcmp(req->uri, "/python") == 0)
	{
		struct evbuffer *returnbuffer = evbuffer_new();
		std::map<std::string, std::string>::iterator iter = param_map.find("name");
		if (iter == param_map.end())
			return;

		if (!Py_IsInitialized())		
			Py_Initialize();
		if (!Py_IsInitialized())
		{
			evbuffer_add_printf(returnbuffer, "py initialize failed\n");
			evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
			evbuffer_free(returnbuffer);
			return;		
		}

		PyRun_SimpleString("import sys");
		PyRun_SimpleString("sys.path.append('./')");
		PyRun_SimpleString("sys.path.append('./unit_test/')"); 		

		PyObject* pModule = PyImport_ImportModule(iter->second.c_str());
		if (!pModule)
		{
			evbuffer_add_printf(returnbuffer, "Cant open python file! %s\n", iter->second.c_str());
			evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
			evbuffer_free(returnbuffer);
			return;		
		}
		PyObject* pDict = PyModule_GetDict(pModule);
		if (!pDict)
		{
			evbuffer_add_printf(returnbuffer, "Cant find dictionary.\n");
			evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
			evbuffer_free(returnbuffer);
			return;		
		}
		PyObject* pFunHi = PyDict_GetItemString(pDict, "py_install");
		if (!pFunHi)
		{
			evbuffer_add_printf(returnbuffer, "Cant find py_install.\n");
			evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
			evbuffer_free(returnbuffer);
			return;		
		}
		PyObject_CallFunction(pFunHi, (char *)"");
		
		Py_DECREF(pFunHi);
		Py_Finalize();
		
		evbuffer_add_printf(returnbuffer, "load python %s success!", iter->second.c_str());
		evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
		evbuffer_free(returnbuffer);
		return;		
	}
    if (strcmp(req->uri, "/py") == 0)//不卸载
	{
		struct evbuffer *returnbuffer = evbuffer_new();
		std::map<std::string, std::string>::iterator iter = param_map.find("name");
		if (iter == param_map.end())
			return;

		if (!Py_IsInitialized())		
			Py_Initialize();
		if (!Py_IsInitialized())
		{
			evbuffer_add_printf(returnbuffer, "py initialize failed\n");
			evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
			evbuffer_free(returnbuffer);
			return;		
		}

		PyRun_SimpleString("import sys");
		PyRun_SimpleString("sys.path.append('./')");
		PyRun_SimpleString("sys.path.append('./unit_test/')"); 		

		PyObject* pModule = PyImport_ImportModule(iter->second.c_str());
		if (!pModule)
		{
			evbuffer_add_printf(returnbuffer, "Cant open python file! %s\n", iter->second.c_str());
			evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
			evbuffer_free(returnbuffer);
			return;		
		}
		PyObject* pDict = PyModule_GetDict(pModule);
		if (!pDict)
		{
			evbuffer_add_printf(returnbuffer, "Cant find dictionary.\n");
			evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
			evbuffer_free(returnbuffer);
			return;		
		}
		PyObject* pFunHi = PyDict_GetItemString(pDict, "py_install");
		if (!pFunHi)
		{
			evbuffer_add_printf(returnbuffer, "Cant find py_install.\n");
			evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
			evbuffer_free(returnbuffer);
			return;		
		}
		PyObject_CallFunction(pFunHi, (char *)"");
		
		evbuffer_add_printf(returnbuffer, "load python %s success!", iter->second.c_str());
		evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
		evbuffer_free(returnbuffer);
		return;		
	}

	if (g_http_request_func)
		g_http_request_func(req, arg);
}
extern struct event_base *event_global_current_base_;
void init_http_server(uint32_t port)
{
	if (port <= 0)
		return;
	short          http_port = port;
	const char          *http_addr = "0.0.0.0";
	struct evhttp *http_server = NULL;

//	base = event_init();
	event_global_current_base_ = base;
	http_server = evhttp_start(http_addr, http_port);
	evhttp_set_gencb(http_server, generic_request_handler, NULL);
}

static void *so_gamesrv;
static void set_so_funcs()
{
	g_http_request_func = (http_request_func)dlsym(so_gamesrv, "on_http_request");
	g_cb_timer_func = (cb_timer_func)dlsym(so_gamesrv, "cb_gamesrv_timer");
	g_game_recv_func = (game_recv_func)dlsym(so_gamesrv, "game_recv_func");
	g_db_recv_func = (db_recv_func)dlsym(so_gamesrv, "db_recv_func");

	if (!g_http_request_func)
		LOG_ERR("%s: %d", __FUNCTION__, __LINE__);
	if (!g_cb_timer_func)
		LOG_ERR("%s: %d", __FUNCTION__, __LINE__);	
	if (!g_game_recv_func)
		LOG_ERR("%s: %d", __FUNCTION__, __LINE__);	
	if (!g_db_recv_func)
		LOG_ERR("%s: %d", __FUNCTION__, __LINE__);			
}
static int install_so(int argc, char **argv)
{
#ifdef __RAID_SRV__
    so_gamesrv = dlopen("./so_game_srv/libraidsrv.so", RTLD_NOW | RTLD_GLOBAL);	
#else	
    so_gamesrv = dlopen("./so_game_srv/libgamesrv.so", RTLD_NOW | RTLD_GLOBAL);
#endif	
	if (!so_gamesrv)
	{
		LOG_ERR("dlerror = %s", dlerror());
		return (-1);
	}
	install_func t = (install_func)dlsym(so_gamesrv, "install");
	if (!t)
	{
		LOG_ERR("dlsym install failed");
		return (-1);		
	}
	set_so_funcs();
	return t(argc, argv);
}

static int reload_so()
{
#ifdef __RAID_SRV__
    so_gamesrv = dlopen("./so_game_srv/libraidsrv.so", RTLD_NOW | RTLD_GLOBAL);	
#else
    so_gamesrv = dlopen("./so_game_srv/libgamesrv.so", RTLD_NOW | RTLD_GLOBAL);
#endif	
	if (!so_gamesrv)
	{
		LOG_ERR("dlerror = %s", dlerror());
		return (-1);
	}
	reload_func t = (reload_func)dlsym(so_gamesrv, "reload");
	if (!t)
	{
		LOG_ERR("dlsym install failed");
		return (-1);		
	}
	set_so_funcs();	
	return t();
}

static void uninstall_so()
{
	assert(so_gamesrv);
	uninstall_func t = (uninstall_func)dlsym(so_gamesrv, "uninstall");
	if (!t)
	{
		LOG_ERR("err = %s\n", dlerror());
	}
	else
	{
		t();
	}

	g_http_request_func = NULL;
	g_cb_timer_func = NULL;
	g_game_recv_func = NULL;
	g_db_recv_func = NULL;

	dlclose(so_gamesrv);
}

static struct event gamesrv_event_timer;
struct timeval gamesrv_timeout;
uint64_t timer_loop_count;
void cb_gamesrv_timer(evutil_socket_t, short, void* /*arg*/)
{
	if (g_cb_timer_func)
		g_cb_timer_func();
	add_timer(gamesrv_timeout, &gamesrv_event_timer, NULL);
}

int main(int argc, char **argv)
{
	srandom(time(NULL));
	srand(time(NULL));

	int ret = log4c_init();
	if (ret != 0) {
		printf("log4c_init failed[%d]\n", ret);
		return (ret);
	}

	init_mycat();
	if (!mycat) {
		printf("log4c_category_get(\"six13log.log.app.application1\"); failed\n");
		return (0);
	}

	if (install_so(argc, argv) != 0)
	{
		LOG_ERR("install so failed");
		return (0);
	}

	gamesrv_event_timer.ev_callback = cb_gamesrv_timer;
	add_timer(gamesrv_timeout, &gamesrv_event_timer, NULL);
	
	ret = event_base_loop(base, 0);
	LOG_INFO("event_base_loop stoped[%d]", ret);

	struct timeval tv;
	event_base_gettimeofday_cached(base, &tv);

	LOG_INFO("game_srv stoped[%d]", ret);
	return (ret);
}

