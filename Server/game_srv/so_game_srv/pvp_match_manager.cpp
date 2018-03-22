#include "pvp_match_manager.h"
#include "player_manager.h"
#include "msgid.h"
#include "uuid.h"
#include "raid_manager.h"
#include "../proto/pvp_raid.pb-c.h"
#include "../proto/raid.pb-c.h"
#include "../proto/scene_transfer.pb-c.h"
#include "lua_config.h"
#include "team.h"
#include <map>
#include <vector>
#include <assert.h>

//player_id, 段位
//等待匹配的玩家或者队伍
extern std::map<uint64_t, uint8_t> pvp_waiting_player_3;
extern std::map<uint64_t, uint8_t> pvp_waiting_player_5;
extern std::map<uint64_t, uint8_t> pvp_waiting_team_3;
extern std::map<uint64_t, uint8_t> pvp_waiting_team_5;

struct matched_team_3
{
	uint32_t end_time;
	uint64_t team1[PVP_MATCH_PLAYER_NUM_3];
	uint64_t team2[PVP_MATCH_PLAYER_NUM_3];
};
struct matched_team_5
{
	uint32_t end_time;
	uint64_t team1[PVP_MATCH_PLAYER_NUM_5];
	uint64_t team2[PVP_MATCH_PLAYER_NUM_5];
};
//完成匹配，等待玩家点击准备的队伍
extern std::map<uint64_t, struct matched_team_3 *> pvp_map_team_3;
extern std::map<uint64_t, struct matched_team_5 *> pvp_map_team_5;
extern uint64_t pvp_matched_index;

static void	send_cancel_notify(uint64_t player_id, uint64_t cancel_player_id)
{
	if (get_entity_type(player_id) != ENTITY_TYPE_PLAYER)
		return;
	PvpMatchCancelNotify nty;
	pvp_match_cancel_notify__init(&nty);
	nty.player_id = cancel_player_id;
	nty.result = 1;
	EXTERN_DATA ext;
	ext.player_id = player_id;
	fast_send_msg(&conn_node_gamesrv::connecter, &ext,
		MSG_ID_PVP_MATCH_CANCEL_NOTIFY, pvp_match_cancel_notify__pack, nty);
}
static void	send_ready_notify(uint64_t player_id, uint64_t ready_player_id)
{
	if (get_entity_type(player_id) != ENTITY_TYPE_PLAYER)
		return;
	PvpMatchReadyNotify nty;
	pvp_match_ready_notify__init(&nty);
	nty.player_id = ready_player_id;
	EXTERN_DATA ext;
	ext.player_id = player_id;
	fast_send_msg(&conn_node_gamesrv::connecter, &ext,
		MSG_ID_PVP_MATCH_READY_NOTIFY, pvp_match_ready_notify__pack, nty);
}

static bool	check_team_info_3(struct matched_team_3 *team)
{
	Team *t[PVP_MATCH_PLAYER_NUM_3];
	for (int i = 0; i < PVP_MATCH_PLAYER_NUM_3; ++i)
	{
		player_struct *player = player_manager::get_player_by_id(team->team1[0]);
		if (!player)
			return false;
		t[i] = player->m_team;
		if (t[i] != t[0])
			return false;
	}
	if (t[0] && t[0]->GetMemberSize() != PVP_MATCH_PLAYER_NUM_3)
		return false;

//	if (t[1] != t[0] || t[2] != t[0])
//		return false;

	for (int i = 0; i < PVP_MATCH_PLAYER_NUM_3; ++i)
	{
		player_struct *player = player_manager::get_player_by_id(team->team2[0]);
		if (!player)
			return false;
		t[i] = player->m_team;
		if (t[i] != t[0])
			return false;		
	}
	if (t[0] && t[0]->GetMemberSize() != PVP_MATCH_PLAYER_NUM_3)
		return false;
//	if (t[1] != t[0] || t[2] != t[0])
//		return false;
	return true;
}

static int create_tmp_team_3(raid_struct *raid, struct matched_team_3 *team)
{
	player_struct *player[PVP_MATCH_PLAYER_NUM_3];
	player[0] = player_manager::get_player_by_id(team->team1[0]);
	if (!player[0]->m_team)
	{
		for (int i = 1; i < PVP_MATCH_PLAYER_NUM_3; ++i)
			player[i] = player_manager::get_player_by_id(team->team1[i]);
//		player[2] = player_manager::get_player_by_id(team->team1[2]);
		Team *team = Team::CreateTeam(player, PVP_MATCH_PLAYER_NUM_3);
		raid->data->delete_team1 = true;
		raid->data->team_id = team->GetId();
		raid->m_raid_team = team;
		team->m_data->m_raid_uuid = raid->data->uuid;
	}

	player[0] = player_manager::get_player_by_id(team->team2[0]);
	if (!player[0]->m_team)
	{
		for (int i = 1; i < PVP_MATCH_PLAYER_NUM_3; ++i)		
			player[i] = player_manager::get_player_by_id(team->team2[i]);
//		player[2] = player_manager::get_player_by_id(team->team2[2]);
		Team *team = Team::CreateTeam(player, PVP_MATCH_PLAYER_NUM_3);
		raid->data->delete_team2 = true;
		raid->data->team2_id = team->GetId();
		raid->m_raid_team2 = team;
		team->m_data->m_raid_uuid = raid->data->uuid;
	}
	return (0);
}

static int start_pvp_raid_3(struct matched_team_3 *team)
{
//确保都是一个队伍的，或者都没有队伍
	if (!check_team_info_3(team))
	{
		LOG_ERR("%s check team info fail", __FUNCTION__);
	}

	raid_struct *raid = raid_manager::create_raid(sg_3v3_pvp_raid_param1[0], NULL);
	if (!raid)
	{
		LOG_ERR("%s: create raid failed", __FUNCTION__);
		return (-10);
	}

	create_tmp_team_3(raid, team);

	uint32_t max_lv = 0;
	uint32_t player_num = 0;

	player_struct *player;
//	EXTERN_DATA extern_data;
//	EnterRaidNotify notify;
//	SceneTransferAnswer resp;
//	enter_raid_notify__init(&notify);
//	notify.raid_id = sg_3v3_pvp_raid_param1[0];
	for (int i = 0; i < PVP_MATCH_PLAYER_NUM_3; ++i)
	{
		player = player_manager::get_player_by_id(team->team1[i]);
		assert(player);

		player_num++;
		max_lv += player->get_level();
		
// 		raid->m_player[i] = player;
// //		memset(&raid->data->player_info[i], 0, sizeof(raid->data->player_info[i]));
// //		raid->data->player_info[i].player_id = player->get_uuid();
// 		raid->set_player_info(player, &raid->data->player_info[i]);
// 		player->set_enter_raid_pos_and_scene(raid);

 		float rand1 = (random() % (sg_3v3_pvp_raid_param1[5] * 2) - sg_3v3_pvp_raid_param1[5]) / 100.0;
 		float pos_x = sg_3v3_pvp_raid_param1[1] + rand1;
 		rand1 = (random() % (sg_3v3_pvp_raid_param1[5] * 2) - sg_3v3_pvp_raid_param1[5]) / 100.0;		
 		float pos_z = sg_3v3_pvp_raid_param1[3] + rand1;

		raid->player_enter_raid_impl(player, i, pos_x, pos_z);
		
// 		player->set_pos(pos_x, pos_z);

		// if (get_entity_type(player->get_uuid()) == ENTITY_TYPE_AI_PLAYER)
		// {
		// 	raid->add_player_to_scene(player);
		// 	if (raid->ai && raid->ai->raid_on_player_ready)
		// 		raid->ai->raid_on_player_ready(raid, player);
		// }
		// else
		// {
		// 	extern_data.player_id = player->get_uuid();
		// 	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ENTER_RAID_NOTIFY, enter_raid_notify__pack, notify);

		// 	player->send_scene_transfer(sg_3v3_pvp_raid_param1[4], pos_x, sg_3v3_pvp_raid_param1[2],
		// 		pos_z, sg_3v3_pvp_raid_param1[0], 0);
		// }

 		// raid->on_player_enter_raid(player);

		player = player_manager::get_player_by_id(team->team2[i]);
		assert(player);
// 		raid->m_player2[i] = player;
// //		memset(&raid->data->player_info2[i], 0, sizeof(raid->data->player_info[i]));
// //		raid->data->player_info2[i].player_id = player->get_uuid();
// 		raid->set_player_info(player, &raid->data->player_info2[i]);		
// 		player->set_enter_raid_pos_and_scene(raid);
// 		player->set_pos(sg_3v3_pvp_raid_param2[1], sg_3v3_pvp_raid_param2[3]);

		raid->player_enter_raid_impl(player, i + MAX_TEAM_MEM, sg_3v3_pvp_raid_param2[1], sg_3v3_pvp_raid_param2[3]);

		// if (get_entity_type(player->get_uuid()) == ENTITY_TYPE_AI_PLAYER)
		// {
		// 	raid->add_player_to_scene(player);
		// 	if (raid->ai && raid->ai->raid_on_player_ready)
		// 		raid->ai->raid_on_player_ready(raid, player);
		// }
		// else
		// {
		// 	extern_data.player_id = player->get_uuid();
		// 	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ENTER_RAID_NOTIFY, enter_raid_notify__pack, notify);

		// 	player->send_scene_transfer(sg_3v3_pvp_raid_param2[4], sg_3v3_pvp_raid_param2[1], sg_3v3_pvp_raid_param2[2],
		// 		sg_3v3_pvp_raid_param2[3], sg_3v3_pvp_raid_param2[0], 0);
		// }

		// raid->on_player_enter_raid(player);
	}

	raid->PVP_DATA.average_lv = max_lv / player_num;
	LOG_INFO("%s: start pvp raid[%p][%lu], average lv = %u", __FUNCTION__, raid, raid->data->uuid, raid->PVP_DATA.average_lv);
	assert(raid->PVP_DATA.average_lv > 0);
	return (0);
}
static int start_pvp_raid_5(struct matched_team_5 *team)
{
	raid_struct *raid = raid_manager::create_raid(sg_5v5_pvp_raid_param1[0], NULL);
	if (!raid)
	{
		LOG_ERR("%s: create raid failed", __FUNCTION__);
		return (-10);
	}
	player_struct *player;
	for (int i = 0; i < 5; ++i)
	{
		player = player_manager::get_player_by_id(team->team1[i]);
		assert(player);
// 		raid->m_player[i] = player;
// //		memset(&raid->data->player_info[i], 0, sizeof(raid->data->player_info[i]));
// //		raid->data->player_info[i].player_id = player->get_uuid();
// 		raid->set_player_info(player, &raid->data->player_info[i]);		
// 		player->set_enter_raid_pos_and_scene(raid);

// 		EXTERN_DATA extern_data;
// 		extern_data.player_id = player->get_uuid();

// 		EnterRaidNotify notify;
// 		enter_raid_notify__init(&notify);
// 		notify.raid_id = sg_5v5_pvp_raid_param1[0];
// 		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ENTER_RAID_NOTIFY, enter_raid_notify__pack, notify);

// 		player->send_scene_transfer(sg_5v5_pvp_raid_param1[4], sg_5v5_pvp_raid_param1[1], sg_5v5_pvp_raid_param1[2],
// 			sg_5v5_pvp_raid_param1[3], sg_5v5_pvp_raid_param1[0], 0);

// 		raid->on_player_enter_raid(player);
		raid->player_enter_raid_impl(player, i, sg_5v5_pvp_raid_param1[1], sg_5v5_pvp_raid_param1[3]);

		player = player_manager::get_player_by_id(team->team2[i]);
		assert(player);
// 		raid->m_player2[i] = player;
// //		memset(&raid->data->player_info2[i], 0, sizeof(raid->data->player_info[i]));
// //		raid->data->player_info2[i].player_id = player->get_uuid();
// 		raid->set_player_info(player, &raid->data->player_info2[i]);		
// 		player->set_enter_raid_pos_and_scene(raid);

// 		extern_data.player_id = player->get_uuid();
// 		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ENTER_RAID_NOTIFY, enter_raid_notify__pack, notify);

// 		player->send_scene_transfer(sg_5v5_pvp_raid_param2[4], sg_5v5_pvp_raid_param2[1], sg_5v5_pvp_raid_param2[2],
// 			sg_5v5_pvp_raid_param2[3], sg_5v5_pvp_raid_param2[0], 0);

// 		raid->on_player_enter_raid(player);
		raid->player_enter_raid_impl(player, i + MAX_TEAM_MEM, sg_5v5_pvp_raid_param2[1], sg_5v5_pvp_raid_param2[3]);
	}
	return (0);
}

//玩家离线
void pvp_match_on_player_offline(player_struct *player)
{
	// if (player->m_team && pvp_match_is_team_in_waiting(player->m_team->GetId()))
	// {
	// 	PvpMatchCancelNotify ans;
	// 	pvp_match_cancel_notify__init(&ans);
	// 	ans.player_id = player->get_uuid();
	// 	ans.result = 190500113;
	// 	player->m_team->BroadcastToTeam(MSG_ID_PVP_MATCH_CANCEL_NOTIFY, &ans, (pack_func)pvp_match_cancel_notify__pack);
	// }
	pvp_match_player_cancel(player);
	// switch (player->data->pvp_raid_data.state)
	// {
	//	case pvp_match_state_waiting_3:
	//		break;
	//	case pvp_match_state_waiting_5:
	//		break;
	//	case pvp_match_state_not_ready_3:
	//	case pvp_match_state_ready_3:
	//	{
	//		uint64_t player_id = player->get_uuid();
	//		std::map<uint64_t, struct matched_team_3 *>::iterator ite = map_team_3.find(player->data->pvp_raid_data.matched_index);
	//		assert(ite != map_team_3.end());
	//		struct matched_team_3 *team = ite->second;
	//		map_team_3.erase(ite);
	//		for (int i = 0; i < 3; ++i)
	//		{
	//			if (team->team1[i] == player_id)
	//				continue;
	//			if (team->team2[i] == player_id)
	//				continue;

	//				// 重新加回等待队列, 发送MSG_ID_PVP_MATCH_CANCEL_NOTIFY
	//			player_struct *t_player = player_manager::get_player_by_id(team->team1[i]);
	//			if (t_player)
	//			{
	//				pvp_match_add_player_to_waiting(t_player, PVP_TYPE_DEFINE_3);
	//				send_cancel_notify(team->team1[i], player_id);
	//			}
	//			t_player = player_manager::get_player_by_id(team->team2[i]);
	//			if (t_player)
	//			{
	//				pvp_match_add_player_to_waiting(t_player, PVP_TYPE_DEFINE_3);
	//				send_cancel_notify(team->team2[i], player_id);
	//			}
	//		}
	//		free(team);
	//	}
	//	break;
	//	case pvp_match_state_not_ready_5:
	//	case pvp_match_state_ready_5:
	//	{
	//		uint64_t player_id = player->get_uuid();
	//		std::map<uint64_t, struct matched_team_5 *>::iterator ite = map_team_5.find(player->data->pvp_raid_data.matched_index);
	//		assert(ite != map_team_5.end());
	//		struct matched_team_5 *team = ite->second;
	//		map_team_5.erase(ite);
	//		for (int i = 0; i < 5; ++i)
	//		{
	//			if (team->team1[i] == player_id)
	//				continue;
	//			if (team->team2[i] == player_id)
	//				continue;
	//				// 重新加回等待队列, 发送MSG_ID_PVP_MATCH_CANCEL_NOTIFY
	//			player_struct *t_player = player_manager::get_player_by_id(team->team1[i]);
	//			if (t_player)
	//			{
	//				pvp_match_add_player_to_waiting(t_player, PVP_TYPE_DEFINE_5);
	//				send_cancel_notify(team->team1[i], player_id);
	//			}
	//			t_player = player_manager::get_player_by_id(team->team2[i]);
	//			if (t_player)
	//			{
	//				pvp_match_add_player_to_waiting(t_player, PVP_TYPE_DEFINE_5);
	//				send_cancel_notify(team->team2[i], player_id);
	//			}

	//		}
	//		free(team);
	//	}
	//	break;
	//	default:
	//		break;
	// }

	// std::map<uint64_t, uint8_t>::iterator ite;
	// ite = waiting_player_3.find(player_id);
	// if (ite != waiting_player_3.end())
	//	waiting_player_3.erase(ite);
	// ite = waiting_player_5.find(player_id);
	// if (ite != waiting_player_5.end())
	//	waiting_player_5.erase(ite);
}

//队伍人员变更
void pvp_match_on_team_member_changed(player_struct *player)
{
	if (player->m_team && player->data->team_raid_id_wait_ready != 0)
	{
		player->m_team->unset_raid_id_wait_ready();
		TeamRaidCancelNotify nty;
		team_raid_cancel_notify__init(&nty);
		nty.player_id = player->get_uuid();
		nty.result = 190500112;
		player->m_team->BroadcastToTeam(MSG_ID_TEAM_RAID_CANCEL_NOTIFY, &nty, (pack_func)team_raid_cancel_notify__pack, 0);
	}
	
//	send_pvp_match_start_answer(player, extern_data, 190500103, t_player->get_uuid(), cd);
	if (player->m_team && pvp_match_is_team_in_waiting(player->m_team->GetId()))
	{
		PvpMatchCancelNotify ans;
		pvp_match_cancel_notify__init(&ans);
		ans.player_id = player->get_uuid();
		ans.result = 190500112;
		player->m_team->BroadcastToTeam(MSG_ID_PVP_MATCH_CANCEL_NOTIFY, &ans, (pack_func)pvp_match_cancel_notify__pack);
	}

	pvp_match_player_cancel(player);
}

uint32_t pvp_match_is_player_in_cd(player_struct *player)
{
	uint32_t ret = time_helper::get_cached_time() / 1000 - player->pvp_raid_cancel_time;
	if (ret >= 15)
		return 0;
	return 15 - ret;
}

bool pvp_match_is_player_in_waiting(uint64_t player_id)
{
	if (pvp_waiting_player_3.find(player_id) != pvp_waiting_player_3.end())
		return true;
	if (pvp_waiting_player_5.find(player_id) != pvp_waiting_player_5.end())
		return true;
	return false;
}
bool pvp_match_is_team_in_waiting(uint64_t id)
{
	if (pvp_waiting_team_3.find(id) != pvp_waiting_team_3.end())
		return true;
	if (pvp_waiting_team_5.find(id) != pvp_waiting_team_5.end())
		return true;
	return false;
}

void pvp_raid_get_relive_pos(raid_struct *raid, int32_t *ret_pos_x, int32_t *ret_pos_z, int32_t *ret_direct)
{
	int index = -1;
	index = random() % raid->res_config->n_RelivePointX;
	int rand_x = random();
	rand_x = rand_x % (2 * raid->res_config->ReliveRange[index]);
	rand_x = rand_x - raid->res_config->ReliveRange[index];
	int rand_z = random();
	rand_z = rand_z % (2 * raid->res_config->ReliveRange[index]);
	rand_z = rand_z - raid->res_config->ReliveRange[index];
	*ret_pos_x = raid->res_config->RelivePointX[index] + rand_x;
	*ret_pos_z = raid->res_config->RelivePointZ[index] + rand_z;
	*ret_direct = raid->res_config->ReliveFaceY[index];

	if (raid->get_area_by_pos(*ret_pos_x, *ret_pos_z) == NULL)
	{
		LOG_ERR("%s: pos[%d][%d] failed", __FUNCTION__, *ret_pos_x, *ret_pos_z);
		*ret_pos_x = raid->res_config->RelivePointX[index];
		*ret_pos_z = raid->res_config->RelivePointZ[index];
		assert(raid->get_area_by_pos(*ret_pos_x, *ret_pos_z) != NULL);
	}
}

int pvp_match_add_player_to_waiting(player_struct *player, int type)
{
	if (get_entity_type(player->get_uuid()) == ENTITY_TYPE_AI_PLAYER)
		return 0;

	if (type == PVP_TYPE_DEFINE_3)
	{
		pvp_waiting_player_3[player->get_uuid()] = player->data->pvp_raid_data.level_3;
		player->data->pvp_raid_data.state = pvp_match_state_waiting_3;
	}
	else
	{
		pvp_waiting_player_5[player->get_uuid()] = player->data->pvp_raid_data.level_5;
		player->data->pvp_raid_data.state = pvp_match_state_waiting_5;
	}
	return (0);
}
int pvp_match_add_team_to_waiting(player_struct *player, int type)
{
	if (type == PVP_TYPE_DEFINE_3)
	{
		pvp_waiting_team_3[player->m_team->GetId()] = player->data->pvp_raid_data.level_3;
		for (int pos = 0; pos < player->m_team->m_data->m_memSize; ++pos)
		{
			player_struct *t_player = player_manager::get_player_by_id(player->m_team->m_data->m_mem[pos].id);
			assert(t_player);
			t_player->data->pvp_raid_data.state = pvp_match_state_waiting_3;
		}
	}
	else
	{
		pvp_waiting_team_5[player->m_team->GetId()] = player->data->pvp_raid_data.level_5;
		for (int pos = 0; pos < player->m_team->m_data->m_memSize; ++pos)
		{
			player_struct *t_player = player_manager::get_player_by_id(player->m_team->m_data->m_mem[pos].id);
			assert(t_player);
			t_player->data->pvp_raid_data.state = pvp_match_state_waiting_5;
		}
	}
	return (0);
}

int pvp_match_del_player_from_waiting(player_struct *player)
{
	return (0);
}
int pvp_match_del_team_from_waiting(uint64_t id)
{
	return (0);
}

static bool check_team_all_ready_3(struct matched_team_3 *team)
{
	player_struct *player;
	for (int i = 0; i < PVP_MATCH_PLAYER_NUM_3; ++i)
	{
		player = player_manager::get_player_by_id(team->team1[i]);
		assert(player);
		if (player->data->pvp_raid_data.state != pvp_match_state_ready_3)
			return false;

		player = player_manager::get_player_by_id(team->team2[i]);
		assert(player);
		if (player->data->pvp_raid_data.state != pvp_match_state_ready_3)
			return false;
	}
	return true;
}
static bool check_team_all_ready_5(struct matched_team_5 *team)
{
	player_struct *player;
	for (int i = 0; i < 5; ++i)
	{
		player = player_manager::get_player_by_id(team->team1[i]);
		assert(player);
		if (player->data->pvp_raid_data.state != pvp_match_state_ready_5)
			return false;

		player = player_manager::get_player_by_id(team->team2[i]);
		assert(player);
		if (player->data->pvp_raid_data.state != pvp_match_state_ready_5)
			return false;
	}
	return true;
}

static void	free_team_3_ai_player(struct matched_team_3 *team)
{
	player_struct *player;
	for (int i = 0; i < PVP_MATCH_PLAYER_NUM_3; ++i)
	{
		if (get_entity_type(team->team1[i]) == ENTITY_TYPE_AI_PLAYER)
		{
			player = player_manager::get_player_by_id(team->team1[i]);
			assert(player);
			player_manager::delete_player(player);
		}
		if (get_entity_type(team->team2[i]) == ENTITY_TYPE_AI_PLAYER)
		{
			player = player_manager::get_player_by_id(team->team2[i]);
			assert(player);
			player_manager::delete_player(player);
		}
	}
}

int pvp_match_player_set_ready(player_struct *player)
{
		// TODO: 检查是否在pvp副本排队中
	
	
	LOG_INFO("%s: player[%lu] state[%d]", __FUNCTION__,
		player->get_uuid(),	player->data->pvp_raid_data.state);
	
	switch (player->data->pvp_raid_data.state)
	{
		case pvp_match_state_not_ready_3:
		{
			uint64_t player_id = player->get_uuid();
			player->data->pvp_raid_data.state = pvp_match_state_ready_3;

			std::map<uint64_t, struct matched_team_3 *>::iterator ite = pvp_map_team_3.find(player->data->pvp_raid_data.matched_index);
			assert(ite != pvp_map_team_3.end());
			struct matched_team_3 *team = ite->second;

				// 检查其他人是否都准备好了
			if (check_team_all_ready_3(team))
			{
				start_pvp_raid_3(team);
				pvp_map_team_3.erase(ite);
				free(team);
				return (0);;
			}

				// 发送MSG_ID_PVP_MATCH_READY_NOTIFY
			for (int i = 0; i < PVP_MATCH_PLAYER_NUM_3; ++i)
			{
				if (team->team1[i] != player_id)
					send_ready_notify(team->team1[i], player_id);
				if (team->team2[i] != player_id)
					send_ready_notify(team->team2[i], player_id);
			}
		}
		break;
		case pvp_match_state_not_ready_5:
		{
			uint64_t player_id = player->get_uuid();
			player->data->pvp_raid_data.state = pvp_match_state_ready_5;

			std::map<uint64_t, struct matched_team_5 *>::iterator ite = pvp_map_team_5.find(player->data->pvp_raid_data.matched_index);
			assert(ite != pvp_map_team_5.end());
			struct matched_team_5 *team = ite->second;

				// TODO: 检查其他人是否都准备好了
			if (check_team_all_ready_5(team))
			{
				start_pvp_raid_5(team);
				return (0);;
			}

				// 发送MSG_ID_PVP_MATCH_READY_NOTIFY
			for (int i = 0; i < 5; ++i)
			{
				if (team->team1[i] == player_id)
					continue;
				if (team->team2[i] == player_id)
					continue;
				send_ready_notify(team->team1[i], player_id);
				send_ready_notify(team->team2[i], player_id);
			}
		}
		break;
		default:
		{
			LOG_ERR("%s: player[%lu] in state[%d]", __FUNCTION__, player->get_uuid(), player->data->pvp_raid_data.state);
		}
		return -1;
	}
	return (0);
}

static void	cancel_match_3(uint64_t id, player_struct *player)
{
	LOG_DEBUG("%s: player[%lu] id[%lu]", __FUNCTION__, player->get_uuid(), id);
	std::map<uint64_t, struct matched_team_3 *>::iterator ite = pvp_map_team_3.find(id);
	assert(ite != pvp_map_team_3.end());
	struct matched_team_3 *team = ite->second;
	pvp_map_team_3.erase(ite);

	uint64_t player_id = player->get_uuid();

	if (player->m_team)
	{
		uint64_t *no_player_team = NULL;
		uint64_t *player_team = NULL;
		for (int i = 0; i < PVP_MATCH_PLAYER_NUM_3; ++i)
		{
			if (team->team1[i] == player_id)
			{
				no_player_team = team->team2;
				player_team = team->team1;
				break;
			}
			if (team->team2[i] == player_id)
			{
				no_player_team = team->team1;
				player_team = team->team2;
				break;
			}
		}
		assert(no_player_team);

		player_struct *t_player = player_manager::get_player_by_id(no_player_team[0]);
		assert(t_player);
		pvp_match_add_team_to_waiting(t_player, PVP_TYPE_DEFINE_3);

		for (int i = 0; i < PVP_MATCH_PLAYER_NUM_3; ++i)
		{
				// 发送MSG_ID_PVP_MATCH_CANCEL_NOTIFY
			player_struct *t_player = player_manager::get_player_by_id(no_player_team[i]);
			if (t_player)
			{
				send_cancel_notify(team->team1[i], player_id);
			}

			t_player = player_manager::get_player_by_id(player_team[i]);
			if (t_player)
			{
				send_cancel_notify(team->team2[i], player_id);
			}
		}
	}
	else
	{
		player_struct *t_player;
		for (int i = 0; i < PVP_MATCH_PLAYER_NUM_3; ++i)
		{
			t_player = player_manager::get_player_by_id(team->team1[i]);
			if (t_player)
			{
				send_cancel_notify(team->team1[i], player_id);
				if (team->team1[i] != player_id)
				{
						// 重新加回等待队列, 发送MSG_ID_PVP_MATCH_CANCEL_NOTIFY
					pvp_match_add_player_to_waiting(t_player, PVP_TYPE_DEFINE_3);
				}
			}

			t_player = player_manager::get_player_by_id(team->team2[i]);
			if (t_player)
			{
				send_cancel_notify(team->team2[i], player_id);
				if (team->team2[i] != player_id)
				{
					pvp_match_add_player_to_waiting(t_player, PVP_TYPE_DEFINE_3);
				}
			}
		}
	}
	free_team_3_ai_player(team);
	free(team);
}

static void	cancel_match_5(uint64_t id, player_struct *player)
{
	LOG_DEBUG("%s: player[%lu] id[%lu]", __FUNCTION__, player->get_uuid(), id);
	std::map<uint64_t, struct matched_team_5 *>::iterator ite = pvp_map_team_5.find(id);
	assert(ite != pvp_map_team_5.end());
	struct matched_team_5 *team = ite->second;
	pvp_map_team_5.erase(ite);

	uint64_t player_id = player->get_uuid();

	if (player->m_team)
	{
		uint64_t *no_player_team = NULL;
		uint64_t *player_team = NULL;
		for (int i = 0; i < 5; ++i)
		{
			if (team->team1[i] == player_id)
			{
				no_player_team = team->team2;
				player_team = team->team1;
				break;
			}
			if (team->team2[i] == player_id)
			{
				no_player_team = team->team1;
				player_team = team->team2;
				break;
			}
		}
		assert(no_player_team);

		player_struct *t_player = player_manager::get_player_by_id(no_player_team[0]);
		assert(t_player);
		pvp_match_add_team_to_waiting(t_player, PVP_TYPE_DEFINE_5);

		for (int i = 0; i < 5; ++i)
		{
				// 发送MSG_ID_PVP_MATCH_CANCEL_NOTIFY
			player_struct *t_player = player_manager::get_player_by_id(no_player_team[i]);
			if (t_player)
			{
				send_cancel_notify(team->team1[i], player_id);
			}

			t_player = player_manager::get_player_by_id(player_team[i]);
			if (t_player)
			{
				send_cancel_notify(team->team2[i], player_id);
			}
		}
	}
	else
	{
		for (int i = 0; i < 5; ++i)
		{
			if (team->team1[i] == player_id)
				continue;
			if (team->team2[i] == player_id)
				continue;

				// 重新加回等待队列, 发送MSG_ID_PVP_MATCH_CANCEL_NOTIFY
			player_struct *t_player = player_manager::get_player_by_id(team->team1[i]);
			if (t_player)
			{
				pvp_match_add_player_to_waiting(t_player, PVP_TYPE_DEFINE_5);
				send_cancel_notify(team->team1[i], player_id);
			}
			t_player = player_manager::get_player_by_id(team->team2[i]);
			if (t_player)
			{
				pvp_match_add_player_to_waiting(t_player, PVP_TYPE_DEFINE_5);
				send_cancel_notify(team->team2[i], player_id);
			}
		}
	}
	free(team);
}

int	pvp_match_player_cancel(player_struct *player)
{
	LOG_INFO("%s: player[%lu] state[%d]", __FUNCTION__,
		player->get_uuid(),	player->data->pvp_raid_data.state);
	
	switch (player->data->pvp_raid_data.state)
	{
		case pvp_match_state_ready_3:
			break;
		case pvp_match_state_not_ready_3:
			cancel_match_3(player->data->pvp_raid_data.matched_index, player);
			break;
		case pvp_match_state_ready_5:
			break;
		case pvp_match_state_not_ready_5:
			cancel_match_5(player->data->pvp_raid_data.matched_index, player);
			break;
		case pvp_match_state_start_3:
		case pvp_match_state_start_5:
			break;
		case pvp_match_state_waiting_3:
			if (player->m_team)
				pvp_waiting_team_3.erase(player->m_team->GetId());
			pvp_waiting_player_3.erase(player->get_uuid());
			break;
		case pvp_match_state_waiting_5:
			if (player->m_team)
				pvp_waiting_team_3.erase(player->m_team->GetId());
			pvp_waiting_player_3.erase(player->get_uuid());
			break;
		default:
			break;
	}

	player->data->pvp_raid_data.state = pvp_match_state_out;
	player->pvp_raid_cancel_time = time_helper::get_cached_time() / 1000;
	return (0);
}

// int pvp_raid_get_player_index(raid_struct *raid, player_struct *player)
// {
//	for (int i = 0; i < MAX_TEAM_MEM; ++i)
//	{
//		if (raid->m_player[i] && raid->m_player[i] == player)
//		{
//			return i;
//		}
//		if (raid->m_player2[i] && raid->m_player2[i] == player)
//		{
//			return i + MAX_TEAM_MEM;
//		}
//	}
//	assert(0);
//	return -1;
// }
// int pvp_raid_get_player_index2(raid_struct *raid, uint64_t player_id)
// {
//	for (int i = 0; i < MAX_TEAM_MEM; ++i)
//	{
//		if (raid->m_player[i] && raid->m_player[i]->get_uuid() == player_id)
//		{
//			return i;
//		}
//		if (raid->m_player2[i] && raid->m_player2[i]->get_uuid() == player_id)
//		{
//			return i + MAX_TEAM_MEM;
//		}
//	}
//	assert(0);
//	return -1;
// }

int pvp_match_player_praise(player_struct *player, uint64_t target_id)
{
	raid_struct *raid = player->get_raid();
	if (!raid || !raid->m_config)
	{
		LOG_ERR("%s: player[%lu] not in raid", __FUNCTION__, player->get_uuid());
		return (-1);
	}

//	int index = pvp_raid_get_player_index2(raid, target_id);
//	if (index < 0)
	int target_index;
	if (raid->get_raid_player_info(target_id, &target_index) == NULL)
	{
		LOG_ERR("%s: player[%lu] praise wrong player_id[%lu]", __FUNCTION__, player->get_uuid(), target_id);
		return (-5);
	}

//	int player_index = pvp_raid_get_player_index(raid, player);
//	if (player_index < 0)
	int player_index;
	if (raid->get_raid_player_info(player->get_uuid(), &player_index) == NULL)
	{
		LOG_ERR("%s: player[%lu] index wrong, bug?", __FUNCTION__, player->get_uuid());
		return (-6);
	}
	// if (index == player_index)
	// {
	//	LOG_ERR("%s: player[%lu] can not praise self", __FUNCTION__, player->get_uuid());
	//	return (-7);
	// }

	assert(target_index < MAX_TEAM_MEM * 2);
	assert(player_index < MAX_TEAM_MEM * 2);

	if (raid->PVP_DATA.praise_index[player_index].praise[target_index])
	{
		LOG_ERR("%s: player[%lu] already praise index %d", __FUNCTION__, player->get_uuid(), raid->PVP_DATA.praise_index[player_index].praise[target_index]);
		return (-8);
	}
	raid->PVP_DATA.praise_index[player_index].praise[target_index] = true;

	//++target_index;

	// if (raid->PVP_DATA.praise_index[player_index] != 0)
	// {
	// 	LOG_ERR("%s: player[%lu] already praise index %d", __FUNCTION__, player->get_uuid(), raid->PVP_DATA.praise_index[player_index]);
	// 	return (-8);
	// }

	//raid->PVP_DATA.praise_index[player_index] = target_index;

	PvpRaidPraiseNotify nty;
	pvp_raid_praise_notify__init(&nty);
	nty.player_id = player->data->player_id;
	nty.target_id = target_id;

	for (int i = 0; i < MAX_TEAM_MEM * 2; ++i)
	{
		if (raid->PVP_DATA.praise_index[i].praise[target_index])
//		if (raid->PVP_DATA.praise_index[i] == target_index)
			++nty.praise_num;
	}
	raid->broadcast_to_raid(MSG_ID_PVP_RAID_PRAISE_NOTIFY, &nty, (pack_func)pvp_raid_praise_notify__pack, true);
	return (0);
}

static void match_success(std::map<uint64_t, uint8_t> *waiting_player,	std::vector<std::map<uint64_t, uint8_t>::iterator> *team1,
	std::vector<std::map<uint64_t, uint8_t>::iterator> *team2)
{
	for (std::vector<std::map<uint64_t, uint8_t>::iterator>::iterator t = team1->begin(); t != team1->end(); ++t)
		waiting_player->erase(*t);
	for (std::vector<std::map<uint64_t, uint8_t>::iterator>::iterator t = team2->begin(); t != team2->end(); ++t)
		waiting_player->erase(*t);
}

typedef std::multimap<uint8_t, uint64_t>::iterator sort_ite;
// struct match_player
// {
//	sort_ite player;
//	uint8_t level;
// };
//返回0表示成功, level1段位，level2等级
//从单人匹配队列里面找到匹配的队伍
static int match_single_team_3(std::multimap<uint8_t, uint64_t> *sort_map, int level1, int level2, sort_ite first,
	sort_ite match_player[PVP_MATCH_PLAYER_NUM_3], sort_ite *next)
{
	uint8_t index = 0;
	uint32_t leader_level; //队长段位

	if (first == sort_map->end())
		return (-1);

	for (sort_ite ite = first; ite != sort_map->end(); ++ite)
	{
		if (index == 0)
		{
			if (level1 > 0)
			{
				assert(level2 > 0);
				if (ite->first < level1 + TEAM_LEVEL1_DIFF_L)
					continue;
				if (ite->first > level1 + TEAM_LEVEL1_DIFF_R)
					continue;
				player_struct *player = player_manager::get_player_by_id(ite->second);
				assert(player);
				int32_t lv = player->get_attr(PLAYER_ATTR_LEVEL);
				if (lv < level2 + TEAM_LEVEL2_DIFF_L)
					continue;
				if (lv > level2 + TEAM_LEVEL2_DIFF_R)
					continue;
			}
			leader_level = ite->first;
		}
		else
		{
			if (ite->first > leader_level + MATCH_LEVEL_DIFF)
				continue;
		}

		match_player[index] = ite;
		++index;

		if (index == PVP_MATCH_PLAYER_NUM_3)
		{
			*next = (++ite);
			return (0);
		}
	}

		//换个队长
	sort_ite new_first = ++match_player[0];
	return match_single_team_3(sort_map, level1, level2, new_first, match_player, next);
}

// TODO:
static int add_single_player_to_pvp_team()
{
	return (-1);
}

//从组队匹配队列里面找到合适的队伍
static int match_team_team_3(std::multimap<uint8_t, uint64_t> *sort_map, int level1, int level2, sort_ite first,
	uint64_t match_player[PVP_MATCH_PLAYER_NUM_3], sort_ite *next, uint64_t *team_id)
{
	if (first == sort_map->end())
		return (-1);

	for (sort_ite ite = first; ite != sort_map->end(); ++ite)
	{
		if (level1 > 0)
		{
			assert(level2 > 0);
			if (ite->first < level1 - TEAM_LEVEL1_DIFF_L)
				continue;
			if (ite->first > level1 + TEAM_LEVEL1_DIFF_R)
				continue;
		}
		Team *team = Team::GetTeam(ite->second);
		assert(team);
		assert(team->GetMemberSize() <= 3);
		if (team->GetMemberSize() != 3)
		{
			int ret = add_single_player_to_pvp_team();
			if (ret == 0)
			{
					// TODO:
				return (0);
			}
			continue;
		}
		for (int i = 0; i < team->m_data->m_memSize; ++i)
		{
			match_player[i] = team->m_data->m_mem[i].id;
		}
		*next = (++ite);
		return (0);
	}
	return (-1);
}

static void send_match_success_notify_3(struct matched_team_3 *team)
{
	PvpMatchPlayerInfo info[PVP_MATCH_PLAYER_NUM_3 * 2];
	PvpMatchPlayerInfo *info_p[PVP_MATCH_PLAYER_NUM_3 * 2];
	PvpMatchSuccessNotify nty;
	pvp_match_success_notify__init(&nty);
	nty.n_team1_player_info = PVP_MATCH_PLAYER_NUM_3;
	nty.n_team2_player_info = PVP_MATCH_PLAYER_NUM_3;
	nty.team1_player_info = &info_p[0];
	nty.team2_player_info = &info_p[PVP_MATCH_PLAYER_NUM_3];

	for (int i = 0; i < PVP_MATCH_PLAYER_NUM_3; ++i)
	{
		info_p[i] = &info[i];
		pvp_match_player_info__init(&info[i]);
		player_struct *player = player_manager::get_player_by_id(team->team1[i]);
		assert(player);
		info[i].head_icon = player->get_attr(PLAYER_ATTR_HEAD);
		info[i].job = player->get_attr(PLAYER_ATTR_JOB);
		info[i].name = player->get_name();
		info[i].player_id = player->get_uuid();
		info[i].lv = player->get_attr(PLAYER_ATTR_LEVEL);
	}

	for (int i = PVP_MATCH_PLAYER_NUM_3; i < PVP_MATCH_PLAYER_NUM_3 * 2; ++i)
	{
		info_p[i] = &info[i];
		pvp_match_player_info__init(&info[i]);
		player_struct *player = player_manager::get_player_by_id(team->team2[i-PVP_MATCH_PLAYER_NUM_3]);
		assert(player);
		info[i].head_icon = player->get_attr(PLAYER_ATTR_HEAD);
		info[i].job = player->get_attr(PLAYER_ATTR_JOB);
		info[i].name = player->get_name();
		info[i].player_id = player->get_uuid();
		info[i].lv = player->get_attr(PLAYER_ATTR_LEVEL);
	}

	for (int i = 0; i < PVP_MATCH_PLAYER_NUM_3; ++i)
	{
		EXTERN_DATA ext;

		ext.player_id = team->team1[i];
		if (get_entity_type(ext.player_id) == ENTITY_TYPE_PLAYER)
			fast_send_msg(&conn_node_gamesrv::connecter, &ext,
				MSG_ID_PVP_MATCH_SUCCESS_NOTIFY, pvp_match_success_notify__pack, nty);

		ext.player_id = team->team2[i];
		if (get_entity_type(ext.player_id) == ENTITY_TYPE_PLAYER)
			fast_send_msg(&conn_node_gamesrv::connecter, &ext,
				MSG_ID_PVP_MATCH_SUCCESS_NOTIFY, pvp_match_success_notify__pack, nty);
	}
}

static void try_match_team_3()
{
	uint64_t match_player_1[PVP_MATCH_PLAYER_NUM_3];
	uint64_t match_player_2[PVP_MATCH_PLAYER_NUM_3];
	uint64_t match_team_id[2];

	std::multimap<uint8_t, uint64_t> sort_map;
	for (std::map<uint64_t, uint8_t>::iterator ite = pvp_waiting_team_3.begin(); ite != pvp_waiting_team_3.end(); ++ite)
		sort_map.insert(std::make_pair(ite->second, ite->first));

	sort_ite first = sort_map.begin();
	sort_ite next;
	for (;;)
	{
		if (match_team_team_3(&sort_map, -1, -1, first, match_player_1, &next, &match_team_id[0]) != 0)
			return;
		player_struct *player = player_manager::get_player_by_id(match_player_1[0]);
		assert(player);
		if (match_team_team_3(&sort_map, player->data->pvp_raid_data.level_3,
				player->get_attr(PLAYER_ATTR_LEVEL), next, match_player_2, &next, &match_team_id[1]) != 0)
			return;
		first = next;
			//匹配成功
		struct matched_team_3 *team = (struct matched_team_3 *)malloc(sizeof(struct matched_team_3));
		pvp_map_team_3[pvp_matched_index] = team;
		for (int i = 0; i < PVP_MATCH_PLAYER_NUM_3; ++i)
		{
			// 加入匹配成功队列
			team->team1[i] = match_player_1[i];
			team->team2[i] = match_player_2[i];
			// 从等待队列移走
			pvp_waiting_team_3.erase(match_team_id[0]);
			pvp_waiting_team_3.erase(match_team_id[1]);
			// 修改状态
			player = player_manager::get_player_by_id(team->team1[i]);
			assert(player);
			player->data->pvp_raid_data.matched_index = pvp_matched_index;
			player->data->pvp_raid_data.state = pvp_match_state_not_ready_3;
		}
			// 发送MSG_ID_PVP_MATCH_SUCCESS_NOTIFY
		send_match_success_notify_3(team);
		++pvp_matched_index;
	}
}

static void try_match_3()//std::map<uint64_t, uint8_t> *waiting_player)
{
	sort_ite match_player_1[PVP_MATCH_PLAYER_NUM_3];
	sort_ite match_player_2[PVP_MATCH_PLAYER_NUM_3];

	std::multimap<uint8_t, uint64_t> sort_map;
	for (std::map<uint64_t, uint8_t>::iterator ite = pvp_waiting_player_3.begin(); ite != pvp_waiting_player_3.end(); ++ite)
		sort_map.insert(std::make_pair(ite->second, ite->first));

	sort_ite first = sort_map.begin();
	sort_ite next;
	for (;;)
	{
		if (match_single_team_3(&sort_map, -1, -1, first, match_player_1, &next) != 0)
			return;
		player_struct *player = player_manager::get_player_by_id(match_player_1[0]->second);
		assert(player);
		if (match_single_team_3(&sort_map, match_player_1[0]->first, player->get_attr(PLAYER_ATTR_LEVEL), next, match_player_2, &next) != 0)
			return;
		first = next;
			//匹配成功
		struct matched_team_3 *team = (struct matched_team_3 *)malloc(sizeof(struct matched_team_3));
		pvp_map_team_3[pvp_matched_index] = team;
		for (int i = 0; i < PVP_MATCH_PLAYER_NUM_3; ++i)
		{
			// 加入匹配成功队列
			team->team1[i] = match_player_1[i]->second;
			team->team2[i] = match_player_2[i]->second;
			// 从等待队列移走
			pvp_waiting_player_3.erase(match_player_1[i]->second);
			pvp_waiting_player_3.erase(match_player_2[i]->second);
			// 修改状态
			player = player_manager::get_player_by_id(team->team1[i]);
			assert(player);
			player->data->pvp_raid_data.matched_index = pvp_matched_index;
			player->data->pvp_raid_data.state = pvp_match_state_not_ready_3;

			player = player_manager::get_player_by_id(team->team2[i]);
			assert(player);
			player->data->pvp_raid_data.matched_index = pvp_matched_index;
			player->data->pvp_raid_data.state = pvp_match_state_not_ready_3;
		}
			// 发送MSG_ID_PVP_MATCH_SUCCESS_NOTIFY
		send_match_success_notify_3(team);
		++pvp_matched_index;
	}
}

//同一组相差2级以内，两组一共相差三级以内
//如果差2级以内，优先放入1组，如果差三级，放入2组
//如果最终不能满足匹配, 把第一组人的等级放入忽略列表
//返回true继续匹配，返回false不能再匹配
__attribute_used__ static bool try_match_notuse(std::map<uint64_t, uint8_t> *waiting_player, uint8_t num, uint64_t ignore_bits)
{
	assert(num == 3 || num == 5);
//	std::map<uint64_t, uint8_t>::iterator team1[num];
//	std::map<uint64_t, uint8_t>::iterator team2[num];

	std::vector<std::map<uint64_t, uint8_t>::iterator> team1, team2;

	uint8_t team1_min_level = 0, team1_max_level = 0;
	uint8_t team2_min_level = 0, team2_max_level = 0;

	std::map<uint64_t, uint8_t>::iterator ite;
	int left = waiting_player->size();
	int max_left = num * 2;
	for (ite = waiting_player->begin(); ite != waiting_player->end(); ++ite, --left)
	{
		uint8_t level = ite->second;
		if ((1 << level) & ignore_bits)
			continue;
		team1.push_back(ite);
		team1_min_level = level - 2;
		team1_max_level = level + 2;

		team2_min_level = level - 3;
		team2_max_level = level + 3;
		break;
	}
	if (left < max_left)
		return false;

	for (++ite; ite != waiting_player->end() && left >= max_left; ++ite)
	{
		uint8_t level = ite->second;
		if (team1.size() < num)
		{
			if (level >= team1_min_level && level <= team1_max_level)
			{
				team1.push_back(ite);
				if (team1.size() == num && team2.size() == num)
				{
					match_success(waiting_player, &team1, &team2);
					return true;
				}
				if (level - 2 > team1_min_level)
					team1_min_level = level - 2;
				if (level + 2 < team1_max_level)
					team1_max_level = level + 2;
				if (level - 3 > team2_min_level)
					team2_min_level = level - 3;
				if (level + 3 < team2_max_level)
					team2_max_level = level + 3;

				continue;
			}
		}

		if (team2.size() < num)
		{
			if (level >= team2_min_level && level <= team2_max_level)
			{
				team2.push_back(ite);
				if (team1.size() == num && team2.size() == num)
				{
					match_success(waiting_player, &team1, &team2);
					return true;
				}
				if (level - 2 > team2_min_level)
					team2_min_level = level - 2;
				if (level + 2 < team2_max_level)
					team2_max_level = level + 2;

				continue;
			}
		}

			//两个队伍都加不进去
		--left;
	}

	for (std::vector<std::map<uint64_t, uint8_t>::iterator>::iterator t = team1.begin(); t != team1.end(); ++t)
	{
		uint8_t level = ite->second;
		ignore_bits |= (1 << level);
	}
	return true;
}


int pvp_match_single_ai_player_3(player_struct *player)
{
		//匹配成功
	struct matched_team_3 *team = (struct matched_team_3 *)malloc(sizeof(struct matched_team_3));
	pvp_map_team_3[pvp_matched_index] = team;

		// 加入匹配成功队列
	team->team1[0] = player->get_uuid();
		// 修改状态
	player->data->pvp_raid_data.matched_index = pvp_matched_index;
	player->data->pvp_raid_data.state = pvp_match_state_not_ready_3;

	int name_index = random();
	player_struct *ai_player;
	for (int i = 1; i < PVP_MATCH_PLAYER_NUM_3; ++i)
	{
		ai_player = player_manager::create_ai_player(player, NULL, name_index, 1);
		name_index ++;
		team->team1[i] = ai_player->get_uuid();

		if (ai_player->ai_data)
			ai_player->ai_data->patrol_index = UINT8_MAX - 1;
		ai_player->data->pvp_raid_data.matched_index = pvp_matched_index;
		ai_player->data->pvp_raid_data.state = pvp_match_state_ready_3;
	}
	for (int i = 0; i < PVP_MATCH_PLAYER_NUM_3; ++i)
	{
		ai_player = player_manager::create_ai_player(player, NULL, name_index, 1);
		name_index ++;
		team->team2[i] = ai_player->get_uuid();

		if (ai_player->ai_data)
			ai_player->ai_data->patrol_index = UINT8_MAX - 1;		
		ai_player->data->pvp_raid_data.matched_index = pvp_matched_index;
		ai_player->data->pvp_raid_data.state = pvp_match_state_ready_3;
	}

		// 发送MSG_ID_PVP_MATCH_SUCCESS_NOTIFY
	send_match_success_notify_3(team);

	for (int i = 1; i < PVP_MATCH_PLAYER_NUM_3; ++i)
	{
		send_ready_notify(player->get_uuid(), team->team1[i]);
	}
	for (int i = 0; i < PVP_MATCH_PLAYER_NUM_3; ++i)
	{
		send_ready_notify(player->get_uuid(), team->team2[i]);
	}

	++pvp_matched_index;
	return (0);
}

void pvp_match_manager_on_tick()
{
	try_match_team_3();
	try_match_3();//&pvp_waiting_player_3);
}
