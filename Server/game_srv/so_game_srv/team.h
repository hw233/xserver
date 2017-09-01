#pragma once

#include "conn_node_gamesrv.h"
#include "global_param.h"
#include <vector>
#include <set>


class player_struct;
struct  _TeamLimited;
struct _TeamMemInfo;
class scene_struct;
class Team;

extern TEAM_MAP team_manager_s_teamContain;
extern struct comm_pool team_manager_teamDataPool;
extern TEAM_CONTAIN team_manager_m_team;
extern ALL_ROLE_CONTAIN team_manager_m_allRole;
extern TARGET_ROLE_CONTAIN team_manager_m_targetRole;

static const int MAX_TEAM_MEM = 5;
static const int MAX_TEAM_APPLY = 20;
static const int MAX_TEAM_LIST = 20;
static const int MAX_CHANNEL = 10;

class MEM_INFO 
{
public:
	uint64_t id;
	time_t timeremove;
	bool follow;
	time_t appLeadCd;
	int last_pos_x;
	int last_pos_z;
	uint32_t level;

	MEM_INFO()
	{
		id = 0;
		timeremove = 0;
		follow = false;
		appLeadCd = 0;
		last_pos_x = 0;
		last_pos_z = 0;
	}

	bool operator==(const MEM_INFO &o) const
	{
		return id == o.id ? true : false;
	}
};

struct Team_data
{
	uint64_t m_applyList[MAX_TEAM_APPLY];
	uint64_t m_id;
	uint32_t m_lvLimit;
	int m_targrt;
	bool m_autoAccept;
	int m_lvType;
	int m_lvMin;
	int m_lvMax;
	int m_memSize;
	int m_applySize;
	int m_agreed;
	int m_guoyuType;
	bool m_guoyuItem;
	MEM_INFO m_mem[MAX_TEAM_MEM + 1];
	uint64_t m_raid_uuid;   //队伍正在进行的副本的唯一ID
	time_t speekCd[MAX_CHANNEL];

	Team_data()
	{
		memset(this, 0, sizeof(Team_data));
	}
};

typedef std::vector<MEM_INFO> TEAM_MEM_CONTAIN;
typedef std::set<uint64_t> TEAM_APPLY_CONTAIN;
class Team
{
public:
	Team();
	~Team();

	bool AddMember(player_struct &player);
		//把在线玩家移出队伍	
	void RemoveMember(player_struct &player, bool kick = false);
		//把不在线玩家移出队伍
	void RemoveMember(uint64_t playerid, bool kick = false);
	bool ChangeLeader(player_struct &player);
	player_struct * AutoChangeLeader();
	void OnLeaderChange(uint64_t id_old, uint64_t id_new);
	void SetLimited(_TeamLimited &limit, player_struct &player);
	static void PackMemberInfo(_TeamMemInfo &notice, player_struct &player);
	int PackAllMemberInfo(_TeamMemInfo *notice, _TeamMemInfo **noticeArr);
	int AddApply(player_struct &player);
	void RemoveApply(uint64_t pid);
	void RemoveAllApply() { m_data->m_applySize = 0; }
	void PackLimit(_TeamLimited &limit);
	void NotityXiayi();
	int PackLvJob(int *lv, int *job);
	bool IsAutoAccept() { return m_data->m_autoAccept; }
	bool IsFull() { return m_data->m_memSize == MAX_TEAM_MEM ? true : false; }
	bool CheckLevel(int lv);
	void SetFollow(player_struct &player, bool follow);
	bool IsFollow(player_struct &player);
	void SummonMem();
	void FollowLeadTrans(uint32_t scene_id, double pos_x, double pos_y, double pos_z, double direct);
	uint32_t GetAverageLevel() { return m_data->m_memSize == 0 ? 0 : m_sumLevel / m_data->m_memSize; }
	void SendXunbaoPoint(player_struct &player);

	void set_raid_id_wait_ready(uint32_t raid_id);
	void unset_raid_id_wait_ready();	

	void OnTimer();

	player_struct *GetLead();
	uint64_t GetLeadId();
	int GetMemberSize() { return m_data->m_memSize; }
	uint64_t GetId() { return m_data->m_id; }
	uint32_t GetTarget() { return m_data->m_targrt; }
//	TEAM_MEM_CONTAIN GetMemberOnline();
	int CkeckApplyCd(uint64_t playerid);

	void MemberOffLine(player_struct &player);
	bool MemberOnLine(player_struct &player);
	void BroadcastToTeam(uint16_t msg_id, void *msg_data, pack_func func, uint64_t except = 0);
	void BroadcastToTeamNotinSight(player_struct &player, uint16_t msg_id, void *msg_data, pack_func func, uint64_t except = 0);
	void SendApplyList(player_struct &player);
	void SendWholeTeamInfo(player_struct &player);
	int OnMemberHpChange(player_struct &player);
	void OnTeamidChange(player_struct &player);
	void broadcast_leader_pos(struct position *pos, uint32_t scene_id, uint64_t playerid);  //广播所有人的位置变化，不止是队长
	void Disband();

	static Team *CreateTeam(player_struct **player, int size);	
	static int CreateTeam(player_struct &player, int type = 2, int target = 0);
	static Team *  GetTeam(uint64_t teamid);
	static void DestroyTeam(Team *pTeam);
	static void DestroyTeamAndNotify(Team *pTeam);
	static void GetTargetTeamList(uint32_t target, player_struct &player);
	static void GetNearTeamList(player_struct &player);
	static void SendTeamList(TEAM_MAP &teamContain, player_struct &player, int target = 0);
	static void Timer();
	static uint32_t GetTeamNum();
	static uint32_t get_team_pool_max_num();

	Team_data *m_data;
	player_struct *m_team_player[MAX_TEAM_MEM];
	
public:
	static int InitTeamData(int num, unsigned long key);

private:
	int FindMember(uint64_t id);
	int FindApply(uint64_t id);
		

	//int leader_last_pos_x;
	//int leader_last_pos_z;
	uint32_t m_sumLevel;
//	bool m_appearBoss;
private:	
//	static uint32_t s_id;
};


class TeamMatch
{
public:
	static void AddRole(player_struct &player, int target);
	static uint64_t DelRole(uint64_t playerid);
	static void AddTeam(uint64_t id);
	static void DelTeam(uint64_t id);
	static Team * GetMatchTeam(uint32_t target, player_struct &player);
	static Team * GetRandomMatchTeam(player_struct &player);
	static Team * GetNearMatchTeam(player_struct &player);
	static int GetTargetRoleNum(int target);
	static void Timer();
	static bool IsInMacher(uint64_t id);
	static bool CheckFbNum(player_struct &player, int target);
};
