#include "server_level.h"
#include "conn_node_dbsrv.h"
#include "player_db.pb-c.h"
#include "game_config.h"
#include "time_helper.h"
#include "player.h"
#include "uuid.h"
#include "app_data_statis.h"
#include "msgid.h"
#include "role.pb-c.h"
#include "player_manager.h"

extern uint32_t sg_server_open_time;
extern void notify_server_level_info(player_struct *player, EXTERN_DATA *extern_data);

void load_server_level_info(void)
{
	EXTERN_DATA ext_data;
	fast_send_msg_base(&conn_node_dbsrv::connecter, &ext_data, SERVER_PROTO_LOAD_SERVER_LEVEL_REQUEST, 0, 0);
}

void save_server_level_info(void)
{
	DBServerLevel db_info;
	dbserver_level__init(&db_info);

	db_info.level_id = global_shared_data->server_level.level_id;
	db_info.break_goal = global_shared_data->server_level.break_goal;
	db_info.break_num = global_shared_data->server_level.break_num;
	db_info.break_reward = global_shared_data->server_level.break_reward;
	db_info.n_break_reward = MAX_SERVER_LEVEL_REWARD_NUM;

	EXTERN_DATA ext_data;
	fast_send_msg(&conn_node_dbsrv::connecter, &ext_data, SERVER_PROTO_SAVE_SERVER_LEVEL_REQUEST, dbserver_level__pack, db_info);
}

void break_server_level(void)
{
	ServerLevelTable *config = get_config_by_id(global_shared_data->server_level.level_id + 1, &server_level_config);
	if (!config)
	{
		return;
	}

	uint32_t *pData = (uint32_t*)conn_node_dbsrv::connecter.get_send_data();
	*pData++ = global_shared_data->server_level.config->Level;

	EXTERN_DATA ext_data;
	fast_send_msg_base(&conn_node_dbsrv::connecter, &ext_data, SERVER_PROTO_BREAK_SERVER_LEVEL_REQUEST, sizeof(uint32_t), 0);
}

void check_server_level(void)
{
	ServerLevelTable *next_config = get_config_by_id(global_shared_data->server_level.level_id + 1, &server_level_config);
	if (!next_config)
	{
		return;
	}

	uint32_t now = time_helper::get_cached_time() / 1000;
	ServerLevelTable *config = global_shared_data->server_level.config;
	if (now > sg_server_open_time && (now - sg_server_open_time >= (uint32_t)config->Time * 3600))
	{
		break_server_level();
	}
}

void mark_server_level_break(player_struct *player)
{
	if (global_shared_data->server_level.breaking)
	{
		return;
	}

	global_shared_data->server_level.breaking = true;
	global_shared_data->server_level.break_num = 0;
	memset(&global_shared_data->server_level.break_reward, 0, sizeof(global_shared_data->server_level.break_reward));
	broadcast_server_level_info();
	player->add_title(global_shared_data->server_level.config->Title);
}

void server_level_listen_raid_finish(uint32_t raid_id, player_struct *player)
{
	ServerLevelTable *next_config = get_config_by_id(global_shared_data->server_level.level_id + 1, &server_level_config);
	if (!next_config)
	{
		return;
	}

	ServerLevelInfo *info = &global_shared_data->server_level;
	if (!info->breaking)
	{
		return;
	}
	if (raid_id != (uint32_t)info->config->Dungeon)
	{
		return;
	}
	if (player->data->server_level_break_count == 0)
	{
		player->data->server_level_break_count++;
		info->break_num++;
		bool has_get = false;
		int free_idx = -1;
		for (int i = 0; i < MAX_SERVER_LEVEL_REWARD_NUM; ++i)
		{
			if (info->break_reward[i] == 0)
			{
				free_idx = i;
				break;
			}
			if (info->break_reward[i] == player->get_uuid())
			{
				has_get = true;
				break;
			}
		}

		if (!has_get && free_idx >= 0)
		{
			info->break_reward[free_idx] = player->get_uuid();
			std::map<uint32_t, uint32_t> reward_map;
			reward_map[sg_server_level_reward_item_id] = sg_server_level_reward_item_num;
			player->add_item_list_otherwise_send_mail(reward_map, MAGIC_TYPE_SERVER_LEVEL_BREAK, 270200002, NULL);
		}

		broadcast_server_level_info();

		if (info->break_num >= info->break_goal)
		{
			break_server_level();
		}
	}
}

void broadcast_server_level_info(void)
{
	ServerLevelInfoNotify nty;
	server_level_info_notify__init(&nty);

	nty.level_id = global_shared_data->server_level.level_id;
	nty.break_goal = global_shared_data->server_level.break_goal;
	nty.break_num = global_shared_data->server_level.break_num;
	nty.breaking = global_shared_data->server_level.breaking;

	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SERVER_LEVEL_INFO_NOTIFY, &nty, (pack_func)server_level_info_notify__pack);
	for (std::map<uint64_t, player_struct *>::iterator iter = player_manager_all_players_id.begin(); iter != player_manager_all_players_id.end(); ++iter)
	{
		if (get_entity_type(iter->first) == ENTITY_TYPE_AI_PLAYER)
			continue;
		
		player_struct *player = iter->second;
		if (player->is_online())
		{
			ppp = conn_node_gamesrv::broadcast_msg_add_players(player->get_uuid(), ppp);
		}
	}
	conn_node_gamesrv::broadcast_msg_send();
}

bool is_server_level_limit(uint32_t player_level)
{
	return (player_level >= (uint32_t)global_shared_data->server_level.config->Level);
}

double get_server_exp_addition(uint32_t player_level)
{
	ServerLevelTable *config = global_shared_data->server_level.config;
	if (config && global_shared_data->server_level.breaking && player_level >= (uint32_t)config->LevelPlusEXP1 && (int)config->Level - (int)player_level > (int)config->LevelPlusEXP2)
	{
		return ((config->Level - player_level) * 0.01);
	}

	return 0.0;
}

ServerLevelTable *get_server_level_config(void)
{
	return global_shared_data->server_level.config;
}

