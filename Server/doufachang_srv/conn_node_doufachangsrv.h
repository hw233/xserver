#ifndef _CONN_NODE_DOUFACHANGSRV_H__
#define _CONN_NODE_DOUFACHANGSRV_H__

#include "conn_node.h"
#include "player_redis_info.pb-c.h"
#include "redis_client.h"
#include <string>


extern uint32_t sg_server_id;
extern CRedisClient sg_redis_client;
extern uint32_t sg_max_conn;

extern std::string sg_server_key;
/// map<playerid, pair<day, ts> >
extern std::map<uint32_t, std::pair<uint32_t, uint32_t> > sg_filter_list;
extern long sg_filter_map_locker;


extern struct event sg_event_timer;
extern struct timeval sg_timeout;
extern void cb_on_timer(evutil_socket_t, short, void *arg);


//extern std::set<uint32_t>  filterPlayerList;

class conn_node_doufachangsrv: public conn_node_base
{
public:
	virtual int recv_func(evutil_socket_t fd);

	static conn_node_doufachangsrv* instance(void);
	static char server_key[64];
	static char doufachang_rank_key[64];  //排名 => playerid
	static char doufachang_rank2_key[64]; //playerid => 排名
	static char doufachang_rank_reward_key[64];  //playerid	 => 排名
	static char doufachang_lock_key[64];
	static char doufachang_record_key[64];
	static char doufachang_key[64];
	static PlayerDoufachangInfo default_info;
private:
	int handle_challenge_answer(EXTERN_DATA *extern_data);
	int handle_challenge_request(EXTERN_DATA *extern_data);
	int handle_info_request(EXTERN_DATA *extern_data);
	int handle_get_reward_request(EXTERN_DATA *extern_data);
	int handle_record_request(EXTERN_DATA *extern_data);
	int handle_buy_challenge_request(EXTERN_DATA *extern_data);
	int handle_server_buy_challenge_answer(EXTERN_DATA *extern_data);
	int handle_server_add_reward_answer(EXTERN_DATA *extern_data);
	int handle_player_online_notify(EXTERN_DATA *extern_data);

	uint32_t add_challenge_rank(DOUFACHANG_CHALLENGE_ANSWER *ans);	
	int add_challenge_record(DOUFACHANG_CHALLENGE_ANSWER *ans, uint32_t rank_add);
	int is_player_locked(uint64_t player_id, uint64_t now);
	int set_player_locked(uint64_t player_id, uint64_t target_id, uint64_t now);
	int set_player_unlocked(uint64_t player_id, uint64_t target_id);
	conn_node_doufachangsrv();
	virtual ~conn_node_doufachangsrv();

	static conn_node_doufachangsrv *connecter;
};



#endif
