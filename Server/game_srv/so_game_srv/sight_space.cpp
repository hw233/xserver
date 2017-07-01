#include "sight_space.h"
#include "monster_manager.h"
#include "msgid.h"

sight_space_struct::sight_space_struct()
{
	for (int i = 0; i < MAX_PLAYER_IN_SIGHT_SPACE; ++i)
		players[i] = NULL;
	for (int i = 0; i < MAX_MONSTER_IN_SIGHT_SPACE; ++i)
		monsters[i] = NULL;

	LOG_DEBUG("%s %p", __FUNCTION__, this);
}

sight_space_struct::~sight_space_struct()
{
	for (int i = 0; i < MAX_PLAYER_IN_SIGHT_SPACE; ++i)
		assert(players[i] == NULL);
	for (int i = 0; i < MAX_MONSTER_IN_SIGHT_SPACE; ++i)
	{
		if (monsters[i] == NULL)
			continue;
		assert(monsters[i]->sight_space == this);
		monsters[i]->sight_space = NULL;
		monster_manager::delete_monster(monsters[i]);
	}
	LOG_DEBUG("%s %p", __FUNCTION__, this);	
}

int sight_space_struct::broadcast_player_delete(player_struct *player)
{
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

