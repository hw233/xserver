#ifndef CONN_NODE_FRIENDSRV_H
#define CONN_NODE_FRIENDSRV_H

#include "conn_node.h"
#include "redis_client.h"

extern char sg_str_server_id[64];
extern uint32_t sg_server_id;
extern 	CRedisClient sg_redis_client;
extern struct event sg_event_timer;
extern struct timeval sg_timeout;	
extern void cb_on_timer(evutil_socket_t, short, void *arg);

typedef size_t (*pack_func)(const void *message, uint8_t *out);
class conn_node_friendsrv: public conn_node_base
{
public:
	conn_node_friendsrv();
	virtual ~conn_node_friendsrv();

	virtual int recv_func(evutil_socket_t fd);
	
	void send_system_notice(uint64_t player_id, uint32_t id, std::vector<char*> *args, uint64_t target_id = 0);
	static int broadcast_message(uint16_t msg_id, void *msg_data, pack_func packer, std::vector<uint64_t> &players);
	static void send_to_all_player(uint16_t msg_id, void *data, pack_func func);

	static conn_node_friendsrv connecter;
	static char server_key[64];
	static char server_wyk_key[64];	
private:
	void handle_team_info();
	void handle_team_apply_list();
	void handle_team_list();
	void handle_add_wanyaoka();	
	void handle_save_wanyaoka();
	void handle_list_wanyaoka();	
	void handle_cache_get();	
	void handle_cache_clear();	
	void handle_cache_insert();	
	void handle_chengjie_list();
	void handle_chengjie_task();
	void handle_chengjie_task_complete();
	void handle_chengjie_money_back();
	void handle_chose_zhenying();
	void handle_change_zhenying();
	void handle_zhenying_power();
	void handle_zhenying_change_power();
	void handle_zhenying_add_kill();
	void handle_zhenying_fight_myside_score();
	void handle_zhenying_fight_settle();

	void handle_friend_info_request();
	void handle_friend_add_contact_request();
	void handle_friend_del_contact_request();
	void handle_friend_add_block_request();
	void handle_friend_del_block_request();
	void handle_friend_del_enemy_request();
	void handle_friend_create_group_request();
	void handle_friend_edit_group_request();
	void handle_friend_remove_group_request();
	void handle_friend_move_player_group_request();
	void handle_friend_deal_apply_request();
	void handle_friend_recommend_request();
	void handle_friend_send_gift_request();
	void handle_friend_gift_cost_answer();

	void handle_friend_chat_request(); //好友频道聊天
	void handle_player_online_notify(); //玩家上线
	void handle_player_offline_notify(); //玩家下线
	void handle_friend_add_enemy_request(); //玩家被杀，增加仇人
	void handle_friend_extend_contact_request(); //使用道具扩展好友上限
	void handle_friend_cost_answer(); //去game_srv扣除消耗结果
	void handle_friend_turn_switch_request(); //翻转好友申请开关
	void handle_friend_rename_request(); //玩家改名
};



#endif /* CONN_NODE_FRIENDSRV_H */


