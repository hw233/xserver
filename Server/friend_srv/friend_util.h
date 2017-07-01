#ifndef _FRIEND_UTIL_H__
#define _FRIEND_UTIL_H__

#include "friend_struct.h"
#include "player_redis_info.pb-c.h"
#include <vector>
#include <map>
#include <set>

class AutoReleaseBatchRedisPlayer
{
public:
	AutoReleaseBatchRedisPlayer();
	~AutoReleaseBatchRedisPlayer();

	void push_back(PlayerRedisInfo *player);
private:
	std::vector<PlayerRedisInfo *> pointer_vec;
};

class AutoReleaseBatchFriendPlayer
{
public:
	AutoReleaseBatchFriendPlayer();
	~AutoReleaseBatchFriendPlayer();

	void push_back(FriendPlayer *player);
private:
	std::vector<FriendPlayer *> pointer_vec;
};

struct FriendListAdd
{
	uint32_t group_id;
	uint64_t player_id;
	uint32_t closeness;
};
struct FriendListDel
{
	uint32_t group_id;
	uint64_t player_id;
};

struct FriendListChangeInfo
{
	std::vector<FriendListAdd> adds;
	std::vector<FriendListDel> dels;
};

void init_redis_keys(uint32_t server_id);
PlayerRedisInfo *get_redis_player(uint64_t player_id);
bool player_is_exist(uint64_t player_id); //是否存在该玩家
int save_friend_player(FriendPlayer *player);
FriendPlayer *get_friend_player(uint64_t player_id);

int get_more_redis_player(std::set<uint64_t> &player_ids, std::map<uint64_t, PlayerRedisInfo*> &redis_players);
void get_all_friend_id(FriendPlayer *player, std::set<uint64_t> &player_ids);
int get_all_friend_redis_player(FriendPlayer *player, std::map<uint64_t, PlayerRedisInfo*> &redis_players);
int get_friend_closeness(FriendPlayer *player, uint64_t friend_id);

PlayerRedisInfo *find_redis_from_map(std::map<uint64_t, PlayerRedisInfo*> &redis_players, uint64_t player_id);
void notify_friend_list_change(FriendPlayer *player, FriendListChangeInfo &changes);
void sync_friend_num_to_game_srv(FriendPlayer *player);

bool is_in_contact(FriendPlayer *player, uint64_t target_id);
bool is_in_block(FriendPlayer *player, uint64_t target_id);

int get_recent_limit_num(void);
int get_block_limit_num(void);
int get_enemy_limit_num(void);
int get_apply_limit_num(void);
int get_group_limit_num(void);
int get_contact_limit_num(FriendPlayer *player);
int get_contact_num(FriendPlayer *player);
int get_contact_idx(FriendPlayer *player, uint64_t member_id);
uint32_t get_new_group_id(FriendPlayer *player);

int add_contact(FriendPlayer *player, uint64_t target_id, FriendListChangeInfo &change_info, bool bDelApply);
int del_contact(FriendPlayer *player, uint64_t target_id, FriendListChangeInfo &change_info);
int add_apply(FriendPlayer *player, uint64_t target_id);
int del_apply(FriendPlayer *player, uint64_t target_id, FriendListChangeInfo *change_info);
int del_apply_idx(FriendPlayer *player, int idx); //只改变数据
int add_block(FriendPlayer *player, uint64_t target_id);
int del_block(FriendPlayer *player, uint64_t target_id, FriendListChangeInfo &change_info);
int add_enemy(FriendPlayer *player, uint64_t target_id);
int del_enemy(FriendPlayer *player, uint64_t target_id, FriendListChangeInfo &change_info);
int add_recent(FriendPlayer *player, uint64_t target_id);
int del_recent(FriendPlayer *player, uint64_t target_id, FriendListChangeInfo &change_info);

void try_reset_friend_player(FriendPlayer *player); //检测每日重置
void notify_friend_closeness_update(FriendPlayer *player, FriendUnit &unit);
std::set<uint64_t> *get_contact_me_players(uint64_t player_id);
std::set<uint64_t> *get_watch_me_players(uint64_t player_id);
void rebuild_watch_info(void); //重建反向映射表
bool is_friend_gone(FriendPlayer *player, uint64_t target_id);

#endif
