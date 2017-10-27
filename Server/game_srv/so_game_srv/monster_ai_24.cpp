#include <math.h>
#include <stdlib.h>
#include "cash_truck_manager.h"
#include "camp_judge.h"
#include "game_event.h"
#include "monster_ai.h"
#include "monster_manager.h"
#include "path_algorithm.h"
#include "player_manager.h"
#include "time_helper.h"
#include "unit.h"
#include "raid.h"
#include "check_range.h"
#include "cached_hit_effect.h"
#include "count_skill_damage.h"
#include "msgid.h"
#include "buff.h"

void ai_tick_24(monster_struct *monster)
{
		//不复活
//	if (monster->ai_state == AI_DEAD_STATE)
//		return do_normal_dead(monster);

	monster->data->ontick_time = time_helper::get_cached_time() + monster->ai_data.type24_ai.ai_24_config->MonsterTime * 1000;

		//check MonsterMax
	raid_struct *raid = monster->get_raid();
	if (raid)
	{
		uint32_t num = raid->get_id_monster_num(monster->ai_data.type24_ai.ai_24_config->MonsterID);
		if (num >= monster->ai_data.type24_ai.ai_24_config->MonsterMax)
			return;
	}
	
	struct position *pos = monster->get_pos();
	monster_struct *t_monster = monster_manager::create_monster_at_pos(monster->scene, monster->ai_data.type24_ai.ai_24_config->MonsterID,
		1, pos->pos_x, pos->pos_z, 0, NULL, 0);
	if (t_monster)
	{
		t_monster->set_ai_interface(25);
//		t_monster->ai_data.type25_ai.pos_x = (int)(monster->ai_data.type24_ai.ai_24_config->MovePointXZ[0]);
//		t_monster->ai_data.type25_ai.pos_z = (int)(monster->ai_data.type24_ai.ai_24_config->MovePointXZ[1]);
//		t_monster->reset_pos();
		t_monster->data->move_path.pos[1].pos_x = (int)(monster->ai_data.type24_ai.ai_24_config->MovePointXZ[0]);
		t_monster->data->move_path.pos[1].pos_z = (int)(monster->ai_data.type24_ai.ai_24_config->MovePointXZ[1]);
		t_monster->send_patrol_move();
	}
}

void ai_init_24(monster_struct *monster, unit_struct *)
{
	assert(monster);
	monster->ai_data.type24_ai.ai_24_config = get_config_by_id(monster->data->monster_id, &GenerateMonster_config);
	assert(monster->ai_data.type24_ai.ai_24_config);
}

struct ai_interface monster_ai_24_interface =
{
	ai_tick_24,
	NULL,
	normal_ai_dead,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	.on_monster_ai_init = ai_init_24,
	NULL,
	NULL,
};




