#ifndef _CONN_NODE_RANKSRV_H__ 
#define _CONN_NODE_RANKSRV_H__


#include "conn_node.h"
#include "redis_client.h"
typedef size_t (*pack_func)(const void *message, uint8_t *out);
class conn_node_ranksrv: public conn_node_base
{
	typedef int (conn_node_ranksrv::*handle_func)(EXTERN_DATA*);
	typedef std::map<uint32_t, handle_func> HandleMap;
public:
	conn_node_ranksrv();
	virtual ~conn_node_ranksrv();

	void add_msg_handle(uint32_t msg_id, handle_func func);

	virtual int recv_func(evutil_socket_t fd);

	static conn_node_ranksrv connecter;

	 static int broadcast_message(uint16_t msg_id, void *msg_data, pack_func packer, std::vector<uint64_t> &players);
private:
	int handle_refresh_player_info(EXTERN_DATA *extern_data);
	int handle_rank_info_request(EXTERN_DATA *extern_data);
	int handle_zhenying_leader(EXTERN_DATA *extern_data);
	int handle_player_online_notify(EXTERN_DATA *extern_data);

	int handle_refresh_guild_info(EXTERN_DATA *extern_data);

	int handle_refresh_player_world_boss_info(EXTERN_DATA *extern_data);
	int handle_world_boss_real_rank_info_request(EXTERN_DATA *extern_data);
	int handle_world_boss_zhu_jiemian_info_request(EXTERN_DATA *extern_data);
	int handle_world_boss_last_rank_info_request(EXTERN_DATA *extern_data);
	int handle_world_timing_birth_updata_info(EXTERN_DATA *extern_data);
	int refresh_befor_world_boss_info();
	int broadcast_world_boss_rank_info(uint64_t boss_id);
	int updata_player_cur_world_boss_info(uint64_t boss_id, EXTERN_DATA *extern_data);
	int world_boss_provide_rank_reward(uint64_t boss_id);
	int receive_world_boss_reward_to_player(uint32_t rank, uint64_t boss_id, uint64_t player_id, uint32_t score);
	int world_boss_provide_kill_reward(uint64_t boss_id);

private:
	HandleMap   m_handleMap;
};



#endif

