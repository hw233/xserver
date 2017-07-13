#ifndef _FRIEND_STRUCT_H__
#define _FRIEND_STRUCT_H__

#include <stdint.h>

#define MAX_FRIEND_GROUP_NAME_LEN   100 //好友分组名称最大长度
#define MAX_FRIEND_RECENT_NUM       200  //最近联系人最大数
#define MAX_FRIEND_CONTACT_NUM      200 //我的好友最大数
#define MAX_FRIEND_BLOCK_NUM        200 //黑名单最大数
#define MAX_FRIEND_ENEMY_NUM        200 //仇人最大数
#define MAX_FRIEND_APPLY_NUM        200  //申请最大数
#define MAX_FRIEND_GROUP_NUM        6   //分组最大数
#define MAX_FRIEND_RECOMMEND_NUM    4   //推荐玩家数
#define MAX_FRIEND_CHAT_NUM			100 //好友聊天数
#define MAX_FRIEND_SYSTEM_NUM	    100 //系统消息数
#define MAX_FRIEND_GIFT_NUM	        100 //接收礼物数

struct FriendGroup
{
	uint32_t group_id;
	uint32_t create_time;
	char     group_name[MAX_FRIEND_GROUP_NAME_LEN + 1];
};

struct FriendUnit
{
	uint64_t player_id;
	uint32_t closeness; //好感度
	uint32_t gift_num; //礼物数
	uint32_t group_id; //分组ID
};

struct FriendPlayer
{
	uint64_t player_id;
	uint8_t  apply_switch; //好友申请开关
	uint32_t contact_extend; //我的好友上限是否扩展
	uint64_t    recents[MAX_FRIEND_RECENT_NUM]; //最近联系人
	FriendUnit  contacts[MAX_FRIEND_CONTACT_NUM]; //我的好友
	FriendUnit  blocks[MAX_FRIEND_BLOCK_NUM]; //黑名单
	uint64_t    enemies[MAX_FRIEND_ENEMY_NUM]; //仇人
	uint64_t    applys[MAX_FRIEND_APPLY_NUM]; //申请列表
	FriendGroup groups[MAX_FRIEND_GROUP_NUM]; //分组
	uint32_t gift_accept; //接收礼物数
	uint32_t reset_time; //每日重置时间
};

#endif
