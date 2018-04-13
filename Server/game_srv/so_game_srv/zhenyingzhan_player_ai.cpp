#include "path_algorithm.h"
#include "so_game_srv/zhenying_battle.h"
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

//static player_struct **get_enemy_team_player(player_struct *player);
static void do_ai_player_relive(raid_struct *raid, player_struct *player, struct ai_player_data *ai_player_data);

static void zhenyingzhan_player_ai_tick(player_struct *player)
{
	if (!player->ai_data)
		return;
//	return;
	if (player->buff_state & BUFF_STATE_STUN
		|| player->buff_state & BUFF_STATE_CHANRAO
		|| player->buff_state & BUFF_STATE_FENGYIN)
	{
//		LOG_DEBUG("aitest: [%s] lock", player->get_name());		
		return;
	}
	
	if (player->ai_data->stop_ai)
		return;
	if (player->is_unit_in_move())
		return;
	if (player->scene->get_scene_type() != SCENE_TYPE_RAID)
		return;

	player->reset_pos();

		// try attack others
#if 0	
	player_struct **enemy = NULL;
	enemy = get_enemy_team_player(player, &ai_player_data);
	assert(enemy);
#endif
	struct ai_player_data *ai_player_data = player->ai_data;
	
	if (time_helper::get_cached_time() < ai_player_data->ontick_time)
		return;

	if (!player->is_alive())
	{
		return do_ai_player_relive((raid_struct *)player->scene, player, ai_player_data);
	}

//	LOG_DEBUG("aitest: [%s]ai_state = %d", player->get_name(), ai_player_data->ai_state);


	if (ai_player_data->ai_state == AI_ATTACK_STATE)
		return do_ai_player_attack(player, ai_player_data, NULL, 0);

	uint32_t skill_id = choose_rand_skill(player);

	if (ai_player_data->target_player_id)
	{
		player_struct *enemy = player_manager::get_player_by_id(ai_player_data->target_player_id);
		if (!enemy || !enemy->is_alive())
		{
			ai_player_data->target_player_id = 0;			
		}
		else
		{
				//检查追击距离
			struct position *his_pos = enemy->get_pos();
			struct position ori_pos;
			if (ai_player_data->patrol_index > 0)
			{
				assert (ai_player_data->patrol_index <= ai_player_data->ai_patrol_config->n_patrol);
				ori_pos.pos_x = ai_player_data->ai_patrol_config->patrol[ai_player_data->patrol_index - 1]->pos_x;
				ori_pos.pos_z = ai_player_data->ai_patrol_config->patrol[ai_player_data->patrol_index - 1]->pos_z;
				if (!check_distance_in_range(his_pos, &ori_pos, ai_player_data->chase_range))
				{
//					LOG_DEBUG("aitest: [%s] chase distance = %.1f %.1f", player->get_name(),
//						his_pos->pos_x - ori_pos.pos_x, his_pos->pos_z - ori_pos.pos_z);
					ai_player_data->target_player_id = 0;			
				}
			}
		}
		
	}
	

	if (skill_id != 0)
	{
		for (int i = 0; i < player->data->cur_sight_player; ++i)
		{
			player_struct *enemy = player_manager::get_player_by_id(player->data->sight_player[i]);
			if (!enemy)
			{
				LOG_ERR("%s: player[%lu] can't find sight player[%lu]", __FUNCTION__, player->get_uuid(), player->data->sight_player[i]);
				continue;
			}
			if (get_unit_fight_type(player, enemy) != UNIT_FIGHT_TYPE_ENEMY)							
				continue;

			if (ai_player_data->target_player_id == 0)
			{
					//检查主动攻击距离
				struct position *my_pos = player->get_pos();
				struct position *his_pos = enemy->get_pos();
				if (!check_distance_in_range(my_pos, his_pos, ai_player_data->active_attack_range))
				{
//					LOG_DEBUG("aitest: [%s] active attack distance = %.1f %.1f", player->get_name(),
//						his_pos->pos_x - my_pos->pos_x, his_pos->pos_z - my_pos->pos_z);
					continue;
				}
			}
			
			if (do_attack(player, ai_player_data, enemy, skill_id))
			{
				ai_player_data->target_player_id = enemy->get_uuid();
//				LOG_DEBUG("aitest: [%s] try attack %s", player->get_name(), enemy[i]->get_name());
				return;
			}
		}
	}

//	LOG_DEBUG("aitest: [%s]rand move, index = %u, skill_id = %u", player->get_name(), player->data->patrol_index, skill_id);
	ai_player_data->target_player_id = 0;


	if (ai_player_data->patrol_index < ai_player_data->ai_patrol_config->n_patrol)
	{	
		player->data->move_path.pos[1].pos_x = ai_player_data->ai_patrol_config->patrol[ai_player_data->patrol_index]->pos_x;
		player->data->move_path.pos[1].pos_z = ai_player_data->ai_patrol_config->patrol[ai_player_data->patrol_index]->pos_z;	
		pvp_player_ai_send_move(player);
		ai_player_data->patrol_index++;

		// LOG_DEBUG("%s: jacktangmove player[%lu][%s] move from[%.1f][%.1f] to [%.1f][%.1f]", __FUNCTION__, player->get_uuid(), player->get_name(),
		// 	player->data->move_path.pos[0].pos_x, player->data->move_path.pos[0].pos_z,
		// 	player->data->move_path.pos[1].pos_x, player->data->move_path.pos[1].pos_z);
	}
	else
	{
//		ai_player_data->patrol_index = ai_player_data->ai_patrol_config->n_patrol;
		assert(ai_player_data->patrol_index == ai_player_data->ai_patrol_config->n_patrol);
	}

}


// static player_struct **get_enemy_team_player(player_struct *player)
// {
// 	raid_struct *raid = (raid_struct *)player->scene;
// //	assert(raid->m_config->DengeonRank == DUNGEON_TYPE_PVP_3 || raid->m_config->DengeonRank == DUNGEON_TYPE_PVP_5);

// 	player_struct **enemy = NULL;
// 	for (int i = 0; i < MAX_TEAM_MEM; ++i)
// 	{
// 		if (player == raid->m_player[i])
// 		{
// 			enemy = raid->m_player2;
// 			return enemy;
// 		}
// 		if (player == raid->m_player2[i])
// 		{
// 			enemy = raid->m_player;
// 			return enemy;
// 		}
// 	}
// 	return NULL;
// }

static void do_ai_player_relive(raid_struct *raid, player_struct *player, struct ai_player_data *ai_player_data)
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

		// 出生点
	int32_t pos_x, pos_z;
	double direct;
	BattlefieldTable *table = zhenying_fight_config.begin()->second;
	ZhenyingBattle::GetInstance()->GetRelivePos(table, player->get_attr(PLAYER_ATTR_ZHENYING), &pos_x, &pos_z, &direct);	
	ai_player_data->patrol_index = 0;

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

void install_zhenyingzhan_ai_player_handle()
{
	player_manager::m_ai_player_handle[3] = zhenyingzhan_player_ai_tick;
}

