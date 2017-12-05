#include "game_event.h"
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

__attribute__((used)) static int handle_ai_player_move(EXTERN_DATA *extern_data)
{
	return (0);
}

__attribute__((used)) static int handle_ai_player_cast_skill(EXTERN_DATA *extern_data)
{
	return (0);
}

__attribute__((used)) static int handle_ai_player_hit_skill(EXTERN_DATA *extern_data)
{
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


