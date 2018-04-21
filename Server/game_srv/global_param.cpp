#include "global_param.h"
#include "server_proto.h"

class player_struct;
std::map<uint64_t, player_struct *> player_manager_all_players_id;
std::map<uint64_t, player_struct *> player_manager_all_ai_players_id;
std::list<player_struct *> player_manager_delete_ai_players;
std::list<player_struct *> player_manager_player_free_list;
std::set<player_struct *> player_manager_player_used_list;
std::list<struct ai_player_data *> player_manager_ai_data_free_list;
std::set<struct ai_player_data *> player_manager_ai_data_used_list;

struct comm_pool player_manager_player_data_pool;

class buff_struct;
struct minheap buff_manager_m_minheap;	
std::list<buff_struct *> buff_manager_buff_free_list;
std::set<buff_struct *> buff_manager_buff_used_list;
struct mass_pool buff_manager_buff_data_pool;

class monster_struct;
//class boss_struct;
struct ai_interface *all_monster_ai_interface[MAX_MONSTER_AI_INTERFACE];
std::map<uint64_t, monster_struct *> monster_manager_all_monsters_id;
//std::map<uint64_t, boss_struct *> monster_manager_all_boss_id;
struct minheap monster_manager_m_minheap;
std::list<monster_struct *> monster_manager_delete_list;
std::list<monster_struct *> monster_manager_monster_free_list;
std::set<monster_struct *> monster_manager_monster_used_list;
//std::list<boss_struct *> monster_manager_boss_free_list;
//std::set<boss_struct *> monster_manager_boss_used_list;
struct minheap monster_manager_m_boss_minheap;
struct comm_pool monster_manager_monster_data_pool;
//struct comm_pool monster_manager_boss_data_pool;
std::map<uint64_t, monster_struct *> world_boss_all_monsters_id;//怪物id为索引找世界boss,已约定世界boss的怪物id是唯一的

std::list<cash_truck_struct *> cash_truck_manager_free_list;
std::set<cash_truck_struct *> cash_truck_manager_used_list;
struct comm_pool cash_truck_manager_data_pool;
std::map<uint64_t, cash_truck_struct *> cash_truck_manager_all_id;

class raid_struct;
struct raid_ai_interface *all_raid_ai_interface[MAX_RAID_AI_INTERFACE];
std::map<uint64_t, raid_struct *> raid_manager_all_raid_id;	
std::list<raid_struct *> raid_manager_raid_free_list;
std::set<raid_struct *> raid_manager_raid_used_list;
struct comm_pool raid_manager_raid_data_pool;

class scene_struct;
std::map<uint64_t, scene_struct *> scene_manager_scene_map;

class sight_space_struct;
comm_pool sight_space_manager_sight_space_data_pool;
std::vector<sight_space_struct *> sight_space_manager_mark_delete_sight_space;

class skill_struct;
std::list<skill_struct *> skill_manager_skill_free_list;
std::set<skill_struct *> skill_manager_skill_used_list;
struct comm_pool skill_manager_skill_data_pool;	

class zhenying_raid_struct;
std::map<uint64_t, zhenying_raid_struct *> zhenying_raid_manager_all_raid_id;
std::list<zhenying_raid_struct *> zhenying_raid_manager_raid_free_list;
std::set<zhenying_raid_struct *> zhenying_raid_manager_raid_used_list;
struct comm_pool zhenying_raid_manager_raid_data_pool;
uint64_t zhenying_raid_manager_reflesh_collect;

//struct ProtoGuildInfo;
std::map<uint32_t, ProtoGuildInfo> guild_summary_map;

class Team;
struct matched_team;
std::vector<player_struct *> guild_battle_manager_cant_matched_player;   //不能组成队伍的人
std::set<player_struct *> guild_battle_manager_waiting_player;   //个人
std::set<Team *> guild_battle_manager_waiting_team;  //队伍，保证队伍里面都是同一个工会
std::multimap<uint32_t, struct matched_team *> guild_battle_manager_matched_team; //组合好的队伍

uint32_t guild_battle_manager_activity_start_ts = 0;
uint32_t guild_battle_manager_action_tick = 0;
uint32_t guild_battle_manager_action_state = 0;
uint32_t guild_battle_manager_action_round = 0;
uint32_t guild_battle_manager_action_act = 0;

std::map<uint32_t, uint32_t> guild_battle_manager_guild_participate;
std::map<uint32_t, uint32_t> guild_battle_manager_guild_call_cd;
std::set<uint32_t> guild_battle_manager_final_guild_id; //参加决赛的帮会

uint32_t guild_battle_manager_final_list_state = 0;
uint32_t guild_battle_manager_final_list_tick = 0;

std::set<uint64_t> guild_battle_manager_participate_players; //参加帮会战的玩家

class guild_wait_raid_struct;
std::map<uint64_t, guild_wait_raid_struct *> guild_wait_raid_manager_all_raid_id;
std::list<guild_wait_raid_struct *> guild_wait_raid_manager_raid_free_list;
std::set<guild_wait_raid_struct *> guild_wait_raid_manager_raid_used_list;
struct comm_pool guild_wait_raid_manager_raid_data_pool;
std::map<uint32_t, guild_wait_raid_struct *> guild_wait_raid_manager_raid_map;

class guild_land_raid_struct;
std::map<uint64_t, guild_land_raid_struct *> guild_land_raid_manager_all_raid_id;
std::list<guild_land_raid_struct *> guild_land_raid_manager_raid_free_list;
std::set<guild_land_raid_struct *> guild_land_raid_manager_raid_used_list;
struct comm_pool guild_land_raid_manager_raid_data_pool;
std::map<uint32_t, guild_land_raid_struct *> guild_land_raid_manager_raid_map;

TEAM_MAP team_manager_s_teamContain;
struct comm_pool team_manager_teamDataPool;
TEAM_CONTAIN team_manager_m_team;
ALL_ROLE_CONTAIN team_manager_m_allRole;
TARGET_ROLE_CONTAIN team_manager_m_targetRole;

uint32_t collect_manager_s_id;
COLLECT_MAP collect_manager_s_collectContain;

CHENGJIE_CONTAIN ChengJieTaskManage_m_contain;
CHENGJIE_VECTOR ChengJieTaskManage_m_containVt;
CHENGJIE_TARGET ChengJieTaskManage_m_target;
CHENGJIE_ROLE_LEVEL ChengJieTaskManage_m_RoleLevel;

class partner_struct;
struct partner_ai_interface *all_partner_ai_interface[MAX_PARTNER_AI_INTERFACE];
std::map<uint64_t, partner_struct *> partner_manager_all_partner_id;
std::list<partner_struct *> partner_manager_partner_free_list;
std::set<partner_struct *> partner_manager_partner_used_list;
struct mass_pool partner_manager_partner_data_pool;
struct minheap partner_manager_minheap;

const char g_tmp_name[MAX_PLAYER_NAME_LEN + 1] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
UNIT_FIGHT_TYPE pk_type_to_fight_type[MAX_PK_TYPE][MAX_PK_TYPE];

ZhenyingBattle *g_battle_ins;
int ZhenyingBattle_battle_num = 0;
int zhenying_raid_struct_raid_num = 0;
//PRIVATE_BATTLE_T g_battle_private;

bool do_not_remove_team_member = false;

int collect_g_collect_num;

std::map<uint64_t, uint8_t> pvp_waiting_player_3;
std::map<uint64_t, uint8_t> pvp_waiting_player_5;
std::map<uint64_t, uint8_t> pvp_waiting_team_3;
std::map<uint64_t, uint8_t> pvp_waiting_team_5;
std::map<uint64_t, struct matched_team_3 *> pvp_map_team_3;
std::map<uint64_t, struct matched_team_5 *> pvp_map_team_5;
uint64_t pvp_matched_index = 10;

std::map<uint64_t, uint64_t> g_special_mon_map;



