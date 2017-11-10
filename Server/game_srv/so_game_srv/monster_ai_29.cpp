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
#include "raid.h"
#include "raid.pb-c.h"

static int monster_type29_ai_hp_deal_with(monster_struct *monster, scene_struct *scene)
{
	if(monster == NULL || monster->ai_data.circle_ai.ai_config == NULL)
		return -1;

	double cur_percent = monster->data->attrData[PLAYER_ATTR_HP]/monster->data->attrData[PLAYER_ATTR_MAXHP];
	cur_percent *= 100;

	if(cur_percent >  (double)monster->ai_data.circle_ai.ai_config->Separate)
		return -2;

	if(monster->ai_data.circle_ai.ai_config->n_SeparateMonster <= 0)
		return -3;
	
	scene_struct * o_scene = (scene == NULL ? monster->scene : scene);
	if(o_scene == NULL)
		return -4;
	if(o_scene->get_scene_type() != SCENE_TYPE_RAID)
		return -5;

	raid_struct *raid = (raid_struct *)o_scene;

	struct position *pos = monster->get_pos();
	for(size_t i = 0; i <  monster->ai_data.circle_ai.ai_config->SeparateNum; i++)
	{
		uint32_t xiabiao = rand() % monster->ai_data.circle_ai.ai_config->n_SeparateMonster;
		int32_t pos_x = 0;
		int32_t pos_z = 0;
		int j = 0;
		while(1)
		{
			if(j > 6)
			{
				pos_x = pos->pos_x;
				pos_z = pos->pos_z;
				break;
			}
			pos_x =  pos->pos_x + monster->ai_data.circle_ai.ai_config->SeparateRange - rand()%(2*monster->ai_data.circle_ai.ai_config->SeparateRange);
			pos_z =  pos->pos_z + monster->ai_data.circle_ai.ai_config->SeparateRange - rand()%(2*monster->ai_data.circle_ai.ai_config->SeparateRange);
			struct map_block *block_start = get_map_block(o_scene->map_config, pos_x, pos_z);
			if (block_start != NULL && block_start->can_walk == true)
				break;
			j++;
		}
		if(monster->ai_data.circle_ai.ai_config->Effects != NULL && monster->ai_data.circle_ai.ai_config->n_EffectsParameter >= 2)
		{
			double parama[5];
			parama[0] = pos_x;
			parama[1] = 10000;
			parama[2] = pos_z;
			parama[3] = monster->ai_data.circle_ai.ai_config->EffectsParameter[0];
			parama[4] = monster->ai_data.circle_ai.ai_config->EffectsParameter[1];
			RaidEventNotify nty;
			raid_event_notify__init(&nty);
			nty.type = 45;
			nty.param1 = parama;
			nty.n_param1 = 5;
			nty.param2 = &monster->ai_data.circle_ai.ai_config->Effects;
			nty.n_param2 = 1;
			raid->broadcast_to_raid(MSG_ID_RAID_EVENT_NOTIFY, &nty, (pack_func)raid_event_notify__pack);
		}
		monster_struct *t_monster = monster_manager::create_monster_at_pos(o_scene, monster->ai_data.circle_ai.ai_config->SeparateMonster[xiabiao], raid->data->monster_level, pos_x, pos_z, 0, NULL, 0);
		if(t_monster)
		{
		//	t_monster->set_ai_interface(29);
		}
	}
	if(scene == NULL)
	{
		if(monster->scene)
		{
			monster->scene->delete_monster_from_scene(monster, true);
		}
		monster_manager::delete_monster(monster);
	}

	return 0;
}

		

void type29_ai_tick(monster_struct *monster)
{
	if (monster->ai_type != 29)
		return;

	if (monster->ai_state == AI_DEAD_STATE)
		return;

	if (monster->is_in_lock_time())
		return;
	if (monster->buff_state & BUFF_STATE_STUN)
		return;

	if (!monster->is_alive())
		assert(0);

	assert(monster->scene);
	area_struct *area = monster->area;
	if (area)
	{
		if (!area->is_all_neighbour_have_player())
			return;
	}
	switch (monster->ai_state)
	{
		case AI_ATTACK_STATE:
			do_normal_attack(monster);
			break;
		case AI_PURSUE_STATE:  //追击
			do_normal_pursue(monster);
			break;
//		case AI_GO_BACK_STATE:
//			monster->data->ontick_time += random() % 2000;
//			do_goback(monster);
//			break;
		case AI_PATROL_STATE:  //巡逻
			monster->data->ontick_time += monster->count_rand_patrol_time();
			do_normal_patrol(monster);
			break;
	}
}

//巡逻中被攻击则进入追击
static void type29_ai_beattack(monster_struct *monster, unit_struct *player)
{
	if(monster_type29_ai_hp_deal_with(monster, NULL) == 0)
		return;
	if (monster->ai_state != AI_PATROL_STATE)
		return;
		
	monster->ai_state = AI_PURSUE_STATE;
	monster->data->target_pos.pos_x = 0;
	monster->data->target_pos.pos_z = 0;		
	if (monster->target && monster->target->is_avaliable())
		return;
	monster->target = player;

	uint64_t now = time_helper::get_cached_time();			
	monster->reset_timer(now + 500);
//	monster_manager::monster_ontick_reset_timer(monster);
}

//被击飞击退击倒
static void type29_ai_befly(monster_struct *monster, unit_struct *player)
{
		//打断技能
	if (monster->ai_state == AI_ATTACK_STATE)
	{
		monster->ai_state = AI_PURSUE_STATE;
		monster->data->target_pos.pos_x = 0;
		monster->data->target_pos.pos_z = 0;			
		monster->data->skill_id = 0;		
	}
}

static bool	type29_ai_player_leave_sight(monster_struct *monster, player_struct *player)
{
	if (monster->target && monster->target->get_uuid() == player->data->player_id)
		monster->target = NULL;
	return true;
}

static void ai_init_29(monster_struct *monster, unit_struct *)
{
	assert(monster);
	monster->ai_data.circle_ai.ai_config = get_config_by_id(monster->data->monster_id, &maogui_monster_config);
	assert(monster->ai_data.circle_ai.ai_config);
}

static void type29_ai_dead(monster_struct *monster, scene_struct *scene)
{
	monster_type29_ai_hp_deal_with(monster, scene);
}
struct ai_interface monster_ai_29_interface =
{
	type29_ai_tick,
	type29_ai_beattack,
	type29_ai_dead,
	type29_ai_befly,
	NULL,
	type29_ai_player_leave_sight,
	NULL,
	NULL,
	NULL,
	NULL,
	ai_init_29,
};




