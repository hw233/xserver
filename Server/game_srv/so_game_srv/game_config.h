#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <map>
#include <vector>
#include <stdint.h>
#include <stdio.h>
#include "excel_data.h"
#include "lua_config.h"
#include "attr_id.h"

//矩形
#define SKILL_RANGE_TYPE_RECT 1
//扇形
#define SKILL_RANGE_TYPE_FAN 2
//圆形
#define SKILL_RANGE_TYPE_CIRCLE 0
//以目标为中心的矩形
#define SKILL_RANGE_TYPE_TARGET_RECT 3
//直接命中
//#define SKILL_RANGE_TYPE_DIRECT 3

int read_all_excel_data();
int reload_config();

int add_all_scene();
bool is_raid_scene_id(uint32_t id);
bool is_wanyaogu_raid(uint32_t id);
bool is_guild_scene_id(uint32_t id);
bool is_guild_wait_raid(uint32_t id);
bool is_guild_battle_raid(uint32_t id);
bool scene_can_make_team(uint32_t scene_id); //场景是否能组队
int get_scene_birth_pos(uint32_t scene_id, float *pos_x, float *pos_y, float *pos_z, float *face_y);
int bagua_card_to_bind_item(uint32_t card_id);
RandomMonsterTable *get_random_monster_config(uint32_t type, uint64_t level);
RandomDungeonTable *get_random_guoyu_fb_config(uint32_t type);
SpecialtySkillTable *get_yaoshi_skill_config(int type, uint64_t level);
WeekTable *get_rand_week_config();
uint32_t get_rand_question(std::vector<uint32_t> &vtQuestion);
bool check_active_open(uint32_t id, uint32_t &cd);
bool task_is_team(uint32_t task_id);
bool escort_is_team(uint32_t escort_id);
uint64_t get_task_chapter_id(uint32_t task_id);
TaskChapterTable *get_task_chapter_config(uint32_t chapter_id);
void get_task_reward_item_from_config(uint32_t reward_id, std::map<uint32_t, uint32_t> &item_list);

ActorTable *get_actor_config(uint32_t job);
ActorLevelTable *get_actor_level_config(uint32_t job, uint32_t level);
EquipmentTable *get_equip_config(uint32_t job, uint32_t type, uint32_t stair, uint32_t star);
EquipStarLv *get_equip_star_config(uint32_t job, uint32_t type, uint32_t stair, uint32_t star);
EquipLock *get_equip_lock_config(uint32_t job, uint32_t type);
GemAttribute *get_gem_config(uint32_t item_id);
BaguaStarTable *get_bagua_star_config(uint32_t star);
SpecialtyLevelTable *get_specialty_level_table(int type, int level);
TypeLevelTable *get_guoyu_level_table(int level);
ChangeSpecialty *get_change_special_table(int level);
RewardTable *get_chengjie_reward_table(int level);
MoneyQuestTable *get_shangjin_task_table(uint32_t level);
CharmTable *get_charm_table(uint32_t level);
SpecialTitleTable *get_yaoshi_title_table(int type, int level);
FactionBattleTable *get_zhenying_battle_table(uint32_t level);
LifeSkillTable *get_medicine_table(uint32_t type, uint32_t lv);
PartnerLevelTable *get_partner_level_config(uint32_t level);
SkillLvTable *get_skill_level_config(uint32_t skill_id, uint32_t level);
RecruitTable *get_partner_recruit_config(uint32_t type);
GangsSkillTable *get_guild_skill_config(uint32_t type, uint32_t level);
AchievementHierarchyTable *get_achievement_config(uint32_t achievement_id, uint32_t star);
SceneCreateMonsterTable *get_daily_zhenying_truck_config(uint32_t id);

uint32_t get_item_relate_id(uint32_t id);
int get_item_bind_and_unbind_id(uint32_t id, uint32_t *bind_id, uint32_t *unbind_id);
uint32_t get_bag_total_num(uint32_t job, uint32_t level);
uint32_t get_item_stack_num(uint32_t id);
int get_drop_item(uint32_t drop_id, std::map<uint32_t, uint32_t> &item_list, uint32_t stack = 0);
int get_player_sex(uint32_t job); //获取角色性别
int get_task_type(uint32_t task_id);
int task_is_trunk(uint32_t task_id);
int task_is_branch(uint32_t task_id);
bool item_is_bind(uint32_t item_id);
int get_transfer_point(uint32_t transfer_id, uint32_t *scene_id, double *pos_x, double *pos_y, double *pos_z, double *direct); //获取传送点信息
bool item_is_baguapai(uint32_t item_id);
int bagua_item_to_card(uint32_t item_id);
int get_actor_skill_index(uint32_t job, uint32_t skill_id);
bool item_is_partner_fabao(uint32_t item_id);
uint32_t get_friend_close_level(uint32_t closeness);
bool activity_is_open(uint32_t activity_id);
int get_dungeon_type(uint32_t raid_id);
int item_id_to_trade_id(uint32_t item_id);
int trade_id_to_item_id(uint32_t trade_id);

#define DEFAULT_SCENE_ID  (10012) 

enum SCENE_TYPE_DEFINE
{
	SCENE_TYPE_WILD,
	SCENE_TYPE_RAID,
};
//场景表现上的类型，比如帮会领地，实际上是副本，但是表现上是野外，相关的一些进出规则要和野外一样
SCENE_TYPE_DEFINE get_scene_looks_type(uint32_t scene_id);

#endif /* GAME_CONFIG_H */
