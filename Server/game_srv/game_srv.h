#ifndef GAME_SRV_H
#define GAME_SRV_H
#include "conn_node_gamesrv.h"
#include "conn_node_dbsrv.h"
#include "conn_node_aisrv.h"

typedef int (*game_recv_func)(evutil_socket_t fd, conn_node_gamesrv *node);
typedef int (*db_recv_func)(evutil_socket_t fd, conn_node_dbsrv *node);
typedef int (*ai_recv_func)(evutil_socket_t fd, conn_node_aisrv *node);

extern game_recv_func g_game_recv_func;
extern db_recv_func g_db_recv_func;
extern ai_recv_func g_ai_recv_func;

#endif /* GAME_SRV_H */
