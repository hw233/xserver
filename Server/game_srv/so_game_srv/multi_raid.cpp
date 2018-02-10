#include "multi_raid.h"

int multi_raid_struct::init_raid(player_struct *player)
{
	m_players.clear();
	return raid_struct::init_raid(player);
}

int multi_raid_struct::add_player_to_scene(player_struct *player)
{
	int ret = scene_struct::add_player_to_scene(player);
	if (ret != 0)
	{
		return ret;
	}

	m_players.insert(std::make_pair(player->get_uuid(), player));

	return 0;
}

int multi_raid_struct::delete_player_from_scene(player_struct *player)
{
	int ret = scene_struct::delete_player_from_scene(player);
	if (ret != 0)
	{
		return ret;
	}

	m_players.erase(player->get_uuid());

	return 0;
}

int multi_raid_struct::broadcast_to_raid(uint32_t msg_id, void *msg_data, pack_func func, bool include_not_ready)
{
	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(msg_id, msg_data, func);
	PROTO_HEAD_CONN_BROADCAST *head;
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;

	for (std::map<uint64_t, player_struct*>::iterator iter = m_players.begin(); iter != m_players.end(); ++iter)
	{
		broadcast_msg_add_players(iter->second, ppp, include_not_ready);		
//		conn_node_gamesrv::broadcast_msg_add_players(iter->first, ppp);					
	}

	if (head->num_player_id > 0)
	{
//		head->len += sizeof(uint64_t) * head->num_player_id;
		return conn_node_gamesrv::broadcast_msg_send();
	}
	return (0);
}

int multi_raid_struct::set_m_player_and_player_info(player_struct *player, int index)
{
	return (0);
}

int multi_raid_struct::clear_m_player_and_player_info(player_struct *player, bool clear_player_info)
{
	return (0);
}

bool multi_raid_struct::use_m_player()
{
	return false;
}


