#ifndef _CONN_NODE_GAMESRV_H__
#define _CONN_NODE_GAMESRV_H__

#include <stdint.h>
#include <vector>
#include "conn_node.h"
#include "server_proto.h"

typedef size_t (*pack_func)(const void *message, uint8_t *out);

class player_struct;
class conn_node_gamesrv;

//处理和连接服务器连接的对象
class conn_node_gamesrv: public conn_node_base
{
public:
	conn_node_gamesrv();
	virtual ~conn_node_gamesrv();

	virtual int recv_func(evutil_socket_t fd);
	static void send_to_all_player(uint16_t msg_id, void *data, pack_func func);

	static uint64_t *prepare_broadcast_msg_to_players(uint32_t msg_id, void *msg_data, pack_func func);
	static uint64_t *broadcast_msg_add_players(uint64_t player_id, uint64_t *ppp);
	static int broadcast_msg_send();

	static void send_to_friend(EXTERN_DATA *extern_data, uint16_t msg_id, void *data, pack_func func);
	static void notify_gamesrv_start(void); //通知其他服务器game服启动

//	这个是旧的接口，一般不用了, 用上面的接口更好
//	static void broadcast_msg_to_players(uint32_t msg_id, uint8_t *data, int len, std::vector<uint64_t>& playerlist);
	
	static conn_node_gamesrv connecter;                      //和连接服务器连接的对象，用于发送数据给连接服务器
//	static int add_msg_handle(uint32_t msg_id, handle_func func);
//	static int del_msg_handle(uint32_t msg_id);
//	static char global_send_buf[1024 * 64];                    //发送数据用的buf

	int transfer_to_dbsrv(void);
	int transfer_to_connsrv(void);
public:
//	int default_handle(player_struct *player, EXTERN_DATA *extern_data);
//	int handle_move_request(player_struct *player, EXTERN_DATA *extern_data);	

//private:
};

#endif
