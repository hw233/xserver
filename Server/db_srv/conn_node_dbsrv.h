#ifndef _CONN_NODE_DBSRV_H__
#define _CONN_NODE_DBSRV_H__

#include "conn_node.h"

class conn_node_dbsrv: public conn_node_base
{
public:
	conn_node_dbsrv();
	virtual ~conn_node_dbsrv();

	virtual int recv_func(evutil_socket_t fd);
	static conn_node_dbsrv *server_node;
private:
	void handle_save_player(EXTERN_DATA *extern_data);
	void handle_find_player(EXTERN_DATA *extern_data);
	void handle_save_player_msg(EXTERN_DATA *extern_data);
	void handle_enter_game(EXTERN_DATA *extern_data);
	void handle_doufachang_load(EXTERN_DATA *extern_data);
	void handle_rename_request(EXTERN_DATA *extern_data);
	void handle_save_client_data(EXTERN_DATA *extern_data);
	void handle_load_client_data(EXTERN_DATA *extern_data);
	void handle_add_chengjie(EXTERN_DATA *extern_data);
	void handle_del_chengjie(EXTERN_DATA *extern_data);
	void handle_update_chengjie(EXTERN_DATA *extern_data);
	void handle_load_chengjie(EXTERN_DATA *extern_data);
	void handle_search_player(EXTERN_DATA *extern_data);
	void handle_get_other_info(EXTERN_DATA *extern_data);
	void handle_load_server_level(EXTERN_DATA *extern_data);
	void handle_save_server_level(EXTERN_DATA *extern_data);
	void handle_break_server_level(EXTERN_DATA *extern_data);
};

#endif
