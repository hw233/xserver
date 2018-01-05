#include "lua_config.h"
//#include <stdio.h>
//#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//extern "C"
//{
//#include "lua5.2/lua.h"
//#include "lua5.2/lualib.h"
//#include "lua5.2/lauxlib.h"
//};
//#include "sproto.h"
//#include "sprotoc_common.h"
//#include "game_event.h"

std::map<uint64_t, struct PassiveSkillTable *> passive_skill_config;
std::map<uint64_t, struct ActiveSkillTable *> active_skill_config;
std::map<uint64_t, struct SkillMoveTable *> move_skill_config;
std::map<uint64_t, struct MonsterTable *> monster_config;
std::map<uint64_t, struct ActorTable *> actor_config;
std::map<uint64_t, struct ActorAttributeTable *> actor_attribute_config;
std::map<uint64_t, struct SceneResTable *> scene_res_config;
std::map<uint64_t, struct region_config *> scene_region_config;  //区域数据
std::map<uint64_t, struct map_config *> scene_map_config;  //阻挡，寻路数据
std::map<uint64_t, struct SkillTable *> skill_config;
std::map<uint64_t, struct SkillTable *> fuwen_config;
std::map<uint64_t, struct SkillLvTable *> skill_lv_config;
std::map<uint64_t, struct FlySkillTable *> fly_skill_config;
std::map<uint64_t, struct BuffTable *> buff_config;
std::map<uint64_t, struct SkillEffectTable *> skill_effect_config;
std::map<uint64_t, struct ActorLevelTable *> actor_level_config;
std::map<uint64_t, struct ItemsConfigTable *> item_config;
std::map<uint64_t, struct ParameterTable *> parameter_config;
std::map<uint64_t, struct DropConfigTable *> drop_config;
std::map<uint64_t, struct BaseAITable *> base_ai_config;
std::map<uint64_t, struct ActorHeadTable *> actor_head_config; //头像配置
std::map<uint64_t, struct CollectTable *> collect_config;
std::map<uint64_t, struct TaskTable *> task_config; //任务配置
std::map<uint64_t, struct TaskConditionTable *> task_condition_config; //任务条件配置
std::map<uint64_t, struct TaskEventTable *> task_event_config; //任务事件配置
std::map<uint64_t, struct TaskDropTable *> task_drop_config; //任务掉落配置
std::map<uint64_t, struct TaskRewardTable *> task_reward_config; //任务奖励配置
std::map<uint64_t, struct TaskMonsterTable *> task_monster_config; //任务刷怪配置
std::map<uint64_t, struct TaskChapterTable *> task_chapter_config; //任务章节配置
std::map<uint64_t, struct ActorFashionTable *> fashion_config; //时装配置
std::map<uint64_t, struct DungeonTable*> all_raid_config; //副本配置
std::map<uint64_t, struct ControlTable*> all_control_config; //副本进入条件收益次数配置
std::map<uint64_t, struct EquipmentTable*> equipment_config; //装备配置
std::map<uint64_t, struct EquipStarLv*> equip_star_config; //装备升星配置
std::map<uint64_t, struct EquipLock*> equip_lock_config; //装备镶嵌配置
std::map<uint64_t, struct EquipAttribute*> equip_attr_config; //装备随机属性配置
std::map<uint64_t, struct GemAttribute*> equip_gem_config; //装备宝石配置
std::map<uint64_t, struct AttributeTypeTable*> attribute_type_config; //属性类型配置
std::map<uint64_t, struct ColourTable*> color_table_config; //
std::map<uint64_t, struct ShopListTable*> shop_list_config; //商城页签配置
std::map<uint64_t, struct ShopTable*> shop_config; //商品配置
std::map<uint64_t, struct TransferPointTable*> transfer_config; //传送配置
std::map<uint64_t, struct SpiritTable*> spirit_config; //坐骑皮肤配置
std::map<uint64_t, struct MountsTable*> horse_config; //坐骑皮肤配置
std::map<uint64_t, struct CastSpiritTable*> horse_soul_config; //坐骑铸灵配置
std::map<uint64_t, struct RandomCardTable*> wanyaoka_config; //万妖卡配置
std::map<uint64_t, struct RandomCardRewardTable*> wanyaoka_reward_config; //万妖卡奖励配置
std::map<uint64_t, struct PulseTable*> yuqidao_jingmai_config; //御气道经脉配置
std::map<uint64_t, struct AcupunctureTable*> yuqidao_acupoint_config; //御气道穴位配置
std::map<uint64_t, struct BreakTable*> yuqidao_break_config; //御气道冲穴配置
std::map<uint64_t, struct StageTable*> pvp_raid_config; //pvp副本配置
std::map<uint64_t, struct BaguaTable*> bagua_config; //八卦牌配置
std::map<uint64_t, struct BaguaStarTable*> bagua_star_config; //八卦牌炼星配置
std::map<uint64_t, struct BaguaViceAttributeTable*> bagua_vice_attr_config; //八卦牌洗炼配置
std::map<uint64_t, struct BaguaSuitTable*> bagua_suit_config; //八卦牌套装配置
std::vector<struct ActorRobotTable*> robot_config[ROBOT_CONFIG_TYPE_SIZE]; //机器人
std::vector<struct BootNameTable*> rand_name_config; //随机名字
std::map<uint64_t, struct RandomMonsterTable*> random_monster;
std::map<uint64_t, struct SpecialtyLevelTable*> specialty_level_config;
std::map<uint64_t, struct TypeLevelTable*> guoyu_level_config; //国御难度
std::map<uint64_t, struct ChangeSpecialty*> change_special_config; //改专精费用 
std::map<uint64_t, struct RandomDungeonTable*> random_guoyu_dungenon_config; //国御随机副本
std::map<uint64_t, struct RewardTable*> chengjie_reward_config; //惩戒奖励
std::map<uint64_t, struct SpecialTitleTable*> yaoshi_title_config; //妖师专精称号
std::map<uint64_t, struct MoneyQuestTable*> shangjin_task_config; //妖师赏金任务
std::map<uint64_t, struct SpecialtySkillTable*> yaoshi_skill_config; //妖师技能
std::map<uint64_t, struct EventCalendarTable*> activity_config; //活动配置
std::map<uint64_t, struct ActiveTable*> activity_activeness_config; //活动活跃度配置
std::map<uint64_t, struct ChivalrousTable*> activity_chivalry_config; //侠义活动配置
std::map<uint64_t, struct GangsTable*> guild_building_config; //帮会建筑表
std::map<uint64_t, struct GangsJurisdictionTable*> guild_office_config; //帮会职权表
std::map<uint64_t, struct GangsSkillTable*> guild_skill_config; //帮会技能表
std::map<uint64_t, struct GangsBuildTaskTable*> guild_build_task_config; //帮会建设任务表
std::map<uint64_t, struct CampTable*> zhenying_base_config; //阵营基础信息表
std::map<uint64_t, struct BattlefieldTable*> zhenying_fight_config; //阵营战表
std::map<uint64_t, struct BattleFieldRank*> zhenying_fight_rank_config; //阵营战表
std::map<uint64_t, struct GradeTable*> zhenying_level_config; //阵营等级表
std::map<uint64_t, struct WeekTable*> zhenying_week_config; //阵营周目标表
std::map<uint64_t, struct QuestionTable*> questions_config; //考题表
std::map<char *, std::vector<struct RaidScriptTable*> *> all_raid_script_config; //副本AI配置表
std::vector<struct RobotPatrolTable*> robot_patrol_config; //机器人巡逻
std::vector<struct RobotPatrolTable*> robot_zhenyingzhan_config; //阵营战机器人移动路径
std::vector<struct FactionBattleTable*> zhenying_battle_config; //战场
std::map<uint64_t, struct LifeSkillTable*> medicine_config; //炼药表
std::map<uint64_t, struct NoticeTable*> notify_config; //公告表
std::map<uint64_t, struct SearchTable*> xunbao_config; //寻宝表
std::map<uint64_t, struct TreasureTable*> xunbao_map_config; //寻宝地图表
std::map<uint64_t, struct EscortTask*> escort_config; //护送表
std::map<uint64_t, struct PartnerTable*> partner_config; //伙伴表
std::map<uint64_t, struct GodYaoAttributeTable*> partner_god_attr_config; //伙伴表神耀属性
std::map<uint64_t, struct RecruitTable*> partner_recruit_config; //伙伴招募表
std::map<uint64_t, struct PartnerLevelTable*> partner_level_config; //伙伴等级表
std::map<uint64_t, struct FetterTable*> partner_bond_config; //伙伴羁绊表
std::map<uint64_t, struct BiaocheTable*> cash_truck_config; //镖车表
std::map<uint64_t, struct FunctionUnlockTable*> function_unlock_config; //功能开启表
std::map<uint64_t, struct NpcTalkTable*> monster_talk_config; //怪物冒泡说话
std::map<uint64_t, struct MonsterPkTypeTable*> pk_type_config; //pk type 设置
std::map<uint64_t, struct BiaocheRewardTable*> cash_truck_reward_config; //
std::map<uint64_t, struct CharmTable *> charm_config; //时装配置
std::map<uint64_t, struct WeaponsEffectTable*> weapon_color_config; //

std::map<uint64_t, std::vector<struct SceneCreateMonsterTable *> *> all_scene_create_monster_config;
std::map<uint64_t, struct LifeMagicTable*> lifemagic_config; //伙伴法宝配置表
std::map<uint64_t, struct MagicTable*> MagicTable_config;   //伙伴法宝属性主表
std::map<uint64_t, struct MagicAttributeTable*> MagicAttrbute_config;   //伙伴法宝副属性表
std::map<uint64_t, struct GenerateMonster*> GenerateMonster_config;   //定时刷怪配置
//std::map<uint64_t, std::vector<struct LifeProbabilitytable *> *> LifeProbabi_config; //法宝命体对应表
std::map<uint64_t, struct ServerResTable*> server_res_config; //服务器资源配置
std::map<uint64_t, struct ServerLevelTable*> server_level_config; //服务器等级配置
std::map<uint64_t, struct AchievementFunctionTable*> achievement_function_config; //成就功能点表
std::map<uint64_t, struct AchievementHierarchyTable*> achievement_hierarchy_config; //成就层级表
std::map<uint64_t, struct DegreeTable*> friend_close_config; //好友亲密度表
std::map<uint64_t, struct ArenaRewardTable*> doufachang_reward_config; //斗法场奖励
std::map<uint64_t, struct TitleFunctionTable*> title_function_config; //称号功能点表
std::map<uint64_t, struct WorldBossTable*> world_boss_config; //世界boss表(以id为索引)
std::map<uint64_t, struct WorldBossTable*> monster_to_world_boss_config; //世界boss表(以怪物id为索引)
std::map<uint64_t, struct ChallengeTable*> hero_challenge_config; //英雄挑战
std::map<uint64_t, struct ChallengeTable*> raidid_to_hero_challenge_config; //英雄挑战(以副本id为索引)
std::map<uint64_t, struct UndergroundTask*> mijing_xiulian_config; //秘境修炼任务信息
std::map<uint64_t, struct UndergroundTask*> taskid_to_mijing_xiulian_config; //秘境修炼任务信息(以任务id为索引)
std::map<uint64_t, struct CampDefenseTable*> zhenying_daily_config; //日常阵营战
std::map<uint64_t, struct FishingTable*> fishing_config; //钓鱼表
std::map<uint64_t, struct GrowupTable*> strong_config; //变强表
std::map<uint64_t, struct FactionActivity*> guild_activ_config; //帮会活动表
std::map<char *, std::vector<struct SceneCreateMonsterTable*> *> all_raid_ai_monster_config; //副本AI表里面配置的怪物刷新表
std::set<char* > some_monster_config_name; //有些场景会配置多个怪物刷新表文件，这里集合表名 
std::map<uint64_t, struct TradingTable*> trade_item_config; //交易物品表
std::map<uint64_t, struct AuctionTable*> auction_config; //拍卖品表
std::map<uint64_t, struct MGLYdiaoxiangTable*> maogui_diaoxiang_config; //猫鬼乐园雕像表
std::map<uint64_t, struct MGLYmaoguiTable*> maogui_monster_config; //猫鬼乐园相关猫鬼怪物表
std::map<uint64_t, struct MGLYyanseTable*> maogui_colour_config; //猫鬼乐园猫鬼颜色配置表
std::map<uint64_t, struct MGLYyanseTable*> maogui_diaoxiang_colour_config; //猫鬼乐园猫鬼雕像颜色配置表
std::map<uint64_t, struct MGLYyanseTable*> maogui_zhengning_colour_config; //猫鬼乐园狰狞猫鬼颜色配置表
std::map<uint64_t, struct MGLYyanseTable*> maogui_shouling_colour_config; //猫鬼乐园鬼王召唤的首领怪颜色配置表
std::map<uint64_t, struct MGLYyanseTable*> maogui_xiaoguai_colour_config;  //猫鬼乐园鬼王召唤的小怪颜色配置表
std::map<uint64_t, struct MGLYmaoguiwangTable*> maogui_maogui_wang_config; //猫鬼乐园猫鬼王配置表
std::map<uint64_t, struct MGLYshoulingTable*> maogui_shouling_to_xiaoguai_config; //猫鬼乐园猫首领死亡删除对应小怪
std::map<uint64_t, struct MonsterIDTable*> raid_jincheng_suiji_kill_monster; //副本进程击杀id对应此表的随机一只怪物，进程加一
std::vector<struct raidsrv_config*> vec_raidsrv_config;
std::map<uint32_t, uint32_t> map_raidsrv_config;
std::map<uint64_t, struct TravelTable*> travel_config; //游历任务表
std::map<uint64_t, struct LevelReward*> level_reward_config; //等级奖励表
std::map<uint64_t, struct TimeReward*> online_reward_config; //在线奖励表
std::map<uint64_t, struct OnlineTimes*> online_time_config; //在线奖励时间表
std::map<uint64_t, struct SignDay*> sign_day_config; //每日签到奖励表
std::map<uint64_t, struct SignMonth*> sign_month_config; //每日签到累计奖励表
std::map<uint64_t, std::map<uint64_t, struct SignDay*> >sign_day_zhuan_config; //每日签到奖励表转表
std::map<uint64_t, struct SignDay*> sign_day_to_zhuan_config[MAX_ONE_YEARS_THE_MONTH]; //12个月
std::map<uint64_t, std::map<uint64_t, struct SignMonth*> >sign_month_zhuan_config; //每日签到累计奖励表转表
std::map<uint64_t, struct SignMonth*> sign_month_to_zhuan_config[MAX_ONE_YEARS_THE_MONTH]; //12个月
std::map<uint64_t, struct RewardBack*> reward_back_config; //活动奖励找回表
std::map<uint64_t, struct P20076Table*> tower_level_config; //冲塔表
std::map<uint64_t, struct LoginGifts*> login_gifts_config; //登录奖励表
std::map<uint64_t, struct CiFuTable*> ci_fu_config; //赐福奖励表
std::map<uint64_t, struct RandomBox*> random_box_config; //随机宝箱表

/////////////////////
uint32_t sg_bag_unlock_base_price;
uint32_t sg_bag_unlock_incr_factor;
uint32_t sg_rename_item_id;
uint32_t sg_rename_item_num;
uint32_t sg_first_trunk_task_id = 0;
std::map<uint64_t, TaskChapterTable*> task_chapter_map;
int sg_fight_rand;
int sg_relive_free_times;
int sg_relive_first_cost;
int sg_relive_grow_cost;
int sg_relive_max_cost;
int sg_gem_strip_coin; //宝石剥离消耗银币
int sg_player_level_limit; //角色等级上限
int sg_wanyaogu_range;
int sg_wanyaogu_reward;
int sg_wanyaogu_time_delta;
int sg_wanyaogu_time_total;
int sg_wanyaogu_start_time;

int sg_set_pk_type_cd;
int sg_pk_level;
int sg_muder_num_max;
int sg_muder_add_num;
int sg_muder_sub_num;
int sg_muder_item_value;
int sg_qiecuo_out_range_timeout;
int sg_qiecuo_range;
uint32_t sg_qiecuo_god_time;  //切磋倒计时

float sg_hp_pool_percent;

int sg_raid_keep_time;

std::vector<uint32_t> sg_vec_wanyaogu_raid_id;

int sg_yuqidao_break_item_id;
int sg_yuqidao_break_item_num;
double sg_fighting_capacity_coefficient[PLAYER_ATTR_MAX]; //战斗力系数
uint32_t sg_fighting_capacity_count_in[PLAYER_ATTR_MAX]; //战斗力计算
double sg_fighting_capacity_crt_dmg_init_val;

struct ControlTable *sg_pvp_control_config_3;
struct ControlTable *sg_pvp_control_config_5;

int sg_guild_raid_param1[6];  //队伍1出生点
int sg_guild_raid_param2[6];  //队伍2出生点
int sg_guild_raid_final_param1[6];  //队伍1出生点
int sg_guild_raid_final_param2[6];  //队伍2出生点
int sg_guild_raid_final_param3[6];  //队伍3出生点
int sg_guild_raid_final_param4[6];  //队伍4出生点

int sg_3v3_pvp_raid_param1[6];  //队伍1出生点
int sg_3v3_pvp_raid_param2[6];  //队伍2出生点
int sg_5v5_pvp_raid_param1[6]; //队伍1出生点
int sg_5v5_pvp_raid_param2[6]; //队伍2出生点
int sg_3v3_pvp_monster_id[3];
int sg_5v5_pvp_monster_id[3];
int sg_3v3_pvp_monster_place[13];
int sg_5v5_pvp_monster_place[13];

int sg_3v3_pvp_buff_id[3];
int sg_5v5_pvp_buff_id[3];

int sg_pvp_raid_red_region_buff_id[2];
int sg_pvp_raid_blue_region_buff_id[2];
int sg_pvp_raid_blue_region_buff_rate; //蓝色buff触发几率

double sg_pvp_raid_win_score_param;
double sg_pvp_raid_tie_score_param;
double sg_pvp_raid_lose_score_param;
double sg_pvp_raid_win_gold_param;
double sg_pvp_raid_tie_gold_param;
double sg_pvp_raid_lose_gold_param;
double sg_pvp_raid_reward_score_3v3_param;
double sg_pvp_raid_reward_score_5v5_param;
double sg_pvp_raid_kill_param;
double sg_pvp_raid_assist_param;
double sg_pvp_raid_basic_param;

uint32_t sg_hp_pool_max;

int MATCH_LEVEL_DIFF;         //段位范围
int TEAM_LEVEL1_DIFF_L;       //对手段位下限
int TEAM_LEVEL1_DIFF_R;
int TEAM_LEVEL2_DIFF_L;       //对手等级下限
int TEAM_LEVEL2_DIFF_R;

int sg_muder_punish_point;
int sg_muder_punish_base;
int sg_muder_punish_inc[2];
int sg_muder_punish_max;
int sg_muder_cant_set_pktype;
int sg_muder_debuff[2];

int sg_pvp_raid_relive_cd;
int sg_pvp_raid_monster_refresh_time;;
int sg_pvp_raid_monster_first_refresh_time;;

int sg_3v3_pvp_raid_red_buff_param[5];
int sg_3v3_pvp_raid_blue_buff_param[5];
int sg_5v5_pvp_raid_red_buff_param[5];
int sg_5v5_pvp_raid_blue_buff_param[5];

uint32_t sg_guild_raid_final_monster_id[4];  //帮会战决赛4组小怪的ID

double sg_leiminggu_pos[4]; //雷鸣鼓坐标
double sg_leiminggu_boss_pos[4]; //雷鸣鼓boss坐标
uint32_t sg_leiminggu_collect_id; //雷鸣鼓采集物ID

uint8_t sg_n_shishen_id;
uint32_t *sg_shishen_xiaoguai_id;  //侍神小怪ID
uint32_t *sg_shishen_shouling_id;  //侍神首领ID

uint32_t sg_pvp_raid_buff_relive_time;
int sg_pvp_center_buff_id[2];

int sg_pvp_raid_fighting_capacity_range[2];
std::map<uint32_t, uint32_t> sg_bagua_bind_item_map; //通过八卦ID找绑定道具ID
std::map<uint32_t, uint32_t> sg_partner_item_map; //skill_id, book_id

std::vector<uint32_t> sg_common_question;
std::vector<uint32_t> sg_award_question;

std::vector<RandomDungeonTable *> random_guoyu_fb_arr[MAX_GUOYU_TASK_TYPE + 1];
uint64_t sg_yaoshi_level_limited[YAOSHI_SKILL_MAX_NUM];
std::map<uint32_t, struct SpecialtySkillTable*> sg_yaoshi_skill_map; //
std::map<uint32_t, std::vector<RandomMonsterTable *> > sg_random_monster_map; //
std::map<uint64_t, struct SearchTable*> sg_xunbao; //寻宝表
std::map<uint64_t, std::vector<uint64_t> > sg_xunbao_map; //寻宝地图
std::map<uint64_t, uint64_t> sg_show_collect; //任务ID 采集点ID
std::map<uint64_t, struct FunctionUnlockTable*> sg_jijiangopen; //即将开启表
std::map<uint32_t, std::vector<BattleFieldStepRank *> > sg_battle_award;

uint32_t sg_transfer_out_stuck_cd_time;
uint32_t sg_guild_scene_id; //帮会领地场景ID
uint32_t sg_guild_wait_raid_id = 20029; //帮战准备区场景ID
uint32_t sg_guild_battle_match_time; //帮战预赛匹配时间
uint32_t sg_guild_battle_fight_time; //帮战预赛战斗时间
uint32_t sg_guild_battle_settle_time; //帮战预赛结算时间
uint32_t sg_guild_battle_round_num; //帮战预赛回合数
uint32_t sg_guild_battle_final_match_time; //帮战决赛匹配时间
uint32_t sg_guild_battle_final_fight_time; //帮战决赛战斗时间
uint32_t sg_guild_battle_final_settle_time; //帮战决赛结算时间
uint32_t sg_guild_battle_final_round_num; //帮战决赛回合数
uint32_t sg_guild_battle_brave_init; //帮战初始勇武值
uint32_t sg_guild_battle_wait_award_interval; //帮战等待区奖励时间间隔
std::map<uint32_t, uint32_t> sg_guild_battle_wait_award;
int sg_guild_battle_fight_win_reward[4];
int sg_guild_battle_fight_lose_reward[4];
int sg_guild_battle_fight_draw_reward[4];
int sg_guild_battle_fight_auto_win_reward[4];
int sg_guild_battle_treasure_factor[3];
int sg_guild_battle_final_fight_reward_0[4];
int sg_guild_battle_final_fight_reward_1[3];
int sg_guild_battle_final_fight_reward_2[3];
int sg_guild_battle_final_fight_reward_3[3];
int sg_guild_battle_final_fight_reward_4[3];
int sg_guild_battle_final_treasure_factor[1];
int sg_guild_battle_final_score_factor[4];

double sg_partner_assist_percent;
uint32_t sg_partner_anger_max;
uint32_t sg_partner_relive_time;
uint32_t sg_partner_sanshenshi_id;
uint32_t sg_partner_sanshenshi_score;
uint32_t sg_partner_sanshenshi_coin;
uint32_t sg_partner_qiyaoshi_id;
uint32_t sg_partner_qiyaoshi_score;
uint32_t sg_partner_qiyaoshi_coin;

double sg_fight_param_161000274;                //体质转生命系数
double sg_fight_param_161000275;                //力量转攻击系数
double sg_fight_param_161000276;                //敏捷转闪避系数
double sg_fight_param_161000277;                //敏捷转全系抗性系数
double sg_fight_param_161000278;                //灵巧转命中系数
double sg_fight_param_161000279;                //灵巧转会心几率系数
double sg_fight_param_161000280;                //命中等级系数
double sg_fight_param_161000281;                //命中基础值
double sg_fight_param_161000282;                //闪避等级系数
double sg_fight_param_161000283;                //闪避基础值
double sg_fight_param_161000284;                //实际命中几率下限
double sg_fight_param_161000285;                //会心等级系数
double sg_fight_param_161000286;                //会心基础值
double sg_fight_param_161000287;                //五行抗性等级系数
double sg_fight_param_161000288;                //五行抗性基础值
double sg_fight_param_161000289;                //特殊属性基础值
double sg_fight_param_161000290;                //buff持续时间保底比例
double sg_fight_param_161000291;                //PVP等级系数
double sg_fight_param_161000292;                //PVP基础值
double sg_fight_param_161000293;                //PVP保底比例
double sg_fight_param_161000393;                //伤害公式加入战斗力计算
double sg_fight_param_161000394;                //伤害公式加入战斗力计算
double sg_fight_param_161000395;                //伤害公式加入战斗力计算
double sg_fight_param_161000396;                //伤害公式加入战斗力计算

uint32_t sg_server_level_reward_item_id;
uint32_t sg_server_level_reward_item_num;

uint32_t sg_doufachang_ai[2];
uint32_t sg_doufachang_raid_id;
uint32_t sg_doufachang_raid_win_reward[2];
uint32_t sg_doufachang_raid_lose_reward[2];

uint32_t DEFAULT_HORSE;

std::map<uint32_t, GangsSkillTable*> skill_config_map;

double sg_exp_turn_zhenqi_percent;

SceneCreateMonsterTable sg_zhenying_truck[2];

std::map<uint32_t, uint32_t> sg_strong_chapter_map;
std::map<uint64_t, GrowupTable*> sg_strong_chapter_reward;

//队员和队长的最大距离20米，太远就要传送过来
const uint64_t max_team_mem_distance = 20 * 20;

std::map<uint32_t, uint32_t> sg_item_trade_map;
double sg_on_shelf_fee_percent;

//帮会入侵相关数据
uint32_t sg_guild_ruqin_huodui_monster_id;
int sg_guild_ruqin_huodui_fanwei;
uint32_t sg_guild_ruqin_huodui_exp;
uint32_t sg_guild_ruqin_huodui_jiange;
uint32_t sg_guild_ruqin_huodui_chixutime;
uint64_t sg_guild_ruqin_renzu_bossid;
uint64_t sg_guild_ruqin_yaozu_bossid;

uint32_t sg_maogui_diaoxiang_stop_buff;
uint32_t sg_maogui_guiwang_wudi_buff;

char *sg_xunbao_boss_notice;

std::map<uint64_t, EquipAttribute*> sg_attr_seek_map;
std::map<uint64_t, std::vector<EquipAttribute*> > sg_attr_pool_map;

uint32_t sg_travel_round_amount;
uint32_t sg_travel_task_amount;

uint32_t sg_yaoqian_shu_sum_num;   //摇钱树可使用基础次数
uint32_t sg_yaoqian_shu_free_num;  //摇钱树的免费次数
uint32_t sg_yaoqian_shu_use_gold;  //摇钱树消耗元宝基础数量
uint32_t sg_yaoqian_shu_add_gold;  //摇钱树每次增加消耗元宝的数量
uint32_t sg_yaoqian_shu_max_gold;  //摇钱树消耗元宝的上限
uint32_t sg_yaoqian_shu_add_coin;  //摇钱树获取的银票基础值

int AWARD_QUESTION_ACTIVE_ID;
int COMMON_QUESTION_ACTIVE_ID;
const uint32_t sg_login_reward_function_id = 292010606;  //登录奖励FunctionUnlockTable表id
uint32_t sg_login_reward_chixu_day;  //登录奖励持续天数
uint32_t sg_new_battle_point;  //新手阵营战占旗加积分

uint32_t sg_strong_function_time;

