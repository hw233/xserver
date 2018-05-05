#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <map>
#include <vector>
#include <stdint.h>
#include <stdio.h>
#include "excel_data.h"
#include "lua_config.h"
#include "attr_id.h"
#include "comm_message.pb-c.h"

//矩形
#define SKILL_RANGE_TYPE_RECT 1
//扇形
#define SKILL_RANGE_TYPE_FAN 2
//圆形
#define SKILL_RANGE_TYPE_CIRCLE 0
//以目标为中心的矩形
#define SKILL_RANGE_TYPE_TARGET_RECT 3
//以目标为中心的圆形
#define SKILL_RANGE_TYPE_TARGET_CIRCLE 4

#define MAX_SKILL_TIME_CONFIG_NUM 5

//直接命中
//#define SKILL_RANGE_TYPE_DIRECT 3

int read_all_excel_data();
int free_all_excel_data();
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
int raid_in_raidsrv(uint32_t raid_id);
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
/* void get_skill_configs(uint32_t skill_lv, uint32_t skill_id, struct SkillTable **ski_config, */
/* 	struct SkillLvTable **lv_config1, struct PassiveSkillTable **pas_config, */
/* 	struct SkillLvTable **lv_config2, struct ActiveSkillTable **act_config); */

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
SkillLevelTable *get_partner_skill_level_config(uint32_t id);
bool is_high_partner_skill(uint64_t id);   //是否是高级伙伴技能
int get_partner_skill_levelup_exp(uint64_t id, int *need_lv);   //获取伙伴技能升级所需经验
RecruitTable *get_partner_recruit_config(uint32_t type);
GangsSkillTable *get_guild_skill_config(uint32_t type, uint32_t level);
AchievementHierarchyTable *get_achievement_config(uint32_t achievement_id, uint32_t star);
SceneCreateMonsterTable *get_daily_zhenying_truck_config(uint32_t id);
EquipAttribute *get_rand_attr_config(uint32_t pool, uint32_t attr_id);
TravelTable *get_travel_config(uint32_t level);
GradeTable *get_zhenying_grade_table(uint32_t zhenying, uint32_t level);
CastSpiritTable *get_horse_soul_table(uint64_t horseid, uint32_t step, uint32_t star);

uint32_t get_item_relate_id(uint32_t id);
int get_item_bind_and_unbind_id(uint32_t id, uint32_t *bind_id, uint32_t *unbind_id);
uint32_t get_bag_total_num(uint32_t job, uint32_t level);
uint32_t get_item_stack_num(uint32_t id);
int get_item_quality(uint32_t item_id);
int get_drop_item(uint32_t drop_id, std::map<uint32_t, uint32_t> &item_list, uint32_t stack = 0);
uint32_t get_drop_by_lv(uint32_t lv, uint32_t star, uint32_t n_Rewards, uint64_t *Rewards, uint32_t n_ItemRewardSection, uint64_t *ItemRewardSectio);
#define MAX_DROP_ITEM_DATA_NUM 30
int pack_drop_config_item(uint32_t drop_id, int max, int *begin, ItemData ***point);
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
bool friend_close_can_sworn(uint32_t closeness);
bool friend_close_can_marry(uint32_t closeness);
bool activity_is_open(uint32_t activity_id);
uint32_t get_activity_reward_time(uint32_t activity_id); //获取活动的收益次数
int get_dungeon_type(uint32_t raid_id);
int item_id_to_trade_id(uint32_t item_id);
int trade_id_to_item_id(uint32_t trade_id);
bool strong_goal_is_open(uint32_t goal_id, uint32_t player_lv);
int get_equip_enchant_attr_color(uint32_t pool, uint32_t attr_id, double attr_val);
int get_one_rand_attr(uint32_t pool, uint32_t &attr_id, double &attr_val, std::vector<uint32_t> *except_attrs = NULL);
bool item_is_random_box(uint32_t item_id);
int get_random_box_fixed_item(uint32_t box_id, uint32_t &item_id, uint32_t &item_num);
int get_random_box_random_item(uint32_t box_id, uint32_t &item_id, uint32_t &item_num);
UndergroundTask *get_digong_xiulian_config(uint32_t level);
int get_partner_recruit_convert_item(uint32_t partner_id, uint32_t &item_id, uint32_t &item_num);
bool is_script_raid(uint32_t raid_id);

#define DEFAULT_SCENE_ID  (10012) 

enum SCENE_TYPE_DEFINE
{
	SCENE_TYPE_WILD,
	SCENE_TYPE_RAID,
};
//场景表现上的类型，比如帮会领地，实际上是副本，但是表现上是野外，相关的一些进出规则要和野外一样
SCENE_TYPE_DEFINE get_scene_looks_type(uint32_t scene_id);

/* "0=普通副本 */
/* 3=随机主副本 */
/* 4=随机小关卡 */
/* 5=3V3 PVP副本 */
/* 6=5V5 PVP副本 */
/* 7=妖师客栈 */
/* 8=走AI流程副本 */
/* 9=战场副本 */
/* 10=公会准备区副本 */
/* 11=预赛 */
/* 12=决赛" */
/* 15=普通阵营战 */
/* 16=新手阵营战 */
/* 17=帮会领地副本活动 */
/* 18=猫鬼乐园副本*/
/* 19=幻宝地牢*/
/* 20=情人岛*/
enum DUNGEON_TYPE_DEFINE
{
	DUNGEON_TYPE_NORMAL = 0,
	DUNGEON_TYPE_RAND_MASTER = 3,
	DUNGEON_TYPE_RAND_SLAVE = 4,
	DUNGEON_TYPE_PVP_3 = 5,
	DUNGEON_TYPE_PVP_5 = 6,
	DUNGEON_TYPE_YAOSHI = 7,
	DUNGEON_TYPE_SCRIPT = 8,
	DUNGEON_TYPE_ZHENYING = 9,
	DUNGEON_TYPE_GUILD_WAIT = 10,
	DUNGEON_TYPE_GUILD_RAID = 11,
	DUNGEON_TYPE_GUILD_FINAL_RAID = 12,	
	DUNGEON_TYPE_BATTLE = 15,
	DUNGEON_TYPE_BATTLE_NEW = 16,
	DUNGEON_TYPE_GUILD_LAND = 17,
	DUNGEON_TYPE_MAOGUI_LEYUAN = 18,
	DUNGEON_TYPE_TOWER = 19,
	DUNGEON_TYPE_QINGREN_DAO = 20,
};

#endif /* GAME_CONFIG_H */
