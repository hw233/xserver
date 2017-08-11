#if 0
#ifndef BOSS_H
#define BOSS_H

#include "monster.h"

#define MAX_HATE_UNIT 20
//#define MAX_BOSS_SKILL 10

struct hate_unit_struct
{
	uint64_t uuid;
	double hate_value;
};

class boss_struct: public monster_struct
{
public:
	UNIT_TYPE get_unit_type();	
	void init_monster();
	virtual void on_beattack(unit_struct *player, uint32_t skill_id, int32_t damage);
	bool on_player_leave_sight(uint64_t player_id);
	bool on_monster_leave_sight(uint64_t uuid);
	void reset_timer(uint64_t time);
	void set_timer(uint64_t time);
	void on_go_back();
	void on_pursue();
//	virtual int add_skill_cd(uint32_t index, uint64_t now);
//	virtual bool is_skill_in_cd(uint32_t index, uint64_t now);
	virtual void clear_monster_timer();
private:
	void count_hate(unit_struct *player, uint32_t skill_id, int32_t damage);  //计算仇恨
	void update_target();
	bool on_unit_leave_sight(uint64_t unid);
	
	hate_unit_struct hate_unit[MAX_HATE_UNIT];
	uint64_t next_hate_reduce_time;
};
#endif /* BOSS_H */
#endif
