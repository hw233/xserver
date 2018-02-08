#include <math.h>
#include "so_game_srv/raid.h"
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

static bool check_monster_ai_valid(monster_struct *monster)
{
	assert(monster->ai_type == 34);
	if (!monster || !monster->ai_data.type34_ai.truck || !monster->ai_data.type34_ai.truck->data
		|| monster->ai_data.type34_ai.truck->get_uuid() != monster->ai_data.type34_ai.truck_uuid)
		return false;
	if (time_helper::get_cached_time() < monster->ai_data.type34_ai.start_time)
		return false;

	if (monster->ai_state == AI_DEAD_STATE)
		return false;

	if (monster->is_in_lock_time())
		return false;
	if (monster->buff_state & BUFF_STATE_STUN)
		return false;
	return true;
}

static void reset_speed(monster_struct *monster)
{
	monster->set_attr(PLAYER_ATTR_MOVE_SPEED, monster->ai_data.type34_ai.truck->get_speed());
}

void monster_34_patrol(monster_struct *monster)
{
	if (!monster->config)
		return;
//	if (monster->ai_type != AI_TYPE_NORMAL)
//		return;
	if (monster->is_unit_in_move())
		return;
	
	monster_struct *truck = monster->ai_data.type34_ai.truck;
	struct position *cur_pos = monster->get_pos();
	struct position *his_pos = truck->get_pos();

		// 走回马车附近
	if (!check_distance_in_range(cur_pos, his_pos, monster->ai_config->GuardRange))
	{
		if (get_circle_random_position_v2(monster->scene, cur_pos, his_pos, monster->ai_config->GuardRange / 4, &monster->data->move_path.pos[1]))		
		{
			monster->send_patrol_move();
		}
		else  //寻路不了就拉扯过去
		{
			if (monster->scene)
			{
				scene_struct *scene = monster->scene;
				monster->scene->delete_monster_from_scene(monster, true);
				monster->set_pos(his_pos->pos_x, his_pos->pos_z);
				scene->add_monster_to_scene(monster, 0);
			}
		}
		return;
	}

		// TODO: 选择马车的仇恨目标
	if (monster->try_active_attack())
	{
		monster->ai_state = AI_PURSUE_STATE;
		struct position *target_pos = monster->target->get_pos();
		monster->data->target_pos.pos_x = target_pos->pos_x;
		monster->data->target_pos.pos_z = target_pos->pos_z;		
//	uint64_t now = time_helper::get_cached_time();			
//	monster->reset_timer(now + 500);
		return;
	}
	
//	monster->data->move_path.pos[0].pos_x = monster->data->move_path.pos[monster->data->move_path.cur_pos].pos_x;
//	monster->data->move_path.pos[0].pos_z = monster->data->move_path.pos[monster->data->move_path.cur_pos].pos_z;
	monster->reset_pos();

		// TODO: 随机坐标
	if (find_rand_position(monster->scene, &monster->data->move_path.pos[0],
			monster->ai_config->GuardRange, &monster->data->move_path.pos[1]) == 0)
	{
		monster->send_patrol_move();		
	}
}

static unit_struct *monster_34_choose_target(monster_struct *monster)
{
	monster_struct *truck = monster->ai_data.type34_ai.truck;
	return truck->get_hate_target();
}

// static void move_to_player(monster_struct *monster, player_struct *player)
// {
// 	if (monster->is_unit_in_move())
// 		return;

// 		// TODO: 间隔超过4米开始移动, 超过10米闪现
// 	struct position *owner_pos = player->get_pos();
// 	struct position *cur_pos = monster->get_pos();

// 	float x = owner_pos->pos_x - cur_pos->pos_x;
// 	float z = owner_pos->pos_z - cur_pos->pos_z;
// 	float d = x * x + z * z;
	
// 	if (d >= 24 * 24)
// 	{
// 			// 超过10米闪现
// 		struct position target_pos;
// 		player->calc_partner_pos(&target_pos, get_rand_distance());
// 		if (monster->scene)
// 			monster->scene->delete_monster_from_scene(monster, true);
// 		monster->set_pos(target_pos.pos_x, target_pos.pos_z);
// 		player->scene->add_monster_to_scene(monster, 0);
// 	}
// 	else if (d >= 3 * 3)
// 	{
// 			// TODO: 间隔超过4米开始移动
// 		monster->reset_pos();
		
// 		struct position target_pos;
// 		player->calc_partner_pos(&target_pos, get_rand_distance());

// 		struct position last;
// 		int ret = get_last_can_walk(monster->scene, &monster->data->move_path.pos[0], &target_pos, &last);
// 		if (ret == 0)
// 		{
// 			monster->data->move_path.pos[1].pos_x = last.pos_x;
// 			monster->data->move_path.pos[1].pos_z = last.pos_z;					
// 		}
// 		else
// 		{
// 			monster->data->move_path.pos[1].pos_x = target_pos.pos_x;
// 			monster->data->move_path.pos[1].pos_z = target_pos.pos_z;		
// 		}
		
// 		monster->data->move_path.start_time = time_helper::get_cached_time();
// 		monster->data->move_path.max_pos = 1;
// 		monster->broadcast_monster_move();
// 	}
// }

static void monster_34_tick(monster_struct *monster)
{
	if (!check_monster_ai_valid(monster))
		return;

	if (!monster->is_alive())
		assert(0);

	reset_speed(monster);
	assert(monster->scene);
	switch (monster->ai_state)
	{
		case AI_ATTACK_STATE:
			do_normal_attack(monster);
			break;
		case AI_PURSUE_STATE:  //追击
			do_normal_pursue(monster);
			break;
		case AI_PATROL_STATE:  //巡逻
			monster->data->ontick_time += monster->count_rand_patrol_time();
			monster_34_patrol(monster);
			break;
	}
}

struct ai_interface monster_ai_34_interface =
{
	monster_34_tick,
	normal_ai_beattack,
	normal_ai_dead,
	normal_ai_befly,
	NULL,
	normal_ai_player_leave_sight,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	.monster_ai_choose_target = monster_34_choose_target,
};




