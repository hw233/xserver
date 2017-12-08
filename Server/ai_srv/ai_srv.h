#ifndef AI_SRV_H
#define AI_SRV_H
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <netinet/in.h>
#include <search.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <list>
#ifdef _XOPEN_SOURCE_EXTENDED
#include <arpa/inet.h>
#endif
#include <dlfcn.h>
#include <evhttp.h>
#include <sys/socket.h>
#include <map>
#include "app_data_statis.h"
#include "cgi_common.h"
#include "conn_node_aisrv.h"
#include "game_event.h"
#include "listen_node_aisrv.h"
#include "oper_config.h"
#include "time_helper.h"

typedef int (*ai_recv_func)(evutil_socket_t fd, conn_node_aisrv *node);

extern ai_recv_func g_ai_recv_func;

#endif /* AI_SRV_H */
