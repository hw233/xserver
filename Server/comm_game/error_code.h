#ifndef __ERROR_CODE_H__
#define __ERROR_CODE_H__

#define ERROR_ID_SUCCESS                 0 //成功，没错误
#define ERROR_ID_ACCOUNT_BANNED          1000 //账号被封
#define ERROR_ID_LOGIN_TOKEN             1001 //登录用的token错误
#define ERROR_ID_SRV_ACCOUNT_FULL        1002 //服务器满人
#define ERROR_ID_ACCOUNT_PLAYER_FULL     1003 //账号角色已满
#define ERROR_ID_DUPLICATE_NAME          1004 //角色重名
#define ERROR_ID_SAVE_DB                 1005 //保存数据库失败
#define ERROR_ID_NO_CONFIG               1006 //没有所需配置表
#define ERROR_ID_NAME_LEN                1007 //名字长度不对
#define ERROR_ID_MYSQL_QUERY             1008 //查询MySQL数据库失败
#define ERROR_ID_SERVER                  1009 //服务器内部错误
#define ERROR_ID_REQUEST_UNPACK          1010 //请求消息解包出错
#define ERROR_ID_PLAYER_DEAD             1011 //玩家已死亡
#define ERROR_ID_CONFIG                  1012 //配置表错误

#define ERROR_ID_GOLD_NOT_ENOUGH         1050 //元宝不足
#define ERROR_ID_COIN_NOT_ENOUGH         1051 //银两不足
#define ERROR_ID_LEVEL_NOT_ENOUGH        1052 //等级不足
#define ERROR_ID_ZHENQI_NOT_ENOUGH       1053 //真气不足
#define ERROR_ID_CHIVALRY_NOT_ENOUGH     1054 //侠义值不足
#define ERROR_ID_SHANGJIN_COIN_NOT_ENOUGH     1055 //赏金货币不足
#define ERROR_ID_CHENGJIE_COIN_NOT_ENOUGH     1056 //惩戒货币不足
#define ERROR_ID_GUOYU_COIN_NOT_ENOUGH     1057 //国御货币不足

#define ERROR_ID_BAG_UNLOCK_ALL          1100 //已解锁所有背包格子
#define ERROR_ID_BAG_GRID_NOT_ENOUGH     1101 //背包格子不足
#define ERROR_ID_BAG_POS                 1102 //背包格子索引超出范围
#define ERROR_ID_PROP_CAN_NOT_SELL       1103 //道具不能出售
#define ERROR_ID_PROP_NOT_ENOUGH         1104 //道具数量不足
#define ERROR_ID_PROP_CAN_NOT_USE        1105 //道具不能使用
#define ERROR_ID_PROP_IN_CD              1106 //道具CD中
#define ERROR_ID_PROP_EFFECT_TASK        1107 //道具不能再加任务

#define ERROR_ID_ICON_USING              1200 //头像已在使用
#define ERROR_ID_ICON_NEED_UNLOCK        1201 //头像尚未解锁

#define ERROR_ID_TASK_CONDITION          1300 //任务条件不满足
#define ERROR_ID_TASK_NOT_ACCEPT         1301 //任务还没接
#define ERROR_ID_TASK_NOT_ACHIEVE        1302 //任务还没完成
#define ERROR_ID_TASK_ID                 1303 //任务ID错误
#define ERROR_ID_PLAYER_IN_SIGHT_SPACE   1304 //玩家已在镜像场景内
#define ERROR_ID_TASK_CAN_NOT_ABANDON    1305 //任务不能放弃
#define ERROR_ID_TASK_CONDITION_ID       1306 //任务没有该条件
#define ERROR_ID_TASK_EVENT_ID           1307 //任务没有该事件
#define ERROR_ID_TASK_EVENT_CLASS        1308 //任务事件触发类型错误
#define ERROR_ID_TASK_EVENT_TYPE         1309 //任务事件类型错误
#define ERROR_ID_TASK_POSITION           1310 //任务校验位置错误
#define ERROR_ID_TASK_TEAM_LEADER        1311 //组队任务只能队长接
#define ERROR_ID_TASK_ACCEPTED           1312 //任务已接
#define ERROR_ID_TASK_CHAPTER_REWARD     1313 //任务章节奖励还不能领
#define ERROR_ID_TASK_FINISHED           1314 //任务已完成

#define ERROR_ID_NOT_SINGING             1400 //玩家没有在吟唱
#define ERROR_ID_SING_TIME               1401 //吟唱时间未结束

#define ERROR_ID_EQUIP_NOT_ACTIVE        1500 //装备尚未激活
#define ERROR_ID_EQUIP_STAR_MAX          1501 //装备已达到最大星
#define ERROR_ID_EQUIP_NOT_ALL_STAR_MAX  1502 //还有装备没达到最大星
#define ERROR_ID_EQUIP_STAIR_MAX         1503 //装备已达到最大阶
#define ERROR_ID_EQUIP_ENCHANT_INDEX     1504 //装备附魔索引错误
#define ERROR_ID_EQUIP_STAR_NOT_ENOUGH   1505 //装备星级不足
#define ERROR_ID_EQUIP_ENCHANT_NO_RETAIN     1506 //装备附魔还没保留
#define ERROR_ID_EQUIP_ENCHANT_NO_IN_RETAIN     1507 //装备附魔不在保留状态
#define ERROR_ID_EQUIP_ENCHANT_RETAIN_INDEX     1508 //装备附魔保留索引错误
#define ERROR_ID_EQUIP_HOLE_INDEX        1509 //装备宝石孔索引错误
#define ERROR_ID_EQUIP_HOLE_OPENED       1510 //装备宝石孔已经开启
#define ERROR_ID_EQUIP_HOLE_NOT_OPEN     1511 //装备宝石孔尚未开启
#define ERROR_ID_EQUIP_GEM_TYPE_NOT_FIT  1512 //该类型宝石不能镶嵌在这个孔
#define ERROR_ID_EQUIP_GEM_NO_BIND_ID    1513 //非绑定宝石没有对应的绑定ID
#define ERROR_ID_EQUIP_HOLE_NO_GEM       1514 //装备宝石孔没宝石
#define ERROR_ID_GEM_CAN_NOT_COMPOSE     1515 //宝石不能合成
#define ERROR_ID_GEM_COMPOSE_MATERIAL_NOT_ENOUGH     1516 //宝石合成材料不足
#define ERROR_ID_GEM_COMPOSE_MATERIAL_TYPE     1517 //宝石合成材料类型不对

#define ERROR_ID_MAIL_ATTACH_HAS_GOT     1600 //邮件附件已经领取
#define ERROR_ID_MAIL_ATTACH_GETTING     1601 //邮件附件正在领取
#define ERROR_ID_MAIL_NO_ATTACH          1602 //邮件没有附件
#define ERROR_ID_MAIL_ATTACH_NOT_GET     1603 //邮件还有附件未领取

#define ERROR_ID_SHOP_CLOSED             1700 //商城页签未开启
#define ERROR_ID_SHOP_GOODS_NOT_SELL     1701 //商城商品不出售
#define ERROR_ID_SHOP_GOODS_REMAIN       1702 //商城商品余量不足
#define ERROR_ID_SHOP_GOODS_BUY_NUM      1703 //商城商品购买数量不对

#define ERROR_ID_YUQIDAO_MAI_NOT_OPEN    1800 //御气道经脉未开启
#define ERROR_ID_YUQIDAO_MAI_FILL_LV_MAX 1801 //御气道经脉灌入真气已满
#define ERROR_ID_YUQIDAO_BREAK_NOT_OPEN  1802 //御气道冲脉未激活
#define ERROR_ID_YUQIDAO_BREAK_RETAIN_ATTR  1803 //御气道冲脉没有可保留的属性

#define ERROR_ID_BAGUAPAI_STYLE_ID       1900 //八卦牌套ID错误
#define ERROR_ID_BAGUAPAI_BAG_POS        1901 //八卦牌背包索引有误
#define ERROR_ID_BAGUAPAI_PART_ID        1902 //八卦牌部位错误
#define ERROR_ID_BAGUAPAI_NO_RETAIN_ATTR     1903 //八卦牌没有可保留的属性

#define ERROR_ID_ACTIVE_REWARD_HAS_GOT     2000 //该活跃度奖励已领取
#define ERROR_ID_ACTIVENESS_NOT_ENOUGH     2001 //活跃度不够，无法领奖励
#define ERROR_ID_ACTIVITY_NOT_OPEN		2002 //活动未开启

#define ERROR_ID_TRANSFER_OUT_STUCK_CDING     2100 //脱离卡死正在CD中
#define ERROR_ID_TRANSFER_OUT_STUCK_FAIL      2101 //脱离卡死传送失败

#define ERROR_ID_GUILD_PLAYER_NOT_JOIN        2200 //玩家尚未加入帮会
#define ERROR_ID_GUILD_PLAYER_ALREADY_JOIN    2201 //玩家已经加入帮会
#define ERROR_ID_GUILD_NAME_EXIST             2203 //帮会名字已存在
#define ERROR_ID_GUILD_ID                     2204 //帮会不存在
#define ERROR_ID_GUILD_PLAYER_APPLIED_JOIN    2207 //玩家已申请过该帮会
#define ERROR_ID_GUILD_PLAYER_NO_PERMISSION   2208 //玩家没权限
#define ERROR_ID_GUILD_PLAYER_NO_IN_LIST      2209 //玩家不在列表里
#define ERROR_ID_GUILD_CAN_NOT_DO_IT_TO_SELF  2210 //不能对自己进行该操作
#define ERROR_ID_GUILD_HAS_NOT_PLAYER         2211 //帮会里没有该玩家
#define ERROR_ID_GUILD_PLAYER_HAS_OFFICE      2212 //玩家已经有官职
#define ERROR_ID_GUILD_OFFICE_MAX             2213 //该官职已达最大人数
#define ERROR_ID_GUILD_PLAYER_ALREADY_IS      2214 //该玩家已经是这个职位
#define ERROR_ID_GUILD_CAMP                   2215 //阵营不符
#define ERROR_ID_GUILD_NO_REWARD_TODAY        2216 //今日没有奖励
#define ERROR_ID_GUILD_REWARD_HAS_GOT         2217 //今日奖励已领取
#define ERROR_ID_GUILD_MASTER_CANNOT_EXIT     2218 //帮主不能退帮
#define ERROR_ID_GUILD_BUILDING_LEVEL_MAX     2220 //建筑已满级
#define ERROR_ID_GUILD_HALL_LEVEL_NOT_ENOUGH  2221 //大厅等级不够
#define ERROR_ID_GUILD_TREASURE_NOT_ENOUGH    2222 //帮会资金不足
#define ERROR_ID_GUILD_BOARD_NOT_ENOUGH       2223 //帮会建设令不足
#define ERROR_ID_GUILD_LIBRARY_LEVEL_NOT_ENOUGH  2224 //藏经阁等级不够
#define ERROR_ID_GUILD_SKILL_LEVEL_MAX        2225 //技能已满级
#define ERROR_ID_GUILD_SKILL_DEVELOP_LEVEL    2226 //技能研发等级不足
#define ERROR_ID_GUILD_PLAYER_DONATION        2227 //玩家帮贡不足
#define ERROR_ID_GUILD_HISTORY_DONATION       2228 //玩家历史帮贡不足
#define ERROR_ID_GUILD_CALL_CD                2229 //帮战征召CD
#define ERROR_ID_GUILD_TASK_ON_GOING          2231 //当前正在进行帮会建设任务

#define ERROR_ID_FRIEND_IN_CONTACT            2251 //玩家已是你的好友
#define ERROR_ID_FRIEND_IN_BLOCK              2252 //你已把对方拉黑
#define ERROR_ID_FRIEND_LIST_FULL             2253 //列表人数已满
#define ERROR_ID_FRIEND_ID                    2254 //该玩家不存在
#define ERROR_ID_FRIEND_SELF                  2255 //不能对自己操作
#define ERROR_ID_FRIEND_NOT_IN_LIST           2256 //玩家不在列表
#define ERROR_ID_FRIEND_IN_APPLY              2257 //玩家已在申请列表
#define ERROR_ID_FRIEND_IN_ENEMY              2258 //玩家已在仇人列表
#define ERROR_ID_FRIEND_GROUP_NAME_LEN        2259 //好友分组名字长度
#define ERROR_ID_FRIEND_GROUP_ID              2262 //分组不存在
#define ERROR_ID_FRIEND_CHAT_RECEIVER         2263 //私聊对象不存在
#define ERROR_ID_FRIEND_SEARCH_ID             2264 //查找的玩家不存在
#define ERROR_ID_FRIEND_EXTENDED              2265 //好友上限已经扩展，无法再次扩展
#define ERROR_ID_FRIEND_GIFT_ID               2268 //礼物ID错误

#define ERROR_ID_RANK_TYPE                    2300 //排行榜类型错误
#define ERROR_ID_RANK_REDIS                   2301 //排行榜查询redis失败

#define ERROR_ID_PARTNER_UUID                 2400 //伙伴唯一ID错误
#define ERROR_ID_PARTNER_POS                  2401 //伙伴位置索引错误
#define ERROR_ID_PARTNER_SKILL_INDEX          2402 //伙伴技能索引错误
#define ERROR_ID_PARTNER_CANT_LEARN_SAME_SKILL    2403 //伙伴不能学习同样的技能
#define ERROR_ID_PARTNER_SKILL_LEVEL_MAX      2404 //伙伴技能已达最大级
#define ERROR_ID_PARTNER_IN_FORMATION         2405 //伙伴在阵上
#define ERROR_ID_PARTNER_NOT_IN_DICTIONARY    2406 //图鉴里没有该伙伴
#define ERROR_ID_PARTNER_NUM_MAX              2407 //玩家拥有伙伴数量最大
#define ERROR_ID_PARTNER_RECRUIT_TYPE         2408 //伙伴招募类型错误
#define ERROR_ID_PARTNER_LEVEL_MAX            2410 //伙伴等级已达最大
#define ERROR_ID_PARTNER_BOND_ACTIVED         2411 //伙伴羁绊已激活
#define ERROR_ID_PARTNER_BOND_ACTIVE_CONDITION    2412 //伙伴羁绊激活条件不满足
#define ERROR_ID_PARTNER_BOND_REWARD_GET      2413 //伙伴羁绊奖励已领取
#define ERROR_ID_PARTNER_BOND_REWARD_CONDITION    2414 //伙伴羁绊奖励条件不满足
#define ERROR_ID_PARTNER_COMPOSE_STONE_TYPE   2415 //伙伴神曜合成材料类型不一致
#define ERROR_ID_PARTNER_COMPOSE_STONE_SCORE   2416 //伙伴神曜合成材料积分不足

#define ERROR_ID_ACHIEVEMENT_ID               2500 //成就ID错误
#define ERROR_ID_ACHIEVEMENT_CANT_REWARD      2501 //成就奖励不可领

#define ERROR_ID_TITLE_ID                     2530 //称号ID错误
#define ERROR_ID_TITLE_NOT_ACTIVE             2531 //称号未激活

#define ERROR_ID_NOT_IN_FISHING_REGION		  2600 //不在钓鱼区域

#define ERROR_ID_STRONG_CHAPTER_ID			2700 //变强章节id错误
#define ERROR_ID_STRONG_CHAPTER_STATE		2701 //变强章节奖励状态错误
#define ERROR_ID_STRONG_GOAL_ID				2702 //变强目标id错误
#define ERROR_ID_STRONG_GOAL_STATE			2702 //变强目标奖励状态错误

#define ERROR_ID_TRADE_ITEM_CANT_TRADE      2800 //该道具不能交易
#define ERROR_ID_TRADE_PRICE                2801 //交易价格错误
#define ERROR_ID_TRADE_SHELF_INDEX          2803 //货架索引范围出错
#define ERROR_ID_TRADE_ITEM_OFF_SHELF       2804 //货品已下架
#define ERROR_ID_TRADE_ITEM_ADJUSTING       2805 //货品正在调整
#define ERROR_ID_TRADE_ITEM_NUM             2806 //货品数量只能下调
#define ERROR_ID_TRADE_SHELF_NUM_MAX        2807 //货架格子已达最大
#define ERROR_ID_TRADE_ITEM_LEFT_NUM        2808 //货品剩余数量不足
#define ERROR_ID_TRADE_OPERATE_NUM          2809 //操作数量不能为0
#define ERROR_ID_TRADE_NO_EARNING           2810 //当前没有收益
#define ERROR_ID_AUCTION_SOLD               2811 //拍卖品已出售
#define ERROR_ID_AUCTION_PRICE              2812 //拍卖品竞价失败


#endif
