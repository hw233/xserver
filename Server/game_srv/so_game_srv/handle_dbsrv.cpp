#include "game_event.h"
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

extern int pack_player_online(player_struct *player, EXTERN_DATA *extern_data, bool load_db, bool reconnect);
extern void answer_friend_search(EXTERN_DATA *extern_data, int result, player_struct *target, uint32_t logout_time);
extern void answer_get_other_info(EXTERN_DATA *extern_data, int result, player_struct *target);
extern void notify_server_level_info(player_struct *player, EXTERN_DATA *extern_data);
extern void notify_server_level_break(player_struct *player, EXTERN_DATA *extern_data);

DbHandleMap m_db_handle_map;

static int add_msg_handle(uint32_t msg_id, db_handle_func func)
{
	m_db_handle_map[msg_id] = func;
	return (0);
}

static int handle_find_player_answer(EXTERN_DATA *extern_data)
{
	PROTO_FIND_PLAYER_ANS *proto = (PROTO_FIND_PLAYER_ANS *)conn_node_dbsrv::connecter.buf_head();
	LOG_DEBUG("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);

	AnsFindTarget send;
	ans_find_target__init(&send);
	if (proto->player_id == 0)
	{
		send.ret = 190500126;
	}
	else
	{
		send.pid = proto->player_id;
		send.name = proto->name;
		send.lv = proto->lv;
		send.ret = 0;
		ChengJieTaskManage::AddRoleLevel(send.pid, send.lv, proto->cd);
	}
	if (extern_data->player_id != 0)
	{
		fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_CHENGJIE_FIND_TARGET_ANSWER, ans_find_target__pack, send);
	}
	
	return 0;
}

void add_chengjie_task_immp(ChengjieTaskDb *req, uint32_t taskid)
{
	STChengJie task;
	task.id = taskid;
	task.pid = req->playerid;
	task.fail = req->fail;
	task.shuangjin = req->shuangjin;
	task.exp = req->exp;
	task.courage = req->courage;
	task.timeOut = req->cd;
	task.complete = req->complete;
	task.taskTime = req->complete_cd;
	task.acceptCd = req->accept_cd;
	task.investor = req->investor;
	task.anonymous = req->anonymous;
	task.step = req->step;
	strncpy(task.declaration, req->declaration, 256);

	ChengJieTaskManage::AddTask(task, req->accepter);
}

static int handle_load_chengjie_answer(EXTERN_DATA *extern_data)
{
	PROTO_ADD_CHENGJIE *proto = (PROTO_ADD_CHENGJIE *)conn_node_dbsrv::connecter.buf_head();
	LOG_DEBUG("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);

	LoadChengjieTask *req = load_chengjie_task__unpack(NULL, proto->data_size, proto->data);
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	for (uint32_t i = 0; i < req->n_data; ++i)
	{
		add_chengjie_task_immp(req->data[i], req->data[i]->taskid);
	}
	
	load_chengjie_task__free_unpacked(req, NULL);

	return 0;
}

static int handle_add_chengjie_answer(EXTERN_DATA *extern_data)
{
	PROTO_ADD_CHENGJIE_ANS *proto = (PROTO_ADD_CHENGJIE_ANS *)conn_node_dbsrv::connecter.buf_head();
	LOG_DEBUG("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);

	ChengjieTaskDb *req = chengjie_task_db__unpack(NULL, proto->data_size, proto->data);
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	add_chengjie_task_immp(req, proto->taskid);

	//AnsAddChengjieTask send;
	//ans_add_chengjie_task__init(&send);
	//send.ret = 0;
	//fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_ADD_CHENGJIE_TASK_ANSWER, ans_add_chengjie_task__pack, send);

	chengjie_task_db__free_unpacked(req, NULL);

	return 0;
}

static int handle_doufachang_load_player_answer(EXTERN_DATA *extern_data)
{
	DOUFACHANG_LOAD_PLAYER_ANSWER *ans = (DOUFACHANG_LOAD_PLAYER_ANSWER *)conn_node_dbsrv::connecter.get_data();
	LOG_DEBUG("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	
	player_struct *player1 = player_manager::get_player_by_id(ans->player_id);
	if (!player1 || !player1->is_avaliable())
	{
		return (0);
	}
	player_struct *player = player_manager::create_doufachang_ai_player(ans);
	if (!player)
	{
		LOG_ERR("%s %d: can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);		
		return (-1);
	}

	raid_struct *raid = raid_manager::create_raid(sg_doufachang_raid_id, NULL);
	if (!raid)
	{
		LOG_ERR("%s: create raid failed", __FUNCTION__);
		player_manager::delete_player(player);
		return (-1);
	}

	raid->player_enter_raid_impl(player1, 0, sg_3v3_pvp_raid_param1[1], sg_3v3_pvp_raid_param1[3]);
	raid->player_enter_raid_impl(player, MAX_TEAM_MEM, sg_3v3_pvp_raid_param2[1], sg_3v3_pvp_raid_param2[3]);
	return 0;
}

//从db_srv返回登录结果
static int handle_player_enter_game_answer(EXTERN_DATA *extern_data)
{
	PROTO_ENTER_GAME_RESP *proto = (PROTO_ENTER_GAME_RESP *)conn_node_dbsrv::connecter.buf_head();
	LOG_DEBUG("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);

	player_struct * player = player_manager::create_player(proto, extern_data->player_id);
	if (!player)
	{
		LOG_ERR("%s %d: can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	return pack_player_online(player, extern_data, true, proto->reconnect != 0);
}

//返回保存数据库结果
static int handle_player_save_answer(EXTERN_DATA *extern_data)
{
	LOG_DEBUG("%s %d: player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	player_struct *player;
	player = player_manager::get_player_by_id(extern_data->player_id);
	if (!player) {
			//还没确认保存就删掉了？bug
		LOG_ERR("%s %d: can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	LOG_DEBUG("%s %d: player[%lu], status: %u", __FUNCTION__, __LINE__, extern_data->player_id, player->data->status);
		//又重新上线了，不需要删除了
	if (player->data->status == ONLINE) {
		return (0);
	}

	player_manager::delete_player(player);

	PROTO_SAVE_PLAYER_RESP *req = (PROTO_SAVE_PLAYER_RESP *)conn_node_dbsrv::connecter.buf_head();

	if (req->again>0) {
		CommAnswer resp;
		comm_answer__init(&resp);
		resp.result = 0;

		fast_send_msg(&conn_node_gamesrv::connecter, extern_data, SERVER_PROTO_KICK_ROLE_ANSWER, comm_answer__pack, resp);
	}

	return (0);
}

//返回改名结果
static int handle_player_rename_answer(EXTERN_DATA *extern_data)
{
	LOG_DEBUG("%s %d: player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	player_struct *player = player_manager::get_player_by_id(extern_data->player_id);
	if (!player || !player->is_online())\
	{
		LOG_ERR("%s %d: can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PLAYER_RENAME_DB_ANSWER *req = (PLAYER_RENAME_DB_ANSWER *)conn_node_dbsrv::connecter.buf_head();

	if (req->result == 0)
	{
		//扣除道具
		player->del_item(sg_rename_item_id, sg_rename_item_num, MAGIC_TYPE_RENAME);

		char old_name[MAX_PLAYER_NAME_LEN + 1];
		memcpy(old_name, player->data->name, sizeof(player->data->name));
		
		//修改名字
		memcpy(player->data->name, req->name, MAX_PLAYER_NAME_LEN);

		//同步消息到好友服
		PROTO_FRIEND_SYNC_RENAME *req_friend = (PROTO_FRIEND_SYNC_RENAME*)conn_node_base::get_send_buf(SERVER_PROTO_FRIEND_SYNC_RENAME, 0);
		memcpy(req_friend->old_name, old_name, sizeof(old_name));
		memcpy(req_friend->new_name, player->data->name, sizeof(player->data->name));
		req_friend->head.len = ENDION_FUNC_4(sizeof(PROTO_FRIEND_SYNC_RENAME));
		conn_node_base::add_extern_data(&req_friend->head, extern_data);
		if (conn_node_gamesrv::connecter.send_one_msg(&req_friend->head, 1) != (int)ENDION_FUNC_4(req_friend->head.len))
		{
			LOG_ERR("[%s:%d] send to friend_srv failed err[%d]", __FUNCTION__, __LINE__, errno);
		}

		//同步消息到RANK_SRV
		player->refresh_player_redis_info(false);

		//场景广播
		PlayerNameNotify nty;
		player_name_notify__init(&nty);
		nty.player_id = extern_data->player_id;
		nty.player_name = player->data->name;
		player->broadcast_to_sight_and_team(MSG_ID_PLAYER_NAME_NOTIFY, &nty, (pack_func)player_name_notify__pack, true);
//		if (player->m_team != NULL)
//		{
//			player->m_team->BroadcastToTeamNotinSight(*player, MSG_ID_PLAYER_NAME_NOTIFY, &nty, (pack_func)player_name_notify__pack);
//		}

		player->add_achievement_progress(ACType_PLAYER_RENAME, 0, 0, 1);
	}

	CommAnswer resp;
	comm_answer__init(&resp);
	resp.result = req->result;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PLAYER_RENAME_ANSWER, comm_answer__pack, resp);

	return (0);
}

static int handle_friend_search_answer(EXTERN_DATA *extern_data)
{
	LOG_DEBUG("%s %d: player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	player_struct *player = player_manager::get_player_by_id(extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("%s %d: can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PROTO_ENTER_GAME_RESP *proto = (PROTO_ENTER_GAME_RESP *)conn_node_dbsrv::connecter.buf_head();

	int ret = 0;
	uint32_t logout_time = 0;
	player_struct *target = NULL;
	bool bDelete = false;
	do
	{
		if (proto->player_id == 0)
		{
			ret = ERROR_ID_FRIEND_SEARCH_NOT_EXIST;
			break;
		}

		target = player_manager::get_player_by_id(proto->player_id);
		if (target)
		{
			break;
		}

		target = player_manager::create_player(proto, proto->player_id);
		if (!target)
		{
			ret = ERROR_ID_FRIEND_SEARCH_NOT_EXIST;
			break;
		}
		
		bDelete = true;
		logout_time = proto->logout_time;
	} while(0);

	answer_friend_search(extern_data, ret, target, logout_time);

	if (target && bDelete)
	{
		player_manager::delete_player(target);
	}

	return (0);
}

static int handle_get_other_info_answer(EXTERN_DATA *extern_data)
{
	LOG_DEBUG("%s %d: player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	player_struct *player = player_manager::get_player_by_id(extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("%s %d: can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PROTO_ENTER_GAME_RESP *proto = (PROTO_ENTER_GAME_RESP *)conn_node_dbsrv::connecter.buf_head();

	int ret = 0;
//	uint32_t logout_time = 0;
	player_struct *target = NULL;
	do
	{
		if (proto->player_id == 0)
		{
			ret = ERROR_ID_FRIEND_SEARCH_ID;
			break;
		}

		target = player_manager::create_player(proto, proto->player_id);
		if (!target)
		{
			ret = ERROR_ID_FRIEND_SEARCH_ID;
			break;
		}
	} while(0);

	answer_get_other_info(extern_data, ret, target);

	if (target)
	{
		player_manager::delete_player(target);
	}

	return (0);
}

static int handle_load_server_level_answer(EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] ", __FUNCTION__, __LINE__);
	memset(&global_shared_data->server_level, 0, sizeof(global_shared_data->server_level));

	int data_size = conn_node_dbsrv::connecter.get_data_len();
	DBServerLevel *db_info = NULL;
	if (data_size != 0)
	{
		db_info = dbserver_level__unpack(NULL, data_size, conn_node_dbsrv::connecter.get_data());
	}

	ServerLevelTable *config = NULL;
	if (db_info != NULL)
	{
		global_shared_data->server_level.level_id = db_info->level_id;
		global_shared_data->server_level.break_goal = db_info->break_goal;
		global_shared_data->server_level.break_num = db_info->break_num;
		for (size_t i = 0; i < db_info->n_break_reward && i < MAX_SERVER_LEVEL_REWARD_NUM; ++i)
		{
			global_shared_data->server_level.break_reward[i] = db_info->break_reward[i];
		}

		config = get_config_by_id(global_shared_data->server_level.level_id, &server_level_config);
	}
	else
	{
		config = server_level_config.begin()->second;
	}

	if (config != NULL)
	{
		global_shared_data->server_level.config = config;
		if (global_shared_data->server_level.level_id == 0)
		{
			global_shared_data->server_level.level_id = config->ID;
			global_shared_data->server_level.break_goal = config->DungeonSchedule;
		}
	}

	if (db_info != NULL)
	{
		dbserver_level__free_unpacked(db_info, NULL);
	}

	return 0;
}

static int handle_break_server_level_answer(EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] ", __FUNCTION__, __LINE__);

	uint32_t* pData = (uint32_t*)conn_node_dbsrv::connecter.get_data();
	uint32_t num = *pData++;

	ServerLevelTable *config = get_config_by_id(global_shared_data->server_level.level_id + 1, &server_level_config);
	if (!config)
	{
		return -1;
	}

	memset(&global_shared_data->server_level, 0, sizeof(global_shared_data->server_level));
	if (num != 0)
	{
		global_shared_data->server_level.break_goal = num;
	}
	else
	{
		global_shared_data->server_level.break_goal = config->DungeonSchedule;
	}

	global_shared_data->server_level.level_id = config->ID;
	global_shared_data->server_level.config = config;
	save_server_level_info();

	broadcast_server_level_info();
	{
		uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SERVER_LEVEL_BREAK_NOTIFY, NULL, (pack_func)NULL);
		for (std::map<uint64_t, player_struct *>::iterator iter = player_manager_all_players_id.begin(); iter != player_manager_all_players_id.end(); ++iter)
		{
			if (get_entity_type(iter->first) == ENTITY_TYPE_AI_PLAYER)
				continue;

			player_struct *player = iter->second;
			if (player->is_online())
			{
				ppp = conn_node_gamesrv::broadcast_msg_add_players(player->get_uuid(), ppp);
				player->data->server_level_break_notify = global_shared_data->server_level.level_id;
			}
		}
		conn_node_gamesrv::broadcast_msg_send();
	}

	return 0;
}

void install_db_msg_handle()
{
	add_msg_handle(SERVER_PROTO_ENTER_GAME_ANSWER, handle_player_enter_game_answer);
	add_msg_handle(SERVER_PROTO_SAVE_PLAYER, handle_player_save_answer);
	add_msg_handle(SERVER_PROTO_RENAME_ANSWER, handle_player_rename_answer);
	add_msg_handle(SERVER_PROTO_FIND_PLAYER_ANSWER, handle_find_player_answer);
	add_msg_handle(SERVER_PROTO_ADD_CHENGJIE_ANSWER, handle_add_chengjie_answer);
	add_msg_handle(SERVER_PROTO_LOAD_CHENGJIE_ANSWER, handle_load_chengjie_answer);
	add_msg_handle(MSG_ID_FRIEND_SEARCH_ANSWER, handle_friend_search_answer);
	add_msg_handle(MSG_ID_GET_OTHER_INFO_ANSWER, handle_get_other_info_answer);
	add_msg_handle(SERVER_PROTO_LOAD_SERVER_LEVEL_ANSWER, handle_load_server_level_answer);
	add_msg_handle(SERVER_PROTO_BREAK_SERVER_LEVEL_ANSWER, handle_break_server_level_answer);
	add_msg_handle(SERVER_PROTO_DOUFACHANG_LOAD_PLAYER_ANSWER, handle_doufachang_load_player_answer);
}

void uninstall_db_msg_handle()
{
	m_db_handle_map.clear();
}


