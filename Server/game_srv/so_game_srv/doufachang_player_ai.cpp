#include "doufachang_player_ai.h"
#include "path_algorithm.h"
#include "player_manager.h"
#include "time_helper.h"
#include "msgid.h"
#include "player_ai.h"
#include "camp_judge.h"
#include "buff_manager.h"
#include "pvp_match_manager.h"
#include "raid.h"
#include "check_range.h"
#include "count_skill_damage.h"
#include "camp_judge.h"
#include "cached_hit_effect.h"
#include "buff.h"
#include "skill.h"
#include "count_pvp_skill_unit.h"

static void doufachang_player_ai_tick(player_struct *player)
{
	if (player->buff_state & BUFF_STATE_STUN)
	{
		LOG_DEBUG("aitest: [%s] lock", player->get_name());		
		return;
	}
	
	if (player->data->stop_ai)
		return;
	if (player->is_unit_in_move())
		return;
	if (player->scene->get_scene_type() != SCENE_TYPE_RAID)
		return;

	player->reset_pos();

	raid_struct *raid = (raid_struct *)player->scene;

		// try attack others
	player_struct *enemy[1];
	enemy[0] = raid->m_player[0];
	if (!enemy[0] || !enemy[0]->is_avaliable())
		return;

	struct ai_player_data *ai_player_data = &raid->DOUFACHANG_DATA.ai_player_data;

	if (time_helper::get_cached_time() < ai_player_data->ontick_time)
		return;

	if (!player->is_alive())
	{
		LOG_ERR("%s: player dead, raid not finish, bug", __FUNCTION__);
		return;
	}

	LOG_DEBUG("aitest: [%s]ai_state = %d", player->get_name(), ai_player_data->ai_state);


	if (ai_player_data->ai_state == AI_ATTACK_STATE)
		return do_ai_player_attack(player, ai_player_data, enemy, 1);

	uint32_t skill_id;
	if (ai_player_data->skill_id != 0)
		skill_id = ai_player_data->skill_id;
	else skill_id = choose_rand_skill(player);

	if (skill_id != 0 && enemy[0] && enemy[0]->is_avaliable())
	{
		if (ai_player_data->target_player_id)
		{
			assert(ai_player_data->target_player_id == enemy[0]->get_uuid());
				
				//检查追击距离
			struct position *his_pos = enemy[0]->get_pos();
			struct position ori_pos;
			ori_pos.pos_x = player->ai_patrol_config->patrol[player->data->patrol_index]->pos_x;
			ori_pos.pos_z = player->ai_patrol_config->patrol[player->data->patrol_index]->pos_z;
			if (!check_distance_in_range(his_pos, &ori_pos, player->data->chase_range))
			{
				LOG_DEBUG("aitest: [%s] chase distance = %.1f %.1f", player->get_name(),
					his_pos->pos_x - ori_pos.pos_x, his_pos->pos_z - ori_pos.pos_z);
//				break;
			}
		}
		else
		{
				//检查主动攻击距离
			struct position *my_pos = player->get_pos();
			struct position *his_pos = enemy[0]->get_pos();
			if (!check_distance_in_range(my_pos, his_pos, player->data->active_attack_range))
			{
//				continue;
			}
		}
			
		if (do_attack(player, ai_player_data, enemy[0], skill_id))
		{
			ai_player_data->target_player_id = enemy[0]->get_uuid();
//				LOG_DEBUG("aitest: [%s] try attack %s", player->get_name(), enemy[i]->get_name());
			return;
		}
	}

	LOG_DEBUG("aitest: [%s]rand move, index = %u, skill_id = %u", player->get_name(), player->data->patrol_index, skill_id);
	ai_player_data->target_player_id = 0;

//	if (find_rand_position(player->scene, &player->data->move_path.pos[0],
//			3, &player->data->move_path.pos[1]) == 0)
	{
		find_next_position(player);
		pvp_player_ai_send_move(player);
	}
}

void install_doufachang_ai_player_handle()
{
	player_manager::m_ai_player_handle[2] = doufachang_player_ai_tick;
}

