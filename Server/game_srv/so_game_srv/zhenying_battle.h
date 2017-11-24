#ifndef _ZHENYING_BATTLE_H__
#define _ZHENYING_BATTLE_H__

#include <map>
#include "global_param.h"
#include "conn_node_gamesrv.h"
#include "scene.h"

class player_struct;
//class scene_struct;
class raid_struct;
class monster_struct;
class Collect;
class ZhenyingBattle;
struct _OneScore;
struct BattlefieldTable;
struct _ZhenYingResult;

enum BATTLE_STATE
{
	JOIN_STATE,  //报名的时间
	BATTLE_READY_STATE, //准备
	AI_PLAYER_WAIT_STATE, //空气墙开启，AI玩家等待
	RUN_STATE,   //副本开始
	REST_STATE   //副本结束了
};

static const uint32_t MAX_ROOM_SIDE = 30;
static const uint32_t MAX_ROOM_FLAG = 3;

struct BATTLE_JOINER
{
	uint32_t room;
	uint32_t zhenying;
	uint32_t lv; 
	uint32_t kill;
	uint32_t dead;
	uint32_t help;
	uint32_t continKill;
	uint32_t continHelp;
	uint32_t point;
	bool in;
	bool ready;
};

struct REGION_INFO
{
	int state;
	int own;
	uint32_t id;
	uint32_t npc;
	uint64_t endTime;
	std::set<uint64_t> playerarr[2];
};

enum BATTLE_FLAG_STATE
{
	BATTLE_FLAG_NORMOR,
	BATTLE_FLAG_GATHERING,
	BATTLE_FLAG_COMMPLETE,
};
typedef std::vector<uint64_t> ROOM_MAN_T;
struct ROOM_INFO
{
	uint32_t step;
	uint32_t m_state;
	uint64_t m_nextTick;
	uint64_t uid;
	uint32_t totalPoint[2];
	uint32_t readyNum[2];
	uint64_t addTick[2];
	ROOM_MAN_T fighter[2];
	REGION_INFO flag[MAX_ROOM_FLAG];
};
typedef std::map<uint64_t, BATTLE_JOINER> JOIN_T;
typedef std::vector<uint64_t> STEP_JOIN_T;
typedef std::map<uint32_t, ROOM_INFO> ROOM_T;

//普通阵营战
class ZhenyingBattle //
{
public:
	static ZhenyingBattle *GetInstance();
	static const int MAX_STEP = 4;
	static const uint32_t s_border[MAX_STEP];
	static int battle_num;

	int CreatePrivateBattle(player_struct &player, raid_struct *raid);

	~ZhenyingBattle();
	int Join(player_struct &player, bool isNew = false);
	int CancelJoin(player_struct &player);
	int CheckCanJoin(player_struct &player, bool isNew);
	bool CheckCreateNewRoom(player_struct &player, int s, bool isNew, uint32_t &room);
	uint32_t CreateNewRoom(player_struct &player, int s, bool isNew);
	int SetReady(player_struct &player, bool ready);
	int GetReadyState(player_struct &player);
	//void Tick();
	void Tick(uint32_t room, raid_struct *raid);
	int IntoBattle(player_struct &player);   //进入阵营战
	uint32_t GetStep(player_struct &player);
	uint32_t CalcStep(player_struct &player);
	void SendMyScore(player_struct &player);
	void GetMySideScore(player_struct &player);
	void KillEnemy(unit_struct *killer, player_struct &dead);
	BATTLE_JOINER *GetJoins(uint64_t pid);
	void Settle(uint32_t room);
	void BroadMessageRoom(uint32_t room, uint16_t msg_id, void *msg_data, pack_func func, uint64_t except = 0);
	void OnRegionChanged(raid_struct *raid, player_struct *player, uint32_t old_region, uint32_t new_region);
	//void ClearRob();
	void ClearRob(uint32_t room);
	void StartRob();	
	void DestroyRoom(uint32_t room);

	bool PackOneScore(_OneScore *side, uint32_t rank, uint64_t playerid);
	void GetRelivePos(BattlefieldTable *table, int zhenying, int *x, int *z, double *direct);

	int get_room_num();	

	void GmStartBattle();

	void LeaveRegion(uint32_t room, player_struct *player, uint32_t old_region);

protected:
	ZhenyingBattle();

	void FlagOnTick(ROOM_T::iterator &itRoom, BattlefieldTable *table, uint64_t now);
	void AddFlagScore(ROOM_T::iterator &itRoom, BattlefieldTable *table, uint32_t i, uint64_t now);
	uint32_t CalcFlagTime(int state, uint64_t &end); //计算夺旗时间
	void DoRealSettle(ROOM_T::iterator &itRoom, BattlefieldTable *table, _ZhenYingResult *send, bool toF);
private:
	JOIN_T m_allJoin;   //所有报名的人
	ROOM_T m_room;
	uint32_t m_state;
	uint32_t m_tmpRoom[MAX_STEP];
	uint64_t m_nextTick;
};




#endif 
