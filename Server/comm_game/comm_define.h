#ifndef __COMM_DEFINE_H__
#define __COMM_DEFINE_H__

#define MAX_PLAYER_NAME_LEN     32 //人物名字最大长度
#define MAX_BAG_GRID_NUM        100 //背包格子数
#define MAX_MONEY_VALUE         999999999 //货币储存最大值
#define RAND_RATE_BASE          10000 //概率基数
#define MAX_HEAD_ICON_NUM       30 //最大头像数
#define MAX_TASK_TARGET_NUM     6 //最大任务目标数
#define MAX_TASK_NUM            300 //最大任务数
#define MAX_TASK_ACCEPTED_NUM   11 //最大可接任务数
#define MAX_EQUIP_ENCHANT_RAND_NUM   3 //最大装备附魔随机属性数
#define MAX_EQUIP_ENCHANT_NUM   3 //最大装备附魔属性条数
#define MAX_EQUIP_INLAY_NUM     6 //最大装备镶嵌宝石数
#define MAX_EQUIP_NUM           10 //最大装备数
#define MAX_COLOR_NUM           30 //最大颜色数
#define MAX_WEAPON_COLOR_NUM           100 //最大颜色数
#define MAX_FASHION_NUM           100 //最大时装数
#define MAX_HORSE_NUM           100 //最大坐骑数
#define DEFAULT_HORSE           180000001 //
//最大副本数目
#define MAX_RAID_NUM            1000 
#define SCENCE_DEPART           (20000)

//邮件
#define MAX_MAIL_NUM            100 //最大邮件数
#define MAX_MAIL_ATTACH_NUM     8 //单封邮件最大附件数

//商城
#define MAX_SHOP_LIST_NUM       20 //商城页签最大数
#define MAX_SHOP_GOODS_NUM      500 //商城商品最大数

//御气道
#define MAX_YUQIDAO_MAI_NUM     8 //御气道经脉最大数
#define MAX_YUQIDAO_BREAK_NUM   4 //御气道冲穴最大数
#define MAX_YUQIDAO_BREAK_ATTR_NUM 4 //御气道冲穴属性最大数

//八卦牌
#define MAX_BAGUAPAI_STYLE_NUM  2 //八卦牌风格最大数
#define MAX_BAGUAPAI_DRESS_NUM  8 //八卦牌穿戴件数
#define MAX_BAGUAPAI_MINOR_ATTR_NUM  3 //八卦牌副属性最大数

//活动
#define MAX_ACTIVE_REWARD_NUM   15 //活跃度奖励最大数
#define MAX_DAILY_ACTIVITY_NUM   100 //日常活动最大数
#define MAX_CHIVALRY_ACTIVITY_NUM   10 //侠义活动最大数

#define MAX_PERSONALITY_LOCATION_LEN 200
#define MAX_PERSONALITY_TAG_NUM      4
#define MAX_PERSONALITY_TEXT_INTRO_LEN 200
#define MAX_PERSONALITY_VOICE_INTRO_LEN 200
#define MAX_FRIEND_RECOMMEND_PLAYER    1000

#define MAX_PLAYER_BASE_ATTR_NUM 10
#define MAX_RAID_TEAM_NUM  4
#define MAX_GUILD_RAID_TEAM_NUM  2
#define MAX_GUILD_BATTLE_FINAL_GUILD_NUM  4 //帮战决赛只有四个帮会参加

#define MAX_PARTNER_TYPE   100
#define MAX_PARTNER_NUM   150
#define MAX_PARTNER_FORMATION_NUM   4
#define MAX_PARTNER_SKILL_NUM   6
#define MAX_PARTNER_BATTLE_NUM  2

#define MAX_ESCORT_NUM   2
#define MAX_ESCORT_MONSTER_NUM   (30 + 1)
#define MAX_ESCORT_MONSTER_WAVE   (3)


//玩家状态
enum PlayerStatus
{
	ONLINE,               //正常在线玩家
	OFFLINE_SAVING,       //下线玩家,保存数据未返回
};

//参数表ID定义
enum ParameterID
{
	PARAM_ID_BAG_UNLOCK = 162000001, //背包解锁
	PARAM_ID_RENAME = 161000002, //改名道具
	PARAM_ID_RELIVE_FREE_TIMES = 161000008, //每天免费原地复活次数
	PARAM_ID_RELIVE_FIRST_COST, //每天付费原地复活初始消耗
	PARAM_ID_RELIVE_GROW_COST, //每天付费原地复活消耗增长
	PARAM_ID_RELIVE_MAX_COST, //每天付费原地复活消耗上限
	PARAM_ID_RAID_KEEP_TIME = 161000007,  //副本保留时间
	PARAM_ID_GEM_STRIP_COIN = 161000019,  //宝石剥离消耗银币
	PARAM_ID_PLAYER_LEVEL_LIMIT = 161000020,  //角色等级上限
	PARAM_ID_BIRTH_MAP = 161000023,  //角色出生点
	PARAM_ID_YUQIDAO_BREAK_ITEM = 161000041,  //御气道冲脉道具
};

//道具类型
enum ItemType
{
	ITEM_TYPE_COIN = 1, //银两
	ITEM_TYPE_BIND_GOLD = 2, //绑定元宝
	ITEM_TYPE_GOLD = 3, //元宝
	ITEM_TYPE_EXP = 4, //经验
	ITEM_TYPE_ITEM = 5, //普通道具
	ITEM_TYPE_EQUIP = 6, //装备
	ITEM_TYPE_GUOYU_EXP = 7, //国御经验
	ITEM_TYPE_CHENGJIE_EXP = 8, //惩戒经验
	ITEM_TYPE_SHANGJIN_EXP = 9, //赏金经验
	ITEM_TYPE_GUOYU_COIN = 10, //国御货币
	ITEM_TYPE_CHENGJIE_COIN = 11, //惩戒货币
	ITEM_TYPE_SHANGJIN_COIN = 12, //赏金货币
	ITEM_TYPE_GUILD_TREASURE = 13, //帮会资金
	ITEM_TYPE_GUILD_DONATION = 14, //帮贡
	ITEM_TYPE_PARTNER_EXP = 15, //伙伴经验
};

enum JobDefine
{
	JOB_DEFINE_MONSTER = 0, //怪物
	JOB_DEFINE_DAO = 1, //大刀男
	JOB_DEFINE_GONG = 2, //弓箭女
	JOB_DEFINE_BI = 3, //毛笔男
	JOB_DEFINE_QIANG = 4, //长枪妖师
	JOB_DEFINE_FAZHANG = 5, //提杖妖师
//	JOB_DEFINE_COLLECT = 6, //采集点
};

enum SexDefine
{
	SEX_DEFINE_MALE = 1, //男性
	SEX_DEFINE_FEMALE = 2, //女性
};

enum MonsterTypeDefine
{
	MONSTER_TYPE_DEFINE_NORMAL = 1, //普通怪物
	MONSTER_TYPE_DEFINE_ELITE = 2, //精英怪物
	MONSTER_TYPE_DEFINE_BOSS = 3, //BOSS怪物
	MONSTER_TYPE_DEFINE_JIANTA = 4, //箭塔
	MONSTER_TYPE_DEFINE_TRAP = 5, //陷阱
};

enum AddItemDealWay
{
	ADD_ITEM_WILL_FAIL = 1, //添加道具会失败，要么全添加，要么全失败
	ADD_ITEM_AS_MUCH_AS_POSSIBLE = 2, //尽可能多的添加，背包不足时丢弃
	ADD_ITEM_SEND_MAIL_WHEN_BAG_FULL = 3, //背包空间不足时，发邮件
};

enum HeadIconStatus
{
	HIS_NEW_UNLOCK = 1,
};

enum HeadUnlockConditionType
{
	HUCT_LEVEL_UP = 1, //角色等级
	HUCT_ACTIVITY = 2, //参加活动获得
	HUCT_ITEM_USE = 3, //道具使用
	HUCT_RECHARGE = 4, //充值获得
	HUCT_VIP_LEVEL = 5, //VIP等级获得
};

enum ItemUseEffect
{
	IUE_UNLOCK_HEAD = 1, //解锁头像功能
	IUE_TRANSFER_SCENE = 2, //场景传送
	IUE_FIND_TARGET = 7, //追踪
	IUE_SUB_MURDER = 8, //降低杀戮值
	IUE_EXTEND_FRIEND_NUM = 10, //提升好友上限
	IUE_ADD_HP = 11, //加血药剂
	IUE_ADD_HP_POOL = 12, //血池
	IUE_ADD_ENERGY = 13, //精力值
	IUE_XUNBAO = 14, //寻宝
	IUE_ADD_HORSE = 18, //获得坐骑
};

enum TaskConditionType
{
	TCT_BASIC = 1, //基础条件
	TCT_ACCEPT = 2, //接受任务---接取条件
	TCT_SUBMIT = 3, //提交任务---没用到
	TCT_FINISH = 4, //完成任务---完成条件
	TCT_JOIN_CAMP = 5, //加入阵营---完成条件
	TCT_TEAM = 6, //组队
	TCT_JOIN_GUILD = 7, //加入帮派
	TCT_GUILD_DONATION = 8, //拥有帮贡
	TCT_KILL_MONSTER = 9, //击杀怪物
	TCT_SKILL_LEVEL_UP = 10, //技能升级
	TCT_CARRY_ITEM = 12, //携带物品
	TCT_USE_PROP = 13, //使用物品
	TCT_WEAR_BAGUA = 14, //佩戴八卦
	TCT_ACTIVITY = 17, //完成活动
	TCT_PARTNER_OUT_FIGHT = 18, //伙伴参战
	TCT_FINISH_RAID = 20, //通关副本
	TCT_KILL_PLAYER = 21, //杀人
	TCT_EXPLORE = 22, //探索
	TCT_ESCORT = 24, //护送
	TCT_TRACK = 25, //跟踪
	TCT_TALK = 28, //对话
	TCT_SNEAK = 29, //潜入---弃用
	TCT_QUESTION = 30, //答题
	TCT_ZHENYING_SCORE = 31, //阵营攻防活动积分
	TCT_TRUE = 32, //限制客户端自动接任务用
	TCT_TRUCK = 33, //押镖
	TCT_EQUIP_STAR = 34, //装备星级
	TCT_EQUIP_STAR_UP = 35, //装备升星
	TCT_EQUIP_ENCHANT = 36, //装备附魔
	TCT_EQUIP_INLAY = 37, //装备镶嵌
	TCT_GEM_COMPOSE = 38, //宝石合成
	TCT_WEAR_FASHION = 39, //佩戴时装
	TCT_FRIEND_NUM = 40, //好友数量
	TCT_JOIN_TEAM = 41, //加入队伍
};

enum TaskBasicCondition
{
	TBC_JOB = 1, //职业
	TBC_LEVEL = 2, //等级
	TBC_SEX = 3, //性别
	TBC_EXP = 4, //经验
	TBC_COIN = 5, //银两
	TBC_GOLD = 6, //元宝
};

enum TaskType
{
	TT_TRUNK = 1, //主线
	TT_BRANCH = 2, //支线
	TT_ACTIVITY = 3, //活动
	TT_SHANGJIN = 4, //赏金
	TT_QUESTION = 6, //答题
	TT_RAID = 9, //副本
	TT_CASH_TRUCK = 10, //押镖
};

enum TaskEventClass
{
	TEC_ACCEPTABLE = 1, //可接事件
	TEC_ACCEPT = 2, //接受事件
	TEC_ACHIEVE = 3, //完成事件
	TEC_SUBMIT = 4, //提交事件
	TEC_TALK = 5, //对话事件
};

enum TaskEventType
{
	TET_ADD_ITEM = 1, //添加物品
	TET_DEL_ITEM = 2, //删除物品
	TET_ADD_MONSTER = 3, //刷怪
	TET_DEL_MONSTER = 4, //删除怪物
	TET_ADD_BUFF = 5, //添加状态
	TET_DEL_BUFF = 6, //删除状态
	TET_ADD_NPC = 7, //添加NPC
	TET_DEL_NPC = 16, //删除NPC
	TET_NPC_FOLLOW = 19, //NPC跟随
	TET_ESCORT = 20, //护送
};

enum TaskRewardType
{
	TRT_ITEM = 1, //物品
	TRT_BUFF = 2, //BUFF
	TRT_GOLD = 4, //元宝
	TRT_BIND_GOLD = 5, //绑定元宝
};

enum SystemNoticeType
{
	SNT_ADD_ITEM_TEXT = 190200001,
	SNT_ADD_COIN_TEXT = 190200003,
	SNT_ADD_GOLD_TEXT = 190200004,
	SNT_ADD_EXP_TEXT = 190200006,

	SNT_ADD_EXP_ART = 190300001,
	SNT_ADD_COIN_ART = 190300002,
};

enum EquipmentType
{
	ET_WEAPON = 1, //武器
	ET_HELM = 2, //头盔
	ET_COAT = 3, //衣服
	ET_BELT = 4, //腰带
	ET_BRACER = 5, //护腕
	ET_GLOVE = 6, //手套
	ET_PANT = 7, //裤子
	ET_SHOE = 8, //鞋子
	ET_RING = 9, //戒指
	ET_NECKLACE = 10, //项链
};

enum FashionType
{
	FASHION_TYPE_WEAPON = 1,
	FASHION_TYPE_HAT = 2,
	FASHION_TYPE_CLOTHES = 3
};

enum MailIDDefine
{
	MAIL_ID_WANYAOKA = 270200001, //万妖卡
	MAIL_ID_BAG_FULL = 270200002, //背包不足
	MAIL_ID_GUILD_DISBAND = 270200003, //帮会解散
	MAIL_ID_GUILD_APPOINT_ELDER = 270200004, //帮会任命长老
	MAIL_ID_GUILD_APPOINT_VICE_MASTER = 270200005, //帮会任命副帮主
	MAIL_ID_GUILD_REVOKE_VICE_MASTER = 270200006, //帮会撤销副帮主
	MAIL_ID_GUILD_REVOKE_ELDER = 270200007, //帮会撤销长老
	MAIL_ID_GUILD_KICK = 270200008, //帮会踢人
	MAIL_ID_GUILD_REPLACE_MASTER = 270200009, //帮主转让
	MAIL_ID_GUILD_DAILY_REWAERD = 270200010, //帮会每日福利
};

enum PlayerOfflineCacheType
{
	CACHE_SUB_EXP = 1,
	CACHE_PVP_RAID_LOSE,
	CACHE_PVP_RAID_WIN,
};

enum ActivityMatter
{
	AM_RAID = 1, //通关副本
	AM_XUNBAO = 3, //寻宝
	AM_ANSWER = 4, //答题
	AM_TRUCK = 5, //押镖
	AM_YAOSHI = 6, //妖师客栈
};

#define MAX_GUILD_MEMBER_NUM 500 //每个帮会最大成员数

#endif
