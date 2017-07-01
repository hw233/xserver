#include "sight_space_manager.h"
#include "msgid.h"
#include "../proto/raid.pb-c.h"
#include "cash_truck_manager.h"

// comm_pool sight_space_manager::sight_space_manager_sight_space_data_pool;
// std::vector<sight_space_struct *> sight_space_manager::sight_space_manager_mark_delete_sight_space;

static void player_enter_sight_space(sight_space_struct *sight_space, player_struct *player)
{
	LOG_DEBUG("%s: player[%lu] enter sightspace %p", __FUNCTION__, player->get_uuid(), sight_space);
	
	player->sight_space = sight_space;
//	struct position *pos = player->get_pos();
//	sight_space->players_prev_pos[0].pos_x = pos->pos_x;
//	sight_space->players_prev_pos[0].pos_z = pos->pos_z;	
		//玩家离开视野
	scene_struct *scene = player->scene;
	if (scene)
	{
		player->send_clear_sight();
		bool addTruck = false; 
		cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(player->data->truck.truck_id);
		if (truck != NULL)
		{
			if (player->data->truck.on_truck)
			{
				truck->sight_space = sight_space;
				scene->delete_cash_truck_from_scene(truck);
				truck->scene = scene;
				addTruck = true;
			}
		}
		
		scene->delete_player_from_scene(player);
		player->scene = scene; //之前delete的时候会清空这个指针，这里需要恢复
		if (addTruck)
		{
			player->add_truck_to_sight(truck->get_uuid());
		}
	}

	EnterPlanesRaid send;
	enter_planes_raid__init(&send);
	send.type = sight_space->data->type;
	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ENTER_PLANES_RAID_NOTIFY, enter_planes_raid__pack, send);
}

int sight_space_manager::init_sight_space(int num, unsigned long key)
{
	return init_comm_pool(0, sizeof(sight_space_data), num, key, &sight_space_manager_sight_space_data_pool);
}

void sight_space_manager::on_tick()
{
	for (std::vector<sight_space_struct *>::iterator ite = sight_space_manager_mark_delete_sight_space.begin(); ite != sight_space_manager_mark_delete_sight_space.end(); ++ite)
	{
		sight_space_struct *sight_space = (*ite);
		for (int i = 0; i < MAX_PLAYER_IN_SIGHT_SPACE; ++i)
		{
			if (sight_space->players[i])
			{
				player_struct *player = sight_space->players[i];
				assert(player->sight_space == sight_space);
				player->sight_space = NULL;
				sight_space->players[i] = NULL;
				sight_space->data->player_id[i] = 0;

				EXTERN_DATA extern_data;
				extern_data.player_id = player->get_uuid();	
				fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_LEAVE_PLANES_RAID_NOTIFY, 0, 0);

				player->send_clear_sight();
				player->clear_player_sight();
				if (player->scene)
				{
					player->scene->add_player_to_scene(player);
					cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(player->data->truck.truck_id);
					if (truck != NULL && truck->sight_space != NULL)
					{
						truck->clear_cash_truck_sight();
						truck->scene->add_cash_truck_to_scene(truck);
						truck->sight_space = NULL;
						truck->sight_space = NULL;						
					}
				}
					
			}
		}
		comm_pool_free(&sight_space_manager_sight_space_data_pool, sight_space->data);		
		delete sight_space;
	}
	sight_space_manager_mark_delete_sight_space.clear();
}

sight_space_struct *sight_space_manager::create_sight_space(player_struct *player, int type)
{
	assert(player->sight_space == NULL);

	struct sight_space_data *data = NULL;
	data = (sight_space_data *)comm_pool_alloc(&sight_space_manager_sight_space_data_pool);
	if (!data)
		return NULL;
	memset(data, 0, sizeof(sight_space_data));
	
	sight_space_struct *ret = new(std::nothrow) sight_space_struct;
	if (!ret)
	{
		LOG_ERR("%s %d: malloc failed", __FUNCTION__, __LINE__);
		comm_pool_free(&sight_space_manager_sight_space_data_pool, data);
		return NULL;
	}
	ret->data = data;
	ret->data->type = type;
	
	ret->players[0] = player;
	ret->data->player_id[0] = player->get_uuid();
	player_enter_sight_space(ret, player);
	return ret;
}

// int sight_space_manager::add_player_to_sight_space(sight_space_struct *sight_space, player_struct *player)
// {
// 	assert(player->sight_space == NULL);
// 	for (int i = 0; i < MAX_PLAYER_IN_SIGHT_SPACE; ++i)
// 	{
// 		if (!sight_space->players[i])
// 		{
// 			sight_space->players[i] = player;
// 			sight_space->data->player_id[i] = player->get_uuid();
// 			player_enter_sight_space(sight_space, player);
// 			return (0);
// 		}
// 	}
// 	return (-1);
// }
int sight_space_manager::del_player_from_sight_space(sight_space_struct *sight_space, player_struct *player, bool enter_scene)
{
	assert(player->sight_space == sight_space);
	player->sight_space = NULL;
	LOG_DEBUG("%s: player[%lu] leave sightspace %p", __FUNCTION__, player->get_uuid(), sight_space);	
	bool empty = true;

		//已经在删除队列了，就不用管了
	if (sight_space->data->mark_delete)
	{
		empty = false;
	}
	else
	{
		for (int i = 0; i < MAX_PLAYER_IN_SIGHT_SPACE; ++i)
		{
			if (sight_space->players[i] == player)
			{
				sight_space->players[i] = NULL;
				sight_space->data->player_id[i] = 0;
			}
			else if (sight_space->players[i])
			{
				empty = false;
			}
		}
	}

	if (empty)
	{
		comm_pool_free(&sight_space_manager_sight_space_data_pool, sight_space->data);		
		delete sight_space;
	}
	else
	{
		sight_space->broadcast_player_delete(player);
	}

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
			cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(player->data->truck.truck_id);
			if (truck != NULL && truck->sight_space != NULL)
			{
				truck->clear_cash_truck_sight();
				truck->scene->add_cash_truck_to_scene(truck);
				truck->sight_space = NULL;
			}
			
		}
			
	}
	
	return (0);
}

void sight_space_manager::mark_sight_space_delete(sight_space_struct *sight_space)
{
	if (sight_space->data->mark_delete)
		return;
	sight_space_manager_mark_delete_sight_space.push_back(sight_space);
	sight_space->data->mark_delete = true;
}

int sight_space_manager::get_sight_space_num()
{
	return get_inuse_comm_pool_num(&sight_space_manager_sight_space_data_pool);
}
int sight_space_manager::get_sight_space_pool_max_num()
{
	return sight_space_manager_sight_space_data_pool.num;
}
