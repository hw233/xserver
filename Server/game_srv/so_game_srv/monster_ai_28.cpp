#include <math.h>
#include <stdlib.h>
#include "game_event.h"
#include "monster_ai.h"
#include "path_algorithm.h"
#include "player_manager.h"
#include "time_helper.h"
#include "unit.h"
#include "buff.h"
#include "raid.h"
#include "check_range.h"
#include "cached_hit_effect.h"
#include "count_skill_damage.h"
#include "msgid.h"
#include "monster_manager.h"
#include "raid.pb-c.h"
/*static void do_ai_28_suiji_patrol(monster_struct *monster)
{
	if (!monster->config)
		return;
	
	struct position *cur_pos = monster->get_pos();
	
	if ((fabsf(cur_pos->pos_x - monster->ai_data.circle_ai.ret_pos.pos_x) > (int)(monster->ai_config->GuardRange)
			|| fabsf(cur_pos->pos_z - monster->ai_data.circle_ai.ret_pos.pos_z) > (int)(monster->ai_config->GuardRange)))
	{
		return monster->go_back();
	}
	
	if (monster->is_unit_in_move())
		return;

	if (monster->try_active_attack())
	{
		monster->ai_state = AI_PURSUE_STATE;
		struct position *target_pos = monster->target->get_pos();
		monster->data->target_pos.pos_x = target_pos->pos_x;
		monster->data->target_pos.pos_z = target_pos->pos_z;		
		return;
	}
	
	monster->reset_pos();
	if (find_rand_position(monster->scene, &monster->data->move_path.pos[0],
			monster->ai_config->GuardRange, &monster->data->move_path.pos[1]) == 0)
	{
		monster->send_patrol_move();		
	}
}*/

static int monster_ai_28_arrive_end_point(monster_struct * monster)
{
	if(monster == NULL || monster->data == NULL)
		return -1;
	if(monster->maogui_config->n_MovePointX <= 0)
		return -2;
	if(monster->ai_data.circle_ai.cur_pos_index < monster->maogui_config->n_MovePointX)
		return -3;
	if(monster->is_unit_in_move() || monster->ai_state != AI_PATROL_STATE)
		return -4;
	monster->ai_type = 29;
	monster->set_ai_interface(monster->ai_type);
	return 0;

}

static int get_monster_ai_28_wait_time(monster_struct * monster)
{
	assert(monster->maogui_config->n_MovePointX > 0 && monster->maogui_config->n_StopTime > 0);
	if (monster->ai_data.circle_ai.cur_pos_index < monster->maogui_config->n_MovePointX && monster->ai_data.circle_ai.cur_pos_index < monster->maogui_config->n_StopTime)
	{
		return monster->maogui_config->StopTime[monster->ai_data.circle_ai.cur_pos_index];
	}
	else
	{
		return monster->maogui_config->StopTime[0];
	}
}
	
static int monster_ai_hp_deal_with(monster_struct *monster, scene_struct *scene)
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
			raid->broadcast_to_raid(MSG_ID_RAID_EVENT_NOTIFY, &nty, (pack_func)raid_event_notify__pack, false);
		}
		uint32_t xiabiao = rand() % monster->ai_data.circle_ai.ai_config->n_SeparateMonster;
		monster_struct *t_monster = monster_manager::create_monster_at_pos(o_scene, monster->ai_data.circle_ai.ai_config->SeparateMonster[xiabiao], raid->lv, pos_x, pos_z, 0, NULL, 0);
		if(t_monster)
		{
			t_monster->set_ai_interface(29);
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
bool get_28_ai_next_pos(monster_struct * monster, float *pos_x, float *pos_z)
{
	assert(monster->maogui_config->n_MovePointX > 0);
	if (monster->ai_data.circle_ai.cur_pos_index < 0 || monster->ai_data.circle_ai.cur_pos_index >= monster->maogui_config->n_MovePointX)
	{
		return false;
	}
	*pos_x = monster->maogui_config->MovePointX[monster->ai_data.circle_ai.cur_pos_index];
	*pos_z = monster->maogui_config->MovePointZ[monster->ai_data.circle_ai.cur_pos_index];
	++monster->ai_data.circle_ai.cur_pos_index;
	
	
	return true;
}

void do_type28_ai_wait(monster_struct *monster)
{
	monster->ai_state = AI_PATROL_STATE;
	float pos_x, pos_z;
	if (!get_28_ai_next_pos(monster, &pos_x, &pos_z))
	{
		if (monster->scene->get_scene_type() == SCENE_TYPE_RAID)
		{
			raid_struct *raid = (raid_struct *)monster->scene;
			if (raid->ai != NULL && raid->ai->raid_on_escort_end_piont != NULL)
			{
				raid->ai->raid_on_escort_end_piont(raid, monster);
			}
		}
		return;
	}	
	monster->reset_pos();
	monster->data->move_path.pos[1].pos_x = pos_x;
	monster->data->move_path.pos[1].pos_z = pos_z;
	monster->send_patrol_move();
	return;
}
void do_monster_ai_28_patrol(monster_struct *monster)
{
	if (!monster->config)
		return;

	if (monster->try_active_attack())
	{
		monster->ai_state = AI_PURSUE_STATE;
		monster->data->target_pos.pos_x = 0;
		monster->data->target_pos.pos_z = 0;
		struct position *pos = monster->get_pos();
		monster->ai_data.circle_ai.ret_pos.pos_x = pos->pos_x;
		monster->ai_data.circle_ai.ret_pos.pos_z = pos->pos_z;			
		return;
	}

	if (monster->is_unit_in_move())
		return;

	monster->ai_state = AI_WAIT_STATE;
	monster->data->ontick_time = time_helper::get_cached_time() + get_monster_ai_28_wait_time(monster) * 10;
	return;
}

static void ai_tick_28(monster_struct *monster)
{
	if (monster->ai_type != 28)
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
	if(monster_ai_28_arrive_end_point(monster) == 0)
		return;
	switch (monster->ai_state)
	{
		case AI_WAIT_STATE:
			do_type28_ai_wait(monster);
			break;
		case AI_ATTACK_STATE:
			do_normal_attack(monster);
			break;
		case AI_PURSUE_STATE:
			do_normal_pursue(monster);
			break;
		case AI_PATROL_STATE:
			//if (monster->ai_data.circle_ai.cur_pos_index >= monster->maogui_config->n_MovePointX)
			//{
			//	do_ai_28_suiji_patrol(monster);
			//}
			//else 
			//{
				do_monster_ai_28_patrol(monster);
			//}
			break;
		case AI_PRE_GO_BACK_STATE:
			if( !monster->is_unit_in_move())
			{
				if(monster->ai_data.circle_ai.cur_pos_index > 0 )
				{
					--monster->ai_data.circle_ai.cur_pos_index;
					do_type28_ai_wait(monster);
				}

				monster->ai_state =	AI_PATROL_STATE;
			}
			break;
	}
}


void ai_init_28(monster_struct *monster, unit_struct *)
{
	assert(monster);
	monster->ai_data.circle_ai.ai_config = get_config_by_id(monster->data->monster_id, &maogui_monster_config);
	assert(monster->ai_data.circle_ai.ai_config);
}


static void type28_ai_beattack(monster_struct *monster, unit_struct *player)
{
	if(monster_ai_hp_deal_with(monster, NULL) == 0)
		return;
	if (monster->target && monster->target->is_avaliable())
		return;
	
	monster->ai_state = AI_PURSUE_STATE;
	monster->data->target_pos.pos_x = 0;
	monster->data->target_pos.pos_z = 0;		
	monster->target = player;
	struct position *pos = monster->get_pos();
	monster->ai_data.circle_ai.ret_pos.pos_x = pos->pos_x;
	monster->ai_data.circle_ai.ret_pos.pos_z = pos->pos_z;	
}
static void type28_ai_dead(monster_struct *monster, scene_struct *scene)
{
	monster_ai_hp_deal_with(monster,scene);
}

struct ai_interface monster_ai_28_interface =
{
	ai_tick_28,
	type28_ai_beattack,
	type28_ai_dead,
	circle_ai_befly,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	ai_init_28,
	.on_monster_ai_check_goback = circle_ai_check_goback,
	NULL,
	.on_monster_ai_do_goback = type22_ai_do_goback,	
};




