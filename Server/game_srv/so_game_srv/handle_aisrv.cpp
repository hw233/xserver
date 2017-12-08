#include "game_event.h"
#include "conn_node_aisrv.h"
#include "ai.pb-c.h"
#include "uuid.h"
#include "so_game_srv/raid_manager.h"
#include "time_helper.h"
#include "player.h"
#include "msgid.h"
#include "player_manager.h"
#include "conn_node_dbsrv.h"
#include "../proto/comm_message.pb-c.h"
#include "../proto/role.pb-c.h"
#include "../proto/hotel.pb-c.h"
#include "../proto/player_db.pb-c.h"
#include "../proto/friend.pb-c.h"
#include "app_data_statis.h"
#include "game_config.h"
#include "comm_define.h"
#include "team.h"
#include "chengjie.h"
#include "error_code.h"
#include "register_gamesrv.h"
#include "server_level.h"

#include <assert.h>
#include <errno.h>
#include <vector>
#include <stdint.h>
#include <stdlib.h>
#include <map>
#include <set>
#include <string.h>

#define get_data_len() conn_node_aisrv::server_node->get_data_len()
#define get_data() conn_node_aisrv::server_node->get_data()

extern int handle_move_request_impl(player_struct *player, EXTERN_DATA *extern_data, MoveRequest *req);
static int handle_ai_player_move(EXTERN_DATA *extern_data)
{
	player_struct *player = player_manager::get_ai_player_by_id(extern_data->player_id);
	if (!player)
	{
		LOG_ERR("%s %d: can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-20);
	}
	assert(get_entity_type(player->get_uuid()) == ENTITY_TYPE_AI_PLAYER);	
	MoveRequest *req = move_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-30);
	}

	if (req->n_data > 150)
	{
		LOG_DEBUG("%s: move data too much, req->n_data = %lu", __FUNCTION__, req->n_data);
	}
	
	int ret = handle_move_request_impl(player, extern_data, req);
	move_request__free_unpacked(req, NULL);
	return (ret);
}

static int handle_ai_player_cast_skill(EXTERN_DATA *extern_data)
{
	player_struct *player = player_manager::get_ai_player_by_id(extern_data->player_id);
	if (!player)
	{
		LOG_ERR("%s %d: can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-20);
	}
	assert(get_entity_type(player->get_uuid()) == ENTITY_TYPE_AI_PLAYER);	
	
	SkillCastRequest *req = skill_cast_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-30);
	}

	SkillTable *config = get_config_by_id(req->skillid, &skill_config);
	if (!config)
	{
		LOG_ERR("%s: %lu cast skill %u but no config", __FUNCTION__, extern_data->player_id, req->skillid);
		skill_cast_request__free_unpacked(req, NULL);
		return (-40);
	}
	struct ActiveSkillTable *active_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
	player->deal_skill_cast_request(req, config, active_config);
	skill_cast_request__free_unpacked(req, NULL);
	return (0);
}

static int handle_ai_player_hit_skill(EXTERN_DATA *extern_data)
{
	player_struct *player = player_manager::get_ai_player_by_id(extern_data->player_id);
	if (!player)
	{
		LOG_ERR("%s %d: can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-20);
	}
	assert(get_entity_type(player->get_uuid()) == ENTITY_TYPE_AI_PLAYER);
	SkillHitRequest *req = skill_hit_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	player->deal_skill_hit_request(req);
	skill_hit_request__free_unpacked(req, NULL);
	return (0);
}

DbHandleMap m_ai_handle_map;
static int add_msg_handle(uint32_t msg_id, db_handle_func func)
{
	m_ai_handle_map[msg_id] = func;
	return (0);
}

void install_ai_msg_handle()
{
    add_msg_handle(AI_SERVER_MSG_ID__MOVE, handle_ai_player_move);
    add_msg_handle(AI_SERVER_MSG_ID__CAST_SKILL, handle_ai_player_cast_skill);
    add_msg_handle(AI_SERVER_MSG_ID__HIT_SKILL, handle_ai_player_hit_skill);			
}

void uninstall_ai_msg_handle()
{
	m_ai_handle_map.clear();
}


