#ifndef _ZHENYING_BATTLE_H__
#define _ZHENYING_BATTLE_H__

#include <map>
#include "conn_node_gamesrv.h"
#include "scene.h"

class player_struct;
//class scene_struct;
class monster_struct;
class Collect;

enum BATTLE_STATE
{
	JOIN_STATE,
	RUN_STATE,
	REST_STATE
};

static const uint32_t MAX_ROOM_SIDE = 1;

struct BATTLE_JOINER
{
	uint32_t room;
	uint32_t zhenying;
	uint32_t lv; 
	uint32_t kill;
	uint32_t dead;
	uint32_t point;
};

struct ROOM_INFO
{
	uint32_t step;
	uint64_t uid;
	uint32_t totalPoint[2];
	std::vector<uint64_t> fighter[2];
};
typedef std::map<uint64_t, BATTLE_JOINER> JOIN_T;
typedef std::vector<uint64_t> STEP_JOIN_T;
typedef std::map<uint32_t, ROOM_INFO> ROOM_T;
class ZhenyingBattle //
{
public:
	static ZhenyingBattle *GetInstance();
	static const int MAX_STEP = 4;
	static const uint32_t s_border[MAX_STEP];

	void Join(player_struct &player);
	void Tick();
	int IntoBattle(player_struct &player);
	uint32_t GetStep(player_struct &player);
	void SendMyScore(player_struct &player);
	void KillEnemy(player_struct &player, player_struct &dead);
protected:
	ZhenyingBattle();

	void CreateRoom();
private:
	static ZhenyingBattle *ins;
	JOIN_T m_allJoin;
	ROOM_T m_room;
	uint32_t m_state;
	uint64_t m_nextTick;
};




#endif 
