#ifndef __LUA_CONFIG_H__
#define __LUA_CONFIG_H__

#include <map>
#include <vector>
#include <stdint.h>
#include "excel_data.h"
#include "lua_template.h"
#include "attr_id.h"

//一个关卡的万妖卡最多条件参数 
#define MAX_WANYAOKA_COND_PARAM 8  

extern std::map<uint64_t, struct ActiveSkillTable *> active_skill_config;
extern std::map<uint64_t, struct PassiveSkillTable *> passive_skill_config;
extern std::map<uint64_t, struct MonsterTable *> monster_config;
extern std::map<uint64_t, struct ActorTable *> actor_config;
extern std::map<uint64_t, struct ActorAttributeTable *> actor_attribute_config;
extern std::map<uint64_t, struct region_config *> scene_region_config;  //区域数据
extern std::map<uint64_t, struct SceneResTable *> scene_res_config; //阻挡，寻路数据
extern std::map<uint64_t, struct map_config *> scene_map_config;
extern std::map<uint64_t, std::vector<struct SceneCreateMonsterTable *> *> all_scene_create_monster_config;
extern std::map<uint64_t, struct SkillTable *> skill_config;
extern std::map<uint64_t, struct SkillTable *> fuwen_config;
extern std::map<uint64_t, struct SkillLvTable *> skill_lv_config;
extern std::map<uint64_t, struct FlySkillTable *> fly_skill_config;
extern std::map<uint64_t, struct BuffTable *> buff_config;
extern std::map<uint64_t, struct SkillEffectTable *> skill_effect_config;
extern std::map<uint64_t, struct ActorLevelTable *> actor_level_config; //角色等级配置
extern std::map<uint64_t, struct ItemsConfigTable *> item_config; //道具配置
extern std::map<uint64_t, struct ParameterTable *> parameter_config; //参数配置
extern std::map<uint64_t, struct DropConfigTable *> drop_config; //掉落配置
extern std::map<uint64_t, struct BaseAITable *> base_ai_config;
extern std::map<uint64_t, struct ActorHeadTable *> actor_head_config; //头像配置
extern std::map<uint64_t, struct CollectTable *> collect_config;
extern std::map<uint64_t, struct TaskTable *> task_config; //任务配置
extern std::map<uint64_t, struct TaskConditionTable *> task_condition_config; //任务条件配置
extern std::map<uint64_t, struct TaskEventTable *> task_event_config; //任务事件配置
extern std::map<uint64_t, struct TaskDropTable *> task_drop_config; //任务掉落配置
extern std::map<uint64_t, struct TaskRewardTable *> task_reward_config; //任务奖励配置
extern std::map<uint64_t, struct TaskMonsterTable *> task_monster_config; //任务刷怪配置
extern std::map<uint64_t, struct TaskChapterTable *> task_chapter_config; //任务章节配置
extern std::map<uint64_t, struct ActorFashionTable *> fashion_config; //时装配置
extern std::map<uint64_t, struct CharmTable *> charm_config; //时装配置
extern std::map<uint64_t, struct DungeonTable*> all_raid_config; //副本配置
extern std::map<uint64_t, struct ControlTable*> all_control_config; //副本进入条件收益次数配置
extern std::map<uint64_t, struct EquipmentTable*> equipment_config; //装备配置
extern std::map<uint64_t, struct EquipStarLv*> equip_star_config; //装备升星配置
extern std::map<uint64_t, struct EquipLock*> equip_lock_config; //装备镶嵌配置
extern std::map<uint64_t, struct EquipAttribute*> equip_attr_config; //装备随机属性配置
extern std::map<uint64_t, struct GemAttribute*> equip_gem_config; //装备宝石配置
extern std::map<uint64_t, struct AttributeTypeTable*> attribute_type_config; //属性类型配置
extern std::map<uint64_t, struct ColourTable*> color_table_config; //
extern std::map<uint64_t, struct ShopListTable*> shop_list_config; //商城页签配置
extern std::map<uint64_t, struct ShopTable*> shop_config; //商品配置
extern std::map<uint64_t, struct TransferPointTable*> transfer_config; //传送配置
extern std::map<uint64_t, struct SpiritTable*> spirit_config; //坐骑皮肤配置
extern std::map<uint64_t, struct MountsTable*> horse_config; //坐骑皮肤配置
extern std::map<uint64_t, struct CastSpiritTable*> horse_soul_config; //坐骑铸灵配置
extern std::map<uint64_t, struct RandomCardTable*> wanyaoka_config; //万妖卡配置
extern std::map<uint64_t, struct RandomCardRewardTable*> wanyaoka_reward_config; //万妖卡奖励配置
extern std::map<uint64_t, struct PulseTable*> yuqidao_jingmai_config; //御气道经脉配置
extern std::map<uint64_t, struct AcupunctureTable*> yuqidao_acupoint_config; //御气道穴位配置
extern std::map<uint64_t, struct BreakTable*> yuqidao_break_config; //御气道冲穴配置
extern std::map<uint64_t, struct StageTable*> pvp_raid_config; //pvp副本配置
extern std::map<uint64_t, struct BaguaTable*> bagua_config; //八卦牌配置
extern std::map<uint64_t, struct BaguaStarTable*> bagua_star_config; //八卦牌炼星配置
extern std::map<uint64_t, struct BaguaViceAttributeTable*> bagua_vice_attr_config; //八卦牌洗炼配置
extern std::map<uint64_t, struct BaguaSuitTable*> bagua_suit_config; //八卦牌套装配置
extern std::vector<struct BootNameTable*> rand_name_config; //随机名字
extern std::vector<struct ActorRobotTable*> robot_config; //机器人
extern std::map<uint64_t, struct RandomMonsterTable*> random_monster; //
extern std::map<uint64_t, struct SpecialtyLevelTable*> specialty_level_config; //妖师专精
extern std::map<uint64_t, struct TypeLevelTable*> guoyu_level_config; //国御难度
extern std::map<uint64_t, struct ChangeSpecialty*> change_special_config; //改专精费用 
extern std::map<uint64_t, struct RandomDungeonTable*> random_guoyu_dungenon_config; //国御随机副本
extern std::map<uint64_t, struct RewardTable*> chengjie_reward_config; //惩戒奖励
extern std::map<uint64_t, struct SpecialTitleTable*> yaoshi_title_config; //妖师专精称号
extern std::map<uint64_t, struct MoneyQuestTable*> shangjin_task_config; //妖师赏金任务
extern std::map<uint64_t, struct SpecialtySkillTable*> yaoshi_skill_config; //妖师技能
extern std::map<uint64_t, struct EventCalendarTable*> activity_config; //活动配置
extern std::map<uint64_t, struct ActiveTable*> activity_activeness_config; //活动活跃度配置
extern std::map<uint64_t, struct ChivalrousTable*> activity_chivalry_config; //侠义活动配置
extern std::map<uint64_t, struct GangsTable*> guild_building_config; //帮会建筑表
extern std::map<uint64_t, struct GangsJurisdictionTable*> guild_office_config; //帮会职权表
extern std::map<uint64_t, struct GangsSkillTable*> guild_skill_config; //帮会技能表
extern std::map<uint64_t, struct CampTable*> zhenying_base_config; //阵营基础信息表
extern std::map<uint64_t, struct GradeTable*> zhenying_level_config; //阵营等级表
extern std::map<uint64_t, struct WeekTable*> zhenying_week_config; //阵营周目标表
extern std::map<char *, std::vector<struct RaidScriptTable*> *> all_raid_script_config; //副本AI配置表
extern std::map<uint64_t, struct QuestionTable*> questions_config; //考题表
extern std::vector<struct RobotPatrolTable*> robot_patrol_config; //机器人巡逻
extern std::vector<struct FactionBattleTable*> zhenying_battle_config; //战场
extern std::map<uint64_t, struct LifeSkillTable*> medicine_config; //炼药表
extern std::map<uint64_t, struct NoticeTable*> notify_config; //公告表
extern std::map<uint64_t, struct SearchTable*> xunbao_config; //寻宝表
extern std::map<uint64_t, struct TreasureTable*> xunbao_map_config; //寻宝地图表
extern std::map<uint64_t, struct EscortTask*> escort_config; //护送表
extern std::map<uint64_t, struct PartnerTable*> partner_config; //伙伴表
extern std::map<uint64_t, struct GodYaoAttributeTable*> partner_god_attr_config; //伙伴表神耀属性
extern std::map<uint64_t, struct RecruitTable*> partner_recruit_config; //伙伴招募表
extern std::map<uint64_t, struct PartnerLevelTable*> partner_level_config; //伙伴等级表
extern std::map<uint64_t, struct FetterTable*> partner_bond_config; //伙伴羁绊表
extern std::map<uint64_t, struct BiaocheTable*> cash_truck_config; //镖车表
extern std::map<uint64_t, struct FunctionUnlockTable*> function_unlock_config; //功能开启表
extern std::map<uint64_t, struct NpcTalkTable*> monster_talk_config; //怪物冒泡说话
extern std::map<uint64_t, struct MonsterPkTypeTable*> pk_type_config; //pk type 设置
extern std::map<uint64_t, struct BiaocheRewardTable*> cash_truck_reward_config; //
extern std::map<uint64_t, struct WeaponsEffectTable*> weapon_color_config; //
extern std::map<uint64_t, struct LifeMagicTable*> lifemagic_config; //伙伴法宝配置表
extern std::map<uint64_t, struct MagicTable*> MagicTable_config;   //伙伴法宝属性主表
extern std::map<uint64_t, struct MagicAttributeTable*> MagicAttrbute_config;   //伙伴法宝副属性表
//extern std::map<uint64_t, std::vector<struct LifeProbabilitytable *> *> LifeProbabi_config; //法宝命体对应表

//////////////////
extern uint32_t sg_bag_unlock_base_price;
extern uint32_t sg_bag_unlock_incr_factor;
extern uint32_t sg_rename_item_id;
extern uint32_t sg_rename_item_num;
extern std::map<uint64_t, TaskChapterTable*> task_chapter_map;
extern uint32_t sg_first_trunk_task_id; //第一个主线任务id
extern int sg_relive_free_times;
extern int sg_relive_first_cost;
extern int sg_relive_grow_cost;
extern int sg_relive_max_cost;
extern int sg_gem_strip_coin; //宝石剥离消耗银币
extern int sg_player_level_limit; //角色等级上限
extern int sg_wanyaogu_range;
extern int sg_wanyaogu_reward;
extern int sg_wanyaogu_time_delta;
extern int sg_wanyaogu_time_total;
extern int sg_wanyaogu_start_time;

extern int sg_set_pk_type_cd;
extern int sg_pk_level;
extern int sg_muder_num_max;
extern int sg_muder_add_num;
extern int sg_muder_sub_num;
extern int sg_muder_item_value;
extern int sg_qiecuo_out_range_timeout;
extern int sg_qiecuo_range;
extern uint32_t sg_qiecuo_god_time;  //切磋倒计时

extern float sg_hp_pool_percent;  

extern int sg_raid_keep_time;

extern std::vector<uint32_t> sg_vec_wanyaogu_raid_id;

extern int sg_yuqidao_break_item_id;
extern int sg_yuqidao_break_item_num;
extern double sg_fighting_capacity_coefficient[PLAYER_ATTR_FIGHT_MAX]; //战斗力系数

extern struct ControlTable *sg_pvp_control_config_3;
extern struct ControlTable *sg_pvp_control_config_5;

extern int sg_guild_raid_param1[6];  //队伍1出生点
extern int sg_guild_raid_param2[6];  //队伍2出生点
extern int sg_guild_raid_final_param1[6];  //队伍1出生点
extern int sg_guild_raid_final_param2[6];  //队伍2出生点
extern int sg_guild_raid_final_param3[6];  //队伍3出生点
extern int sg_guild_raid_final_param4[6];  //队伍4出生点

extern int sg_3v3_pvp_raid_param1[6];
extern int sg_3v3_pvp_raid_param2[6];
extern int sg_5v5_pvp_raid_param1[6];
extern int sg_5v5_pvp_raid_param2[6];
extern int sg_3v3_pvp_monster_id[3];
extern int sg_5v5_pvp_monster_id[3];
extern int sg_3v3_pvp_monster_place[13];
extern int sg_5v5_pvp_monster_place[13];
extern int sg_pvp_raid_relive_cd;
extern int sg_pvp_raid_monster_refresh_time;
extern int sg_pvp_raid_monster_first_refresh_time;
extern int sg_pvp_raid_red_region_buff_id[2];
extern int sg_pvp_raid_blue_region_buff_id[2];
extern int sg_pvp_raid_blue_region_buff_rate;

extern int MATCH_LEVEL_DIFF;         //段位范围
extern int TEAM_LEVEL1_DIFF_L;       //对手段位下限
extern int TEAM_LEVEL1_DIFF_R;
extern int TEAM_LEVEL2_DIFF_L;       //对手等级下限
extern int TEAM_LEVEL2_DIFF_R;

extern int sg_muder_punish_point;
extern int sg_muder_punish_base;
extern int sg_muder_punish_inc[2];
extern int sg_muder_punish_max;
extern int sg_muder_cant_set_pktype;
extern int sg_muder_debuff[2];

extern double sg_pvp_raid_win_score_param;
extern double sg_pvp_raid_tie_score_param;
extern double sg_pvp_raid_lose_score_param;
extern double sg_pvp_raid_win_gold_param;
extern double sg_pvp_raid_tie_gold_param;
extern double sg_pvp_raid_lose_gold_param;
extern double sg_pvp_raid_reward_score_3v3_param;
extern double sg_pvp_raid_reward_score_5v5_param;
extern double sg_pvp_raid_kill_param;
extern double sg_pvp_raid_assist_param;
extern double sg_pvp_raid_basic_param;

extern uint32_t sg_hp_pool_max;

extern int sg_3v3_pvp_buff_id[3];
extern int sg_5v5_pvp_buff_id[3];

extern int sg_3v3_pvp_raid_red_buff_param[5];
extern int sg_3v3_pvp_raid_blue_buff_param[5];
extern int sg_5v5_pvp_raid_red_buff_param[5];
extern int sg_5v5_pvp_raid_blue_buff_param[5];

extern uint32_t sg_pvp_raid_buff_relive_time;
extern int sg_pvp_center_buff_id[2];

extern uint32_t sg_guild_raid_final_monster_id[4];  //帮会战决赛4组小怪的ID

extern double sg_leiminggu_pos[4]; //雷鸣鼓坐标
extern double sg_leiminggu_boss_pos[4]; //雷鸣鼓boss坐标
extern uint32_t sg_leiminggu_collect_id; //雷鸣鼓采集物ID

extern uint32_t sg_shishen_xiaoguai_id[5];  //侍神小怪ID
extern uint32_t sg_shishen_shouling_id[5];  //侍神首领ID

enum YAOSHI_SKILL
{
	SHANGJIN_ONE = 1,
	SHANGJIN_TWO = 2,
	SHANGJIN_THREE = 3,
	SHANGJIN_FOUR = 4,
	CHENGJIE_FIVE = 5,
	CHENGJIE_SIX = 6,
	CHENGJIE_SEVEN = 7,
	CHENGJIE_EIGHT = 8,
	GUOYU_NINE = 9,
	GUOYU_TEN = 10,
	GUOYU_ELEVEN = 11,
	GUOYU_TWELVE = 12,
	YAOSHI_SKILL_MAX_NUM
};

#define MAX_GUOYU_TASK_TYPE  (4)

extern uint64_t sg_yaoshi_level_limited[YAOSHI_SKILL_MAX_NUM];

extern int sg_pvp_raid_fighting_capacity_range[2];
extern std::map<uint32_t, uint32_t> sg_bagua_bind_item_map; //通过八卦ID找绑定道具ID
extern std::map<uint32_t, uint32_t> sg_partner_item_map; //skill_id, book_id
extern std::map<uint32_t, struct SpecialtySkillTable*> sg_yaoshi_skill_map; //
extern std::map<uint32_t, std::vector<RandomMonsterTable *> > sg_random_monster_map; //
extern std::vector<RandomDungeonTable *> random_guoyu_fb_arr[MAX_GUOYU_TASK_TYPE + 1];
extern std::vector<uint32_t> sg_common_question;
extern std::vector<uint32_t> sg_award_question;
extern std::map<uint64_t, struct SearchTable*> sg_xunbao; //寻宝表
extern std::map<uint64_t, std::vector<uint64_t> > sg_xunbao_map; //寻宝地图
extern std::map<uint64_t, struct FunctionUnlockTable*> sg_jijiangopen; //即将开启表

extern uint32_t sg_transfer_out_stuck_cd_time;
extern uint32_t sg_guild_scene_id;
extern uint32_t sg_guild_wait_raid_id; //帮战准备区场景ID
extern uint32_t sg_guild_battle_match_time; //帮战预赛匹配时间
extern uint32_t sg_guild_battle_fight_time; //帮战预赛战斗时间
extern uint32_t sg_guild_battle_settle_time; //帮战预赛结算时间
extern uint32_t sg_guild_battle_round_num; //帮战预赛回合数
extern uint32_t sg_guild_battle_final_match_time; //帮战决赛匹配时间
extern uint32_t sg_guild_battle_final_fight_time; //帮战决赛战斗时间
extern uint32_t sg_guild_battle_final_settle_time; //帮战决赛结算时间
extern uint32_t sg_guild_battle_final_round_num; //帮战决赛回合数
extern uint32_t sg_guild_battle_brave_init; //帮战初始勇武值
extern uint32_t sg_guild_battle_wait_award_interval; //帮战等待区奖励时间间隔
extern std::map<uint32_t, uint32_t> sg_guild_battle_wait_award;
extern int sg_guild_battle_fight_win_reward[4];
extern int sg_guild_battle_fight_lose_reward[4];
extern int sg_guild_battle_fight_draw_reward[4];
extern int sg_guild_battle_fight_auto_win_reward[4];
extern int sg_guild_battle_treasure_factor[3];
extern int sg_guild_battle_final_fight_reward_0[4];
extern int sg_guild_battle_final_fight_reward_1[3];
extern int sg_guild_battle_final_fight_reward_2[3];
extern int sg_guild_battle_final_fight_reward_3[3];
extern int sg_guild_battle_final_fight_reward_4[3];
extern int sg_guild_battle_final_treasure_factor[1];
extern int sg_guild_battle_final_score_factor[4];

extern double sg_partner_assist_percent;
extern uint32_t sg_partner_anger_max;
extern uint32_t sg_partner_relive_time;
extern uint32_t sg_partner_sanshenshi_id;
extern uint32_t sg_partner_sanshenshi_score;
extern uint32_t sg_partner_sanshenshi_coin;
extern uint32_t sg_partner_qiyaoshi_id;
extern uint32_t sg_partner_qiyaoshi_score;
extern uint32_t sg_partner_qiyaoshi_coin;

extern double sg_fight_param_161000274;                //体质转生命系数
extern double sg_fight_param_161000275;                //力量转攻击系数
extern double sg_fight_param_161000276;                //敏捷转闪避系数
extern double sg_fight_param_161000277;                //敏捷转全系抗性系数
extern double sg_fight_param_161000278;                //灵巧转命中系数
extern double sg_fight_param_161000279;                //灵巧转会心几率系数
extern double sg_fight_param_161000280;                //命中等级系数
extern double sg_fight_param_161000281;                //命中基础值
extern double sg_fight_param_161000282;                //闪避等级系数
extern double sg_fight_param_161000283;                //闪避基础值
extern double sg_fight_param_161000284;                //实际命中几率下限
extern double sg_fight_param_161000285;                //会心等级系数
extern double sg_fight_param_161000286;                //会心基础值
extern double sg_fight_param_161000287;                //五行抗性等级系数
extern double sg_fight_param_161000288;                //五行抗性基础值
extern double sg_fight_param_161000289;                //特殊属性基础值
extern double sg_fight_param_161000290;                //buff持续时间保底比例
extern double sg_fight_param_161000291;                //PVP等级系数
extern double sg_fight_param_161000292;                //PVP基础值
extern double sg_fight_param_161000293;                //PVP保底比例

#endif /* __LUA_CONFIG_H__ */
