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
#include "raid.pb-c.h"

static void ai_tick_27(monster_struct *monster)
{
	if(monster->buff_state & BUFF_STATE_STUN)
		return; 

	monster->data->ontick_time = time_helper::get_cached_time() + monster->ai_data.type27_ai.ai_27_config->MonsterTime * 1000;

		//check MonsterMax
	raid_struct *raid = monster->get_raid();
	if (raid == NULL )
		return;
	uint32_t num = raid->m_monster.size();
	if(num >= monster->ai_data.type27_ai.ai_27_config->MonsterMax)
	//if(num >=2)
		return;
	
	struct position *pos = monster->get_pos();
	if(monster->ai_data.type27_ai.ai_27_config->Effects != NULL && monster->ai_data.type27_ai.ai_27_config->n_EffectsParameter >= 2)
	{
		double parama[5];
		parama[0] = pos->pos_x;
		parama[1] = 10000;
		parama[2] = pos->pos_z;
		parama[3] = monster->ai_data.type27_ai.ai_27_config->EffectsParameter[0];
		parama[4] = monster->ai_data.type27_ai.ai_27_config->EffectsParameter[1];
		RaidEventNotify nty;
		raid_event_notify__init(&nty);
		nty.type = 45;
		nty.param1 = parama;
		nty.n_param1 = 5;
		nty.param2 = &monster->ai_data.type27_ai.ai_27_config->Effects;
		nty.n_param2 = 1;
		raid->broadcast_to_raid(MSG_ID_RAID_EVENT_NOTIFY, &nty, (pack_func)raid_event_notify__pack);
	}

	monster_struct *t_monster = monster_manager::create_monster_at_pos(monster->scene, monster->ai_data.type27_ai.ai_27_config->MonsterID, raid->data->monster_level, pos->pos_x, pos->pos_z, 0, NULL, 0);
	assert(monster->ai_data.type27_ai.ai_27_config->n_MovePointX > 0 && monster->ai_data.type27_ai.ai_27_config->n_MovePointZ > 0 && monster->ai_data.type27_ai.ai_27_config->n_MovePointX == monster->ai_data.type27_ai.ai_27_config->n_MovePointZ);
	
	if(t_monster)
	{
		t_monster->set_ai_interface(28);
		t_monster->ai_type = 28;
		t_monster->maogui_config = monster->ai_data.type27_ai.ai_27_config;
	}
}

static void ai_init_27(monster_struct *monster, unit_struct *)
{
	assert(monster);
	monster->ai_data.type27_ai.ai_27_config = get_config_by_id(monster->data->monster_id, &maogui_diaoxiang_config);
	assert(monster->ai_data.type27_ai.ai_27_config);
}


struct ai_interface monster_ai_27_interface =
{
	ai_tick_27,
	NULL,
	normal_ai_dead,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	.on_monster_ai_init = ai_init_27,
	NULL,
	NULL,
};




