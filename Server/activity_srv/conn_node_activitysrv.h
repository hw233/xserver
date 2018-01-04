#ifndef _CONN_NODE_ACTIVITYSRV_H__ 
#define _CONN_NODE_ACTIVITYSRV_H__


#include "conn_node.h"
#include "redis_client.h"
typedef size_t (*pack_func)(const void *message, uint8_t *out);
class conn_node_activitysrv: public conn_node_base
{
	typedef int (conn_node_activitysrv::*handle_func)(EXTERN_DATA*);
	typedef std::map<uint32_t, handle_func> HandleMap;
public:
	conn_node_activitysrv();
	virtual ~conn_node_activitysrv();

	void add_msg_handle(uint32_t msg_id, handle_func func);

	virtual int recv_func(evutil_socket_t fd);

	static conn_node_activitysrv connecter;

	static int broadcast_message(uint16_t msg_id, void *msg_data, pack_func packer, std::vector<uint64_t> &players);
	static int broadcast_message_to_all(uint16_t msg_id, void *msg_data, pack_func packer);
private:
	int handle_player_online_notify(EXTERN_DATA *extern_data);
	int handle_gamesrv_reward_answer(EXTERN_DATA *extern_data);
	int handle_guildsrv_give_shidamenzong_reward_answer(EXTERN_DATA *extern_data);

	int handle_zhanlidaren_get_reward_request(EXTERN_DATA *extern_data);

private:
	HandleMap   m_handleMap;
};



#endif

