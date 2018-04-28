#ifndef _CHENGJIE_H__
#define _CHENGJIE_H__

#include "conn_node_gamesrv.h"
#include "global_param.h"

#include <map>
#include <vector>
#include <set>

struct _PlayerDBInfo;

class player_struct;
class monster_struct;
struct _ChengjieTask;

extern CHENGJIE_CONTAIN ChengJieTaskManage_m_contain; //所有task的map
extern CHENGJIE_VECTOR ChengJieTaskManage_m_containVt;//所有task的vector
extern CHENGJIE_TARGET ChengJieTaskManage_m_target; ////目标 接任务者
extern CHENGJIE_ROLE_LEVEL ChengJieTaskManage_m_RoleLevel; //被悬赏者的等级和CD 

class ChengJieTaskManage
{
public:

	static void AddTask(STChengJie &task);
	static void AddTask(STChengJie &task, uint64_t accepter);
	static void AddTaskDb(STChengJie &task, EXTERN_DATA *extern_data);
	static void DelTask(uint32_t taskId, bool rm_target);
	static void DelTaskDb(uint32_t taskId);
	static void UpdateTaskDb(STChengJie &task);
	static void CommpleteTask(player_struct *player, player_struct *target, STChengJie &task);
	static STChengJie * FindTask(uint32_t taskId);
	static void ClientGetTaskList(player_struct *player, int type);
	static bool IsTarget(uint64_t pid);
	static uint64_t GetTaskAccept(uint64_t pid);
	static void AddRoleLevel(uint64_t playerid, uint32_t level, uint64_t cd);
	static void DelRoleLevel(uint64_t playerid);
	static uint32_t GetRoleLevel(uint64_t playerid);
	static uint64_t GetRoleCd(uint64_t playerid);
	static void LoadAllTask();
	static void SortList();
	static void TaskFail(player_struct *player, player_struct *target);
	static void SetRoleTarget(uint64_t target, uint64_t playerid);

	static void NotifyTargetLogin(player_struct *target);
	static int NotifyTargetPos(player_struct *player);
	static int CanUseFollowItem(player_struct *player);

	static void OnTimer();

//	static ChengJieTaskManage *GetInstance();
	static bool PackTask(STChengJie &task, _ChengjieTask &send);
	static void pack_yaoshi_to_dbinfo(player_struct *player, _PlayerDBInfo &db_info);
	static void unpack_yaoshi(player_struct *player, _PlayerDBInfo *db_info);
	//返回对悬赏目标的伤害加成百分比 以10000为基数
	static int ChengjieAddHurt(player_struct &player, player_struct &target);
	//返回受到悬赏目标的伤害减少的百分比 以10000为基数
	static int ChengjieRedeuceHurt(player_struct &player, player_struct &target);

	//返回对国御目标的伤害加成百分比 以10000为基数
	static int GuoyuAddHurt(player_struct &player, monster_struct &target);
	//返回受到国御目标的伤害减少的百分比 以10000为基数
	static int GuoyuRedeuceHurt(monster_struct &monster, player_struct &target);
	static void TaskExpire(STChengJie &task);

//protected:
//	ChengJieTaskManage() {}
private:

//	static ChengJieTaskManage *s_ins;
};

class ShangjinManage
{
public:
	static void RefreshTask(player_struct *player);
	static void RefreshTaskAndSend(player_struct *player, bool sys);
	static void CompleteTask(player_struct *player, uint32_t taskid);
};


#endif
