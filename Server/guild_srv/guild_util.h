#ifndef _GUILD_UTIL_H__
#define _GUILD_UTIL_H__

#include "guild_struct.h"
#include "guild_config.h"
#include "player_redis_info.pb-c.h"
#include "redis_client.h"
#include <string>
#include <vector>
#include <set>
#include <map>
#include "chat.pb-c.h"

typedef size_t(*pack_func)(const void *message, uint8_t *out);

extern CRedisClient sg_redis_client;
extern uint32_t sg_server_id;
extern char sg_player_key[];
extern char sg_rank_guild_battle_key[]; //帮战积分排行
extern char sg_guild_battle_final_key[]; //帮战决赛参与者
extern bool guild_battle_opening;
extern struct event second_timer;
extern struct timeval second_timeout;

void handle_daily_reset_timeout(void);
void cb_second_timer(evutil_socket_t, short, void* /*arg*/);
void load_guild_module(void);
void sync_all_guild_to_gamesrv(void);

PlayerRedisInfo *find_redis_from_map(std::map<uint64_t, PlayerRedisInfo*> &redis_players, uint64_t player_id);
void update_redis_player_guild(GuildPlayer *player);
std::map<uint32_t, GuildInfo*> &get_all_guild(void);
GuildInfo *get_guild(uint32_t guild_id);
GuildPlayer *get_guild_player(uint64_t player_id);

int save_guild_info(GuildInfo *guild);
int save_guild_player(GuildPlayer *player);
int get_guild_level(GuildInfo *guild);
int get_guild_max_member(GuildInfo *guild);
bool check_guild_join_cd(GuildPlayer *player);
bool check_guild_name(std::string &name);
void delete_player_join_apply(uint64_t player_id); //清除玩家的入帮申请信息
void delete_guild_join_apply(uint32_t guild_id); //清除帮会的入帮申请信息
void delete_guild_player_join_apply(uint32_t guild_id, uint64_t player_id); //清除帮会的某个玩家入帮申请信息
bool check_player_applied_join(uint64_t player_id, uint32_t guild_id); //玩家是否已经向帮会申请过
int insert_player_join_apply(uint64_t player_id, uint32_t guild_id); //插入玩家对帮会的申请
int get_player_join_apply(uint64_t player_id, std::vector<uint32_t> &applyIds); //获取玩家的申请列表
int get_guild_join_apply(uint32_t guild_id, std::vector<uint64_t> &applyIds); //获取帮会的申请列表
void broadcast_guild_join_apply(GuildInfo *guild, uint64_t player_id, PlayerRedisInfo *redis_player);
int save_guild_switch(GuildInfo *guild, uint32_t type);
int save_announcement(GuildInfo *guild, uint32_t type);
void replace_guild_master(uint32_t guild_id, uint64_t master_id); //转让帮主，更新数据库
int save_guild_name(GuildInfo *guild);
int delete_guild(uint32_t guild_id);
int save_guild_popularity(GuildInfo *guild);
bool player_has_permission(GuildPlayer *player, uint32_t type); //查看职位是否有操作权限
int insert_one_invite(uint64_t inviter_id, uint64_t invitee_id, uint32_t guild_id); //插入一条邀请信息
void mark_invite_deal(uint64_t inviter_id, uint64_t invitee_id, uint32_t guild_id, uint32_t deal_type);
uint32_t get_invite_cd_time(uint64_t inviter_id, uint64_t invitee_id, uint32_t guild_id);
bool check_invite_is_deal(uint64_t inviter_id, uint64_t invitee_id, uint32_t guild_id);

void sync_guild_rename_to_gamesrv(GuildInfo *guild);
void sync_guild_info_to_gamesrv(GuildPlayer *player);
void sync_guild_task_to_gamesrv(GuildPlayer *player);
void refresh_guild_redis_info(GuildInfo *guild);
int create_guild(uint64_t player_id, uint32_t icon, std::string &name, GuildPlayer *&player); //创建帮会
int join_guild(uint64_t player_id, GuildInfo *guild); //加入帮会
int appoint_office(GuildPlayer *appointor, GuildPlayer *appointee, uint32_t office);
int kick_member(GuildPlayer *player, uint64_t kick_id);
int exit_guild(GuildPlayer *player);
int disband_guild(GuildInfo *&guild);

//帮会资源
void notify_guild_attr_update(uint64_t player_id, uint32_t id, uint32_t val);
void notify_guild_attrs_update(uint64_t player_id, AttrMap &attrs);
void broadcast_guild_attr_update(GuildInfo *guild, uint32_t id, uint32_t val, uint64_t except_id = 0);
void broadcast_guild_str_attr_update(uint32_t id, char *val, std::vector<uint64_t> &player_ids);
void broadcast_guild_str_attr_update(GuildInfo *guild, uint32_t id, char *val, uint64_t except_id = 0);
void broadcast_guild_object_attr_update(GuildInfo *guild, uint32_t type, uint32_t id, uint32_t val, uint64_t except_id = 0);
void get_guild_broadcast_objects(GuildInfo *guild, std::vector<uint64_t> &player_ids, uint64_t except_id = 0);
int add_guild_popularity(GuildInfo *guild, uint32_t num);
int add_guild_treasure(GuildInfo *guild, uint32_t num);
int add_guild_build_board(GuildInfo *guild, uint32_t num);
int add_player_donation(GuildPlayer *player, uint32_t num);
int add_player_contribute_treasure(GuildPlayer *player, uint32_t num);
int sub_guild_popularity(GuildInfo *guild, uint32_t num);
int sub_guild_treasure(GuildInfo *guild, uint32_t num, bool save = true);
int sub_guild_build_board(GuildInfo *guild, uint32_t num, bool save = true);
int sub_player_donation(GuildPlayer *player, uint32_t num, bool save = true);
int add_guild_battle_score(GuildInfo *guild, uint32_t num);
int add_player_battle_score(GuildPlayer *player, uint32_t num);
void broadcast_guild_battle_score(GuildInfo *guild, std::vector<uint64_t> &player_ids);
void sync_player_donation_to_game_srv(GuildPlayer *player, uint32_t is_change = 0, uint32_t change_val = 0);

void init_guild_permission(GuildInfo *guild);
void broadcast_permission_update(GuildInfo *guild, uint32_t office, uint32_t type, uint32_t state, uint64_t except_id = 0);
GuildLog *get_usual_insert_log(GuildInfo *guild);
void broadcast_usual_log_add(GuildInfo *guild, GuildLog *log);
GuildLog *get_important_insert_log(GuildInfo *guild);
void broadcast_important_log_add(GuildInfo *guild, GuildLog *log);

//帮会建筑
int init_guild_building(GuildInfo *guild);
GuildBuilding *get_building_info(GuildInfo *guild, uint32_t type);
int get_building_level(GuildInfo * guild, uint32_t type);
uint32_t get_guild_resource_upper_limit(GuildInfo* guild, uint32_t type);
int upgrade_building_level(GuildInfo *guild);
void broadcast_building_upgrade_update(GuildInfo *guild);
int sub_building_upgrade_time(GuildInfo *guild, uint32_t time);

//帮会技能
GuildSkill *get_guild_skill_info(GuildInfo *guild, uint32_t skill_id);
GuildSkill *get_player_skill_info(GuildPlayer *player, uint32_t skill_id);
void broadcast_skill_develop_update(GuildInfo *guild, GuildSkill *skill);
void sync_guild_skill_to_gamesrv(GuildPlayer *player);

//帮会商店
void resp_guild_shop_info(uint64_t player_id);
GuildGoods *get_player_goods_info(GuildPlayer* player, uint32_t goods_id);
bool goods_is_on_sell(GuildInfo *guild, uint32_t goods_id);

//帮会聊天
void broadcast_guild_chat(GuildInfo *guild, Chat *msg);
void broadcast_guild_message(GuildInfo *guild, uint16_t msg_id, void *msg_data, pack_func func, uint64_t except = 0);
void open_all_guild_answer();

//帮会战
bool guild_battle_is_final(uint32_t activity_id);
void clear_player_act_battle_score(); //清除玩家本次活动积分
void clear_player_total_battle_score(); //清除玩家所有活动积分
void save_guild_battle_final_list(std::vector<uint32_t> &guild_ids);
int load_guild_battle_final_list(std::vector<uint32_t> &guild_ids);
bool is_guild_battle_opening();
bool is_in_guild_battle_activity_time();
uint32_t get_guild_land_active_reward_count(GuildPlayer *player, uint32_t guild_active_id);
void  add_guild_land_active_reward_count(GuildPlayer *player, uint32_t guild_active_id);

int get_player_donate_remain_count(GuildPlayer *player);


#endif
