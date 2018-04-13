#include "sight_space.h"
#include "collect.h"
#include "monster_manager.h"
#include "cash_truck_manager.h"
#include "time_helper.h"
#include "msgid.h"

sight_space_struct::sight_space_struct()
{
	for (int i = 0; i < MAX_PLAYER_IN_SIGHT_SPACE; ++i)
	{
		players[i] = NULL;
		trucks[i] = NULL;
	}
	for (int i = 0; i < MAX_MONSTER_IN_SIGHT_SPACE; ++i)
		monsters[i] = NULL;
	for (int i = 0; i < MAX_PARTNER_IN_SIGHT_SPACE; ++i)	
		partners[i] = NULL;
	for (int i = 0; i < MAX_COLLECT_IN_SIGHT_SPACE; ++i)	
		collects[i] = NULL;
	
	LOG_DEBUG("%s %p", __FUNCTION__, this);
}

sight_space_struct::~sight_space_struct()
{
	for (int i = 0; i < MAX_PLAYER_IN_SIGHT_SPACE; ++i)
	{
		assert(players[i] == NULL);
		assert(trucks[i] == NULL);		
	}
	for (int i = 0; i < MAX_PARTNER_IN_SIGHT_SPACE; ++i)
		assert(partners[i] == NULL);
	
	for (int i = 0; i < MAX_MONSTER_IN_SIGHT_SPACE; ++i)
	{
		if (monsters[i] == NULL)
			continue;
		assert(monsters[i]->sight_space == this);
		monsters[i]->sight_space = NULL;
		monster_manager::delete_monster(monsters[i]);
	}

	for (int i = 0; i < MAX_COLLECT_IN_SIGHT_SPACE; ++i)
	{
		if (collects[i] == NULL)
			continue;
		Collect::DestroyCollect(collects[i]->m_uuid);
		collects[i] = NULL;
	}
	
	LOG_DEBUG("%s %p", __FUNCTION__, this);	
}

int sight_space_struct::broadcast_player_delete(player_struct *player, bool enter_scene)
{
	player->take_truck_out_sight_space(this);
	player->take_partner_out_sight_space(this);
	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();	
	fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_LEAVE_PLANES_RAID_NOTIFY, 0, 0);	

	if (enter_scene)
	{
		player->send_clear_sight();
		player->clear_player_sight();
		if (player->scene)
		{
			player->scene->add_player_to_scene(player);
			player->take_truck_into_scene();			
			
			// cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(player->data->truck.truck_id);
			// if (truck != NULL && truck->sight_space != NULL)
			// {
			// 	truck->clear_cash_truck_sight();
			// 	truck->scene->add_cash_truck_to_scene(truck);
			// 	truck->sight_space = NULL;
			// 	truck->data->fb_time = time_helper::get_cached_time() + truck->truck_config->Interval * 1000;
			// }
			
		}
	}
	//if (data->type == 1)
	//{
	//	fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_XUNBAO_USE_NEXT_NOTIFY, 0, 0);
	//}
	return (0);
}

// int sight_space_struct::broadcast_collect_delete(Collect *collect)
// {
// 	LOG_DEBUG("%s %d: delete sightspace collect %u %lu at %p [%.1f][%.1f]", __FUNCTION__, __LINE__,
// 		collect->m_collectId, collect->m_uuid, this, 
// 		collect->m_pos.pos_x, collect->m_pos.pos_z);
	
// 	for (int i = 0; i < MAX_PLAYER_IN_SIGHT_SPACE; ++i)
// 	{
		
// 	}
// }

int sight_space_struct::broadcast_truck_delete(cash_truck_struct *truck, bool send)
{
	LOG_DEBUG("%s %d: delete sightspace truck %u %lu at %p [%.1f][%.1f]", __FUNCTION__, __LINE__,
		truck->data->monster_id, truck->get_uuid(), this, 
		truck->get_pos()->pos_x, truck->get_pos()->pos_z);
	
	for (int i = 0; i < MAX_PLAYER_IN_SIGHT_SPACE; ++i)
	{
		if (trucks[i] == truck)
		{
			trucks[i] = NULL;
			break;
		}
	}

	truck->sight_space = NULL;
	truck->broadcast_cash_truck_delete(send);
	return (0);
}

int sight_space_struct::broadcast_partner_delete(partner_struct *partner)
{
	LOG_DEBUG("%s %d: delete partner %u %lu at %p [%.1f][%.1f]", __FUNCTION__, __LINE__,
		partner->data->partner_id, partner->get_uuid(), this, 
		partner->get_pos()->pos_x, partner->get_pos()->pos_z);
	
	for (int i = 0; i < MAX_PARTNER_IN_SIGHT_SPACE; ++i)
	{
		if (partners[i] == partner)
		{
			partners[i] = NULL;
			break;
		}
	}
	partner->partner_sight_space = NULL;
	partner->broadcast_partner_delete(true);
	return (0);
}
int sight_space_struct::broadcast_monster_delete(monster_struct *monster)
{
	--data->n_monster_uuid;
	return (0);
}

int sight_space_struct::broadcast_player_create(player_struct *player)
{
	return (0);
}

int sight_space_struct::broadcast_collect_create(Collect *collect)
{
	assert(!collect->area);

	LOG_DEBUG("%s %d: create collect %u %u at %p [%.1f][%.1f]", __FUNCTION__, __LINE__,
		collect->m_collectId, collect->m_uuid, this, 
		collect->m_pos.pos_x, collect->m_pos.pos_z);

	for (int i = 0; i < MAX_PLAYER_IN_SIGHT_SPACE; ++i)
	{
		if (!players[i])
			continue;
		collect->NotifyCollectCreate(players[i]);
	}
	return (0);
}

int sight_space_struct::broadcast_truck_create(cash_truck_struct *truck)
{
	assert(!truck->area);

	LOG_DEBUG("%s %d: create sightspace truck %u %lu at %p [%.1f][%.1f]", __FUNCTION__, __LINE__,
		truck->data->monster_id, truck->get_uuid(), this, 
		truck->get_pos()->pos_x, truck->get_pos()->pos_z);

	for (int i = 0; i < MAX_PLAYER_IN_SIGHT_SPACE; ++i)
	{
		if (!trucks[i])
		{
//			assert(partners[i] == NULL);
//			data->partner_uuid[i] = partner->get_uuid();
			trucks[i] = truck;
			break;
		}
	}
	
	SightChangedNotify notify;
	sight_changed_notify__init(&notify);
	SightCashTruckInfo truck_info[1];
	SightCashTruckInfo *truck_info_point[1];
	truck_info_point[0] = &truck_info[0];
	notify.n_add_cash_truck = 1;
	notify.add_cash_truck = truck_info_point;
	truck->pack_sight_cash_truck_info(truck_info_point[0]);

	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
	PROTO_HEAD_CONN_BROADCAST *head;	
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;	

	truck->add_sight_space_player_to_sight(this, &head->num_player_id, ppp);
	truck->add_sight_space_monster_to_sight(this);
	
	if (head->num_player_id > 0)
	{
		head->len += sizeof(uint64_t) * head->num_player_id;	
		conn_node_gamesrv::broadcast_msg_send();
	}
	return (0);
}

int sight_space_struct::broadcast_partner_create(partner_struct *partner)
{
	assert(!partner->area);

	LOG_DEBUG("%s %d: create partner %u %lu at %p [%.1f][%.1f]", __FUNCTION__, __LINE__,
		partner->data->partner_id, partner->get_uuid(), this, 
		partner->get_pos()->pos_x, partner->get_pos()->pos_z);

	for (int i = 0; i < MAX_PARTNER_IN_SIGHT_SPACE; ++i)
	{
		if (!partners[i])
		{
//			assert(partners[i] == NULL);
//			data->partner_uuid[i] = partner->get_uuid();
			partners[i] = partner;
			break;
		}
	}
	
	SightChangedNotify notify;
	sight_changed_notify__init(&notify);
	SightPartnerInfo partner_info[1];
	SightPartnerInfo *partner_info_point[1];
	partner_info_point[0] = &partner_info[0];
	notify.n_add_partner = 1;
	notify.add_partner = partner_info_point;
	partner->pack_sight_partner_info(partner_info_point[0]);

	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
	PROTO_HEAD_CONN_BROADCAST *head;	
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;	

	partner->add_sight_space_player_to_sight(this, &head->num_player_id, ppp);
	partner->add_sight_space_monster_to_sight(this);
	
	if (head->num_player_id > 0)
	{
		head->len += sizeof(uint64_t) * head->num_player_id;	
		conn_node_gamesrv::broadcast_msg_send();
	}
	return (0);
}

int sight_space_struct::broadcast_monster_create(monster_struct *monster)
{
	assert(!monster->area);

	LOG_DEBUG("%s %d: create monster %u %lu at %p [%.1f][%.1f]", __FUNCTION__, __LINE__,
		monster->data->monster_id, monster->get_uuid(), this, 
		monster->get_pos()->pos_x, monster->get_pos()->pos_z);
	
	SightChangedNotify notify;
	sight_changed_notify__init(&notify);
	SightMonsterInfo monster_info[1];
	SightMonsterInfo *monster_info_point[1];
	monster_info_point[0] = &monster_info[0];
	notify.n_add_monster = 1;
	notify.add_monster = monster_info_point;
	monster->pack_sight_monster_info(monster_info_point[0]);

	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
	PROTO_HEAD_CONN_BROADCAST *head;	
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;	

	monster->add_sight_space_player_to_sight(this, &head->num_player_id, ppp);
	monster->add_sight_space_monster_to_sight(this);
	monster->add_sight_space_partner_to_sight(this);
	monster->add_sight_space_truck_to_sight(this);		
	
	if (head->num_player_id > 0)
	{
		head->len += sizeof(uint64_t) * head->num_player_id;	
		conn_node_gamesrv::broadcast_msg_send();
	}
	++data->n_monster_uuid;
	return (0);
}

bool sight_space_struct::is_task_event_exist(uint64_t event_id)
{
	if (event_id == 0)
	{
		return false;
	}

	if (!data)
	{
		return false;
	}

	for (int i = 0; i < MAX_MONSTER_IN_SIGHT_SPACE; ++i)
	{
		if (data->task_event[i] == event_id)
		{
			return true;
		}
	}

	return false;
}

int sight_space_struct::insert_task_event(uint64_t event_id)
{
	if (event_id == 0)
	{
		return 0;
	}
	
	for (int i = 0; i < MAX_MONSTER_IN_SIGHT_SPACE; ++i)
	{
		if (data->task_event[i] == 0)
		{
			data->task_event[i] = event_id;
			break;
		}
	}

	return 0;
}

void sight_space_struct::stop_monster_ai()
{
	for (int i = 0; i < MAX_MONSTER_IN_SIGHT_SPACE; ++i)
	{
		if (monsters[i] == NULL)
			continue;
		monsters[i]->data->stop_ai = true;
	}
	for (int i = 0; i < MAX_PLAYER_IN_SIGHT_SPACE; ++i)
	{
		if (players[i] == NULL)
			continue;
		players[i]->stop_partner_ai();
	}
}
void sight_space_struct::start_monster_ai()
{
	for (int i = 0; i < MAX_MONSTER_IN_SIGHT_SPACE; ++i)
	{
		if (monsters[i] == NULL)
			continue;
		monsters[i]->data->stop_ai = false;
	}
	for (int i = 0; i < MAX_PLAYER_IN_SIGHT_SPACE; ++i)
	{
		if (players[i] == NULL)
			continue;
		players[i]->start_partner_ai();
	}
}

void sight_space_struct::delete_monster(monster_struct *monster)
{
	if (monster->sight_space != this || !players[0] || !players[0]->data)
		return;
	monster->data->stop_ai = true;
	broadcast_monster_delete(monster);
	SightChangedNotify notify;
	sight_changed_notify__init(&notify);

	//发送给需要在视野里面删除怪物的通知
	notify.n_delete_monster = 1;
	uint64_t player_ids[1];
	player_ids[0] = monster->get_uuid();
	notify.delete_monster = player_ids;

	EXTERN_DATA extern_data;
	extern_data.player_id = players[0]->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_SIGHT_CHANGED_NOTIFY, sight_changed_notify__pack, notify);	
}
void sight_space_struct::delete_collect(Collect *collect)
{
	if (collect->sight_space != this || !players[0] || !players[0]->data)
		return;
	
	SightChangedNotify notify;
	sight_changed_notify__init(&notify);

	//发送给需要在视野里面删除怪物的通知
	notify.n_delete_collect = 1;
	uint32_t collect_ids[1];
	collect_ids[0] = collect->m_uuid;
	notify.delete_collect = collect_ids;

	EXTERN_DATA extern_data;
	extern_data.player_id = players[0]->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_SIGHT_CHANGED_NOTIFY, sight_changed_notify__pack, notify);

//	delete collect;
	Collect::DestroyCollect(collect->m_uuid);	
}

