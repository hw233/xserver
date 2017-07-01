#ifndef _ZHENYING_BATTLE_H__
#define _ZHENYING_BATTLE_H__

#include "conn_node_gamesrv.h"
#include "scene.h"

class player_struct;
//class scene_struct;
class monster_struct;
class Collect;

class BattleField : public scene_struct
{
public:
	virtual void on_monster_dead(monster_struct *monster, unit_struct *killer);
	virtual void on_player_dead(player_struct *player, unit_struct *killer);
	virtual void on_collect(player_struct *player, Collect *collect);

	void update_task_process(uint32_t type, player_struct *player);
private:

};

class ZhenyingBattle //todo 把这个类删掉
{
public:
	static int GotoBattle(player_struct &player);
	static void UpdateOneTeamInfo(player_struct &player);
	static const int MAX_LINE_NUM = 8;
};




#endif 
