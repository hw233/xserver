#include <math.h>
#include <stdlib.h>
#include "game_event.h"
#include "raid_ai.h"
#include "raid_manager.h"
#include "map_config.h"
#include "time_helper.h"
#include "player_manager.h"
#include "check_range.h"
#include "unit.h"
#include "uuid.h"
#include "msgid.h"
#include "pvp_raid.pb-c.h"
#include "raid.pb-c.h"
#include "scene_transfer.pb-c.h"
#include "app_data_statis.h"
#include "buff_manager.h"
#include "monster_manager.h"
#include "pvp_match_manager.h"
#include "../proto/relive.pb-c.h"
#include "server_level.h"

static void pvp_raid_refresh_monster(raid_struct *raid);
static void get_team_kill_num(raid_struct *raid, int *kill1, int *kill2);
static void	finished_raid(raid_struct *raid, int win_team);
static void	pvp_raid_player_killmyself(raid_struct *raid, int target_index, player_struct *target);
static void pvp_raid_player_kill(raid_struct *raid, int target_index, player_struct *player, player_struct *target);
static void pvp_raid_ai_init(raid_struct *raid, player_struct *player)
{
}

static bool check_player_in_buff_range(player_struct *player, int pos_x, int pos_z)
{
	if (!player)
		return false;

	if (!player->is_alive())
		return false;

	struct position *pos1 = player->get_pos();
	struct position pos2;
	pos2.pos_x = pos_x;
	pos2.pos_z = pos_z;
	return check_distance_in_range(pos1, &pos2, 1);
}

static void send_get_buff(raid_struct *raid, uint32_t type, uint64_t player_id)
{
	PvpRaidBuffGetNotify nty;
	pvp_raid_buff_get_notify__init(&nty);
	nty.type = type;
	nty.player_id = player_id;
	raid->broadcast_to_raid(MSG_ID_PVP_RAID_BUFF_GET_NOTIFY, &nty, (pack_func)pvp_raid_buff_get_notify__pack);
}

static player_struct *get_last_attack_player(raid_struct *raid, player_struct *player)
{
	uint32_t now = time_helper::get_cached_time() / 1000;
	int target_index;// = pvp_raid_get_player_index(raid, player);
	struct raid_player_info * t = raid->get_raid_player_info(player->get_uuid(), &target_index);
	assert(t);

	uint32_t last_attack_time = 0;
	uint64_t ret = -1;
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (raid->PVP_DATA.assist_data[target_index][i].player_id == 0)
			break;
		if (raid->PVP_DATA.assist_data[target_index][i].player_id == player->get_uuid())
			continue;

		if (raid->PVP_DATA.assist_data[target_index][i].damage_time > last_attack_time)
		{
			last_attack_time = raid->PVP_DATA.assist_data[target_index][i].damage_time;
			ret = raid->PVP_DATA.assist_data[target_index][i].player_id;
		}
	}

	if (now - last_attack_time > 3)
	{
		return NULL;
	}
	return player_manager::get_player_by_id(ret);
}

static void do_frozen_effect(player_struct *player)
{
	uint16_t region_id = player->get_attr(PLAYER_ATTR_REGION_ID);
	if (region_id != 13)
		return;
	if (player->buff_state & BUFF_STATE_AVOID_BLUE_BUFF)
		return;
	int32_t randnum = random() % 10000;

	LOG_INFO("%s: player[%lu] in frozen rand = %d", __FUNCTION__, player->get_uuid(), randnum);

	if (randnum <= sg_pvp_raid_blue_region_buff_rate)
		buff_manager::create_default_buff(sg_pvp_raid_blue_region_buff_id[0], player, player, true);
}

// public enum MapAreaType
// {
//	Area_Normal=10,//普通区域
//	Area_Peace=11,//和平区域
//	Area_Camp=12,//阵营区域
//	Area_Shade = 13,//阴区域
//	Area_Positive = 14,//阳区域
//	Area_Positive = 15,//悬崖
// }
static void	do_region_effect(raid_struct *raid, player_struct *player)
{
	uint16_t region_id = player->get_attr(PLAYER_ATTR_REGION_ID);
	LOG_INFO("%s: player[%lu] change region %u", __FUNCTION__, player->get_uuid(), region_id);

	switch(region_id)
	{
		case 13:
		{
			if (player->buff_state & BUFF_STATE_AVOID_BLUE_BUFF)
				return;
//			player->send_enter_region_notify(region_id);
//			int32_t randnum = random() % 10000;
//			if (randnum <= sg_pvp_raid_blue_region_buff_rate)
//				buff_manager::create_buff(sg_pvp_raid_blue_region_buff_id[0], player, player, true);
			buff_manager::create_default_buff(sg_pvp_raid_blue_region_buff_id[1], player, player, true);
		}
		break;
		case 14:
		{
			if (player->buff_state & BUFF_STATE_AVOID_RED_BUFF)
				return;
//			player->send_enter_region_notify(region_id);
			buff_manager::create_default_buff(sg_pvp_raid_red_region_buff_id[0], player, player, true);
			buff_manager::create_default_buff(sg_pvp_raid_red_region_buff_id[1], player, player, true);
		}
		break;
		case 15:
		{
			PvpRaidPlayerFallNotify nty;
			pvp_raid_player_fall_notify__init(&nty);
			nty.player_id = player->get_uuid();
			raid->broadcast_to_raid(MSG_ID_PVP_RAID_PLAYER_FALL_NOTIFY, &nty, (pack_func)pvp_raid_player_fall_notify__pack);

//			player->send_enter_region_notify(region_id);
			player->set_attr(PLAYER_ATTR_HP, -1);
			player->broadcast_one_attr_changed(PLAYER_ATTR_HP, -1, true, true);
			player->on_hp_changed(0);
			player->on_dead(player);

			// player_struct *killer = get_last_attack_player(raid, player);
			// if (killer)
			// {
			//	int index;// = pvp_raid_get_player_index(raid, player);
			//	struct raid_player_info *t = raid->get_raid_player_info(player->get_uuid(), &index);
			//	assert(t);
			//	pvp_raid_player_kill(raid, index, killer, player);
			// }
		}
		break;
		default:
		{
			player->clear_one_buff(sg_pvp_raid_blue_region_buff_id[0]);
			player->clear_one_buff(sg_pvp_raid_blue_region_buff_id[1]);
			player->clear_one_buff(sg_pvp_raid_red_region_buff_id[0]);
			player->clear_one_buff(sg_pvp_raid_red_region_buff_id[1]);
		}
	}
}

static void pvp_raid_ai_player_region_changed(raid_struct *raid, player_struct *player, uint32_t old_region, uint32_t region_id)
{
	do_region_effect(raid, player);
}

static void send_center_buff_changed_notify(raid_struct *raid)
{
	PvpRaidBuffReliveTimeNotify nty;
	pvp_raid_buff_relive_time_notify__init(&nty);
	nty.red_center_buff_relive_time = raid->PVP_DATA.red_buff_relive_time;	
	nty.blue_center_buff_relive_time = raid->PVP_DATA.blue_buff_relive_time;

	raid->broadcast_to_raid(MSG_ID_PVP_RAID_BUFF_RELIVE_TIME_NOTIFY, &nty, (pack_func)pvp_raid_buff_relive_time_notify__pack);	
}

static void	check_buff_range(raid_struct *raid, uint32_t now)
{
	bool send = false;
	if (raid->PVP_DATA.red_buff_relive_time == 0)
	{
		for (int i = 0; i < MAX_TEAM_MEM; ++i)
		{
			if (check_player_in_buff_range(raid->m_player[i], sg_3v3_pvp_raid_red_buff_param[1], sg_3v3_pvp_raid_red_buff_param[3]))
			{
					//  add buff
				buff_manager::create_default_buff(sg_pvp_center_buff_id[0], raid->m_player[i], raid->m_player[i], true);
				raid->PVP_DATA.red_buff_relive_time = now + sg_pvp_raid_buff_relive_time;
				send_get_buff(raid, 0, raid->m_player[i]->get_uuid());
				send = true;
				break;
			}
			if (check_player_in_buff_range(raid->m_player2[i], sg_3v3_pvp_raid_red_buff_param[1], sg_3v3_pvp_raid_red_buff_param[3]))
			{
					// add buff
				buff_manager::create_default_buff(sg_pvp_center_buff_id[0], raid->m_player2[i], raid->m_player2[i], true);
				raid->PVP_DATA.red_buff_relive_time = now + sg_pvp_raid_buff_relive_time;
				send_get_buff(raid, 0, raid->m_player2[i]->get_uuid());
				send = true;
				break;
			}
		}
	}
	else if (now > raid->PVP_DATA.red_buff_relive_time)
	{
		raid->PVP_DATA.red_buff_relive_time = 0;
		send = true;
	}

	if (raid->PVP_DATA.blue_buff_relive_time == 0)
	{
		for (int i = 0; i < MAX_TEAM_MEM; ++i)
		{
			if (check_player_in_buff_range(raid->m_player[i], sg_3v3_pvp_raid_blue_buff_param[1], sg_3v3_pvp_raid_blue_buff_param[3]))
			{
					// add buff
				buff_manager::create_default_buff(sg_pvp_center_buff_id[1], raid->m_player[i], raid->m_player[i], true);
				raid->PVP_DATA.red_buff_relive_time = now + sg_pvp_raid_buff_relive_time;
				send_get_buff(raid, 1, raid->m_player[i]->get_uuid());
				send = true;
				break;
			}
			if (check_player_in_buff_range(raid->m_player2[i], sg_3v3_pvp_raid_blue_buff_param[1], sg_3v3_pvp_raid_blue_buff_param[3]))
			{
					// add buff
				buff_manager::create_default_buff(sg_pvp_center_buff_id[1], raid->m_player2[i], raid->m_player2[i], true);
				raid->PVP_DATA.red_buff_relive_time = now + sg_pvp_raid_buff_relive_time;
				send_get_buff(raid, 1, raid->m_player2[i]->get_uuid());
				send = true;
				break;
			}
		}
	}
	else if (now > raid->PVP_DATA.blue_buff_relive_time)
	{
		raid->PVP_DATA.blue_buff_relive_time = 0;
		send = true;
	}

	if (send)
	{
		send_center_buff_changed_notify(raid);
	}

		//冰冻buff要间隔生效，不能在区域变化的时候检测
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (raid->m_player[i] && raid->m_player[i]->is_alive())
		{
			do_frozen_effect(raid->m_player[i]);
		}
		if (raid->m_player2[i] && raid->m_player2[i]->is_alive())
		{
			do_frozen_effect(raid->m_player2[i]);
		}
	}
}

static void pvp_raid_wait_start(raid_struct *raid)
{
//	LOG_DEBUG("%s", __FUNCTION__);
	raid->PVP_DATA.pvp_raid_state = PVP_RAID_STATE_WAIT_START;
	raid->data->start_time = time_helper::get_cached_time() + 10 * 1000;

	raid->stop_monster_ai();

	PvpRaidStartNotify nty;
	pvp_raid_start_notify__init(&nty);
	nty.start_time = raid->data->start_time / 1000;
	raid->broadcast_to_raid(MSG_ID_PVP_RAID_START_NOTIFY, &nty, (pack_func)pvp_raid_start_notify__pack);
}

static void pvp_raid_start(raid_struct *raid)
{
//	LOG_DEBUG("%s", __FUNCTION__);
	raid->PVP_DATA.pvp_raid_state = PVP_RAID_STATE_START;
//	pvp_raid_refresh_monster(raid);

	raid->PVP_DATA.refresh_monster_time = time_helper::get_cached_time() / 1000 + sg_pvp_raid_monster_first_refresh_time;
	
	raid->start_player_ai();
	raid->start_monster_ai();
}

static void pvp_raid_ai_tick(raid_struct *raid)
{
	uint32_t now = time_helper::get_cached_time() / 1000;
	int delta_time = now - raid->data->start_time / 1000;
//	LOG_DEBUG("%s %d state[%d]", __FUNCTION__, delta_time, raid->PVP_DATA.pvp_raid_state);
	switch (raid->PVP_DATA.pvp_raid_state)
	{
		case PVP_RAID_STATE_INIT:
		{
			if (delta_time >= 15)
				pvp_raid_wait_start(raid);
		}
		return;
		case PVP_RAID_STATE_START:
		{
		}
		break;
		case PVP_RAID_STATE_WAIT_START:
		{
			if (delta_time >= 0)
				pvp_raid_start(raid);
		}
		return;
		default:
			return;
	}

	//		// TODO: 改成normal raid state
	// if (raid->data->state == RAID_STATE_PASS)
	//	return;

	if (raid->PVP_DATA.refresh_monster_time != 0 && now > raid->PVP_DATA.refresh_monster_time)
	{
		pvp_raid_refresh_monster(raid);
	}

	check_buff_range(raid, now);
}

static void pvp_raid_ai_player_dead(raid_struct *raid, player_struct *player, unit_struct *)
{
	int index;// = pvp_raid_get_player_index(raid, player);
	struct raid_player_info *t = raid->get_raid_player_info(player->get_uuid(), &index);
	assert(t);
	raid->PVP_DATA.dead_record[index]++;

	player_struct *killer = get_last_attack_player(raid, player);
	if (killer)
	{
		pvp_raid_player_kill(raid, index, killer, player);
	}
	else
	{
		pvp_raid_player_killmyself(raid, index, player);
	}
}

static void pvp_raid_ai_monster_dead(raid_struct *raid, monster_struct *monster, unit_struct *killer)
{
	raid->PVP_DATA.refresh_monster_time = time_helper::get_cached_time() / 1000 + sg_pvp_raid_monster_refresh_time;
	int pos;
	if (raid->get_raid_player_info(killer->get_uuid(), &pos) == NULL)
		return;

	int buffid = -1;

	for (int i = 0; i < 3; ++i)
	{
		if ((int)monster->data->monster_id == sg_3v3_pvp_monster_id[i])
		{
			buffid = sg_3v3_pvp_buff_id[i];
//			buff_manager::create_buff(sg_3v3_pvp_buff_id[i], killer, monster, true);
			break;
		}
	}

	if (buffid == -1)
		return;

	player_struct **player;
	if (pos < MAX_TEAM_MEM)
		player = &raid->m_player[0];
	else
		player = &raid->m_player2[0];

	for (int i = 0; i < PVP_MATCH_PLAYER_NUM_3; ++i)
	{
		if (!player[i])
			continue;
		buff_manager::create_default_buff(buffid, player[i], player[i], true);
	}
}

static void pvp_raid_refresh_monster(raid_struct *raid)
{
	uint32_t rand = random() % 3;
	monster_manager::create_monster_at_pos(raid, sg_3v3_pvp_monster_id[rand], 1, sg_3v3_pvp_monster_place[rand * 4 + 1], sg_3v3_pvp_monster_place[rand * 4 + 3], 0, NULL);
	raid->PVP_DATA.refresh_monster_time = 0;
}

static void pvp_raid_ai_player_relive(raid_struct *raid, player_struct *player, uint32_t type)
{
	if (type == 1)	//原地复活
		return;
	player->data->attrData[PLAYER_ATTR_HP] = player->data->attrData[PLAYER_ATTR_MAXHP];
	player->on_hp_changed(0);

	ReliveNotify nty;
	relive_notify__init(&nty);
	nty.playerid = player->get_uuid();
	nty.type = type;
	pvp_raid_get_relive_pos(raid, &nty.pos_x, &nty.pos_z, &nty.direct);
	LOG_DEBUG("%s: player[%lu] relive to pos[%d][%d][%d]", __FUNCTION__, player->get_uuid(), nty.pos_x, nty.pos_z, nty.direct);

	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_RELIVE_NOTIFY, relive_notify__pack, nty);

	player->send_clear_sight();
	scene_struct *t_scene = raid;
	raid->delete_player_from_scene(player);
	player->set_pos(nty.pos_x, nty.pos_z);
	int camp_id = player->get_camp_id();
	t_scene->add_player_to_scene(player);
	player->set_camp_id(camp_id);

		//复活的时候加上一个无敌buff
	buff_manager::create_default_buff(114400001, player, player, false);

	player->m_team == NULL ? true : player->m_team->OnMemberHpChange(*player);
}

static struct assist_data *get_assist_data(raid_struct *raid, uint64_t player_id, int index)
{
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (raid->PVP_DATA.assist_data[index][i].player_id == 0)
		{
			raid->PVP_DATA.assist_data[index][i].player_id = player_id;
			return &raid->PVP_DATA.assist_data[index][i];
		}
		if (raid->PVP_DATA.assist_data[index][i].player_id == player_id)
			return &raid->PVP_DATA.assist_data[index][i];
	}
	return NULL;
}

static void get_team_kill_num(raid_struct *raid, int *kill1, int *kill2)
{
	int a = 0, b = 0;
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		a += raid->PVP_DATA.kill_record[i];
		b += raid->PVP_DATA.kill_record[i + MAX_TEAM_MEM];
	}
	*kill1 = a;
	*kill2 = b;
}

// static void add_team_raid_score(player_struct *player[MAX_TEAM_MEM])
// {
//	for (int i = 0; i < MAX_TEAM_MEM; ++i)
//	{
//		if (!player[i])
//			continue;
//		player[i]->add_pvp_raid_score(PVP_TYPE_DEFINE_3, 1);
//	}
// }

// static void sub_team_raid_score(player_struct *player[MAX_TEAM_MEM])
// {
//	for (int i = 0; i < MAX_TEAM_MEM; ++i)
//	{
//		if (!player[i])
//			continue;
//		player[i]->sub_pvp_raid_score(PVP_TYPE_DEFINE_3, 1);
//	}
// }

static int calc_expect_win_rate(raid_struct *raid, double *t1, double *t2)
{
		//理论胜率=1/（1+10^(（敌方总积分-我方总积分）/400）
	int t1_value = 0, t2_value = 0;
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (raid->m_player[i])
		{
			t1_value += raid->m_player[i]->data->pvp_raid_data.score_3;
		}
		if (raid->m_player2[i])
		{
			t2_value += raid->m_player2[i]->data->pvp_raid_data.score_3;
		}
	}

	double delta = (t1_value - t2_value) / 400.0;
	*t1 = 1.0 / (1 + pow(10, delta));
	*t2 = 1.0 / (1 + pow(10, -delta));
	return (0);
}

// static int32_t calc_win_score(raid_struct *raid, player_struct *player, double rate)
// {
//		//结算积分=（胜/负/平系数-理论胜率）*段位K值*3V3系数
//	rate = (sg_pvp_raid_win_score_param - rate) * sg_pvp_raid_reward_score_3v3_param;
//	struct StageTable *config = get_config_by_id(player->data->pvp_raid_data.cur_level_id_3, &pvp_raid_config);
//	if (!config)
//		return 0;
//	return rate * config->StageValue;
// }

// static int32_t calc_tie_score(raid_struct *raid, player_struct *player, double rate)
// {
//	rate = (sg_pvp_raid_tie_score_param - rate) * sg_pvp_raid_reward_score_3v3_param;
//	struct StageTable *config = get_config_by_id(player->data->pvp_raid_data.cur_level_id_3, &pvp_raid_config);
//	if (!config)
//		return 0;
//	return rate * config->StageValue;
// }

// static int32_t calc_lose_score(raid_struct *raid, player_struct *player, double rate)
// {
//	rate = (sg_pvp_raid_lose_score_param - rate) * sg_pvp_raid_reward_score_3v3_param;
//	struct StageTable *config = get_config_by_id(player->data->pvp_raid_data.cur_level_id_3, &pvp_raid_config);
//	if (!config)
//		return 0;
//	return rate * config->StageValue;
// }

static int32_t calc_score_changed(raid_struct *raid, player_struct *player, double rate, double param)
{
		//结算积分=（胜/负/平系数-理论胜率）*段位K值*3V3系数
	rate = (param - rate) * sg_pvp_raid_reward_score_3v3_param;
	struct StageTable *config = get_config_by_id(player->data->pvp_raid_data.cur_level_id_3, &pvp_raid_config);
	if (!config)
		return 0;
	int32_t ret = rate * config->StageValue;
	if (ret < 0 && player->data->pvp_raid_data.level_3 == 1)
		ret = 0;
	return ret;
}
static int32_t calc_courage_gold_changed(raid_struct *raid, player_struct *player, int index, int team_kill, int team_assist, double param)
{
//结算货币=段位基础货币*胜/负/平货币系数*（基础贡献值+杀人货币系数*个人杀人数/团队杀人数+辅助货币系数*个人辅助数/团队辅助数）
	struct StageTable *config = get_config_by_id(player->data->pvp_raid_data.cur_level_id_3, &pvp_raid_config);
	if (!config)
		return 0;

	if (team_kill == 0)
		team_kill = 1;
	if (team_assist == 0)
		team_assist = 1;

	double rate = sg_pvp_raid_basic_param + sg_pvp_raid_kill_param * raid->PVP_DATA.kill_record[index] / team_kill;
	rate += sg_pvp_raid_assist_param * raid->PVP_DATA.assist_record[index] / team_assist;
	return config->BasicsCoinValue * param * rate;
}

// static int32_t calc_win_courage_gold(raid_struct *raid, player_struct *player, int index, int team_kill, int team_assist)
// {
// //结算货币=段位基础货币*胜/负/平货币系数*（基础贡献值+杀人货币系数*个人杀人数/团队杀人数+辅助货币系数*个人辅助数/团队辅助数）
//	struct StageTable *config = get_config_by_id(player->data->pvp_raid_data.cur_level_id_3, &pvp_raid_config);
//	if (!config)
//		return 0;

//	if (team_kill == 0)
//		team_kill = 1;
//	if (team_assist == 0)
//		team_assist = 1;

//	double rate = sg_pvp_raid_basic_param + sg_pvp_raid_kill_param * raid->PVP_DATA.kill_record[index] / team_kill;
//	rate += sg_pvp_raid_assist_param * raid->PVP_DATA.assist_record[index] / team_assist;
//	return config->BasicsCoinValue * sg_pvp_raid_win_gold_param * rate;
// }
// static int32_t calc_tie_courage_gold(raid_struct *raid, player_struct *player, int index, int team_kill, int team_assist)
// {
// //结算货币=段位基础货币*胜/负/平货币系数*（基础贡献值+杀人货币系数*个人杀人数/团队杀人数+辅助货币系数*个人辅助数/团队辅助数）
//	struct StageTable *config = get_config_by_id(player->data->pvp_raid_data.cur_level_id_3, &pvp_raid_config);
//	if (!config)
//		return 0;

//	if (team_kill == 0)
//		team_kill = 1;
//	if (team_assist == 0)
//		team_assist = 1;

//	double rate = sg_pvp_raid_basic_param + sg_pvp_raid_kill_param * raid->PVP_DATA.kill_record[index] / team_kill;
//	rate += sg_pvp_raid_assist_param * raid->PVP_DATA.assist_record[index] / team_assist;
//	return config->BasicsCoinValue * sg_pvp_raid_tie_gold_param * rate;
// }
// static int32_t calc_lose_courage_gold(raid_struct *raid, player_struct *player, int index, int team_kill, int team_assist)
// {
// //结算货币=段位基础货币*胜/负/平货币系数*（基础贡献值+杀人货币系数*个人杀人数/团队杀人数+辅助货币系数*个人辅助数/团队辅助数）
//	struct StageTable *config = get_config_by_id(player->data->pvp_raid_data.cur_level_id_3, &pvp_raid_config);
//	if (!config)
//		return 0;

//	if (team_kill == 0)
//		team_kill = 1;
//	if (team_assist == 0)
//		team_assist = 1;

//	double rate = sg_pvp_raid_basic_param + sg_pvp_raid_kill_param * raid->PVP_DATA.kill_record[index] / team_kill;
//	rate += sg_pvp_raid_assist_param * raid->PVP_DATA.assist_record[index] / team_assist;
//	return config->BasicsCoinValue * sg_pvp_raid_lose_gold_param * rate;
// }

// 副本结束
static void	finished_raid(raid_struct *raid, int win_team)
{
	LOG_DEBUG("%s: raid[%u][%lu] curtime = %lu", __FUNCTION__, raid->data->ID, raid->data->uuid, time_helper::get_cached_time());
	raid->clear_monster();
	raid->stop_player_ai();
	
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (raid->m_player[i] && raid->m_player[i]->data)
		{
			raid->m_player[i]->clear_player_sight();
			raid->m_player[i]->del_battle_partner_from_scene();
		}
//			raid->delete_player_from_scene(raid->m_player[i]);
		if (raid->m_player2[i] && raid->m_player2[i]->data)
		{
			raid->m_player2[i]->clear_player_sight();
			raid->m_player2[i]->del_battle_partner_from_scene();
		}
//			raid->delete_player_from_scene(raid->m_player2[i]);
	}

	raid->PVP_DATA.pvp_raid_state = PVP_RAID_STATE_FINISH;

	double t1_rate, t2_rate;
	calc_expect_win_rate(raid, &t1_rate, &t2_rate);

	PvpRaidFinishedNotify nty;
	pvp_raid_finished_notify__init(&nty);
	PvpRaidFinishPlayerInfo team1[MAX_TEAM_MEM], team2[MAX_TEAM_MEM];
	PvpRaidFinishPlayerInfo *team1_point[MAX_TEAM_MEM], *team2_point[MAX_TEAM_MEM];
	int index1 = 0, index2 = 0;
	int team1_kill = 0, team2_kill = 0;
	int team1_assist = 0, team2_assist = 0;
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (!raid->m_player[i])
			continue;

		team1_point[index1] = &team1[index1];
		pvp_raid_finish_player_info__init(&team1[index1]);
		team1[index1].player_id = raid->m_player[i]->get_uuid();
		team1[index1].name = raid->m_player[i]->get_name();
		team1[index1].head_icon = raid->m_player[i]->get_attr(PLAYER_ATTR_HEAD);
		team1[index1].kill = raid->PVP_DATA.kill_record[i];
		team1[index1].dead = raid->PVP_DATA.dead_record[i];
		team1[index1].assist = raid->PVP_DATA.assist_record[i];
		team1_kill += team1[index1].kill;
		team1_assist += team1[index1].assist;
		++index1;
	}

	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (!raid->m_player2[i])
			continue;

		team2_point[index2] = &team2[index2];
		pvp_raid_finish_player_info__init(&team2[index2]);
		team2[index2].player_id = raid->m_player2[i]->get_uuid();
		team2[index2].name = raid->m_player2[i]->get_name();
		team2[index2].head_icon = raid->m_player2[i]->get_attr(PLAYER_ATTR_HEAD);
		team2[index2].kill = raid->PVP_DATA.kill_record[i + MAX_TEAM_MEM];
		team2[index2].dead = raid->PVP_DATA.dead_record[i + MAX_TEAM_MEM];
		team2[index2].assist = raid->PVP_DATA.assist_record[i + MAX_TEAM_MEM];
		team2_kill += team2[index2].kill;
		team2_assist += team2[index2].assist;
		++index2;
	}

	EXTERN_DATA extern_data;
	double param1, param2;
	double gold_param1, gold_param2;
	switch (win_team)
	{
		case 1:
			param1 = sg_pvp_raid_win_score_param;
			param2 = sg_pvp_raid_lose_score_param;
			gold_param1 = sg_pvp_raid_win_gold_param;
			gold_param2 = sg_pvp_raid_lose_gold_param;
			break;
		case 2:
			param1 = sg_pvp_raid_lose_score_param;
			param2 = sg_pvp_raid_win_score_param;
			gold_param1 = sg_pvp_raid_lose_gold_param;
			gold_param2 = sg_pvp_raid_win_gold_param;
			break;
		default:
			param1 = sg_pvp_raid_tie_score_param;
			param2 = sg_pvp_raid_tie_score_param;
			gold_param1 = sg_pvp_raid_tie_gold_param;
			gold_param2 = sg_pvp_raid_tie_gold_param;
			break;
	}
	nty.raid_result = win_team;
	nty.team1_kill = team1_kill;
	nty.n_team1_player = index1;
	nty.team1_player = team1_point;
	nty.team2_kill = team2_kill;
	nty.n_team2_player = index2;
	nty.team2_player = team2_point;

	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (raid->m_player[i] && get_entity_type(raid->m_player[i]->get_uuid()) != ENTITY_TYPE_AI_PLAYER)
		{
			if (win_team == 1)
				raid->m_player[i]->add_today_pvp_win_num(PVP_TYPE_DEFINE_3);

			nty.score_changed = calc_score_changed(raid, raid->m_player[i], t1_rate, param1);

			uint32_t count = raid->m_player[i]->get_raid_reward_count(sg_3v3_pvp_raid_param1[0]);
			if (count >= sg_pvp_control_config_3->RewardTime)
			{
				nty.courage_gold_changed = 0;
			}
			else
			{
				nty.courage_gold_changed = calc_courage_gold_changed(raid, raid->m_player[i], i, team1_kill, team1_assist, gold_param1);
				raid->m_player[i]->add_attr(PLAYER_ATTR_COURAGE_GOLD, nty.courage_gold_changed);
				raid->m_player[i]->add_raid_reward_count(sg_3v3_pvp_raid_param1[0]);				
			}
			extern_data.player_id = raid->m_player[i]->get_uuid();
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_PVP_RAID_FINISHED_NOTIFY, pvp_raid_finished_notify__pack, nty);
			raid->m_player[i]->change_pvp_raid_score(PVP_TYPE_DEFINE_3, nty.score_changed);
			raid->m_player[i]->check_activity_progress(AM_RAID, raid->data->ID);
			server_level_listen_raid_finish(raid->data->ID, raid->m_player[i]);
		}
		if (raid->m_player2[i] && get_entity_type(raid->m_player2[i]->get_uuid()) != ENTITY_TYPE_AI_PLAYER)
		{
			if (win_team == 2)
				raid->m_player2[i]->add_today_pvp_win_num(PVP_TYPE_DEFINE_3);

			nty.score_changed = calc_score_changed(raid, raid->m_player2[i], t2_rate, param2);

			uint32_t count = raid->m_player2[i]->get_raid_reward_count(sg_3v3_pvp_raid_param1[0]);
			if (count >= sg_pvp_control_config_3->RewardTime)
			{
				nty.courage_gold_changed = 0;				
			}
			else
			{
				nty.courage_gold_changed = calc_courage_gold_changed(raid, raid->m_player2[i], i + MAX_TEAM_MEM, team2_kill, team2_assist, gold_param2);
				raid->m_player2[i]->add_attr(PLAYER_ATTR_COURAGE_GOLD, nty.courage_gold_changed);
				raid->m_player2[i]->add_raid_reward_count(sg_3v3_pvp_raid_param1[0]);
			}
			extern_data.player_id = raid->m_player2[i]->get_uuid();
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_PVP_RAID_FINISHED_NOTIFY, pvp_raid_finished_notify__pack, nty);
			raid->m_player2[i]->change_pvp_raid_score(PVP_TYPE_DEFINE_3, -nty.score_changed);
			raid->m_player2[i]->check_activity_progress(AM_RAID, raid->data->ID);
			server_level_listen_raid_finish(raid->data->ID, raid->m_player2[i]);
		}
//		add_team_raid_score(raid->m_player);
//		sub_team_raid_score(raid->m_player2);
	}
//	raid->broadcast_to_raid(MSG_ID_PVP_RAID_FINISHED_NOTIFY, &nty, (pack_func)pvp_raid_finished_notify__pack);

}

static void pvp_raid_check_ace(raid_struct *raid, int target_index)
{
	bool ace = true;
	int index = target_index;
//	struct raid_player_info *t = raid->get_raid_player_info(player->get_uuid(), &index);
//	assert(t);

		//检查ACE
	if (index < MAX_TEAM_MEM)
	{
		for (int i = 0; i < MAX_TEAM_MEM; ++i)
		{
			if (raid->m_player[i] && raid->m_player[i]->is_alive())
			{
				ace = false;
				break;
			}
		}
		if (ace)
		{
			raid->m_raid_team2->BroadcastToTeam(MSG_ID_PVP_RAID_ACE_NOTIFY, NULL, NULL, 0);
			raid->m_raid_team->BroadcastToTeam(MSG_ID_PVP_RAID_BE_ACE_NOTIFY, NULL, NULL, 0);
		}
	}
	else
	{
		for (int i = 0; i < MAX_TEAM_MEM; ++i)
		{
			if (raid->m_player2[i] && raid->m_player2[i]->is_alive())
			{
				ace = false;
				break;
			}
		}
		if (ace)
		{
			raid->m_raid_team->BroadcastToTeam(MSG_ID_PVP_RAID_ACE_NOTIFY, NULL, NULL, 0);
			raid->m_raid_team2->BroadcastToTeam(MSG_ID_PVP_RAID_BE_ACE_NOTIFY, NULL, NULL, 0);
		}
	}
}

static void	pvp_raid_player_killmyself(raid_struct *raid, int target_index, player_struct *target)
{
	pvp_raid_check_ace(raid, target_index);	
}

static void pvp_raid_player_kill(raid_struct *raid, int target_index, player_struct *player, player_struct *target)
{
	uint32_t now = time_helper::get_cached_time() / 1000;
		//kill_record, dead_record, assist_record
//	raid->PVP_DATA.dead_record[target_index]++;
	int t;// = pvp_raid_get_player_index(raid, player);
	struct raid_player_info *tmp = raid->get_raid_player_info(player->get_uuid(), &t);
	assert(tmp);

	raid->PVP_DATA.kill_record[t]++;

	PvpKillNotify nty;
	pvp_kill_notify__init(&nty);
	nty.dead_player_id = target->get_uuid();
	nty.kill_player_id = player->get_uuid();
	uint64_t assist_player_id[MAX_TEAM_MEM];

	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (raid->PVP_DATA.assist_data[target_index][i].player_id == 0)
			break;
		if (raid->PVP_DATA.assist_data[target_index][i].player_id == player->get_uuid())
			continue;

			// 检查时间
		if (now - raid->PVP_DATA.assist_data[target_index][i].damage_time > 5)
			continue;

//		t = pvp_raid_get_player_index2(raid, raid->PVP_DATA.assist_data[target_index][i].player_id);
		tmp = raid->get_raid_player_info(raid->PVP_DATA.assist_data[target_index][i].player_id, &t);
		assert(tmp);
		raid->PVP_DATA.assist_record[t]++;

		assist_player_id[nty.n_assist_player_id++] = raid->PVP_DATA.assist_data[target_index][i].player_id;
	}
	nty.assist_player_id = assist_player_id;
	raid->broadcast_to_raid(MSG_ID_PVP_RAID_KILL_NOTIFY, &nty, (pack_func)pvp_kill_notify__pack);

		// 检查杀人数目是否足够结束游戏
	int team_kill_1, team_kill_2;
	get_team_kill_num(raid, &team_kill_1, &team_kill_2);

	if (team_kill_1 >= (int)raid->m_config->PassValue[0])
	{
		finished_raid(raid, 1);
		return;
	}
	else if (team_kill_2 >= (int)raid->m_config->PassValue[0])
	{
		finished_raid(raid, 2);
		return;
	}
	pvp_raid_check_ace(raid, target_index);
}

static void pvp_raid_ai_player_attack(raid_struct *raid, player_struct *player, unit_struct *target, int damage)
{
	if (target->get_unit_type() != UNIT_TYPE_PLAYER)
		return;

	int index;// = pvp_raid_get_player_index(raid, (player_struct *)target);
	struct raid_player_info *t = raid->get_raid_player_info(target->get_uuid(), &index);
	assert(t);

	struct assist_data *data = get_assist_data(raid, player->get_uuid(), index);
	assert(data);
	data->damage_time = time_helper::get_cached_time() / 1000;

	// if (!target->is_alive())
	// {
	//	pvp_raid_player_kill(raid, index, player, (player_struct *)target);
	// }
}

static void pvp_raid_ai_player_leave(raid_struct *raid, player_struct *player)
{
	player->set_out_raid_pos();
}

static void pvp_raid_ai_player_ready(raid_struct *raid, player_struct *player)
{
	int pos;
	raid->get_raid_player_info(player->get_uuid(), &pos);
	if (pos >= MAX_TEAM_MEM)
	{
			//这里和配置对应，不用宏PVP_MATCH_PLAYER_NUM_3, 而是写死了3
		if (player->ai_data)
			player->ai_data->ai_patrol_config = robot_patrol_config[pos - MAX_TEAM_MEM + 3];		
		player->set_camp_id(2);
	}
	else
	{
		if (player->ai_data)		
			player->ai_data->ai_patrol_config = robot_patrol_config[pos];				
		player->set_camp_id(1);
	}
//	assert(player->ai_patrol_config);
//	LOG_DEBUG("%s: player[%s] ID[%lu] camp[%d] pos[%d]", __FUNCTION__, player->get_name(), player->ai_patrol_config->ID
//		, player->data->camp_id, pos);

	if (raid->PVP_DATA.pvp_raid_state != PVP_RAID_STATE_INIT)
	{
		PvpRaidStartNotify nty;
		pvp_raid_start_notify__init(&nty);
		nty.start_time = raid->data->start_time / 1000;
		EXTERN_DATA extern_data;
		extern_data.player_id = player->get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_PVP_RAID_START_NOTIFY, pvp_raid_start_notify__pack, nty);

		return;
	}

	// if (pos >= MAX_TEAM_MEM)
	// {
	//	pos -= MAX_TEAM_MEM;
	// }
	raid->PVP_DATA.pvp_raid_ready[pos] = true;

	for (int i = 0; i < PVP_MATCH_PLAYER_NUM_3; ++i)
	{
		if (!raid->PVP_DATA.pvp_raid_ready[i])
			return;
		if (!raid->PVP_DATA.pvp_raid_ready[i + MAX_TEAM_MEM])
			return;
	}

	pvp_raid_wait_start(raid);
}

static void pvp_raid_ai_timeout(raid_struct *raid)
{
//	assert(raid->m_config->n_Score == 1);
//	assert(raid->m_config->Score[0] == 1);
//	if (delta_time > (int)(raid->m_config->ScoreValue[0]))
	{
			// 时间到了，副本结束
		int team_kill_1, team_kill_2;
		get_team_kill_num(raid, &team_kill_1, &team_kill_2);
		if (team_kill_1 > team_kill_2)
			finished_raid(raid, 1);
		else if (team_kill_1 < team_kill_2)
			finished_raid(raid, 2);
		else
			finished_raid(raid, 0);
		return;
	}
}

struct raid_ai_interface raid_ai_pvp3_interface =
{
	pvp_raid_ai_init,
	pvp_raid_ai_tick,
	NULL, //pvp_raid_ai_player_enter,
	pvp_raid_ai_player_leave,
	pvp_raid_ai_player_dead,
	pvp_raid_ai_player_relive,
	pvp_raid_ai_monster_dead,
	NULL,// pvp_raid_ai_collect,
	pvp_raid_ai_player_ready,
	NULL,// pvp_raid_ai_finished,
	pvp_raid_ai_player_attack,
	pvp_raid_ai_player_region_changed,
	NULL,
	NULL,
	NULL,
	.raid_on_failed = pvp_raid_ai_timeout,
};
