#ifndef RAID_AI_COMMON_H
#define RAID_AI_COMMON_H

enum
{
	SCRIPT_EVENT_CREATE_MONSTER_NUM        = 1, //刷新配置表内指定怪物
	SCRIPT_EVENT_CREATE_MONSTER_ALL        = 2, //刷新配置表内所有指定怪物
	SCRIPT_EVENT_CREATE_COLLECT_NUM        = 3, //刷新配置表内指定采集物
	SCRIPT_EVENT_CREATE_COLLECT_ALL        = 4, //刷新配置表内所有指定采集物
	SCRIPT_EVENT_CREATE_NPC_NUM            = 5, //刷新配置表内指定NPC
	SCRIPT_EVENT_CREATE_NPC_ALL            = 6, //刷新配置表内所有指定NPC
	SCRIPT_EVENT_CREATE_TRANSFER_NUM       = 7, //刷新配置表内指定传送点
	SCRIPT_EVENT_CREATE_TRANSFER_ALL       = 8, //刷新配置表内所有指定传送点
	SCRIPT_EVENT_TIME_OUT                  = 9, //副本计时
	SCRIPT_EVENT_MONSTER_DEAD_NUM          = 10, //指定怪物死亡
	SCRIPT_EVENT_MONSTER_DEAD_ALL          = 11, //所有指定怪物死亡
	SCRIPT_EVENT_COLLECT_NUM               = 12, //采集指定采集物
	SCRIPT_EVENT_ALIVE_MONSTER_EQUEL       = 13, //等待副本内还存活指定怪物数量等于某值
	SCRIPT_EVENT_ALIVE_MONSTER_LESSER      = 14, //等待副本内还存活指定怪物数量小于某值
	SCRIPT_EVENT_ALIVE_MONSTER_GREATER     = 15, //等待副本内还存活指定怪物数量大于某值
	SCRIPT_EVENT_ALIVE_COLLECT_EQUEL       = 16, //等待副本内还存活指定采集物数量等于某值
	SCRIPT_EVENT_CREATE_AIR_WALL           = 17, //产生空气墙
	SCRIPT_EVENT_REMOVE_AIR_WALL           = 18, //删除空气墙
	SCRIPT_EVENT_REMOVE_NPC                = 19, //删除指定NPC
	SCRIPT_EVENT_PLAY_NPC_ACTION           = 20, //指定NPC播放动作
	SCRIPT_EVENT_PLAY_ANIMATION            = 21, //播放动画
	SCRIPT_EVENT_ACCEPT_TASK               = 22, //等待玩家身上拥有指定任务
	SCRIPT_EVENT_MONSTER_ARRIVE_POSITION   = 23, //等待怪物到达指定坐标范围
	SCRIPT_EVENT_PLAYER_ARRIVE_POSITION    = 24, //等待玩家到达指定坐标范围
	SCRIPT_EVENT_TRANSFER_PLAYER_POSITION  = 25, //改变玩家坐标
	SCRIPT_EVENT_PLAYER_TASK_FORK          = 26, //玩家选择任务分支
	SCRIPT_EVENT_FINISH_RAID               = 27, //通关副本或者结束副本
	SCRIPT_EVENT_COLLECT_CALLBACK          = 28, //采集回调的脚本名
	SCRIPT_EVENT_MONSTER_13_TYPE           = 29, //AI类型为13的怪物设置type
	SCRIPT_EVENT_ESCORT_RESULT             = 30, //等待护送结果
	SCRIPT_EVENT_START_ESCORT              = 31, //开始护送
	SCRIPT_EVENT_START_MONSTER_AI          = 32, //开启怪物AI
	SCRIPT_EVENT_STOP_MONSTER_AI           = 33, //停止怪物AI
	SCRIPT_EVENT_START_GONGCHENGCHUI       = 34, //触发攻城锤
	SCRIPT_EVENT_RAID_STOP                 = 35, //直接停止副本，把玩家传出去
	SCRIPT_EVENT_SET_PK_MODE                 = 36, //设置PK模式
	SCRIPT_EVENT_SET_CAMP                 = 37, //设置阵营
	SCRIPT_EVENT_SET_NOVICERAID_FLAG                 = 38, //设置新手副本完成标志
	SCRIPT_EVENT_ADD_BUFF                 = 39, //添加buff
	SCRIPT_EVENT_DEL_BUFF                 = 40, //删除buff
	SCRIPT_EVENT_WAIT_NPC_TALK                 = 41, //等待和NPC对话
	SCRIPT_EVENT_WAIT_MONST_HP            = 42, //等待指定怪物ID血量低于某百分比
	SCRIPT_EVENT_ADD_BUFF_TO_MONSTER	  = 43, //给指定怪物ID增加buff
	SCRIPT_EVENT_AUTOMATIC_NPC_TALK		  = 44, //自动npc对话通知
	SCRIPT_EVENT_PLAY_EFFECT			  = 45, //播放特效
	SCRIPT_EVENT_DELETE_MONSTER			  = 46, //删除副本内特定怪物
	SCRIPT_EVENT_FAIL_RAID               = 47, //副本失败
	SCRIPT_EVENT_SET_PLAYER_ATTR		  = 48, //修改玩家属性
	SCRIPT_EVENT_RECOVER_PLAYER_ATTR	  = 49, //恢复玩家属性
	SCRIPT_EVENT_SET_REGION_BUFF          = 50, //设置区域buff信息
	SCRIPT_EVENT_STOP_REGION_BUFF          = 51, //清除区域buff信息	
};

void do_script_raid_init_cond(raid_struct *raid, struct raid_script_data *script_data);
void script_ai_common_monster_dead(raid_struct *raid, monster_struct *monster, unit_struct *killer, struct raid_script_data *script_data);
void script_ai_common_collect(raid_struct *raid, player_struct *player, Collect *collect, struct raid_script_data *script_data);
void script_ai_common_tick(raid_struct *raid, struct raid_script_data *script_data);
void script_ai_common_player_ready(raid_struct *raid, player_struct *player, struct raid_script_data *script_data);
void script_ai_common_escort_stop(raid_struct *raid, player_struct *player, uint32_t escort_id, bool success, struct raid_script_data *script_data);
//void script_ai_common_npc_talk(raid_struct *raid, uint32_t npc_id, struct raid_script_data *script_data);
#endif /* RAID_AI_COMMON_H */
