#include <math.h>
#include <stdlib.h>
#include "camp_judge.h"
#include "game_event.h"
#include "monster_ai.h"
#include "monster_manager.h"
#include "path_algorithm.h"
#include "player_manager.h"
#include "time_helper.h"
#include "unit.h"
#include "check_range.h"
#include "cached_hit_effect.h"
#include "count_skill_damage.h"
#include "msgid.h"
#include "buff.h"

extern void normal_ai_tick(monster_struct *monster);
extern void normal_ai_befly(monster_struct *monster, unit_struct *player);
extern bool normal_ai_player_leave_sight(monster_struct *monster, player_struct *player);
extern void normal_ai_beattack(monster_struct *monster, unit_struct *player);

static void ai_hp_changed_32(monster_struct *monster, int damage)
{
	if (!monster->is_alive())
		return;
	if (monster->ai_data.type32_ai.state != 0)
		return;
	
	double cur_percent = monster->data->attrData[PLAYER_ATTR_HP]/monster->data->attrData[PLAYER_ATTR_MAXHP];
	if (cur_percent >= 0.6)
		return;

	MonsterTalkNotify nty;
	monster_talk_notify__init(&nty);
	nty.talkid = 156000056;
	nty.uuid = monster->get_uuid();
	monster->broadcast_to_sight(MSG_ID_MONSTER_TALK_NOTIFY, &nty, (pack_func)monster_talk_notify__pack, false);
	
	uint64_t now = time_helper::get_cached_time();				
	monster->data->ontick_time = now + 5000;
	monster->set_attr(PLAYER_ATTR_PK_TYPE, 5);
	monster->broadcast_one_attr_changed(PLAYER_ATTR_PK_TYPE, 5, false, false);
	monster->ai_data.type32_ai.state = 1;
}

static void ai_tick_32(monster_struct *monster)
{
	if (monster->ai_data.type32_ai.state == 1)
	{
		monster->set_attr(PLAYER_ATTR_PK_TYPE, monster->config->PkType);
		monster->broadcast_one_attr_changed(PLAYER_ATTR_PK_TYPE, monster->config->PkType, false, false);		
		monster->ai_data.type32_ai.state = 2;
	}
	
	return normal_ai_tick(monster);
	
}

struct ai_interface monster_ai_32_interface =
{
	ai_tick_32,
	normal_ai_beattack,
	normal_ai_dead,
	normal_ai_befly,
	NULL,
	normal_ai_player_leave_sight,
	NULL,
	NULL,
	NULL,	
	.on_hp_changed = ai_hp_changed_32,
};




