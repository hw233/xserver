#ifndef _PLAYER_MANAGER_H__
#define _PLAYER_MANAGER_H__

#include "mem_pool.h"
#include "player.h"
#include "server_proto.h"
#include <map>
#include <set>
#include <list>

typedef void (*ai_player_handle)(player_struct *);

extern std::map<uint64_t, player_struct *> player_manager_all_players_id;
extern std::map<uint64_t, player_struct *> player_manager_all_ai_players_id;
extern std::list<player_struct *> player_manager_player_free_list;
extern std::set<player_struct *> player_manager_player_used_list;
extern struct comm_pool player_manager_player_data_pool;
#define MAX_PLAYER_AI_TYPE 10

class player_manager
{
public:
	player_manager();
	virtual ~player_manager();

	static void on_tick_1();
	static void on_tick_5();
	static void on_tick_10();		

	static player_struct *create_doufachang_ai_player(DOUFACHANG_LOAD_PLAYER_ANSWER *ans);
	static player_struct *create_doufachang_ai_player(player_struct *player);	//生成斗法场ai玩家
	static player_struct *create_ai_player(player_struct *player, scene_struct *scene, int name_index);	  //根据玩家战斗力生成随机ai玩家
	static player_struct *create_tmp_player(uint64_t player_id);   //临时测试用	
	static player_struct *create_player(PROTO_ENTER_GAME_RESP *proto, uint64_t player_id);  //玩家登陆的时候调用，创建玩家并加入主城
	static void delete_player(player_struct *p);                                       //玩家下线，并且数据被保存下来以后调用
	static void delete_player_by_id(uint64_t player_id);
	static player_struct * get_player_by_id(uint64_t id);
	static player_struct * get_ai_player_by_id(uint64_t id);
	static player_struct *get_online_player(uint64_t id);
	static int send_to_player(uint64_t player_id, PROTO_HEAD *head);

	static unsigned int get_pool_max_num();
	
	static int init_player_struct(int num, unsigned long key);
//	static int resume_player_struct(int num, unsigned long key);
	
//	static std::map<uint64_t, player_struct *> all_players_id;
//	static std::map<uint64_t, player_struct *> all_ai_players_id;

	static ai_player_handle m_ai_player_handle[MAX_PLAYER_AI_TYPE];  //1 pvp副本  2 斗法场
private:
//	static int resume_player_bag_data(player_struct *player);
	static player_struct *add_player(uint64_t player_id);
	static int add_player(player_struct *p);
	static int remove_player(player_struct *p);
	static player_struct *alloc_player();
	static char *get_rand_player_name(int index);
	
private:
//	static std::list<player_struct *> player_free_list;
//	static std::set<player_struct *> player_used_list;
//	static struct comm_pool player_data_pool;
};


#endif
