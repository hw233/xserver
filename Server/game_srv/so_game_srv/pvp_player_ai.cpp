#include "pvp_player_ai.h"
#include "path_algorithm.h"
#include "player_manager.h"
#include "time_helper.h"
#include "msgid.h"
#include "camp_judge.h"
#include "buff_manager.h"
#include "pvp_match_manager.h"
#include "raid.h"
#include "player_ai.h"
#include "check_range.h"
#include "count_skill_damage.h"
#include "camp_judge.h"
#include "cached_hit_effect.h"
#include "buff.h"
#include "skill.h"
#include "count_pvp_skill_unit.h"

static player_struct **get_enemy_team_player(player_struct *player, struct ai_player_data **ai_player_data);
static void do_ai_player_relive(raid_struct *raid, player_struct *player, struct ai_player_data *ai_player_data, player_struct **enemy);

static void pvp_player_ai_tick(player_struct *player)
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

//	raid_struct *raid = (raid_struct *)player->scene;
//	assert(raid->m_config->DengeonRank == 5 || raid->m_config->DengeonRank == 6);

		// try attack others
	player_struct **enemy = NULL;
	struct ai_player_data *ai_player_data = NULL;
	enemy = get_enemy_team_player(player, &ai_player_data);
	assert(enemy);

	if (time_helper::get_cached_time() < ai_player_data->ontick_time)
		return;

	if (!player->is_alive())
	{
		return do_ai_player_relive((raid_struct *)player->scene, player, ai_player_data, enemy);
	}

	LOG_DEBUG("aitest: [%s]ai_state = %d", player->get_name(), ai_player_data->ai_state);


	if (ai_player_data->ai_state == AI_ATTACK_STATE)
		return do_ai_player_attack(player, ai_player_data, enemy, MAX_TEAM_MEM);

	uint32_t skill_id = choose_rand_skill(player);

	if (skill_id != 0)
	{
		for (int i = 0; i < MAX_TEAM_MEM; ++i)
		{
			if (!enemy[i])
				continue;
			if (!enemy[i]->is_alive())
				continue;

			if (ai_player_data->target_player_id)
			{
				if (ai_player_data->target_player_id != enemy[i]->get_uuid())
					continue;
					//检查追击距离
				struct position *his_pos = enemy[i]->get_pos();
				struct position ori_pos;
				ori_pos.pos_x = player->ai_patrol_config->patrol[player->data->patrol_index]->pos_x;
				ori_pos.pos_z = player->ai_patrol_config->patrol[player->data->patrol_index]->pos_z;
				if (!check_distance_in_range(his_pos, &ori_pos, player->data->chase_range))
				{
					LOG_DEBUG("aitest: [%s] chase distance = %.1f %.1f", player->get_name(),
						his_pos->pos_x - ori_pos.pos_x, his_pos->pos_z - ori_pos.pos_z);
					break;
				}
			}
			else
			{
					//检查主动攻击距离
				struct position *my_pos = player->get_pos();
				struct position *his_pos = enemy[i]->get_pos();
				if (!check_distance_in_range(my_pos, his_pos, player->data->active_attack_range))
				{
//					LOG_DEBUG("aitest: [%s] active attack distance = %.1f %.1f", player->get_name(),
//						his_pos->pos_x - my_pos->pos_x, his_pos->pos_z - my_pos->pos_z);
					continue;
				}
			}
			
			if (do_attack(player, ai_player_data, enemy[i], skill_id))
			{
				ai_player_data->target_player_id = enemy[i]->get_uuid();
//				LOG_DEBUG("aitest: [%s] try attack %s", player->get_name(), enemy[i]->get_name());
				return;
			}
		}
	}

	LOG_DEBUG("aitest: [%s]rand move, index = %u, skill_id = %u", player->get_name(), player->data->patrol_index, skill_id);
	ai_player_data->target_player_id = 0;

//	if (find_rand_position(player->scene, &player->data->move_path.pos[0],
//			3, &player->data->move_path.pos[1]) == 0)
	{
		find_next_position(player);
		pvp_player_ai_send_move(player);
		// player->data->move_path.start_time = time_helper::get_cached_time();
		// player->data->move_path.max_pos = 1;
		// player->data->move_path.cur_pos = 0;

		// MoveNotify notify;
		// move_notify__init(&notify);
		// notify.playerid = player->data->player_id;
		// notify.data = player->pack_unit_move_path(&notify.n_data);

		// player->broadcast_to_sight(MSG_ID_MOVE_NOTIFY, &notify, (pack_func)move_notify__pack, true);
		// unit_struct::reset_pos_pool();
	}
	// else
	// {
	// 	LOG_DEBUG("aitest[%s]: rand move failed", player->get_name());		
	// }
}


static player_struct **get_enemy_team_player(player_struct *player, struct ai_player_data **ai_player_data)
{
	raid_struct *raid = (raid_struct *)player->scene;
	assert(raid->m_config->DengeonRank == DUNGEON_TYPE_PVP_3 || raid->m_config->DengeonRank == DUNGEON_TYPE_PVP_5);

	player_struct **enemy = NULL;
	*ai_player_data = NULL;
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (player == raid->m_player[i])
		{
			enemy = raid->m_player2;
			*ai_player_data = &raid->PVP_DATA.ai_player_data[i];
			return enemy;
		}
		if (player == raid->m_player2[i])
		{
			enemy = raid->m_player;
			*ai_player_data = &raid->PVP_DATA.ai_player_data[i + MAX_TEAM_MEM];
			return enemy;
		}
	}
	return NULL;
}

static void do_ai_player_relive(raid_struct *raid, player_struct *player, struct ai_player_data *ai_player_data, player_struct **enemy)
{
	if (ai_player_data->relive_time == 0)
	{
		ai_player_data->relive_time = time_helper::get_cached_time() / 1000 + sg_pvp_raid_relive_cd;
		return;
	}
	if (time_helper::get_cached_time() / 1000 < ai_player_data->relive_time)
		return;

	ai_player_data->relive_time = 0;
	player->data->attrData[PLAYER_ATTR_HP] = player->data->attrData[PLAYER_ATTR_MAXHP];

	player->on_hp_changed(0);

	int32_t pos_x, pos_z, direct;
	pvp_raid_get_relive_pos(raid, &pos_x, &pos_z, &direct);
	reset_patrol_index(player);

	LOG_INFO("%s: ai player[%lu] relive to pos[%d][%d]", __FUNCTION__, player->get_uuid(), pos_x, pos_z);
	
	scene_struct *t_scene = raid;
	raid->delete_player_from_scene(player);
	player->set_pos(pos_x, pos_z);
	int camp_id = player->get_camp_id();
	t_scene->add_player_to_scene(player);
	player->set_camp_id(camp_id);

	if (player->m_team)
		player->m_team->OnMemberHpChange(*player);

		//复活的时候加上一个无敌buff
	buff_manager::create_default_buff(114400001, player, player, false);	
}

void install_pvp_ai_player_handle()
{
	player_manager::m_ai_player_handle[1] = pvp_player_ai_tick;
}

