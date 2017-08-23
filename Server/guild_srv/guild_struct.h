#ifndef _GUILD_STRUCT_H__
#define _GUILD_STRUCT_H__

#include <stdint.h>
#include <comm_define.h>
#include "player_redis_info.pb-c.h"
#include <vector>
#include "guild_answer.h"

#define MAX_GUILD_BUILDING_NUM 5 //每个帮会最大建筑数
#define MAX_GUILD_ANNOUNCEMENT_LEN     200 //帮会公告最大长度
#define MAX_GUILD_GOODS_NUM     30 //帮会商品最大数

enum GuildBuildingType
{
	Building_Hall = 1, //大厅
	Building_Vault = 2, //金库
	Building_Shop = 3, //商店
	Building_Library = 4, //藏经阁
};

enum PlayerExitGuildReason
{
	PEGR_QUIT = 1, //主动退出
	PEGR_KICK = 2, //被踢
	PEGR_DISBAND = 3, //帮会解散
};

enum GuildOfficePermissionType
{
	GOPT_APPOINT_VICE = 1, //任命或撤职副帮主
	GOPT_APPOINT_ELDER = 2, //任命或撤职长老
	GOPT_OPEN_ACTIVITY = 3, //开启活动
	GOPT_DEAL_JOIN = 4, //处理入帮申请
	GOPT_KICK_VICE = 5, //踢副帮主
	GOPT_KICK_ELDER = 6, //踢长老
	GOPT_KICK_MASS = 7, //踢帮众
	GOPT_RECRUIT_SETTING = 8, //招募设置
	GOPT_ANNOUNCEMENT_SETTING = 9, //公告设置
	GOPT_DEVELOP_SKILL = 10, //研发技能
	GOPT_RENAME = 11, //改名
};

//class AutoReleaseRedisPlayer
//{
//public:
// 	AutoReleaseRedisPlayer(PlayerRedisInfo *db) : pointer(db) {}
// 	~AutoReleaseRedisPlayer() { player_redis_info__free_unpacked(pointer, NULL); }
//private:
// 	PlayerRedisInfo *pointer;
//};
//class AutoReleaseBatchRedisPlayer
//{
//public:
// 	AutoReleaseBatchRedisPlayer() {}
// 	~AutoReleaseBatchRedisPlayer()
// 	{
// 		for (std::vector<PlayerRedisInfo*>::iterator iter = pointer_vec.begin(); iter != pointer_vec.end(); ++iter)
// 		{
// 			player_redis_info__free_unpacked(*iter, NULL);
// 		}
// 	}
// 
// 	void push_back(PlayerRedisInfo *player)
// 	{
// 		pointer_vec.push_back(player);
// 	}
//private:
// 	std::vector<PlayerRedisInfo *> pointer_vec;
//};


struct GuildInfo;

struct GuildBuilding
{
	uint32_t level;
};

struct GuildSkill
{
	uint32_t skill_id;
	uint32_t skill_lv;
};

struct GuildGoods
{
	uint32_t goods_id;
	uint32_t bought_num;
};

struct GuildShopReset
{
	uint32_t next_day_time;
	uint32_t next_week_time;
	uint32_t next_month_time;
};

struct GuildPlayer
{
	uint64_t player_id;
	GuildInfo *guild; //帮会指针
	uint32_t donation; //帮贡
	uint32_t all_history_donation; //所有帮会历史帮贡
	uint32_t cur_history_donation; //当前帮会历史帮贡
	uint32_t cur_week_donation; //本周帮贡
	uint32_t office; //职位
	uint32_t week_reset_time; //每周重置时间
	uint32_t join_time; //入帮时间
	uint32_t exit_time; //离帮时间
	GuildSkill skills[MAX_GUILD_SKILL_NUM]; //修炼技能列表
	GuildGoods goods[MAX_GUILD_GOODS_NUM]; //购买商品列表
	GuildShopReset shop_reset;
	uint32_t battle_score; //帮战积分
	uint32_t act_battle_score; //本场帮战积分
};

struct GuildInfo
{
	uint32_t guild_id;
	char name[MAX_GUILD_NAME_LEN + 1]; //帮名
	uint32_t icon; //图标
	uint64_t master_id; //帮主ID
	uint32_t approve_state; //审核开关，0：不需要审批，非0：需要审批
	uint32_t recruit_state; //招募开关，0：不开启招募，非0：开启招募
	char recruit_notice[MAX_GUILD_ANNOUNCEMENT_LEN + 1]; //招募宣言
	char announcement[MAX_GUILD_ANNOUNCEMENT_LEN + 1]; //公告
	uint32_t popularity; //人气
	uint32_t member_num; //帮会人数
	GuildPlayer *members[MAX_GUILD_MEMBER_NUM]; //帮会成员

	uint32_t rename_time; //改名时间

	uint32_t treasure; //帮会资金
	uint32_t build_board; //帮会建筑令
	uint32_t maintain_time; //维护时间
	GuildBuilding buildings[MAX_GUILD_BUILDING_NUM]; //帮会建筑
	uint32_t building_upgrade_id; //正在升级的建筑
	uint32_t building_upgrade_end; //建筑升级结束时间戳

	GuildSkill skills[MAX_GUILD_SKILL_NUM]; //研发技能列表
	GuildAnswer answer; // 帮会答题
	uint32_t battle_score; //帮战积分
};

#endif
