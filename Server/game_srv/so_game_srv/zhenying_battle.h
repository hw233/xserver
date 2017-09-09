#ifndef _ZHENYING_BATTLE_H__
#define _ZHENYING_BATTLE_H__

#include <map>
#include "conn_node_gamesrv.h"
#include "scene.h"

class player_struct;
//class scene_struct;
class raid_struct;
class monster_struct;
class Collect;
class ZhenyingBattle;

enum BATTLE_STATE
{
	JOIN_STATE,
	RUN_STATE,
	REST_STATE
};

static const uint32_t MAX_ROOM_SIDE = 15;
static const uint32_t MAX_ROOM_FLAG = 3;

struct BATTLE_JOINER
{
	uint32_t room;
	uint32_t zhenying;
	uint32_t lv; 
	uint32_t kill;
	uint32_t dead;
	uint32_t point;
	bool in;
};

struct REGION_INFO
{
	int state;
	int own;
	uint32_t id;
	uint32_t npc;
	uint64_t startTime;
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
	uint64_t uid;
	uint32_t totalPoint[2];
	ROOM_MAN_T fighter[2];
	REGION_INFO flag[MAX_ROOM_FLAG];
};
typedef std::map<uint64_t, BATTLE_JOINER> JOIN_T;
typedef std::vector<uint64_t> STEP_JOIN_T;
typedef std::map<uint32_t, ROOM_INFO> ROOM_T;
typedef std::map<uint64_t, ZhenyingBattle *> PRIVATE_BATTLE_T;

//普通阵营战
class ZhenyingBattle //
{
public:
	static ZhenyingBattle *GetInstance();
	static const int MAX_STEP = 4;
	static const uint32_t s_border[MAX_STEP];

	static ZhenyingBattle *CreatePrivateBattle(player_struct &player, raid_struct *raid);
	static ZhenyingBattle *GetPrivateBattle(uint64_t raid);
	static void DestroyPrivateBattle(uint64_t raid);

	void Join(player_struct &player);
	void Tick();
	int IntoBattle(player_struct &player);   //进入阵营战
	uint32_t GetStep(player_struct &player);
	void SendMyScore(player_struct &player);
	void GetMySideScore(player_struct &player);
	void KillEnemy(player_struct &player, player_struct &dead);
	BATTLE_JOINER *GetJoins(uint64_t pid);
	void Settle(scene_struct *scence, uint32_t room);
	void BroadMessageRoom(uint32_t room, uint16_t msg_id, void *msg_data, pack_func func, uint64_t except = 0);
	void OnRegionChanged(raid_struct *raid, player_struct *player, uint32_t old_region, uint32_t new_region);
	void ClearRob();

	void GmStartBattle();

protected:
	ZhenyingBattle();

	void CreateRoom();
	void FlagOnTick();
private:
	static ZhenyingBattle *ins;
	static PRIVATE_BATTLE_T s_private;
	JOIN_T m_allJoin;   //所有报名的人
	ROOM_T m_room;
	uint32_t m_state;
	uint64_t m_nextTick;
};




#endif 
