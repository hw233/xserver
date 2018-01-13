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

static float get_rand_distance()
{
	float ret = random() % 10000 - 5000;
	ret = ret / 10000.0;
	return ret;
}

static void move_to_player(monster_struct *monster, player_struct *player)
{
	if (monster->is_unit_in_move())
		return;

		// TODO: 间隔超过4米开始移动, 超过10米闪现
	struct position *owner_pos = player->get_pos();
	struct position *cur_pos = monster->get_pos();

	float x = owner_pos->pos_x - cur_pos->pos_x;
	float z = owner_pos->pos_z - cur_pos->pos_z;
	float d = x * x + z * z;
	
	if (d >= 24 * 24)
	{
			// 超过10米闪现
		struct position target_pos;
		player->calc_partner_pos(&target_pos, get_rand_distance());
		if (monster->scene)
			monster->scene->delete_monster_from_scene(monster, true);
		monster->set_pos(target_pos.pos_x, target_pos.pos_z);
		player->scene->add_monster_to_scene(monster, 0);
	}
	else if (d >= 3 * 3)
	{
			// TODO: 间隔超过4米开始移动
		monster->reset_pos();
		
		struct position target_pos;
		player->calc_partner_pos(&target_pos, get_rand_distance());

		struct position last;
		int ret = get_last_can_walk(monster->scene, &monster->data->move_path.pos[0], &target_pos, &last);
		if (ret == 0)
		{
			monster->data->move_path.pos[1].pos_x = last.pos_x;
			monster->data->move_path.pos[1].pos_z = last.pos_z;					
		}
		else
		{
			monster->data->move_path.pos[1].pos_x = target_pos.pos_x;
			monster->data->move_path.pos[1].pos_z = target_pos.pos_z;		
		}
		
		monster->data->move_path.start_time = time_helper::get_cached_time();
		monster->data->move_path.max_pos = 1;
		monster->broadcast_monster_move();
	}
}

static void monster_33_tick(monster_struct *monster)
{
	if (monster->ai_state == AI_DEAD_STATE)
		return do_normal_dead(monster);

	if (monster->is_in_lock_time())
		return;
	if (monster->buff_state & BUFF_STATE_STUN)
		return;

	if (!monster->is_alive())
		assert(0);

	assert(monster->scene);
	if (monster->scene->get_scene_type() != SCENE_TYPE_RAID)
		return;
	raid_struct *raid = (raid_struct *)(monster->scene);
	player_struct *player = raid->get_team_leader_player();
	if (!player)
		return;
	move_to_player(monster, player);
}

struct ai_interface monster_ai_33_interface =
{
	monster_33_tick,
	normal_ai_beattack,
	normal_ai_dead,
	normal_ai_befly,
	NULL,
	normal_ai_player_leave_sight,
	NULL,
	NULL,
	NULL,	
};




