#include "game_event.h"
#include "time_helper.h"
#include "player.h"
#include "uuid.h"
#include "msgid.h"
#include "player_manager.h"
#include "sight_space_manager.h"
#include "raid_manager.h"
#include "conn_node_dbsrv.h"
#include "scene_manager.h"
#include "game_config.h"
#include "team.h"
#include "camp_judge.h"
#include "monster_manager.h"
#include "../../proto/cast_skill.pb-c.h"
#include "../../proto/move.pb-c.h"
#include "../../proto/raid.pb-c.h"
#include "../../proto/move_direct.pb-c.h"
#include "../../proto/login.pb-c.h"
#include "../../proto/scene_transfer.pb-c.h"
#include "../../proto/bag.pb-c.h"
#include "../../proto/relive.pb-c.h"
#include "../../proto/role.pb-c.h"
#include "../../proto/chat.pb-c.h"
#include "../../proto/collect.pb-c.h"
#include "../../proto/task.pb-c.h"
#include "../../proto/team.pb-c.h"
#include "../../proto/equip.pb-c.h"
#include "../../proto/pk.pb-c.h"
#include "../../proto/pvp_raid.pb-c.h"
#include "../../proto/hotel.pb-c.h"
#include "../../proto/mail_db.pb-c.h"
#include "../../proto/shop.pb-c.h"
#include "../../proto/yuqidao.pb-c.h"
#include "../../proto/fashion.pb-c.h"
#include "../../proto/baguapai.pb-c.h"
#include "../../proto/activity.pb-c.h"
#include "../../proto/setting.pb-c.h"
#include "../../proto/zhenying.pb-c.h"
#include "../../proto/answer.pb-c.h"
#include "../../proto/player_redis_info.pb-c.h"
#include "../../proto/personality.pb-c.h"
#include "../../proto/friend.pb-c.h"
#include "../../proto/guild_battle.pb-c.h"
#include "../../proto/xunbao.pb-c.h"
#include "../../proto/partner.pb-c.h"
#include "../../proto/gift_package.pb-c.h"
#include "auto_add_hp.pb-c.h"
#include "horse.pb-c.h"
#include "unit_path.h"
#include "unit.h"
#include "cached_hit_effect.h"
#include "count_skill_damage.h"
#include "skill.h"
#include "buff_manager.h"
#include "error_code.h"
#include "app_data_statis.h"
#include "chat.h"
#include "collect.h"
#include "team.h"
#include "check_range.h"
#include "player_redis_info.pb-c.h"
#include "pvp_match_manager.h"
#include "chengjie.h"
#include "zhenying_raid_manager.h"
#include "zhenying_battle.h"
#include "guild_battle_manager.h"
#include "guild_wait_raid_manager.h"
#include "register_gamesrv.h"
#include "attr_calc.h"
#include "partner_manager.h"
#include "cash_truck_manager.h"
#include <assert.h>
#include <errno.h>
#include <vector>
#include <stdint.h>
#include <stdlib.h>
#include <map>
#include <set>
#include <string.h>
#include <math.h>

extern uint32_t sg_gm_cmd_open;
//队员和队长的最大距离20米，太远就要传送过来
static const uint64_t max_team_mem_distance = 20 * 20;

extern uint32_t guild_battle_manager_final_list_state;
extern uint32_t guild_battle_manager_final_list_tick;

//#define db_connecter(a,b) conn_node_dbsrv::connecter.send_one_msg(a,b)
//#define connecter conn_node_gamesrv::connecter

#define get_data_len() conn_node_gamesrv::connecter.get_data_len()
#define get_data() conn_node_gamesrv::connecter.get_data()
#define get_seq() conn_node_gamesrv::connecter.get_seq()

#define buf_head() conn_node_gamesrv::connecter.buf_head()

#define get_send_data() conn_node_base::get_send_data()
#define get_send_buf(A,B) conn_node_base::get_send_buf(A,B)
#define add_extern_data(A,B) conn_node_base::add_extern_data(A,B)

static int notify_head_icon_info(player_struct *player, EXTERN_DATA *extern_data);
static int notify_task_list(player_struct *player, EXTERN_DATA *extern_data);
static int notify_equip_list(player_struct *player, EXTERN_DATA *extern_data);
static int notify_yuqidao_info(player_struct *player, EXTERN_DATA *extern_data);
static int notify_pvp_raid_info(player_struct *player, int type, EXTERN_DATA *extern_data);
static int player_online_enter_scene_after(player_struct *player, EXTERN_DATA *extern_data);
static void player_online_to_other_srvs(player_struct *player, EXTERN_DATA *extern_data, bool reconnect);
static void player_ready_enter_scene(player_struct *player, EXTERN_DATA *extern_data, bool reconnect);
static int notify_baguapai_info(player_struct *player, EXTERN_DATA *extern_data);
static int notify_setting_switch_info(player_struct *player, EXTERN_DATA *extern_data);
static int notify_transfer_out_stuck_info(player_struct *player, EXTERN_DATA *extern_data);
static int notify_partner_info(player_struct *player, EXTERN_DATA *extern_data);

GameHandleMap   m_game_handle_map;

static void add_msg_handle(uint32_t msg_id, game_handle_func func)
{
	m_game_handle_map[msg_id] = func;
}

static void send_comm_answer(uint32_t msgid, uint32_t result, EXTERN_DATA *extern_data)
{
	CommAnswer resp;
	comm_answer__init(&resp);
	resp.result = result;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, msgid, comm_answer__pack, resp);
}

static int comm_check_player_valid(player_struct *player, uint64_t player_id)
{
	if (!player) {
		LOG_ERR("%s %d: do not have player[%lu]", __FUNCTION__, __LINE__, player_id);
//		player = player_manager::create_tmp_player(extern_data->player_id);
		return (-1);
	}

	if (!player->is_avaliable())
	{
		LOG_ERR("%s: player[%lu] do not have scene or data", __FUNCTION__, player_id);
		return -10;
	}

	if (!player->is_alive())
	{
		LOG_ERR("%s: player[%lu] dead", __FUNCTION__, player_id);
		return -20;
	}
	return (0);
}

/*
static int set_player_pos_impl(player_struct *player, float pos_x, float pos_z)
{
	scene_struct *scene = player->scene;
	area_struct *old_area = player->area;
	set_unit_pos(&player->data->move_path, pos_x, pos_z);
	area_struct *new_area = scene->get_area_by_pos(pos_x, pos_z);
	if (old_area != new_area)
	{
		player->update_player_sight(old_area, new_area);
	}
	return (0);
}
*/

int handle_move_y_start_request_impl(player_struct *player, EXTERN_DATA *extern_data, MoveYStartRequest *req)
{
	player->data->pos_y = req->cur_pos_y;
	MoveYStartNotify notify;
	move_y_start_notify__init(&notify);
	notify.playerid = extern_data->player_id;
	notify.cur_pos = req->cur_pos;
	notify.cur_pos_y = req->cur_pos_y;
	notify.direct_y = req->direct_y;

	player->broadcast_to_sight(MSG_ID_MOVE_Y_START_NOTIFY, &notify, (pack_func)move_y_start_notify__pack, false);

	player->interrupt();
	return (0);
}

int handle_move_start_request_impl(player_struct *player, EXTERN_DATA *extern_data, MoveStartRequest *req)
{
	if (!player->data || !player->scene)
	{
//		move_request__free_unpacked(req, NULL);
		LOG_ERR("%s %d: player[%lu] do not have data[%p] or scene[%p]", __FUNCTION__, __LINE__, extern_data->player_id, player->data, player->scene);
		return (-10);
	}

	uint64_t now = time_helper::get_cached_time();
	if (now - player->last_change_area_time < 500)
	{
		LOG_DEBUG("%s %d: player[%lu] request move too much[%ld]", __FUNCTION__, __LINE__, player->data->player_id, now - player->last_change_area_time);
	}
	player->last_change_area_time = now;

	LOG_INFO("%s: player[%lu] cur_pos[%.1f][%.1f] req_pos[%.1f][%.1f] directx[%.2f] directz[%.2f]",
		__FUNCTION__, player->get_uuid(), player->get_pos()->pos_x, player->get_pos()->pos_z,
		req->cur_pos->pos_x, req->cur_pos->pos_z, req->direct_x, req->direct_z);

	if (player->check_pos_distance(req->cur_pos->pos_x, req->cur_pos->pos_z) != 0)
	{
		struct position *pos = player->get_pos();
		LOG_ERR("%s %d: player[%lu] cur_pos[%.1f][%.1f] flash to [%.1f][%.1f]", __FUNCTION__, __LINE__, player->get_uuid(),
			pos->pos_x, pos->pos_z, req->cur_pos->pos_x, req->cur_pos->pos_z);
	}

//	player->data->pos_y = req->cur_pos_y;
	player->set_pos_with_broadcast(req->cur_pos->pos_x, req->cur_pos->pos_z);
//	set_player_pos_impl(player, req->cur_pos->pos_x, req->cur_pos->pos_z);
/*
	scene_struct *scene = player->scene;
		// TODO: 1: 检查当前位置
		// TODO: 2: 检查路径的阻挡

	area_struct *old_area = player->area;
	struct position *pos = player->get_player_pos();
	pos->pos_x = req->cur_pos->pos_x;
	pos->pos_z = req->cur_pos->pos_z;
*/

	float c = req->direct_x * req->direct_x + req->direct_z * req->direct_z;
	c = FastSqrtQ3(c);
	if (c == 0)
	{
		LOG_ERR("%s %d: player %lu direct wrong", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-15);
	}

	double speed = player->get_speed();
	player->data->move_path.speed_x = req->direct_x / c * speed;
	player->data->move_path.speed_z = req->direct_z / c * speed;
	player->data->move_path.start_time = time_helper::get_cached_time();
	player->data->m_angle = pos_to_angle(req->direct_x, req->direct_z);
//		float t = player->data->m_angle / M_PI * 180 * -1 + 90;
//		LOG_DEBUG("%s: m_angle = %.3f, %.3f, %.3f %.3f", __FUNCTION__, player->data->m_angle, player->data->m_angle / M_PI, player->data->m_angle / M_PI * 180, t);	
/*
	area_struct *new_area = scene->get_area_by_pos(pos->pos_x, pos->pos_z);
	if (old_area != new_area)
	{
		player->update_player_sight(old_area, new_area);
	}

		//设置玩家的移动路径
	player->data->move_path.start_time = time_helper::get_cached_time();
*/
/*
	player->data->move_path.cur_pos = 0;
	player->data->move_path.max_pos = req->n_data - 1;
	if (player->data->move_path.max_pos >= MAX_PATH_POSITION)
		player->data->move_path.max_pos = MAX_PATH_POSITION - 1;
	for (size_t i = 0; i <= player->data->move_path.max_pos; ++i)
	{
		player->data->move_path.pos[i].pos_x = req->data[i]->pos_x;
		player->data->move_path.pos[i].pos_z = req->data[i]->pos_z;
	}
*/
	CommAnswer resp;
	comm_answer__init(&resp);
//	uint32_t ret;

//	FAST_SEND_TO_CLIENT(MSG_ID_MOVE_ANSWER, comm_answer__pack);
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_MOVE_START_ANSWER, comm_answer__pack, resp);

	MoveStartNotify notify;
	move_start_notify__init(&notify);
	notify.playerid = extern_data->player_id;
	if (player->data->truck.on_truck)
	{
		cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(player->data->truck.truck_id);
		if (truck != NULL)
		{
			notify.playerid = truck->get_uuid();
		}
	}
	
	notify.cur_pos = req->cur_pos;
	notify.cur_pos_y = player->data->pos_y;
	notify.direct_x = req->direct_x;
//	notify.direct_y = req->direct_y;
	notify.direct_z = req->direct_z;

	player->broadcast_to_sight(MSG_ID_MOVE_START_NOTIFY, &notify, (pack_func)move_start_notify__pack, false);

	player->interrupt();
	return (0);
}

int handle_move_y_stop_request_impl(player_struct *player, EXTERN_DATA *extern_data, MoveYStopRequest *req)
{
	player->data->pos_y = req->cur_pos_y;
	MoveYStopNotify notify;
	move_y_stop_notify__init(&notify);
	notify.playerid = extern_data->player_id;
	notify.cur_pos = req->cur_pos;
	notify.cur_pos_y = req->cur_pos_y;

	player->broadcast_to_sight(MSG_ID_MOVE_Y_STOP_NOTIFY, &notify, (pack_func)move_y_stop_notify__pack, false);
	return (0);
}

int handle_move_stop_request_impl(player_struct *player, EXTERN_DATA *extern_data, MoveStopRequest *req)
{
	if (!player->data || !player->scene)
	{
//		move_request__free_unpacked(req, NULL);
		LOG_ERR("%s %d: player[%lu] do not have data[%p] or scene[%p]", __FUNCTION__, __LINE__, extern_data->player_id, player->data, player->scene);
		return (-10);
	}

	uint64_t now = time_helper::get_cached_time();
	if (now - player->last_change_area_time < 500)
	{
		LOG_DEBUG("%s %d: player[%lu] request move too much[%ld]", __FUNCTION__, __LINE__, player->data->player_id, now - player->last_change_area_time);
	}
	player->last_change_area_time = now;

	LOG_INFO("%s: player[%lu] cur_pos[%.1f][%.1f] req_pos[%.1f][%.1f]",
		__FUNCTION__, player->get_uuid(), player->get_pos()->pos_x, player->get_pos()->pos_z,
		req->cur_pos->pos_x, req->cur_pos->pos_z);

	if (player->check_pos_distance(req->cur_pos->pos_x, req->cur_pos->pos_z) != 0)
	{
		struct position *pos = player->get_pos();
		LOG_ERR("%s %d: player[%lu] cur_pos[%.1f][%.1f] flash to [%.1f][%.1f]", __FUNCTION__, __LINE__, player->get_uuid(),
			pos->pos_x, pos->pos_z, req->cur_pos->pos_x, req->cur_pos->pos_z);
	}

//	player->data->pos_y = req->cur_pos_y;
	player->set_pos_with_broadcast(req->cur_pos->pos_x, req->cur_pos->pos_z);
/*
	player->data->move_path.speed_x = 0;
	player->data->move_path.speed_z = 0;
	scene_struct *scene = player->scene;
		// TODO: 1: 检查当前位置
		// TODO: 2: 检查路径的阻挡

	area_struct *old_area = player->area;
	struct position *pos = player->get_player_pos();
	pos->pos_x = req->cur_pos->pos_x;
	pos->pos_z = req->cur_pos->pos_z;
	player->data->move_path.speed_x = 0;
	player->data->move_path.speed_z = 0;
	area_struct *new_area = scene->get_area_by_pos(pos->pos_x, pos->pos_z);
	if (old_area != new_area)
	{
		player->update_player_sight(old_area, new_area);
	}
*/
		//设置玩家的移动路径
/*
	player->data->move_path.start_time = time_helper::get_cached_time();
	player->data->move_path.cur_pos = 0;
	player->data->move_path.max_pos = req->n_data - 1;
	if (player->data->move_path.max_pos >= MAX_PATH_POSITION)
		player->data->move_path.max_pos = MAX_PATH_POSITION - 1;
	for (size_t i = 0; i <= player->data->move_path.max_pos; ++i)
	{
		player->data->move_path.pos[i].pos_x = req->data[i]->pos_x;
		player->data->move_path.pos[i].pos_z = req->data[i]->pos_z;
	}
*/
	CommAnswer resp;


	comm_answer__init(&resp);
//	uint32_t ret;

//	FAST_SEND_TO_CLIENT(MSG_ID_MOVE_ANSWER, comm_answer__pack);
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_MOVE_STOP_ANSWER, comm_answer__pack, resp);

	MoveStopNotify notify;
	move_stop_notify__init(&notify);
	notify.playerid = extern_data->player_id;
	if (player->data->truck.on_truck)
	{
		cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(player->data->truck.truck_id);
		if (truck != NULL)
		{
			notify.playerid = truck->get_uuid();
		}
	}
	notify.cur_pos = req->cur_pos;
	notify.cur_pos_y = player->data->pos_y;

	player->broadcast_to_sight(MSG_ID_MOVE_STOP_NOTIFY, &notify, (pack_func)move_stop_notify__pack, false);
	return (0);
}

int handle_move_request_impl(player_struct *player, EXTERN_DATA *extern_data, MoveRequest *req)
{
		// TODO:	PROTO_ENTER_GAME_RESP *proto = (PROTO_ENTER_GAME_RESP *)conn_node_dbsrv::connecter.buf_head();
		// TODO:    player_struct * player = player_manager::create_player(proto, extern_data->player_id);

	if (!player->data || !player->scene)
	{
//		move_request__free_unpacked(req, NULL);
		LOG_ERR("%s %d: player[%lu] do not have data[%p] or scene[%p]", __FUNCTION__, __LINE__, extern_data->player_id, player->data, player->scene);
		return (-10);
	}

	if (req->n_data == 0)
		return (0);
		// TODO: 1: 检查当前位置
	if (player->check_pos_distance(req->data[0]->pos_x, req->data[0]->pos_z) != 0)
	{
		struct position *pos = player->get_pos();
		LOG_ERR("%s %d: player[%lu] cur_pos[%.1f][%.1f] flash to [%.1f][%.1f]", __FUNCTION__, __LINE__, player->get_uuid(),
			pos->pos_x, pos->pos_z, req->data[0]->pos_x, req->data[0]->pos_z);
	}
		// TODO: 2: 检查路径的阻挡
/*
	scene_struct *scene = player->scene;
	area_struct *old_area = player->area;
	struct position *pos = player->get_player_pos();
	pos->pos_x = req->data[0]->pos_x;
	pos->pos_z = req->data[0]->pos_z;
	area_struct *new_area = scene->get_area_by_pos(pos->pos_x, pos->pos_z);
	if (old_area != new_area)
	{
		player->update_player_sight(old_area, new_area);
	}
*/
	player->set_pos_with_broadcast(req->data[0]->pos_x, req->data[0]->pos_z);
//	set_player_pos_impl(player, req->data[0]->pos_x, req->data[0]->pos_z);
		//设置玩家的移动路径
	player->data->move_path.start_time = time_helper::get_cached_time();
	player->data->move_path.cur_pos = 0;
	player->data->move_path.max_pos = req->n_data - 1;
	if (req->n_data - 1 >= MAX_PATH_POSITION)
	{
		LOG_ERR("%s %d: player[%lu] move pos too much[%lu]", __FUNCTION__, __LINE__, player->get_uuid(), req->n_data);
		player->data->move_path.max_pos = MAX_PATH_POSITION - 1;
	}
	for (size_t i = 0; i <= player->data->move_path.max_pos; ++i)
	{
		player->data->move_path.pos[i].pos_x = req->data[i]->pos_x;
		player->data->move_path.pos[i].pos_z = req->data[i]->pos_z;
	}

	if (player->data->move_path.max_pos >= 1)
	{
		player->data->m_angle = pos_to_angle(player->data->move_path.pos[1].pos_x - player->data->move_path.pos[0].pos_x,
			player->data->move_path.pos[1].pos_z - player->data->move_path.pos[0].pos_z);

//		float t = player->data->m_angle / M_PI * 180 * -1 + 90;
//		LOG_DEBUG("%s: m_angle = %.3f, %.3f, %.3f %.3f", __FUNCTION__, player->data->m_angle, player->data->m_angle / M_PI, player->data->m_angle / M_PI * 180, t);
	}
	
	CommAnswer resp;
	comm_answer__init(&resp);
//	uint32_t ret;

//	FAST_SEND_TO_CLIENT(MSG_ID_MOVE_ANSWER, comm_answer__pack);
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_MOVE_ANSWER, comm_answer__pack, resp);

	MoveNotify notify;
	move_notify__init(&notify);
	if (player->data->truck.on_truck)
	{
		notify.playerid = player->data->truck.truck_id;
	} 
	else
	{
		notify.playerid = extern_data->player_id;
	}
	notify.n_data = req->n_data;
	notify.data = req->data;

	player->broadcast_to_sight(MSG_ID_MOVE_NOTIFY, &notify, (pack_func)move_notify__pack, false);

	player->interrupt();
	return (0);
}

static int handle_move_y_start_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}

	if (player->buff_state & BUFF_STATE_STUN)
	{
		LOG_ERR("%s: %lu is in lock state", __FUNCTION__, extern_data->player_id);
		return (-10);
	}
	if (player->buff_state & BUFF_STATE_TAUNT)
	{
		LOG_ERR("%s: %lu is in taunt state", __FUNCTION__, extern_data->player_id);
		return (-20);
	}

	MoveYStartRequest *req = move_y_start_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-30);
	}
	int ret = handle_move_y_start_request_impl(player, extern_data, req);
	move_y_start_request__free_unpacked(req, NULL);

	return (ret);
}
static int handle_move_y_stop_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player) {
		LOG_ERR("%s %d: do not have player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
//		player = player_manager::create_tmp_player(extern_data->player_id);
		return (-1);
	}

	if (!player->is_avaliable())
	{
		LOG_ERR("%s: player[%lu] do not have scene or data", __FUNCTION__, extern_data->player_id);
		return -10;
	}

	if (player->buff_state & BUFF_STATE_STUN)
	{
		LOG_ERR("%s: %lu is in lock state", __FUNCTION__, extern_data->player_id);
		return (-20);
	}
	if (player->buff_state & BUFF_STATE_TAUNT)
	{
		LOG_ERR("%s: %lu is in taunt state", __FUNCTION__, extern_data->player_id);
		return (-21);
	}

	MoveYStopRequest *req = move_y_stop_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-30);
	}
	int ret = handle_move_y_stop_request_impl(player, extern_data, req);
	move_y_stop_request__free_unpacked(req, NULL);
	return (ret);
}

static int handle_move_start_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}

	if (player->buff_state & BUFF_STATE_STUN)
	{
		LOG_ERR("%s: %lu is in lock state", __FUNCTION__, extern_data->player_id);
		return (-10);
	}
	if (player->buff_state & BUFF_STATE_TAUNT)
	{
		LOG_ERR("%s: %lu is in taunt state", __FUNCTION__, extern_data->player_id);
		return (-20);
	}

	MoveStartRequest *req = move_start_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-30);
	}
	int ret = handle_move_start_request_impl(player, extern_data, req);
	move_start_request__free_unpacked(req, NULL);
	return (ret);
}
static int handle_move_stop_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player) {
		LOG_ERR("%s %d: do not have player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
//		player = player_manager::create_tmp_player(extern_data->player_id);
		return (-1);
	}

	if (!player->is_avaliable())
	{
		LOG_ERR("%s: player[%lu] do not have scene or data", __FUNCTION__, extern_data->player_id);
		return -10;
	}

	if (player->buff_state & BUFF_STATE_STUN)
	{
		LOG_ERR("%s: %lu is in lock state", __FUNCTION__, extern_data->player_id);
		return (-20);
	}
	if (player->buff_state & BUFF_STATE_TAUNT)
	{
		LOG_ERR("%s: %lu is in taunt state", __FUNCTION__, extern_data->player_id);
		return (-21);
	}

	MoveStopRequest *req = move_stop_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-30);
	}
	int ret = handle_move_stop_request_impl(player, extern_data, req);
	move_stop_request__free_unpacked(req, NULL);
	return (ret);
}

static int handle_move_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}

	if (player->buff_state & BUFF_STATE_STUN)
	{
		LOG_ERR("%s: %lu is in lock state", __FUNCTION__, extern_data->player_id);
		return (-10);
	}
	if (player->buff_state & BUFF_STATE_TAUNT)
	{
		LOG_ERR("%s: %lu is in taunt state", __FUNCTION__, extern_data->player_id);
		return (-20);
	}

	MoveRequest *req = move_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-30);
	}
// TODO:
	if (req->n_data > 150)
	{
		LOG_DEBUG("%s: move data too much, req->n_data = %lu", __FUNCTION__, req->n_data);
	}
	
	int ret = handle_move_request_impl(player, extern_data, req);
	move_request__free_unpacked(req, NULL);
	return (ret);
}
/*
static int handle_skill_hit_notify(player_struct *player, EXTERN_DATA *extern_data)
{
	SkillHitNotify *notify = skill_hit_notify__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!notify) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	unit_broadcast_to_sight(player, MSG_ID_SKILL_HIT_NOTIFY, notify, (pack_func)skill_hit_notify__pack, false);
	skill_hit_notify__free_unpacked(notify, NULL);

	return (0);
}
*/

static int handle_learn_skill_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}

	LearnSkillReq *req = learn_skill_req__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	if (req->autoup)
	{
		req->num = player->m_skill.GetLevelUpTo(req->id);
	}
	if (req->num == 0)
	{
		req->num = 1;
	}

		LearnSkillAns notify;
		learn_skill_ans__init(&notify);
		notify.id = req->id;
		notify.ret = player->m_skill.Learn(req->id, req->num);
		skill_struct *skill = player->m_skill.GetSkill(req->id);
		if (skill != NULL)
		{
			notify.lv = skill->data->lv;
		}

		fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_LEARN_SKILL_ANSWER, learn_skill_ans__pack, notify);


	learn_skill_req__free_unpacked(req, NULL);

	return (0);
}

extern SkillData skillData[MySkill::MAX_SKILL_NUM];
extern SkillData *skillDataPoint[MySkill::MAX_SKILL_NUM];
static int handle_open_skill_for_guide_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}

	if (player->scene == NULL || player->data->scene_id != 20035)
	{
		return -2;
	}

	LearnSkillReq *req = learn_skill_req__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	SkillList send;
	skill_list__init(&send);
	int i = 0;

	skill_struct * pSkillStruct = player->m_skill.InsertSkill(req->id);
	if (pSkillStruct != NULL)
	{
		skill_data__init(skillData + i);
		skillData[i].id = req->id;
		skillData[i].lv = pSkillStruct->data->lv;
		skillData[i].cur_fuwen = pSkillStruct->data->cur_fuwen;
		skillData[i].n_fuwen = pSkillStruct->data->fuwen_num;
		skillData[i].fuwen = pSkillStruct->data->fuwen;

		skillDataPoint[i] = skillData + i;
		++i;
	}
	send.n_data = i;
	send.data = skillDataPoint;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_NEW_SKILL_NOTIFY, skill_list__pack, send);


	learn_skill_req__free_unpacked(req, NULL);

	return (0);
}

static int handle_set_fuwen_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}
	Fuwen *req = fuwen__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	FuwenAns send;
	fuwen_ans__init(&send);
	send.ret = player->m_skill.SetFuwen(req->skill, req->fuwen);
	send.fuwen = req->fuwen;
	send.skill = req->skill;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_SET_FUWEN_ANSWER, fuwen_ans__pack, send);

	fuwen__free_unpacked(req, NULL);


	return 0;
}
static int get_add_medicine_exp(player_struct *player, uint32_t type, uint32_t num, uint32_t &add)
{
	if (type > MAX_LIVE_SKILL_NUM)
	{
		return 1;
	}
	LifeSkillTable *table = get_medicine_table(type, player->data->live_skill.level[type]);
	if (table == NULL)
	{
		return 2;
	}
	if (num == 0)
	{
		if (player->data->live_skill.book[type] < 1)
		{
			return 190410003;
		}
		if (player->del_item(table->NeedItem, 1, MAGIC_TYPE_LIVE_SKILL) < 0)
		{
			return 190410004;
		}
		add = 10;
		--player->data->live_skill.book[type];
	}
	else
	{
		if (player->sub_coin(table->NeedCoin * num, MAGIC_TYPE_LIVE_SKILL, false) < 0)
		{
			return 190410005;
		}
		//todo  扣帮贡
		add = table->ExpAdd * num;

		PROTO_UNDO_COST *proto_head;
		proto_head = (PROTO_UNDO_COST *)conn_node_base::global_send_buf;
		proto_head->head.msg_id = ENDION_FUNC_2(SERVER_PROTO_GUILD_PRODUCE_MEDICINE);
		proto_head->head.seq = 0;
		proto_head->cost.coin = num;
		proto_head->cost.gold = table->NeedDonation * num;
		proto_head->cost.statis_id = type;
		proto_head->head.len = ENDION_FUNC_4(sizeof(PROTO_UNDO_COST));

		EXTERN_DATA extern_data;
		extern_data.player_id = player->get_uuid();
		conn_node_base::add_extern_data((PROTO_HEAD *)proto_head, &extern_data);
		if (conn_node_gamesrv::connecter.send_one_msg((PROTO_HEAD *)proto_head, 1) != (int)(ENDION_FUNC_4(proto_head->head.len))) {
			LOG_ERR("%s %d: send to all failed err[%d]", __FUNCTION__, __LINE__, errno);
		}
	}

	return 0;
}
static int handle_learn_live_skill_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}

	LearnLiveReq *req = learn_live_req__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint32_t type = req->type; //LIVE_SKILL_TYPE
	uint32_t num = req->num; //升级次数 0物品升级
	learn_live_req__free_unpacked(req, NULL);

	uint32_t add = 0;
	AnsLearnLiveSkill send;
	ans_learn_live_skill__init(&send);
	send.ret = get_add_medicine_exp(player, type, num, add);
	if (send.ret == 0)
	{
		if (num == 0)
		{
			send.type = type;
			player->data->live_skill.exp[type] += add;
			LifeSkillTable *table = get_medicine_table(type, player->data->live_skill.level[type]);
			while (player->data->live_skill.exp[type] >= table->Exp && table != NULL)
			{
				if (player->data->live_skill.level[type] >= player->data->live_skill.broken[type])
				{
					break;
				}
				++player->data->live_skill.level[type];
				player->data->live_skill.exp[type] -= table->Exp;
				table = get_medicine_table(type, player->data->live_skill.level[type]);
			}
			send.book = player->data->live_skill.book[type];
			send.lv = player->data->live_skill.level[type];
			send.exp = player->data->live_skill.exp[type];

			fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_LEARN_LIVE_SKILL_ANSWER, ans_learn_live_skill__pack, send);
		}
		
	}
	else
	{
		fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_LEARN_LIVE_SKILL_ANSWER, ans_learn_live_skill__pack, send);
	}
	
	return 0;
}
static int check_can_live_skill_break(player_struct *player, uint32_t type)
{
	if (player->data->live_skill.level[type] != player->data->live_skill.broken[type])
	{
		return 3;
	}

	LifeSkillTable *table1 = get_medicine_table(type, player->data->live_skill.level[type] + 1);
	if (table1 == NULL)
	{
		return 4;
	}
	LifeSkillTable *table = get_medicine_table(type, player->data->live_skill.level[type]);
	if (table == NULL)
	{
		return 2;
	}
	if (player->data->live_skill.exp[type] < table->Exp)
	{
		return 6;
	}
	if (player->get_attr(PLAYER_ATTR_LEVEL) < table->BreachLv)
	{
		return 190410005;
	}

	if (player->del_item(table->BreachItem, table->BreachNum, MAGIC_TYPE_LIVE_SKILL) < 0)
	{
		return 190410007;
	}
	return 0;
}
static int handle_live_skill_break_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}

	LearnLiveReq *req = learn_live_req__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint32_t type = req->type; //LIVE_SKILL_TYPE
	learn_live_req__free_unpacked(req, NULL);

	AnsLearnLiveSkill send;
	ans_learn_live_skill__init(&send);

	send.ret = check_can_live_skill_break(player, type);
	if (send.ret == 0)
	{
		LifeSkillTable *table = get_medicine_table(type, player->data->live_skill.level[type] + 1);
		if (table != NULL)
		{
			player->data->live_skill.broken[type] = table->LvMax;
		}
		send.type = type;

		table = get_medicine_table(type, player->data->live_skill.level[type]);
		while (player->data->live_skill.exp[type] >= table->Exp && table != NULL)
		{
			if (player->data->live_skill.level[type] >= player->data->live_skill.broken[type])
			{
				break;
			}
			++player->data->live_skill.level[type];
			player->data->live_skill.exp[type] -= table->Exp;
			table = get_medicine_table(type, player->data->live_skill.level[type]);
		}

		send.book = player->data->live_skill.book[type];
		send.lv = player->data->live_skill.level[type];
		send.exp = player->data->live_skill.exp[type];
		send.broken = player->data->live_skill.broken[type];
		//send.lv = player->data->live_skill.broken[type];
	}
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_LIVE_SKILL_BREAK_ANSWER, ans_learn_live_skill__pack, send);

	return 0;
}
static int handle_guild_prodece_medicine(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}
	PROTO_UNDO_COST *req = (PROTO_UNDO_COST*)buf_head();

	uint32_t type = req->cost.statis_id; //LIVE_SKILL_TYPE

	LifeSkillTable *table = get_medicine_table(type, player->data->live_skill.level[type]);
	if (table == NULL)
	{
		return -2;
	}

	uint32_t add = req->cost.gold * table->ExpAdd;
	AnsLearnLiveSkill send;
	ans_learn_live_skill__init(&send);
	send.ret = 0;
	if (req->cost.coin != 0)
	{
		player->add_coin(req->cost.coin * table->NeedCoin, MAGIC_TYPE_LIVE_SKILL, false);
		send.ret = 190410005;
		fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_LEARN_LIVE_SKILL_ANSWER, ans_learn_live_skill__pack, send);
		return -3;
	}

	send.type = type;
	player->data->live_skill.exp[type] += add;
	while (player->data->live_skill.exp[type] >= table->Exp && table != NULL)
	{
		if (player->data->live_skill.level[type] >= player->data->live_skill.broken[type])
		{
			break;
		}
		++player->data->live_skill.level[type];
		player->data->live_skill.exp[type] -= table->Exp;
		table = get_medicine_table(type, player->data->live_skill.level[type]);
	}
	send.book = player->data->live_skill.book[type];
	send.lv = player->data->live_skill.level[type];
	send.exp = player->data->live_skill.exp[type];

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_LEARN_LIVE_SKILL_ANSWER, ans_learn_live_skill__pack, send);

	return 0;
}


static int handle_add_speed_buff_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}
	buff_manager::create_default_buff(114400019, player, player, true);		
	return (0);
}
static int handle_del_speed_buff_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}
	player->clear_one_buff(114400019);	
	return (0);
}

static int handle_produce_medicine_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}

	LearnLiveReq *req = learn_live_req__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint32_t type = req->type; //LIVE_SKILL_TYPE
	learn_live_req__free_unpacked(req, NULL);

	LifeSkillTable *table = get_medicine_table(type, player->data->live_skill.level[type]);
	if (table == NULL)
	{
		return -2;
	}
	int ret = 0;
	
	uint64_t r = rand() % table->ProMax;
	uint64_t all = 0;
	uint32_t i = 0;
	AttrMap attrs;
	if (player->get_attr(PLAYER_ATTR_ENERGY) < table->NeedJingli)
	{
		ret = 190410001;
		goto done;
	}
	for (; i < table->n_LvPro; ++i)
	{
		all += table->LvPro[i];
		if (r <= all)
		{
			if (player->add_item(table->ItemID[i], 1, MAGIC_TYPE_LIVE_SKILL) != 0)
			{
				ret = 190410002;
				goto done;
			}
			break;
		}
	}
	player->set_attr(PLAYER_ATTR_ENERGY, player->get_attr(PLAYER_ATTR_ENERGY) - table->NeedJingli);

	attrs[PLAYER_ATTR_ENERGY] = player->get_attr(PLAYER_ATTR_ENERGY);
	player->notify_attr(attrs);

done:
	send_comm_answer(MSG_ID_PRODUCE_MEDICINE_ANSWER, ret, extern_data);

	return 0;
}

// static int handle_skill_move_request(player_struct *player, EXTERN_DATA *extern_data)
// {
//	if (comm_check_player_valid(player, extern_data->player_id) != 0)
//	{
//		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
//		return (-1);
//	}

//	SkillMoveRequest *req = skill_move_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
//	if (!req) {
//		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
//		return (-10);
//	}
//	float pos_x, pos_z;
//	pos_x = req->cur_pos->pos_x;
//	pos_z = req->cur_pos->pos_z;
//	skill_move_request__free_unpacked(req, NULL);

//		// TODO: 检查位移是否合法
//	player->set_pos_with_broadcast(pos_x, pos_z);

//	SkillMoveNotify notify;
//	skill_move_notify__init(&notify);
//	notify.playerid = player->get_uuid();
//	PosData cur_pos;
//	pos_data__init(&cur_pos);
//	cur_pos.pos_x = player->get_pos()->pos_x;
//	cur_pos.pos_z = player->get_pos()->pos_z;
//	notify.cur_pos = &cur_pos;
//	player->broadcast_to_sight(MSG_ID_SKILL_MOVE_NOTIFY, &notify, (pack_func)skill_move_notify__pack, true);
//	return (0);
// }

//1表示P1胜利，2表示P2胜利，3表示平局，0表示无结果
// int check_qiecuo_finished(player_struct *p1, player_struct *p2)
// {
// 	if (!p1->is_in_qiecuo() || !p1->is_qiecuo_target(p2))
// 		return (0);

// 	int ret = 0;
// 	if (!p1->is_alive())
// 	{
// 		p1->set_attr(PLAYER_ATTR_HP, 1);
// 		ret += 2;
// 	}

// 	if (!p2->is_alive())
// 	{
// 		p2->set_attr(PLAYER_ATTR_HP, 1);
// 		ret += 1;
// 	}

// 	if (ret != 0)
// 	{
// 		LOG_INFO("%s: player[%lu][%lu] qiecuo result %d", __FUNCTION__, p1->get_uuid(), p2->get_uuid(), ret);

// 		p1->clear_debuff();
// 		p2->clear_debuff();
// 		p1->finish_qiecuo();
// 		p2->finish_qiecuo();

// 		QiecuoFinishNotify nty;
// 		EXTERN_DATA ext;
// 		qiecuo_finish_notify__init(&nty);

// 		switch (ret)
// 		{
// 			case 1:
// 				nty.result = 0;
// 				ext.player_id = p1->get_uuid();
// 				fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_FINISH_NOTIFY, qiecuo_finish_notify__pack, nty);

// 				nty.result = 1;
// 				ext.player_id = p2->get_uuid();
// 				fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_FINISH_NOTIFY, qiecuo_finish_notify__pack, nty);
// 				break;
// 			case 2:
// 				nty.result = 1;
// 				ext.player_id = p1->get_uuid();
// 				fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_FINISH_NOTIFY, qiecuo_finish_notify__pack, nty);

// 				nty.result = 0;
// 				ext.player_id = p2->get_uuid();
// 				fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_FINISH_NOTIFY, qiecuo_finish_notify__pack, nty);
// 				break;
// 			case 3:
// 				nty.result = 2;
// 				ext.player_id = p1->get_uuid();
// 				fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_FINISH_NOTIFY, qiecuo_finish_notify__pack, nty);
// 				ext.player_id = p2->get_uuid();
// 				fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_FINISH_NOTIFY, qiecuo_finish_notify__pack, nty);
// 				break;
// 		}
// 	}


// 	return (ret);
// }

// static int handle_skill_call_attack_request(player_struct *player, EXTERN_DATA *extern_data)
// {
// 	if (comm_check_player_valid(player, extern_data->player_id) != 0)
// 	{
// 		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
// 		return (-1);
// 	}
// 		// TODO: 检查是客户端召唤类技能


// 	SkillCallAttackRequest *req = skill_call_attack_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
// 	if (!req) {
// 		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
// 		return (-10);
// 	}

// 	if (req->n_target_playerid != req->n_target_pos || req->n_target_playerid != req->n_hit_index) {
// 		LOG_ERR("%s %d: player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
// 		skill_call_attack_request__free_unpacked(req, NULL);
// 		return (-20);
// 	}

// 	int n_hit_effect = 0;
// 	int n_buff = 0;
// 	struct SkillLvTable *lv_config1, *lv_config2;
// 	struct PassiveSkillTable *pas_config;
// 	struct SkillTable *ski_config;
// 	skill_struct *skill_struct = player->m_skill.GetSkillStructFromFuwen(player->get_skill_id());
// //	uint32_t skill_lv = player->m_skill.GetSkillLevel(player->get_skill_id());
// //	if (skill_lv < 1)
// 	if (!skill_struct)
// 	{
// 		LOG_ERR("%s %d: player[%lu] skill[%u] no config", __FUNCTION__, __LINE__, extern_data->player_id, player->get_skill_id());
// 		skill_call_attack_request__free_unpacked(req, NULL);
// 		return (-30);
// 	}
// 		//检查CD
// 	if (time_helper::get_cached_time() < skill_struct->data->cd_time)
// 	{
// 		LOG_ERR("%s: %lu cast skill %u already in cd", __FUNCTION__, extern_data->player_id, req->skillid);
// 		skill_call_attack_request__free_unpacked(req, NULL);
// 		return (-40);
// 	}

// 	get_skill_configs(skill_struct->data->lv, player->get_skill_id(), &ski_config, &lv_config1, &pas_config, &lv_config2);
// //	if (!lv_config1 && !lv_config2)
// 	if (!lv_config1)
// 	{
// 		LOG_ERR("%s %d: player[%lu] skill[%u] no config", __FUNCTION__, __LINE__, extern_data->player_id, player->get_skill_id());
// 		skill_call_attack_request__free_unpacked(req, NULL);
// 		return (-50);
// 	}

// 	struct ActiveSkillTable *active_config = get_config_by_id(ski_config->SkillAffectId, &active_skill_config);
// 	if (!active_config)
// 	{
// 		LOG_ERR("%s: %lu skill %u not active_config", __FUNCTION__, extern_data->player_id, req->skillid);
// 		skill_call_attack_request__free_unpacked(req, NULL);
// 		return (-60);
// 	}

// 	player->set_pos_with_broadcast(req->attack_pos->pos_x, req->attack_pos->pos_z);

// 	std::vector<unit_struct *> sight_all;
// 	std::vector<unit_struct *> dead_all;
// 	sight_all.push_back(player);

// 	skill_struct->add_cd(lv_config1, active_config);
// 	// if (lv_config1->CD != 0)
// 	// 	skill_struct->data->cd_time = time_helper::get_cached_time() + lv_config1->CD - 200;
// 	// else
// 	// 	skill_struct->data->cd_time = time_helper::get_cached_time() + active_config->TotalSkillDelay - 200;

// 	uint32_t life_steal = 0;
// 	uint32_t damage_return = 0;

// 	for (size_t i = 0; i < req->n_target_playerid; ++i)
// 	{
// 		unit_struct *target = unit_struct::get_unit_by_uuid(req->target_playerid[i]);
// 		if (!target)
// 			continue;
// //		if (target->buff_state & BUFF_STATE_GOD)
// //			continue;
// //		if (!check_can_attack(player, target))
// 		UNIT_FIGHT_TYPE fight_type = get_unit_fight_type(player, target);
// 		if (fight_type != UNIT_FIGHT_TYPE_ENEMY)
// 		{
// 			LOG_ERR("%s %d: %lu can not attack unit %lu", __FUNCTION__, __LINE__, extern_data->player_id, target->get_uuid());
// 			continue;
// 		}

// 		if (!target->is_alive())
// 		{
// 			LOG_ERR("%s %d: %lu kill already dead unit %lu", __FUNCTION__, __LINE__, extern_data->player_id, target->get_uuid());
// 			continue;
// 		}

// 			//检查距离，场景
// 		if (player->scene != target->scene)
// 		{
// 			continue;
// 		}

// //		LOG_DEBUG("%s %d: player[%lu] attack target %lu", __FUNCTION__, __LINE__, extern_data->player_id, target->get_uuid());

// 		target->set_pos(req->target_pos[i]->pos_x, req->target_pos[i]->pos_z);
// 		sight_all.push_back(target);

// 		cached_hit_effect_point[n_hit_effect] = &cached_hit_effect[n_hit_effect];
// 		skill_hit_effect__init(&cached_hit_effect[n_hit_effect]);
// 		uint32_t add_num = 0;
// 		int32_t damage;

// 		int32_t other_rate = count_other_skill_damage_effect(player, target);
// 		damage = count_skill_total_damage(fight_type, ski_config,
// 			lv_config1, pas_config, lv_config2,
// 			player, target,
// 			&cached_hit_effect[n_hit_effect].effect,
// 			&cached_buff_id[n_buff],
// 			&add_num, other_rate);

// 		life_steal += player->count_life_steal_effect(damage);
// 		damage_return += player->count_damage_return(damage, target);

// 		raid_struct *raid = player->get_raid();
// 		if (raid)
// 		{
// 			raid->on_player_attack(player, target, damage);
// 		}

// 		player->on_attack(target);

// 		LOG_DEBUG("%s: unit[%lu][%p] damage[%d] hp[%f]", __FUNCTION__, target->get_uuid(), target, damage, target->get_attr(PLAYER_ATTR_HP));

// 		if (target->get_unit_type() == UNIT_TYPE_PLAYER)
// 		{
// 			check_qiecuo_finished(player, (player_struct *)target);
// 		}

// 		if (target->is_alive())
// 		{
// 			target->on_beattack(player, player->get_skill_id(), damage);
// 		}
// 		else
// 		{
// 			dead_all.push_back(target);
// 		}
// 		cached_target_index[n_hit_effect] = req->hit_index[i];

// 		cached_hit_effect[n_hit_effect].playerid = req->target_playerid[i];
// 		cached_hit_effect[n_hit_effect].n_add_buff = add_num;
// 		cached_hit_effect[n_hit_effect].add_buff = &cached_buff_id[n_buff];
// 		cached_hit_effect[n_hit_effect].hp_delta = damage;
// 		cached_hit_effect[n_hit_effect].cur_hp = target->get_attr(PLAYER_ATTR_HP);
// //		cached_hit_effect[n_hit_effect].attack_pos = &cached_attack_pos[n_hit_effect];
// 		cached_hit_effect[n_hit_effect].target_pos = &cached_target_pos[n_hit_effect];
// //		pos_data__init(&cached_attack_pos[n_hit_effect]);
// 		pos_data__init(&cached_target_pos[n_hit_effect]);
// //		cached_attack_pos[n_hit_effect].pos_x = player->get_pos()->pos_x;
// //		cached_attack_pos[n_hit_effect].pos_z = player->get_pos()->pos_z;
// 		cached_target_pos[n_hit_effect].pos_x = target->get_pos()->pos_x;
// 		cached_target_pos[n_hit_effect].pos_z = target->get_pos()->pos_z;
// 		n_buff += add_num;
// 		++n_hit_effect;
// 	}

// 	skill_call_attack_request__free_unpacked(req, NULL);

// 	if (!player->is_alive())
// 	{
// 		player->on_dead(player);
// 	}

// //	CommAnswer resp;
// //	comm_answer__init(&resp);
// //	uint32_t ret;

// //	FAST_SEND_TO_CLIENT(MSG_ID_CAST_SKILL_ANSWER, comm_answer__pack);
// //	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_SKILL_HIT_ANSWER, comm_answer__pack, resp);

// 	for (std::vector<unit_struct *>::const_iterator iter = dead_all.begin(); iter != dead_all.end(); ++iter)
// 	{
// 		unit_struct *target = (*iter);
// 			//怪物死亡的话清空视野
// 		target->on_dead(player);
// 	}

// 	SkillCallAttackNotify notify;
// 	skill_call_attack_notify__init(&notify);
// 	notify.playerid = extern_data->player_id;
// 	notify.skillid = player->get_skill_id();
// 	notify.n_target_player = n_hit_effect;
// 	notify.n_hit_index = n_hit_effect;
// 	notify.target_player = cached_hit_effect_point;
// 	notify.hit_index = cached_target_index;

// 	notify.attack_cur_hp = player->get_attr(PLAYER_ATTR_HP);
// 	notify.life_steal = life_steal;
// 	notify.damage_return = damage_return;

// 	PosData attack_pos;
// 	pos_data__init(&attack_pos);
// 	attack_pos.pos_x = player->get_pos()->pos_x;
// 	attack_pos.pos_z = player->get_pos()->pos_z;
// 	notify.attack_pos = &attack_pos;

// 	player->broadcast_to_many_sight(MSG_ID_CALL_ATTACK_NOTIFY, &notify, (pack_func)skill_call_attack_notify__pack, sight_all);

// 	return (0);
// }
/*
static void ttt(player_struct *player, uint64_t player_id, float target_pos_x, float target_pos_z)
{
	unit_struct *target = unit_struct::get_unit_by_uuid(player_id);
	if (!target)
		return;
	if (target->buff_state & BUFF_STATE_GOD)
		return;
	if (!check_can_attack(player, target))
	{
		LOG_ERR("%s %d: %lu can not attack unit %lu", __FUNCTION__, __LINE__, player->get_uuid(), target->get_uuid());
		return;
	}

	if (!target->is_alive())
	{
		LOG_ERR("%s %d: %lu kill already dead unit %lu", __FUNCTION__, __LINE__, player->get_uuid(), target->get_uuid());
		return;
	}

		//检查距离，场景
	if (player->scene != target->scene)
	{
		return;
	}

//		LOG_DEBUG("%s %d: player[%lu] attack target %lu", __FUNCTION__, __LINE__, extern_data->player_id, target->get_uuid());

	target->set_pos(target_pos_x, target_pos_z);
	sight_all.push_back(target);

	cached_hit_effect_point[n_hit_effect] = &cached_hit_effect[n_hit_effect];
	skill_hit_effect__init(&cached_hit_effect[n_hit_effect]);
	uint32_t add_num = 0;
	int32_t damage;

	damage = count_skill_total_damage(ski_config,
		lv_config1, pas_config, lv_config2,
		player, target,
		&cached_hit_effect[n_hit_effect].effect,
		&cached_buff_id[n_buff],
		&add_num);

	raid_struct *raid = player->get_raid();
	if (raid)
	{
		raid->on_player_attack(player, target, damage);
	}

	player->on_attack(target);

	LOG_DEBUG("%s: unit[%lu][%p] damage[%d] hp[%f]", __FUNCTION__, target->get_uuid(), target, damage, target->get_attr(PLAYER_ATTR_HP));

	if (target->is_alive())
	{
		target->on_beattack(player, player->data->skill.skill_id, damage);
	}
	else
	{
		dead_all.push_back(target);
	}

	cached_hit_effect[n_hit_effect].playerid = req->target_playerid[i];
	cached_hit_effect[n_hit_effect].n_add_buff = add_num;
	cached_hit_effect[n_hit_effect].add_buff = &cached_buff_id[n_buff];
	cached_hit_effect[n_hit_effect].hp_delta = damage;
	cached_hit_effect[n_hit_effect].cur_hp = target->get_attr(PLAYER_ATTR_HP);
//		cached_hit_effect[n_hit_effect].attack_pos = &cached_attack_pos[n_hit_effect];
	cached_hit_effect[n_hit_effect].target_pos = &cached_target_pos[n_hit_effect];
//		pos_data__init(&cached_attack_pos[n_hit_effect]);
	pos_data__init(&cached_target_pos[n_hit_effect]);
//		cached_attack_pos[n_hit_effect].pos_x = player->get_pos()->pos_x;
//		cached_attack_pos[n_hit_effect].pos_z = player->get_pos()->pos_z;
	cached_target_pos[n_hit_effect].pos_x = target->get_pos()->pos_x;
	cached_target_pos[n_hit_effect].pos_z = target->get_pos()->pos_z;
	n_buff += add_num;
	++n_hit_effect;
}
*/

static bool skill_can_attack(struct SkillTable *ski_config, UNIT_FIGHT_TYPE fight_type, player_struct *player, unit_struct *target)
{
	for (uint32_t i = 0; i < ski_config->n_TargetType; ++i)
	{
		switch (ski_config->TargetType[i])
		{
			case 1://敌方
				if (fight_type == UNIT_FIGHT_TYPE_ENEMY)
					return true;
				break;
			case 101://自身
				if (fight_type == UNIT_FIGHT_TYPE_MYSELF)
					return true;
				break;
			case 102://友方
				if (fight_type == UNIT_FIGHT_TYPE_FRIEND)
					return true;
				break;
		}
	}
	return false;
}

static int handle_skill_hit_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}

//	static SkillHitEffect cached_hit_effect[256];
//	static SkillHitEffect *cached_hit_effect_point[256];
//	static uint32_t cached_buff_id[512];

	if (player->get_skill_id() == 0)
		return (-1);

	SkillHitRequest *req = skill_hit_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	if (req->n_target_playerid != req->n_target_pos) {
		LOG_ERR("%s %d: player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		skill_hit_request__free_unpacked(req, NULL);
		return (-20);
	}

	if (player->check_pos_distance(req->attack_pos->pos_x, req->attack_pos->pos_z) != 0)
	{
		struct position *pos = player->get_pos();
		LOG_ERR("%s %d: player[%lu] cur_pos[%.1f][%.1f] flash to [%.1f][%.1f]", __FUNCTION__, __LINE__, player->get_uuid(),
			pos->pos_x, pos->pos_z, req->attack_pos->pos_x, req->attack_pos->pos_z);
	}

	int n_hit_effect = 0;
	int n_buff = 0;
	std::vector<unit_struct *> sight_all;
	std::vector<unit_struct *> dead_all;
	sight_all.push_back(player);

	struct SkillLvTable *lv_config1, *lv_config2;
	struct PassiveSkillTable *pas_config;
	struct SkillTable *ski_config;
	struct ActiveSkillTable *act_config;

//	uint32_t skill_lv = player->m_skill.GetSkillLevel(player->get_skill_id());
//	if (skill_lv < 1)
	if (player->is_in_buff3())
	{
		get_skill_configs(1, player->get_skill_id(), &ski_config, &lv_config1, &pas_config, &lv_config2, &act_config);		
	}
	else
	{
		skill_struct *skill_struct = player->m_skill.GetSkillStructFromFuwen(player->get_skill_id());
		if (!skill_struct)
		{
			LOG_ERR("%s %d: player[%lu] skill[%u] no config", __FUNCTION__, __LINE__, extern_data->player_id, player->get_skill_id());
			return (-25);
		}

		get_skill_configs(skill_struct->data->lv, player->get_skill_id(), &ski_config, &lv_config1, &pas_config, &lv_config2, &act_config);
	}

	if (!lv_config1 && !lv_config2)
	{
		LOG_ERR("%s %d: player[%lu] skill[%u] no config", __FUNCTION__, __LINE__, extern_data->player_id, player->get_skill_id());
		return (-30);
	}
	

	if (act_config && act_config->CanMove)
	{
			//旋风斩可以持续移动
	}
	else
	{
		player->set_pos_with_broadcast(req->attack_pos->pos_x, req->attack_pos->pos_z);
	}

	uint32_t life_steal = 0;
	uint32_t damage_return = 0;

	for (size_t i = 0; i < req->n_target_playerid; ++i)
	{
		unit_struct *target = unit_struct::get_unit_by_uuid(req->target_playerid[i]);
		if (!target)
			continue;
//		if (target->buff_state & BUFF_STATE_GOD)
//			continue;
//		if (!check_can_attack(player, target))

		if (!target->is_alive())
		{
			LOG_ERR("%s %d: %lu kill already dead unit %lu", __FUNCTION__, __LINE__, extern_data->player_id, target->get_uuid());
			continue;
		}

		UNIT_FIGHT_TYPE fight_type = get_unit_fight_type(player, target);
		if (!skill_can_attack(ski_config, fight_type, player, target))
		{
			LOG_ERR("%s: camp[%d] camp[%d] type[%d] type[%d] pktype[%d][%d] buff_state[%d] fight_type[%d] skill[%lu]",
				__FUNCTION__, player->get_camp_id(), target->get_camp_id(),
				get_entity_type(player->get_uuid()), get_entity_type(target->get_uuid()),
				(int)(player->get_attr(PLAYER_ATTR_PK_TYPE)), (int)(target->get_attr(PLAYER_ATTR_PK_TYPE)),
				target->buff_state, fight_type, ski_config->ID);
//			get_unit_fight_type(player, target);
			LOG_ERR("%s %d: %lu can not attack unit %lu", __FUNCTION__, __LINE__, extern_data->player_id, target->get_uuid());
			continue;
		}

			//检查距离，场景
		if (player->scene != target->scene)
		{
			continue;
		}

//		LOG_DEBUG("%s %d: player[%lu] attack target %lu", __FUNCTION__, __LINE__, extern_data->player_id, target->get_uuid());

		if (target->check_pos_distance(req->target_pos[i]->pos_x, req->target_pos[i]->pos_z) != 0)
		{
			struct position *pos = player->get_pos();
			LOG_ERR("%s %d: player[%lu] target[%lu] cur_pos[%.1f][%.1f] flash to [%.1f][%.1f]", __FUNCTION__, __LINE__,
				player->get_uuid(), target->get_uuid(),
				pos->pos_x, pos->pos_z, req->target_pos[i]->pos_x, req->target_pos[i]->pos_z);
		}

		target->set_pos(req->target_pos[i]->pos_x, req->target_pos[i]->pos_z);
		sight_all.push_back(target);

		cached_hit_effect_point[n_hit_effect] = &cached_hit_effect[n_hit_effect];
		skill_hit_effect__init(&cached_hit_effect[n_hit_effect]);
		uint32_t add_num = 0;
		int32_t damage;
		int32_t other_rate = count_other_skill_damage_effect(player, target);
		damage = count_skill_total_damage(fight_type, ski_config,
			lv_config1, pas_config, lv_config2,
			player, target,
			&cached_hit_effect[n_hit_effect].effect,
			&cached_buff_id[n_buff],
			&cached_buff_end_time[n_buff],
			&add_num, other_rate);

		life_steal += player->count_life_steal_effect(damage);
		damage_return += player->count_damage_return(damage, target);

		target->on_hp_changed(damage);

		raid_struct *raid = player->get_raid();
		if (raid)
		{
			raid->on_player_attack(player, target, damage);
		}

		player->on_attack(target);

		LOG_DEBUG("%s: unit[%lu][%p] damage[%d] hp[%f]", __FUNCTION__, target->get_uuid(), target, damage, target->get_attr(PLAYER_ATTR_HP));

		if (target->get_unit_type() == UNIT_TYPE_PLAYER)
		{
			check_qiecuo_finished(player, (player_struct *)target);
		}


		if (target->is_alive())
		{
			target->on_beattack(player, player->get_skill_id(), damage);
		}
		else
		{
			dead_all.push_back(target);
		}

		cached_hit_effect[n_hit_effect].playerid = req->target_playerid[i];
		cached_hit_effect[n_hit_effect].n_add_buff = add_num;
		cached_hit_effect[n_hit_effect].add_buff = &cached_buff_id[n_buff];
//		cached_hit_effect[n_hit_effect].add_buff_end_time = &cached_buff_end_time[n_buff];
		cached_hit_effect[n_hit_effect].hp_delta = damage;
		cached_hit_effect[n_hit_effect].cur_hp = target->get_attr(PLAYER_ATTR_HP);
//		cached_hit_effect[n_hit_effect].attack_pos = &cached_attack_pos[n_hit_effect];
		cached_hit_effect[n_hit_effect].target_pos = &cached_target_pos[n_hit_effect];
//		pos_data__init(&cached_attack_pos[n_hit_effect]);
		pos_data__init(&cached_target_pos[n_hit_effect]);
//		cached_attack_pos[n_hit_effect].pos_x = player->get_pos()->pos_x;
//		cached_attack_pos[n_hit_effect].pos_z = player->get_pos()->pos_z;
		cached_target_pos[n_hit_effect].pos_x = target->get_pos()->pos_x;
		cached_target_pos[n_hit_effect].pos_z = target->get_pos()->pos_z;
		n_buff += add_num;
		++n_hit_effect;
	}

	skill_hit_request__free_unpacked(req, NULL);

	player->on_hp_changed(damage_return);	

	if (!player->is_alive())
	{
		player->on_dead(player);
	}

//	CommAnswer resp;
//	comm_answer__init(&resp);
//	uint32_t ret;

//	FAST_SEND_TO_CLIENT(MSG_ID_CAST_SKILL_ANSWER, comm_answer__pack);
//	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_SKILL_HIT_ANSWER, comm_answer__pack, resp);

	for (std::vector<unit_struct *>::const_iterator iter = dead_all.begin(); iter != dead_all.end(); ++iter)
	{
		unit_struct *target = (*iter);
			//怪物死亡的话清空视野
		target->on_dead(player);
	}

	SkillHitNotify notify;
	skill_hit_notify__init(&notify);
	notify.playerid = extern_data->player_id;
	notify.owneriid = notify.playerid;
	notify.skillid = player->get_skill_id();
	notify.n_target_player = n_hit_effect;
	notify.target_player = cached_hit_effect_point;

	notify.attack_cur_hp = player->get_attr(PLAYER_ATTR_HP);
	notify.life_steal = life_steal;
	notify.damage_return = damage_return;

	PosData attack_pos;
	pos_data__init(&attack_pos);
	attack_pos.pos_x = player->get_pos()->pos_x;
	attack_pos.pos_z = player->get_pos()->pos_z;
	notify.attack_pos = &attack_pos;

	player->broadcast_to_many_sight(MSG_ID_SKILL_HIT_NOTIFY, &notify, (pack_func)skill_hit_notify__pack, sight_all);



	return (0);
}

static int handle_gather_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}

	if (player->scene == NULL)
	{
		return -3;
	}

	StartCollect *req = start_collect__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	Collect *pCollect = Collect::GetById(req->id);
	uint32_t reqid = req->id;
	uint32_t step = req->step;
	start_collect__free_unpacked(req, NULL);

	LOG_DEBUG("%s: player[%lu] collectid[%u]", __FUNCTION__, extern_data->player_id, reqid);

	if (pCollect == NULL)
	{
		LOG_ERR("%s %d: can not player[%lu] find collect %u", __FUNCTION__, __LINE__, extern_data->player_id, reqid);
		return -11;
	}
	//player->interrupt();
	int ret = pCollect->BegingGather( player , step);
	if (ret != 0)
	{
		send_comm_answer(MSG_ID_COLLECT_BEGIN_ANSWER, ret, extern_data);
		return 0;
	}

	NotifyCollect send;
	notify_collect__init(&send);
	send.playerid = player->get_uuid();
	send.collectid = pCollect->m_uuid;

	player->broadcast_to_sight(MSG_ID_COLLECT_BEGIN_NOTIFY, &send, (pack_func)notify_collect__pack, true);

	return 0;
}
static int handle_gather_complete(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player)
		return (-1);

	if (!player->is_avaliable())
	{
		LOG_ERR("%s: player[%lu] do not have scene or data", __FUNCTION__, extern_data->player_id);
		return -10;
	}

	//CollectId *req = collect_id__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	//if (!req) {
	//	LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
	//	return (-10);
	//}
	//uint32_t reqid = req->id;

	//Collect *pCollect = Collect::GetById(req->id);
	//collect_id__free_unpacked(req, NULL);

	LOG_DEBUG("%s: player[%lu] collectid[%u]", __FUNCTION__, extern_data->player_id, player->data->m_collect_uuid);
	Collect *pCollect = Collect::GetById(player->data->m_collect_uuid);
	if (pCollect == NULL)
	{
		LOG_ERR("%s %d: can not player[%lu] find collect %u", __FUNCTION__, __LINE__, extern_data->player_id, player->data->m_collect_uuid);
		return -11;
	}
	int ret = pCollect->GatherComplete(player);
	send_comm_answer(MSG_ID_COLLECT_COMMPLETE_ANSWER, ret, extern_data);
	if (ret == 0)
	{
		player->scene->on_collect(player, pCollect);
	}
	return 0;
}
static int handle_gather_interupt(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player)
		return (-1);

	if (!player->is_avaliable())
	{
		LOG_ERR("%s: player[%lu] do not have scene or data", __FUNCTION__, extern_data->player_id);
		return -10;
	}

	CollectId *req = collect_id__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	Collect *pCollect = Collect::GetById(req->id);
	collect_id__free_unpacked(req, NULL);
	if (pCollect == NULL)
	{
		return -11;
	}
	pCollect->GatherInterupt(player);

	return 0;
}

int handle_chat_no_check(player_struct *player, EXTERN_DATA *extern_data, Chat *req)
{
	if (req->channel == CHANNEL__private)
	{
		//私聊转发到好友服处理
		size_t data_len = get_data_len();
		uint8_t *pSendData = get_send_data();
		memcpy(pSendData, get_data(), data_len);
		fast_send_msg_base(&conn_node_gamesrv::connecter, extern_data, SERVER_PROTO_FRIEND_CHAT, data_len, 0);
		return 0;

		if (req->has_recvplayerid)
		{
			player_struct *target_player = player_manager::get_player_by_id(req->recvplayerid);
			if (!target_player || !target_player->data)
			{
				return CHAR_RET_CODE__offLine;
			}
			memcpy(conn_node_gamesrv::connecter.get_send_data(), get_data(), get_data_len());
			fast_send_msg_base(&conn_node_gamesrv::connecter, extern_data, MSG_ID_CHAT_NOTIFY, get_data_len(), 0);

			EXTERN_DATA * pTargetData = (EXTERN_DATA *)(conn_node_gamesrv::connecter.get_send_data() + get_data_len());
			pTargetData->player_id = target_player->get_uuid();
			conn_node_gamesrv::connecter.send_one_msg((PROTO_HEAD *)(conn_node_gamesrv::connecter.global_send_buf), 1);
		}
	}
	else if (req->channel == CHANNEL__world)
	{
		ParameterTable *table = get_config_by_id(161000248, &parameter_config);
		if (table == NULL)
		{
			return 2;
		}
		if (player->get_attr(PLAYER_ATTR_LEVEL) < table->parameter1[0])
		{
			return 190400002;
		} 
		else if (player->data->world_chat_cd > time_helper::get_cached_time() / 1000)
		{
			return 190400003;
		}
		player->data->world_chat_cd = time_helper::get_cached_time() / 1000 + table->parameter1[1];
		conn_node_gamesrv::send_to_all_player(MSG_ID_CHAT_NOTIFY, req, (pack_func)chat__pack);
	}
	else if (req->channel == CHANNEL__zhaomu)
	{
		conn_node_gamesrv::send_to_all_player(MSG_ID_CHAT_NOTIFY, req, (pack_func)chat__pack);
	}
	else if (req->channel == CHANNEL__area)
	{
		player->broadcast_to_sight(MSG_ID_CHAT_NOTIFY, req, (pack_func)chat__pack, true);
		if (sg_gm_cmd_open > 0)
		{
			int argc = 0;
			char *argv[10];
			chat_mod::parse_cmd(req->contain, &argc, argv);
			chat_mod::do_gm_cmd(player, argc, argv);
		}
	}
	else if (req->channel == CHANNEL__team)
	{
		if (player->m_team == NULL)
		{
			return 0;
		}
		player->m_team->BroadcastToTeam(MSG_ID_CHAT_NOTIFY, req, (pack_func)chat__pack);
	}
	else if (req->channel == CHANNEL__menpai)
	{
		PROTO_HEAD_CONN_BROADCAST *head;
		PROTO_HEAD *real_head;

		/** ================广播数据============ **/
		head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
		head->msg_id = ENDION_FUNC_2(SERVER_PROTO_BROADCAST);
		real_head = &head->proto_head;

		real_head->msg_id = ENDION_FUNC_2(MSG_ID_CHAT_NOTIFY);
		real_head->seq = 0;
		memcpy(real_head->data, get_data(), get_data_len());
		real_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + get_data_len());
		uint64_t *ppp = (uint64_t*)((char *)&head->player_id + get_data_len());
		int player_num = 0;
		//todo 先遍历所有玩家 以后换成相同职业的集合
		std::map<uint64_t, player_struct *>::iterator it = player_manager_all_players_id.begin();
		for (; it != player_manager_all_players_id.end(); ++it, ++player_num)
		{
			if (it->second->get_job() != player->get_job())
			{
				continue;
			}
			if (get_entity_type(it->second->get_uuid()) == ENTITY_TYPE_AI_PLAYER)
				continue;
			ppp[player_num] = it->second->get_uuid();
		}
		head->num_player_id = player_num;
		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD_CONN_BROADCAST) + get_data_len() + sizeof(uint64_t) * head->num_player_id);

		if (conn_node_gamesrv::connecter.send_one_msg((PROTO_HEAD *)head, 1) != (int)(ENDION_FUNC_4(head->len))) {
			LOG_ERR("%s %d: send to all failed err[%d]", __FUNCTION__, __LINE__, errno);
		}
	}
	else if (req->channel == CHANNEL__group)
	{
		if (player->get_attr(PLAYER_ATTR_ZHENYING) == 0)
		{
			return -1;
		}
		PROTO_HEAD_CONN_BROADCAST *head;
		PROTO_HEAD *real_head;

		/** ================广播数据============ **/
		head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
		head->msg_id = ENDION_FUNC_2(SERVER_PROTO_BROADCAST);
		real_head = &head->proto_head;

		real_head->msg_id = ENDION_FUNC_2(MSG_ID_CHAT_NOTIFY);
		real_head->seq = 0;
		memcpy(real_head->data, get_data(), get_data_len());
		real_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + get_data_len());
		uint64_t *ppp = (uint64_t*)((char *)&head->player_id + get_data_len());
		int player_num = 0;
		//todo 先遍历所有玩家 以后换成相同职业的集合
		std::map<uint64_t, player_struct *>::iterator it = player_manager_all_players_id.begin();
		for (; it != player_manager_all_players_id.end(); ++it, ++player_num)
		{
			if (it->second->get_attr(PLAYER_ATTR_ZHENYING) != player->get_attr(PLAYER_ATTR_ZHENYING))
			{
				continue;
			}
			if (get_entity_type(it->second->get_uuid()) == ENTITY_TYPE_AI_PLAYER)
				continue;
			ppp[player_num] = it->second->get_uuid();
		}
		head->num_player_id = player_num;
		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD_CONN_BROADCAST) + get_data_len() + sizeof(uint64_t) * head->num_player_id);

		if (conn_node_gamesrv::connecter.send_one_msg((PROTO_HEAD *)head, 1) != (int)(ENDION_FUNC_4(head->len))) {
			LOG_ERR("%s %d: send to all failed err[%d]", __FUNCTION__, __LINE__, errno);
		}
	}
	else if (req->channel == CHANNEL__family)
	{
		//帮会聊天转发到帮会服处理
		fast_send_msg(&conn_node_gamesrv::connecter, extern_data, SERVER_PROTO_GUILD_CHAT, chat__pack, *req);
	}

	return (0);
}
static int handle_team_speek_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player)
		return (-1);

	if (!player->is_avaliable())
	{
		LOG_ERR("%s: player[%lu] do not have scene or data", __FUNCTION__, extern_data->player_id);
		return -10;
	}

	Chat *req = chat__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	ParameterTable *table = get_config_by_id(161000005, &parameter_config);
	TeamNotifyCd note;
	team_notify_cd__init(&note);
	note.cd = 0;
	if (req->channel >= MAX_CHANNEL)
	{
		goto done;
	}
	if (player->m_team == NULL)
	{
		note.cd = player->m_team->m_data->speekCd[req->channel] - (time_t)time_helper::get_cached_time() / 1000;
		goto done;
	}
	if (player->m_team->m_data->speekCd[req->channel] > (time_t)time_helper::get_cached_time() / 1000)
	{
		note.cd = player->m_team->m_data->speekCd[req->channel] - (time_t)time_helper::get_cached_time() / 1000;
		goto done;
	}
	
	if (table != NULL)
	{
		player->m_team->m_data->speekCd[req->channel] = time_helper::get_cached_time() / 1000 + table->parameter1[req->channel];
	}
	
	handle_chat_no_check(player, extern_data, req);
	player->m_team->NotityXiayi();
done:
	chat__free_unpacked(req, NULL);
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TEAM_SPEEK_CD_NOTIFY, team_notify_cd__pack, note);

	return (0);
}
static int handle_chat_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player)
		return (-1);

	if (!player->is_avaliable())
	{
		LOG_ERR("%s: player[%lu] do not have scene or data", __FUNCTION__, extern_data->player_id);
		return -10;
	}

	Chat *req = chat__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	AnsChat send;
	ans_chat__init(&send);
	send.ret = handle_chat_no_check(player, extern_data, req);
	if (send.ret == 190400003)
	{
		send.cd = player->data->world_chat_cd - time_helper::get_cached_time() / 1000;
	}
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_CHAT_ANSWER, ans_chat__pack, send);
	chat__free_unpacked(req, NULL);

	return (0);
}

static int handle_partner_skill_cast_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		send_comm_answer(MSG_ID_PARTNER_SKILL_CAST_ANSWER, -1, extern_data);				
		return (-1);
	}

	if (player->get_attr(PLAYER_ATTR_PARTNER_ANGER) + __DBL_EPSILON__ < sg_partner_anger_max)
	{
		LOG_ERR("%s: %lu anger not enough", __FUNCTION__, extern_data->player_id);
		send_comm_answer(MSG_ID_PARTNER_SKILL_CAST_ANSWER, -10, extern_data);				
		return (-10);		
	}

	partner_struct *partner = player->get_battle_partner();
	if (!partner)
	{
		LOG_ERR("%s: %lu do not have partner", __FUNCTION__, extern_data->player_id);
		send_comm_answer(MSG_ID_PARTNER_SKILL_CAST_ANSWER, -20, extern_data);				
		return (-20);		
	}
		 
	uint32_t skill_id = partner->config->Angerskill;
	unit_struct *target = partner->ai->choose_target(partner);
	if (!target)
	{
		send_comm_answer(MSG_ID_PARTNER_SKILL_CAST_ANSWER, 190500320, extern_data);		
		return (0);
	}

	player->reset_partner_anger(true);
	partner->attack_target(skill_id, -1, target);
	send_comm_answer(MSG_ID_PARTNER_SKILL_CAST_ANSWER, 0, extern_data);
	return (0);
}

static int handle_skill_cast_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		CommAnswer resp;
		comm_answer__init(&resp);
		resp.result = -1;
		fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_SKILL_CAST_ANSWER, comm_answer__pack, resp);
		return (-1);
	}

	// if (player->buff_state & BUFF_STATE_STUN)
	// {
	//	LOG_ERR("%s: %lu is in lock state", __FUNCTION__, extern_data->player_id);
	//	return (-10);
	// }
	// if (player->buff_state & BUFF_STATE_TAUNT)
	// {
	//	LOG_ERR("%s: %lu is in taunt state", __FUNCTION__, extern_data->player_id);
	//	return (-20);
	// }

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

	if (!player->is_in_buff3())
	{
		skill_struct *skill_struct = player->m_skill.GetSkillStructFromFuwen(req->skillid);
		if (!skill_struct)
		{
			LOG_ERR("%s: %lu cast skill %u but not learned", __FUNCTION__, extern_data->player_id, req->skillid);
			skill_cast_request__free_unpacked(req, NULL);
			return (-50);
		}

			//检查CD
		if (time_helper::get_cached_time() < skill_struct->data->cd_time)
		{
			LOG_ERR("%s: %lu cast skill %u already in cd[%lu]", __FUNCTION__, extern_data->player_id, req->skillid, skill_struct->data->cd_time);
			skill_cast_request__free_unpacked(req, NULL);
			send_comm_answer(MSG_ID_SKILL_CAST_ANSWER, -2, extern_data);
			return (-55);
		}

		struct ActiveSkillTable *active_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
		if (!active_config)
		{
			LOG_ERR("%s: %lu skill %u not active_config", __FUNCTION__, extern_data->player_id, req->skillid);
			skill_cast_request__free_unpacked(req, NULL);
			return (-60);
		}

		struct SkillLvTable *lv_config = get_config_by_id(config->SkillLv + skill_struct->data->lv - 1, &skill_lv_config);
		if (!lv_config)
		{
			LOG_ERR("%s: %lu skill %u lv[%u] don't have lv_config", __FUNCTION__, extern_data->player_id, req->skillid, skill_struct->data->lv);
			skill_cast_request__free_unpacked(req, NULL);
			return (-70);
		}

		skill_struct->add_cd(lv_config, active_config);
		LOG_INFO("%s: %lu cast skill %u cd[%lu]", __FUNCTION__, extern_data->player_id, req->skillid, skill_struct->data->cd_time);
	}
	// if (lv_config->CD != 0)
	// 	skill_struct->data->cd_time = time_helper::get_cached_time() + lv_config->CD - 200;
	// else
	// 	skill_struct->data->cd_time = time_helper::get_cached_time() + active_config->TotalSkillDelay - 200;

//	LOG_DEBUG("%s: %lu cast %u, cd %lu", __FUNCTION__, extern_data->player_id, req->skillid, skill_struct->data->cd_time);

	if (player->check_pos_distance(req->cur_pos->pos_x, req->cur_pos->pos_z) != 0)
	{
		struct position *pos = player->get_pos();
		LOG_ERR("%s %d: player[%lu] cur_pos[%.1f][%.1f] flash to [%.1f][%.1f]", __FUNCTION__, __LINE__, player->get_uuid(),
			pos->pos_x, pos->pos_z, req->cur_pos->pos_x, req->cur_pos->pos_z);
	}

	player->set_pos_with_broadcast(req->cur_pos->pos_x, req->cur_pos->pos_z);
	player->data->cur_skill.skill_id = req->skillid;
	player->data->cur_skill.direct_x = req->direct_x;
	player->data->cur_skill.direct_z = req->direct_z;
	player->data->cur_skill.start_time = time_helper::get_cached_time();

	SkillCastNotify notify;
	PosData pos_data;
	struct position *pos = player->get_pos();
	pos_data__init(&pos_data);
	pos_data.pos_x = pos->pos_x;
	pos_data.pos_z = pos->pos_z;
	skill_cast_notify__init(&notify);
	notify.playerid = extern_data->player_id;
	notify.skillid = req->skillid;
	notify.cur_pos = &pos_data;
	notify.direct_x = req->direct_x;
	notify.direct_z = req->direct_z;

	skill_cast_request__free_unpacked(req, NULL);
	player->interrupt();

	player->broadcast_to_sight(MSG_ID_SKILL_CAST_NOTIFY, &notify, (pack_func)skill_cast_notify__pack, true);

	if (config->IsMonster)
	{
		monster_manager::create_call_monster(player, config);
	}

	player->clear_god_buff();

	return (0);
}

static int on_login_send_team_task(player_struct *player, EXTERN_DATA *extern_data)
{
	if (player->m_team)
	{
		player_struct *leader = player->m_team->GetLead();
		if (leader)
		{
			for (int i = 0; i < MAX_TASK_ACCEPTED_NUM; ++i)
			{
				TaskInfo *task = &leader->data->task_list[i];
				if (task->id == 0 || !task_is_team(task->id))
				{
					continue;
				}

				player->task_update_notify(task);
			}
		}
	}
	return 0;
}

static int on_login_send_yaoshi(player_struct *player, EXTERN_DATA *extern_data);
static int on_login_send_auto_add_hp_data(player_struct *player, EXTERN_DATA *extern_data);
static int notify_jijiangopen_gift_info(player_struct* player, EXTERN_DATA* extern_data);
//玩家上线处理
int pack_player_online(player_struct *player, EXTERN_DATA *extern_data, bool load_db, bool reconnect)
{
	EnterGameAnswer resp;
	enter_game_answer__init(&resp);

	if (!player)
	{
		resp.result = 1;
		LOG_DEBUG("[%s:%d] result[%d] player_id[%lu] scene[%u]", __FUNCTION__, __LINE__, resp.result, extern_data->player_id, resp.sceneid);
		fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_ENTER_GAME_ANSWER, enter_game_answer__pack, resp);
		return (0);
	}

	if (player->data->n_horse == 0)
	{
		player->data->attrData[PLAYER_ATTR_CUR_HORSE] = DEFAULT_HORSE;
	}

		//如果是从数据库load数据的，要初始化玩家数据
	if (load_db)
	{
		if (player->data->attrData[PLAYER_ATTR_BAGUA] == 0)
		{
			player->data->attrData[PLAYER_ATTR_BAGUA] = 1;
		}
		player->calculate_attribute();
		player->create_item_cache();
		player->fit_bag_grid_num();
		player->init_head_icon();
		player->load_task_end();
		player->login_check_task_time();
		player->check_task_time();
		player->init_yuqidao_mai(0, false);
		player->clear_team_task();
		player->load_partner_end();
	}

	if (player->data->attrData[PLAYER_ATTR_HP] < __DBL_EPSILON__)
	{
		player->data->attrData[PLAYER_ATTR_HP] = player->data->attrData[PLAYER_ATTR_MAXHP];
	}

	if (player->data->teamid != 0)
	{
		player->m_team = Team::GetTeam(player->data->teamid);
		if (player->m_team == NULL)
		{
			player->data->teamid = 0;
		}
		else
		{
			if (!player->m_team->MemberOnLine(*player))
			{
				player->m_team = NULL;
				player->data->teamid = 0;
			}
		}
	}
	position *pos = player->get_pos();
	LOG_DEBUG("[%s:%d] player_id[%lu][%s] scene[%u] posx[%f] posz[%f]", __FUNCTION__, __LINE__, extern_data->player_id, player->data->name, player->data->scene_id, pos->pos_x, pos->pos_z);
	player->try_return_raid();
	//如果下线的时候是在帮会地图，重新检查
	if (is_guild_scene_id(player->data->scene_id))
	{
		//如果已经离帮，把玩家拉出地图
		if (player->data->guild_id == 0)
		{
			player->data->scene_id = player->data->last_scene_id;
			scene_struct *scene = scene_manager::get_scene(player->data->scene_id);
			if (scene)
			{
				player->set_pos(scene->m_born_x, scene->m_born_z);
			}
		}
	}

#ifndef NO_NEWRAID
	//如果是新手直接传入新手副本
	if (player->data->noviceraid_flag == 0)
	{
		raid_struct *raid = raid_manager::create_raid(20035, player);
		if (raid != NULL)
		{	
			//player->scene = raid;
			//player->data->scene_id = 20035;//->m_id;
			//player->set_pos(-73, 115);
			//player->scene = raid;
			player->data->scene_id = raid->res_config->SceneID;
			//player->set_pos(raid->res_config->BirthPointX, raid->res_config->BirthPointZ);
			player->set_enter_raid_pos_and_scene(raid, raid->res_config->BirthPointX, raid->res_config->BirthPointZ);
			raid->set_player_info(player, &raid->data->player_info[0]);
			raid->player_return_raid(player);
				//设置一下新手副本离开的位置
			player->conserve_out_raid_pos_and_scene(raid);
//			buff_manager::create_buff(114400018, player, player, true);
			//player->data->pos_y = raid->res_config->BirthPointY;
			
		}
	}
#endif // !NO_NEWRAID

	if (!reconnect)
	{
		AttrMap attrs;
		for (int i = 1; i < PLAYER_ATTR_MAX; ++i)
		{
			attrs[i] = player->data->attrData[i];
		}
		player->notify_attr(attrs);
			//		notify_head_icon_info(player, extern_data);
		player->data->login_notify = false;
	}

//	float direct = 0.0;
//	get_scene_birth_pos(player->data->scene_id, NULL, NULL, NULL, &direct);
	float direct = c_angle_to_unity_angle(player->data->m_angle);

	player->m_skill.Init();

	player->send_raid_earning_time_notify();
	resp.result = 0;
	resp.posx = player->get_pos()->pos_x;
	resp.posz = player->get_pos()->pos_z;
	resp.posy = player->data->pos_y;
	resp.sceneid = player->data->scene_id;
	resp.curtime = time_helper::get_cached_time() / 1000;
	resp.curhorse = player->data->attrData[PLAYER_ATTR_CUR_HORSE];
	resp.curhorsestate = player->data->attrData[PLAYER_ATTR_ON_HORSE_STATE];
	resp.fly = player->data->horse_attr.fly;
	resp.direct = direct;

	LOG_DEBUG("[%s:%d] result[%d] player_id[%lu] scene[%u] posx[%f] posz[%f]", __FUNCTION__, __LINE__, resp.result, extern_data->player_id, resp.sceneid, resp.posx, resp.posz);

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_ENTER_GAME_ANSWER, enter_game_answer__pack, resp);

	//放在MSG_ID_ENTER_GAME_ANSWER后面，防止客户端出错
	if (reconnect)
	{ //如果是断线重连，不再通知玩家数据
		player->data->login_notify = true;
		player_ready_enter_scene(player, extern_data, true);
		player_online_to_other_srvs(player, extern_data, true);
	}

	ChengJieTaskManage::NotifyTargetLogin(player);
	player->clear_award_question();

	//更新玩家redis数据
	player->refresh_player_redis_info();
	return (0);
}

//玩家登陆时进入场景完成
static int player_online_enter_scene_after(player_struct *player, EXTERN_DATA *extern_data)
{
	on_login_send_auto_add_hp_data(player, extern_data);
	on_login_send_yaoshi(player, extern_data);
	notify_task_list(player, extern_data);
	notify_yuqidao_info(player, extern_data);
	notify_pvp_raid_info(player, -1, extern_data);
	notify_setting_switch_info(player, extern_data);
	notify_transfer_out_stuck_info(player, extern_data);
	notify_partner_info(player, extern_data);
	player->notify_fighting_partner();
	notify_jijiangopen_gift_info(player, extern_data);
	player->notify_activity_info(extern_data);
	on_login_send_team_task(player, extern_data);

	player->data->login_notify = true;

	player_online_to_other_srvs(player, extern_data, false);

	return 0;
}

static void player_online_to_other_srvs(player_struct *player, EXTERN_DATA *extern_data, bool reconnect)
{
	//获取玩家离线缓存
	fast_send_msg_base(&conn_node_gamesrv::connecter, extern_data, SERVER_PROTO_GET_OFFLINE_CACHE_REQUEST, 0, 0);
	
	//用户上线通知的其他服务器
	uint8_t *pData = get_send_data();
	*pData++ = reconnect;
	fast_send_msg_base(&conn_node_gamesrv::connecter, extern_data, SERVER_PROTO_PLAYER_ONLINE_NOTIFY, sizeof(uint8_t), 0);
}

static int check_can_transfer_to_player(player_struct *player, player_struct *target_player)
{
	if (player->is_in_raid())
	{
		LOG_ERR("%s: %lu is in raid", __FUNCTION__, player->get_uuid());
		return 190500045;
	}

	if (player->sight_space)
	{
		LOG_ERR("%s: %lu is in sightspace %p", __FUNCTION__, player->get_uuid(), player->sight_space);
		return 190500045;
	}
	if (player->data->truck.truck_id != 0)
	{
		LOG_ERR("%s: %lu is in yabiao %lu", __FUNCTION__, player->get_uuid(), player->data->truck.truck_id);
		return 190500305;
	}

	if (target_player->scene == player->scene)
	{
		LOG_INFO("%s: %lu target player[%lu] in the same scene", __FUNCTION__, player->get_uuid(), target_player->get_uuid());
		return 190500142;
	}

	if (target_player->check_can_transfer() != 0)
	{
		LOG_INFO("%s: %lu target player[%lu] is in raid", __FUNCTION__, player->get_uuid(), target_player->get_uuid());
		return 190500143;
	}
	return 0;
}
static int handle_transfer_to_player_scene_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		send_comm_answer(MSG_ID_TRANSFER_TO_PLAYER_SCENE_ANSWER, -1, extern_data);
		return (-1);
	}

	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);

	TransferToPlayerSceneRequest *req = transfer_to_player_scene_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint64_t target_id = req->player_id;
	transfer_to_player_scene_request__free_unpacked(req, NULL);

	player_struct *target_player = player_manager::get_player_by_id(target_id);
	if (!target_player || !target_player->is_online() || !target_player->scene)
	{
		LOG_INFO("%s: %lu target player[%lu] not online", __FUNCTION__, extern_data->player_id, target_id);
		send_comm_answer(MSG_ID_TRANSFER_TO_PLAYER_SCENE_ANSWER, 190500144, extern_data);
		return (0);
	}

	int ret = check_can_transfer_to_player(player, target_player);
	if (ret != 0)
	{
		send_comm_answer(MSG_ID_TRANSFER_TO_PLAYER_SCENE_ANSWER, ret, extern_data);
		return -3;
	}

	scene_struct *new_scene = target_player->scene;

	player->transfer_to_new_scene(new_scene->m_id, new_scene->m_born_x,
		new_scene->m_born_y, new_scene->m_born_z, new_scene->m_born_direct, extern_data);
	return (0);
}

static int handle_transfer_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		SceneTransferAnswer resp;
		scene_transfer_answer__init(&resp);
		resp.result = -1;
		fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TRANSFER_ANSWER, scene_transfer_answer__pack, resp);
		return (-1);
	}

	int ret = player->check_can_transfer();
	if (ret != 0)
	{
		SceneTransferAnswer resp;
		scene_transfer_answer__init(&resp);
		resp.result = ret;
		fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TRANSFER_ANSWER, scene_transfer_answer__pack, resp);
		return (-10);
	}

	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);

	SceneTransferRequest *req = scene_transfer_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-20);
	}

	uint32_t id = req->transfer_id;
	uint32_t type = req->type;
/*
	if (scene_id == player->data->scene_id)
	{
		LOG_ERR("%s %d: player[%lu] transfer to the same scene[%u]", __FUNCTION__, __LINE__, extern_data->player_id, scene_id);
		return (-20);
	}
*/
	scene_transfer_request__free_unpacked(req, NULL);

	if (!player->scene->can_transfer(type))
	{
		LOG_ERR("%s player[%lu] can not transfer type[%u] scene[%u]", __FUNCTION__, extern_data->player_id, type, player->scene->m_id);
		SceneTransferAnswer resp;
		scene_transfer_answer__init(&resp);
		resp.result = ERROR_ID_CAN_NOT_TRANSFER;
		fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TRANSFER_ANSWER, scene_transfer_answer__pack, resp);
		return (-30);
	}

	//帮会地图传送
	if (type == 8)
	{
		int ret = player->transfer_to_guild_scene(extern_data);
		if (ret != 0)
		{
			SceneTransferAnswer resp;
			scene_transfer_answer__init(&resp);
			resp.result = ret;
			fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TRANSFER_ANSWER, scene_transfer_answer__pack, resp);
		}
		return (0);
	}

	player->transfer_to_new_scene_by_config(id, extern_data);
/*
	scene_struct *scene = scene_manager::get_scene(scene_id);
	if (!scene)
	{
		LOG_ERR("%s %d: player[%lu] transfer to the wrong scene[%u]", __FUNCTION__, __LINE__, extern_data->player_id, scene_id);
		return (-30);
	}

	player->scene->delete_player_from_scene(player);
	player->data->scene_id = scene_id;
	player->set_pos(scene->m_born_x, scene->m_born_z);
	if (scene_id == player->data->scene_id)
	{
		scene->add_player_to_scene(player);
	}

	SceneTransferAnswer resp;
	scene_transfer_answer__init(&resp);
	resp.direct = scene->m_born_direct;
	resp.new_scene_id = scene_id;
	resp.pos_x = scene->m_born_x;
	resp.pos_y = scene->m_born_y;
	resp.pos_z = scene->m_born_z;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TRANSFER_ANSWER, scene_transfer_answer__pack, resp);
	player->interrupt();
*/
	return (0);
}

static int handle_enter_scene_ready_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (player->scene && player->scene->get_scene_type() == SCENE_TYPE_WILD)
	{
		LOG_ERR("%s: player[%lu] already in scene %u", __FUNCTION__, player->data->player_id, player->scene->m_id);
		return (0);
	}

	if (player->area || player->sight_space)
	{
		LOG_ERR("%s %d: player[%lu] already in scene %u", __FUNCTION__, __LINE__, player->data->player_id, player->scene->m_id);
		return (0);
	}


	player->send_buff_info();
	player_ready_enter_scene(player, extern_data, false);
	player->data->xunbao.door_map = 0;
	player->data->xunbao.door_id = 0;
	player->data->xunbao.cd = 0;
	if (player->m_team != NULL)
	{
		player->m_team->SendXunbaoPoint(*player);
	}

	return (0);
}

//前端进场景loading完，后端把玩家加入场景
static void player_ready_enter_scene(player_struct *player, EXTERN_DATA *extern_data, bool reconnect)
{
	bool bSuccess = false;
	do
	{
		raid_struct *raid = player->get_raid();
		if (raid)
		{
			raid->add_player_to_scene(player);
			if (player->data->noviceraid_flag == 0 && raid->res_config->SceneID == 20035 && get_entity_type(player->get_uuid()) == ENTITY_TYPE_PLAYER)
			{
					//为了通知客户端隐藏副本UI
				EXTERN_DATA extern_data;
				extern_data.player_id = player->get_uuid();

				EnterRaidNotify notify;
				enter_raid_notify__init(&notify);
				notify.raid_id = raid->m_id;
				fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ENTER_RAID_NOTIFY, enter_raid_notify__pack, notify);
//				player->send_scene_transfer(raid->res_config->FaceY, raid->res_config->BirthPointX, raid->res_config->BirthPointY,
//					raid->res_config->BirthPointZ, raid->m_id, 0);
			}
			if (raid->ai && raid->ai->raid_on_player_ready)
				raid->ai->raid_on_player_ready(raid, player);
			//player->set_attr(PLAYER_ATTR_PK_TYPE, PK_TYPE_NORMAL);
			//player->broadcast_one_attr_changed(PLAYER_ATTR_PK_TYPE, PK_TYPE_NORMAL, false, true);
			bSuccess = true;

			if (raid->is_guild_battle_raid() && !is_guild_battle_opening())
			{
				raid->player_leave_raid(player);
				if (raid->check_raid_need_delete())
				{
					if (is_guild_wait_raid(raid->m_id))
					{
						guild_wait_raid_manager::delete_guild_wait_raid((guild_wait_raid_struct*)raid);
					}
					else
					{
						raid_manager::delete_raid(raid);
					}
				}
			}
			break;
		}

		scene_struct *scene = NULL;
		if (is_guild_scene_id(player->data->scene_id))
		{
			scene = scene_manager::get_guild_scene(player->data->guild_id);
		}
		else
		{
			scene = scene_manager::get_scene(player->data->scene_id);
		}
//		if (player->data->scene_id > 30000)
//		{
//			scene = ZhenyingBattle::GetInstance()->GetFiled();
//		}
		if (!scene)
		{
			LOG_ERR("%s %d: player[%lu] want join scene[%u] failed", __FUNCTION__, __LINE__, player->data->player_id, player->data->scene_id);
			break;
		}

		scene->add_player_to_scene(player);
		bSuccess = true;
	} while(0);

	if (bSuccess)
	{
		player->take_partner_into_scene();
		if (!player->data->login_notify)
		{
			player_online_enter_scene_after(player, extern_data);
		}
	}
}

//玩家登陆
static int handle_enter_game(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	PROTO_ENTER_GAME_REQ* req = (PROTO_ENTER_GAME_REQ*)conn_node_gamesrv::connecter.buf_head();
	uint64_t player_id = req->player_id;
	bool reconnect = req->reconnect != 0;

	player_struct* ret = player_manager::get_player_by_id(player_id);
	if (ret) {
		if (ret->data->status == ONLINE) {
				//已经在线了？
			LOG_ERR("[%s:%d] player_id[%lu]", __FUNCTION__, __LINE__, player_id);
			return (0);
		}
		else {
			//之前写回DB还没确认的玩家, 此时上线直接使用之前的内存数据
			ret->data->status = ONLINE;
		}
	}
	else {
		int result = conn_node_dbsrv::connecter.send_one_msg(&req->head, 1);
		if (result != (int)ENDION_FUNC_4(req->head.len)) {
			LOG_ERR("[%s : %d]: send data to db server failed", __FUNCTION__, __LINE__);
		}

		return 0;
	}

//  scene_struct *scene = scene_manager::get_scene(ret->data->scene_id);
//	if (scene) {
//		scene->add_player_to_scene(ret);
//	}
//	ret->data->raid_uuid = 0;

	return pack_player_online(ret, extern_data, false, reconnect);
}

//static int handle_set_fashion_request(player_struct *player, EXTERN_DATA *extern_data)
//{
//	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
//	if (!player || !player->is_online())
//	{
//		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
//		return (-1);
//	}
//
//	SetFashion *req = set_fashion__unpack(NULL, get_data_len(), (uint8_t *)get_data());
//	if (!req)
//	{
//		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
//		return (-10);
//	}
//
//	if (req->id >= PLAYER_ATTR_CLOTHES && req->id <= PLAYER_ATTR_CLOTHES_COLOR_DOWN)
//	{
//		player->set_attr(req->id, req->vaual);
//		AttrMap attrs;
//		attrs[req->id] = req->vaual;
//		player->notify_attr(attrs, true, true);
//	}
//
//	set_fashion__free_unpacked(req, NULL);
//
//	return 0;
//}

//玩家下线
static int handle_kick_player(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	LOG_DEBUG("[%s:%d] player_id:%lu, player:%p, data:%p", __FUNCTION__, __LINE__, player->data->player_id, player, player->data);

	PROTO_ROLE_KICK *notify = (PROTO_ROLE_KICK*)buf_head();
	bool again = (notify->again == 0 ? false : true);
	player->process_offline(again, extern_data);

	return 0;
}

//背包信息请求
static int handle_bag_info_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

//	player->tidy_bag();
	BagInfoAnswer resp;
	bag_info_answer__init(&resp);

	BagGrid bag_data[MAX_BAG_GRID_NUM];
	BagGrid *bag_data_point[MAX_BAG_GRID_NUM];
	ItemBaguaData bagua_data[MAX_BAG_GRID_NUM];
	ItemPartnerFabaoData fabao_data[MAX_BAG_GRID_NUM];
	AttrData bagua_attr[MAX_BAG_GRID_NUM][MAX_BAGUAPAI_MINOR_ATTR_NUM];
	AttrData* bagua_attr_point[MAX_BAG_GRID_NUM][MAX_BAGUAPAI_MINOR_ATTR_NUM];
	AttrData item_fabao_attr[MAX_BAG_GRID_NUM][MAX_HUOBAN_FABAO_MINOR_ATTR_NUM];
	AttrData* item_fabao_attr_point[MAX_BAG_GRID_NUM][MAX_HUOBAN_FABAO_MINOR_ATTR_NUM];
	AttrData fabao_attr;

	resp.result = 0;
	resp.curgridnum = player->data->bag_grid_num;
	resp.totalgridnum = get_bag_total_num(player->data->attrData[PLAYER_ATTR_JOB], player->data->attrData[PLAYER_ATTR_LEVEL]);
	resp.n_grids = 0;
	resp.grids = bag_data_point;
	for (uint32_t i = 0; i < player->data->bag_grid_num; ++i)
	{
		if (player->data->bag[i].id == 0)
		{
			continue;
		}

		bag_data_point[resp.n_grids] = &bag_data[resp.n_grids];
		bag_grid__init(&bag_data[resp.n_grids]);
		bag_data[resp.n_grids].index = i;
		bag_data[resp.n_grids].id = player->data->bag[i].id;
		bag_data[resp.n_grids].num = player->data->bag[i].num;
		bag_data[resp.n_grids].usedcount = player->data->bag[i].used_count;
		bag_data[resp.n_grids].expiretime = player->data->bag[i].expire_time;
		if (item_is_baguapai(player->data->bag[i].id))
		{
			bag_data[resp.n_grids].bagua = &bagua_data[resp.n_grids];
			item_bagua_data__init(&bagua_data[resp.n_grids]);
			bagua_data[resp.n_grids].star = player->data->bag[i].especial_item.baguapai.star;
			bagua_data[resp.n_grids].main_attr_val = player->data->bag[i].especial_item.baguapai.main_attr_val;
			uint32_t attr_num = 0;
			for (int j = 0; j < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++j)
			{
				if (player->data->bag[i].especial_item.baguapai.minor_attrs[j].id == 0)
				{
					break;
				}

				bagua_attr_point[resp.n_grids][attr_num] = &bagua_attr[resp.n_grids][attr_num];
				attr_data__init(&bagua_attr[resp.n_grids][attr_num]);
				bagua_attr[resp.n_grids][attr_num].id = player->data->bag[i].especial_item.baguapai.minor_attrs[j].id;
				bagua_attr[resp.n_grids][attr_num].val = player->data->bag[i].especial_item.baguapai.minor_attrs[j].val;
				attr_num++;
			}
			bagua_data[resp.n_grids].minor_attrs = bagua_attr_point[resp.n_grids];
			bagua_data[resp.n_grids].n_minor_attrs = attr_num;
		}
		if (item_is_partner_fabao(player->data->bag[i].id))
		{
			bag_data[resp.n_grids].fabao = &fabao_data[resp.n_grids];
			item_partner_fabao_data__init(&fabao_data[resp.n_grids]);
			fabao_data[resp.n_grids].main_attr = &fabao_attr;
			attr_data__init(&fabao_attr);
			fabao_attr.id =  player->data->bag[i].especial_item.fabao.main_attr.id;
			fabao_attr.val = player->data->bag[i].especial_item.fabao.main_attr.val;
			uint32_t attr_num = 0;
			for (int j = 0; j < MAX_HUOBAN_FABAO_MINOR_ATTR_NUM; ++j)
			{
				if (player->data->bag[i].especial_item.fabao.minor_attr[j].id == 0)
				{
					break;
				}

				item_fabao_attr_point[resp.n_grids][attr_num] = &item_fabao_attr[resp.n_grids][attr_num];
				attr_data__init(&bagua_attr[resp.n_grids][attr_num]);
				item_fabao_attr[resp.n_grids][attr_num].id = player->data->bag[i].especial_item.fabao.minor_attr[j].id;
				item_fabao_attr[resp.n_grids][attr_num].val = player->data->bag[i].especial_item.fabao.minor_attr[j].val;
				attr_num++;
			}
			fabao_data[resp.n_grids].minor_attr = item_fabao_attr_point[resp.n_grids];
			fabao_data[resp.n_grids].n_minor_attr = attr_num;
		}
		resp.n_grids++;
	}

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_BAG_INFO_ANSWER, bag_info_answer__pack, resp);

	return 0;
}

//背包开格子请求
static int handle_bag_unlock_grid_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	int ret = 0;
	uint32_t total_grid_num = 0;
	do
	{
		uint32_t player_job = player->data->attrData[PLAYER_ATTR_JOB];
		uint32_t player_level = player->data->attrData[PLAYER_ATTR_LEVEL];
		ActorLevelTable *config = get_actor_level_config(player_job, player_level);
		if (!config)
		{
			LOG_ERR("[%s:%d] player[%lu] get ActorLevelTable failed, job:%u, level:%u", __FUNCTION__, __LINE__, extern_data->player_id, player_job, player_level);
			ret = ERROR_ID_NO_CONFIG;
			break;
		}

		total_grid_num = config->FreeGrid + config->LockGrid;

		if (player->data->bag_unlock_num >= config->LockGrid)
		{
			LOG_ERR("[%s:%d] player[%lu] has unlocked all, has_unlock:%u, can_unlock:%lu", __FUNCTION__, __LINE__, extern_data->player_id, player->data->bag_unlock_num, config->LockGrid);
			ret = ERROR_ID_BAG_UNLOCK_ALL;
			break;
		}

		const uint32_t unlock_num_per_time = 5;
		uint32_t unlock_times = ceil(player->data->bag_unlock_num / (double)unlock_num_per_time);
		uint32_t need_gold = sg_bag_unlock_base_price + sg_bag_unlock_incr_factor * unlock_times;
		uint32_t player_gold = player->get_comm_gold();
		if (player_gold < need_gold)
		{
			LOG_ERR("[%s:%d] player[%lu] gold not enough, need_gold:%u, player_gold:%u", __FUNCTION__, __LINE__, extern_data->player_id, need_gold, player_gold);
			ret = ERROR_ID_UNLOCK_BAG_GOLD_NOT_ENOUGH;
			break;
		}

		//check end
		player->sub_comm_gold(need_gold, MAGIC_TYPE_BAG_UNLOCK);
		player->data->bag_unlock_num = std::min(player->data->bag_unlock_num + unlock_num_per_time, (uint32_t)config->LockGrid);
		player->fit_bag_grid_num();

	} while(0);

	BagUnlockGridAnswer resp;
	bag_unlock_grid_answer__init(&resp);

	resp.result = ret;
	resp.curgridnum = player->data->bag_grid_num;
	resp.totalgridnum = (total_grid_num > 0 ? total_grid_num : resp.curgridnum);

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_BAG_UNLOCK_GRID_ANSWER, bag_unlock_grid_answer__pack, resp);

	return 0;
}

//背包出售物品请求
static int handle_bag_sell_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	BagSellRequest *req = bag_sell_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t pos = req->index;
	uint32_t num = req->num;

	bag_sell_request__free_unpacked(req, NULL);

	int ret = 0;
	ItemData sell_item;
	item_data__init(&sell_item);
	AttrData gain_money;
	attr_data__init(&gain_money);
	do
	{
		if (!(pos < player->data->bag_grid_num))
		{
			ret = ERROR_ID_BAG_POS;
			LOG_ERR("[%s:%d] player[%lu] bag pos error, pos:%u, grid_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, pos, player->data->bag_grid_num);
			break;
		}

		bag_grid_data& grid = player->data->bag[pos];
		ItemsConfigTable *prop_config = get_config_by_id(grid.id, &item_config);
		if (!prop_config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get itemconfig failed, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, grid.id);
			break;
		}

		if (prop_config->Price == (uint64_t)-1)
		{
			ret = ERROR_ID_PROP_CAN_NOT_SELL;
			LOG_ERR("[%s:%d] player[%lu] prop can not be sold", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (num > grid.num)
		{
			ret = ERROR_ID_PROP_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] prop num error, num:%u, bag_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, num, grid.num);
			break;
		}

		sell_item.id = grid.id;
		sell_item.num = num;
		player->del_item_by_pos(pos, num, MAGIC_TYPE_BAG_SELL);
		uint32_t sell_coin = prop_config->Price * num;
		if (sell_coin > 0)
		{
			player->add_coin(sell_coin, MAGIC_TYPE_BAG_SELL);
			gain_money.id = PLAYER_ATTR_COIN;
			gain_money.val = sell_coin;
		}
	} while(0);

	BagSellAnswer resp;
	bag_sell_answer__init(&resp);

	resp.result = ret;
	resp.sellitem = &sell_item;
	resp.gainmoney = &gain_money;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_BAG_SELL_ANSWER, bag_sell_answer__pack, resp);

	return 0;
}

//背包使用物品请求
static int handle_bag_use_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	if (!player->is_alive())
	{
		LOG_ERR("%s: player[%lu] dead", __FUNCTION__, extern_data->player_id);
		return (-5);
	}

	BagUseRequest *req = bag_use_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t pos = req->index;
	uint32_t use_all = req->use_all;
	bool is_easy = req->isesay;

	bag_use_request__free_unpacked(req, NULL);

	ItemsConfigTable *prop_config = NULL;
	int ret = 0;
	uint32_t item_id = 0;
	ItemUseEffectInfo *effect_info = new ItemUseEffectInfo();
	effect_info->pos = pos;
	effect_info->use_all = use_all;
	effect_info->is_easy = is_easy;
	bool need_free = true;
	std::map<uint32_t, uint32_t> *item_map = NULL;
	do
	{
		if (player->is_item_expire(pos))
		{
			ret = ERROR_ID_USE_PROP_IS_EXPIRE;
			LOG_ERR("[%s:%d] player[%lu] item is expire, pos:%u", __FUNCTION__, __LINE__, extern_data->player_id, pos);
			break;
		}

		if (!(pos < player->data->bag_grid_num))
		{
			ret = ERROR_ID_BAG_POS;
			LOG_ERR("[%s:%d] player[%lu] bag pos error, pos:%u, grid_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, pos, player->data->bag_grid_num);
			break;
		}

		bag_grid_data& grid = player->data->bag[pos];
		prop_config = get_config_by_id(grid.id, &item_config);
		if (!prop_config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get itemconfig failed, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, grid.id);
			break;
		}

		if (player->check_item_cd(prop_config) != 0)
		{
			ret = ERROR_ID_PROP_IN_CD;
			LOG_ERR("[%s:%d] player[%lu] in cd, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, grid.id);
			break;
		}

		item_id = grid.id;

		if (prop_config->UseDegree == 0)
		{
			ret = ERROR_ID_PROP_CAN_NOT_USE;
			LOG_ERR("[%s:%d] player[%lu] item can't use, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, grid.id);
			break;
		}

		ret = player->try_use_prop(pos, use_all, effect_info);
		if (ret != 0)
		{
			break;
		}

		if (prop_config->ItemEffect == IUE_EXTEND_FRIEND_NUM)
		{
			//换一个消息号，转发到好友服处理
			int data_len = get_data_len();
			memcpy(get_send_data(), get_data(), data_len);
			fast_send_msg_base(&conn_node_gamesrv::connecter, extern_data, SERVER_PROTO_FRIEND_EXTEND_CONTACT_REQUEST, data_len, get_seq());
			return 0;
		}

		if (prop_config->CostTime > 0)
		{
			need_free = false;
			player->stop_move();
			if (prop_config->ItemEffect == IUE_XUNBAO)
			{
				player->begin_sing(SING_TYPE__XUNBAO, prop_config->CostTime, false, true, effect_info);
			}
			else
			{
				player->begin_sing(SING_TYPE__USE_PROP, prop_config->CostTime, true, true, effect_info);
			}
		}
		else
		{
			ret = player->use_prop(pos, use_all, effect_info);
			item_map = &effect_info->items;
		}
	} while(0);

	BagUseAnswer resp;
	bag_use_answer__init(&resp);

	ItemData item_data[50];
	ItemData* item_data_point[50];

	resp.result = ret;
	if (ret == 0)
	{
		assert(prop_config);
		resp.cd = player->set_item_cd(prop_config);
	}
	resp.item_id = item_id;
	resp.n_itemlist = 0;
	resp.itemlist = item_data_point;
	if (item_map)
	{
		for (std::map<uint32_t, uint32_t>::iterator iter = item_map->begin(); iter != item_map->end() && resp.n_itemlist < 50; ++iter)
		{
			item_data_point[resp.n_itemlist] = &item_data[resp.n_itemlist];
			item_data__init(&item_data[resp.n_itemlist]);
			item_data[resp.n_itemlist].id = iter->first;
			item_data[resp.n_itemlist].num = iter->second;
			resp.n_itemlist++;
		}
	}
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_BAG_USE_ANSWER, bag_use_answer__pack, resp);

	if (need_free)
	{
		delete effect_info;
		effect_info = NULL;
	}

	return 0;
}

//背包堆叠物品请求
static int handle_bag_stack_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	BagStackRequest *req = bag_stack_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t id = req->id;

	bag_stack_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		player->merge_item(id);
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_BAG_STACK_ANSWER, comm_answer__pack, resp);

	return 0;
}

//背包整理请求
static int handle_bag_tidy_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}


//	int ret = 0;
	do
	{
		player->tidy_bag();
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = 0;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_BAG_TIDY_ANSWER, comm_answer__pack, resp);

	return 0;
}

static int handle_relive_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player->is_avaliable())
	{
		LOG_ERR("%s: player[%lu] do not have scene or data", __FUNCTION__, extern_data->player_id);
		return -10;
	}

	if (player->is_alive())
	{
		LOG_ERR("%s: player[%lu] already alive", __FUNCTION__, extern_data->player_id);
		send_comm_answer(MSG_ID_RELIVE_ANSWER, -1, extern_data);
		return (-1);
	}
	ReliveRequest *req = relive_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t type = req->type;
	relive_request__free_unpacked(req, NULL);
	LOG_INFO("%s: player %lu type %u", __FUNCTION__, extern_data->player_id, type);

	player->on_relive(type);
	return (0);
}

//改名请求
static int handle_rename_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PlayerRenameRequest *req = player_rename_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	std::string name(req->name);
	player_rename_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		size_t name_len = name.length();
		if (name_len == 0 || name_len >= MAX_PLAYER_NAME_LEN)
		{
			ret = ERROR_ID_NAME_LEN;
			LOG_ERR("[%s:%d] player[%lu] name length error, name_len:%lu", __FUNCTION__, __LINE__, extern_data->player_id, name_len);
			break;
		}

		uint32_t item_id = sg_rename_item_id;
		uint32_t item_num = sg_rename_item_num;
		uint32_t bag_num = player->get_item_can_use_num(item_id);
		if (bag_num < item_num)
		{
			ret = ERROR_ID_RENAME_PROP_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] item not enough, id:%u, need_num:%u, bag_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, item_id, item_num, bag_num);
			break;
		}
	} while(0);

	if (ret == 0)
	{
		memcpy(get_send_data(), get_data(), get_data_len());
		fast_send_msg_base(&conn_node_dbsrv::connecter, extern_data, MSG_ID_PLAYER_RENAME_REQUEST, get_data_len(), get_seq());
	}
	else
	{
		CommAnswer resp;
		comm_answer__init(&resp);

		resp.result = ret;
		fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PLAYER_RENAME_ANSWER, comm_answer__pack, resp);
	}

	return 0;
}

//头像信息请求
static int handle_head_icon_info_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	return notify_head_icon_info(player, extern_data);
}

static int notify_head_icon_info(player_struct *player, EXTERN_DATA *extern_data)
{
	HeadIconInfoAnswer resp;
	head_icon_info_answer__init(&resp);

	HeadIconData head_data[MAX_HEAD_ICON_NUM];
	HeadIconData* head_data_point[MAX_HEAD_ICON_NUM];
	resp.result = 0;
	resp.n_icon_list = 0;
	resp.icon_list = head_data_point;
	for (int i = 0; i < MAX_HEAD_ICON_NUM; ++i)
	{
		if (player->data->head_icon_list[i].id == 0)
		{
			continue;
		}

		head_data_point[resp.n_icon_list] = &head_data[resp.n_icon_list];
		head_icon_data__init(&head_data[resp.n_icon_list]);
		head_data[resp.n_icon_list].id = player->data->head_icon_list[i].id;
		head_data[resp.n_icon_list].status = player->data->head_icon_list[i].status;
		resp.n_icon_list++;
	}

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_HEAD_ICON_INFO_ANSWER, head_icon_info_answer__pack, resp);

	return 0;
}

//头像更换请求
static int handle_head_icon_replace_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	HeadIconReplaceRequest *req = head_icon_replace_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t replace_icon = req->icon_id;
	head_icon_replace_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		uint32_t using_icon = player->data->attrData[PLAYER_ATTR_HEAD];
		if (using_icon == replace_icon)
		{
//			ret = ERROR_ID_ICON_USING;
//			LOG_ERR("[%s:%d] player[%lu] icon is using, icon:%u", __FUNCTION__, __LINE__, extern_data->player_id, replace_icon);
			break;
		}

		HeadIconInfo *icon_info = player->get_head_icon(replace_icon);
		if (!icon_info)
		{
			ret = ERROR_ID_ICON_NEED_UNLOCK;
			LOG_ERR("[%s:%d] player[%lu] icon is not unlock, icon:%u", __FUNCTION__, __LINE__, extern_data->player_id, replace_icon);
			break;
		}

		player->data->attrData[PLAYER_ATTR_HEAD] = replace_icon;
		AttrMap attrs;
		attrs[PLAYER_ATTR_HEAD] = player->data->attrData[PLAYER_ATTR_HEAD];
		player->notify_attr(attrs, true);

	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_HEAD_ICON_REPLACE_ANSWER, comm_answer__pack, resp);

	return 0;
}

//头像更换请求
static int handle_head_icon_old_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	HeadIconReplaceRequest *req = head_icon_replace_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t icon_id = req->icon_id;
	head_icon_replace_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		HeadIconInfo *icon_info = player->get_head_icon(icon_id);
		if (!icon_info)
		{
			ret = ERROR_ID_ICON_NEED_UNLOCK;
			LOG_ERR("[%s:%d] player[%lu] icon is not unlock, icon:%u", __FUNCTION__, __LINE__, extern_data->player_id, icon_id);
			break;
		}

		icon_info->status = 0;
	} while(0);

	HeadIconOldAnswer resp;
	head_icon_old_answer__init(&resp);

	resp.result = ret;
	resp.icon_id = icon_id;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_HEAD_ICON_OLD_ANSWER, head_icon_old_answer__pack, resp);

	return 0;
}

// static int handle_bag_show_request(player_struct *player, EXTERN_DATA *extern_data)
// {
// 	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
// 	if (!player || !player->is_online())
// 	{
// 		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
// 		return (-1);
// 	}

// 	ShowItemRequest *req = show_item_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
// 	if (!req)
// 	{
// 		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
// 		return (-10);
// 	}

// 	//uint32_t index = req->index;
// 	show_item_request__free_unpacked(req, NULL);

// 	SHOW_ITEM_TO_ITEMSRV *pShow = (SHOW_ITEM_TO_ITEMSRV *)&conn_node_base::global_send_buf[0];

// 	ShowItemData showData;
// 	show_item_data__init(&showData);
// 	pShow->data_size = show_item_data__pack(&showData, pShow->data);
// 	showData.id = 11111;
// 	//memcpy(&(pShow->data[0]), &index, sizeof(uint32_t));
// 	pShow->head.msg_id = 0;
// 	pShow->type = 1;
// 	pShow->head.len = ENDION_FUNC_4(sizeof(SHOW_ITEM_TO_ITEMSRV) + pShow->data_size);

// 	conn_node_base::add_extern_data(&pShow->head, extern_data);

//	if (conn_node_itemsrv::connecter.send_one_msg(&pShow->head, 1) != (int)ENDION_FUNC_4(pShow->head.len)) {
//		LOG_ERR("%s %d: send to itemsrv err[%d]", __FUNCTION__, __LINE__, errno);
//	}

// 	return 0;
// }

//任务列表请求
static int handle_task_list_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	return notify_task_list(player, extern_data);
}

static int notify_task_list(player_struct *player, EXTERN_DATA *extern_data)
{
	TaskListAnswer resp;
	task_list_answer__init(&resp);

	TaskRewardData reward_data[MAX_TASK_ACCEPTED_NUM];
	ItemData item_data[MAX_TASK_ACCEPTED_NUM][MAX_SHANGJIN_AWARD_NUM];
	ItemData *item_data_point[MAX_TASK_ACCEPTED_NUM][MAX_SHANGJIN_AWARD_NUM];
	TaskData task_data[MAX_TASK_ACCEPTED_NUM];
	TaskData* task_data_point[MAX_TASK_ACCEPTED_NUM];
	TaskCount count_data[MAX_TASK_ACCEPTED_NUM][MAX_TASK_TARGET_NUM];
	TaskCount* count_data_point[MAX_TASK_ACCEPTED_NUM][MAX_TASK_TARGET_NUM];
	resp.result = 0;
	resp.n_ongoing_list = 0;
	resp.ongoing_list = task_data_point;
	for (int i = 0; i < MAX_TASK_ACCEPTED_NUM; ++i)
	{
		TaskInfo &info = player->data->task_list[i];
		if (info.id == 0)
		{
			continue;
		}

		task_data_point[resp.n_ongoing_list] = &task_data[resp.n_ongoing_list];
		task_data__init(&task_data[resp.n_ongoing_list]);
		task_data[resp.n_ongoing_list].id = info.id;
		task_data[resp.n_ongoing_list].status = info.status;
		task_data[resp.n_ongoing_list].expiretime = player->get_task_expire_time(&info);
		uint32_t nTarget = 0;
		for (int j = 0; j < MAX_TASK_TARGET_NUM; j++)
		{
			if (info.progress[j].id == 0)
			{
				continue;
			}

			count_data_point[i][nTarget] = &count_data[i][nTarget];
			task_count__init(&count_data[i][nTarget]);
			count_data[i][nTarget].type = info.progress[j].id;
			count_data[i][nTarget].count = info.progress[j].num;
			nTarget++;
		}
		task_data[resp.n_ongoing_list].n_progress = nTarget;
		task_data[resp.n_ongoing_list].progress = count_data_point[i];

		if (get_task_type(info.id) == TT_SHANGJIN)
		{//只发当前赏金任务奖励
			task_reward_data__init(&reward_data[resp.n_ongoing_list]);
			task_data[resp.n_ongoing_list].reward = &reward_data[resp.n_ongoing_list];
			reward_data[resp.n_ongoing_list].exp = player->data->shangjin.task[player->data->shangjin.cur_task].exp;
			reward_data[resp.n_ongoing_list].coin = player->data->shangjin.task[player->data->shangjin.cur_task].coin;
			reward_data[resp.n_ongoing_list].items = item_data_point[resp.n_ongoing_list];
			size_t &n_item = reward_data[resp.n_ongoing_list].n_items;
			for (; n_item < player->data->shangjin.task[player->data->shangjin.cur_task].n_award; ++n_item)
			{
				item_data_point[resp.n_ongoing_list][n_item] = &(item_data[resp.n_ongoing_list][n_item]);
				item_data__init(item_data_point[resp.n_ongoing_list][n_item]);
				item_data[resp.n_ongoing_list][n_item].id = player->data->shangjin.task[player->data->shangjin.cur_task].award[n_item].id;
				item_data[resp.n_ongoing_list][n_item].num = player->data->shangjin.task[player->data->shangjin.cur_task].award[n_item].val;
			}
		}
		else if (get_task_type(info.id) == TT_CASH_TRUCK)
		{
			BiaocheTable *table = get_config_by_id(player->data->truck.active_id, &cash_truck_config);
			if (table != NULL)
			{
				BiaocheRewardTable *reward_config = get_config_by_id(table->Reward, &cash_truck_reward_config);
				if (reward_config != NULL)
				{
					task_reward_data__init(&reward_data[resp.n_ongoing_list]);
					task_data[resp.n_ongoing_list].reward = &reward_data[resp.n_ongoing_list];
					reward_data[resp.n_ongoing_list].exp = reward_config->RewardExp1 * player->get_attr(PLAYER_ATTR_LEVEL);
					reward_data[resp.n_ongoing_list].coin = reward_config->RewardMoney1 * player->get_attr(PLAYER_ATTR_LEVEL) * (reward_config->Deposit + 10000) / 10000.0;
					reward_data[resp.n_ongoing_list].items = item_data_point[resp.n_ongoing_list];
					size_t &n_item = reward_data[resp.n_ongoing_list].n_items;
					for (uint32_t i = 0; i < reward_config->n_RewardItem1; ++n_item, ++i)
					{
						item_data_point[resp.n_ongoing_list][n_item] = &(item_data[resp.n_ongoing_list][n_item]);
						item_data__init(item_data_point[resp.n_ongoing_list][n_item]);
						item_data[resp.n_ongoing_list][n_item].id = reward_config->RewardItem1[i];
						item_data[resp.n_ongoing_list][n_item].num = reward_config->RewardNum1[i];
					}
					item_data_point[resp.n_ongoing_list][n_item] = &(item_data[resp.n_ongoing_list][n_item]);
					item_data__init(item_data_point[resp.n_ongoing_list][n_item]);
					item_data[resp.n_ongoing_list][n_item].id = 201010002;
					item_data[resp.n_ongoing_list][n_item].num = reward_config->RewardLv1 * player->get_attr(PLAYER_ATTR_LEVEL);
					++n_item;
				}
			}
		}
		resp.n_ongoing_list++;
	}

	std::vector<uint32_t> finish_data;
	std::copy(player->task_finish_set.begin(), player->task_finish_set.end(), back_inserter(finish_data));
	resp.n_finish_list = finish_data.size();
	resp.finish_list = &finish_data[0];
	player->get_task_chapter_info(resp.chapterid, resp.chapterstate);

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TASK_LIST_ANSWER, task_list_answer__pack, resp);

	return 0;
}

//接任务请求
static int handle_task_accept_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	TaskCommRequest *req = task_comm_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t task_id = req->task_id;
	task_comm_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		ret = player->accept_task(task_id);
		if (ret != 0)
		{
			LOG_ERR("[%s:%d] player[%lu], task_id:%u, ret:%d", __FUNCTION__, __LINE__, extern_data->player_id, task_id, ret);
			break;
		}
	} while(0);

	TaskCommAnswer resp;
	task_comm_answer__init(&resp);

	resp.result = ret;
	resp.task_id = task_id;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TASK_ACCEPT_ANSWER, task_comm_answer__pack, resp);

	return 0;
}

//交任务请求
static int handle_task_submit_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	TaskCommRequest *req = task_comm_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t task_id = req->task_id;
	task_comm_request__free_unpacked(req, NULL);

	bool shangJin = false;
	int ret = 0;
	do
	{
		TaskTable *config = get_config_by_id(task_id, &task_config);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get task failed, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, task_id);
			break;
		}

		if (config->Team == 1)
		{
			if (!player->m_team)
			{
				ret = ERROR_ID_TASK_TEAM_LEADER;
				break;
			}

			if (player->m_team->GetLeadId() != player->get_uuid())
			{
				ret = ERROR_ID_TASK_TEAM_LEADER;
				break;
			}
		}

		TaskInfo *task_info = player->get_task_info(task_id);
		if (!task_info)
		{
			ret = ERROR_ID_TASK_NOT_ACCEPT;
			LOG_ERR("[%s:%d] player[%lu] get task info failed, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, task_id);
			break;
		}

		if (task_info->status != TASK_STATUS__ACHIEVED)
		{
			ret = ERROR_ID_TASK_NOT_ACHIEVE;
			LOG_ERR("[%s:%d] player[%lu] task not achieve, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, task_id);
			break;
		}

		//检查背包空间
		std::map<uint32_t, uint32_t> item_list;
		player->get_task_event_item(task_id, TEC_SUBMIT, item_list);
		player->get_task_reward_item(task_id, item_list);
		if (item_list.size() > 0 && !player->check_can_add_item_list(item_list))
		{
			ret = ERROR_ID_BAG_NOT_ABLE_ADD_TASK_SUBMIT;
			LOG_ERR("[%s:%d] player[%lu] bag not enough, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, task_id);
			break;
		}

		TaskInfo tmp_info;
		memcpy(&tmp_info, task_info, sizeof(TaskInfo));
		memset(task_info, 0, sizeof(TaskInfo));
		tmp_info.status = TASK_STATUS__FINISH;
		player->task_update_notify(&tmp_info);
		player->touch_task_event(task_id, TEC_SUBMIT);
		player->give_task_reward(task_id);
		player->add_finish_task(task_id);
		player->clear_one_buff(114400019);	

		//章节、主线下一个任务
		if (config->TaskType == TT_TRUNK)
		{
			if (config->FollowTask > 0)
			{
				player->add_task(config->FollowTask, TASK_STATUS__NOT_ACCEPT_YET, true);
				if (config->ChapterId != get_task_chapter_id(config->FollowTask))
				{
					player->update_task_chapter_info();
				}
			}
		}
		if (config->TaskType == TT_SHANGJIN)
		{
			shangJin = true;
		}
	} while(0);

	TaskCommAnswer resp;
	task_comm_answer__init(&resp);

	resp.result = ret;
	resp.task_id = task_id;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TASK_SUBMIT_ANSWER, task_comm_answer__pack, resp);

	if (shangJin)
	{
		ShangjinManage::CompleteTask(player, task_id);
	}

	return 0;
}

//放弃任务请求
static int handle_task_abandon_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	TaskCommRequest *req = task_comm_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t task_id = req->task_id;
	task_comm_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		TaskTable *config = get_config_by_id(task_id, &task_config);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get task config failed, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, task_id);
			break;
		}

		TaskInfo *task_info = player->get_task_info(task_id);
		if (!task_info)
		{
			ret = ERROR_ID_TASK_NOT_ACCEPT;
			LOG_ERR("[%s:%d] player[%lu] get task info failed, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, task_id);
			break;
		}

		if (task_info->status != TASK_STATUS__UNACHIEVABLE)
		{
			ret = ERROR_ID_TASK_CAN_NOT_ABANDON;
			LOG_ERR("[%s:%d] player[%lu] task can not abandon, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, task_id);
			break;
		}

		task_info->status = TASK_STATUS__NOT_ACCEPT_YET;
		player->task_update_notify(task_info);

		memset(task_info, 0, sizeof(TaskInfo));

		if (config->TaskType == TT_QUESTION) //todo 删除完成列表里面的答题任务 重置答题信息
		{
		}
	} while(0);

	TaskCommAnswer resp;
	task_comm_answer__init(&resp);

	resp.result = ret;
	resp.task_id = task_id;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TASK_ABANDON_ANSWER, task_comm_answer__pack, resp);

	return 0;
}

//任务刷怪请求
static int handle_task_monster_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	TaskEventRequest *req = task_event_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t task_id = req->task_id;
	uint32_t event_id = req->event_id;
	task_event_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		TaskInfo *info = player->get_task_info(task_id);
		if (!info || info->status != TASK_STATUS__ACCEPTED)
		{
			ret = ERROR_ID_TASK_ID;
			LOG_ERR("[%s:%d] player[%lu] get task failed, task_id:%u, event_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, task_id, event_id);
			break;
		}

//		if (player->sight_space != NULL)
//		{
//			ret = ERROR_ID_PLAYER_IN_SIGHT_SPACE;
//			LOG_ERR("[%s:%d] player[%lu] in sight space, task_id:%u, event_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, task_id, event_id);
//			break;
//		}

		TaskTable *config = get_config_by_id(task_id, &task_config);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get task failed, task_id:%u, event_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, task_id, event_id);
			break;
		}

		bool event_fit = false;
		for (uint32_t i = 0; i < config->n_EventID; ++i)
		{
			if ((uint32_t)config->EventID[i] == event_id)
			{
				event_fit = true;
				break;
			}
		}

		if (!event_fit)
		{
			ret = ERROR_ID_TASK_EVENT_ID;
			LOG_ERR("[%s:%d] player[%lu] has not event, task_id:%u, event_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, task_id, event_id);
			break;
		}

		TaskEventTable *event_config = get_config_by_id(event_id, &task_event_config);
		if (!event_config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get event config failed, task_id:%u, event_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, task_id, event_id);
			break;
		}

		uint32_t event_class = (uint32_t)event_config->EventClass;
		if (!(event_class == TEC_ACCEPT || event_class == TEC_TALK))
		{
			ret = ERROR_ID_TASK_EVENT_CLASS;
			LOG_ERR("[%s:%d] player[%lu] event class error, task_id:%u, event_id:%u, event_class:%u", __FUNCTION__, __LINE__, extern_data->player_id, task_id, event_id, event_class);
			break;
		}

		if (event_config->EventType != TET_ADD_MONSTER)
		{
			ret = ERROR_ID_TASK_EVENT_TYPE;
			LOG_ERR("[%s:%d] player[%lu] event type error, task_id:%u, event_id:%u, event_type:%lu", __FUNCTION__, __LINE__, extern_data->player_id, task_id, event_id, event_config->EventType);
			break;
		}

		player->execute_task_event(event_id, event_class, false);
	} while(0);

	TaskCommAnswer resp;
	task_comm_answer__init(&resp);

	resp.result = ret;
	resp.task_id = task_id;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TASK_MONSTER_ANSWER, task_comm_answer__pack, resp);

	return 0;
}

int check_task_complete_pos(TaskConditionTable *config, player_struct *player, uint32_t task_id, uint32_t cond_id)
{
	position *pos = player->get_pos();
	uint32_t target_scene = 0;
	if (get_transfer_point((uint32_t)config->Scene, &target_scene, NULL, NULL, NULL, NULL) != 0)
	{
		LOG_ERR("[%s:%d] player[%lu] get transfer point failed, task_id:%u, cond_id:%u, id:%u", __FUNCTION__, __LINE__, player->get_uuid(), task_id, cond_id, (uint32_t)config->Scene);
		return ERROR_ID_NO_CONFIG;
	}

	position target_pos;
	target_pos.pos_x = config->PointX;
	target_pos.pos_z = config->PointZ;
	if (!(player->data->scene_id == target_scene && check_circle_in_range(pos, &target_pos, config->Radius)))
	{
		LOG_ERR("[%s:%d] player[%lu] scene:%u, posx:%f, posz:%f, targetscene:%u, targetx:%f, targetz:%f, task_id:%u, cond_id:%u radius:%lu", __FUNCTION__, __LINE__, player->get_uuid() , player->data->scene_id, pos->pos_x, pos->pos_z, target_scene, target_pos.pos_x, target_pos.pos_z, task_id, cond_id, config->Radius);
		return ERROR_ID_TASK_POSITION;
	}
	return 0;
}
//任务完成请求
static int handle_task_complete_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	TaskCompleteRequest *req = task_complete_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t task_id = req->task_id;
	uint32_t cond_id = req->condition_id;
	task_complete_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		TaskInfo *info = player->get_task_info(task_id);
		if (!info || info->status != TASK_STATUS__ACCEPTED)
		{
			ret = ERROR_ID_TASK_ID;
			LOG_ERR("[%s:%d] player[%lu] get task failed, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, task_id);
			break;
		}

		TaskCountInfo *count_info = NULL;
		for (int i = 0; i < MAX_TASK_TARGET_NUM; ++i)
		{
			if (info->progress[i].id == cond_id)
			{
				count_info = &info->progress[i];
				break;
			}
		}

		if (!count_info)
		{
			ret = ERROR_ID_TASK_CONDITION_ID;
			LOG_ERR("[%s:%d] player[%lu] condition not found, task_id:%u, condition_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, task_id, cond_id);
			break;
		}

		TaskConditionTable *config = get_config_by_id(cond_id, &task_condition_config);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get config failed, task_id:%u, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, task_id, cond_id);
			break;
		}

		if (count_info->num >= (uint32_t)config->ConditionNum)
		{
			break;
		}

		switch (config->ConditionType)
		{
		case TCT_EXPLORE:
		{
			ret = check_task_complete_pos(config, player,task_id, cond_id);
			if (ret != 0)
			{
				break;
			}
		}
		break;
		case TCT_TRUCK:
		{
			ret = check_task_complete_pos(config, player, task_id, cond_id);
			if (ret != 0)
			{
				break;
			}
			if (!player->data->truck.on_truck)
			{
				ret = ERROR_ID_TASK_CONDITION_ID;
				break;
			}
			player->go_down_cash_truck();
			cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(player->data->truck.truck_id);
			if (truck != NULL)
			{
				if (truck->sight_space != NULL)
				{
					truck->sight_space->broadcast_truck_delete(truck);
					player->data->truck.truck_id = 0;
					player->data->truck.active_id = 0;									
					sight_space_manager::del_player_from_sight_space(truck->sight_space, player, true);
				}
				else
				{
					player->data->truck.truck_id = 0;
					player->data->truck.active_id = 0;			
					truck->scene->delete_cash_truck_from_scene(truck);
				}
				cash_truck_manager::delete_cash_truck(truck);
			}
		}
		break;
			case TCT_TALK:
				{
					if (get_task_type(task_id) == TT_QUESTION)
					{
						//if (player->data->award_answer.question != 0)
						//{
						//	send_comm_answer(MSG_ID_GET_AWARD_QUESTION_FAILE_NOTIFY, 0, extern_data);
						//	break;
						//}
					}
				}
				break;
			case TCT_TRACK:
				break;
			default:
				{
					ret = ERROR_ID_TASK_CONDITION_ID;
					LOG_ERR("[%s:%d] player[%lu] condition illegal, task_id:%u, condition_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, task_id, cond_id);
					break;
				}
		}

		if (ret != 0)
		{
			break;
		}

		player->add_task_progress(config->ConditionType, config->ConditionTarget, 1, task_id, cond_id);
		if (config->ConditionType == TCT_TRUCK)
		{
			player->add_task_progress(TCT_TRUCK_NUM, 0, 1);
		}
	} while(0);

	TaskCommAnswer resp;
	task_comm_answer__init(&resp);

	resp.result = ret;
	resp.task_id = task_id;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TASK_COMPLETE_ANSWER, task_comm_answer__pack, resp);

	return 0;
}

//任务失败请求
static int handle_task_fail_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	TaskCommRequest *req = task_comm_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t task_id = req->task_id;
	task_comm_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		TaskInfo *info = player->get_task_info(task_id);
		if (!info || info->status != TASK_STATUS__ACCEPTED)
		{
			ret = ERROR_ID_TASK_ID;
			LOG_ERR("[%s:%d] player[%lu] get task failed, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, task_id);
			break;
		}

		if (!player_struct::task_condition_can_fail(task_id))
		{
			ret = ERROR_ID_TASK_ID;
			LOG_ERR("[%s:%d] player[%lu] condition can't fail, task_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, task_id);
			break;
		}

		player->set_task_fail(info);
	} while(0);

	TaskCommAnswer resp;
	task_comm_answer__init(&resp);

	resp.result = ret;
	resp.task_id = task_id;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TASK_FAIL_ANSWER, task_comm_answer__pack, resp);

	return 0;
}

//任务章节奖励
static int handle_task_chapter_reward_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	int ret = 0;
	do
	{
		uint32_t chapter_id = 0, chapter_state = 0;
		if (player->get_task_chapter_info(chapter_id, chapter_state) != 0)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get task chapter failed, task_chapter_reward:%u", __FUNCTION__, __LINE__, extern_data->player_id, player->data->task_chapter_reward);
			break;
		}

		if (chapter_state == 0)
		{
			ret = ERROR_ID_TASK_CHAPTER_REWARD;
			LOG_ERR("[%s:%d] player[%lu] chapter reward not achieve, chapter_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, chapter_id);
			break;
		}

		TaskChapterTable *config = get_task_chapter_config(chapter_id);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get chapter config failed, chapter_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, chapter_id);
			break;
		}

		std::map<uint32_t, uint32_t> item_list;
		for (uint32_t i = 0; i < config->n_RewardID; ++i)
		{
			get_task_reward_item_from_config(config->RewardID[i], item_list);
		}

		if (!player->check_can_add_item_list(item_list))
		{
			ret = ERROR_ID_BAG_GRID_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] bag not enough, chapter_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, chapter_id);
			break;
		}

		for (uint32_t i = 0; i < config->n_RewardID; ++i)
		{
			player->give_task_reward_by_reward_id(config->RewardID[i], MAGIC_TYPE_TASK_CHAPTER);
		}
		
		player->data->task_chapter_reward = chapter_id;
		player->update_task_chapter_info();
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TASK_CHAPTER_REWARD_ANSWER, comm_answer__pack, resp);

	return 0;
}

int handle_create_team_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	if (player->m_team != NULL)
	{
		return 2;
	}

	TeamTarget *req = team_target__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	send_comm_answer(MSG_ID_CREATE_TEAM_ANSWER, Team::CreateTeam(*player, req->type, req->target), extern_data);
	team_target__free_unpacked(req, NULL);


	return 0;
}

int handle_apply_team_request_impl(player_struct &player, uint64_t teamid)
{
	Team *pTeam = Team::GetTeam(teamid);
	if (pTeam == NULL)
	{
		return 190500025;
	}
	if (!pTeam->CheckLevel(player.get_attr(PLAYER_ATTR_LEVEL)))
	{
		return 190500026;
	}
	if (pTeam->GetLead() == NULL)
	{
		return 190500064;
	}
	if (pTeam->m_data->m_raid_uuid > SCENCE_DEPART)
	{
		return 190500066;
	}
	if (pTeam->IsFull())
	{
		return 190500024;
	}
	int ret = pTeam->AddApply(player);
	if (ret > 0)
	{
		return ret;
	}
	else if (ret == 0)
	{
		player_struct *lead = pTeam->GetLead();
		if (lead != NULL && !pTeam->IsAutoAccept())
		{
				TeamMemInfo notice;
				pTeam->PackMemberInfo(notice, player);

				EXTERN_DATA toExtern;
				toExtern.player_id = lead->get_uuid();
				fast_send_msg(&conn_node_gamesrv::connecter, &toExtern, MSG_ID_APPLY_TEAM_NOTIFY, team_mem_info__pack, notice);
		}
	}


	return 0;
}
int handle_apply_team_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	if (player->m_team != NULL)
	{
		return (-2);
	}

	Teamid *req = teamid__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	int ret = handle_apply_team_request_impl(*player, req->id);
	uint64_t teamId = req->id;
	teamid__free_unpacked(req, NULL);

	TeamApplyAnswer send;
	team_apply_answer__init(&send);
	send.errcode = ret;
	send.teamid = teamId;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_APPLY_TEAM_ANSWER, team_apply_answer__pack, send);
	//send_comm_answer(MSG_ID_APPLY_TEAM_ANSWER, ret, extern_data);
	if (ret == 0)
	{
		Team *pTeam = Team::GetTeam(teamId);
		if (pTeam == NULL)
		{
			return 190500025;
		}
		if (pTeam->IsAutoAccept())
		{
			pTeam->AddMember(*player);
		}
	}

	return 0;
}

int handle_chat_broadcast_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player)
		return (-1);

	if (!player->is_avaliable())
	{
		LOG_ERR("%s: player[%lu] do not have scene or data", __FUNCTION__, extern_data->player_id);
		return -10;
	}

	//todo 扣喇叭
	std::map<uint64_t, struct ParameterTable *>::iterator it = parameter_config.find(161000004);
	if (it == parameter_config.end())
	{
		return -3;
	}
	Chat *req = chat__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	if (req->costtype == CHAT_BROADCAST_TYPE__ITEM)
	{
		if (player->del_item(201070023, it->second->parameter1[0], MAGIC_TYPE_BAG_USE) != 0)
			goto done;
	}
	else
	{
		if (player->sub_comm_gold(it->second->parameter1[1], MAGIC_TYPE_BAG_USE) != 0)
			goto done;
	}
	conn_node_gamesrv::send_to_all_player(MSG_ID_CHAT_BROADCAST_NOTIFY, req, (pack_func)chat__pack);

done:
	chat__free_unpacked(req, NULL);

	return 0;
}

int handle_quit_team_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}


	Team *pTeam = player->m_team;
	if (pTeam == NULL)
	{
		return 2;
	}
	if (player->is_in_pvp_raid())
	{
		return 3;
	}

	pTeam->RemoveMember(*player);
	if (pTeam->GetMemberSize() == 0)
	{
		Team::DestroyTeam(pTeam);
	}
	return 0;
}

int handle_team_change_lead_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	if (player->m_team == NULL)
	{
		return 2;
	}
	if (player->is_in_pvp_raid())
	{
		return 5;
	}
	TeamPlayerid *req = team_playerid__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	player_struct *otherPlayer = player_manager::get_player_by_id(req->id);
	team_playerid__free_unpacked(req, NULL);
	if (otherPlayer == NULL)
	{
		return 3;
	}
	if (player->get_uuid() != player->m_team->GetLeadId())
	{
		return 4;
	}

	if (player->m_team->ChangeLeader(*otherPlayer))
	{
		TeamPlayerid send;
		team_playerid__init(&send);
		send.id = otherPlayer->get_uuid();
		player->m_team->BroadcastToTeam(MSG_ID_TEAM_CHANGE_LEAD_NOTIFY, &send, (pack_func)team_playerid__pack);
		player->m_team->SendApplyList(*otherPlayer);
	}

	return 0;
}

int handle_team_list_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	if (player->m_team != NULL)
	{
		return 2;
	}
	TeamTarget *req = team_target__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	if (req->type == TEAM_TARGET_TYPE__fb)
	{
		Team::GetTargetTeamList(req->target, *player);
	}
	else
	{
		Team::GetNearTeamList(*player);
	}
	team_target__free_unpacked(req, NULL);

	return 0;
}

int handle_be_team_lead_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	if (player->m_team == NULL)
	{
		return 1;
	}
	if (player->m_team->GetLeadId() != player->get_uuid())
	{
		return 2;
	}

	TeamBeLead *req = team_be_lead__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	player_struct *otherPlayer = player_manager::get_player_by_id(req->playerid);
	int chose = req->chose;

	BeLeadAnswer send;
	be_lead_answer__init(&send);
	int ret = 0;
	if (otherPlayer == NULL)
	{
		ret = 190500023;

	}
	else
	{
		if (chose == TEAM_CHOSE__YES)
		{
			if (player->m_team->ChangeLeader(*otherPlayer))
			{
				TeamPlayerid send;
				team_playerid__init(&send);
				send.id = otherPlayer->get_uuid();
				player->m_team->BroadcastToTeam(MSG_ID_TEAM_CHANGE_LEAD_NOTIFY, &send, (pack_func)team_playerid__pack);
				player->m_team->SendApplyList(*otherPlayer);
			}
			else
			{
				ret = 190500018;
			}
		}
		else
		{
			TeamMemInfo mem;
			player->m_team->PackMemberInfo(mem, *player);
			EXTERN_DATA ext_data;
			ext_data.player_id = otherPlayer->get_uuid();
			fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_TEAM_REFUCE_BE_LEAD_NOTIFY, team_mem_info__pack, mem);
		}
	}

	send.ret = ret;
	send.name = req->name;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TEAM_HANDLE_BE_LEAD_ANSWER, be_lead_answer__pack, send);

	team_be_lead__free_unpacked(req, NULL);
	return 0;
}

int handle_team_be_lead_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	if (player->m_team == NULL)
	{
		return 2;
	}
	if (player->is_in_pvp_raid())
	{
		return 4;
	}

	int cd = player->m_team->CkeckApplyCd(extern_data->player_id);
	TeamNotifyCd note;
		team_notify_cd__init(&note);
		note.cd = cd;
		fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TEAM_BE_LEAD_ANSWER, team_notify_cd__pack, note);
	if (cd != 0)
	{
		return 3;
	}

	player_struct *lead = player->m_team->GetLead();
	if (lead != NULL)
	{
		TeamPlayerid send;
		team_playerid__init(&send);
		send.id = player->get_uuid();
		EXTERN_DATA ext_data;
		ext_data.player_id = lead->get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_TEAM_BE_LEAD_NOTIFY, team_playerid__pack, send);
	}

	return 0;
}

int handle_kick_team_mem_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	if (player->m_team == NULL)
	{
		return 2;
	}
	TeamPlayerid *req = team_playerid__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	player_struct *otherPlayer = player_manager::get_player_by_id(req->id);
	uint64_t otherid = req->id;
	team_playerid__free_unpacked(req, NULL);

	if (player->is_in_pvp_raid())
	{
		return 5;
	}
	if (player->get_uuid() != player->m_team->GetLeadId())
	{
		return 4;
	}
	if (otherPlayer == NULL)
	{
		player->m_team->RemoveMember(otherid, true);
	}
	else
	{
		player->m_team->RemoveMember(*otherPlayer, true);
	}

	return 0;
}

int handle_team_invite_request_impl(player_struct *player, uint64_t id)
{
	player_struct *otherPlayer = player_manager::get_player_by_id(id);

	if (otherPlayer == NULL)
	{
		return 190500034;
	}
	if (otherPlayer->m_team != NULL)
	{
		return 190500033;
	}
	if (otherPlayer->data->scene_id > SCENCE_DEPART)
	{
		return 190500065;
	}
	if (otherPlayer->data->team_invite_switch != 0)
	{
		return ERROR_ID_TEAM_INVITE_CLOSE;
	}
	if (player->m_team != NULL && !player->m_team->CheckLevel(otherPlayer->get_attr(PLAYER_ATTR_LEVEL)))
	{
		return 190500086;
	}
	if (otherPlayer->m_inviter.find(player->get_uuid()) != otherPlayer->m_inviter.end())
	{
		return 0;
	}
	TeamInvite send;
	team_invite__init(&send);
	TeamMemInfo mem;
	player->m_team->PackMemberInfo(mem, *player);
	if (player->m_team != NULL)
	{
		send.teamid = player->m_team->GetId();
		send.target = player->m_team->GetTarget();
	}
	send.lead = &mem;

	EXTERN_DATA ext_data;
	ext_data.player_id = otherPlayer->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_INVITE_TEAM_NOTIFY, team_invite__pack, send);
	//otherPlayer->m_inviter.insert(otherPlayer->get_uuid());

	return 0;
}
int handle_team_invite_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	if (player->data->scene_id > SCENCE_DEPART)
	{
		return 3;
	}

	//if (player->m_team == NULL)
	//{
	//	int ret = Team::CreateTeam(*player);
	//	if (ret != 0)
	//	{
	//		send_comm_answer(MSG_ID_CREATE_TEAM_ANSWER, ret, extern_data);
	//		return 2;
	//	}

	//}
	TeamPlayerid *req = team_playerid__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	int ret = handle_team_invite_request_impl(player, req->id);
	team_playerid__free_unpacked(req, NULL);

	send_comm_answer(MSG_ID_INVITE_TEAM_ANSWER, ret, extern_data);

	return 0;
}

int handle_team_invite_handle_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	player->m_inviter.erase(player->get_uuid());
	HandleTeamInvite *req = handle_team_invite__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	//Team *pTeam = Team::GetTeam(req->teamid);
	player_struct *otherPlayer = player_manager::get_player_by_id(req->playerid);
	int accept = req->accept;
	handle_team_invite__free_unpacked(req, NULL);
	//if (pTeam == NULL)
	//{
	//	return 2;
	//}
	if (otherPlayer == NULL)
	{
		return 3;
	}
	if (otherPlayer->data->scene_id > SCENCE_DEPART)
	{
		return 4;
	}
	EXTERN_DATA ext_data;
	ext_data.player_id = otherPlayer->get_uuid();
	if (accept == TEAM_CHOSE__YES)
	{
		if (otherPlayer->m_team == NULL)
		{
			int ret = Team::CreateTeam(*otherPlayer);
			if (ret != 0)
			{
				send_comm_answer(MSG_ID_CREATE_TEAM_ANSWER, ret, &ext_data);
				return 2;
			}

		}
		otherPlayer->m_team->AddMember(*player);
	}
	else if (otherPlayer != NULL)
	{
		TeamMemInfo mem;
		Team::PackMemberInfo(mem, *player);
		fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_REFUCE_INVITE_TEAM_NOTIFY, team_mem_info__pack, mem);
	}


	return 0;
}

int handle_team_setlimit_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	if (player->m_team == NULL)
	{
		return 2;
	}

	if (player->get_uuid() != player->m_team->GetLeadId())
	{
		return 4;
	}

	TeamLimited *req = team_limited__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	player->m_team->SetLimited(*req, *player);
	player->m_team->BroadcastToTeam(MSG_ID_TEAM_LIMITED_NOTIFY, req, (pack_func)team_limited__pack);
	team_limited__free_unpacked(req, NULL);

	return 0;
}

int handle_handle_apply_team_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	if (player->m_team == NULL)
	{
		return 2;
	}
	if (player->get_uuid() != player->m_team->GetLeadId())
	{
		return 3;
	}
	HandleTeamApply *req = handle_team_apply__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	player->m_team->RemoveApply(req->id);

	int ret = 0;
	player_struct *applyer = player_manager::get_player_by_id(req->id);
	if (req->chose == TEAM_CHOSE__YES)
	{
		if (applyer == NULL)
		{
			ret = 190500084;
			goto done;
		}
		if (applyer->m_team != NULL)
		{
			ret = 190500085;
			goto done;
		}
		if (!player->m_team->CheckLevel(applyer->get_attr(PLAYER_ATTR_LEVEL)))
		{
			ret = 190500086;
			goto done;
		}
		player->m_team->AddMember(*applyer);
	}
	else
	{
		if (applyer != NULL)
		{
			RefuceApplyTeam note;
			refuce_apply_team__init(&note);
			note.name = player->data->name;
			EXTERN_DATA ext;
			ext.player_id = applyer->get_uuid();
			fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_REFUCE_APPLY_TEAM_NOTIFY, refuce_apply_team__pack, note);
		}
	}

done:
	send_comm_answer(MSG_ID_HANDLE_APPLY_TEAM_ANSWER, ret, extern_data);

	handle_team_apply__free_unpacked(req, NULL);

	//player->m_team->SendApplyList(*player);

	return 0;
}

static int handle_team_match_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	if (player->m_team != NULL)
	{
		return 2;
	}
	TeamTarget *req = team_target__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	TeamMatch::AddRole(*player, req->target);

	MatchAnser resp;
	match_anser__init(&resp);
	resp.target = req->target;
	resp.ret = 0;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TEAM_MATCH_ANSWER, match_anser__pack, resp);

	team_target__free_unpacked(req, NULL);

	return 0;
}

static int handle_team_lead_pos_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	if (player->m_team == NULL)
	{
		return 2;
	}

	player_struct *lead = player->m_team->GetLead();
	if (lead == NULL) //不在线
	{
		return 3;
	}

	SyncPlayerPosNotify resp;
	sync_player_pos_notify__init(&resp);
	resp.player_id = lead->get_uuid();
	resp.scene_id = lead->data->scene_id;
	resp.pos_x = lead->get_pos()->pos_x;
	resp.pos_z = lead->get_pos()->pos_z;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TEAM_LEAD_POS_ANSWER, sync_player_pos_notify__pack, resp);

	return 0;
}

static int handle_team_summon_mem_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	if (player->m_team == NULL)
	{
		return 2;
	}
	if (player->m_team->GetLeadId() != player->get_uuid())
	{
		return 3;
	}
	player->m_team->SummonMem();
	return 0;
}

static int handle_team_set_follow_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	if (player->m_team == NULL)
	{
		return 2;
	}

	Follow *req = follow__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	player->m_team->SetFollow(*player, req->state);
	follow__free_unpacked(req, NULL);
	return 0;
}

static int handle_cancel_team_match_request(player_struct *player, EXTERN_DATA *extern_data)
{
	uint64_t target = TeamMatch::DelRole(player->get_uuid());

	MatchAnser resp;
	match_anser__init(&resp);
	resp.target = target;
	resp.ret = 0;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TEAM_CANCEL_MATCH_ANSWER, match_anser__pack, resp);
	return 0;
}

static void on_login_send_all_horse(player_struct *player, EXTERN_DATA *extern_data)
{
	std::map<uint64_t, struct SpiritTable*>::iterator it = spirit_config.begin();
	for (int i = 0; i < MAX_HORSE_ATTR_NUM; ++i)
	{
		player->data->horse_attr.attr[i] = it->second->SpiritAttribute[i];
	}
	if (player->data->horse_attr.step == 0)
	{
		//player->add_horse(DEFAULT_HORSE, 0);
		player->data->horse_attr.step = 1;
		player->data->horse_attr.soul = 1;
		for (int i = 0; i < MAX_HORSE_ATTR_NUM; ++i)
		{
			player->data->horse_attr.attr_exp[i] = 0;
		}
		memset(player->data->horse_attr.soul_exp, 0, sizeof(player->data->horse_attr.soul_exp));
		player->data->horse_attr.cur_soul = 1;
		player->data->horse_attr.soul_full = false;
		player->data->horse_attr.fly = 1;
	}
	HorseData horse[MAX_HORSE_NUM];
	HorseData *horsePoint[MAX_HORSE_NUM];
	HorseList sendHorse;
	horse_list__init(&sendHorse);
	uint32_t i = 0;
	for (uint32_t j = 0; j < player->data->n_horse; ++j)
	{
		horse_data__init(&horse[i]);
		horse[i].id = player->data->horse[i].id;
		horse[i].isnew = player->data->horse[i].isNew;

		if (player->data->horse[i].timeout != 0)
		{
			if (player->data->horse[i].timeout <= (time_t)time_helper::get_cached_time() / 1000)
			{
				horse[i].isexpire = true;
				continue;
			}
			else
			{
				horse[i].cd = player->data->horse[i].timeout - time_helper::get_cached_time() / 1000;
				horse[i].isexpire = false;
			}
		}
		else
		{
			horse[i].isexpire = false;
			horse[i].cd = 0;
		}
		if (horse[i].id == player->get_attr(PLAYER_ATTR_CUR_HORSE))
		{
			horse[i].is_current = true;
		}
		else
		{
			horse[i].is_current = false;
		}
		horsePoint[i] = &horse[i];
		++i;
	}

	HorseCommonAttr sendAttr;
	horse_common_attr__init(&sendAttr);
	sendAttr.step = player->data->horse_attr.step;
	sendAttr.cur_soul = player->data->horse_attr.cur_soul;
	sendAttr.soul_level = player->data->horse_attr.soul;
	//sendAttr.n_soul_level = MAX_HORSE_SOUL;
	sendAttr.soul_num = &(player->data->horse_attr.soul_exp[1]);
	sendAttr.n_soul_num = MAX_HORSE_SOUL;
	sendAttr.attr = player->data->horse_attr.attr;
	sendAttr.attr_level = player->data->horse_attr.attr_exp;
	sendAttr.n_attr = sendAttr.n_attr_level = MAX_HORSE_ATTR_NUM;
	player->calc_horse_attr();
	player->calculate_attribute(true);
	sendAttr.power = player->data->horse_attr.power;
	sendAttr.soul_full = player->data->horse_attr.soul_full;

	sendHorse.n_data = i;
	sendHorse.data = horsePoint;
	sendHorse.attr = &sendAttr;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_HORSE_LIST_NOTIFY, horse_list__pack, sendHorse);
}
void pack_fashion_info(FashionData &data, player_struct *player, int ind)
{
	fashion_data__init(&data);
	data.id = player->data->fashion[ind].id;
	data.color = player->data->fashion[ind].color;
	data.isnew = player->data->fashion[ind].isNew;
	data.ison = false;

	if (player->data->fashion[ind].timeout != 0)
	{
		if (player->data->fashion[ind].timeout <= (time_t)time_helper::get_cached_time() / 1000)
		{
			data.isexpire = true;
		}
		else
		{
			data.cd = player->data->fashion[ind].timeout - time_helper::get_cached_time() / 1000;
			data.isexpire = false;
		}
	}
	else
	{
		data.isexpire = false;
		data.cd = 0;
	}
	std::map<uint64_t, struct ActorFashionTable *>::iterator itFashion = fashion_config.find(data.id);
	if (itFashion != fashion_config.end())
	{
		if (itFashion->second->Type == FASHION_TYPE_WEAPON)
		{
			if (data.id != player->get_attr(PLAYER_ATTR_WEAPON))
			{
				data.ison = false;
			}
			else
			{
				data.ison = true;
			}
		}
		else if (itFashion->second->Type == FASHION_TYPE_HAT)
		{
			if (data.id != player->get_attr(PLAYER_ATTR_HAT))
			{
				data.ison = false;
			}
			else
			{
				data.ison = true;
			}
		}
		else if (itFashion->second->Type == FASHION_TYPE_CLOTHES)
		{
			if (data.id != player->get_attr(PLAYER_ATTR_CLOTHES))
			{
				data.ison = false;
			}
			else
			{
				data.ison = true;
			}
		}
	}
}
static void on_login_send_all_fashion(player_struct *player, EXTERN_DATA *extern_data)
{
	if (player->data->n_fashion == 0)
	{
		std::map<uint64_t, struct ActorFashionTable *>::iterator itFashion = fashion_config.begin();
		for (; itFashion != fashion_config.end(); ++itFashion)
		{
			if (itFashion->second->Occupation == player->get_job()
				&& itFashion->second->Lock == 2 && itFashion->second->ListAcc == 1)
			{
				player->add_fashion(itFashion->first, itFashion->second->Colour, 0);
			}
		}
		for (uint32_t i = 0; i < player->data->n_fashion; ++i)
		{
			player->data->fashion[i].isNew = false;
		}
		player->data->charm_level = 1;
	}

	//染色
	uint32_t color[MAX_COLOR_NUM] = { 131000001,131000002,131000003,131000004,131000005,};
	bool colorNew[MAX_COLOR_NUM] = { 0,0,0,0,0, };
	ColorList sendColor;
	color_list__init(&sendColor);
	if (player->data->n_color == 0)
	{
		player->data->n_color = 5;
		memcpy(player->data->color, color, sizeof(uint32_t)*player->data->n_color);
		memcpy(player->data->color_is_new, colorNew, sizeof(uint32_t) * player->data->n_color);
	}
	sendColor.n_isnew = sendColor.n_color = player->data->n_color;
	sendColor.color = player->data->color;
	sendColor.isnew = player->data->color_is_new;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_COLOR_LIST_NOTIFY, color_list__pack, sendColor);
	sendColor.n_isnew = sendColor.n_color = player->data->n_weapon_color;
	sendColor.color = player->data->weapon_color;
	sendColor.isnew = player->data->weapon_color_is_new;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_WEAPON_COLOR_LIST_NOTIFY, color_list__pack, sendColor);

	//时装
	FashionData fashion[MAX_FASHION_NUM];
	FashionData *fashionPoint[MAX_FASHION_NUM];
	FashionList sendFashion;
	fashion_list__init(&sendFashion);
	int num = 0;
	for (uint32_t i = 0; i < player->data->n_fashion; ++i)
	{
		if (player->data->fashion[i].timeout != 0)
		{
			if (player->data->fashion[i].timeout <= (time_t)time_helper::get_cached_time() / 1000)
			{
				continue;
			}
		}
		pack_fashion_info(fashion[num], player, i);
		fashionPoint[num] = &fashion[num];
		++num;
	}
	sendFashion.n_data = num;
	sendFashion.data = fashionPoint;
	sendFashion.level = player->data->charm_level;
	sendFashion.charm = player->data->charm_total;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_FASHION_LIST_NOTIFY, fashion_list__pack, sendFashion);
}


static int handle_handle_refuce_apply_team_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	if (player->m_team == NULL)
	{
		return 2;
	}
	if (player->get_uuid() != player->m_team->GetLeadId())
	{
		return 3;
	}

	RefuceApplyTeam note;
	refuce_apply_team__init(&note);
	note.name = player->data->name;
	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_REFUCE_APPLY_TEAM_NOTIFY, &note, (pack_func)refuce_apply_team__pack);
	PROTO_HEAD_CONN_BROADCAST *head;
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
	head->num_player_id = player->m_team->m_data->m_applySize;
	memcpy(ppp, player->m_team->m_data->m_applyList, sizeof(uint64_t) * head->num_player_id);
	if (head->num_player_id > 0)
	{
		head->len += sizeof(uint64_t) * head->num_player_id;
		conn_node_gamesrv::broadcast_msg_send();
	}

	player->m_team->RemoveAllApply();

	CommAnswer resp;
	comm_answer__init(&resp);
	resp.result = 0;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_HANDLE_REFUCE_APPLY_TEAM_ANSWER, comm_answer__pack, resp);

	//player->m_team->SendApplyList(*player);

	return 0;
}

//吟唱中断请求
static int handle_sing_interrupt_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	int ret = 0;
	do
	{
		player->interrupt_sing();
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_SING_INTERRUPT_ANSWER, comm_answer__pack, resp);

	return 0;
}

//吟唱结束请求
static int handle_sing_end_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	int ret = 0;
	do
	{
		if (player->data->sing_info.type == 0)
		{
			ret = ERROR_ID_NOT_SINGING;
			LOG_ERR("[%s:%d] player[%lu] not singing", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		uint64_t cur_time = time_helper::get_cached_time();
		if (player->data->sing_info.start_ts + (uint64_t)player->data->sing_info.time > cur_time)
		{
			ret = ERROR_ID_SING_TIME;
			LOG_ERR("[%s:%d] player[%lu] sing time, start_ts:%lu, time:%u, cur_time:%lu", __FUNCTION__, __LINE__,
				extern_data->player_id, player->data->sing_info.start_ts, player->data->sing_info.time, cur_time);
			break;
		}

		player->end_sing();
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_SING_END_ANSWER, comm_answer__pack, resp);

	return 0;
}

//吟唱开始请求
static int handle_sing_begin_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	SingBeginRequest *req = sing_begin_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t sing_type = req->type;
	uint32_t sing_time = req->time;

	sing_begin_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		bool broadcast = false;
		bool include_myself = false;
		if (sing_type == SING_TYPE__TASK)
		{
			broadcast = false;
			include_myself = false;
		}
		player->begin_sing(sing_type, sing_time, broadcast, include_myself, NULL);
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_SING_BEGIN_ANSWER, comm_answer__pack, resp);

	return 0;
}

static int send_transfer_to_leader_notify(uint64_t player_id, uint64_t leader_player_id)
{
	EXTERN_DATA extern_data;
	extern_data.player_id = player_id;
	TeamMemberRefuseTransferNotify notify;
	team_member_refuse_transfer_notify__init(&notify);
	notify.player_id = leader_player_id;

	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data,
		MSG_ID_TRANSFER_TO_LEADER_NOTIFY, team_member_refuse_transfer_notify__pack, notify);
//	fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_TRANSFER_TO_LEADER_NOTIFY, 0, 0);
	return (0);
}

static int send_team_member_refuse_transfer_notify(player_struct *player, player_struct *leader)
{
	EXTERN_DATA extern_data;
	extern_data.player_id = leader->get_uuid();
	TeamMemberRefuseTransferNotify notify;
	team_member_refuse_transfer_notify__init(&notify);
	notify.player_id = player->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data,
		MSG_ID_TEAM_MEMBER_REFUSE_TRANSFER_NOTIFY, team_member_refuse_transfer_notify__pack, notify);
	return (0);
}

// static int send_enter_raid_fail(player_struct *player, uint32_t err_code, uint32_t n_reason_player, uint64_t *reason_player_id, uint32_t item_id)
// {
//	EnterRaidAnswer resp;
//	enter_raid_answer__init(&resp);
//	resp.result = err_code;
//	resp.n_reson_player_id = n_reason_player;
//	resp.reson_player_id = reason_player_id;
//	resp.item_id = item_id;

//	EXTERN_DATA extern_data;
//	extern_data.player_id = player->get_uuid();
//	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ENTER_RAID_ANSWER, enter_raid_answer__pack, resp);
//	return (0);
// }

// static int check_enter_raid_reward_time(player_struct *player, uint32_t id, struct ControlTable *config)
// {
//	if (config->TimeType != 1)
//		return (0);
//	uint32_t count = player->get_raid_reward_count(id);
//	if (count >= config->RewardTime)
//		return 10;
//	return (0);
// }

// static int check_enter_raid_cost(player_struct *player, struct DungeonTable *r_config)
// {
//		//不消耗
//	if (r_config->CostItemID == 0)
//		return (0);

//	switch (get_item_type(r_config->CostItemID))
//	{
//		case ITEM_TYPE_COIN: //金钱
//			if (player->get_attr(PLAYER_ATTR_COIN) < r_config->CostNum)
//				return 5;
//			break;
//		case ITEM_TYPE_BIND_GOLD: //元宝
//		case ITEM_TYPE_GOLD:
//			if (player->get_comm_gold() < (int)r_config->CostNum)
//				return 6;
//			break;
//		default: //道具
//			if (player->get_item_can_use_num(r_config->CostItemID) < (int)r_config->CostNum)
//				return 4;
//			break;
//	}
//	return (0);
// }

// static int do_enter_raid_cost(player_struct *player, uint32_t item_id, uint32_t item_num)
// {
//	if (item_id == 0 || item_num == 0)
//		return (0);
//	switch (get_item_type(item_id))
//	{
//		case ITEM_TYPE_COIN: //金钱
//			player->sub_coin(item_num, MAGIC_TYPE_ENTER_RAID, true);
//			break;
//		case ITEM_TYPE_BIND_GOLD: //元宝
//		case ITEM_TYPE_GOLD:
//			player->sub_comm_gold(item_num, MAGIC_TYPE_ENTER_RAID, true);
//			break;
//		default: //道具
//			player->del_item(item_id, item_num, MAGIC_TYPE_ENTER_RAID, true);
//			break;
//	}
//	return (0);
// }

// static int check_player_enter_raid(player_struct *player, uint32_t raid_id)
// {
//	uint32_t n_reason_player = 0;
//	uint64_t reason_player_id[MAX_TEAM_MEM];
//	struct DungeonTable *r_config = get_config_by_id(raid_id, &all_raid_config);
//	struct ControlTable *control_config = get_config_by_id(r_config->ActivityControl, &all_control_config);

//	struct SceneResTable *s_config = get_config_by_id(raid_id, &scene_res_config);
//	if (!r_config || !s_config)
//	{
//		LOG_ERR("%s %d: player[%lu] raid[%u]", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
//		return (-1);
//	}

//		//万妖谷小关卡不能直接进入
//	if (r_config->DengeonRank == 4)
//	{
//		LOG_ERR("%s %d: player[%lu] raid[%u]", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
//		return (-2);
//	}

//		//当前在副本中
//	if (player->scene && player->scene->get_scene_type() == SCENE_TYPE_RAID)
//	{
//		LOG_ERR("%s %d: player[%lu] raid[%u]", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
//		return (-5);
//	}

//		//当前在位面副本中
//	if (player->sight_space)
//	{
//		LOG_ERR("%s %d: player[%lu] sightspace[%p]", __FUNCTION__, __LINE__, player->get_uuid(), player->sight_space);
//		return (-6);
//	}

//		//检查等级
//	if (player->get_attr(PLAYER_ATTR_LEVEL) < s_config->level)
//	{
//		LOG_ERR("%s %d: player[%lu] raid[%u]", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
//		return (-10);
//	}
//		// TODO: 表已经改过了
//		//检查时间
//	// if (control_config->n_OpenTime > 0 && control_config->OpenTime[0] != 0)
//	// {
//	//	bool pass = false;
//	//	uint32_t week = time_helper::getWeek(time_helper::get_cached_time() / 1000);
//	//	for (size_t i = 0; i < control_config->n_OpenTime; ++i)
//	//	{
//	//		if (week == control_config->OpenTime[i])
//	//		{
//	//			pass = true;
//	//			break;
//	//		}
//	//	}
//	//	if (pass == false)
//	//	{
//	//		LOG_ERR("%s %d: player[%lu] raid[%u]", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
//	//		send_enter_raid_fail(player, 8, 0, NULL, 0);
//	//		return (-20);
//	//	}
//	// }

//		//个人副本通过
//	if (r_config->DengeonType == 2)
//	{
//		if (player->m_team)
//		{
//			LOG_ERR("%s %d: player[%lu] raid[%u]", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
//			return (-30);
//		}

//		int ret = check_enter_raid_reward_time(player, raid_id, control_config);
//		if (ret != 0)
//		{
//			reason_player_id[0] = player->data->player_id;
//			send_enter_raid_fail(player, 10, 1, reason_player_id, 0);
//			return (-32);
//		}

//		ret = check_enter_raid_cost(player, r_config);
//		if (ret != 0)
//		{
//			reason_player_id[0] = player->data->player_id;
//			send_enter_raid_fail(player, ret, 1, reason_player_id, r_config->CostItemID);
//			return (-35);
//		}
//		do_enter_raid_cost(player, r_config->CostItemID, r_config->CostNum);

//		return (0);
//	}

//	if (r_config->DengeonRank == 9) //阵营战副本
//	{
//		return (0);
//	}

//		//组队副本检查队伍
//	if (!player->m_team)
//	{
//		LOG_ERR("%s %d: player[%lu] raid[%u]", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
//		return (-40);
//	}

//		//是否是队长
//	if (player->get_uuid() != player->m_team->GetLeadId())
//	{
//		LOG_ERR("%s %d: player[%lu] raid[%u]", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
//		return (-50);
//	}

//		//检查人数
//	uint32_t team_mem_num = player->m_team->GetMemberSize();
//	if (control_config->MinActor > team_mem_num
//		|| control_config->MaxActor < team_mem_num)
//	{
//		LOG_ERR("%s %d: player[%lu] raid[%u]", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
//		return (-60);
//	}

//	bool pass = true;
//		//检查是否有人离线
//	for (int pos = 0; pos < player->m_team->m_data->m_memSize; ++pos)
//	{
//		if (player->m_team->m_data->m_mem[pos].timeremove != 0)
//		{
// //			send_enter_raid_fail(player, 7, player->m_team->m_data->m_mem[pos].id, 0);
// //			return (-70);
//			reason_player_id[n_reason_player++] = player->m_team->m_data->m_mem[pos].id;
//			pass = false;
//		}
//	}
//	if (!pass)
//	{
//		send_enter_raid_fail(player, 7, n_reason_player, reason_player_id, 0);
//		return (-70);
//	}


//		//检查等级, 消耗
//	player_struct *team_players[MAX_TEAM_MEM];
//	uint32_t i = 0;
//	for (int pos = 0; pos < player->m_team->m_data->m_memSize; ++pos)
//	{
//		player_struct *t_player = player_manager::get_player_by_id(player->m_team->m_data->m_mem[pos].id);
//		if (!t_player)
//		{
//			LOG_ERR("%s: can not find team mem %lu", __FUNCTION__, player->m_team->m_data->m_mem[pos].id);
// //			send_enter_raid_fail(player, 2, player->m_team->m_data->m_mem[pos].id, 0);
//			return (-50);
//		}
//		if (t_player->get_attr(PLAYER_ATTR_LEVEL) < s_config->level)
//		{
//			pass = false;
//			reason_player_id[n_reason_player++] = player->m_team->m_data->m_mem[pos].id;
// //			send_enter_raid_fail(player, 2, t_player->get_uuid(), 0);
// //			return (-60);
//		}

// //		int ret = check_enter_raid_cost(t_player, r_config);
// //		if (ret != 0)
// //		{
// //			send_enter_raid_fail(player, ret, t_player->data->player_id, r_config->CostItemID);
// //			return (-65);
// //		}

//		team_players[i++] = t_player;
//	}
//	if (!pass)
//	{
//		send_enter_raid_fail(player, 2, n_reason_player, reason_player_id, 0);
//		return (-60);
//	}

// //// 检查收益次数
//	for (int pos = 0; pos < player->m_team->m_data->m_memSize; ++pos)
//	{
//		player_struct *t_player = team_players[pos];
//		int ret = check_enter_raid_reward_time(t_player, raid_id, control_config);
//		if (ret != 0)
//		{
//			pass = false;
//			reason_player_id[n_reason_player++] = player->m_team->m_data->m_mem[pos].id;
//		}
//	}
//	if (!pass)
//	{
//		send_enter_raid_fail(player, 10, n_reason_player, reason_player_id, r_config->CostItemID);
//		return (-62);
//	}
// //// 检查消耗
//	int fail_ret;
//	for (int pos = 0; pos < player->m_team->m_data->m_memSize; ++pos)
//	{
//		player_struct *t_player = team_players[pos];
//		int ret = check_enter_raid_cost(t_player, r_config);
//		if (ret != 0)
//		{
//			fail_ret = ret;
//			pass = false;
//			reason_player_id[n_reason_player++] = player->m_team->m_data->m_mem[pos].id;
// //			send_enter_raid_fail(player, ret, t_player->data->player_id, r_config->CostItemID);
// //			return (-65);
//		}
//	}
//	if (!pass)
//	{
//		send_enter_raid_fail(player, fail_ret, n_reason_player, reason_player_id, r_config->CostItemID);
//		return (-65);
//	}
// ////


// //	if (i != team_mem_num)
// //	{
// //		LOG_ERR("%s %d: player[%lu] raid[%u]", __FUNCTION__, __LINE__, player->get_uuid(), raid_id);
// //		return (-70);
// //	}

//		//检查距离
//	struct position *leader_pos = player->get_pos();
// //	uint64_t too_far_player_id = 0;
//	for (i = 1; i < team_mem_num; ++i)
//	{
//		player_struct *t_player = team_players[i];

//		if (player->scene != t_player->scene)
//		{
//			pass = false;
//			reason_player_id[n_reason_player++] = t_player->get_uuid();
//			continue;
//		}

//		struct position *pos = t_player->get_pos();
//		uint64_t x = (pos->pos_x - leader_pos->pos_x);
//		uint64_t z = (pos->pos_z - leader_pos->pos_z);
//		if (x * x + z * z > max_team_mem_distance)
//		{
//			pass = false;
//			reason_player_id[n_reason_player++] = t_player->get_uuid();
// //			too_far_player_id = t_player->get_uuid();
// //			send_transfer_to_leader_notify(too_far_player_id);
//		}
//	}

// //	if (too_far_player_id != 0)
//	if (!pass)
//	{
//		send_enter_raid_fail(player, 3, n_reason_player, reason_player_id, 0);
//		return (-80);
//	}

//	for (int pos = 0; pos < player->m_team->m_data->m_memSize; ++pos)
//	{
//		player_struct *t_player = player_manager::get_player_by_id(player->m_team->m_data->m_mem[pos].id);
//		do_enter_raid_cost(t_player, r_config->CostItemID, r_config->CostNum);
//	}

//	return (0);
// }
// /*
// static int player_team_enter_raid(player_struct *player, raid_struct *raid)
// {
//	for (std::vector<MEM_INFO>::iterator ite = player->m_team->m_mem.begin(); ite != player->m_team->m_mem.end(); ++ite)
//	{
//		player_struct *t_player = player_manager::get_player_by_id((*ite).id);
//		assert(t_player);
//		if (raid->player_enter_raid(t_player, true) != 0)
//		{
//			LOG_ERR("%s: player[%lu] enter raid failed", __FUNCTION__, t_player->get_uuid());
//		}
//	}
//	return (0);
// }
// */


static int handle_planes_raid_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player->sight_space)
	{
		LOG_ERR("%s: player[%lu] do not have sightspace", __FUNCTION__, extern_data->player_id);
		return (-1);
	}
	sight_space_manager::del_player_from_sight_space(player->sight_space, player, true);
	return (0);
}

static int handle_team_raid_ready_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);	
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	if (!player->m_team)
	{
		LOG_ERR("%s %d: player[%lu] do not have team", __FUNCTION__, __LINE__, player->get_uuid());
		return (-10);
	}
	if (player->data->team_raid_id_wait_ready == 0)
	{
		LOG_ERR("%s %d: player[%lu] do not wait ready", __FUNCTION__, __LINE__, player->get_uuid());
		return (-20);		
	}
	if (player->data->is_team_raid_ready)
	{
		LOG_ERR("%s %d: player[%lu] already set ready", __FUNCTION__, __LINE__, player->get_uuid());
		return (-30);				
	}

	player->data->is_team_raid_ready = true;

	for (int pos = 0; pos < player->m_team->m_data->m_memSize; ++pos)
	{
		player_struct *t_player = player_manager::get_player_by_id(player->m_team->m_data->m_mem[pos].id);
		assert(t_player);
		assert(t_player->data);
		assert(t_player->data->team_raid_id_wait_ready == player->data->is_team_raid_ready);
		if (!t_player->data->is_team_raid_ready)
		{
			TeamRaidReadyNotify nty;
			team_raid_ready_notify__init(&nty);
			nty.player_id = player->get_uuid();
			player->m_team->BroadcastToTeam(MSG_ID_TEAM_RAID_READY_NOTIFY, &nty, (pack_func)team_raid_ready_notify__pack, 0);
			return (0);
		}
	}

		// 如果所有人都准备好了，就进入副本
	uint32_t raid_id = player->data->is_team_raid_ready;
	player->m_team->unset_raid_id_wait_ready();
	int ret = raid_manager::check_player_enter_raid(player, raid_id);
	if (ret != 1)
	{
		LOG_INFO("%s: player[%lu] enter raid[%u] failed", __FUNCTION__, extern_data->player_id, raid_id);
		TeamRaidCancelNotify nty;
		team_raid_cancel_notify__init(&nty);
		nty.player_id = player->get_uuid();
		nty.result = 1;
		player->m_team->BroadcastToTeam(MSG_ID_TEAM_RAID_CANCEL_NOTIFY, &nty, (pack_func)team_raid_cancel_notify__pack, 0);
		return -10;
	}
	raid_manager::do_team_enter_raid_cost(player, raid_id);
	raid_struct *raid = raid_manager::create_raid(raid_id, player);
	if (!raid)
	{
		LOG_ERR("%s: player[%lu] create raid[%u] failed", __FUNCTION__, extern_data->player_id, raid_id);
		return (-20);
	}
	assert(raid->res_config);
	raid->team_enter_raid(player->m_team);
	return (0);	
}
static int handle_team_raid_cancel_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	if (!player->m_team)
	{
		LOG_ERR("%s %d: player[%lu] do not have team", __FUNCTION__, __LINE__, player->get_uuid());
		return (-10);
	}
	if (player->data->team_raid_id_wait_ready == 0)
	{
		LOG_ERR("%s %d: player[%lu] do not wait ready", __FUNCTION__, __LINE__, player->get_uuid());
		return (-20);		
	}

	player->m_team->unset_raid_id_wait_ready();
	TeamRaidCancelNotify nty;
	team_raid_cancel_notify__init(&nty);
	nty.player_id = player->get_uuid();
	nty.result = 1;
	player->m_team->BroadcastToTeam(MSG_ID_TEAM_RAID_CANCEL_NOTIFY, &nty, (pack_func)team_raid_cancel_notify__pack, 0);
	return (0);
}

static int handle_enter_raid_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	EnterRaidRequest *req = enter_raid_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t raid_id = req->raid_id;
	enter_raid_request__free_unpacked(req, NULL);

	if (is_guild_wait_raid(raid_id))
	{
		int ret = 0;
		do
		{
			if (!is_guild_battle_opening())
			{
				ret = ERROR_ID_GUILD_BATTLE_ACTIVITY_NOT_OPEN;
				LOG_ERR("[%s:%d] player[%lu] guild battle not open, raid_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, raid_id);
				break;
			}

			ret = player_can_participate_guild_battle(player);
			if (ret != 0)
			{
				LOG_ERR("[%s:%d] player[%lu] can't participate, raid_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, raid_id);
				break;
			}

			if (player->m_team)
			{
				ret = -1;
				LOG_ERR("[%s:%d] player[%lu] in team, raid_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, raid_id);
				break;
			}

			guild_wait_raid_manager::add_player_to_guild_wait_raid(player);
		} while(0);

		if (ret != 0)
		{
			raid_manager::send_enter_raid_fail(player, ret, 0, NULL, 0);
		}
		return 0;
	}

	int ret = raid_manager::check_player_enter_raid(player, raid_id);
	switch (ret)
	{
		case 0: //进入副本
		case 1:
		{
			raid_struct *raid = raid_manager::create_raid(raid_id, player);
			if (!raid)
			{
				LOG_ERR("%s: player[%lu] create raid[%u] failed", __FUNCTION__, extern_data->player_id, raid_id);
				return (-20);
			}

			assert(raid->res_config);

			if (player->m_team)
			{
				raid->team_enter_raid(player->m_team);
			}
			else
			{
				if (raid->player_enter_raid(player, raid->res_config->BirthPointX, raid->res_config->BirthPointZ) != 0)
				{
					LOG_ERR("%s: player[%lu] enter raid failed", __FUNCTION__, player->get_uuid());
					return (-30);
				}
			}
		}
		break;
		// case 1:
		// {
		// 	assert(player->m_team);
		// 	TeamRaidWaitReadyNotify nty;
		// 	team_raid_wait_ready_notify__init(&nty);
		// 	nty.raid_id = raid_id;
		// 	player->m_team->BroadcastToTeam(MSG_ID_TEAM_RAID_WAIT_READY_NOTIFY, &nty, (pack_func)team_raid_wait_ready_notify__pack, 0);
		// 	player->m_team->set_raid_id_wait_ready(raid_id);
		// }
		// break;
		default:
			LOG_INFO("%s: player[%lu] enter raid[%u] failed", __FUNCTION__, extern_data->player_id, raid_id);
			return (-15);
	}

// 	raid_struct *raid = raid_manager::create_raid(raid_id, player);
// 	if (!raid)
// 	{
// 		LOG_ERR("%s: player[%lu] create raid[%u] failed", __FUNCTION__, extern_data->player_id, raid_id);
// 		return (-20);
// 	}

// 	assert(raid->res_config);

// 	if (player->m_team)
// 	{
// //		player_team_enter_raid(player, raid);
// 		raid->team_enter_raid(player->m_team);
// 	}
// 	else
// 	{
// 		if (raid->player_enter_raid(player, raid->res_config->BirthPointX, raid->res_config->BirthPointZ) != 0)
// 		{
// 			LOG_ERR("%s: player[%lu] enter raid failed", __FUNCTION__, player->get_uuid());
// 			return (-30);
// 		}
// 	}
 	return (0);
}
static int handle_leave_raid_request(player_struct *player, EXTERN_DATA *extern_data)
{
	raid_struct *raid = player->get_raid();
	if (!raid || !raid->data)
	{
		LOG_ERR("%s: player[%lu] not in raid", __FUNCTION__, extern_data->player_id);
		return (-1);
	}

	if (raid->m_config->DengeonType != 2 && raid->data->state == RAID_STATE_START && player->m_team)
	{
			//组队副本如果没结束，那么踢出队伍
		player->m_team->RemoveMember(*player, false);
	}
	else
	{
		raid->player_leave_raid(player);
	}
	return (0);
}

static int handle_transfer_far_team_member_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player->m_team)
		return (-1);
		//是否是队长
	if (player->get_uuid() != player->m_team->GetLeadId())
	{
		LOG_ERR("%s %d: player[%lu]", __FUNCTION__, __LINE__, player->get_uuid());
		return (-10);
	}

	uint32_t team_mem_num = player->m_team->GetMemberSize();
		//检查距离
	struct position *leader_pos = player->get_pos();
	for (uint32_t i = 1; i < team_mem_num; ++i)
	{
		player_struct *t_player = player_manager::get_player_by_id(player->m_team->m_data->m_mem[i].id);
		if (!t_player || !t_player->data)
		{
			continue;
		}

		if (player->scene != t_player->scene)
		{
			send_transfer_to_leader_notify(t_player->data->player_id, player->get_uuid());
			continue;
		}

		struct position *pos = t_player->get_pos();
		uint64_t x = (pos->pos_x - leader_pos->pos_x);
		uint64_t z = (pos->pos_z - leader_pos->pos_z);
		if (x * x + z * z > max_team_mem_distance)
		{
			send_transfer_to_leader_notify(t_player->data->player_id, player->get_uuid());
		}
	}
	return (0);
}

static int handle_transfer_to_leader_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}

	if (!player->m_team)
	{
		LOG_ERR("%s: player[%lu] not have team", __FUNCTION__, extern_data->player_id);
		return (-10);
	}

	if (player->get_uuid() == player->m_team->GetLeadId())
	{
		raid_manager::send_enter_raid_fail(player, 9, 0, NULL, 0);
//		LOG_ERR("%s: player[%lu] is team leader", __FUNCTION__, extern_data->player_id);
		return (-20);
	}

	player_struct *leader = player_manager::get_player_by_id(player->m_team->GetLeadId());
	if (!leader || !leader->is_avaliable())
	{
		raid_manager::send_enter_raid_fail(player, 9, 0, NULL, 0);
//		LOG_ERR("%s: player[%lu] no team leader", __FUNCTION__, extern_data->player_id);
		return (-30);
	}

	int ret = player->check_can_transfer();
	if (ret != 0)
	{
		raid_manager::send_enter_raid_fail(player, ret, 0, NULL, 0);
		return (-35);		
	}
	
	TransferToLeaderRequest *req = transfer_to_leader_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-40);
	}
	int answer = req->result;
	uint64_t leader_id = req->leader_id;
	transfer_to_leader_request__free_unpacked(req, NULL);
	if (answer != 0)
	{
		if (leader_id == leader->get_uuid())
			send_team_member_refuse_transfer_notify(player, leader);
		return 0;
	}

	if (leader_id != leader->get_uuid())
	{
		raid_manager::send_enter_raid_fail(player, 9, 0, NULL, 0);
		return (0);
	}

	if (leader->data->scene_id > SCENCE_DEPART)
	{
		return -3;
	}

	struct position *leader_pos = leader->get_pos();

	player->transfer_to_new_scene(leader->data->scene_id, leader_pos->pos_x, -1, leader_pos->pos_z, 0, extern_data);

	return (0);
}

//装备信息请求
static int handle_equip_list_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	return notify_equip_list(player, extern_data);
}

static int notify_equip_list(player_struct *player, EXTERN_DATA *extern_data)
{
	EquipListAnswer resp;
	equip_list_answer__init(&resp);

	EquipData equip_data[MAX_EQUIP_NUM];
	EquipData* equip_data_point[MAX_EQUIP_NUM];
	EquipEnchantData enchant_data[MAX_EQUIP_NUM][MAX_EQUIP_ENCHANT_NUM];
	EquipEnchantData* enchant_data_point[MAX_EQUIP_NUM][MAX_EQUIP_ENCHANT_NUM];
	AttrData cur_attr[MAX_EQUIP_NUM][MAX_EQUIP_ENCHANT_NUM];
	AttrData* cur_attr_point[MAX_EQUIP_NUM][MAX_EQUIP_ENCHANT_NUM];
	AttrData rand_attr[MAX_EQUIP_NUM][MAX_EQUIP_ENCHANT_NUM][MAX_EQUIP_ENCHANT_RAND_NUM];
	AttrData* rand_attr_point[MAX_EQUIP_NUM][MAX_EQUIP_ENCHANT_NUM][MAX_EQUIP_ENCHANT_RAND_NUM];

	resp.result = 0;
	size_t equip_num = 0;
	for (int k = 0; k < MAX_EQUIP_NUM; ++k)
	{
		EquipInfo &equip_info = player->data->equip_list[k];
		if (equip_info.stair == 0)
		{
			continue;
		}

		equip_data_point[equip_num] = &equip_data[equip_num];
		equip_data__init(&equip_data[equip_num]);

		equip_data[equip_num].type = k + 1;
		equip_data[equip_num].stair = equip_info.stair;
		equip_data[equip_num].starlv = equip_info.star_lv;
		equip_data[equip_num].starexp = equip_info.star_exp;
		size_t enchant_num = 0;
		for (int i = 0; i < MAX_EQUIP_ENCHANT_NUM; ++i)
		{
			EquipEnchantInfo &enchant_info = equip_info.enchant[i];

			enchant_data_point[equip_num][enchant_num] = &enchant_data[equip_num][enchant_num];
			equip_enchant_data__init(&enchant_data[equip_num][enchant_num]);
			enchant_data[equip_num][enchant_num].index = i;

			cur_attr_point[equip_num][enchant_num] = &cur_attr[equip_num][enchant_num];
			attr_data__init(&cur_attr[equip_num][enchant_num]);
			cur_attr[equip_num][enchant_num].id = enchant_info.cur_attr.id;
			cur_attr[equip_num][enchant_num].val = enchant_info.cur_attr.val;
			enchant_data[equip_num][enchant_num].curattr = cur_attr_point[equip_num][enchant_num];

			size_t rand_num = 0;
			if (enchant_info.rand_attr[0].id > 0)
			{
				for (int j = 0; j < MAX_EQUIP_ENCHANT_RAND_NUM; ++j)
				{
					rand_attr_point[equip_num][enchant_num][rand_num] = &rand_attr[equip_num][enchant_num][rand_num];
					attr_data__init(&rand_attr[equip_num][enchant_num][rand_num]);
					rand_attr[equip_num][enchant_num][rand_num].id = enchant_info.rand_attr[j].id;
					rand_attr[equip_num][enchant_num][rand_num].val = enchant_info.rand_attr[j].val;
					rand_num++;
				}
			}
			enchant_data[equip_num][enchant_num].randattr = rand_attr_point[equip_num][enchant_num];
			enchant_data[equip_num][enchant_num].n_randattr = rand_num;

			enchant_num++;
		}
		equip_data[equip_num].n_enchant = enchant_num;
		equip_data[equip_num].enchant = enchant_data_point[equip_num];

		equip_data[equip_num].inlay = &equip_info.inlay[0];
		equip_data[equip_num].n_inlay = MAX_EQUIP_INLAY_NUM;
		equip_num++;
	}
	resp.equips = equip_data_point;
	resp.n_equips = equip_num;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_EQUIP_LIST_ANSWER, equip_list_answer__pack, resp);

	return 0;
}

//装备升星请求
static int handle_equip_star_up_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	EquipStarUpRequest *req = equip_star_up_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t type = req->type;
	uint32_t item_num = req->itemnum;
	equip_star_up_request__free_unpacked(req, NULL);

	int ret = 0;
	EquipInfo *equip_info = NULL;
	do
	{
		equip_info = player->get_equip(type);
		if (!equip_info || equip_info->stair == 0)
		{
			ret = ERROR_ID_EQUIP_NOT_ACTIVE;
			LOG_ERR("[%s:%d] player[%lu] get equip failed, type:%u", __FUNCTION__, __LINE__, extern_data->player_id, type);
			break;
		}

		uint32_t player_job = player->get_attr(PLAYER_ATTR_JOB);
		EquipStarLv *config = get_equip_star_config(player_job, type, equip_info->stair, equip_info->star_lv + 1);
		if (!config)
		{
			ret = ERROR_ID_EQUIP_STAR_MAX;
			LOG_ERR("[%s:%d] player[%lu] get star config failed, job:%u, type:%u, stair:%u, star:%u", __FUNCTION__, __LINE__, extern_data->player_id, player_job, type, equip_info->stair, equip_info->star_lv + 1);
			break;
		}

		uint32_t bag_num = player->get_item_can_use_num(config->ConsumeItem);
		if (bag_num < item_num || item_num == 0)
		{
			ret = ERROR_ID_PROP_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] item not enough, item_id:%u, item_num:%u, bag_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, (uint32_t)config->ConsumeItem, item_num, bag_num);
			break;
		}

		uint32_t player_coin = player->get_attr(PLAYER_ATTR_COIN);
		uint32_t need_coin = config->ConsumeCoin * item_num;
		if (player_coin < need_coin)
		{
			ret = ERROR_ID_COIN_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] coin not enough, player_coin:%u, need_coin:%u", __FUNCTION__, __LINE__, extern_data->player_id, player_coin, need_coin);
			break;
		}

		uint32_t player_lv = player->get_attr(PLAYER_ATTR_LEVEL);
		uint32_t need_lv = (uint32_t)config->Level;
		if (player_lv < need_lv)
		{
			ret = ERROR_ID_LEVEL_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] level not enough, player_lv:%u, need_lv:%u", __FUNCTION__, __LINE__, extern_data->player_id, player_lv, need_lv);
			break;
		}

		ItemsConfigTable *item_config = get_config_by_id(config->ConsumeItem, &::item_config);
		if (!item_config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get item config failed, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, (uint32_t)config->ConsumeItem);
			break;
		}

		if (item_config->n_ParameterEffect == 0)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get item config failed, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, (uint32_t)config->ConsumeItem);
			break;
		}

		uint32_t max_exp = player->get_equip_max_star_need_exp(type);
		uint32_t item_exp = item_config->ParameterEffect[0];
		uint32_t use_num = item_num;
		if (item_exp * item_num > max_exp)
		{
			use_num = ceil((double)max_exp / (double)item_exp);
		}
		player->add_equip_exp(type, item_exp * use_num);

		player->sub_coin(use_num * config->ConsumeCoin, MAGIC_TYPE_EQUIP_STAR_UP);
		player->del_item(config->ConsumeItem, use_num, MAGIC_TYPE_EQUIP_STAR_UP);
	} while(0);

	EquipStarUpAnswer resp;
	equip_star_up_answer__init(&resp);

	resp.result = ret;
	if (equip_info)
	{
		resp.type = type;
		resp.starlv = equip_info->star_lv;
		resp.starexp = equip_info->star_exp;
	}
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_EQUIP_STAR_UP_ANSWER, equip_star_up_answer__pack, resp);

	return 0;
}

//装备进阶请求
static int handle_equip_stair_up_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	int ret = 0;
	do
	{
		bool all_star_max = true;
		bool all_has_next_stair = true;
		uint32_t player_job = player->get_attr(PLAYER_ATTR_JOB);
		uint32_t need_lv = 0;
		for (int i = 0; i < MAX_EQUIP_NUM; ++i)
		{
			EquipInfo &equip_info = player->data->equip_list[i];
			uint32_t type = i + 1;
			if (equip_info.stair == 0)
			{
				all_star_max = false;
				break;
			}

			if (get_equip_star_config(player_job, type, equip_info.stair, equip_info.star_lv + 1))
			{
				all_star_max = false;
				break;
			}

			EquipStarLv *config = get_equip_star_config(player_job, type, equip_info.stair + 1, 0);
			if (!config)
			{
				all_has_next_stair = false;
				break;
			}

			need_lv = std::max(need_lv, (uint32_t)config->Level);
		}

		if (!all_star_max)
		{
			ret = ERROR_ID_EQUIP_NOT_ALL_STAR_MAX;
			LOG_ERR("[%s:%d] player[%lu] not all equip star max", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (!all_has_next_stair)
		{
			ret = ERROR_ID_EQUIP_STAIR_MAX;
			LOG_ERR("[%s:%d] player[%lu] equip stair max", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		uint32_t player_lv = player->get_attr(PLAYER_ATTR_LEVEL);
		if (player_lv < need_lv)
		{
			ret = ERROR_ID_LEVEL_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] level not enough, player_lv:%u, need_lv:%u", __FUNCTION__, __LINE__, extern_data->player_id, player_lv, need_lv);
			break;
		}

		for (int i = 0; i < MAX_EQUIP_NUM; ++i)
		{
			EquipInfo &equip_info = player->data->equip_list[i];
			equip_info.stair++;
			equip_info.star_lv = 0;
			equip_info.star_exp = 0;
			player->add_task_progress(TCT_EQUIP_STAR, i, equip_info.stair * 10 + equip_info.star_lv);
		}

		//刷新武器外形
		player->update_weapon_skin(true);
		//刷新属性
		player->calculate_attribute(true);
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_EQUIP_STAIR_UP_ANSWER, comm_answer__pack, resp);

	return 0;
}

//装备附魔请求
static int handle_equip_enchant_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	EquipEnchantRequest *req = equip_enchant_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t type = req->type;
	uint32_t index = req->index - 1;
	uint32_t skip = req->skipretain;

	equip_enchant_request__free_unpacked(req, NULL);

	int ret = 0;
	EquipInfo *equip_info = NULL;
	do
	{
		equip_info = player->get_equip(type);
		if (!equip_info || equip_info->stair == 0)
		{
			ret = ERROR_ID_EQUIP_NOT_ACTIVE;
			LOG_ERR("[%s:%d] player[%lu] get equip failed, type:%u", __FUNCTION__, __LINE__, extern_data->player_id, type);
			break;
		}

		if (index >= MAX_EQUIP_ENCHANT_NUM)
		{
			ret = ERROR_ID_EQUIP_ENCHANT_INDEX;
			LOG_ERR("[%s:%d] player[%lu] enchant index error, type:%u, index:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, index);
			break;
		}

		EquipEnchantInfo *enchant_info = &equip_info->enchant[index];
		if (skip == 0 && enchant_info->rand_attr[0].id > 0)
		{
			ret = ERROR_ID_EQUIP_ENCHANT_NO_RETAIN;
			LOG_ERR("[%s:%d] player[%lu] enchant not retain yet, type:%u, index:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, index);
			break;
		}

		uint32_t player_job = player->get_job();
		EquipLock *lock_config = get_equip_lock_config(player_job, type);
		if (!lock_config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get lock config failed, type:%u, job:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, player_job);
			break;
		}

		uint32_t need_stair = lock_config->EnchantQualityLock[index];
		uint32_t need_star = lock_config->EnchantStarLock[index];
		if (equip_info->stair < need_stair || (equip_info->stair == need_stair && equip_info->star_lv < need_star))
		{
			ret = ERROR_ID_EQUIP_STAR_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] star not enough, type:%u, index:%u, need_stair:%u, need_star:%u, cur_stair:%u, cur_star:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, index, need_stair, need_star, equip_info->stair, equip_info->star_lv);
			break;
		}

		uint32_t need_item_id = lock_config->EnchantItem[index];
		uint32_t need_item_num = lock_config->EnchantItemNum[index];
		uint32_t need_coin = lock_config->EnchantCoin[index];
		uint32_t bag_item_num = player->get_item_can_use_num(need_item_id);
		if (bag_item_num < need_item_num)
		{
			ret = ERROR_ID_PROP_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] item not enough, item_id:%u, need_num:%u, bag_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, need_item_id, need_item_num, bag_item_num);
			break;
		}

		uint32_t player_coin = player->get_coin();
		if (player_coin < need_coin)
		{
			ret = ERROR_ID_COIN_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] coin not enough, need_num:%u, has_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, need_coin, player_coin);
			break;
		}

		uint32_t n_type;
		uint64_t *enchant_type;
		switch(index)
		{
			case 0:
				{
					n_type = lock_config->n_Enchant1Type;
					enchant_type = lock_config->Enchant1Type;
				}
				break;
			case 1:
				{
					n_type = lock_config->n_Enchant2Type;
					enchant_type = lock_config->Enchant2Type;
				}
				break;
			case 2:
				{
					n_type = lock_config->n_Enchant3Type;
					enchant_type = lock_config->Enchant3Type;
				}
				break;
		}

		EquipStarLv *star_config = get_equip_star_config(player_job, type, equip_info->stair, equip_info->star_lv);
		if (!star_config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get star config failed, type:%u, job:%u, stair:%u, star:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, player_job, equip_info->stair, equip_info->star_lv);
			break;
		}

		uint32_t attr_quality = star_config->EnchantQuality - 1;
		int rand_idx = 0;
		if (enchant_info->cur_attr.id > 0)
		{
			enchant_info->rand_attr[rand_idx].id = enchant_info->cur_attr.id;
			enchant_info->rand_attr[rand_idx].val = enchant_info->cur_attr.val;
			rand_idx++;
		}
		for (; rand_idx < MAX_EQUIP_ENCHANT_RAND_NUM; ++rand_idx)
		{
			uint32_t rand_val = rand() % n_type;
			uint32_t attr_type = enchant_type[rand_val];
			EquipAttribute *attr_config = get_config_by_id(attr_type, &equip_attr_config);
			if (!attr_config)
			{
				ret = ERROR_ID_NO_CONFIG;
				LOG_ERR("[%s:%d] player[%lu] get attr config failed, attr_type:%u", __FUNCTION__, __LINE__, extern_data->player_id, attr_type);
				break;
			}

			if (attr_config->n_Rand <= attr_quality || attr_config->Rand[attr_quality] == 0)
			{
				ret = ERROR_ID_NO_CONFIG;
				break;
			}

			double attr_val = 0;
			if (attr_config->Rand[attr_quality] > 1.00)
			{
				rand_val = rand() % (uint32_t)attr_config->Rand[attr_quality];
				attr_val = rand_val + 1;
			}
			else //随机0-1的小数
			{
				attr_val = (1 + rand() % (uint32_t)(attr_config->Rand[attr_quality] * 10000)) / 10000.0;
			}
			enchant_info->rand_attr[rand_idx].id = attr_type;
			enchant_info->rand_attr[rand_idx].val = attr_val;
		}

		if (ret != 0)
		{
			memset(enchant_info->rand_attr, 0, sizeof(AttrInfo) * MAX_EQUIP_ENCHANT_RAND_NUM);
			break;
		}

		player->del_item(need_item_id, need_item_num, MAGIC_TYPE_EQUIP_ENCHANT);
		player->sub_coin(need_coin, MAGIC_TYPE_EQUIP_ENCHANT);
		player->add_task_progress(TCT_EQUIP_ENCHANT, type, 1);
	} while(0);

	EquipEnchantAnswer resp;
	equip_enchant_answer__init(&resp);

	AttrData rand_data[MAX_EQUIP_ENCHANT_RAND_NUM];
	AttrData* rand_data_point[MAX_EQUIP_ENCHANT_RAND_NUM];

	resp.result = ret;
	resp.type = type;
	resp.index = index + 1;
	if (ret == 0 && equip_info != NULL)
	{
		for (int i = 0; i < MAX_EQUIP_ENCHANT_RAND_NUM; ++i)
		{
			rand_data_point[i] = &rand_data[i];
			attr_data__init(&rand_data[i]);
			rand_data[i].id = equip_info->enchant[index].rand_attr[i].id;
			rand_data[i].val = equip_info->enchant[index].rand_attr[i].val;
		}
		resp.randattr = rand_data_point;
		resp.n_randattr = MAX_EQUIP_ENCHANT_RAND_NUM;
	}
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_EQUIP_ENCHANT_ANSWER, equip_enchant_answer__pack, resp);

	return 0;
}

//装备附魔保留请求
static int handle_equip_enchant_retain_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	EquipEnchantRetainRequest *req = equip_enchant_retain_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t type = req->type;
	uint32_t attr_index = req->attrindex - 1;
	uint32_t retain_index = req->retainindex - 1;

	equip_enchant_retain_request__free_unpacked(req, NULL);

	int ret = 0;
	EquipInfo *equip_info = NULL;
	AttrData retain_attr;
	attr_data__init(&retain_attr);
	do
	{
		equip_info = player->get_equip(type);
		if (!equip_info || equip_info->stair == 0)
		{
			ret = ERROR_ID_EQUIP_NOT_ACTIVE;
			LOG_ERR("[%s:%d] player[%lu] get equip failed, type:%u", __FUNCTION__, __LINE__, extern_data->player_id, type);
			break;
		}

		if (attr_index >= MAX_EQUIP_ENCHANT_NUM)
		{
			ret = ERROR_ID_EQUIP_ENCHANT_INDEX;
			LOG_ERR("[%s:%d] player[%lu] enchant index error, type:%u, index:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, attr_index);
			break;
		}

		EquipEnchantInfo *enchant_info = &equip_info->enchant[attr_index];
		if (enchant_info->rand_attr[0].id == 0)
		{
			ret = ERROR_ID_EQUIP_ENCHANT_NO_IN_RETAIN;
			LOG_ERR("[%s:%d] player[%lu] enchant not in retain, type:%u, index:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, attr_index);
			break;
		}

		if (retain_index >= MAX_EQUIP_ENCHANT_RAND_NUM)
		{
			ret = ERROR_ID_EQUIP_ENCHANT_RETAIN_INDEX;
			LOG_ERR("[%s:%d] player[%lu] enchant retain index error, type:%u, attr_index:%u, retain_index:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, attr_index, retain_index);
			break;
		}

		if (!(retain_index == 0 && enchant_info->cur_attr.id > 0))
		{
			enchant_info->cur_attr.id = enchant_info->rand_attr[retain_index].id;
			enchant_info->cur_attr.val = enchant_info->rand_attr[retain_index].val;
			player->calculate_attribute(true);
		}

		memset(enchant_info->rand_attr, 0, sizeof(AttrInfo) * MAX_EQUIP_ENCHANT_RAND_NUM);
		retain_attr.id = enchant_info->cur_attr.id;
		retain_attr.val = enchant_info->cur_attr.val;
	} while(0);

	EquipEnchantRetainAnswer resp;
	equip_enchant_retain_answer__init(&resp);

	resp.result = ret;
	resp.type = type;
	resp.attrindex = attr_index + 1;
	resp.retainattr = &retain_attr;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_EQUIP_ENCHANT_RETAIN_ANSWER, equip_enchant_retain_answer__pack, resp);

	return 0;
}

//装备解锁孔位请求
static int handle_equip_drill_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	EquipDrillRequest *req = equip_drill_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t type = req->type;
	uint32_t index = req->index - 1;

	equip_drill_request__free_unpacked(req, NULL);

	int ret = 0;
	EquipInfo *equip_info = NULL;
	do
	{
		equip_info = player->get_equip(type);
		if (!equip_info || equip_info->stair == 0)
		{
			ret = ERROR_ID_EQUIP_NOT_ACTIVE;
			LOG_ERR("[%s:%d] player[%lu] get equip failed, type:%u", __FUNCTION__, __LINE__, extern_data->player_id, type);
			break;
		}

		if (index >= MAX_EQUIP_INLAY_NUM)
		{
			ret = ERROR_ID_EQUIP_HOLE_INDEX;
			LOG_ERR("[%s:%d] player[%lu] hole index error, type:%u, index:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, index);
			break;
		}

		if (equip_info->inlay[index] != -1)
		{
			ret = ERROR_ID_EQUIP_HOLE_OPENED;
			LOG_ERR("[%s:%d] player[%lu] hole opened, type:%u, index:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, index);
			break;
		}

		uint32_t player_job = player->get_job();
		EquipLock *config = get_equip_lock_config(player_job, type);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get lock config failed, type:%u, job:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, player_job);
			break;
		}

		uint32_t player_lv = player->get_attr(PLAYER_ATTR_LEVEL);
		uint32_t need_lv = config->LockLv[index];
		if (player_lv < need_lv)
		{
			ret = ERROR_ID_LEVEL_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] level not enough, type:%u, index:%u, need_lv:%u, player_lv:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, index, need_lv, player_lv);
			break;
		}

		uint32_t need_stair = config->LockQuality[index];
		uint32_t need_star = config->LockStar[index];
		if (equip_info->stair < need_stair || (equip_info->stair == need_stair && equip_info->star_lv < need_star))
		{
			ret = ERROR_ID_EQUIP_STAR_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] star not enough, type:%u, index:%u, need_stair:%u, need_star:%u, cur_stair:%u, cur_star:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, index, need_stair, need_star, equip_info->stair, equip_info->star_lv);
			break;
		}

		uint32_t need_item_id = config->LockItem[index];
		uint32_t need_item_num = config->LockItemNum[index];
		uint32_t bag_item_num = player->get_item_can_use_num(need_item_id);
		if (bag_item_num < need_item_num)
		{
			ret = ERROR_ID_PROP_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] item not enough, type:%u, index:%u, item_id:%u, need_num:%u, bag_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, index, need_item_id, need_item_num, bag_item_num);
			break;
		}

		equip_info->inlay[index] = 0;
		player->del_item(need_item_id, need_item_num, MAGIC_TYPE_EQUIP_DRILL);
	} while(0);

	EquipDrillAnswer resp;
	equip_drill_answer__init(&resp);

	resp.result = ret;
	resp.type = type;
	resp.index = index + 1;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_EQUIP_DRILL_ANSWER, equip_drill_answer__pack, resp);

	return 0;
}

//装备镶嵌请求
static int handle_equip_inlay_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	EquipInlayRequest *req = equip_inlay_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t type = req->type;
	uint32_t index = req->index - 1;
	uint32_t gem_id = req->gemid;

	equip_inlay_request__free_unpacked(req, NULL);

	int ret = 0;
	EquipInfo *equip_info = NULL;
	uint32_t inlay_gem = gem_id;
	do
	{
		equip_info = player->get_equip(type);
		if (!equip_info || equip_info->stair == 0)
		{
			ret = ERROR_ID_EQUIP_NOT_ACTIVE;
			LOG_ERR("[%s:%d] player[%lu] get equip failed, type:%u", __FUNCTION__, __LINE__, extern_data->player_id, type);
			break;
		}

		if (index >= MAX_EQUIP_INLAY_NUM)
		{
			ret = ERROR_ID_EQUIP_HOLE_INDEX;
			LOG_ERR("[%s:%d] player[%lu] hole index error, type:%u, index:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, index);
			break;
		}

		if (equip_info->inlay[index] == -1)
		{
			ret = ERROR_ID_EQUIP_HOLE_NOT_OPEN;
			LOG_ERR("[%s:%d] player[%lu] hole not open, type:%u, index:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, index);
			break;
		}

		uint32_t player_job = player->get_job();
		EquipLock *lock_config = get_equip_lock_config(player_job, type);
		if (!lock_config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get lock config failed, type:%u, job:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, player_job);
			break;
		}

		GemAttribute *gem_config = get_gem_config(gem_id);
		if (!gem_config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get gem config failed, gem_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, gem_id);
			break;
		}

//		bool bTypeFit = false;
//		for (uint32_t i = 0; i < lock_config->n_MosaicType; ++i)
//		{
//			if (lock_config->MosaicType[i] == gem_config->GemType)
//			{
//				bTypeFit = true;
//				break;
//			}
//		}

		if (gem_config->GemType != lock_config->MosaicType[index])
		{
			ret = ERROR_ID_EQUIP_GEM_TYPE_NOT_FIT;
			LOG_ERR("[%s:%d] player[%lu] gem type error, type:%u, index:%u, gem_id:%u, gem_type:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, index, gem_id, (uint32_t)gem_config->GemType);
			break;
		}

		uint32_t need_gem_num = 1;
		uint32_t bag_gem_num = player->get_item_num_by_id(gem_id);
		if (bag_gem_num < need_gem_num)
		{
			ret = ERROR_ID_PROP_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] gem not enough, type:%u, index:%u, gem_id:%u, need_num:%u, bag_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, index, gem_id, need_gem_num, bag_gem_num);
			break;
		}

		//剥离消耗
		uint32_t need_coin = 0;
		if (equip_info->inlay[index] > 0)
		{
			need_coin = sg_gem_strip_coin;
			uint32_t player_coin = player->get_coin();
			if (player_coin < need_coin)
			{
				ret = ERROR_ID_COIN_NOT_ENOUGH;
				LOG_ERR("[%s:%d] player[%lu] coin not enough, type:%u, index:%u, need_num:%u, has_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, index, need_coin, player_coin);
				break;
			}

			if (!player->check_can_add_item(equip_info->inlay[index], 1, NULL))
			{
				ret = ERROR_ID_BAG_GRID_NOT_ENOUGH;
				LOG_ERR("[%s:%d] player[%lu] bag not enough, type:%u, index:%u, item_id:%u, item_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, index, equip_info->inlay[index], 1);
				break;
			}
		}

		//非绑定宝石变绑定
		if (!item_is_bind(gem_id))
		{
			inlay_gem = get_item_relate_id(gem_id);
			if (inlay_gem == 0)
			{
				ret = ERROR_ID_EQUIP_GEM_NO_BIND_ID;
				LOG_ERR("[%s:%d] player[%lu] can't found bind id, type:%u, index:%u, gem_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, index, gem_id);
				break;
			}
		}

		//先剥离
		if (equip_info->inlay[index] > 0)
		{
			player->add_item(equip_info->inlay[index], 1, MAGIC_TYPE_EQUIP_STRIP);
			player->sub_coin(need_coin, MAGIC_TYPE_EQUIP_STRIP);
			equip_info->inlay[index] = 0;
		}

		//后镶嵌
		player->del_item_by_id(gem_id, need_gem_num, MAGIC_TYPE_EQUIP_INLAY);
		equip_info->inlay[index] = inlay_gem;
		player->calculate_attribute(true);
		player->add_task_progress(TCT_EQUIP_INLAY, type, 1);
	} while(0);

	EquipInlayAnswer resp;
	equip_inlay_answer__init(&resp);

	resp.result = ret;
	resp.type = type;
	resp.index = index + 1;
	resp.gemid = inlay_gem;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_EQUIP_INLAY_ANSWER, equip_inlay_answer__pack, resp);

	return 0;
}

//装备剥离请求
static int handle_equip_strip_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	EquipStripRequest *req = equip_strip_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t type = req->type;
	uint32_t index = req->index - 1;

	equip_strip_request__free_unpacked(req, NULL);

	int ret = 0;
	EquipInfo *equip_info = NULL;
	do
	{
		equip_info = player->get_equip(type);
		if (!equip_info || equip_info->stair == 0)
		{
			ret = ERROR_ID_EQUIP_NOT_ACTIVE;
			LOG_ERR("[%s:%d] player[%lu] get equip failed, type:%u", __FUNCTION__, __LINE__, extern_data->player_id, type);
			break;
		}

		if (index >= MAX_EQUIP_INLAY_NUM)
		{
			ret = ERROR_ID_EQUIP_HOLE_INDEX;
			LOG_ERR("[%s:%d] player[%lu] hole index error, type:%u, index:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, index);
			break;
		}

		if (equip_info->inlay[index] <= 0)
		{
			ret = ERROR_ID_EQUIP_HOLE_NO_GEM;
			LOG_ERR("[%s:%d] player[%lu] hole has not gem, type:%u, index:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, index);
			break;
		}

		//剥离消耗
		uint32_t need_coin = sg_gem_strip_coin;
		uint32_t player_coin = player->get_coin();
		if (player_coin < need_coin)
		{
			ret = ERROR_ID_COIN_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] coin not enough, type:%u, index:%u, need_num:%u, has_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, index, need_coin, player_coin);
			break;
		}

		if (!player->check_can_add_item(equip_info->inlay[index], 1, NULL))
		{
			ret = ERROR_ID_BAG_GRID_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] bag not enough, type:%u, index:%u, item_id:%u, item_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, index, equip_info->inlay[index], 1);
			break;
		}

		player->add_item(equip_info->inlay[index], 1, MAGIC_TYPE_EQUIP_STRIP);
		player->sub_coin(need_coin, MAGIC_TYPE_EQUIP_STRIP);
		equip_info->inlay[index] = 0;
		player->calculate_attribute(true);
	} while(0);

	EquipStripAnswer resp;
	equip_strip_answer__init(&resp);

	resp.result = ret;
	resp.type = type;
	resp.index = index + 1;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_EQUIP_STRIP_ANSWER, equip_strip_answer__pack, resp);

	return 0;
}

//宝石合成请求
static int handle_equip_gem_compose_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	EquipGemComposeRequest *req = equip_gem_compose_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t gem_id = req->gemid;
	uint32_t material_type = req->materialtype;

	equip_gem_compose_request__free_unpacked(req, NULL);

	int ret = 0;
	uint32_t product_id = 0;
	do
	{
		GemAttribute *gem_config = get_gem_config(gem_id);
		if (!gem_config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get gem config failed, gem_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, gem_id);
			break;
		}

		uint32_t material_id = gem_config->GemSynthetic;
		if (material_id == 0)
		{
			ret = ERROR_ID_GEM_CAN_NOT_COMPOSE;
			LOG_ERR("[%s:%d] player[%lu] gem can't compose, gem_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, gem_id);
			break;
		}

		uint32_t gem_bind = 0, gem_unbind = 0;
		uint32_t material_bind = 0, material_unbind = 0;
		if (item_is_bind(gem_id))
		{
			gem_bind = gem_id;
			gem_unbind = get_item_relate_id(gem_id);
		}
		else
		{
			gem_bind = get_item_relate_id(gem_id);
			gem_unbind = gem_id;
		}

		if (item_is_bind(material_id))
		{
			material_bind = material_id;
			material_unbind = get_item_relate_id(material_id);
		}
		else
		{
			material_bind = get_item_relate_id(material_id);
			material_unbind = material_id;
		}

		if (gem_bind == 0 || gem_unbind == 0 || material_bind == 0 || material_unbind == 0)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu], gem_bind:%u, gem_unbind:%u, material_bind:%u, material_unbind:%u", __FUNCTION__, __LINE__, extern_data->player_id, gem_bind, gem_unbind, material_bind, material_unbind);
			break;
		}

		uint32_t need_material_num = gem_config->Number;
		std::map<uint32_t, uint32_t> material_map;
		uint32_t material_bind_num = player->get_item_num_by_id(material_bind);
		uint32_t material_unbind_num = player->get_item_num_by_id(material_unbind);
		switch(material_type)
		{
			case 1:
				{
					if (material_bind_num < need_material_num)
					{
						ret = ERROR_ID_GEM_COMPOSE_MATERIAL_NOT_ENOUGH;
						LOG_ERR("[%s:%d] player[%lu] need_id:%u, need_num:%u, has_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, material_bind, need_material_num, material_bind_num);
						break;
					}

					product_id = gem_bind;
					material_map[material_bind] = need_material_num;
				}
				break;
			case 2:
				{
					if (material_unbind_num < need_material_num)
					{
						ret = ERROR_ID_GEM_COMPOSE_MATERIAL_NOT_ENOUGH;
						LOG_ERR("[%s:%d] player[%lu] need_id:%u, need_num:%u, has_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, material_unbind, need_material_num, material_unbind_num);
						break;
					}

					product_id = gem_unbind;
					material_map[material_unbind] = need_material_num;
				}
				break;
			case 3:
				{
					if (material_bind_num + material_unbind_num < need_material_num)
					{
						ret = ERROR_ID_GEM_COMPOSE_MATERIAL_NOT_ENOUGH;
						LOG_ERR("[%s:%d] player[%lu] need_num:%u, bind_num:%u, unbind_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, need_material_num, material_bind_num, material_unbind_num);
						break;
					}

					if (material_bind_num > 0)
					{
						product_id = gem_bind;
						if (material_bind_num < need_material_num)
						{
							material_map[material_bind] = material_bind_num;
							material_map[material_unbind] = need_material_num - material_bind_num;
						}
						else
						{
							material_map[material_bind] = need_material_num;
						}
					}
					else
					{
						product_id = gem_unbind;
						material_map[material_unbind] = need_material_num;
					}
				}
				break;
		}

		if (product_id == 0)
		{
			if (ret == 0)
			{
				ret = ERROR_ID_GEM_COMPOSE_MATERIAL_TYPE;
			}
			break;
		}

		uint32_t product_num = 1;
		if (!player->check_can_add_item(product_id, product_num, NULL))
		{
			ret = ERROR_ID_BAG_GRID_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] bag not enough, item_id:%u, item_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, product_id, product_num);
			break;
		}

		uint32_t need_coin = gem_config->Consumption;
		uint32_t player_coin = player->get_attr(PLAYER_ATTR_COIN);
		if (player_coin < need_coin)
		{
			ret = ERROR_ID_COIN_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] coin not enough, need_coin:%u, has_coin:%u", __FUNCTION__, __LINE__, extern_data->player_id, need_coin, player_coin);
			break;
		}

		player->sub_coin(need_coin, MAGIC_TYPE_GEM_COMPOSE);
		for (std::map<uint32_t, uint32_t>::iterator iter = material_map.begin(); iter != material_map.end(); ++iter)
		{
			player->del_item_by_id(iter->first, iter->second, MAGIC_TYPE_GEM_COMPOSE);
		}

		player->add_item(product_id, product_num, MAGIC_TYPE_GEM_COMPOSE);
		player->add_task_progress(TCT_GEM_COMPOSE, 0, 1);
	} while(0);

	EquipGemComposeAnswer resp;
	equip_gem_compose_answer__init(&resp);

	resp.result = ret;
	resp.gemid = product_id;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_EQUIP_GEM_COMPOSE_ANSWER, equip_gem_compose_answer__pack, resp);

	return 0;
}

//邮件附件发放请求
static int handle_mail_give_attach_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	MailGiveAttachRequest *req = mail_give_attach_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	std::vector<uint64_t> success_ids;
	std::vector<uint64_t> fail_ids;
	for (size_t i = 0; i < req->n_mails; ++i)
	{
		MailAttachGiveInfo *give_info = req->mails[i];
		std::map<uint32_t, uint32_t> attach_map;
		for (size_t j = 0; j < give_info->n_attachs; ++j)
		{
			attach_map[give_info->attachs[j]->id] += give_info->attachs[j]->num;
		}

		if (player->add_item_list(attach_map, give_info->statisid, true))
		{
			success_ids.push_back(give_info->mailid);
		}
		else
		{
			fail_ids.push_back(give_info->mailid);
		}
	}

	mail_give_attach_request__free_unpacked(req, NULL);

	MailGiveAttachAnswer resp;
	mail_give_attach_answer__init(&resp);

	resp.successids = &success_ids[0];
	resp.n_successids = success_ids.size();
	resp.failids = &fail_ids[0];
	resp.n_failids = fail_ids.size();

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, SERVER_PROTO_MAIL_GIVE_ATTACH_ANSWER, mail_give_attach_answer__pack, resp);

	return 0;
}

static int handle_buy_fashion_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	BuyFashion *req = buy_fashion__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	int shopid = req->id;
	int ind = req->during;
	buy_fashion__free_unpacked(req, NULL);

	std::map<uint64_t, struct ActorFashionTable *>::iterator it = fashion_config.find(shopid);
	if (it == fashion_config.end())
	{
		return 3;
	}
	if (ind < 0 || ind > 2)
	{
		return 4;
	}

	BuyFashionAnswer send;
	buy_fashion_answer__init(&send);
	FashionData data;
	send.data = &data;
	send.ret = 0;
	fashion_data__init(&data);
	if (it->second->UnlockType == 1)
	{
		if (player->get_comm_gold() < (int)it->second->WingBinding[ind])
		{
			send.ret = 190400005;
			fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_BUY_FASHION_ANSWER, buy_fashion_answer__pack, send);
			return 5;
		}
	}
	else if (it->second->UnlockType == 2)
	{
		if (player->del_item(it->second->UnlockEffect1, it->second->UnlockEffect2, MAGIC_TYPE_FASHION) < 0)
		{
			return 7;
		}
	}
	else
	{
		return 6;
	}
	
	int ret = player->add_fashion(shopid, it->second->Colour, it->second->Time[ind]);
	if (ret >= 0)
	{
		pack_fashion_info(data, player, ret);
		player->sub_comm_gold(it->second->WingBinding[ind], MAGIC_TYPE_FASHION);
	}
	else
	{
		send.ret = 190300008; //时装包裹满了
	}

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_BUY_FASHION_ANSWER, buy_fashion_answer__pack, send);

	return 0;
}
static int handle_unlock_color_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	//UnlockColor *req = unlock_color__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	//if (!req)
	//{
	//	LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
	//	return (-10);
	//}

	//unlock_color__free_unpacked(req, NULL);
	std::vector<std::map<uint64_t, struct ColourTable*>::iterator> vcolor;
	int color = 0;
	std::map<uint64_t, struct ColourTable*>::iterator it = color_table_config.begin();
	for (;it != color_table_config.end(); ++it)
	{
		if (player->get_color(it->first) < 0)
		{
			vcolor.push_back(it);
		}
	}
	UnlockColorAns send;
	unlock_color_ans__init(&send);
	if (vcolor.empty())
	{
		send.ret = 190500087;
		goto done;
	}
	color = vcolor[rand()%vcolor.size()]->first;

	send.ret = 0;
	if (player->del_item(vcolor[rand() % vcolor.size()]->second->OpenColourItem, vcolor[rand() % vcolor.size()]->second->OpenColourNum, MAGIC_TYPE_FASHION) < 0)
	{
		send.ret = 190500063;
		goto done;
	}

	player->unlock_color(color);

done:
	send.color = color;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_UNLOCK_COLOR_ANSWER, unlock_color_ans__pack, send);


	return 0;
}
static int handle_unlock_weapon_color_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	UnlockColor *req = unlock_color__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint32_t color = req->color;
	unlock_color__free_unpacked(req, NULL);

	WeaponsEffectTable *table = get_config_by_id(color, &weapon_color_config);
	if (table == NULL)
	{
		return 2;
	}
	EquipInfo *equ = player->get_equip(ET_WEAPON);
	if (equ == NULL)
	{
		return 3;
	}
	EquipStarLv *tableEqu = get_config_by_id(table->Requirement2, &equip_star_config);
	if (tableEqu == NULL)
	{
		return 4;
	}

	UnlockColorAns send;
	unlock_color_ans__init(&send);
	send.ret = 0;
	if (player->get_attr(PLAYER_ATTR_LEVEL) < table->Requirement1)
	{
		send.ret = 190500064;
		goto done;
	}

	if (equ->stair < tableEqu->Quality || equ->star_lv < tableEqu->StarLv)
	{
		send.ret = 190500063;
		goto done;
	}
	
	player->unlock_weapon_color(color);

done:
	send.color = color;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_UNLOCK_WEAPON_COLOR_ANSWER, unlock_color_ans__pack, send);

	

	return 0;
}
static int handle_set_fashion_color_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	SetFashionColor *req = set_fashion_color__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint32_t fashoinid = req->id;
	uint32_t color = req->color;
	bool isdown = req->isdown;
	set_fashion_color__free_unpacked(req, NULL);

	if (player->get_color(color) < 0)
	{
		return 1;
	}

	SetFashionColorAnswer send;
	set_fashion_color_answer__init(&send);
	send.ret = 0;
	std::map<uint64_t, struct ColourTable*>::iterator it = color_table_config.find(color);
	AttrMap attrs;
	std::map<uint64_t, struct ActorFashionTable *>::iterator itFashion = fashion_config.find(fashoinid);
	ParameterTable *param_config = get_config_by_id(161000025, &parameter_config);
	int i = player->get_fashion(fashoinid);
	if (i < 0)
	{
		send.ret = 190500088; //过期
		goto done;
	}
	if (player->data->fashion[i].timeout != 0
		&& player->data->fashion[i].timeout <= (time_t)time_helper::get_cached_time() / 1000)
	{
		send.ret = 190500088; //过期
		goto done;
	}

	if (itFashion == fashion_config.end())
	{
		return 4;
	}
	if (itFashion->second->Type == FASHION_TYPE_WEAPON)
	{
		return 5;
	}

	if (it == color_table_config.end())
	{
		return 6;
	}


	if (param_config == NULL)
	{
		return 8;
	}
	if (player->del_item(param_config->parameter1[0], param_config->parameter1[1], MAGIC_TYPE_FASHION) < 0)
	{
		return 7;
	}

	//if (player->del_item(it->second->ColourItem, it->second->ColourNum, MAGIC_TYPE_FASHION) < 0)
	//{
	//	return 7;
	//}


	if (isdown)
	{
		player->data->fashion[i].colordown = color;
		player->set_attr(PLAYER_ATTR_CLOTHES_COLOR_DOWN, player->data->fashion[i].colordown);
		attrs[PLAYER_ATTR_CLOTHES_COLOR_DOWN] = player->data->fashion[i].colordown;
	}
	else
	{
		player->data->fashion[i].color = color;
		if (itFashion->second->Type == FASHION_TYPE_CLOTHES)
		{
			player->set_attr(PLAYER_ATTR_CLOTHES_COLOR_UP, player->data->fashion[i].color);
			attrs[PLAYER_ATTR_CLOTHES_COLOR_UP] = player->data->fashion[i].color;
		}
		else
		{
			player->set_attr(PLAYER_ATTR_HAT_COLOR, player->data->fashion[i].color);
			attrs[PLAYER_ATTR_HAT_COLOR] = player->data->fashion[i].color;;
		}
	}
	player->notify_attr(attrs, true, true);
done:
	send.id = fashoinid;
	send.color = color;
	send.isdown = isdown;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_SET_FASHION_COLOR_ANSWER, set_fashion_color_answer__pack, send);

	return 0;
}
static int handle_set_weapon_color_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	SetFashionColor *req = set_fashion_color__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint32_t fashoinid = req->id;
	uint32_t color = req->color;
	set_fashion_color__free_unpacked(req, NULL);

	if (player->get_weapon_color(color) < 0)
	{
		return 1;
	}
	WeaponsEffectTable *table = get_config_by_id(color, &weapon_color_config);
	if (table == NULL)
	{
		return 2;
	}
	
	SetFashionColorAnswer send;
	set_fashion_color_answer__init(&send);
	send.ret = 0;
	std::map<uint64_t, struct ActorFashionTable *>::iterator itFashion = fashion_config.find(fashoinid);
	std::map<uint64_t, struct ColourTable*>::iterator it = color_table_config.find(color);
	AttrMap attrs;
	int i = player->get_fashion(fashoinid);
	if (i < 0)
	{
		send.ret = 190500088; //过期
		goto done;
	}
	if (player->data->fashion[i].timeout != 0
		&& player->data->fashion[i].timeout <= (time_t)time_helper::get_cached_time() / 1000)
	{
		send.ret = 190500088; //过期
		goto done;
	}

	if (itFashion->second->Type != FASHION_TYPE_WEAPON)
	{
		return 5;
	}

	if (player->del_item(table->Item, table->ItemNum, MAGIC_TYPE_FASHION) < 0)
	{
		send.ret = 190400006;
		goto done;
	}

	player->data->fashion[i].color = color;
	player->set_attr(PLAYER_ATTR_WEAPON_COLOR, player->data->fashion[i].color);
	attrs[PLAYER_ATTR_WEAPON_COLOR] = player->data->fashion[i].color;;

	player->notify_attr(attrs, true, true);
done:
	send.id = fashoinid;
	send.color = color;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_SET_FASHION_COLOR_ANSWER, set_fashion_color_answer__pack, send);


	return 0;
}
static int  handle_puton_fashion_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PutonFashion *req = puton_fashion__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint32_t fashionId = req->id;
	puton_fashion__free_unpacked(req, NULL);

	int index = player->get_fashion(fashionId);
	if (index < 0)
	{
		return 2;
	}
	std::map<uint64_t, struct ActorFashionTable *>::iterator itFashion = fashion_config.find(fashionId);
	if (itFashion == fashion_config.end())
	{
		return 4;
	}
	AttrMap attrs;
	int resType = PLAYER_ATTR_CLOTHES;
	int colorType = PLAYER_ATTR_CLOTHES_COLOR_UP;
	PutonFashionAns send;
	puton_fashion_ans__init(&send);
	if (player->data->fashion[index].timeout != 0
		&& player->data->fashion[index].timeout <= (time_t)time_helper::get_cached_time() / 1000)
	{
		send.ret = 190500088;
		goto done; //过期
	}

	if (itFashion->second->Type == FASHION_TYPE_WEAPON)
	{
		resType = PLAYER_ATTR_WEAPON;
	}
	else if (itFashion->second->Type == FASHION_TYPE_HAT)
	{
		resType = PLAYER_ATTR_HAT;
		colorType = PLAYER_ATTR_HAT_COLOR;
	}

	send.newid = player->get_attr(resType);
	send.ret = 0;
	send.id = fashionId;


	player->set_attr(resType, fashionId);

	attrs[resType] = fashionId;
	if (resType != PLAYER_ATTR_WEAPON)
	{
		player->set_attr(colorType, player->data->fashion[index].color);
		attrs[colorType] = player->data->fashion[index].color;
	}
	if (resType == PLAYER_ATTR_CLOTHES)
	{
		player->set_attr(PLAYER_ATTR_CLOTHES_COLOR_DOWN, player->data->fashion[index].colordown);
		attrs[PLAYER_ATTR_CLOTHES_COLOR_DOWN] = player->data->fashion[index].colordown;
	}
	player->notify_attr(attrs, true, true);
	player->add_task_progress(TCT_FASHION_WEAR, fashionId, 1);
done:
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PUTON_FASHION_ANSWER, puton_fashion_ans__pack, send);

	return 0;
}
static int  handle_takeoff_fashion_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PutonFashion *req = puton_fashion__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint32_t fashionId = req->id;
	puton_fashion__free_unpacked(req, NULL);

	if (player->get_fashion(fashionId) < 0)
	{
		return 2;
	}
	std::map<uint64_t, struct ActorFashionTable *>::iterator itFashion = fashion_config.find(fashionId);
	if (itFashion == fashion_config.end())
	{
		return 3;
	}

	//换默认
	uint32_t carrerid = 101000000 + player->get_job();
	std::map<uint64_t, struct ActorTable *>::iterator it = actor_config.find(carrerid);
	if (it == actor_config.end())
	{
		return 5;
	}

	AttrMap attrs;
	PutonFashionAns send;
	puton_fashion_ans__init(&send);
	if (itFashion->second->Type == FASHION_TYPE_WEAPON)
	{
		player->set_attr(PLAYER_ATTR_WEAPON, it->second->WeaponId);
		attrs[PLAYER_ATTR_WEAPON] = it->second->WeaponId;
		send.newid = it->second->WeaponId;
	}
	else if (itFashion->second->Type == FASHION_TYPE_HAT)
	{
		if (it->second->n_HairResId > 0)
		{
			player->set_attr(PLAYER_ATTR_HAT, it->second->HairResId[0]);
			attrs[PLAYER_ATTR_HAT] = it->second->HairResId[0];
			send.newid = it->second->HairResId[0];
		}
		std::map<uint64_t, struct ActorFashionTable *>::iterator itFNew = fashion_config.find(attrs[PLAYER_ATTR_HAT]);
		if (itFNew != fashion_config.end())
		{
			if (itFNew->second->n_ColourID1 > 0)
			{
				player->set_attr(PLAYER_ATTR_HAT_COLOR, itFNew->second->ColourID1[0]);
				attrs[PLAYER_ATTR_HAT_COLOR] = itFNew->second->ColourID1[0];
			}
		}
	}
	else if (itFashion->second->Type == FASHION_TYPE_CLOTHES)
	{
		if (it->second->n_ResId > 0)
		{
			player->set_attr(PLAYER_ATTR_CLOTHES, it->second->ResId[0]);
			attrs[PLAYER_ATTR_CLOTHES] = it->second->ResId[0];
			send.newid = it->second->ResId[0];
		}
		std::map<uint64_t, struct ActorFashionTable *>::iterator itFNew = fashion_config.find(it->second->ResId[0]);
		if (itFNew != fashion_config.end())
		{
			if (itFNew->second->n_ColourID1 > 0)
			{
				player->set_attr(PLAYER_ATTR_CLOTHES_COLOR_UP, itFNew->second->ColourID1[0]);
				attrs[PLAYER_ATTR_CLOTHES_COLOR_UP] = itFNew->second->ColourID1[0];
			}
			if (itFNew->second->n_ColourID2 > 0)
			{
				player->set_attr(PLAYER_ATTR_CLOTHES_COLOR_DOWN, itFNew->second->ColourID2[0]);
				attrs[PLAYER_ATTR_CLOTHES_COLOR_DOWN] = itFNew->second->ColourID2[0];
			}
		}
	}
	player->notify_attr(attrs, true, true);
	send.ret = 0;
	send.id = fashionId;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TAKEOFF_FASHION_ANSWER, puton_fashion_ans__pack, send);

	return 0;
}
static int handle_set_color_old_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	UnlockColor *req = unlock_color__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	int index = player->get_color(req->color);
	if (index >= 0)
	{
		player->data->color_is_new[index] = false;
	}

	UnlockColorAns send;
	unlock_color_ans__init(&send);
	send.ret = 0;
	send.color = req->color;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_SET_COLOR_OLD_ANSWER, unlock_color_ans__pack, send);

	unlock_color__free_unpacked(req, NULL);
	return 0;
}
static int handle_set_weapon_color_old_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	UnlockColor *req = unlock_color__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	int index = player->get_weapon_color(req->color);
	if (index >= 0)
	{
		player->data->weapon_color_is_new[index] = false;
	}

	UnlockColorAns send;
	unlock_color_ans__init(&send);
	send.ret = 0;
	send.color = req->color;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_SET_WEAPON_COLOR_OLD_ANSWER, unlock_color_ans__pack, send);

	unlock_color__free_unpacked(req, NULL);
	return 0;
}
static int handle_set_fashion_old_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PutonFashion *req = puton_fashion__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint32_t fashionId = req->id;
	puton_fashion__free_unpacked(req, NULL);

	int index = player->get_fashion(fashionId);
	if (index < 0)
	{
		return 2;
	}

	player->set_fashion_old(fashionId);

	PutonFashionAns send;
	puton_fashion_ans__init(&send);
	send.ret = 0;
	send.id = fashionId;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_FASHION_OLD_ANSWER, puton_fashion_ans__pack, send);


	return 0;
}
static int handle_set_cur_horse_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	HorseId *req = horse_id__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint32_t horseid = req->id;
	horse_id__free_unpacked(req, NULL);

	int i = player->get_horse(horseid);
	if (i < 0)
	{
		return -2;
	}

	int oldHorse = player->get_attr(PLAYER_ATTR_CUR_HORSE);
	SetCurHorseAns send;
	set_cur_horse_ans__init(&send);
	send.ret = 0;
	if ((uint64_t)player->data->horse[i].timeout != 0 && time_helper::get_cached_time() / 1000 >= (uint64_t)player->data->horse[i].timeout)
	{
		send.ret = 190500096;
		goto done;
	}

	player->set_attr(PLAYER_ATTR_CUR_HORSE, horseid);

	send.id = horseid;
	player->calc_horse_attr();
	send.power = player->data->horse_attr.power;
	send.playerid = player->get_uuid();
	player->calculate_attribute(true);
	send.old_id = oldHorse;
done:
	player->broadcast_to_sight(MSG_ID_SET_CUR_HORSE_ANSWER, &send, (pack_func)set_cur_horse_ans__pack, true);


	return 0;
}

static int handle_buy_horse_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	BuyHorse *req = buy_horse__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint32_t horseId = req->id;
	uint32_t shopid = req->shopid;
	int type = req->type;
	buy_horse__free_unpacked(req, NULL);

	std::map<uint64_t, struct MountsTable *>::iterator it = horse_config.find(horseId);
	if (it == horse_config.end())
	{
		return 3;
	}
	if (shopid < 0 || shopid > 2)
	{
		return 4;
	}
	BuyHorseAns send;
	buy_horse_ans__init(&send);

	send.ret = 0;
	if (type == 0)
	{
		if (shopid >= it->second->n_Item || shopid >= it->second->n_ItemNum ||
			player->del_item(it->second->Item[shopid], it->second->ItemNum[shopid], MAGIC_TYPE_HORSE) < 0)
		{
			send.ret = 190400006;
		}
	} 
	else
	{
		if (shopid >= it->second->n_WingBinding ||
			player->sub_comm_gold(it->second->WingBinding[shopid], MAGIC_TYPE_HORSE) < 0)
		{
			send.ret = 190400005;
		}
	}
	if (send.ret != 0)
	{
		fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_BUY_HORSE_ANSWER, buy_horse_ans__pack, send);
		return -2;
	}

	if (shopid >= it->second->n_Time)
	{
		return 5;
	}

	//int i = player->add_horse(horseId, it->second->Time[shopid]);
	int i = player->add_horse(horseId, 30);
	if (i < 0 || i >= MAX_HORSE_NUM)
	{
		return -1;
	}
	player->notify_add_horse(i);
	//HorseData hData;
	//send.data = &hData;
	//horse_data__init(&hData);
	//hData.id = player->data->horse[i].id;
	//hData.isnew = player->data->horse[i].isNew;

	//if (player->data->horse[i].timeout != 0)
	//{
	//	if (player->data->horse[i].timeout <= (time_t)time_helper::get_cached_time() / 1000)
	//	{
	//		hData.isexpire = true;
	//	}
	//	else
	//	{
	//		hData.cd = player->data->horse[i].timeout - time_helper::get_cached_time() / 1000;
	//		hData.isexpire = false;
	//	}
	//}
	//else
	//{
	//	hData.isexpire = false;
	//	hData.cd = 0;
	//}
	//hData.is_current = false;

	//fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_BUY_HORSE_ANSWER, buy_horse_ans__pack, send);

	return 0;
}
static int handle_down_horse_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}

	if (player->data->attrData[PLAYER_ATTR_CUR_HORSE] == 0)
	{
		return 1;
	}
	if (player->data->attrData[PLAYER_ATTR_ON_HORSE_STATE] == 0)
	{
		return 2;
	}

	ActorLevelTable *level_config = get_actor_level_config(player->get_attr(PLAYER_ATTR_JOB), player->get_attr(PLAYER_ATTR_LEVEL));
	if (level_config != NULL)
	{
		ActorAttributeTable* config = get_config_by_id(level_config->ActorLvAttri, &actor_attribute_config);
		if (config != NULL)
		{
			AttrMap attrs;
			attrs[PLAYER_ATTR_MOVE_SPEED] = config->MoveSpeed;
			player->set_attr(PLAYER_ATTR_MOVE_SPEED, config->MoveSpeed);
			player->notify_attr(attrs, true, true);
		}
	}

	player->data->attrData[PLAYER_ATTR_ON_HORSE_STATE] = 0;
	OnHorse send;
	on_horse__init(&send);
	send.playerid = player->get_uuid();
	send.horseid = player->data->attrData[PLAYER_ATTR_CUR_HORSE];
	player->broadcast_to_sight(MSG_ID_DOWN_HORSE_NOTIFY, &send, (pack_func)on_horse__pack, false);
	player->adjust_battle_partner();

	return 0;
}
static int calc_add_horse_exp(player_struct *player, int num, uint32_t ind, int &ret)
{
	std::map<uint64_t, struct SpiritTable*>::iterator it = spirit_config.find(player->data->horse_attr.step);
	if (it == spirit_config.end())
	{
		return 1;
	}
	//if (player->data->horse_attr.step >= spirit_config.size())
	//{
	//	return 2;
	//}
	//if (player->get_attr(PLAYER_ATTR_LEVEL) < it->second->Level)
	//{
	//	return 190500094;
	//}
	ret = 0;
	for (int tmp = 1; tmp <= num || num == 0; ++tmp)
	{

		if (player->data->horse_attr.attr_exp[ind] + tmp > it->second->GradeNum[ind])
		{
			break;
		}
		if (player->del_item(it->second->SpiritExpend, it->second->ExpendNum, MAGIC_TYPE_HORSE) < 0)
		{
			 break;
		}
		++ret;
	}

	if (ret == 0)
	{
		return 190500063;  //
	}
	else
	{
		return 0;
	}

}
static int handle_add_horse_exp_request(player_struct *player, EXTERN_DATA *extern_data) //修灵
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}

	HorseAttr *req = horse_attr__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint32_t num = req->type;
	uint32_t attid = req->arrtid;
	horse_attr__free_unpacked(req, NULL);

	uint32_t i = 0;
	for (; i < MAX_HORSE_ATTR_NUM; ++i)
	{
		if (player->data->horse_attr.attr[i] == attid)
		{
			break;
		}
	}
	if (i == MAX_HORSE_ATTR_NUM)
	{
		return -11;
	}

	HorseAttrAns send;
	horse_attr_ans__init(&send);
	int add = 0;
	send.ret = calc_add_horse_exp(player, num, i, add);
	if (send.ret != 0)
	{
		goto done;
	}

	player->data->horse_attr.attr_exp[i] += add;
	player->add_task_progress(TCT_HORSE_ADD_EXP, 0, 1);

done:
	send.arrtid = attid;
	send.num = player->data->horse_attr.attr_exp[i];
	player->calc_horse_attr();
	send.power = player->data->horse_attr.power;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_ADD_HORSE_EXP_ANSWER, horse_attr_ans__pack, send);
	player->calculate_attribute(true);
	return 0;
}
static int check_horse_exp_full(player_struct *player)
{
	std::map<uint64_t, struct SpiritTable*>::iterator it = spirit_config.find(player->data->horse_attr.step);
	if (it == spirit_config.end())
	{
		return 1;
	}
	if (player->get_attr(PLAYER_ATTR_LEVEL) < it->second->Level)
	{
		return 2;
	}
	if (player->data->horse_attr.step == spirit_config.size())
	{
		return 3;
	}

	for (int i = 0; i < MAX_HORSE_ATTR_NUM; ++i)
	{
		if (player->data->horse_attr.attr_exp[i] < it->second->GradeNum[i])
		{
			return 4;
		}
	}
	return 0;
}
static int handle_add_horse_step_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}


	HorseStepAns send;
	horse_step_ans__init(&send);
	send.ret = check_horse_exp_full(player);

	if (send.ret != 0)
	{
		goto done;
	}

	++(player->data->horse_attr.step);
	for (int i = 0; i < MAX_HORSE_ATTR_NUM; ++i)
	{
		player->data->horse_attr.attr_exp[i] = 0;
	}
done:
	send.step = player->data->horse_attr.step;
	player->calc_horse_attr();
	send.power = player->data->horse_attr.power;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_ADD_HORSE_STEP_ANSWER, horse_step_ans__pack, send);
	player->calculate_attribute(true);

	return 0;
}
static int handle_add_horse_soul_level_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}

	std::map<uint64_t, struct CastSpiritTable*>::iterator it = horse_soul_config.find(MAX_HORSE_SOUL); //坐骑铸灵配置
	if (it == horse_soul_config.end())
	{
		return -2;
	}
	if (player->data->horse_attr.soul == it->second->n_ToddlerLv)
	{
		return -3;//满级
	}

	for (uint32_t i = 1; i <= MAX_HORSE_SOUL; ++i)
	{
		//std::map<uint64_t, struct CastSpiritTable*>::iterator it = horse_soul_config.find(i); //每个卦升级次数都是一样的 
		//if (it == horse_soul_config.end())
		//{
		//	return -2;
		//}
		if (player->data->horse_attr.soul_exp[i] < it->second->GradeNum[player->data->horse_attr.soul - 1])
		{
			return -4;
		}
	}
	if (it->second->AdvancedLv[player->data->horse_attr.soul - 1] > player->get_attr(PLAYER_ATTR_LEVEL))
	{
		return 190500092;
	}

	memset(player->data->horse_attr.soul_exp, 0, sizeof(player->data->horse_attr.soul_exp));

	MountsTable *table = get_config_by_id(it->second->AdvancedGet[player->data->horse_attr.soul - 1], &horse_config);
	if (table != NULL)
	{
		int pos = player->add_horse(it->second->AdvancedGet[player->data->horse_attr.soul - 1], 0);
		if (pos >= 0)
		{
			player->notify_add_horse(pos);
		}
	}
	player->data->horse_attr.soul += 1;

	HorseSoulLevelAns send;
	horse_soul_level_ans__init(&send);
	send.ret = 0;
	send.level = player->data->horse_attr.soul;
	player->calc_horse_attr();
	send.power = player->data->horse_attr.power;
	player->data->horse_attr.soul_full = false;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_ADD_HORSE_SOUL_LEVEL_ANSWER, horse_soul_level_ans__pack, send);
	player->calculate_attribute(true);
	return 0;
}
static int handle_set_horse_old_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	HorseId *req = horse_id__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint32_t horseid = req->id;
	horse_id__free_unpacked(req, NULL);

	int i = player->get_horse(horseid);
	if (i < 0)
	{
		return -2;
	}
	player->data->horse[i].isNew = false;
	HorseId send;
	horse_id__init(&send);
	send.id = horseid;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_SET_HORSE_OLD_ANSWER, horse_id__pack, send);

	return 0;
}
static int handle_add_horse_soul_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}

	int cur = player->data->horse_attr.cur_soul;
	std::map<uint64_t, struct CastSpiritTable*>::iterator it = horse_soul_config.find(cur); //坐骑铸灵配置
	if (it == horse_soul_config.end())
	{
		return -2;
	}
	HorseSoulAns send;
	horse_soul_ans__init(&send);
	send.ret = 0;
	int lv = player->data->horse_attr.soul - 1;
	if (player->data->horse_attr.soul_exp[cur] == it->second->GradeNum[lv])
	{
		return -3; //满了
	}
	//if (player->data->horse_attr.soul == it->second->n_GradeNum)
	//{
	//	return -5;//满级
	//}
	if (player->get_item_num_by_id(it->second->Cast1Expend[lv]) <= 0)
	{
		send.ret = 190600003;
		goto done;
	}
	if (player->get_item_num_by_id(it->second->Cast2Expend[lv]) <= 0)
	{
		send.ret = 190600003;
		goto done;
	}
	player->del_item(it->second->Cast1Expend[lv], it->second->Expend1Num[lv], MAGIC_TYPE_HORSE);
	player->del_item(it->second->Cast2Expend[lv], it->second->Expend2Num[lv], MAGIC_TYPE_HORSE);

	++(player->data->horse_attr.soul_exp[cur]);

	if (player->data->horse_attr.soul_exp[cur] == it->second->GradeNum[lv])
	{
		++player->data->horse_attr.cur_soul; //满了
		if (player->data->horse_attr.cur_soul > MAX_HORSE_SOUL)
		{
			player->data->horse_attr.cur_soul = 1;
			player->data->horse_attr.soul_full = true;
		}
	}
done:
	send.soul = cur;
	player->calc_horse_attr();
	send.power = player->data->horse_attr.power;
	send.num = player->data->horse_attr.soul_exp[cur];
	send.cur_soul = player->data->horse_attr.cur_soul;
	send.soul_full = player->data->horse_attr.soul_full;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_ADD_HORSE_SOUL_ANSWER, horse_soul_ans__pack, send);
	player->calculate_attribute(true);

	return 0;
}
static int handle_on_horse_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}

	if (!player->scene->can_use_horse())
	{
		LOG_ERR("%s: %lu can not use horse in scene %u", __FUNCTION__, extern_data->player_id, player->scene->m_id);
		return (-10);
	}

	if (player->data->attrData[PLAYER_ATTR_CUR_HORSE] == 0)
	{
		return 1;
	}
	if (player->data->attrData[PLAYER_ATTR_ON_HORSE_STATE] == 1)
	{
		return 2;
	}
	if (player->data->truck.truck_id != 0)
	{
		return 4;
	}

	OnHorseRequest *req = on_horse_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (3);
	}

	player->interrupt();

	player->data->pos_y = req->pos_y;
	on_horse_request__free_unpacked(req, NULL);

	AttrMap attrs;
	attrs[PLAYER_ATTR_MOVE_SPEED] = player->data->horse_attr.total[PLAYER_ATTR_MOVE_SPEED];
	player->set_attr(PLAYER_ATTR_MOVE_SPEED, player->data->horse_attr.total[PLAYER_ATTR_MOVE_SPEED]);
	player->notify_attr(attrs, true, true);
	player->data->horse_attr.fly = 1;

	player->data->attrData[PLAYER_ATTR_ON_HORSE_STATE] = 1;
	OnHorse send;
	on_horse__init(&send);
	send.playerid = player->get_uuid();
	send.horseid = player->data->attrData[PLAYER_ATTR_CUR_HORSE];
	player->broadcast_to_sight(MSG_ID_ON_HORSE_NOTIFY, &send, (pack_func)on_horse__pack, false);
	player->adjust_battle_partner();

	return 0;
}
static int handle_set_horse_fly_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}

	if (player->data->attrData[PLAYER_ATTR_CUR_HORSE] == 0)
	{
		return 1;
	}
	if (player->data->attrData[PLAYER_ATTR_ON_HORSE_STATE] == 0)
	{
		return 2;
	}

	FlyState *req = fly_state__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (3);
	}
	int fly = req->fly;
	fly_state__free_unpacked(req, NULL);

	player->data->horse_attr.fly = fly;
	AttrMap attrs;
	if (fly == 2)
	{
		attrs[PLAYER_ATTR_MOVE_SPEED] = player->data->horse_attr.total[PLAYER_ATTR_FLY_SPEED];
		player->set_attr(PLAYER_ATTR_MOVE_SPEED, player->data->horse_attr.total[PLAYER_ATTR_FLY_SPEED]);
	}
	else
	{
		attrs[PLAYER_ATTR_MOVE_SPEED] = player->data->horse_attr.total[PLAYER_ATTR_MOVE_SPEED];
		player->set_attr(PLAYER_ATTR_MOVE_SPEED, player->data->horse_attr.total[PLAYER_ATTR_MOVE_SPEED]);
	}

	player->notify_attr(attrs, true, true);

	return 0;
}
static int handle_horse_restory_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (comm_check_player_valid(player, extern_data->player_id) != 0)
	{
		LOG_ERR("%s: %lu common check failed", __FUNCTION__, extern_data->player_id);
		return (-1);
	}

	int oldHorse = player->get_attr(PLAYER_ATTR_CUR_HORSE);

	player->set_attr(PLAYER_ATTR_CUR_HORSE, DEFAULT_HORSE);
	SetCurHorseAns send;
	set_cur_horse_ans__init(&send);
	send.ret = 0;
	send.id = DEFAULT_HORSE;
	player->calc_horse_attr();
	send.power = player->data->horse_attr.power;
	send.playerid = player->get_uuid();
	player->calculate_attribute(true);
	send.old_id = oldHorse;
	player->broadcast_to_sight(MSG_ID_SET_CUR_HORSE_ANSWER, &send, (pack_func)set_cur_horse_ans__pack, true);

	return 0;
}

//商城信息请求
static int handle_shop_info_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	uint64_t cur_tick = time_helper::get_cached_time() / 1000;
	std::set<uint64_t> shop_ids;

	ShopInfoAnswer resp;
	shop_info_answer__init(&resp);

	ShopOpenData shop_data[MAX_SHOP_LIST_NUM];
	ShopOpenData* shop_data_point[MAX_SHOP_LIST_NUM];
	GoodsData goods_data[MAX_SHOP_GOODS_NUM];
	GoodsData* goods_data_point[MAX_SHOP_GOODS_NUM];

	resp.n_shops = 0;
	resp.shops = shop_data_point;
	for (std::map<uint64_t, ShopListTable*>::iterator iter = shop_list_config.begin(); iter != shop_list_config.end() && resp.n_shops < MAX_SHOP_LIST_NUM; ++iter)
	{
		ShopListTable *config = iter->second;
		if (config->StartTime == 0 || config->EndTime == 0 ||
				(config->StartTime > 0 && config->EndTime > 0 &&
				 cur_tick >= config->StartTime && cur_tick < config->EndTime))
		{
			shop_data_point[resp.n_shops] = &shop_data[resp.n_shops];
			shop_open_data__init(&shop_data[resp.n_shops]);
			shop_data[resp.n_shops].shopid = config->ID;
			shop_data[resp.n_shops].opentime = config->EndTime;
			resp.n_shops++;
			shop_ids.insert(config->ID);
		}
	}

	resp.n_goods = 0;
	resp.goods = goods_data_point;
	for (int i = 0; i < MAX_SHOP_GOODS_NUM; ++i)
	{
		uint32_t goods_id = player->data->shop_goods[i].goods_id;
		if (goods_id == 0)
		{
			break;
		}

		ShopTable *goods_config = get_config_by_id(goods_id, &shop_config);
		if (!goods_config || goods_config->Acc == 2 || shop_ids.find(goods_config->ShopType) == shop_ids.end())
		{
			continue;
		}

		goods_data_point[resp.n_goods] = &goods_data[resp.n_goods];
		goods_data__init(&goods_data[resp.n_goods]);
		goods_data[resp.n_goods].goodsid = player->data->shop_goods[i].goods_id;
		goods_data[resp.n_goods].boughtnum = player->data->shop_goods[i].bought_num;
		resp.n_goods++;
	}

	resp.result = 0;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_SHOP_INFO_ANSWER, shop_info_answer__pack, resp);

	return 0;
}

//商城购买请求
static int handle_shop_buy_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	ShopBuyRequest *req = shop_buy_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t goods_id = req->goodsid;
	uint32_t buy_num = req->goodsnum;

	shop_buy_request__free_unpacked(req, NULL);

	int ret = 0;
	GoodsInfo *goods_info = NULL;
	do
	{
		ShopTable *goods_config = get_config_by_id(goods_id, &shop_config);
		if (!goods_config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get goods config failed, goods_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, goods_id);
			break;
		}

		ShopListTable *list_config = get_config_by_id(goods_config->ShopType, &shop_list_config);
		if (!list_config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get list config failed, shop_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, goods_config->ShopType);
			break;
		}

		//限时的，检查时间
		uint64_t cur_tick = time_helper::get_cached_time() / 1000;
		if (list_config->StartTime > 0 && list_config->EndTime > 0 &&
				!(cur_tick >= list_config->StartTime && cur_tick <= list_config->EndTime))
		{
			ret = ERROR_ID_SHOP_CLOSED;
			LOG_ERR("[%s:%d] player[%lu] shop is closed, shop_id:%lu, start:%lu, end:%lu, now:%lu", __FUNCTION__, __LINE__, extern_data->player_id, goods_config->ShopType, list_config->StartTime, list_config->EndTime, cur_tick);
			break;
		}

		if (goods_config->Acc == 2)
		{
			ret = ERROR_ID_SHOP_GOODS_NOT_SELL;
			LOG_ERR("[%s:%d] player[%lu] goods not sell, goods_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, goods_id);
			break;
		}

		for (int i = 0; i < MAX_SHOP_GOODS_NUM; ++i)
		{
			uint32_t tmp_id = player->data->shop_goods[i].goods_id;
			if (tmp_id == 0 || tmp_id == goods_id)
			{
				goods_info = &player->data->shop_goods[i];
				break;
			}
		}

		if (!goods_info)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] error, goods_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, goods_id);
			break;
		}

		if (buy_num == 0)
		{
			ret = ERROR_ID_SHOP_GOODS_BUY_NUM;
			break;
		}

		//限购的，检查数量
		if ((int64_t)goods_config->BuyNum > 0 && goods_info->bought_num + buy_num > (uint32_t)goods_config->BuyNum)
		{
			ret = ERROR_ID_SHOP_GOODS_REMAIN;
			LOG_ERR("[%s:%d] player[%lu] goods remain not enough, goods_id:%u, limit_num:%lu, bought_num:%u, want_buy_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, goods_id, goods_config->BuyNum, goods_info->bought_num, buy_num);
			break;
		}

		uint32_t item_id = goods_config->ItemID;
		if (!player->check_can_add_item(item_id, buy_num, NULL))
		{
			ret = ERROR_ID_SHOP_BUY_BAG_FULL;
			LOG_ERR("[%s:%d] player[%lu] bag not enough, goods_id:%u, want_buy_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, goods_id, buy_num);
			break;
		}

		uint32_t money_type = goods_config->ConsumptionType;
		uint32_t need_money = goods_config->Discount * buy_num;
		uint32_t has_money = 0;
		uint32_t error_id = 0;
		switch(money_type)
		{
			case 1: //银币
				has_money = player->get_coin();
				error_id = ERROR_ID_COIN_NOT_ENOUGH;
				break;
			case 2: //元宝
				has_money = player->get_attr(PLAYER_ATTR_GOLD);
				error_id = ERROR_ID_GOLD_NOT_ENOUGH;
				break;
			case 3: //绑定元宝，不够可以消耗非绑定
				has_money = player->get_comm_gold();
				error_id = ERROR_ID_GOLD_NOT_ENOUGH;
				break;
			case 4: //积分
				break;
		}

		if (has_money < need_money)
		{
			ret = error_id;
			LOG_ERR("[%s:%d] player[%lu] money not enough, goods_id:%u, money_type:%u, need_money:%u, has_money:%u", __FUNCTION__, __LINE__, extern_data->player_id, goods_id, money_type, need_money, has_money);
			break;
		}

		switch(money_type)
		{
			case 1: //银币
				player->sub_coin(need_money, MAGIC_TYPE_SHOP_BUY);
				break;
			case 2: //元宝
				player->sub_unbind_gold(need_money, MAGIC_TYPE_SHOP_BUY);
				break;
			case 3: //绑定元宝，不够可以消耗非绑定
				player->sub_comm_gold(need_money, MAGIC_TYPE_SHOP_BUY);
				break;
			case 4: //积分
				break;
		}

		player->add_item(item_id, buy_num, MAGIC_TYPE_SHOP_BUY);

		//只有限购的商品，才记录
		if ((int64_t)goods_config->BuyNum > 0)
		{
			goods_info->goods_id = goods_id;
			goods_info->bought_num += buy_num;
		}

		player->add_task_progress(TCT_SHOP_BUY, goods_info->goods_id, 1);
	} while(0);

	ShopBuyAnswer resp;
	shop_buy_answer__init(&resp);

	resp.result = ret;
	resp.goodsid = goods_id;
	resp.boughtnum = (goods_info ? goods_info->bought_num : 0);
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_SHOP_BUY_ANSWER, shop_buy_answer__pack, resp);

	return 0;
}

//获取御气道信息请求
static int handle_yuqidao_info_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	return notify_yuqidao_info(player, extern_data);
}

static int notify_pvp_raid_info(player_struct *player, int type, EXTERN_DATA *extern_data)
{
	player->send_pvp_raid_score_changed(PVP_TYPE_DEFINE_3);
	player->send_pvp_raid_score_changed(PVP_TYPE_DEFINE_5);
	return (0);
}

static int notify_yuqidao_info(player_struct *player, EXTERN_DATA *extern_data)
{
	YuqidaoInfoAnswer resp;
	yuqidao_info_answer__init(&resp);

	YuqidaoMaiData mai_data[MAX_YUQIDAO_MAI_NUM];
	YuqidaoMaiData* mai_data_point[MAX_YUQIDAO_MAI_NUM];

	YuqidaoBreakData break_data[MAX_YUQIDAO_BREAK_NUM];
	YuqidaoBreakData* break_data_point[MAX_YUQIDAO_BREAK_NUM];

	resp.n_mais = 0;
	resp.mais = mai_data_point;
	for (int i = 0; i < MAX_YUQIDAO_MAI_NUM; ++i)
	{
		YuqidaoMaiInfo &info = player->data->yuqidao_mais[i];
		if (info.mai_id == 0)
		{
			continue;
		}

		mai_data_point[resp.n_mais] = &mai_data[resp.n_mais];
		yuqidao_mai_data__init(&mai_data[resp.n_mais]);
		mai_data[resp.n_mais].maiid = info.mai_id;
		mai_data[resp.n_mais].acupointid = info.acupoint_id;
		mai_data[resp.n_mais].filllv = info.fill_lv;
		resp.n_mais++;
	}

	resp.n_breaks = 0;
	resp.breaks = break_data_point;
	for (int i = 0; i < MAX_YUQIDAO_BREAK_NUM; ++i)
	{
		YuqidaoBreakInfo &info = player->data->yuqidao_breaks[i];
		if (info.id == 0)
		{
			continue;
		}

		break_data_point[resp.n_breaks] = &break_data[resp.n_breaks];
		yuqidao_break_data__init(&break_data[resp.n_breaks]);
		break_data[resp.n_breaks].id = info.id;
		break_data[resp.n_breaks].curattr = info.cur_val;
		break_data[resp.n_breaks].n_curattr = MAX_YUQIDAO_BREAK_ATTR_NUM;
		break_data[resp.n_breaks].newattr = info.new_val;
		break_data[resp.n_breaks].n_newattr = MAX_YUQIDAO_BREAK_ATTR_NUM;
		break_data[resp.n_breaks].newaddn = info.new_addn;
		break_data[resp.n_breaks].n_newaddn = MAX_YUQIDAO_BREAK_ATTR_NUM;
		resp.n_breaks++;
	}

	resp.result = 0;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_YUQIDAO_INFO_ANSWER, yuqidao_info_answer__pack, resp);

	return 0;
}

//御气道灌入真气请求
static int handle_yuqidao_fill_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	YuqidaoFillRequest *req = yuqidao_fill_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t mai_id = req->maiid;

	yuqidao_fill_request__free_unpacked(req, NULL);

	int ret = 0;
	YuqidaoMaiInfo *mai_info = NULL;
	do
	{
		mai_info = player->get_yuqidao_mai(mai_id);
		if (!mai_info)
		{
			ret = ERROR_ID_YUQIDAO_MAI_NOT_OPEN;
			LOG_ERR("[%s:%d] player[%lu] jingmai is not open, mai_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, mai_id);
			break;
		}

		AcupunctureTable *config = get_config_by_id(mai_info->acupoint_id, &yuqidao_acupoint_config);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get acupoint config failed, mai_id:%u, acupoint_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, mai_id, mai_info->acupoint_id);
			break;
		}

		uint32_t need_coin = config->ExpendSilver;
		uint32_t has_coin = player->get_coin();
		if (has_coin < need_coin)
		{
			ret = ERROR_ID_COIN_IS_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] coin not enough, mai_id:%u, acupoint_id:%u, need_coin:%u, has_coin:%u", __FUNCTION__, __LINE__, extern_data->player_id, mai_id, mai_info->acupoint_id, need_coin, has_coin);
			break;
		}

		uint32_t need_zhenqi = config->ExpendQi;
		uint32_t has_zhenqi = player->get_zhenqi();
		if (has_zhenqi < need_zhenqi)
		{
			ret = ERROR_ID_ZHENQI_IS_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] zhenqi not enough, mai_id:%u, acupoint_id:%u, need_zhenqi:%u, has_zhenqi:%u", __FUNCTION__, __LINE__, extern_data->player_id, mai_id, mai_info->acupoint_id, need_zhenqi, has_zhenqi);
			break;
		}

		if (mai_info->fill_lv >= (uint32_t)config->GradeNum)
		{
			ret = ERROR_ID_YUQIDAO_MAI_FILL_LV_MAX;
			LOG_ERR("[%s:%d] player[%lu] fill level max, mai_id:%u, acupoint_id:%u, cur_lv:%u, max_lv:%u", __FUNCTION__, __LINE__, extern_data->player_id, mai_id, mai_info->acupoint_id, mai_info->fill_lv, (uint32_t)config->GradeNum);
			break;
		}

		player->sub_coin(need_coin, MAGIC_TYPE_YUQIDAO_FILL);
		player->sub_zhenqi(need_zhenqi, MAGIC_TYPE_YUQIDAO_FILL);

		mai_info->fill_lv++;
		if (mai_info->fill_lv >= (uint32_t)config->GradeNum)
		{
			uint32_t next_acupoint = mai_info->acupoint_id + 1;
			if (get_config_by_id(next_acupoint, &yuqidao_acupoint_config) != NULL)
			{
				mai_info->acupoint_id = next_acupoint;
				mai_info->fill_lv = 0;
			}
			else
			{
				mai_info->acupoint_id = 0;
				mai_info->fill_lv = 0;

				PulseTable *mai_config = get_config_by_id(mai_id, &yuqidao_jingmai_config);
				if (mai_config)
				{
					bool open_break = true;
					for (std::map<uint64_t, struct PulseTable*>::iterator iter = yuqidao_jingmai_config.begin(); iter != yuqidao_jingmai_config.end(); ++iter)
					{
						PulseTable *tmp_config = iter->second;
						if (tmp_config->ID == mai_config->ID)
						{
							continue;
						}

						if (tmp_config->Break != mai_config->Break)
						{
							continue;
						}

						YuqidaoMaiInfo *tmp_mai = player->get_yuqidao_mai(tmp_config->ID);
						if (!tmp_mai || tmp_mai->acupoint_id != 0)
						{
							open_break = false;
							break;
						}
					}

					if (open_break)
					{
						player->init_yuqidao_break(mai_config->Break);
					}
				}
			}
		}

		player->calculate_attribute(true);
	} while(0);

	YuqidaoFillAnswer resp;
	yuqidao_fill_answer__init(&resp);

	resp.result = ret;
	resp.maiid = mai_id;
	if (mai_info)
	{
		resp.acupointid = mai_info->acupoint_id;
		resp.filllv = mai_info->fill_lv;
	}
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_YUQIDAO_FILL_ANSWER, yuqidao_fill_answer__pack, resp);

	return 0;
}

//御气道突破请求
static int handle_yuqidao_break_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	YuqidaoBreakRequest *req = yuqidao_break_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t break_id = req->breakid;
	uint32_t use_time = req->time;
	LOG_INFO("[%s:%d] player[%lu], break_id:%u, use_time:%u", __FUNCTION__, __LINE__, extern_data->player_id, break_id, use_time);

	yuqidao_break_request__free_unpacked(req, NULL);

	int ret = 0;
	YuqidaoBreakInfo *break_info = NULL;
	do
	{
		break_info = player->get_yuqidao_break(break_id);
		if (!break_info)
		{
			ret = ERROR_ID_YUQIDAO_BREAK_NOT_OPEN;
			LOG_ERR("[%s:%d] player[%lu] break is not open, break_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, break_id);
			break;
		}

		BreakTable *break_config = get_config_by_id(break_id, &yuqidao_break_config);
		if (!break_config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get break config failed, break_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, break_id);
			break;
		}

		uint32_t need_lv = break_config->PulseLv;
		uint32_t player_lv = player->get_attr(PLAYER_ATTR_LEVEL);
		if (player_lv < need_lv)
		{
			ret = ERROR_ID_LEVEL_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] level not enough, break_id:%u, need_lv:%u, player_lv:%u", __FUNCTION__, __LINE__, extern_data->player_id, break_id, need_lv, player_lv);
			break;
		}

		uint32_t need_item_id = sg_yuqidao_break_item_id;
		uint32_t need_item_num = sg_yuqidao_break_item_num;
		uint32_t has_item_num = player->get_item_can_use_num(need_item_id);
		if (has_item_num < need_item_num)
		{
			ret = ERROR_ID_PROP_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] item not enough, break_id:%u, item_id:%u, need_num:%u, has_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, break_id, need_item_id, need_item_num, has_item_num);
			break;
		}

		uint32_t tmp_new_attr[MAX_YUQIDAO_BREAK_ATTR_NUM] = {0};
		uint32_t tmp_new_addn[MAX_YUQIDAO_BREAK_ATTR_NUM] = {0};
		bool boom = (break_info->count >= (uint32_t)break_config->Minimum);
		for (int i = 0; i < MAX_YUQIDAO_BREAK_ATTR_NUM; ++i)
		{
			int quality = -1;
			uint64_t *probs = NULL;
			int n_probs = break_config->n_AttributeColor;
			switch (i)
			{
				case 0:
					probs = (boom ? break_config->MeridiansMinimum : break_config->MeridiansProbability);
					break;
				case 1:
					probs = (boom ? break_config->BloodMinimum : break_config->BloodProbability);
					break;
				case 2:
					probs = (boom ? break_config->VitalMinimum : break_config->VitalProbability);
					break;
				case 3:
					probs = (boom ? break_config->MarrowMinimum : break_config->MarrowProbability);
					break;
			}

			//先随机出落在那个品质
			uint32_t rand_num = rand() % probs[n_probs - 1];
			for (int j = 0; j < n_probs; ++j)
			{
				uint32_t lower = (j == 0 ? 0 : probs[j - 1]);
				uint32_t upper = probs[j];
				if (rand_num >= lower && rand_num < upper)
				{
					quality = j;
				}
			}

			if (quality < 0)
			{
				ret = ERROR_ID_NO_CONFIG;
				LOG_ERR("[%s:%d] player[%lu] can't get quality, break_id:%u, attr_no:%u", __FUNCTION__, __LINE__, extern_data->player_id, break_id, i);
				break;
			}

			//再在品质区间内随机出一个值
			int32_t percent_lower = (quality == 0 ? 0 : break_config->AttributeColor[quality - 1]);
			int32_t percent_upper = break_config->AttributeColor[quality];
			int32_t percent_block = percent_upper - percent_lower;
			if (percent_block <= 0)
			{
				ret = ERROR_ID_NO_CONFIG;
				LOG_ERR("[%s:%d] player[%lu] can't get quality block, break_id:%u, attr_no:%u", __FUNCTION__, __LINE__, extern_data->player_id, break_id, i);
				break;
			}

			rand_num = rand() % percent_block;
			double attr_percent = (double)(percent_lower + rand_num) / (double)percent_upper;
			uint32_t attr_lower = break_config->AttributeLower[i];
			uint32_t attr_upper = break_config->AttributeUpper[i];
			uint32_t attr_block = attr_upper - attr_lower;
			tmp_new_attr[i] = attr_lower + attr_percent * attr_block;

			//二次属性，对比配置的按下时间
			bool over = use_time > (uint32_t)break_config->Time;
			int second_part = (over ? break_config->Lost : break_config->Secondary);
			double second_percent = (double)second_part / 10000.0;
			uint32_t second_val = (over ? attr_block * second_percent : attr_block * second_percent * (double)use_time / (double)break_config->Time);
			tmp_new_addn[i] = std::min(second_val, attr_upper - tmp_new_attr[i]); //加起来不能超过上限
			LOG_INFO("[%s:%d] player[%lu], i:%u, second_percent:%f, second_val:%u, addn_val:%u", __FUNCTION__, __LINE__, extern_data->player_id, i, second_percent, second_val, tmp_new_addn[i]);
		}

		if (ret != 0)
		{
			break;
		}

		player->del_item(need_item_id, need_item_num, MAGIC_TYPE_YUQIDAO_BREAK);
		for (int i = 0; i < MAX_YUQIDAO_BREAK_ATTR_NUM; ++i)
		{
			break_info->new_val[i] = tmp_new_attr[i];
			break_info->new_addn[i] = tmp_new_addn[i];
		}

		if (boom)
		{
			break_info->count = 0;
		}
		else
		{
			break_info->count++;
		}
	} while(0);

	YuqidaoBreakAnswer resp;
	yuqidao_break_answer__init(&resp);

	YuqidaoBreakData break_data;
	yuqidao_break_data__init(&break_data);
	break_data.id = break_id;
	if (break_info)
	{
		break_data.curattr = break_info->cur_val;
		break_data.n_curattr = MAX_YUQIDAO_BREAK_ATTR_NUM;
		break_data.newattr = break_info->new_val;
		break_data.n_newattr = MAX_YUQIDAO_BREAK_ATTR_NUM;
		break_data.newaddn = break_info->new_addn;
		break_data.n_newaddn = MAX_YUQIDAO_BREAK_ATTR_NUM;
	}

	resp.result = ret;
	resp.breakinfo = &break_data;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_YUQIDAO_BREAK_ANSWER, yuqidao_break_answer__pack, resp);

	return 0;
}

//御气道突破保留请求
static int handle_yuqidao_break_retain_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	YuqidaoBreakRetainRequest *req = yuqidao_break_retain_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t break_id = req->breakid;

	yuqidao_break_retain_request__free_unpacked(req, NULL);

	int ret = 0;
	YuqidaoBreakInfo *break_info = NULL;
	do
	{
		break_info = player->get_yuqidao_break(break_id);
		if (!break_info)
		{
			ret = ERROR_ID_YUQIDAO_BREAK_NOT_OPEN;
			LOG_ERR("[%s:%d] player[%lu] break is not open, break_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, break_id);
			break;
		}

		if (break_info->new_val[0] == 0)
		{
			ret = ERROR_ID_YUQIDAO_BREAK_RETAIN_ATTR;
			LOG_ERR("[%s:%d] player[%lu] break hasn't new attr, break_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, break_id);
			break;
		}

		bool first_break = (break_info->cur_val[0] == 0);
		for (int i = 0; i < MAX_YUQIDAO_BREAK_ATTR_NUM; ++i)
		{
			break_info->cur_val[i] = break_info->new_val[i] + break_info->new_addn[i];
			break_info->new_val[i] = 0;
			break_info->new_addn[i] = 0;
		}

		player->calculate_attribute(true);

		//第一次冲脉成功，开启下一级穴脉
		if (first_break)
		{
			player->init_yuqidao_mai(break_id, true);
		}
	} while(0);

	YuqidaoBreakRetainAnswer resp;
	yuqidao_break_retain_answer__init(&resp);

	YuqidaoBreakData break_data;
	yuqidao_break_data__init(&break_data);
	break_data.id = break_id;
	if (break_info)
	{
		break_data.curattr = break_info->cur_val;
		break_data.n_curattr = MAX_YUQIDAO_BREAK_ATTR_NUM;
		break_data.newattr = break_info->new_val;
		break_data.n_newattr = MAX_YUQIDAO_BREAK_ATTR_NUM;
		break_data.newaddn = break_info->new_addn;
		break_data.n_newaddn = MAX_YUQIDAO_BREAK_ATTR_NUM;
	}

	resp.result = ret;
	resp.breakinfo = &break_data;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_YUQIDAO_BREAK_RETAIN_ANSWER, yuqidao_break_retain_answer__pack, resp);

	return 0;
}

static void send_set_pk_type_answer(uint32_t result, uint32_t type, EXTERN_DATA *extern_data)
{
	SetPkTypeAnswer resp;
	set_pk_type_answer__init(&resp);
	resp.result = result;
	resp.pk_type = type;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_SET_PK_TYPE_ANSWER, set_pk_type_answer__pack, resp);
}


static int handle_set_pk_type_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	if (player->get_attr(PLAYER_ATTR_MURDER) > sg_muder_cant_set_pktype)
	{
		LOG_ERR("%s: player[%lu] murder punish", __FUNCTION__, extern_data->player_id);
		return (-2);
	}

	if (player->is_in_qiecuo())
	{
		LOG_INFO("%s: player[%lu] in qiecuo state", __FUNCTION__, extern_data->player_id);
		send_set_pk_type_answer(1, 0, extern_data);
		return (-5);
	}

	SetPkTypeRequest *req = set_pk_type_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint32_t type = req->pk_type;
	set_pk_type_request__free_unpacked(req, NULL);

		//检查等级
	if (player->get_attr(PLAYER_ATTR_LEVEL) < sg_pk_level)
	{
		LOG_ERR("%s: player[%lu] level not enough", __FUNCTION__, extern_data->player_id);
		return (-20);
	}
		// TODO: 检查区域

	uint32_t cur_type = (uint32_t)player->get_attr(PLAYER_ATTR_PK_TYPE);
	if (type == cur_type)
		return (0);
	// if (type != PK_TYPE_NORMAL && type != PK_TYPE_CAMP && type != PK_TYPE_MURDER)
	// {
	//	LOG_ERR("%s: player[%lu] pktype wrong[%d]", __FUNCTION__, player->get_uuid(), type);
	//	return (-1);
	// }
	cash_truck_struct *pTruck = cash_truck_manager::get_cash_truck_by_id(player->data->truck.truck_id);
	if (pTruck != NULL && pTruck->get_truck_type() == 2)
	{
		send_set_pk_type_answer(190500311, type, extern_data);
		return (-3);
	}

	switch (type)
	{
		case PK_TYPE_NORMAL:
				//检查CD
			if (player->data->next_set_pktype_time > time_helper::get_cached_time() / 1000 + 2)
			{
				LOG_INFO("[%s:%d] player[%lu] in cd", __FUNCTION__, __LINE__, extern_data->player_id);
				send_set_pk_type_answer(2, type, extern_data);
				return (0);
			}
			break;
		case PK_TYPE_CAMP:
				//增加CD
			if (cur_type == PK_TYPE_NORMAL)
				player->data->next_set_pktype_time = time_helper::get_cached_time() / 1000 + sg_set_pk_type_cd;
			break;
		case PK_TYPE_MURDER:
				//消耗道具
			if (player->del_item(201070037, 1, MAGIC_TYPE_SET_PK_TYPE) != 0)
			{
				LOG_ERR("[%s:%d] player[%lu] do not have item", __FUNCTION__, __LINE__, extern_data->player_id);
				return (-30);
			}
				//增加CD
			if (cur_type == PK_TYPE_NORMAL)
				player->data->next_set_pktype_time = time_helper::get_cached_time() / 1000 + sg_set_pk_type_cd;
			break;
		default:
			LOG_ERR("%s: player[%lu] pktype wrong[%d]", __FUNCTION__, player->get_uuid(), type);
			return (-40);
	}

	LOG_INFO("%s: player[%lu] set pktype [%d]", __FUNCTION__, player->get_uuid(), type);

	send_set_pk_type_answer(0, type, extern_data);

	player->set_attr(PLAYER_ATTR_PK_TYPE, type);
	player->broadcast_one_attr_changed(PLAYER_ATTR_PK_TYPE, type, false, true);

	return (0);
}
static int handle_qiecuo_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	QiecuoRequest *req = qiecuo_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint64_t target_id = req->player_id;
	qiecuo_request__free_unpacked(req, NULL);

	player_struct *target = player_manager::get_player_by_id(target_id);
	if (!target || !target->is_online())
	{
		send_comm_answer(MSG_ID_QIECUO_ANSWER, 1, extern_data);
		return (0);
	}

	if (player->is_in_qiecuo() || target->is_in_qiecuo())
	{
		send_comm_answer(MSG_ID_QIECUO_ANSWER, 190300028, extern_data);
		return (0);
	}

	//检查目标是否关闭了邀请
	if (target->data->qiecuo_invite_switch != 0)
	{
		send_comm_answer(MSG_ID_QIECUO_ANSWER, ERROR_ID_QIECUO_INVITE_CLOSE, extern_data);
		return (0);
	}

		// TODO: 检查地图是否允许切磋
		// TODO: 检查是否在安全区

	player->data->qiecuo_target = target_id;

	QiecuoNotify nty;
	qiecuo_notify__init(&nty);
	nty.name = player->get_name();
	nty.player_id = player->get_uuid();
	EXTERN_DATA ext;
	ext.player_id = target_id;
	fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_NOTIFY, qiecuo_notify__pack, nty);

	return (0);
}

static int handle_qiecuo_refuse_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	QiecuoRefuseRequest *req = qiecuo_refuse_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint64_t target_id = req->player_id;
	qiecuo_refuse_request__free_unpacked(req, NULL);
	player_struct *target = player_manager::get_player_by_id(target_id);
	if (!target || !target->is_online())
	{
		return (0);
	}
	if (target->is_in_qiecuo())
	{
		return (0);
	}
	if (target->data->qiecuo_target != player->get_uuid())
	{
		return (0);
	}
	QiecuoRefuseNotify nty;
	qiecuo_refuse_notify__init(&nty);
	nty.player_id = player->get_uuid();
	nty.name = player->get_name();
	EXTERN_DATA ext;
	ext.player_id = target_id;
	fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_REFUSE_NOTIFY, qiecuo_refuse_notify__pack, nty);
	return (0);
}

static int handle_qiecuo_start_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	QiecuoStartRequest *req = qiecuo_start_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint64_t target_id = req->player_id;
	qiecuo_start_request__free_unpacked(req, NULL);

	player_struct *target = player_manager::get_player_by_id(target_id);
	if (!target || !target->is_online())
	{
		send_comm_answer(MSG_ID_QIECUO_START_ANSWER, 1, extern_data);
		return (0);
	}

	if (player->is_in_qiecuo() || target->is_in_qiecuo())
	{
		send_comm_answer(MSG_ID_QIECUO_START_ANSWER, 2, extern_data);
		return (0);
	}
	if (target->data->qiecuo_target != player->get_uuid())
	{
		send_comm_answer(MSG_ID_QIECUO_START_ANSWER, 3, extern_data);
		return (0);
	}

	if (target->scene != player->scene)
	{
		send_comm_answer(MSG_ID_QIECUO_START_ANSWER, 4, extern_data);
		return (0);
	}

	if (player->get_attr(PLAYER_ATTR_PK_TYPE) != PK_TYPE_NORMAL
		|| target->get_attr(PLAYER_ATTR_PK_TYPE) != PK_TYPE_NORMAL)
	{
		send_comm_answer(MSG_ID_QIECUO_START_ANSWER, 190300029, extern_data);
		return (0);
	}

	// 检查安全区
	if (player->get_attr(PLAYER_ATTR_REGION_ID) == 11
		|| target->get_attr(PLAYER_ATTR_REGION_ID) == 11)
	{
		send_comm_answer(MSG_ID_QIECUO_START_ANSWER, 190300031, extern_data);
		return (0);
	}

		// TODO: 检查地图是否允许切磋
		// TODO: 检查距离

	struct position *player_pos = player->get_pos();
	struct position *target_pos = target->get_pos();
	uint32_t pos_x = (player_pos->pos_x - target_pos->pos_x) / 2 + target_pos->pos_x;
	uint32_t pos_z = (player_pos->pos_z - target_pos->pos_z) / 2 + target_pos->pos_z;
	player->set_qiecuo(pos_x, pos_z, target_id);
	target->set_qiecuo(pos_x, pos_z, player->get_uuid());

	QiecuoStartNotify nty;
	EXTERN_DATA ext;
	qiecuo_start_notify__init(&nty);
	nty.pos_x = pos_x;
	nty.pos_z = pos_z;

	nty.name = player->get_name();
	nty.player_id = player->get_uuid();
	ext.player_id = target_id;
	fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_START_NOTIFY, qiecuo_start_notify__pack, nty);

	nty.name = target->get_name();
	nty.player_id = target->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_QIECUO_START_NOTIFY, qiecuo_start_notify__pack, nty);

	return (0);
}

//处理玩家离线的缓存
static int handle_player_cache_answer(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);

	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PlayerOfflineCache *req = player_offline_cache__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	//处理离线的缓存
	for (size_t i = 0; i < req->n_sub_exps; ++i)
	{
		player->sub_exp(req->sub_exps[i]->val, req->sub_exps[i]->statis_id);
	}

	for (size_t i = 0; i < req->n_pvp_lose; ++i)
	{
		player->change_pvp_raid_score(req->pvp_lose[i]->type, req->pvp_lose[i]->score);
	}
	uint32_t today_sec = time_helper::get_cached_time() / 1000;
	for (size_t i = 0; i < req->n_pvp_win; ++i)
	{
		player->change_pvp_raid_score(req->pvp_win[i]->type, req->pvp_win[i]->score);
		//使用time字段来判断是否是今日的胜利
		if (!time_helper::is_same_day(req->pvp_win[i]->time, today_sec))
		{
			if (req->pvp_win[i]->type == PVP_TYPE_DEFINE_3)
				--player->data->pvp_raid_data.oneday_win_num_3;
			else
				--player->data->pvp_raid_data.oneday_win_num_5;
		}
	}

	player_offline_cache__free_unpacked(req, NULL);

	//通知redis清除缓存的数据
	fast_send_msg_base(&conn_node_gamesrv::connecter, extern_data, SERVER_PROTO_CLEAR_OFFLINE_CACHE, 0, get_seq());

	return 0;
}

static void send_pvp_match_start_answer(player_struct *player, EXTERN_DATA *extern_data, int result, uint64_t player_id, int cd)
{
	PvpMatchAnswer ans;
	pvp_match_answer__init(&ans);
	ans.playerid = player_id;
	ans.result = result;
	ans.cdtime = cd;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PVP_MATCH_START_ANSWER, pvp_match_answer__pack, ans);
}

static int handle_pvp_match_team_start_request(player_struct *player, EXTERN_DATA *extern_data, int type)
{	
		// 是不是队长
	if (player->m_team->GetLeadId() != player->get_uuid())
	{
		LOG_ERR("%s : player[%lu] not leader ", __FUNCTION__, player->get_uuid());
		return (-1);
	}

		// 是不是已经在队列中了
	if (pvp_match_is_team_in_waiting(player->m_team->GetId()))
	{
		LOG_ERR("%s : player[%lu] team already in waiting ", __FUNCTION__, player->get_uuid());
		send_pvp_match_start_answer(player, extern_data, 190500175, player->get_uuid(), 0);
		return (-10);
	}

	if (type == PVP_TYPE_DEFINE_3)
	{
		if (player->data->pvp_raid_data.level_3 == 1)
		{
			send_pvp_match_start_answer(player, extern_data, 190500108, player->get_uuid(), 0);
			return (-15);
		}
		
		// 队伍人数有没有超过
		if (player->m_team->GetMemberSize() != PVP_MATCH_PLAYER_NUM_3)
		{
			LOG_ERR("%s: player[%lu] team have too much member", __FUNCTION__, player->get_uuid());
			return (-20);
		}
		// 等级是否符合	以及是否在线
		uint8_t leader_lv = player->data->pvp_raid_data.level_3;
		int min_lv = leader_lv - MATCH_LEVEL_DIFF;
		int max_lv = leader_lv + MATCH_LEVEL_DIFF;

		for (int pos = 0; pos < player->m_team->m_data->m_memSize; ++pos)
		{
			if (player->m_team->m_data->m_mem[pos].timeremove != 0)
			{
				LOG_ERR("%s: player[%lu] team have player offline", __FUNCTION__, player->get_uuid());
				return (-30);
			}
			player_struct *t_player = player_manager::get_player_by_id(player->m_team->m_data->m_mem[pos].id);
			if (!t_player || !t_player->is_online())
			{
				LOG_ERR("%s: player[%lu] team have player offline", __FUNCTION__, player->get_uuid());
				return (-31);
			}
			int lv = t_player->data->pvp_raid_data.level_3;
			if (lv < min_lv || lv > max_lv)
			{
				LOG_ERR("%s: player[%lu] team have player lv[%d] not match", __FUNCTION__, player->get_uuid(), lv);
				return (-40);
			}

			if (pvp_match_is_player_in_waiting(t_player->get_uuid()))
			{
				LOG_ERR("[%s:%d] player[%lu] already in waiting", __FUNCTION__, __LINE__, t_player->get_uuid());
				send_pvp_match_start_answer(player, extern_data, 190500175, t_player->get_uuid(), 0);
				return (-50);
			}
			uint32_t cd = pvp_match_is_player_in_cd(t_player);
			if (cd > 0)
			{
				LOG_INFO("[%s:%d] player[%lu] in cd[%u]", __FUNCTION__, __LINE__, t_player->get_uuid(), cd);
				send_pvp_match_start_answer(player, extern_data, 190500103, t_player->get_uuid(), cd);
				return (-60);
			}
		}
	}
	else
	{
		if (player->data->pvp_raid_data.level_5 == 1)
		{
			send_pvp_match_start_answer(player, extern_data, 190500108, player->get_uuid(), 0);
			return (-15);
		}
		
		// 队伍人数有没有超过
		if (player->m_team->GetMemberSize() != PVP_MATCH_PLAYER_NUM_5)
		{
			LOG_ERR("%s: player[%lu] team have too much member", __FUNCTION__, player->get_uuid());
			return (-21);
		}
		// 等级是否符合	以及是否在线
		uint8_t leader_lv = player->data->pvp_raid_data.level_5;
		int min_lv = leader_lv - MATCH_LEVEL_DIFF;
		int max_lv = leader_lv + MATCH_LEVEL_DIFF;

		for (int pos = 0; pos < player->m_team->m_data->m_memSize; ++pos)
		{
			if (player->m_team->m_data->m_mem[pos].timeremove != 0)
			{
				LOG_ERR("%s: player[%lu] team have player offline", __FUNCTION__, player->get_uuid());
				return (-30);
			}
			player_struct *t_player = player_manager::get_player_by_id(player->m_team->m_data->m_mem[pos].id);
			if (!t_player || !t_player->is_online())
			{
				LOG_ERR("%s: player[%lu] team have player offline", __FUNCTION__, player->get_uuid());
				return (-31);
			}
			int lv = t_player->data->pvp_raid_data.level_5;
			if (lv < min_lv || lv > max_lv)
			{
				LOG_ERR("%s: player[%lu] team have player lv[%d] not match", __FUNCTION__, player->get_uuid(), lv);
				return (-40);
			}
			if (pvp_match_is_player_in_waiting(t_player->get_uuid()))
			{
				LOG_ERR("[%s:%d] player[%lu] already in waiting", __FUNCTION__, __LINE__, t_player->get_uuid());
				send_pvp_match_start_answer(player, extern_data, 190500175, t_player->get_uuid(), 0);
				return (-50);
			}
			uint32_t cd = pvp_match_is_player_in_cd(t_player);
			if (cd > 0)
			{
				LOG_INFO("[%s:%d] player[%lu] in cd[%u]", __FUNCTION__, __LINE__, t_player->get_uuid(), cd);
				send_pvp_match_start_answer(player, extern_data, 190500103, t_player->get_uuid(), cd);
				return (-60);
			}
		}
	}

	pvp_match_add_team_to_waiting(player, type);
	send_pvp_match_start_answer(player, extern_data, 0, 0, 0);
	return (0);
}

static int handle_pvp_match_start_request(player_struct *player, EXTERN_DATA *extern_data)
{
		// TODO: 增加一些检查，比如是否已经在副本中了等

	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PvpMatchRequest *req = pvp_match_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	int type = req->type;

	pvp_match_request__free_unpacked(req, NULL);

	uint32_t raid_id;
	if (type == PVP_TYPE_DEFINE_3)
	{
		raid_id = sg_3v3_pvp_raid_param1[0];
	}
	else
	{
		raid_id = sg_5v5_pvp_raid_param1[0];
	}
	if (raid_manager::check_player_enter_raid(player, raid_id) != 0)
	{
		LOG_INFO("%s: player[%lu] can not enter raid %u", __FUNCTION__, player->get_uuid(), raid_id);
		return (-15);
	}

	if (player->m_team)
	{
		return handle_pvp_match_team_start_request(player, extern_data, type);
	}

	if (type == PVP_TYPE_DEFINE_3)
	{
		if (player->data->pvp_raid_data.level_3 == 1)
		{
			send_pvp_match_start_answer(player, extern_data, 0, 0, 0);
			pvp_match_single_ai_player_3(player);
			return (0);
		}
	}
	else
	{
		if (player->data->pvp_raid_data.level_5 == 1)
		{
		}
	}

		//检查是否已经在队列中了
	if (pvp_match_is_player_in_waiting(player->get_uuid()))
	{
		LOG_ERR("[%s:%d] player[%lu] already in waiting", __FUNCTION__, __LINE__, extern_data->player_id);
		send_pvp_match_start_answer(player, extern_data, 190500175, player->get_uuid(), 0);
		return (-10);
	}

	uint32_t cd = pvp_match_is_player_in_cd(player);
	if (cd > 0)
	{
		LOG_INFO("[%s:%d] player[%lu] in cd[%u]", __FUNCTION__, __LINE__, player->get_uuid(), cd);
		send_pvp_match_start_answer(player, extern_data, 190500103, player->get_uuid(), cd);
		return (-20);
	}

	pvp_match_add_player_to_waiting(player, type);
	send_pvp_match_start_answer(player, extern_data, 0, 0, 0);
	return (0);
}

static int handle_pvp_raid_praise_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	PvpRaidPraiseRequest *req = pvp_raid_praise_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint64_t player_id = req->player_id;

	pvp_raid_praise_request__free_unpacked(req, NULL);

	pvp_match_player_praise(player, player_id);
	return (0);
}

static int handle_pvp_match_ready_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	pvp_match_player_set_ready(player);
	return (0);
}
static int handle_pvp_match_cancel_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	pvp_match_player_cancel(player);
	return (0);
}

// TODO: pvp排行榜 先不做
static int handle_pvp_rank_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	return (0);
}
static int handle_pvp_open_level_reward_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	PvpOpenLevelRewardRequest *req = pvp_open_level_reward_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-30);
	}
	uint32_t type = req->type;
	pvp_open_level_reward_request__free_unpacked(req, NULL);

	PvpOpenLevelRewardAnswer answer;
	pvp_open_level_reward_answer__init(&answer);
	answer.type = type;

	if (type == PVP_TYPE_DEFINE_3)
	{
		if (player->data->pvp_raid_data.cur_level_id_3 < player->data->pvp_raid_data.avaliable_reward_level_3)
		{
			LOG_ERR("%s: player[%lu] level not enough", __FUNCTION__, extern_data->player_id);
			return (-40);
		}
		struct StageTable *config = get_config_by_id(player->data->pvp_raid_data.avaliable_reward_level_3, &pvp_raid_config);
		if (!config)
		{
			LOG_ERR("%s: player[%lu] can not get reward [%u]", __FUNCTION__, extern_data->player_id, player->data->pvp_raid_data.avaliable_reward_level_3);
			return (-50);
		}
		++player->data->pvp_raid_data.avaliable_reward_level_3;
		answer.avaliable_reward_level = player->data->pvp_raid_data.avaliable_reward_level_3;
		player->give_drop_item(config->StageReward, MAGIC_TYPE_PVP_RAID_LEVEL_REWARD, ADD_ITEM_SEND_MAIL_WHEN_BAG_FULL);
	}
	else
	{
		if (player->data->pvp_raid_data.cur_level_id_5 < player->data->pvp_raid_data.avaliable_reward_level_5)
		{
			LOG_ERR("%s: player[%lu] level not enough", __FUNCTION__, extern_data->player_id);
			return (-40);
		}
		struct StageTable *config = get_config_by_id(player->data->pvp_raid_data.avaliable_reward_level_5, &pvp_raid_config);
		if (!config)
		{
			LOG_ERR("%s: player[%lu] can not get reward [%u]", __FUNCTION__, extern_data->player_id, player->data->pvp_raid_data.avaliable_reward_level_5);
			return (-50);
		}
		++player->data->pvp_raid_data.avaliable_reward_level_5;
		answer.avaliable_reward_level = player->data->pvp_raid_data.avaliable_reward_level_5;
		player->give_drop_item(config->StageReward, MAGIC_TYPE_PVP_RAID_LEVEL_REWARD, ADD_ITEM_SEND_MAIL_WHEN_BAG_FULL);
	}

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PVP_OPEN_LEVEL_REWARD_ANSWER, pvp_open_level_reward_answer__pack, answer);
	return (0);
}
static int handle_pvp_open_daily_box_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	PvpOpenDailyBoxRequest *req = pvp_open_daily_box_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("%s %d: can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-30);
	}
	uint32_t box_id = req->box_id;
	uint32_t type = req->type;
	pvp_open_daily_box_request__free_unpacked(req, NULL);
	bool found = false;

	PvpOpenDailyBoxAnswer answer;
	pvp_open_daily_box_answer__init(&answer);
	answer.result = 0;
	answer.type = type;
	uint32_t box[MAX_ONEDAY_PVP_BOX];
	answer.avaliable_box_id = box;

	if (type == PVP_TYPE_DEFINE_3)
	{
		if (player->data->pvp_raid_data.oneday_win_num_3 <= (box_id))
		{
			LOG_ERR("%s: player[%lu] [%u][%u]win num3 not enough", __FUNCTION__, extern_data->player_id,
				player->data->pvp_raid_data.oneday_win_num_3, box_id);
			return (-10);
		}
		for (int i = 0; i < MAX_ONEDAY_PVP_BOX; ++i)
		{
			if (player->data->pvp_raid_data.avaliable_box_3[i] == box_id)
			{
				player->data->pvp_raid_data.avaliable_box_3[i] = -1;
				found = true;
				break;
			}
		}
		if (!found)
		{
			LOG_ERR("%s: player[%lu] already get box %u", __FUNCTION__, extern_data->player_id, box_id);
			return (-20);
		}
		struct StageTable *config = get_config_by_id(player->data->pvp_raid_data.cur_level_id_3, &pvp_raid_config);
		if (!config)
		{
			LOG_ERR("%s: player[%lu] CAN NOT get config  %u", __FUNCTION__, extern_data->player_id, player->data->pvp_raid_data.cur_level_id_3);
			return (-30);
		}
		if (box_id >= config->n_VictoryReward3)
		{
			LOG_ERR("%s: player[%lu] box id no config %u", __FUNCTION__, extern_data->player_id, box_id);
			return (-40);
		}
		player->give_drop_item(config->VictoryReward3[box_id], MAGIC_TYPE_PVP_RAID_DAILY_REWARD, ADD_ITEM_SEND_MAIL_WHEN_BAG_FULL);
		for (int i = 0; i < MAX_ONEDAY_PVP_BOX; ++i)
		{
			if (player->data->pvp_raid_data.avaliable_box_3[i] == (uint8_t)-1)
				continue;
			box[answer.n_avaliable_box_id++] = player->data->pvp_raid_data.avaliable_box_3[i];
		}

	}
	else
	{
		if (player->data->pvp_raid_data.oneday_win_num_5 <= (box_id))
		{
			LOG_ERR("%s: player[%lu] win num5 not enough", __FUNCTION__, extern_data->player_id);
			return (-10);
		}
		for (int i = 0; i < MAX_ONEDAY_PVP_BOX; ++i)
		{
			if (player->data->pvp_raid_data.avaliable_box_5[i] == box_id)
			{
				player->data->pvp_raid_data.avaliable_box_5[i] = -1;
				found = true;
				break;
			}
		}
		if (!found)
		{
			LOG_ERR("%s: player[%lu] already get box %u", __FUNCTION__, extern_data->player_id, box_id);
			return (-20);
		}
		struct StageTable *config = get_config_by_id(player->data->pvp_raid_data.cur_level_id_5, &pvp_raid_config);
		if (!config)
		{
			LOG_ERR("%s: player[%lu] CAN NOT get config  %u", __FUNCTION__, extern_data->player_id, player->data->pvp_raid_data.cur_level_id_5);
			return (-30);
		}
		if (box_id >= config->n_VictoryReward5)
		{
			LOG_ERR("%s: player[%lu] box id no config %u", __FUNCTION__, extern_data->player_id, box_id);
			return (-40);
		}
		player->give_drop_item(config->VictoryReward5[box_id], MAGIC_TYPE_PVP_RAID_DAILY_REWARD, ADD_ITEM_SEND_MAIL_WHEN_BAG_FULL);
		for (int i = 0; i < MAX_ONEDAY_PVP_BOX; ++i)
		{
			if (player->data->pvp_raid_data.avaliable_box_5[i] == (uint8_t)-1)
				continue;
			box[answer.n_avaliable_box_id++] = player->data->pvp_raid_data.avaliable_box_5[i];
		}

	}
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PVP_OPEN_DAILY_BOX_ANSWER, pvp_open_daily_box_answer__pack, answer);
	return (0);
}

//获取八卦牌信息请求
static int handle_baguapai_info_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	return notify_baguapai_info(player, extern_data);
}

static int notify_baguapai_info(player_struct *player, EXTERN_DATA *extern_data)
{
	BaguapaiInfoAnswer resp;
	baguapai_info_answer__init(&resp);

	BaguapaiDressData dress_data[MAX_BAGUAPAI_STYLE_NUM];
	BaguapaiDressData* dress_data_point[MAX_BAGUAPAI_STYLE_NUM];
	BaguapaiCardData card_data[MAX_BAGUAPAI_STYLE_NUM][MAX_BAGUAPAI_DRESS_NUM];
	BaguapaiCardData* card_data_point[MAX_BAGUAPAI_STYLE_NUM][MAX_BAGUAPAI_DRESS_NUM];
	AttrData attr_data[MAX_BAGUAPAI_STYLE_NUM][MAX_BAGUAPAI_DRESS_NUM][MAX_BAGUAPAI_MINOR_ATTR_NUM];
	AttrData* attr_data_point[MAX_BAGUAPAI_STYLE_NUM][MAX_BAGUAPAI_DRESS_NUM][MAX_BAGUAPAI_MINOR_ATTR_NUM];
	AttrData attr_new_data[MAX_BAGUAPAI_STYLE_NUM][MAX_BAGUAPAI_DRESS_NUM][MAX_BAGUAPAI_MINOR_ATTR_NUM];
	AttrData* attr_new_data_point[MAX_BAGUAPAI_STYLE_NUM][MAX_BAGUAPAI_DRESS_NUM][MAX_BAGUAPAI_MINOR_ATTR_NUM];

	uint32_t style_num = 0;
	for (int i = 0; i < MAX_BAGUAPAI_STYLE_NUM; ++i)
	{
		dress_data_point[style_num] = &dress_data[style_num];
		baguapai_dress_data__init(&dress_data[style_num]);
		dress_data[style_num].styleid = i + 1;

		uint32_t card_num = 0;
		for (int j = 0; j < MAX_BAGUAPAI_DRESS_NUM; ++j)
		{
			card_data_point[style_num][card_num] = &card_data[style_num][card_num];
			baguapai_card_data__init(&card_data[style_num][card_num]);
			card_data[style_num][card_num].id = player->data->baguapai_dress[i].card_list[j].id;
			card_data[style_num][card_num].star = player->data->baguapai_dress[i].card_list[j].star;
			card_data[style_num][card_num].mainattrval = player->data->baguapai_dress[i].card_list[j].main_attr_val;
			card_data[style_num][card_num].mainattrvalnew = player->data->baguapai_dress[i].card_list[j].main_attr_val_new;

			uint32_t cur_num = 0;
			for (int k = 0; k < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++k)
			{
				if (player->data->baguapai_dress[i].card_list[j].minor_attrs[k].id == 0)
				{
					break;
				}

				attr_data_point[style_num][card_num][cur_num] = &attr_data[style_num][card_num][cur_num];
				attr_data__init(&attr_data[style_num][card_num][cur_num]);
				attr_data[style_num][card_num][cur_num].id = player->data->baguapai_dress[i].card_list[j].minor_attrs[k].id;
				attr_data[style_num][card_num][cur_num].val = player->data->baguapai_dress[i].card_list[j].minor_attrs[k].val;
				cur_num++;
			}
			card_data[style_num][card_num].n_minorattrs = cur_num;
			card_data[style_num][card_num].minorattrs = attr_data_point[style_num][card_num];

			uint32_t new_num = 0;
			for (int k = 0; k < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++k)
			{
				if (player->data->baguapai_dress[i].card_list[j].minor_attrs_new[k].id == 0)
				{
					break;
				}

				attr_new_data_point[style_num][card_num][new_num] = &attr_new_data[style_num][card_num][new_num];
				attr_data__init(&attr_new_data[style_num][card_num][new_num]);
				attr_new_data[style_num][card_num][new_num].id = player->data->baguapai_dress[i].card_list[j].minor_attrs_new[k].id;
				attr_new_data[style_num][card_num][new_num].val = player->data->baguapai_dress[i].card_list[j].minor_attrs_new[k].val;
				new_num++;
			}
			card_data[style_num][card_num].n_minorattrsnew = new_num;
			card_data[style_num][card_num].minorattrsnew = attr_new_data_point[style_num][card_num];

			card_num++;
		}
		dress_data[style_num].n_cards = card_num;
		dress_data[style_num].cards = card_data_point[style_num];

		style_num++;
	}
	resp.n_datas = style_num;
	resp.datas = dress_data_point;

	resp.result = 0;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_BAGUAPAI_INFO_ANSWER, baguapai_info_answer__pack, resp);

	return (0);
}

//八卦牌切换请求
static int handle_baguapai_switch_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	BaguapaiSwitchRequest *req = baguapai_switch_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t style_id = req->styleid;

	baguapai_switch_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		if (style_id > MAX_BAGUAPAI_STYLE_NUM)
		{
			ret = ERROR_ID_BAGUAPAI_STYLE_ID;
			LOG_ERR("[%s:%d] player[%lu] style_id error, style_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id);
			break;
		}

		uint32_t cur_style = player->get_attr(PLAYER_ATTR_BAGUA);
		if (style_id != cur_style)
		{
			player->data->attrData[PLAYER_ATTR_BAGUA] = style_id;
			AttrMap attrs;
			attrs[PLAYER_ATTR_BAGUA] = player->get_attr(PLAYER_ATTR_BAGUA);
			player->notify_attr(attrs);
			player->calculate_attribute(true);
		}
	} while(0);

	BaguapaiSwitchAnswer resp;
	baguapai_switch_answer__init(&resp);

	resp.result = ret;
	resp.styleid = style_id;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_BAGUAPAI_SWITCH_ANSWER, baguapai_switch_answer__pack, resp);

	return 0;
}

//八卦牌更换请求
static int handle_baguapai_wear_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	BaguapaiWearRequest *req = baguapai_wear_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t style_id = req->styleid;
	std::vector<uint32_t> grid_list;
	std::set<uint32_t> tmp_set;
	for (size_t i = 0; i < req->n_gridids; ++i)
	{
		grid_list.push_back(req->gridids[i]);
		tmp_set.insert(req->gridids[i]);
	}

	baguapai_wear_request__free_unpacked(req, NULL);

	int ret = 0;
	BaguapaiDressInfo *dress_info = NULL;
	do
	{
		//背包格子是否重复
		if (grid_list.size() != tmp_set.size())
		{
			ret = ERROR_ID_BAGUAPAI_BAG_POS;
			LOG_ERR("[%s:%d] player[%lu] bag pos repeated, style_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id);
			break;
		}

		//一次穿戴数量是否合法
		uint32_t wear_num = grid_list.size();
		if (wear_num == 0 || wear_num > MAX_BAGUAPAI_DRESS_NUM)
		{
			ret = ERROR_ID_BAGUAPAI_BAG_POS;
			LOG_ERR("[%s:%d] player[%lu] wear num error, style_id:%u, wear_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, wear_num);
			break;
		}

		std::set<uint32_t> part_set;
		std::map<uint32_t, bag_grid_data *> grid_datas;
		uint32_t player_lv = player->get_attr(PLAYER_ATTR_LEVEL);
		uint32_t suit_id = 0;
		for (std::vector<uint32_t>::iterator iter = grid_list.begin(); iter != grid_list.end(); ++iter)
		{
			uint32_t pos = *iter;
			if (pos > player->data->bag_grid_num)
			{
				ret = ERROR_ID_BAGUAPAI_BAG_POS;
				LOG_ERR("[%s:%d] player[%lu] bag pos invalid, style_id:%u, pos:%u, bag_size:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, pos, player->data->bag_grid_num);
				break;
			}

			bag_grid_data *grid = &player->data->bag[pos];
			uint32_t card_id = bagua_item_to_card(grid->id);
			BaguaTable *card_config = get_config_by_id(card_id, &bagua_config);
			if (!card_config)
			{
				ret = ERROR_ID_BAGUAPAI_BAG_POS;
				LOG_ERR("[%s:%d] player[%lu] get card config failed, style_id:%u, pos:%u, item_id:%u, card_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, pos, grid->id, card_id);
				break;
			}

			//是否是同一套
			if ((uint32_t)card_config->BaguaType != style_id)
			{
				ret = ERROR_ID_BAGUAPAI_BAG_POS;
				LOG_ERR("[%s:%d] player[%lu] card style error, style_id:%u, pos:%u, item_id:%u, card_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, pos, grid->id, card_id);
				break;
			}

			//是否是一个套装的
			if (suit_id == 0)
			{
				suit_id = card_config->Suit;
			}
			else if ((uint32_t)card_config->Suit != suit_id)
			{
				ret = ERROR_ID_BAGUAPAI_BAG_POS;
				LOG_ERR("[%s:%d] player[%lu] not suit, style_id:%u, pos:%u, item_id:%u, card_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, pos, grid->id, card_id);
				break;
			}

			//等级是否达到
			uint32_t need_lv = (uint32_t)card_config->Level;
			if (player_lv < need_lv)
			{
				ret = ERROR_ID_LEVEL_NOT_ENOUGH;
				LOG_ERR("[%s:%d] player[%lu] level, style_id:%u, pos:%u, item_id:%u, card_id:%u, need_lv:%u, player_lv:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, pos, grid->id, card_id, need_lv, player_lv);
				break;
			}

			//部位是否合法
			uint32_t part_id = (uint32_t)card_config->BaguaPosition;
			if (part_id > MAX_BAGUAPAI_DRESS_NUM)
			{
				ret = ERROR_ID_BAGUAPAI_BAG_POS;
				LOG_ERR("[%s:%d] player[%lu] part error, style_id:%u, pos:%u, item_id:%u, card_id:%u, part:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, pos, grid->id, card_id, part_id);
				break;
			}

			part_set.insert(card_config->BaguaPosition);
			grid_datas[part_id] = grid;
		}

		if (ret != 0)
		{
			break;
		}

		//部位是否有重复
		if (part_set.size() != (size_t)wear_num)
		{
			ret = ERROR_ID_BAGUAPAI_BAG_POS;
			LOG_ERR("[%s:%d] player[%lu] part repeated, style_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id);
			break;
		}

		dress_info = player->get_baguapai_dress(style_id);
		if (!dress_info)
		{
			ret = ERROR_ID_BAGUAPAI_STYLE_ID;
			LOG_ERR("[%s:%d] player[%lu] has not the style, style_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id);
			break;
		}

		std::vector<BaguapaiCardInfo> card_caches;
		for (std::map<uint32_t, bag_grid_data *>::iterator iter = grid_datas.begin(); iter != grid_datas.end(); ++iter)
		{
			uint32_t part_id = iter->first;
			bag_grid_data *grid = iter->second;
			uint32_t card_id = bagua_item_to_card(grid->id);
			BaguapaiCardInfo *card_info = &dress_info->card_list[part_id - 1];
			card_caches.push_back(*card_info);
			memset(card_info, 0, sizeof(BaguapaiCardInfo));
			card_info->id = card_id;
			card_info->star = grid->especial_item.baguapai.star;
			card_info->main_attr_val = grid->especial_item.baguapai.main_attr_val;
			memcpy(card_info->minor_attrs, grid->especial_item.baguapai.minor_attrs, sizeof(card_info->minor_attrs));
			player->add_task_progress(TCT_BAGUA_WEAR, card_id, 1);
		}

		//把装备的卡牌从背包删除，由于不是消耗，此处不记录流水
		for (std::vector<uint32_t>::iterator iter = grid_list.begin(); iter != grid_list.end(); ++iter)
		{
			player->del_item_grid(*iter, true);
		}

		//把替换下来的卡牌放回背包
		for (std::vector<BaguapaiCardInfo>::iterator iter = card_caches.begin(); iter != card_caches.end(); ++iter)
		{
			if (iter->id == 0)
			{
				continue;
			}

			player->move_baguapai_to_bag(*iter);
		}

		player->calculate_attribute(true);
	} while(0);

	BaguapaiWearAnswer resp;
	baguapai_wear_answer__init(&resp);

	BaguapaiDressData dress_data;
	baguapai_dress_data__init(&dress_data);

	BaguapaiCardData card_data[MAX_BAGUAPAI_DRESS_NUM];
	BaguapaiCardData* card_data_point[MAX_BAGUAPAI_DRESS_NUM];
	AttrData attr_data[MAX_BAGUAPAI_DRESS_NUM][MAX_BAGUAPAI_MINOR_ATTR_NUM];
	AttrData* attr_data_point[MAX_BAGUAPAI_DRESS_NUM][MAX_BAGUAPAI_MINOR_ATTR_NUM];
	AttrData attr_new_data[MAX_BAGUAPAI_DRESS_NUM][MAX_BAGUAPAI_MINOR_ATTR_NUM];
	AttrData* attr_new_data_point[MAX_BAGUAPAI_DRESS_NUM][MAX_BAGUAPAI_MINOR_ATTR_NUM];

	resp.result = ret;
	if (dress_info)
	{
		resp.datas = &dress_data;
		dress_data.styleid = style_id;
		uint32_t card_num = 0;
		for (int j = 0; j < MAX_BAGUAPAI_DRESS_NUM; ++j)
		{
			card_data_point[card_num] = &card_data[card_num];
			baguapai_card_data__init(&card_data[card_num]);
			card_data[card_num].id = dress_info->card_list[j].id;
			card_data[card_num].star = dress_info->card_list[j].star;
			card_data[card_num].mainattrval = dress_info->card_list[j].main_attr_val;
			card_data[card_num].mainattrvalnew = dress_info->card_list[j].main_attr_val_new;

			uint32_t cur_num = 0;
			for (int k = 0; k < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++k)
			{
				if (dress_info->card_list[j].minor_attrs[k].id == 0)
				{
					break;
				}

				attr_data_point[card_num][cur_num] = &attr_data[card_num][cur_num];
				attr_data__init(&attr_data[card_num][cur_num]);
				attr_data[card_num][cur_num].id = dress_info->card_list[j].minor_attrs[k].id;
				attr_data[card_num][cur_num].val = dress_info->card_list[j].minor_attrs[k].val;
				cur_num++;
			}
			card_data[card_num].n_minorattrs = cur_num;
			card_data[card_num].minorattrs = attr_data_point[card_num];

			uint32_t new_num = 0;
			for (int k = 0; k < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++k)
			{
				if (dress_info->card_list[j].minor_attrs_new[k].id == 0)
				{
					break;
				}

				attr_new_data_point[card_num][new_num] = &attr_new_data[card_num][new_num];
				attr_data__init(&attr_new_data[card_num][new_num]);
				attr_new_data[card_num][new_num].id = dress_info->card_list[j].minor_attrs_new[k].id;
				attr_new_data[card_num][new_num].val = dress_info->card_list[j].minor_attrs_new[k].val;
				new_num++;
			}
			card_data[card_num].n_minorattrsnew = new_num;
			card_data[card_num].minorattrsnew = attr_new_data_point[card_num];

			card_num++;
		}
		dress_data.n_cards = card_num;
		dress_data.cards = card_data_point;
	}
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_BAGUAPAI_WEAR_ANSWER, baguapai_wear_answer__pack, resp);

	return 0;
}

//八卦牌分解请求
static int handle_baguapai_decompose_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	BaguapaiDecomposeRequest *req = baguapai_decompose_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	std::set<uint32_t> grid_ids;
	for (size_t i = 0; i < req->n_gridids; ++i)
	{
		grid_ids.insert(req->gridids[i]);
	}

	baguapai_decompose_request__free_unpacked(req, NULL);

	int ret = 0;
	std::map<uint32_t, uint32_t> gain_map;
	do
	{
		for (std::set<uint32_t>::iterator iter = grid_ids.begin(); iter != grid_ids.end(); ++iter)
		{
			uint32_t pos = *iter;
			if (pos > player->data->bag_grid_num)
			{
				ret = ERROR_ID_BAGUAPAI_BAG_POS;
				LOG_ERR("[%s:%d] player[%lu] bag pos invalid, pos:%u, bag_size:%u", __FUNCTION__, __LINE__, extern_data->player_id, pos, player->data->bag_grid_num);
				break;
			}

			bag_grid_data *grid = &player->data->bag[pos];
			uint32_t card_id = bagua_item_to_card(grid->id);
			BaguaTable *card_config = get_config_by_id(card_id, &bagua_config);
			if (!card_config)
			{
				ret = ERROR_ID_BAGUAPAI_BAG_POS;
				LOG_ERR("[%s:%d] player[%lu] get card config failed, pos:%u, item_id:%u, card_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, pos, grid->id, card_id);
				break;
			}

			for (size_t i = 0; i < card_config->n_DecomposeItem; ++i)
			{
				uint32_t item_id = card_config->DecomposeItem[i];
				uint32_t item_num = card_config->DecomposeNum[i];
				gain_map[item_id] += item_num * grid->num;
			}

			if (grid->especial_item.baguapai.star > 0)
			{
				BaguaStarTable *star_config = get_bagua_star_config(grid->especial_item.baguapai.star);
				if (!star_config)
				{
					ret = ERROR_ID_NO_CONFIG;
					LOG_ERR("[%s:%d] player[%lu] get star config failed, pos:%u, item_id:%u, star:%u", __FUNCTION__, __LINE__, extern_data->player_id, pos, grid->id, grid->especial_item.baguapai.star);
					break;
				}

				for (size_t i = 0; i < star_config->n_DecomposeCompensation; ++i)
				{
					uint32_t item_id = star_config->DecomposeCompensation[i];
					uint32_t item_num = star_config->DecomposeCompensationNum[i];
					gain_map[item_id] += item_num * grid->num;
				}
			}
		}

		if (ret != 0)
		{
			break;
		}

		if (!player->check_can_add_item_list(gain_map))
		{
			ret = ERROR_ID_BAG_NOT_ABLE_BAGUA_DECOMPOSE;
			LOG_ERR("[%s:%d] player[%lu] bag not enough", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		//删除背包里的卡牌
		for (std::set<uint32_t>::iterator iter = grid_ids.begin(); iter != grid_ids.end(); ++iter)
		{
			uint32_t pos = *iter;
			bag_grid_data *grid = &player->data->bag[pos];
			player->del_item_by_pos(pos, grid->num, MAGIC_TYPE_BAGUAPAI_DECOMPOSE);
		}

		//返还材料到背包
		player->add_item_list(gain_map, MAGIC_TYPE_BAGUAPAI_DECOMPOSE);
	} while(0);

	BaguapaiDecomposeAnswer resp;
	baguapai_decompose_answer__init(&resp);

	ItemData gain_item[3];
	ItemData* gain_item_point[3];

	resp.result = ret;
	resp.n_items = 0;
	resp.items = gain_item_point;
	for (std::map<uint32_t, uint32_t>::iterator iter = gain_map.begin(); iter != gain_map.end(); ++iter)
	{
		gain_item_point[resp.n_items] = &gain_item[resp.n_items];
		item_data__init(&gain_item[resp.n_items]);
		gain_item[resp.n_items].id = iter->first;
		gain_item[resp.n_items].num = iter->second;
	}
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_BAGUAPAI_DECOMPOSE_ANSWER, baguapai_decompose_answer__pack, resp);

	return 0;
}

//八卦牌炼星请求
static int handle_baguapai_refine_star_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	BaguapaiRefineCommonRequest *req = baguapai_refine_common_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t style_id = req->styleid;
	uint32_t part_id = req->partid;

	baguapai_refine_common_request__free_unpacked(req, NULL);

	int ret = 0;
	BaguapaiCardInfo *card_info = NULL;
	do
	{
		card_info = player->get_baguapai_card(style_id, part_id);
		if (!card_info)
		{
			ret = ERROR_ID_BAGUAPAI_PART_ID;
			LOG_ERR("[%s:%d] player[%lu] get card info failed, style_id:%u, part_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, part_id);
			break;
		}

		uint32_t next_star = card_info->star + 1;
		BaguaStarTable *config = get_bagua_star_config(next_star);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get star config failed, style_id:%u, part_id:%u, star:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, part_id, next_star);
			break;
		}

		std::map<uint32_t, uint32_t> cost_map;
		for (size_t i = 0; i < config->n_StarItem; ++i)
		{
			uint32_t item_id = config->StarItem[i];
			uint32_t item_num = config->StarNum[i];
			cost_map[item_id] = item_num;
			uint32_t bag_num = player->get_item_can_use_num(item_id);
			if (bag_num < item_num)
			{
				ret = ERROR_ID_PROP_NOT_ENOUGH;
				LOG_ERR("[%s:%d] player[%lu] item not enough, style_id:%u, part_id:%u, item_id:%u, need_num:%u, has_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, part_id, item_id, item_num, bag_num);
				break;
			}
		}

		if (ret != 0)
		{
			break;
		}

		uint32_t has_coin = player->get_coin();
		uint32_t need_coin = config->StarCoin;
		if (has_coin < need_coin)
		{
			ret = ERROR_ID_COIN_IS_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] coin not enough, style_id:%u, part_id:%u, need_num:%u, has_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, part_id, need_coin, has_coin);
			break;
		}

		//先扣除消耗
		for (std::map<uint32_t, uint32_t>::iterator iter = cost_map.begin(); iter != cost_map.end(); ++iter)
		{
			player->del_item(iter->first, iter->second, MAGIC_TYPE_BAGUAPAI_REFINE_STAR);
		}
		player->sub_coin(need_coin, MAGIC_TYPE_BAGUAPAI_REFINE_STAR);

		uint32_t rand_val = rand() % 10000;
		if (rand_val < (uint32_t)config->StatProbability)
		{ //成功升星
			card_info->star++;
		}
		else
		{ //失败降星
			if (card_info->star > 0)
			{
				card_info->star--;
			}
		}

		player->calculate_attribute(true);
		player->add_task_progress(TCT_BAGUA_REFINE_STAR, 0, 1);
	} while(0);

	BaguapaiRefineStarAnswer resp;
	baguapai_refine_star_answer__init(&resp);

	resp.result = ret;
	resp.styleid = style_id;
	resp.partid = part_id;
	resp.star = (card_info ? card_info->star : 0);
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_BAGUAPAI_REFINE_STAR_ANSWER, baguapai_refine_star_answer__pack, resp);

	return 0;
}

//八卦牌重铸请求
static int handle_baguapai_refine_main_attr_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	BaguapaiRefineCommonRequest *req = baguapai_refine_common_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t style_id = req->styleid;
	uint32_t part_id = req->partid;

	baguapai_refine_common_request__free_unpacked(req, NULL);

	int ret = 0;
	BaguapaiCardInfo *card_info = NULL;
	do
	{
		card_info = player->get_baguapai_card(style_id, part_id);
		if (!card_info)
		{
			ret = ERROR_ID_BAGUAPAI_PART_ID;
			LOG_ERR("[%s:%d] player[%lu] get card info failed, style_id:%u, part_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, part_id);
			break;
		}

		BaguaTable *config = get_config_by_id(card_info->id, &bagua_config);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get card config failed, style_id:%u, part_id:%u, card_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, part_id, card_info->id);
			break;
		}

		std::map<uint32_t, uint32_t> cost_map;
		for (size_t i = 0; i < config->n_RecastItem; ++i)
		{
			uint32_t item_id = config->RecastItem[i];
			uint32_t item_num = config->RecastNum[i];
			cost_map[item_id] += item_num;
			uint32_t bag_num = player->get_item_can_use_num(item_id);
			if (bag_num < item_num)
			{
				ret = ERROR_ID_PROP_NOT_ENOUGH;
				LOG_ERR("[%s:%d] player[%lu] item not enough, style_id:%u, part_id:%u, item_id:%u, need_num:%u, has_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, part_id, item_id, item_num, bag_num);
				break;
			}
		}

		if (ret != 0)
		{
			break;
		}

		uint32_t need_coin = config->RecastCoin;
		uint32_t has_coin = player->get_coin();
		if (has_coin < need_coin)
		{
			ret = ERROR_ID_COIN_IS_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] coin not enough, style_id:%u, part_id:%u, need_num:%u, has_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, part_id, need_coin, has_coin);
			break;
		}

		double new_val = 0;
		if (player->generate_baguapai_main_attr(card_info->id, new_val) != 0)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] generate attr failed, style_id:%u, part_id:%u, card_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, part_id, card_info->id);
			break;
		}

		//先扣除消耗
		for (std::map<uint32_t, uint32_t>::iterator iter = cost_map.begin(); iter != cost_map.end(); ++iter)
		{
			player->del_item(iter->first, iter->second, MAGIC_TYPE_BAGUAPAI_REFINE_MAIN_ATTR);
		}
		player->sub_coin(need_coin, MAGIC_TYPE_BAGUAPAI_REFINE_MAIN_ATTR);

		card_info->main_attr_val_new = new_val;
		player->add_task_progress(TCT_BAGUA_REFINE_MAIN_ATTR, 0, 1);
	} while(0);

	BaguapaiRefineMainAttrAnswer resp;
	baguapai_refine_main_attr_answer__init(&resp);

	resp.result = ret;
	resp.styleid = style_id;
	resp.partid = part_id;
	if (card_info)
	{
		resp.mainattrvalnew = card_info->main_attr_val_new;
	}
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_BAGUAPAI_REFINE_MAIN_ATTR_ANSWER, baguapai_refine_main_attr_answer__pack, resp);

	return 0;
}

//八卦牌重铸保留请求
static int handle_baguapai_retain_main_attr_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	BaguapaiRefineCommonRequest *req = baguapai_refine_common_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t style_id = req->styleid;
	uint32_t part_id = req->partid;

	baguapai_refine_common_request__free_unpacked(req, NULL);

	int ret = 0;
	BaguapaiCardInfo *card_info = NULL;
	do
	{
		card_info = player->get_baguapai_card(style_id, part_id);
		if (!card_info)
		{
			ret = ERROR_ID_BAGUAPAI_PART_ID;
			LOG_ERR("[%s:%d] player[%lu] get card info failed, style_id:%u, part_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, part_id);
			break;
		}

		if (card_info->main_attr_val_new == 0)
		{
			ret = ERROR_ID_BAGUAPAI_NO_RETAIN_ATTR;
			LOG_ERR("[%s:%d] player[%lu] no retain attr, style_id:%u, part_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, part_id);
			break;
		}

		card_info->main_attr_val = card_info->main_attr_val_new;
		card_info->main_attr_val_new = 0;

		player->calculate_attribute(true);
	} while(0);

	BaguapaiRetainMainAttrAnswer resp;
	baguapai_retain_main_attr_answer__init(&resp);

	resp.result = ret;
	resp.styleid = style_id;
	resp.partid = part_id;
	if (card_info)
	{
		resp.mainattrval = card_info->main_attr_val;
		resp.mainattrvalnew = card_info->main_attr_val_new;
	}
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_BAGUAPAI_RETAIN_MAIN_ATTR_ANSWER, baguapai_retain_main_attr_answer__pack, resp);

	return 0;
}

//八卦牌洗炼请求
static int handle_baguapai_refine_minor_attr_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	BaguapaiRefineCommonRequest *req = baguapai_refine_common_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t style_id = req->styleid;
	uint32_t part_id = req->partid;

	baguapai_refine_common_request__free_unpacked(req, NULL);

	int ret = 0;
	BaguapaiCardInfo *card_info = NULL;
	do
	{
		card_info = player->get_baguapai_card(style_id, part_id);
		if (!card_info)
		{
			ret = ERROR_ID_BAGUAPAI_PART_ID;
			LOG_ERR("[%s:%d] player[%lu] get card info failed, style_id:%u, part_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, part_id);
			break;
		}

		BaguaTable *config = get_config_by_id(card_info->id, &bagua_config);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get card config failed, style_id:%u, part_id:%u, card_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, part_id, card_info->id);
			break;
		}

		std::map<uint32_t, uint32_t> cost_map;
		for (size_t i = 0; i < config->n_ClearItem; ++i)
		{
			uint32_t item_id = config->ClearItem[i];
			uint32_t item_num = config->ClearNum[i];
			cost_map[item_id] += item_num;
			uint32_t bag_num = player->get_item_can_use_num(item_id);
			if (bag_num < item_num)
			{
				ret = ERROR_ID_PROP_NOT_ENOUGH;
				LOG_ERR("[%s:%d] player[%lu] item not enough, style_id:%u, part_id:%u, item_id:%u, need_num:%u, has_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, part_id, item_id, item_num, bag_num);
				break;
			}
		}

		if (ret != 0)
		{
			break;
		}

		uint32_t need_coin = config->ClearCoin;
		uint32_t has_coin = player->get_coin();
		if (has_coin < need_coin)
		{
			ret = ERROR_ID_COIN_IS_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] coin not enough, style_id:%u, part_id:%u, need_num:%u, has_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, part_id, need_coin, has_coin);
			break;
		}

		AttrInfo attr_new[MAX_BAGUAPAI_MINOR_ATTR_NUM];
		memset(attr_new, 0, sizeof(attr_new));
		if (player->generate_baguapai_minor_attr(card_info->id, attr_new) != 0)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] generate attr failed, style_id:%u, part_id:%u, card_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, part_id, card_info->id);
			break;
		}

		//先扣除消耗
		for (std::map<uint32_t, uint32_t>::iterator iter = cost_map.begin(); iter != cost_map.end(); ++iter)
		{
			player->del_item(iter->first, iter->second, MAGIC_TYPE_BAGUAPAI_REFINE_MINOR_ATTR);
		}
		player->sub_coin(need_coin, MAGIC_TYPE_BAGUAPAI_REFINE_MINOR_ATTR);

		memset(card_info->minor_attrs_new, 0, sizeof(card_info->minor_attrs_new));
		memcpy(card_info->minor_attrs_new, attr_new, sizeof(card_info->minor_attrs_new));
		player->add_task_progress(TCT_BAGUA_REFINE_MINOR_ATTR, 0, 1);
	} while(0);

	BaguapaiRefineMinorAttrAnswer resp;
	baguapai_refine_minor_attr_answer__init(&resp);

	AttrData minor_new_attr[MAX_BAGUAPAI_MINOR_ATTR_NUM];
	AttrData* minor_new_attr_point[MAX_BAGUAPAI_MINOR_ATTR_NUM];

	resp.result = ret;
	resp.styleid = style_id;
	resp.partid = part_id;
	if (card_info)
	{
		uint32_t attr_num = 0;
		for (int i = 0; i < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++i)
		{
			if (card_info->minor_attrs_new[i].id == 0)
			{
				break;
			}

			minor_new_attr_point[i] = &minor_new_attr[i];
			attr_data__init(&minor_new_attr[i]);
			minor_new_attr[i].id = card_info->minor_attrs_new[i].id;
			minor_new_attr[i].val = card_info->minor_attrs_new[i].val;
			attr_num++;
		}
		resp.minorattrsnew = minor_new_attr_point;
		resp.n_minorattrsnew = attr_num;
	}
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_BAGUAPAI_REFINE_MINOR_ATTR_ANSWER, baguapai_refine_minor_attr_answer__pack, resp);

	return 0;
}

//八卦牌洗炼保留请求
static int handle_baguapai_retain_minor_attr_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	BaguapaiRefineCommonRequest *req = baguapai_refine_common_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t style_id = req->styleid;
	uint32_t part_id = req->partid;

	baguapai_refine_common_request__free_unpacked(req, NULL);

	int ret = 0;
	BaguapaiCardInfo *card_info = NULL;
	do
	{
		card_info = player->get_baguapai_card(style_id, part_id);
		if (!card_info)
		{
			ret = ERROR_ID_BAGUAPAI_PART_ID;
			LOG_ERR("[%s:%d] player[%lu] get card info failed, style_id:%u, part_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, part_id);
			break;
		}

		if (card_info->minor_attrs_new[0].id == 0)
		{
			ret = ERROR_ID_BAGUAPAI_NO_RETAIN_ATTR;
			LOG_ERR("[%s:%d] player[%lu] no retain attr, style_id:%u, part_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, style_id, part_id);
			break;
		}

		memset(card_info->minor_attrs, 0, sizeof(card_info->minor_attrs));
		memcpy(card_info->minor_attrs, card_info->minor_attrs_new, sizeof(card_info->minor_attrs));
		memset(card_info->minor_attrs_new, 0, sizeof(card_info->minor_attrs_new));

		player->calculate_attribute(true);
	} while(0);

	BaguapaiRetainMinorAttrAnswer resp;
	baguapai_retain_minor_attr_answer__init(&resp);

	AttrData minor_attr[MAX_BAGUAPAI_MINOR_ATTR_NUM];
	AttrData* minor_attr_point[MAX_BAGUAPAI_MINOR_ATTR_NUM];
	AttrData minor_new_attr[MAX_BAGUAPAI_MINOR_ATTR_NUM];
	AttrData* minor_new_attr_point[MAX_BAGUAPAI_MINOR_ATTR_NUM];

	resp.result = ret;
	resp.styleid = style_id;
	resp.partid = part_id;
	if (card_info)
	{
		int cur_num = 0;
		for (int i = 0; i < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++i)
		{
			if (card_info->minor_attrs[i].id == 0)
			{
				break;
			}

			minor_attr_point[i] = &minor_attr[i];
			attr_data__init(&minor_attr[i]);
			minor_attr[i].id = card_info->minor_attrs[i].id;
			minor_attr[i].val = card_info->minor_attrs[i].val;
			cur_num++;
		}
		resp.minorattrs = minor_attr_point;
		resp.n_minorattrs = cur_num;

		int new_num = 0;
		for (int i = 0; i < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++i)
		{
			if (card_info->minor_attrs_new[i].id == 0)
			{
				break;
			}

			minor_new_attr_point[i] = &minor_new_attr[i];
			attr_data__init(&minor_new_attr[i]);
			minor_new_attr[i].id = card_info->minor_attrs_new[i].id;
			minor_new_attr[i].val = card_info->minor_attrs_new[i].val;
			new_num++;
		}
		resp.minorattrsnew = minor_new_attr_point;
		resp.n_minorattrsnew = new_num;
	}
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_BAGUAPAI_RETAIN_MINOR_ATTR_ANSWER, baguapai_retain_minor_attr_answer__pack, resp);

	return 0;
}

//领取活跃度奖励请求
static int handle_active_reward_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	ActiveRewardRequest *req = active_reward_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint32_t reward_id = req->rewardid;

	active_reward_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		uint32_t *pReward = NULL;
		for (int i = 0; i < MAX_ACTIVE_REWARD_NUM; ++i)
		{
			uint32_t tmp_id = player->data->active_reward[i];
			if (tmp_id == 0 || tmp_id == reward_id)
			{
				pReward = &player->data->active_reward[i];
				break;
			}
		}

		if (!pReward)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] memory not enough, reward_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, reward_id);
			break;
		}

		if (*pReward > 0)
		{
			ret = ERROR_ID_ACTIVE_REWARD_HAS_GOT;
			LOG_ERR("[%s:%d] player[%lu] reward has got, reward_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, reward_id);
			break;
		}

		ActiveTable *config = get_config_by_id(reward_id, &activity_activeness_config);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get config failed, reward_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, reward_id);
			break;
		}

		uint32_t need_activeness = config->Active;
		uint32_t has_activeness = (uint32_t)player->get_attr(PLAYER_ATTR_ACTIVENESS);
		if (has_activeness < need_activeness)
		{
			ret = ERROR_ID_ACTIVENESS_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] activeness not enough, reward_id:%u, need_num:%u, has_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, reward_id, need_activeness, has_activeness);
			break;
		}

		//发放奖励
		for (size_t i = 0; i < config->n_Reward; ++i)
		{
			player->add_item(config->Reward[i], config->RewardNum[i], MAGIC_TYPE_ACTIVE_REWARD);
		}

		//标记已领
		*pReward = reward_id;
	} while(0);

	ActiveRewardAnswer resp;
	active_reward_answer__init(&resp);

	resp.result = ret;
	resp.rewardid = reward_id;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_ACTIVE_REWARD_ANSWER, active_reward_answer__pack, resp);

	return 0;
}




extern bool CheckGuoyuTaskOpen(int level, int &cd);

void send_cur_chengjie_task(player_struct *player, EXTERN_DATA *extern_data)
{
	if (player->data->chengjie.cur_task != 0)
	{
		STChengJie *pTask = ChengJieTaskManage::FindTask(player->data->chengjie.cur_task);
		if (pTask == NULL || (ChengJieTaskManage::GetTaskAccept(pTask->pid) != player->get_uuid() && !pTask->complete))
		{
			player->data->chengjie.cur_task = 0;
		}
		else
		{
			ChengjieTask send;
			chengjie_task__init(&send);
			if (ChengJieTaskManage::PackTask(*pTask, send))
			{
				conn_node_gamesrv::connecter.send_to_friend(extern_data, MSG_ID_CUR_CHENGJIE_TASK_NOTIFY, &send, (pack_func)chengjie_task__pack);
			}
			else
			{
				fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_CUR_CHENGJIE_TASK_NOTIFY, chengjie_task__pack, send);
			}
		}
	}
}
static int on_login_send_auto_add_hp_data(player_struct *player, EXTERN_DATA *extern_data)
{
	AutoAddHpSetData data;
	auto_add_hp_set_data__init(&data);
	data.auto_add_hp_item_id = player->data->auto_add_hp_item_id;
	data.auto_add_hp_percent = player->data->auto_add_hp_percent;
	data.open_auto_add_hp = player->data->open_auto_add_hp;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_AUTO_ADD_HP_SET_ANSWER, auto_add_hp_set_data__pack, data);

	player->send_hp_pool_changed_notify();
	return (0);
}

static int on_login_send_yaoshi(player_struct *player, EXTERN_DATA *extern_data)
{
	if (player->data->guoyu.guoyu_level == 0)
	{
//		player->data->next_update = time_helper::next_time_update();
		player->data->next_update = time_helper::nextOffsetTime(5 * 3600, time_helper::get_cached_time() / 1000);

		player->data->guoyu.guoyu_level = 1;
		player->data->guoyu.type = 1;
		player->data->guoyu.award = false;
		TypeLevelTable *table = get_guoyu_level_table(GUOYU__TASK__TYPE__CRITICAL);
		if (table != NULL)
		{
			player->data->guoyu.guoyu_num = table->RewardTime;
			player->data->guoyu.critical_num = table->ShowTimes;
		}

		player->data->chengjie.level = 1;
		player->data->change_special_num = 1;
		ParameterTable *param_config = get_config_by_id(161000070, &parameter_config);
		if (param_config != NULL)
		{
			player->data->chengjie.chengjie_num = param_config->parameter1[0];
		}

		param_config = get_config_by_id(161000106, &parameter_config);
		if (param_config != NULL)
		{
			player->data->shangjin.shangjin_num = param_config->parameter1[0];
		}
		player->data->shangjin.level = 1;
		ShangjinManage::RefreshTask(player);
		SpecialtySkillTable *tableSkill = get_yaoshi_skill_config(SHANGJIN_THREE, player->data->shangjin.level);
		if (tableSkill != NULL)
		{
			player->data->shangjin.free = tableSkill->EffectValue[0];
		}
		player->data->shangjin.free = 2;
	}

	Yaoshi send;
	yaoshi__init(&send);
	send.cur_major = player->data->cur_yaoshi;
	GuoyuType guoYu;
	guoyu_type__init(&guoYu);
	send.guoyu = &guoYu;
	send.change_num = player->data->change_special_num;
	guoYu.guoyu_level = player->data->guoyu.guoyu_level;
	guoYu.cur_exp = player->data->guoyu.cur_exp;
	guoYu.cur_task = player->data->guoyu.cur_task;
	guoYu.type = player->data->guoyu.type;
	guoYu.award = player->data->guoyu.award;
	//CheckGuoyuTaskOpen(GUOYU__TASK__TYPE__CRITICAL, player->data->guoyu.critical_cd);
	uint64_t now = time_helper::get_cached_time() / 1000;
	if (player->data->guoyu.task_timeout > now)
	{
		guoYu.task_cd = player->data->guoyu.task_timeout - now;
	}
	else
	{
		guoYu.task_cd = 0;
	}
	if (player->data->guoyu.critical_cd > now)
	{
		guoYu.critical_cd = player->data->guoyu.critical_cd - now;
	}
	else
	{
		guoYu.critical_cd = 0;
	}
	guoYu.critical_num = player->data->guoyu.critical_num;
	guoYu.guoyu_num = player->data->guoyu.guoyu_num;// = 999;
	guoYu.map = player->data->guoyu.map;

	ChengjieType chengJie;
	chengjie_type__init(&chengJie);
	send.chengjie = &chengJie;
	chengJie.level = player->data->chengjie.level;
	chengJie.num = player->data->chengjie.chengjie_num;
	chengJie.cur_exp = player->data->chengjie.cur_exp;

	ShangjinType shangJin;
	shangjin_type__init(&shangJin);
	send.shangjin = &shangJin;
	shangJin.free_refresh = player->data->shangjin.free;
	shangJin.level = player->data->shangjin.level;
	shangJin.num = player->data->shangjin.shangjin_num;
	shangJin.cur_exp = player->data->shangjin.cur_exp;
	shangJin.cur_task = player->data->shangjin.cur_task;
	shangJin.accept = player->data->shangjin.accept;
	ShangjinTaskType arrTask[MAX_SHANGJIN_NUM];
	ShangjinTaskType *arrTaskPoint[MAX_SHANGJIN_NUM] = {arrTask, arrTask + 1, arrTask + 2};
	shangJin.task_type = arrTaskPoint;
	shangJin.n_task_type = MAX_SHANGJIN_NUM;
	ShangjinTaskAward arrAward[MAX_SHANGJIN_NUM][MAX_SHANGJIN_AWARD_NUM];
	ShangjinTaskAward *arrAwardPoint[MAX_SHANGJIN_NUM][MAX_SHANGJIN_AWARD_NUM];
	for (int i = 0; i < MAX_SHANGJIN_NUM; ++i)
	{
		shangjin_task_type__init(arrTask + i);
		arrTask[i].id = player->data->shangjin.task[i].id;
		arrTask[i].quality = player->data->shangjin.task[i].quality;
		arrTask[i].n_award = player->data->shangjin.task[i].n_award;
		arrTask[i].reduce = player->data->shangjin.task[i].reduce;
		arrTask[i].coin = player->data->shangjin.task[i].coin;
		arrTask[i].exp = player->data->shangjin.task[i].exp;
		for (uint32_t j = 0; j < player->data->shangjin.task[i].n_award; ++j)
		{
			shangjin_task_award__init(&arrAward[i][j]);// + i * MAX_SHANGJIN_AWARD_NUM + j);
			arrAward[i][j].id = player->data->shangjin.task[i].award[j].id;
			arrAward[i][j].num = player->data->shangjin.task[i].award[j].val;
			arrAwardPoint[i][j] = &arrAward[i][j];// arrAward + i * j;
		}
		arrTask[i].award = arrAwardPoint[i];
	}

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_YAOSHI_NOTIFY, yaoshi__pack, send);

	send_cur_chengjie_task(player, extern_data);
	if (player->data->chengjie.target != 0)
	{
		player_struct *target = player_manager::get_player_by_id(player->data->chengjie.target);
		if (target != NULL)
		{
			ChengjieKiller send;
			chengjie_killer__init(&send);
			send.playerid = player->get_uuid();

			EXTERN_DATA extern_data;
			extern_data.player_id = target->get_uuid();
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_CHENGJIE_KILLER_NOTIFY, chengjie_killer__pack, send);
		}
	}

	return 0;
}
static int refresh_guoyu_task(player_struct *player, int type, bool isItem)
{
	EXTERN_DATA extern_data;
	RandomMonsterTable *table = get_random_monster_config(type, player->data->guoyu.guoyu_level);
	while (table == NULL)
	{
		//LOG_ERR("[%s:%d] can not get guoyu task table player[%lu] ", __FUNCTION__, __LINE__, player->get_uuid());
		//return 1;
		table = get_random_monster_config(type, player->data->guoyu.guoyu_level);
	}
	RandomDungeonTable *tableFb = get_random_guoyu_fb_config(type);
	if (tableFb == NULL)
	{
		LOG_ERR("[%s:%d] can not get guoyu task table player[%lu] ", __FUNCTION__, __LINE__, player->get_uuid());
		return 2;
	}
	if (player->m_team == NULL)
	{
		return 3;
	}
	for (int i = 0; i < player->m_team->m_data->m_memSize; ++i)
	{
		player_struct *mem = player_manager::get_player_by_id(player->m_team->m_data->m_mem[i].id);
		if (mem == NULL)
		{
			continue;
		}
		//if (mem->data->guoyu.cur_task != 0 && i > 0)
		//{
		//	continue;
		//}
		mem->data->guoyu.cur_task = table->ID;
		mem->data->guoyu.map = tableFb->ResID;
		mem->data->guoyu.random_map = tableFb->ID;
		mem->data->guoyu.type = type;
		mem->data->guoyu.task_timeout = time_helper::get_cached_time() / 1000 + table->Times;
		UpdateGuoyuTask send;
		update_guoyu_task__init(&send);
		send.id = mem->data->guoyu.cur_task;
		send.cd = table->Times;
		send.ret = 0;
		send.map = tableFb->ResID;
		send.type = type;
		send.refresh = isItem;
		extern_data.player_id = player->m_team->m_data->m_mem[i].id;	

		if (mem->data->guoyu.guoyu_num > 0 && !isItem)
		{
			--mem->data->guoyu.guoyu_num;
			if (mem->data->guoyu.type == GUOYU__TASK__TYPE__CRITICAL)
			{
				--mem->data->guoyu.critical_num;
			}
			YaoshiNumber notify1;
			yaoshi_number__init(&notify1);
			notify1.critical_num = mem->data->guoyu.critical_num;
			notify1.guoyu_num = mem->data->guoyu.guoyu_num;
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_YAOSHI_NUM_NOTIFY, yaoshi_number__pack, notify1);
			mem->data->guoyu.award = true;
		}
		send.award = mem->data->guoyu.award;
		
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_GUOYU_TASK_NOTIFY, update_guoyu_task__pack, send);
	}
	
	return 0;
}
static int CheckAcceptGuoyuTask(player_struct *player, int level, bool isItem, uint64_t *pid, uint32_t &n_pid)
{
	
	int tmp = 0;
	if (!CheckGuoyuTaskOpen(level, tmp))
	{
		return 190500104;
	}
	TypeLevelTable *table = get_guoyu_level_table(level);
	if (table == NULL)
	{
		return 1;
	}
	if (player->get_attr(PLAYER_ATTR_LEVEL) < table->Level)
	{
		pid[0] = player->get_uuid();
		n_pid = 1;
		return 190500107;
	}
	if (player->m_team == NULL)
	{
		return 190500119;
	}
	if (player->m_team->GetLeadId() != player->get_uuid())
	{
		return 190500105;
	}
	
	
	if (level == GUOYU__TASK__TYPE__CRITICAL)
	{
		if (player->data->guoyu.critical_cd <= time_helper::get_cached_time() / 1000)
		{
			return 190500104;
		}
	}

	std::vector<uint64_t> cl;
	std::vector<uint64_t> coff;
	std::vector<uint64_t> ctask;
	std::vector<uint64_t> cdis;
	std::vector<uint64_t>::iterator it;
	for (int i = 0; i < player->m_team->m_data->m_memSize; ++i)
	{
		uint64_t pid = player->m_team->m_data->m_mem[i].id;
		player_struct *mem = player_manager::get_player_by_id(pid);
		if (mem == NULL)
		{
			coff.push_back(pid);
			continue;
		}
		if (mem->data->guoyu.cur_task != 0 && i > 0)
		{
			ctask.push_back(pid);
		}
		if (mem->scene != player->scene)
		{
			cdis.push_back(pid);
		}
		if (abs(mem->get_pos()->pos_x - player->get_pos()->pos_x) > 20 || abs(mem->get_pos()->pos_z - player->get_pos()->pos_z) > 20)
		{
			cdis.push_back(pid);
		}
		if (mem->get_attr(PLAYER_ATTR_LEVEL) < table->Level)
		{
			cl.push_back(pid);
		}
	}
	int i = 0;
	if (coff.size() > 0)
	{
		for (it = coff.begin(),i = 0; it != coff.end(); ++it, ++i)
		{
			pid[i] = *it;
		}
		n_pid = coff.size();
		return 190500109;
	}
	if (cl.size() > 0)
	{
		for (it = cl.begin(), i = 0; it != cl.end(); ++it, ++i)
		{
			pid[i] = *it;
		}
		n_pid = cl.size();
		return 190500107;
	}
	if (cdis.size() > 0)
	{
		for (it = cdis.begin(), i = 0; it != cdis.end(); ++it, ++i)
		{
			pid[i] = *it;
		}
		n_pid = cdis.size();
		return 190500050;
	}
	if (ctask.size() > 0 && !isItem)
	{
		for (it = ctask.begin(), i = 0; it != ctask.end(); ++it, ++i)
		{
			pid[i] = *it;
		}
		n_pid = ctask.size();
		return 190500120;
	}

	if (isItem)
	{
		for (it = ctask.begin(), i = 0; it != ctask.end(); ++it)
		{
			player_struct *mem = player_manager::get_player_by_id(*it);
			if (mem == NULL)
			{
				return 190500109;
			}
			if (mem->data->guoyu.cur_task != player->data->guoyu.cur_task)
			{
				RandomMonsterTable *taskMy = get_config_by_id(player->data->guoyu.cur_task, &random_monster);
				RandomMonsterTable *taskOther = get_config_by_id(mem->data->guoyu.cur_task, &random_monster);
				if (taskOther == NULL || taskMy == NULL)
				{
					return 5;
				}
				if (taskMy->TypeLevel != taskOther->TypeLevel)
				{
					pid[i++] = *it;
				}
			}
		}
		if (i > 0)
		{
			n_pid = i;
			return 190500147;
		}
		ParameterTable *param_config = get_config_by_id(161000084, &parameter_config);
		if (param_config == NULL)
		{
			return 2;
		}
		if (player->del_item(param_config->parameter1[0], param_config->parameter1[1], MAGIC_TYPE_YAOSHI) < 0)
		{
			return 190500123;
		}
	}
	return 0;
}
static int handle_accect_guoyu_task_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	ReqChoseGuoyuTask *req = req_chose_guoyu_task__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	int type = req->type;
	bool refresh = req->refresh;
	req_chose_guoyu_task__free_unpacked(req, NULL);

	uint64_t pid[MAX_TEAM_MEM + 1];
	uint32_t n_pid = 0;
	int ret = CheckAcceptGuoyuTask(player, type, refresh, pid, n_pid);
	if (ret != 0)
	{
		UpdateGuoyuTask send;
		update_guoyu_task__init(&send);
		send.ret = ret;
		send.pid = pid;
		send.n_pid = n_pid;
		send.id = 0;
		send.cd = 0;
		send.map = 0;
		send.type = 0;
		fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_GUOYU_TASK_NOTIFY, update_guoyu_task__pack, send);
		return 1;
	}
	if (player->m_team != NULL && player->m_team->GetMemberSize() > 1)
	{
		ReqChoseGuoyuTask tmp;
		req_chose_guoyu_task__init(&tmp);
		player->m_team->m_data->m_agreed = 0;
		player->m_team->m_data->m_guoyuType = type;
		player->m_team->m_data->m_guoyuItem = refresh;
		player->m_team->BroadcastToTeam(MSG_ID_ACCECT_GUOYU_TASK_NOTIFY, &tmp, (pack_func)req_chose_guoyu_task__pack, player->get_uuid());
	}
	else
	{
		refresh_guoyu_task(player, type, refresh);
	}
	
	return (0);
}
static int handle_agreed_guoyu_task_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	if (player->m_team == NULL)
	{
		return -2;
	}
	GuoyuSucc *req = guoyu_succ__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	bool accept = req->succ;
	guoyu_succ__free_unpacked(req, NULL);

	if (accept)
	{
		++player->m_team->m_data->m_agreed;
		if (player->m_team->m_data->m_agreed == player->m_team->GetMemberSize() - 1)
		{
			refresh_guoyu_task(player, player->m_team->m_data->m_guoyuType, player->m_team->m_data->m_guoyuItem);
		}
	}
	else
	{
		GuoyuName notify;
		guoyu_name__init(&notify);
		notify.name = player->get_name();
		EXTERN_DATA ext_data;
		ext_data.player_id = player->m_team->GetLeadId();
		fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_REFUCE_GUOYU_TASK_NOTIFY, guoyu_name__pack, notify);
	}
	return 0;
}
static int handle_giveup_guoyu_task_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	player->data->guoyu.cur_task = 0;
	player->data->guoyu.award = false;
	GiveupGuoyuTask send;
	giveup_guoyu_task__init(&send);
	send.ret = 0;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_GIVEUP_GUOYU_TASK_ANSWER, giveup_guoyu_task__pack, send);
	//send_comm_answer(MSG_ID_GIVEUP_GUOYU_TASK_ANSWER, 0, extern_data);

	return (0);
}
static int handle_guoyu_boss_appear_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	if (player->m_team == NULL)
	{
		return -2;
	}

	BossId *req = boss_id__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	int bossId = req->id;
	boss_id__free_unpacked(req, NULL);

	if (player->data->scene_id < SCENCE_DEPART)
	{
		return -3;
	}

	raid_struct *raid = (raid_struct *)player->scene;
	if (!raid->data->ai_data.guoyu_data.note_boss)
	{
		BossId send;
		boss_id__init(&send);
		send.id = bossId;
		fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_GUOYU_BOSS_APPEAR_NOTIFY, boss_id__pack, send);
		raid->data->ai_data.guoyu_data.note_boss = true;
	}

	return (0);
}
static int handle_set_special_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	ReqSetYaoshiSpecial *req = req_set_yaoshi_special__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	int type = req->type;
	req_set_yaoshi_special__free_unpacked(req, NULL);

	uint32_t lv = player->data->change_special_num;
	if (lv > change_special_config.size())
	{
		lv = change_special_config.size();
	}
	AnsYaoshiSpecial send;
	ans_yaoshi_special__init(&send);
	send.ret = 0;
	ChangeSpecialty *table = get_change_special_table(lv);
	if (table == NULL)
	{
		send.ret = 1;
		goto done;
	}
	if (player->sub_comm_gold(table->CostValue, MAGIC_TYPE_YAOSHI) < 0)
	{
		send.ret = 190400005;
		goto done;
	}
	player->data->cur_yaoshi = type;
	++player->data->change_special_num;

done:
	send.type = player->data->cur_yaoshi;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_SET_SPECIAL_ANSWER, ans_yaoshi_special__pack, send);

	return (0);
}
/*
static int handle_enter_guoyu_fb_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	uint32_t raid_id = 20023;
	raid_struct *raid = raid_manager::create_raid(raid_id, player);
	if (!raid)
	{
		LOG_ERR("%s: player[%lu] create raid[%u] failed", __FUNCTION__, extern_data->player_id, raid_id);
		return (-20);
	}

	if (player->m_team)
	{
		//		player_team_enter_raid(player, raid);
		raid->team_enter_raid(player->m_team);
	}
	else
	{
		if (raid->player_enter_raid(player) != 0)
		{
			LOG_ERR("%s: player[%lu] enter raid failed", __FUNCTION__, player->get_uuid());
			return (-30);
		}
	}

	return (0);
}
*/
static int handle_chengjie_find_target_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	ReqFindTarget *req = req_find_target__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	player_struct * target = NULL;
	if (req->name != NULL)
	{
		AnsFindTarget send;
		ans_find_target__init(&send);
		send.ret = 0;
		if (strcmp(req->name, player->get_name()) == 0)
		{
			send.ret = 190500128;
		}
		uint64_t pid = strtoull(req->name, NULL, 10);
		if (pid == player->get_uuid())
		{
			send.ret = 190500128;
		}
		if (send.ret != 0)
		{
			fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_CHENGJIE_FIND_TARGET_ANSWER, ans_find_target__pack, send);
		}
		else
		{
			target = player_manager::get_player_by_id(pid);
			if (target == NULL)
			{
				PROTO_FIND_PLAYER_REQ *todb = (PROTO_FIND_PLAYER_REQ *)&conn_node_base::global_send_buf[0];
				strncpy(todb->name, req->name, 33);
				todb->head.msg_id = ENDION_FUNC_2(SERVER_PROTO_FIND_PLAYER_REQUEST);
				todb->head.len = ENDION_FUNC_4(sizeof(PROTO_FIND_PLAYER_REQ));

				conn_node_base::add_extern_data(&todb->head, extern_data);
				if (conn_node_dbsrv::connecter.send_one_msg(&todb->head, 1) != (int)ENDION_FUNC_4(todb->head.len)) {
					LOG_ERR("%s %d: send to dbsrv err[%d]", __FUNCTION__, __LINE__, errno);
				}
			}
			else
			{
				send.pid = target->get_uuid();
				send.name = target->get_name();
				send.lv = target->get_attr(PLAYER_ATTR_LEVEL);
				fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_CHENGJIE_FIND_TARGET_ANSWER, ans_find_target__pack, send);
				ChengJieTaskManage::AddRoleLevel(send.pid, send.lv, target->data->chengjie.rest);
			}

		}

	}
	req_find_target__free_unpacked(req, NULL);

	return (0);
}
static int handle_refresh_chengjie_list_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	ChengjieRefreshType *req = chengjie_refresh_type__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	ChengJieTaskManage::ClientGetTaskList(player, req->type);
	chengjie_refresh_type__free_unpacked(req, NULL);

	return (0);
}
int check_can_add_chengjie_task(player_struct *player, uint32_t shangjin, uint64_t target, uint32_t lv)
{
	if (ChengJieTaskManage::IsTarget(target))
	{
		return 190500127;
	}
	if (ChengJieTaskManage::GetRoleCd(target) > time_helper::get_cached_time() / 1000)
	{
		return 190500130;
	}
	if (player->get_uuid() == target)
	{
		return 190500128;
	}
	ParameterTable *param_config = get_config_by_id(161000088, &parameter_config);
	if (param_config != NULL && param_config->parameter1[0] > lv)
	{
		//ChengJieTaskManage::DelRoleLevel(target);
		return 190500129;
	}
	param_config = get_config_by_id(161000090, &parameter_config);
	if (param_config != NULL)
	{
		uint64_t cost = (param_config->parameter1[0] + 10000) * shangjin / 10000;
		if (player->sub_coin(cost, MAGIC_TYPE_YAOSHI) < 0)
		{
			return 190500133;
		}
	}
	return 0;
}

void send_system_msg(char * str, player_struct *player);

static int handle_add_chengjie_task_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	ReqAddChengjieTask *req = req_add_chengjie_task__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	static uint32_t tid = 0;

	uint32_t lv = ChengJieTaskManage::GetRoleLevel(req->playerid);


	STChengJie task;
	ParameterTable *param_config = get_config_by_id(161000096, &parameter_config);
	RewardTable * table = get_chengjie_reward_table(lv);
	AnsAddChengjieTask send;
	ans_add_chengjie_task__init(&send);

	if (lv == 0)
	{
		send.ret = 1;
		goto done;
	}
	if (req->step < 1 ||req->step > 3)
	{
		goto done;
	}
	send.ret = check_can_add_chengjie_task(player, req->shangjin, req->playerid, lv);
	if (send.ret != 0)
	{
		goto done;
	}
	task.id = ++tid;;
	task.pid = req->playerid;
	task.step = req->step;
	task.shuangjin = req->shangjin;
	task.timeOut = time_helper::get_cached_time() / 1000 + 3600 * 24 * 7;
	task.investor = player->get_uuid();
	task.anonymous = req->anonymous; 
	if (req->declaration != NULL && strnlen(req->declaration, 256) > 0)
	{
		strncpy(task.declaration, req->declaration, 256);
	}
	

	if (param_config != NULL)
	{
		task.timeOut = time_helper::get_cached_time() / 1000 + param_config->parameter1[0];
	}

	if (table != NULL)
	{
		task.exp = table->ExpCoefficient;
		task.courage = table->CoinReward;
	}
	//ChengJieTaskManage::AddTask(task);
	ChengJieTaskManage::AddTaskDb(task, extern_data);

	if (player->data->guild_id != 0)
	{
		char str[1024];
		ParameterTable * configSpeek = get_config_by_id(161000271, &parameter_config);
		sprintf(str, configSpeek->parameter2, player->get_name(), req->name); 
		player->send_chat(CHANNEL__family, str);
	}
done:
	req_add_chengjie_task__free_unpacked(req, NULL);
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_ADD_CHENGJIE_TASK_ANSWER, ans_add_chengjie_task__pack, send);

	return (0);
}
int CheckCanAcceptChengjieTask(player_struct *player, STChengJie * task)
{
	if (player->data->chengjie.cur_task != 0)
	{
		return 190500136;
	}
	if (player->get_uuid() == task->investor)
	{
		return 190500135;
	}
	if (player->get_uuid() == task->pid)
	{
		return 190500340;
	}
	if (task->complete)
	{
		return 190500138;
	}
	if (player->data->chengjie.chengjie_num < 1)
	{
		return 190500140;
	}
	if (task->timeOut < time_helper::get_cached_time() / 1000)
	{
		return 190500138;
	}
	if (task->taskTime > time_helper::get_cached_time() / 1000)
	{
		return 190500137;
	}
	if (task->acceptCd > time_helper::get_cached_time() / 1000)
	{
		return 190500137;
	}
	ParameterTable * param_config = get_config_by_id(161000091, &parameter_config);
	if (param_config != NULL)
	{
		uint64_t cost = param_config->parameter1[0] * task->shuangjin / 10000;
		if (player->sub_coin(cost, MAGIC_TYPE_YAOSHI) < 0)
		{
			return 190500139;
		}
	}
	return 0;
}
static int handle_accept_chengjie_task_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	ReqAcceptChengjieTask *req = req_accept_chengjie_task__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	AnsAcceptChengjieTask send;
	ans_accept_chengjie_task__init(&send);

	ChengjieKiller notify;
	EXTERN_DATA ext_data;
	player_struct *target = NULL;
	ParameterTable * param_config = get_config_by_id(161000097, &parameter_config);
	STChengJie * pTask = ChengJieTaskManage::FindTask(req->taskid);
	if (pTask == NULL)
	{
		send.ret = 190500138;
		goto done;
	}
	send.ret = CheckCanAcceptChengjieTask(player, pTask);
	if (send.ret != 0)
	{
		goto done;
	}

	player->data->chengjie.cur_task = req->taskid;
	player->data->chengjie.target = pTask->pid;
	if (param_config != NULL)
	{
		pTask->taskTime = time_helper::get_cached_time() / 1000 + param_config->parameter1[0];
		if (pTask->taskTime > pTask->timeOut)
		{
			pTask->taskTime = pTask->timeOut;
		}
	}
ChengJieTaskManage::ClientGetTaskList(player, 0);
	--player->data->chengjie.chengjie_num;
	ChengJieTaskManage::SetRoleTarget(pTask->pid, player->get_uuid());
	ChengJieTaskManage::UpdateTaskDb(*pTask);
	player->data->chengjie.first_hit = false;
	target = player_manager::get_player_by_id(pTask->pid);
	if (target != NULL)
	{
		chengjie_killer__init(&notify);
		notify.playerid = player->get_uuid();
		ext_data.player_id = target->get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_CHENGJIE_KILLER_NOTIFY, chengjie_killer__pack, notify);
	}

	send_cur_chengjie_task(player, extern_data);
	player->check_activity_progress(AM_YAOSHI, 2);
	

done:
	send.taskid = req->taskid;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_ACCEPT_CHENGJIE_TASK_ANSWER, ans_accept_chengjie_task__pack, send);

	req_accept_chengjie_task__free_unpacked(req, NULL);

	return (0);
}
static int handle_submit_chengjie_task_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	uint64_t cost = 0;
	ParameterTable * param_config = get_config_by_id(161000091, &parameter_config);
	ParameterTable * config = get_config_by_id(161000087, &parameter_config);
	STChengJie * pTask = ChengJieTaskManage::FindTask(player->data->chengjie.cur_task);
	SpecialtySkillTable *tableSkill = get_yaoshi_skill_config(CHENGJIE_EIGHT, player->data->chengjie.level);
	uint32_t expadd = 0;
	if (player->data->cur_yaoshi == MAJOR__TYPE__CHENGJIE)
	{
		SpecialTitleTable *title = get_yaoshi_title_table(player->data->cur_yaoshi, player->data->chengjie.level);
		if (title != NULL)
		{
			expadd = title->TitleEffect2;
		}
	}

	AnsAcceptChengjieTask send;
	ans_accept_chengjie_task__init(&send);
	send.ret = 0;
	if (pTask == NULL)
	{
		send.ret = 190500138;
		goto done;
	}
	if (!pTask->complete)
	{
		send.ret = 190500138;
		goto done;
	}

	if (param_config != NULL)
	{
		cost = param_config->parameter1[0]; //押金
	}
	if (config != NULL)
	{
		int rate = pTask->fail;
		if (rate > config->parameter1[0])
		{
			rate = config->parameter1[0];
		}
		if (rate > 0)
		{
			cost += (config->parameter1[1] * rate / 10000); //失败次数加成
		}
	}

	if (tableSkill != NULL)
	{
		cost += tableSkill->EffectValue[0];
		expadd += tableSkill->EffectValue[0];
	}
	player->add_coin(pTask->shuangjin * (10000 + cost)/10000, MAGIC_TYPE_YAOSHI);
	player->add_chengjie_exp(pTask->exp * (10000 + expadd)/10000);
	player->add_chengjie_courage(pTask->courage * (10000 + expadd) / 10000);
	send.taskid = player->data->chengjie.cur_task;

	ChengJieTaskManage::DelTask(player->data->chengjie.cur_task);
	player->data->chengjie.cur_task = 0;
	player->data->chengjie.target = 0;

done:

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_SUBMIT_CHENGJIE_TASK_ANSWER, ans_accept_chengjie_task__pack, send);

	return (0);
}
static int handle_accept_shangjin_task_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	if (player->data->shangjin.accept)
	{
		return (-2);
	}

	int ret = 0;
	if (player->data->shangjin.shangjin_num < 1)
	{
		ret = 190500233;
		goto done;
	}
	ret = player->accept_task(player->data->shangjin.task[0].id, false);
	if (ret != 0)
	{
		goto done;
	}

	--player->data->shangjin.shangjin_num;
	player->data->shangjin.accept = true;
	player->check_activity_progress(AM_YAOSHI, 1);
	player->add_task_progress(TCT_YAOSHI_SHANGJIN, 0, 1);
done:
	send_comm_answer(MSG_ID_ACCEPT_SHANGJIN_TASK_ANSWER, ret, extern_data);

	return (0);
}
static int handle_refresh_shangjin_task_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	if (player->data->shangjin.accept)
	{
		return (-2);
	}

	ShangjinList shangJin;
	shangjin_list__init(&shangJin);
	shangJin.ret = 190500162;
	if (player->data->shangjin.free > 0)
	{
		--player->data->shangjin.free;
		player->send_all_yaoshi_num();
	}
	else
	{
		ParameterTable * config = get_config_by_id(161000084, &parameter_config);
		if (config == NULL || player->del_item(config->parameter1[0], config->parameter1[1], MAGIC_TYPE_YAOSHI) < 0)
		{
			shangJin.ret = 190500123;
			fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_REFRESH_SHANGJIN_TASK_ANSWER, shangjin_list__pack, shangJin);
			return 1;
		}
	}

	//fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_REFRESH_SHANGJIN_TASK_ANSWER, shangjin_list__pack, shangJin);
	ShangjinManage::RefreshTaskAndSend(player, true);

	return 0;
}

static int handle_get_on_cash_truck_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	if (player->data->truck.truck_id == 0)
	{
		return -2;
	}
	if (player->data->truck.on_truck)
	{
		return -3;
	}
	if (player->sight_space != NULL)
	{
		send_comm_answer(MSG_ID_GET_ON_CASH_TRUCK_ANSWER, 190500302, extern_data);
		return -4;
	}
	player->data->truck.on_truck = true;

	send_comm_answer(MSG_ID_GET_ON_CASH_TRUCK_ANSWER, 0, extern_data);

	//UpDownCashTruck send;
	//up_down_cash_truck__init(&send);
	//send.cash_truck = player->data->truck_id;
	//send.playerid = player->get_uuid();
	//conn_node_gamesrv::send_to_all_player(MSG_ID_GET_ON_CASH_TRUCK_NOTIFY, (void *)&send, (pack_func)up_down_cash_truck__pack);


	SightChangedNotify notify;
	sight_changed_notify__init(&notify);
	uint64_t del_player_id[1];
	del_player_id[0] = player->data->player_id;
	notify.delete_player = del_player_id;
	notify.n_delete_player = 1;
	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
	for (int i = 0; i < player->data->cur_sight_player; ++i)
		conn_node_gamesrv::broadcast_msg_add_players(player->data->sight_player[i], ppp);
	conn_node_gamesrv::broadcast_msg_send();

	cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(player->data->truck.truck_id);
	if (truck == NULL)
	{
		return -4;
	}
	TruckEndurance send;
	truck_endurance__init(&send);
	send.endurance = truck->data->endurance;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_CASH_TRUCK_ENDURANCE_NOTIFY, truck_endurance__pack, send);
	player->adjust_battle_partner();

	return 0;
}
static int handle_cash_truck_speed_up_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	if (player->data->truck.truck_id == 0)
	{
		return -2;
	}
	if (!player->data->truck.on_truck)
	{
		return -3;
	}

	ParameterTable *table = get_config_by_id(161000225, &parameter_config);
	if (table == NULL)
	{
		return -6;
	}

	cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(player->data->truck.truck_id);
	if (truck == NULL)
	{
		return -4;
	}
	if (truck->data->endurance < table->parameter1[2])
	{
		return -5;
	}
	//if (truck->data->n_speed_up == MAX_CASH_TRUCK_SPEED_UP)
	//{
	//	return -6;
	//}
	
	if (truck->data->speed_reduce / 1000 > time_helper::get_cached_time() / 1000)
	{
		AnsSpeedUp ans;
		ans_speed_up__init(&ans);
		ans.ret = 190500123;
		ans.cd = (truck->data->speed_reduce - time_helper::get_cached_time()) / 1000;
		fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_CASH_TRUCK_SPEED_UP_ANSWER, ans_speed_up__pack, ans);
		return -7;
	}

	truck->data->endurance -= table->parameter1[2];
	TruckEndurance send;
	truck_endurance__init(&send);
	send.endurance = truck->data->endurance;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_CASH_TRUCK_ENDURANCE_NOTIFY, truck_endurance__pack, send);

	if (truck->data->endurance == 0)
	{
		truck->data->n_speed_up = 0;
		truck->data->speed_reduce = time_helper::get_cached_time() + table->parameter1[4] * 1000;
	}
	else
	{
		if (truck->data->n_speed_up < MAX_CASH_TRUCK_SPEED_UP)
		{
			truck->data->speed_up[truck->data->n_speed_up++] = time_helper::get_cached_time() + table->parameter1[4] * 1000;
		}
	}

	PlayerAttrNotify nty;
	player_attr_notify__init(&nty);
	AttrData attr_data[PLAYER_ATTR_MAX + 2];
	AttrData *attr_data_point[PLAYER_ATTR_MAX + 2];
	nty.player_id = truck->get_uuid();
	nty.n_attrs = 0;
	nty.attrs = attr_data_point;

	attr_data_point[nty.n_attrs] = &attr_data[nty.n_attrs];
	attr_data__init(&attr_data[nty.n_attrs]);
	attr_data[nty.n_attrs].id = PLAYER_ATTR_MOVE_SPEED;
	attr_data[nty.n_attrs].val = truck->get_speed();
	nty.n_attrs++;
	//conn_node_gamesrv::send_to_all_player(MSG_ID_PLAYER_ATTR_NOTIFY, (void *)&nty, (pack_func)player_attr_notify__pack);
	player->broadcast_to_sight(MSG_ID_PLAYER_ATTR_NOTIFY, (void *)&nty, (pack_func)player_attr_notify__pack, true);
	

	return 0;
}
/*
static int handle_submit_cash_truck_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	if (player->data->truck.truck_id == 0)
	{
		return -2;
	}


	BiaocheTable *table = get_config_by_id(player->data->truck.active_id, &cash_truck_config);
	if (table == NULL)
	{
		return -2;
	}
	if (table->Type == 1)  //粮草押镖
	{
		
	}
	else //财宝押镖
	{
		
	}

	player->add_task_progress(TCT_TRUCK, 0, 1);
	player->check_activity_progress(AM_TRUCK, table->ID);

	return 0;
}
*/
static int handle_go_downd_cash_truck_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	player->go_down_cash_truck();

	return 0;
}
int check_can_accept_cash_truck(player_struct *player, uint32_t type)
{
	BiaocheTable *table = get_config_by_id(type, &cash_truck_config);
	if (table == NULL)
	{
		return -2;
	}
	uint32_t cd = 0;
	if (!check_active_open(table->ActivityControl, cd))
	{
		return 190500104;
	}
	if (player->data->truck.truck_id != 0)
	{
		return 190500316;
	}
	if (table->Type == 1)
	{
		if (player->data->truck.num_coin < 1)
		{
			return 190500289;
		}
	}
	else
	{
		if (player->get_attr(PLAYER_ATTR_ZHENYING) == 0)
		{
			return 190500306;
		}
		if (player->data->truck.num_gold < 1)
		{
			return 190500291;
		}
	}
	TaskTable *config = get_config_by_id(table->TaskId, &task_config);
	if (!config)
	{
		return -5;
	}
	if (config->Level > player->get_attr(PLAYER_ATTR_LEVEL))
	{
		return 190500107;
	}
	BiaocheRewardTable *reward_config = get_config_by_id(table->Reward, &cash_truck_reward_config);
	if (!reward_config)
	{
		return -6;
	}
	uint32_t subCoin = reward_config->RewardMoney1 * player->get_attr(PLAYER_ATTR_LEVEL) * reward_config->Deposit / 10000;
	if (subCoin > player->get_coin())
	{
		return 190500294;
	}
	int ret = player->accept_task(table->TaskId, false);
	if (ret != 0)
	{
		return ret;
	}
	if (table->Type == 1)
	{
		--player->data->truck.num_coin;
	}
	else
	{
		--player->data->truck.num_gold;
	}
	player->sub_coin(subCoin, MAGIC_TYPE_CASH_TRUCK);
	return 0;
}
static int handle_accept_cash_truck_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	AcceptCashTruck *req = accept_cash_truck__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint32_t type = req->id;
	accept_cash_truck__free_unpacked(req, NULL);

	BiaocheTable *table = get_config_by_id(type, &cash_truck_config);
	if (table == NULL)
	{
		return -2;
	}
	
	ResAcceptCashTruck send;
	res_accept_cash_truck__init(&send);
	send.id = table->TaskId;
	
	send.ret = check_can_accept_cash_truck(player, type);
	if (send.ret == 0)
	{
		cash_truck_struct *pTruck = cash_truck_manager::create_cash_truck_at_pos(player->scene, table->ID, *player);
		if (pTruck != NULL)
		{
			player->data->truck.truck_id = pTruck->get_uuid();
			player->data->truck.active_id = type;
			player->data->truck.jiefei = 0;
			send.type = pTruck->get_truck_type();

			if (pTruck->get_truck_type() == 2 && player->get_attr(PLAYER_ATTR_PK_TYPE) != PK_TYPE_CAMP)
			{
				player->set_attr(PLAYER_ATTR_PK_TYPE, PK_TYPE_CAMP);
				player->broadcast_one_attr_changed(PLAYER_ATTR_PK_TYPE, PK_TYPE_CAMP, true, true);
			}
		}

		
	}

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_ACCEPT_CASH_TRUCK_ANSWER, res_accept_cash_truck__pack, send);
	
	return 0;
}

static int notify_setting_switch_info(player_struct *player, EXTERN_DATA *extern_data)
{
	SettingSwitchNotify resp;
	setting_switch_notify__init(&resp);

	SettingSwitchData switch_data[2];
	SettingSwitchData* switch_data_point[2];

	switch_data_point[0] = &switch_data[0];
	setting_switch_data__init(&switch_data[0]);
	switch_data[0].type = SETTING_SWITCH_TYPE__qiecuo;
	switch_data[0].state = player->data->qiecuo_invite_switch;

	switch_data_point[1] = &switch_data[1];
	setting_switch_data__init(&switch_data[1]);
	switch_data[1].type = SETTING_SWITCH_TYPE__team_invite;
	switch_data[1].state = player->data->team_invite_switch;

	resp.datas = switch_data_point;
	resp.n_datas = 2;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_SETTING_SWITCH_NOTIFY, setting_switch_notify__pack, resp);

	return 0;
}

//设置开关信息请求
static int handle_setting_turn_switch_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	SettingTurnSwitchRequest *req = setting_turn_switch_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t type = req->type;

	setting_turn_switch_request__free_unpacked(req, NULL);

	int ret = 0;
	uint8_t *pSwitch = NULL;
	do
	{
		switch (type)
		{
			case SETTING_SWITCH_TYPE__qiecuo:
				pSwitch = &player->data->qiecuo_invite_switch;
				break;
			case SETTING_SWITCH_TYPE__team_invite:
				pSwitch = &player->data->team_invite_switch;
				break;
			case SETTING_SWITCH_TYPE__friend:
				{
					//换一个消息号，转发到好友服处理
					fast_send_msg_base(&conn_node_gamesrv::connecter, extern_data, SERVER_PROTO_FRIEND_TURN_SWITCH, 0, get_seq());
					return 0;
				}
		}

		if (!pSwitch)
		{
			ret = 1;
			LOG_ERR("[%s:%d] player[%lu] switch type error, type:%u", __FUNCTION__, __LINE__, extern_data->player_id, type);
			break;
		}

		if (*pSwitch == 0)
		{
			*pSwitch = 1;
		}
		else
		{
			*pSwitch = 0;
		}

		{
			SettingSwitchNotify resp;
			setting_switch_notify__init(&resp);

			SettingSwitchData switch_data[1];
			SettingSwitchData* switch_data_point[1];

			switch_data_point[0] = &switch_data[0];
			setting_switch_data__init(&switch_data[0]);
			switch_data[0].type = (SettingSwitchType)type;
			switch_data[0].state = *pSwitch;

			resp.datas = switch_data_point;
			resp.n_datas = 1;

			fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_SETTING_SWITCH_NOTIFY, setting_switch_notify__pack, resp);
		}

	} while(0);

	SettingTurnSwitchAnswer resp;
	setting_turn_switch_answer__init(&resp);

	SettingSwitchData switch_data;
	setting_switch_data__init(&switch_data);
	switch_data.type = (SettingSwitchType)type;
	switch_data.state = (pSwitch ? *pSwitch : 0);

	resp.result = ret;
	resp.data = &switch_data;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_SETTING_TURN_SWITCH_ANSWER, setting_turn_switch_answer__pack, resp);

	return 0;
}

static int notify_transfer_out_stuck_info(player_struct *player, EXTERN_DATA *extern_data)
{
	TransferOutStuckInfoNotify resp;
	transfer_out_stuck_info_notify__init(&resp);

	resp.cdtimestamp = player->data->out_stuck_time;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TRANSFER_OUT_STUCK_INFO_NOTIFY, transfer_out_stuck_info_notify__pack, resp);

	return 0;
}

//脱离卡死请求
static int handle_transfer_out_stuck_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	int ret = 0;
	do
	{
		uint32_t now = time_helper::get_cached_time() / 1000;
		if (player->data->out_stuck_time > now)
		{
			ret = ERROR_ID_TRANSFER_OUT_STUCK_CDING;
			LOG_ERR("[%s:%d] player[%lu] out stuck cd, out_stuck_time:%u, now:%u", __FUNCTION__, __LINE__, extern_data->player_id, player->data->out_stuck_time, now);
			break;
		}

		if (player->transfer_to_birth_position(extern_data) != 0)
		{
			ret = ERROR_ID_TRANSFER_OUT_STUCK_FAIL;
			LOG_ERR("[%s:%d] player[%lu] transfer fail", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		
		if (player->data->truck.on_truck)
		{
			player->go_down_cash_truck();
		}

		player->data->out_stuck_time = now + sg_transfer_out_stuck_cd_time;

	} while(0);

	TransferOutStuckAnswer resp;
	transfer_out_stuck_answer__init(&resp);

	resp.result = ret;
	resp.cdtimestamp = player->data->out_stuck_time;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TRANSFER_OUT_STUCK_ANSWER, transfer_out_stuck_answer__pack, resp);

	return 0;
}

static int handle_srv_check_and_cost_request(player_struct *player, EXTERN_DATA *extern_data, uint32_t msg_id)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);

	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PROTO_GUILDSRV_CHECK_AND_COST_REQ *req = (PROTO_GUILDSRV_CHECK_AND_COST_REQ*)buf_head();

	int ret = 0;
	SRV_COST_INFO real_cost;
	memset(&real_cost, 0, sizeof(SRV_COST_INFO));
	do
	{
		if (req->cost.gold > 0)
		{
			uint32_t has_gold = player->get_comm_gold();
			if (has_gold < req->cost.gold)
			{
				ret = ERROR_ID_GOLD_NOT_ENOUGH;
				break;
			}
		}
		if (req->cost.coin > 0)
		{
			uint32_t has_coin = player->get_coin();
			if (has_coin < req->cost.coin)
			{
				ret = ERROR_ID_COIN_IS_NOT_ENOUGH;
				break;
			}
		}
		size_t item_len = (sizeof(req->cost.item_id) / sizeof(req->cost.item_id[0]));
		for (size_t i = 0; i < item_len; ++i)
		{
			uint32_t item_id = req->cost.item_id[i];
			uint32_t need_num = req->cost.item_num[i];
			if (item_id > 0 && need_num > 0)
			{
				uint32_t has_num = player->get_item_can_use_num(item_id);
				if (has_num < need_num)
				{
					ret = ERROR_ID_PROP_NOT_ENOUGH;
					break;
				}
			}
		}

		if (ret != 0)
		{
			break;
		}

		//扣除
		if (req->cost.gold > 0)
		{
			uint32_t has_bind_gold = player->get_attr(PLAYER_ATTR_BIND_GOLD);
			uint32_t has_unbind_gold = player->get_attr(PLAYER_ATTR_GOLD);
			player->sub_comm_gold(req->cost.gold, req->cost.statis_id);
			real_cost.gold = has_bind_gold - player->get_attr(PLAYER_ATTR_BIND_GOLD);
			real_cost.unbind_gold = has_unbind_gold - player->get_attr(PLAYER_ATTR_GOLD);
		}
		if (req->cost.coin > 0)
		{
			player->sub_coin(req->cost.coin, req->cost.statis_id);
			real_cost.coin = req->cost.coin;
		}
		uint32_t item_idx = 0;
		for (size_t i = 0; i < item_len; ++i)
		{
			uint32_t item_id = req->cost.item_id[i];
			uint32_t need_num = req->cost.item_num[i];
			if (item_id > 0 && need_num > 0)
			{
				uint32_t item_bind_id = 0, item_unbind_id = 0;
				get_item_bind_and_unbind_id(item_id, &item_bind_id, &item_unbind_id);
				uint32_t item_bind_num = player->get_item_num_by_id(item_bind_id);
				uint32_t item_unbind_num = player->get_item_num_by_id(item_unbind_id);
				player->del_item(item_id, need_num, req->cost.statis_id);
				uint32_t cost_bind_num = item_bind_num - player->get_item_num_by_id(item_bind_id);
				uint32_t cost_unbind_num = item_unbind_num - player->get_item_num_by_id(item_unbind_id);
				if (cost_bind_num > 0)
				{
					real_cost.item_id[item_idx] = item_bind_id;
					real_cost.item_num[item_idx] = cost_bind_num;
					item_idx++;
				}
				if (cost_unbind_num > 0)
				{
					real_cost.item_id[item_idx] = item_unbind_id;
					real_cost.item_num[item_idx] = cost_unbind_num;
					item_idx++;
				}
			}
		}

		real_cost.statis_id = req->cost.statis_id;
	} while(0);

	PROTO_GUILDSRV_CHECK_AND_COST_RES *res = (PROTO_GUILDSRV_CHECK_AND_COST_RES *)get_send_buf(msg_id, get_seq());
	res->head.len = ENDION_FUNC_4(sizeof(PROTO_GUILDSRV_CHECK_AND_COST_RES) + req->data_size);
	memset(res->head.data, 0, sizeof(PROTO_GUILDSRV_CHECK_AND_COST_RES) - sizeof(PROTO_HEAD));
	res->result = ret;
	memcpy(&res->cost, &real_cost, sizeof(SRV_COST_INFO));
	res->data_size = req->data_size;
	if (req->data_size > 0)
	{
		memcpy(res->data, req->data, req->data_size);
	}
	add_extern_data(&res->head, extern_data);
	if (conn_node_gamesrv::connecter.send_one_msg(&res->head, 1) != (int)ENDION_FUNC_4(res->head.len))
	{
		LOG_ERR("[%s:%d] send to conn_srv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}

	return 0;
}

//帮会服请求扣除消耗
static int handle_guildsrv_check_and_cost_request(player_struct *player, EXTERN_DATA *extern_data)
{
	return handle_srv_check_and_cost_request(player, extern_data, SERVER_PROTO_GUILDSRV_COST_ANSWER);
}

//帮会服请求发放奖励
static int handle_guildsrv_reward_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);

	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PROTO_GUILDSRV_REWARD_REQ *req = (PROTO_GUILDSRV_REWARD_REQ*)buf_head();

	int ret = 0;
	do
	{
		std::map<uint32_t, uint32_t> item_map;
		size_t item_len = (sizeof(req->item_id) / sizeof(req->item_id[0]));
		for (size_t i = 0; i < item_len; ++i)
		{
			uint32_t item_id = req->item_id[i];
			uint32_t item_num = req->item_num[i];
			if (item_id > 0 && item_num > 0)
			{
				item_map[item_id] += item_num;
			}
		}

		//检查背包格子
		if (item_map.size() > 0)
		{
			if (!player->check_can_add_item_list(item_map))
			{
				ret = ERROR_ID_BAG_GRID_NOT_ENOUGH;
				break;
			}
		}

		//发放奖励
		if (req->gold > 0)
		{
			player->add_bind_gold(req->gold, req->statis_id);
		}
		if (req->coin > 0)
		{
			player->add_coin(req->coin, req->statis_id);
		}
		if (item_map.size() > 0)
		{
			player->add_item_list(item_map, req->statis_id, true);
		}

		if (req->statis_id == MAGIC_TYPE_SHOP_BUY)
		{
			player->add_task_progress(TCT_SHOP_BUY, 0, 1);
		}
	} while(0);

	PROTO_GUILDSRV_REWARD_RES *res = (PROTO_GUILDSRV_REWARD_RES *)get_send_buf(SERVER_PROTO_GUILDSRV_REWARD_ANSWER, get_seq());
	res->head.len = ENDION_FUNC_4(sizeof(PROTO_GUILDSRV_REWARD_RES) + req->data_size);
	memset(res->head.data, 0, sizeof(PROTO_GUILDSRV_REWARD_RES) - sizeof(PROTO_HEAD));
	res->result = ret;
	res->statis_id = req->statis_id;
	res->data_size = req->data_size;
	if (req->data_size > 0)
	{
		memcpy(res->data, req->data, req->data_size);
	}
	add_extern_data(&res->head, extern_data);
	if (conn_node_gamesrv::connecter.send_one_msg(&res->head, 1) != (int)ENDION_FUNC_4(res->head.len))
	{
		LOG_ERR("[%s:%d] send to guildsrv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}

	return 0;
}

//帮会服同步帮会技能
static int handle_sync_guild_skill_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);

	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	memset(player->data->guild_skill_attr, 0, sizeof(double) * PLAYER_ATTR_MAX);
	PROTO_SYNC_GUILD_SKILL *req = (PROTO_SYNC_GUILD_SKILL*)buf_head();
	for (uint32_t i = 0; i < req->attr_num; ++i)
	{
		player->data->guild_skill_attr[req->attr_id[i]] += req->attr_val[i];
	}

	player->calculate_attribute(true);

	return 0;
}

//帮会服同步帮会信息
static int handle_sync_guild_info_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);

	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	uint32_t old_guild_id = player->data->guild_id;
	PROTO_SYNC_GUILD_INFO *req = (PROTO_SYNC_GUILD_INFO*)buf_head();
	player->data->guild_id = req->guild_id;
	player->data->guild_office = req->guild_office;
	strcpy(player->data->guild_name, req->guild_name);

	if (req->guild_id > 0)
	{
		update_guild_name(req->guild_id, req->guild_name);
	}

	//原来在帮会，现在退出了
	if (old_guild_id > 0 && player->data->guild_id == 0)
	{
		raid_struct *raid = player->get_raid();
		if (raid)
		{
			if (raid->is_guild_battle_raid())
			{
				raid->player_leave_raid(player);
			}
		}
		else
		{
			player->transfer_out_guild_scene(extern_data);
		}
	}
	else if (old_guild_id == 0 && player->data->guild_id > 0)
	{
		player->add_task_progress(TCT_GUILD_JOIN, (player->data->guild_office == 1 ? 2 : 1), 1);
	}

	if (old_guild_id != player->data->guild_id)
	{
		SightPlayerInfoChangeNotify nty;
		sight_player_info_change_notify__init(&nty);

		nty.player_id = player->get_uuid();
		nty.guild_id = player->data->guild_id;
		nty.has_guild_id = true;
		player->broadcast_to_sight(MSG_ID_SIGHT_PLAYER_CHANGE_NOTIFY, &nty, (pack_func)sight_player_info_change_notify__pack, true);
	}

	return 0;
}

static int handle_guild_answer_award(player_struct *player, EXTERN_DATA *extern_data)
{
	if (player == NULL)
	{
		return -1;
	}
	ParameterTable *table = get_config_by_id(161000200, &parameter_config);
	if (table == NULL)
	{
		return -3;
	}
	PROTO_GUILD_DISBAND *req = (PROTO_GUILD_DISBAND*)buf_head();
	uint8_t *pData = (uint8_t *)req->data;
	FactionQuestionResult *reqAward = faction_question_result__unpack(NULL, req->player_num, pData);
	if (reqAward == NULL)
	{
		return -2;
	}

	player->add_item(table->parameter1[0], table->parameter1[1], MAGIC_TYPE_QUESTION);
	if (reqAward->add)
	{
		player->add_item(table->parameter1[2], table->parameter1[3], MAGIC_TYPE_QUESTION);
	}

	player->add_task_progress(TCT_QUESTION_JOIN, 0, 1);

	faction_question_result__free_unpacked(reqAward, NULL);
	return 0;
}

//帮会服同步帮会解散
static int handle_sync_guild_disband_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);

	PROTO_GUILD_DISBAND *req = (PROTO_GUILD_DISBAND*)buf_head();
	uint64_t *pData = (uint64_t*)req->data;

	//将所有玩家拉出帮会地图
	EXTERN_DATA ext_data;
	for (uint32_t i = 0; i < req->player_num; ++i)
	{
		uint64_t member_id = *pData++;
		player_struct *member = player_manager::get_player_by_id(member_id);
		if (member)
		{
			ext_data.player_id = member_id;
			member->transfer_out_guild_scene(&ext_data);
			member->data->guild_id = 0;
			member->data->guild_name[0] = '\0';
		}
	}

	//销毁帮会场景
	scene_manager::del_guild_scene(req->guild_id);

	return 0;
}

//帮会战征召请求
static int handle_guild_battle_call_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PROTO_HEAD *req = (PROTO_HEAD*)buf_head();
	uint32_t player_num = *((uint32_t*)(req->data));
	uint64_t *pData = (uint64_t*)(req->data + sizeof(uint32_t));

	std::vector<uint64_t> playerIds;
	for (uint32_t i = 0; i < player_num; ++i)
	{
		uint64_t member_id = *pData++;
		if (member_id == extern_data->player_id)
		{
			continue;
		}

		player_struct *member = player_manager::get_player_by_id(member_id);
		if (member && member->is_online() && !member->is_in_raid() && member->is_alive())
		{
			playerIds.push_back(member_id);
		}
	}

	int ret = 0;
	do
	{
		if (!is_guild_battle_opening())
		{
			ret = ERROR_ID_GUILD_BATTLE_ACTIVITY_NOT_OPEN;
			LOG_ERR("[%s:%d] player[%lu] guild battle not open", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		uint32_t now = time_helper::get_cached_time() / 1000;
		uint32_t call_cd = get_guild_call_cd(player->data->guild_id);
		if (call_cd > now)
		{
			ret = ERROR_ID_GUILD_CALL_CD;
			LOG_ERR("[%s:%d] player[%lu] call cd, guild_id:%u, now:%u, call_cd:%u", __FUNCTION__, __LINE__, extern_data->player_id, player->data->guild_id, now, call_cd);
			break;
		}

		set_guild_call_cd(player->data->guild_id);
		broadcast_guild_battle_call_notify(playerIds, player->get_uuid(), player->data->name);
	} while(0);
	
	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_GUILD_BATTLE_CALL_ANSWER, comm_answer__pack, resp);

	return 0;
}

//帮会战信息请求
static int handle_guild_battle_info_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	int ret = 0;
	do
	{
		if (!is_guild_battle_opening())
		{
			ret = ERROR_ID_GUILD_BATTLE_ACTIVITY_NOT_OPEN;
			LOG_ERR("[%s:%d] player[%lu] guild battle not open", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (player->data->guild_id <= 0)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not join guild", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		conn_node_gamesrv::connecter.transfer_to_connsrv();
	} while(0);
	
	if (ret != 0)
	{
		GuildBattleInfoAnswer resp;
		guild_battle_info_answer__init(&resp);

		resp.result = ret;
		fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_GUILD_BATTLE_INFO_ANSWER, guild_battle_info_answer__pack, resp);
	}

	return 0;
}

//帮会战同步排名
static int handle_guild_battle_final_list_answer(player_struct * /*player*/, EXTERN_DATA * /*extern_data*/)
{
	LOG_INFO("[%s:%d]", __FUNCTION__, __LINE__);

	PROTO_GUILD_BATTLE_RANK *req = (PROTO_GUILD_BATTLE_RANK*)buf_head();
	set_final_guild_id(req->guild_id, sizeof(req->guild_id) / sizeof(req->guild_id[0]));

	guild_battle_manager_final_list_state = GBFLS_GOTTEN;
	guild_battle_manager_final_list_tick = time_helper::get_cached_time() / 1000;

	return 0;
}

//同步帮贡
static int handle_guild_sync_donation(player_struct * player, EXTERN_DATA * extern_data)
{
	LOG_INFO("[%s:%d]", __FUNCTION__, __LINE__);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PROTO_HEAD *head = (PROTO_HEAD*)buf_head();
	uint32_t *pData = (uint32_t*)head->data;

	player->data->guild_donation = *pData;
	player->add_task_progress(TCT_GUILD_DONATION, 0, player->data->guild_donation);

	return 0;
}

static int on_login_send_zhenying(player_struct *player, EXTERN_DATA *extern_data)
{
	if (player->data->zhenying.level == 0)
	{
		player->data->zhenying.level = 1;
		player->data->zhenying.step = 1;
		std::map<uint64_t, struct CampTable*>::iterator it = zhenying_base_config.begin();
		player->data->zhenying.free = it->second->FreeNum;
		WeekTable *table = get_rand_week_config();
		if (table != NULL)
		{
			player->data->zhenying.task = table->ID;
			player->data->zhenying.task_type = table->Type;
		}
		FactionBattleTable *tableFa = get_zhenying_battle_table(player->get_attr(PLAYER_ATTR_LEVEL));
		if (tableFa != NULL)
		{
			player->data->zhenying.mine = tableFa->BoxOpenNum;
		}
		player->data->zhenying.last_week = time_helper::nextWeek(5 * 3600);
	}

	player->send_zhenying_info();

	return 0;
}
static int handle_into_zhenying_battle_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	zhenying_raid_struct *raid = zhenying_raid_manager::add_player_to_zhenying_raid(player);
	if (raid == NULL)
	{
		LOG_ERR("[%s] :player[%lu] fail", __FUNCTION__, extern_data->player_id);
		return (-10);
	}

	send_comm_answer(MSG_ID_INTO_ZHENYING_BATTLE_ANSWER, 0, extern_data);

	//下面是正常的逻辑 
	CommAnswer *req = comm_answer__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	if (req->result == 1)
	{
		EventCalendarTable *table = get_config_by_id(330100022, &activity_config);
		if (table != NULL && table->n_AuxiliaryValue > 0)
		{
			player->accept_task(table->AuxiliaryValue[0], false);
		}
	}
	comm_answer__free_unpacked(req, NULL);

	return 0;
}
static int handle_zhenying_change_line_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	if (player->scene == NULL)
	{
		return -2;
	}
	if (!player->scene->is_in_zhenying_raid())
	{
		return -3;
	}

	ZhenyingLineInfo *req = zhenying_line_info__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	int line = req->id;
	zhenying_line_info__free_unpacked(req, NULL);

	zhenying_raid_struct *raid = (zhenying_raid_struct *)(player->scene);
	if (raid->get_line_num() == line)
	{
		return -4;
	}

	zhenying_raid_struct *newRaid = zhenying_raid_manager::get_zhenying_raid_by_line(player->data->scene_id, line);
	int ret = 0;
	if (newRaid == NULL)
	{
		ret = 1;
	}
	else
	{
		player->send_clear_sight();
		raid->on_player_leave_raid(player);
		//player->scene->delete_player_from_scene(player);
		newRaid->add_player_to_zhenying_raid(player);
		player_ready_enter_scene(player, extern_data, false);
	}
	send_comm_answer(MSG_ID_ZHENYING_CHANGE_LINE_ANSWER, ret, extern_data);

	return 0;
}
static int handle_zhenying_get_line_info_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	if (player->scene == NULL)
	{
		return -2;
	}
	if (!player->scene->is_in_zhenying_raid())
	{
		return -3;
	}

	ZhenyingLine send;
	zhenying_line__init(&send);
	int n_line = 0;

	ZhenyingLineInfo info[ZhenyingBattle::MAX_LINE_NUM];
	ZhenyingLineInfo *infoPoint[ZhenyingBattle::MAX_LINE_NUM];

	zhenying_raid_struct *raid = NULL;
	std::map<uint64_t, zhenying_raid_struct *>::iterator iter = zhenying_raid_manager_all_raid_id.begin();
	for (; iter != zhenying_raid_manager_all_raid_id.end(); ++iter)
	{
		raid = iter->second;
		if (raid->m_id == player->data->scene_id)
		{
			zhenying_line_info__init(&info[n_line]);
			info[n_line].id = raid->get_line_num();
			info[n_line].man = raid->get_cur_player_num();
			info[n_line].state = ZHENYING__LINE__STATE__NORMAL;
			infoPoint[n_line] = info + n_line;
			++n_line;
		}
	}

	raid = (zhenying_raid_struct *)(player->scene);
	send.my_line = raid->get_line_num();
	send.n_all_line = n_line;
	send.all_line = infoPoint;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_ZHENYING_GET_LINE_INFO_ANSWER, zhenying_line__pack, send);

	return 0;
}
static int handle_get_zhenying_task_award_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	WeekTable *table = get_config_by_id(player->data->zhenying.task, &zhenying_week_config);
	if (table == NULL)
	{
		return -2;
	}

	if (player->data->zhenying.task_num == table->Num)
	{
		//任务完成 给奖励
		std::map<uint32_t, uint32_t> item_list;
		item_list.insert(std::make_pair(table->Reward, 1));
		player->add_item_list_otherwise_send_mail(item_list, MAGIC_TYPE_ZHENYING, 0, NULL);
		//player->give_drop_item(table->Reward, MAGIC_TYPE_MONSTER_DEAD, ADD_ITEM_AS_MUCH_AS_POSSIBLE);
	}

	++player->data->zhenying.week;
	if (player->data->zhenying.week > 6)
	{
		player->data->zhenying.task = 0;
		player->data->zhenying.task_type = 0;
	}
	else
	{
		table = get_rand_week_config();
		if (table != NULL)
		{
			player->data->zhenying.task = table->ID;
			player->data->zhenying.task_type = table->Type;
		}
	}

	send_comm_answer(MSG_ID_GET_ZHENYING_TASK_AWARD_ANSWER, 0, extern_data);

	player->data->zhenying.task_num = 0;

	NewZhenyingTask send;
	new_zhenying_task__init(&send);
	send.task = player->data->zhenying.task;
	send.task_type = player->data->zhenying.task_type;
	send.task_num = player->data->zhenying.task_num;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_NEW_ZHENYING_TASK_NOTIFY, new_zhenying_task__pack, send);

	return 0;
}
static int handle_exit_zhenying_battle_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	player->transfer_to_new_scene_by_config(155000004, extern_data);

	return 0;
}
static int handle_zhenying_team_info_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	ZhenyingTeamInfo info[MAX_TEAM_MEM];
	ZhenyingTeamInfo *pInfo[MAX_TEAM_MEM] = {info, info + 1, info + 2, info + 3, info + 4};
	ZhenyingTeam send;
	zhenying_team__init(&send);
	int num = 0;
	std::vector<uint64_t> vt;
	if (player->m_team != NULL)
	{
		for (int i = 0; i < player->m_team->m_data->m_memSize; ++i)
		{
			vt.push_back(player->m_team->m_data->m_mem[i].id);
		}
	}
	else
	{
		vt.push_back(player->get_uuid());
	}
	for (std::vector<uint64_t>::iterator it = vt.begin(); it != vt.end(); ++it)
	{
		player_struct *mem = player_manager::get_player_by_id(*it);
		if (mem == NULL)
		{
			continue;
		}

		zhenying_team_info__init(info + num);
		info[num].playerid = mem->get_uuid();
		info[num].name = mem->get_name();
		info[num].job = mem->get_attr(PLAYER_ATTR_JOB);
		info[num].lv = mem->get_attr(PLAYER_ATTR_LEVEL);
		info[num].kill = mem->data->zhenying.kill;
		info[num].death = mem->data->zhenying.death;
		info[num].assist = mem->data->zhenying.help;
		info[num].score = mem->data->zhenying.score;
		++num;
	}
	send.n_mem = num;
	send.mem = pInfo;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_ZHENYING_TEAM_INFO_ANSWER, zhenying_team__pack, send);

	return 0;
}
static int handle_zhenying_power_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	//ZhenyingPower send;
	//zhenying_power__init(&send);
	//send.power_fulongguo = 1;
	//send.power_wanyaogu = 2;
	//send.man_wanyaogu = 3;
	//send.man_fulongguo = 5;
	//send.power_man = 0;
	//fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_ZHENYING_POWER_ANSWER, zhenying_power__pack, send);

	AddZhenyingPlayer tofr;
	add_zhenying_player__init(&tofr);
	tofr.name = player->get_name();
	tofr.zhenying = player->get_attr(PLAYER_ATTR_ZHENYING);
	tofr.zhenying_old = player->get_attr(PLAYER_ATTR_ZHENYING);
	tofr.fighting_capacity = player->get_attr(PLAYER_ATTR_FIGHTING_CAPACITY);
	tofr.kill = player->data->zhenying.kill;
	conn_node_gamesrv::connecter.send_to_friend(extern_data, MSG_ID_ZHENYING_POWER_REQUEST, &tofr, (pack_func)add_zhenying_player__pack);

	return 0;
}
static int handle_change_zhenying_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	ChoseZhenying *req = chose_zhenying__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	int zhenYing = req->zhenying;
	chose_zhenying__free_unpacked(req, NULL);

	if (zhenYing == player->get_attr(PLAYER_ATTR_ZHENYING) || player->get_attr(PLAYER_ATTR_ZHENYING) == 0)
	{
		return -2;
	}

	AnsChoseZhenying send;
	ans_chose_zhenying__init(&send);
	if (player->data->zhenying.change_cd > time_helper::get_cached_time() / 1000)
	{
		send.ret = 190500226;
		send.zhenying = player->data->zhenying.change_cd - time_helper::get_cached_time() / 1000;
	}
	else if (player->data->truck.truck_id != 0)
	{
		send.ret = 190500310;
	}

	if (send.ret > 0)
	{
		fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_CHANGE_ZHENYING_ANSWER, ans_chose_zhenying__pack, send);
		return -3;
	}

	AddZhenyingPlayer tofr;
	add_zhenying_player__init(&tofr);
	tofr.name = player->get_name();
	tofr.zhenying = zhenYing;
	tofr.gold = player->get_comm_gold();
	tofr.zhenying_old = player->get_attr(PLAYER_ATTR_ZHENYING);
	tofr.fighting_capacity = player->get_attr(PLAYER_ATTR_FIGHTING_CAPACITY);
	tofr.kill = player->data->zhenying.kill;
	tofr.free = player->data->zhenying.free;
	conn_node_gamesrv::connecter.send_to_friend(extern_data, MSG_ID_CHANGE_ZHENYING_REQUEST, &tofr, (pack_func)add_zhenying_player__pack);

	return 0;
}
static int handle_server_chose_zhenying(player_struct *player, EXTERN_DATA *extern_data)
{
	AddZhenyingPlayer *req = add_zhenying_player__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	if (req->ret == 0)
	{
		player->set_attr(PLAYER_ATTR_ZHENYING, req->zhenying);
		AttrMap nty_list;
		nty_list[PLAYER_ATTR_ZHENYING] = player->get_attr(PLAYER_ATTR_ZHENYING);
		player->notify_attr(nty_list, true, false);
		player->add_task_progress(TCT_JOIN_CAMP, req->zhenying, 1);
		player->refresh_player_redis_info(false);
	}


	AnsChoseZhenying send;
	ans_chose_zhenying__init(&send);
	send.ret = req->ret;
	send.zhenying = req->zhenying;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_CHOSE_ZHENYING_ANSWER, ans_chose_zhenying__pack, send);
	add_zhenying_player__free_unpacked(req, NULL);

	return 0;
}
static int handle_server_change_zhenying(player_struct *player, EXTERN_DATA *extern_data)
{
	AddZhenyingPlayer *req = add_zhenying_player__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	if (req->ret == 0)
	{
		std::map<uint64_t, struct CampTable*>::iterator it = zhenying_base_config.begin();
		CampTable * config = it->second;
		if (player->data->zhenying.free > 0 && player->get_attr(PLAYER_ATTR_LEVEL) > config->FreeLv)
		{
			--player->data->zhenying.free;
		}
		else
		{
			int gold = req->gold;
			if (player->get_comm_gold() < gold)
			{
				gold = player->get_comm_gold();
			}
			player->sub_comm_gold(gold, MAGIC_TYPE_ZHENYING);
		}

		player->set_attr(PLAYER_ATTR_ZHENYING, req->zhenying);
		AttrMap nty_list;
		nty_list[PLAYER_ATTR_ZHENYING] = player->get_attr(PLAYER_ATTR_ZHENYING);
		player->notify_attr(nty_list, true, false);
		ParameterTable * table = get_config_by_id(161000162, &parameter_config);
		if (table != NULL)
		{
			player->data->zhenying.change_cd = time_helper::get_cached_time() / 1000 + table->parameter1[0];
		}
		player->add_task_progress(TCT_JOIN_CAMP, req->zhenying, 1);
		player->refresh_player_redis_info(false);
	}

	AnsChoseZhenying send;
	ans_chose_zhenying__init(&send);
	send.ret = req->ret;
	send.zhenying = req->zhenying;
	send.free = player->data->zhenying.free;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_CHANGE_ZHENYING_ANSWER, ans_chose_zhenying__pack, send);

	add_zhenying_player__free_unpacked(req, NULL);

	return 0;
}
static int handle_chose_zhenying_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	ChoseZhenying *req = chose_zhenying__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	int zhenYing = req->zhenying;
	chose_zhenying__free_unpacked(req, NULL);

	if (player->get_attr(PLAYER_ATTR_ZHENYING) != 0)
	{
		return -2;
	}
	std::map<uint64_t, struct CampTable*>::iterator it = zhenying_base_config.begin();
	CampTable * config = it->second;
	if (player->get_attr(PLAYER_ATTR_LEVEL) < config->Level)
	{
		return 1;
	}
	//player->set_attr(PLAYER_ATTR_ZHENYING, zhenYing);

	//AnsChoseZhenying send;
	//ans_chose_zhenying__init(&send);
	//send.ret = 0;
	//send.zhenying = zhenYing;
	//fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_CHOSE_ZHENYING_ANSWER, ans_chose_zhenying__pack, send);

	AddZhenyingPlayer tofr;
	add_zhenying_player__init(&tofr);
	tofr.name = player->get_name();
	tofr.zhenying = zhenYing;
	tofr.zhenying_old = player->get_attr(PLAYER_ATTR_ZHENYING);
	tofr.fighting_capacity = player->get_attr(PLAYER_ATTR_FIGHTING_CAPACITY);
	tofr.kill = player->data->zhenying.kill;
	tofr.free = player->data->zhenying.free;
	conn_node_gamesrv::connecter.send_to_friend(extern_data, MSG_ID_CHOSE_ZHENYING_REQUEST, &tofr, (pack_func)add_zhenying_player__pack);

	return 0;
}


static void gen_common_question(player_struct *player)
{
	player->data->common_answer.question = get_rand_question(sg_common_question);
	QuestionTable *table = get_config_by_id(player->data->common_answer.question, &questions_config);
	player->data->common_answer.answer[0] = 1;
	player->data->common_answer.answer[1] = 2;
	player->data->common_answer.answer[2] = 3;
	player->data->common_answer.answer[3] = 4;
	if (table != NULL)
	{
		int j = rand() % 3;
		uint32_t tmp = player->data->common_answer.answer[0];
		player->data->common_answer.answer[0] = player->data->common_answer.answer[j + 1];
		player->data->common_answer.answer[j + 1] = tmp;

		j = 0;
		for (int i = 0; i < MAX_QUESTION_ANSWER; ++i)
		{
			if (player->data->common_answer.answer[i] != table->RightAnseer)
			{
				player->data->common_answer.answer_tip[j++] = i + 1;
			}
		}
	}
}
static void on_login_send_question(player_struct *player, EXTERN_DATA *extern_data)
{
	if (player->data->common_answer.number == 0)
	{
		player->data->common_answer.number = 1;
		player->data->common_answer.tip = 3;
		player->data->common_answer.help = 3;
		gen_common_question(player);

		player->data->award_answer.number = 1;
		player->data->award_answer.trun = 1;
		player->data->award_answer.question = get_rand_question(sg_award_question);
	}

	AwardAnswer send;
	award_answer__init(&send);
	send.trun = player->data->award_answer.trun;
	send.npc = player->data->award_answer.npc;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_AWARD_QUESTION_NOTIFY, award_answer__pack, send);
}
static int handle_get_common_question_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	EventCalendarTable *tableAct = get_config_by_id(COMMON_QUESTION_ACTIVE_ID, &activity_config);
	if (tableAct == NULL)
	{
		return -3;
	}
	uint32_t cd = 0;
	if (check_active_open(tableAct->RelationID, cd))
	{
		if (player->data->common_answer.question == 0 && player->data->common_answer.next_open < time_helper::get_cached_time() / 1000)
		{
			player->data->common_answer.number = 1;
			player->data->common_answer.tip = 3;
			player->data->common_answer.help = 3;
			player->data->common_answer.contin = 0;
			player->data->common_answer.exp = 0;
			player->data->common_answer.money = 0;
			player->data->common_answer.right = 0;
			gen_common_question(player);
		}
	}

	CommonQuestion send;
	common_question__init(&send);
	send.question = player->data->common_answer.question;
	send.contin = player->data->common_answer.contin;
	send.right = player->data->common_answer.right;
	send.money = player->data->common_answer.money;
	send.exp = player->data->common_answer.exp;
	send.tip = player->data->common_answer.tip;
	send.help = player->data->common_answer.help;
	send.btip = player->data->common_answer.btip;
	send.bhelp = player->data->common_answer.bhelp;
	send.number = player->data->common_answer.number;
	send.answer = player->data->common_answer.answer;
	send.anstip = player->data->common_answer.answer_tip;
	send.n_answer = MAX_QUESTION_ANSWER;
	send.n_anstip = 2;
	send.cd = cd;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_GET_COMMON_QUESTION_ANSWER, common_question__pack, send);

	return 0;
}
static int handle_answer_common_question_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	ReqAnswer *req = req_answer__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint32_t answer = req->id;
	req_answer__free_unpacked(req, NULL);

	EventCalendarTable *tableAct = get_config_by_id(COMMON_QUESTION_ACTIVE_ID, &activity_config);
	if (tableAct == NULL)
	{
		return -3;
	}
	uint32_t cd = 0;
	if (!check_active_open(tableAct->RelationID, cd))
	{
		return -4;
	}

	QuestionTable *table = get_config_by_id(player->data->common_answer.question, &questions_config);
	ParameterTable *tableExp = get_config_by_id(161000199, &parameter_config);
	if (tableExp == NULL)
	{
		return -5;
	}
	//std::map<uint64_t, struct QuestionTable*>::iterator it = questions_config.find(player->data->common_answer.question);
	//if (it != questions_config.end())
	//{
	//	table = it->second;
	//}
	if (table == NULL)
	{
		return -2;
	}
	int rightAnswer = 0;
	if (answer == table->RightAnseer)
	{
		++player->data->common_answer.right;
		++player->data->common_answer.contin;
	}
	else
	{
		rightAnswer = 1;
		player->data->common_answer.contin = 0;
	}
	
	uint32_t contin = 1;
	if (player->data->common_answer.contin > contin)
	{
		contin = player->data->common_answer.contin;
	}
	uint32_t add = tableExp->parameter1[1] * player->get_attr(PLAYER_ATTR_LEVEL) * contin;
	
	player->add_exp(add, MAGIC_TYPE_QUESTION);
	player->data->common_answer.exp += add;

	add = tableExp->parameter1[0] * player->get_attr(PLAYER_ATTR_LEVEL) * contin;
	if(rightAnswer == 1)
	{
		add /= 2;
	}
	player->add_coin(add, MAGIC_TYPE_QUESTION);
	player->data->common_answer.money += add;

	if (player->data->common_answer.number == 20) //完成一次每日答题
	{
		player->data->common_answer.question = 0;
		player->check_activity_progress(AM_ANSWER, 1);
		player->add_task_progress(TCT_QUESTION_JOIN, 0, 1);
		player->data->common_answer.next_open = time_helper::get_cached_time() / 1000 + 3600 + cd;
	}
	else
	{
		++player->data->common_answer.number;
		gen_common_question(player);
	}
	player->data->common_answer.bhelp = false;
	player->data->common_answer.btip = false;

	send_comm_answer(MSG_ID_ANSWER_COMMON_QUESTION_ANSWER, rightAnswer, extern_data);
	return 0;
}
static int handle_common_question_hint_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	if (player->data->common_answer.btip || player->data->common_answer.tip == 0)
	{
		return 0;
	}

	player->data->common_answer.btip = true;
	--player->data->common_answer.tip;
	send_comm_answer(MSG_ID_COMMON_QUESTION_HINT_ANSWER, 0, extern_data);

	return 0;
}
static int handle_common_question_help_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	if (player->data->common_answer.bhelp || player->data->common_answer.help == 0)
	{
		return 0;
	}

	player->data->common_answer.bhelp = true;
	--player->data->common_answer.help;

	send_comm_answer(MSG_ID_COMMON_QUESTION_HELP_ANSWER, 0, extern_data);

	return 0;
}
static int handle_get_award_question_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	EventCalendarTable *table = get_config_by_id(AWARD_QUESTION_ACTIVE_ID, &activity_config);
	if (table == NULL)
	{
		return -2;
	}
	uint32_t cd = 0;
	if (!check_active_open(table->RelationID, cd))
	{
		return -4;
	}
	if (player->data->award_answer.question == 0)
	{
		return -5;
	}

	player->data->award_answer.begin_time = time_helper::get_cached_time() / 1000;
	player->data->award_answer.bOpenWin = true;

	AwardQuestion send;
	award_question__init(&send);
	send.question = player->data->award_answer.question;
	send.contin = player->data->award_answer.contin;
	send.right = player->data->award_answer.right;
	send.money = player->data->award_answer.money;
	send.exp = player->data->award_answer.exp;
	send.number = player->data->award_answer.number;
	send.trun = player->data->award_answer.trun;
	send.npc = player->data->award_answer.npc;
	send.timer = player->data->award_answer.timer;
	send.cd = cd;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_GET_AWARD_QUESTION_ANSWER, award_question__pack, send);

	return 0;
}
static int handle_answer_award_question_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	EventCalendarTable *tableE = get_config_by_id(AWARD_QUESTION_ACTIVE_ID, &activity_config);
	if (tableE == NULL)
	{
		return -2;
	}
	uint32_t cdAct = 0;
	if (!check_active_open(tableE->RelationID, cdAct))
	{
		return -4;
	}

	ReqAnswer *req = req_answer__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint32_t answer = req->id;
	bool timeout = req->timeout;
	req_answer__free_unpacked(req, NULL);

	QuestionTable *table = get_config_by_id(player->data->award_answer.question, &questions_config);
	if (table == NULL)
	{
		return -2;
	}
	ParameterTable *tableExp = get_config_by_id(161000199, &parameter_config);
	if (tableExp == NULL)
	{
		return -3;
	}
	int rightAnswer = 0;
	ParameterTable * config1 = get_config_by_id(161000170, &parameter_config);
	uint64_t cd = time_helper::get_cached_time() / 1000 - player->data->award_answer.begin_time;
	if (cd > config1->parameter1[0])
	{
		cd = config1->parameter1[0];
	}
	else
	{
		if (answer == table->RightAnseer)
		{
			++player->data->award_answer.right;
			++player->data->award_answer.contin;
		}
		else
		{
			cd = config1->parameter1[0];
			rightAnswer = 1;
			player->data->award_answer.contin = 0;
		}
	}
	if (timeout)
	{
		rightAnswer = 3;
	}
	send_comm_answer(MSG_ID_ANSWER_AWARD_QUESTION_ANSWER, rightAnswer, extern_data);

	uint32_t contin = 1;
	if (player->data->award_answer.contin > contin)
	{
		contin = player->data->award_answer.contin;
	}
	uint32_t add = tableExp->parameter1[1] * player->get_attr(PLAYER_ATTR_LEVEL) * contin;

	player->add_exp(table->Exp, MAGIC_TYPE_QUESTION);
	player->data->award_answer.exp += add;

	add = tableExp->parameter1[0] * player->get_attr(PLAYER_ATTR_LEVEL) * contin;
	if (rightAnswer == 1)
	{
		add /= 2;
	}
	player->add_coin(table->Coin, MAGIC_TYPE_QUESTION);
	player->data->award_answer.money += add;

	player->data->award_answer.timer += cd;
	ParameterTable * config2 = get_config_by_id(161000169, &parameter_config);
	if (player->data->award_answer.number == config2->parameter1[0])
	{
		ParameterTable * config3 = get_config_by_id(161000168, &parameter_config);
		if (player->data->award_answer.trun == config3->parameter1[0])
		{
			player->data->award_answer.question = 0;
		}
		else
		{
			player->data->award_answer.number = 1;
			++player->data->award_answer.trun;
			player->data->award_answer.question = get_rand_question(sg_award_question);
			//send_comm_answer(MSG_ID_NEXT_AWARD_QUESTION_NOTIFY, 0, extern_data);
			on_login_send_question(player, extern_data); //完成一次任务答题
			player->check_activity_progress(AM_ANSWER, 2);
			player->add_task_progress(TCT_QUESTION_JOIN, 0, 1);
		}
		player->data->award_answer.bOpenWin = false;
	}
	else
	{
		++player->data->award_answer.number;
		player->data->award_answer.question = get_rand_question(sg_award_question);
	}

	player->add_task_progress(TCT_QUESTION, 0, 1);

	return 0;
}
static int handle_interupt_award_question_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	EventCalendarTable *table = get_config_by_id(AWARD_QUESTION_ACTIVE_ID, &activity_config);
	if (table == NULL)
	{
		return -2;
	}
	uint32_t cd = 0;
	if (!check_active_open(table->RelationID, cd))
	{
		return -4;
	}

	player->logout_check_award_question();
	player->data->award_answer.bOpenWin = false;

	return 0;
}
static int handle_first_award_question_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	EventCalendarTable *table = get_config_by_id(AWARD_QUESTION_ACTIVE_ID, &activity_config);
	if (table == NULL)
	{
		return -2;
	}
	if (!player->activity_is_unlock(table->ID))
	{
		return -3;
	}
	uint32_t cd = 0;
	if (!check_active_open(table->RelationID, cd))
	{
		return -4;
	}

	if (player->data->award_answer.next_open < time_helper::get_cached_time() / 1000)
	{
		player->accept_task(table->AuxiliaryValue[0], false);
		player->data->award_answer.next_open = time_helper::get_cached_time() / 1000 + cd + 600;
		player->data->award_answer.question = get_rand_question(sg_award_question);
		player->data->award_answer.contin = 0;
		player->data->award_answer.exp = 0;
		player->data->award_answer.money = 0;
		player->data->award_answer.number = 1;
		player->data->award_answer.timer = 0;
		player->data->award_answer.trun = 1;
	}

	return 0;
}

static int on_login_send_live_skill(player_struct *player, EXTERN_DATA *extern_data)
{
	if (player->data->live_skill.level[LIVE__SKILL__TYPE__MEDICINE] == 0)
	{
		player->data->live_skill.level[LIVE__SKILL__TYPE__MEDICINE] = 1;
		player->data->live_skill.level[LIVE__SKILL__TYPE__COOK] = 1;
		ParameterTable *tablePar = get_config_by_id(161000175, &parameter_config);
		if (tablePar != NULL)
		{
			player->data->live_skill.book[LIVE__SKILL__TYPE__COOK] = tablePar->parameter1[0];
			player->data->live_skill.book[LIVE__SKILL__TYPE__MEDICINE] = tablePar->parameter1[0];
		}
		
		LifeSkillTable *table = get_medicine_table(LIVE__SKILL__TYPE__MEDICINE, 1);
		if (table != NULL)
		{
			player->data->live_skill.broken[LIVE__SKILL__TYPE__MEDICINE] = table->LvMax;
		}
		table = get_medicine_table(LIVE__SKILL__TYPE__COOK, 1);
		if (table != NULL)
		{
			player->data->live_skill.broken[LIVE__SKILL__TYPE__COOK] = table->LvMax;
		}

		for (std::map<uint64_t, struct BiaocheTable*>::iterator it = cash_truck_config.begin(); it != cash_truck_config.end(); ++it)
		{
			ControlTable *tableAct = get_config_by_id(it->second->ActivityControl, &all_control_config);
			if (it->second->Type == 1)
			{
				player->data->truck.num_coin = tableAct->RewardTime;
			}
			else
			{
				player->data->truck.num_gold = tableAct->RewardTime;
			}
		}
	}

	if (player->data->truck.truck_id != 0)
	{
		assert(player->data->truck.scene_id > 0);
		scene_struct *pScene = scene_manager::get_scene(player->data->truck.scene_id);
		if (pScene != NULL)
		{
			cash_truck_struct *truck = cash_truck_manager::create_cash_truck_at_pos(pScene, player->data->truck.active_id, *player, player->data->truck.pos.pos_x, player->data->truck.pos.pos_z, player->data->truck.hp);
			player->data->truck.truck_id = truck->get_uuid();
		}
	}

	CashTruckInfo note;
	cash_truck_info__init(&note);
	note.num_cion = player->data->truck.num_coin;
	note.num_gold = player->data->truck.num_gold;
	note.cash_truck = player->data->truck.truck_id;
	note.mapid = player->data->truck.scene_id;
	note.x = player->data->truck.pos.pos_x;
	note.z = player->data->truck.pos.pos_z;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_CASH_TRUCK_INFO_NOTIFY, cash_truck_info__pack, note);

	AnsLiveSkill send;
	ans_live_skill__init(&send);
	send.level = player->data->live_skill.level + 1;
	send.exp = player->data->live_skill.exp + 1;
	send.book = player->data->live_skill.book + 1;
	send.broken = player->data->live_skill.broken + 1;
	send.n_level = MAX_LIVE_SKILL_NUM;
	send.n_exp = MAX_LIVE_SKILL_NUM;
	send.n_book = MAX_LIVE_SKILL_NUM;
	send.n_broken = MAX_LIVE_SKILL_NUM;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_LIVE_SKILL_NOTIFY, ans_live_skill__pack, send);

	return 0;
}

static int handle_team_info_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	//时装
	on_login_send_all_fashion(player, extern_data);

	//技能
	player->m_skill.SendAllSkill();

	on_login_send_all_horse(player, extern_data);
	//on_login_send_yaoshi(player, extern_data);
	on_login_send_zhenying(player, extern_data);
	on_login_send_question(player, extern_data);
	on_login_send_live_skill(player, extern_data);

	//队伍
	if (player->m_team == NULL)
	{
		return 2;
	}
	player->m_team->SendWholeTeamInfo(*player);

	return 0;
}

//个人信息请求
static int handle_personality_info_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PersonalityInfoAnswer resp;
	personality_info_answer__init(&resp);

	PersonalityData personality_data;
	personality_data__init(&personality_data);

	resp.result = 0;
	resp.data = &personality_data;
	personality_data.sex = player->data->personality_sex;
	personality_data.birthday = player->data->personality_birthday;
	personality_data.location = player->data->personality_location;
	personality_data.textintro = player->data->personality_text_intro;
	personality_data.voiceintro = player->data->personality_voice_intro;
	personality_data.tags = player->data->personality_tags;
	personality_data.n_tags = 0;
	for (int i = 0; i < MAX_PERSONALITY_TAG_NUM; ++i)
	{
		if (player->data->personality_tags[i] == 0)
		{
			break;
		}
		personality_data.n_tags++;
	}

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PERSONALITY_INFO_ANSWER, personality_info_answer__pack, resp);

	return 0;
}

//设置普通信息请求
static int handle_personality_set_general_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PersonalitySetGeneralRequest *req = personality_set_general_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	int ret = 0;
	do
	{
		if (strlen(req->location) >= MAX_PERSONALITY_LOCATION_LEN)
		{
			ret = -1;
			break;
		}
		player->data->personality_sex = req->sex;
		player->data->personality_birthday = req->birthday;
		strcpy(player->data->personality_location, req->location);

		player->refresh_player_redis_info();
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PERSONALITY_SET_GENERAL_ANSWER, comm_answer__pack, resp);

	personality_set_general_request__free_unpacked(req, NULL);

	return 0;
}

//设置标签信息请求
static int handle_personality_set_tags_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PersonalitySetTagsRequest *req = personality_set_tags_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	int ret = 0;
	do
	{
		memset(player->data->personality_tags, 0, sizeof(player->data->personality_tags));
		for (size_t i = 0; i < req->n_tags && i < MAX_PERSONALITY_TAG_NUM; ++i)
		{
			player->data->personality_tags[i] = req->tags[i];
		}

		player->refresh_player_redis_info();
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PERSONALITY_SET_TAGS_ANSWER, comm_answer__pack, resp);

	personality_set_tags_request__free_unpacked(req, NULL);
	return 0;
}

//设置签名信息请求
static int handle_personality_set_intro_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PersonalitySetIntroRequest *req = personality_set_intro_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	int ret = 0;
	do
	{
		if (req->textintro)
		{
			if (strlen(req->textintro) >= MAX_PERSONALITY_TEXT_INTRO_LEN)
			{
				ret = -1;
				break;
			}
			strcpy(player->data->personality_text_intro, req->textintro);
			player->refresh_player_redis_info();
		}
		if (req->voiceintro)
		{
			if (strlen(req->voiceintro) >= MAX_PERSONALITY_VOICE_INTRO_LEN)
			{
				ret = -1;
				break;
			}
			strcpy(player->data->personality_voice_intro, req->voiceintro);
		}
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PERSONALITY_SET_INTRO_ANSWER, comm_answer__pack, resp);

	personality_set_intro_request__free_unpacked(req, NULL);

	return 0;
}

void answer_get_other_info(EXTERN_DATA *extern_data, int result, player_struct *target)
{
	GetOtherInfoAnswer resp;
	get_other_info_answer__init(&resp);

	resp.result = result;
	if (result != 0)
	{
		fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_GET_OTHER_INFO_ANSWER, get_other_info_answer__pack, resp);
		return ;
	}

	OtherDetailData detail_data;
	other_detail_data__init(&detail_data);

	PersonalityData personality_data;
	personality_data__init(&personality_data);

	AttrData player_attr[PLAYER_ATTR_MAX];
	AttrData* player_attr_point[PLAYER_ATTR_MAX];

	EquipData equip_data[MAX_EQUIP_NUM];
	EquipData* equip_data_point[MAX_EQUIP_NUM];
	EquipEnchantData enchant_data[MAX_EQUIP_NUM][MAX_EQUIP_ENCHANT_NUM];
	EquipEnchantData* enchant_data_point[MAX_EQUIP_NUM][MAX_EQUIP_ENCHANT_NUM];
	AttrData cur_attr[MAX_EQUIP_NUM][MAX_EQUIP_ENCHANT_NUM];
	AttrData* cur_attr_point[MAX_EQUIP_NUM][MAX_EQUIP_ENCHANT_NUM];
	AttrData rand_attr[MAX_EQUIP_NUM][MAX_EQUIP_ENCHANT_NUM][MAX_EQUIP_ENCHANT_RAND_NUM];
	AttrData* rand_attr_point[MAX_EQUIP_NUM][MAX_EQUIP_ENCHANT_NUM][MAX_EQUIP_ENCHANT_RAND_NUM];

	BaguapaiDressData dress_data[MAX_BAGUAPAI_STYLE_NUM];
	BaguapaiDressData* dress_data_point[MAX_BAGUAPAI_STYLE_NUM];
	BaguapaiCardData card_data[MAX_BAGUAPAI_STYLE_NUM][MAX_BAGUAPAI_DRESS_NUM];
	BaguapaiCardData* card_data_point[MAX_BAGUAPAI_STYLE_NUM][MAX_BAGUAPAI_DRESS_NUM];
	AttrData attr_data[MAX_BAGUAPAI_STYLE_NUM][MAX_BAGUAPAI_DRESS_NUM][MAX_BAGUAPAI_MINOR_ATTR_NUM];
	AttrData* attr_data_point[MAX_BAGUAPAI_STYLE_NUM][MAX_BAGUAPAI_DRESS_NUM][MAX_BAGUAPAI_MINOR_ATTR_NUM];
	AttrData attr_new_data[MAX_BAGUAPAI_STYLE_NUM][MAX_BAGUAPAI_DRESS_NUM][MAX_BAGUAPAI_MINOR_ATTR_NUM];
	AttrData* attr_new_data_point[MAX_BAGUAPAI_STYLE_NUM][MAX_BAGUAPAI_DRESS_NUM][MAX_BAGUAPAI_MINOR_ATTR_NUM];

	resp.data = &detail_data;
	detail_data.playerid = target->data->player_id;
	detail_data.name = target->data->name;

	std::vector<uint32_t> attrIds;
	attrIds.push_back(PLAYER_ATTR_LEVEL);
	attrIds.push_back(PLAYER_ATTR_JOB);
	attrIds.push_back(PLAYER_ATTR_HEAD);
	attrIds.push_back(PLAYER_ATTR_CLOTHES);
	attrIds.push_back(PLAYER_ATTR_CLOTHES_COLOR_UP);
	attrIds.push_back(PLAYER_ATTR_CLOTHES_COLOR_DOWN);
	attrIds.push_back(PLAYER_ATTR_HAT);
	attrIds.push_back(PLAYER_ATTR_HAT_COLOR);
	attrIds.push_back(PLAYER_ATTR_WEAPON);
	attrIds.push_back(PLAYER_ATTR_BAGUA);
	attrIds.push_back(PLAYER_ATTR_ZHENYING);
	attrIds.push_back(PLAYER_ATTR_FIGHTING_CAPACITY);
	detail_data.n_attrs = 0;
	detail_data.attrs = player_attr_point;
	for (std::vector<uint32_t>::iterator iter = attrIds.begin(); iter != attrIds.end(); ++iter)
	{
		player_attr_point[detail_data.n_attrs] = &player_attr[detail_data.n_attrs];
		attr_data__init(&player_attr[detail_data.n_attrs]);
		player_attr[detail_data.n_attrs].id = *iter;
		player_attr[detail_data.n_attrs].val = target->get_attr(*iter);
		detail_data.n_attrs++;
	}

	detail_data.personality = &personality_data;
	personality_data.sex = target->data->personality_sex;
	personality_data.sex = target->data->personality_sex;
	personality_data.birthday = target->data->personality_birthday;
	personality_data.location = target->data->personality_location;
	personality_data.textintro = target->data->personality_text_intro;
	personality_data.voiceintro = target->data->personality_voice_intro;
	personality_data.tags = target->data->personality_tags;
	personality_data.n_tags = 0;
	for (int i = 0; i < MAX_PERSONALITY_TAG_NUM; ++i)
	{
		if (target->data->personality_tags[i] == 0)
		{
			break;
		}
		personality_data.n_tags++;
	}

	size_t equip_num = 0;
	for (int k = 0; k < MAX_EQUIP_NUM; ++k)
	{
		EquipInfo &equip_info = target->data->equip_list[k];
		if (equip_info.stair == 0)
		{
			continue;
		}

		equip_data_point[equip_num] = &equip_data[equip_num];
		equip_data__init(&equip_data[equip_num]);

		equip_data[equip_num].type = k + 1;
		equip_data[equip_num].stair = equip_info.stair;
		equip_data[equip_num].starlv = equip_info.star_lv;
		equip_data[equip_num].starexp = equip_info.star_exp;
		size_t enchant_num = 0;
		for (int i = 0; i < MAX_EQUIP_ENCHANT_NUM; ++i)
		{
			EquipEnchantInfo &enchant_info = equip_info.enchant[i];

			enchant_data_point[equip_num][enchant_num] = &enchant_data[equip_num][enchant_num];
			equip_enchant_data__init(&enchant_data[equip_num][enchant_num]);
			enchant_data[equip_num][enchant_num].index = i;

			cur_attr_point[equip_num][enchant_num] = &cur_attr[equip_num][enchant_num];
			attr_data__init(&cur_attr[equip_num][enchant_num]);
			cur_attr[equip_num][enchant_num].id = enchant_info.cur_attr.id;
			cur_attr[equip_num][enchant_num].val = enchant_info.cur_attr.val;
			enchant_data[equip_num][enchant_num].curattr = cur_attr_point[equip_num][enchant_num];

			size_t rand_num = 0;
			if (enchant_info.rand_attr[0].id > 0)
			{
				for (int j = 0; j < MAX_EQUIP_ENCHANT_RAND_NUM; ++j)
				{
					rand_attr_point[equip_num][enchant_num][rand_num] = &rand_attr[equip_num][enchant_num][rand_num];
					attr_data__init(&rand_attr[equip_num][enchant_num][rand_num]);
					rand_attr[equip_num][enchant_num][rand_num].id = enchant_info.rand_attr[j].id;
					rand_attr[equip_num][enchant_num][rand_num].val = enchant_info.rand_attr[j].val;
					rand_num++;
				}
			}
			enchant_data[equip_num][enchant_num].randattr = rand_attr_point[equip_num][enchant_num];
			enchant_data[equip_num][enchant_num].n_randattr = rand_num;

			enchant_num++;
		}
		equip_data[equip_num].n_enchant = enchant_num;
		equip_data[equip_num].enchant = enchant_data_point[equip_num];

		equip_data[equip_num].inlay = &equip_info.inlay[0];
		equip_data[equip_num].n_inlay = MAX_EQUIP_INLAY_NUM;
		equip_num++;
	}
	detail_data.equips = equip_data_point;
	detail_data.n_equips = equip_num;

	uint32_t style_num = 0;
	for (int i = 0; i < MAX_BAGUAPAI_STYLE_NUM; ++i)
	{
		dress_data_point[style_num] = &dress_data[style_num];
		baguapai_dress_data__init(&dress_data[style_num]);
		dress_data[style_num].styleid = i + 1;

		uint32_t card_num = 0;
		for (int j = 0; j < MAX_BAGUAPAI_DRESS_NUM; ++j)
		{
			card_data_point[style_num][card_num] = &card_data[style_num][card_num];
			baguapai_card_data__init(&card_data[style_num][card_num]);
			card_data[style_num][card_num].id = target->data->baguapai_dress[i].card_list[j].id;
			card_data[style_num][card_num].star = target->data->baguapai_dress[i].card_list[j].star;
			card_data[style_num][card_num].mainattrval = target->data->baguapai_dress[i].card_list[j].main_attr_val;
			card_data[style_num][card_num].mainattrvalnew = target->data->baguapai_dress[i].card_list[j].main_attr_val_new;

			uint32_t cur_num = 0;
			for (int k = 0; k < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++k)
			{
				if (target->data->baguapai_dress[i].card_list[j].minor_attrs[k].id == 0)
				{
					break;
				}

				attr_data_point[style_num][card_num][cur_num] = &attr_data[style_num][card_num][cur_num];
				attr_data__init(&attr_data[style_num][card_num][cur_num]);
				attr_data[style_num][card_num][cur_num].id = target->data->baguapai_dress[i].card_list[j].minor_attrs[k].id;
				attr_data[style_num][card_num][cur_num].val = target->data->baguapai_dress[i].card_list[j].minor_attrs[k].val;
				cur_num++;
			}
			card_data[style_num][card_num].n_minorattrs = cur_num;
			card_data[style_num][card_num].minorattrs = attr_data_point[style_num][card_num];

			uint32_t new_num = 0;
			for (int k = 0; k < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++k)
			{
				if (target->data->baguapai_dress[i].card_list[j].minor_attrs_new[k].id == 0)
				{
					break;
				}

				attr_new_data_point[style_num][card_num][new_num] = &attr_new_data[style_num][card_num][new_num];
				attr_data__init(&attr_new_data[style_num][card_num][new_num]);
				attr_new_data[style_num][card_num][new_num].id = target->data->baguapai_dress[i].card_list[j].minor_attrs_new[k].id;
				attr_new_data[style_num][card_num][new_num].val = target->data->baguapai_dress[i].card_list[j].minor_attrs_new[k].val;
				new_num++;
			}
			card_data[style_num][card_num].n_minorattrsnew = new_num;
			card_data[style_num][card_num].minorattrsnew = attr_new_data_point[style_num][card_num];

			card_num++;
		}
		dress_data[style_num].n_cards = card_num;
		dress_data[style_num].cards = card_data_point[style_num];

		style_num++;
	}
	detail_data.n_baguas = style_num;
	detail_data.baguas = dress_data_point;

	detail_data.teamid = target->data->teamid;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_GET_OTHER_INFO_ANSWER, get_other_info_answer__pack, resp);
}

//查看他人个人信息请求
static int handle_get_other_info_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	GetOtherInfoRequest *req = get_other_info_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint64_t target_id = req->playerid;

	get_other_info_request__free_unpacked(req, NULL);

	player_struct *target = player_manager::get_player_by_id(target_id);
	if (target)
	{
		answer_get_other_info(extern_data, 0, target);
	}
	else
	{
		conn_node_gamesrv::connecter.transfer_to_dbsrv();
	}

	return 0;
}

//推荐好友请求
static int handle_friend_recommend_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PROTO_FRIEND_RECOMMEND *proto = (PROTO_FRIEND_RECOMMEND*)get_send_buf(SERVER_PROTO_FRIEND_RECOMMEND, 0);
	memset(proto->head.data, 0, sizeof(PROTO_FRIEND_RECOMMEND) - sizeof(PROTO_HEAD));
	proto->head.len = ENDION_FUNC_4(sizeof(PROTO_FRIEND_RECOMMEND));

	std::set<uint64_t> playerIds;
	//同一场景地图玩家
	if (player->scene)
	{
		scene_struct *scene = player->scene;
		scene->get_all_player(playerIds);
	}
	//同帮会在线成员
	if (player->data->guild_id > 0)
	{
		for (std::map<uint64_t, player_struct *>::iterator iter = player_manager_all_players_id.begin(); iter != player_manager_all_players_id.end(); ++iter)
		{
			player_struct *tmp_player = iter->second;
			if (tmp_player->data->guild_id == player->data->guild_id)
			{
				playerIds.insert(iter->first);
			}
		}
	}

	for (std::set<uint64_t>::iterator iter = playerIds.begin(); iter != playerIds.end() && proto->player_num < MAX_FRIEND_RECOMMEND_PLAYER; ++iter)
	{
		//剔除自己
		if (*iter == extern_data->player_id)
		{
			continue;
		}
		proto->player_id[proto->player_num] = *iter;
		proto->player_num++;
	}

	add_extern_data(&proto->head, extern_data);

	if (conn_node_gamesrv::connecter.send_one_msg(&proto->head, 1) != (int)ENDION_FUNC_4(proto->head.len))
	{
		LOG_ERR("[%s:%d] send to client failed err[%d]", __FUNCTION__, __LINE__, errno);
	}

	return 0;
}

void answer_friend_search(EXTERN_DATA *extern_data, int result, player_struct *target, uint32_t logout_time)
{
	FriendSearchAnswer resp;
	friend_search_answer__init(&resp);

	FriendPlayerBriefData friend_data;
	friend_player_brief_data__init(&friend_data);

	resp.result = result;
	if (result == 0 && target != NULL)
	{
		resp.data = &friend_data;
		friend_data.playerid = target->data->player_id;
		friend_data.name = target->data->name;
		friend_data.job = target->get_job();
		friend_data.level = target->get_level();
		friend_data.head = target->get_attr(PLAYER_ATTR_HEAD);
		friend_data.offlinetime = logout_time;
		friend_data.closeness = 0;
		friend_data.tags = target->data->personality_tags;
		friend_data.n_tags = 0;
		for (int i = 0; i < MAX_PERSONALITY_TAG_NUM; ++i)
		{
			if (target->data->personality_tags[i] == 0)
			{
				break;
			}
			friend_data.n_tags++;
		}
		friend_data.textintro = target->data->personality_text_intro;
	}

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_FRIEND_SEARCH_ANSWER, friend_search_answer__pack, resp);
}

//搜索好友请求
static int handle_friend_search_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	FriendSearchRequest *req = friend_search_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	int ret = 0;
	do
	{
		if (req->playerid == player->get_uuid() || strcmp(req->playername, player->data->name) == 0)
		{
			ret = ERROR_ID_FRIEND_SEARCH_SELF;
			break;
		}

		bool bFind = false;
		if (req->playerid > 0)
		{
			player_struct *target = player_manager::get_player_by_id(req->playerid);
			if (target)
			{
				answer_friend_search(extern_data, 0, target, 0);
				bFind = true;
			}
		}

		if (!bFind)
		{
			conn_node_gamesrv::connecter.transfer_to_dbsrv();
		}
	} while(0);

	if (ret != 0)
	{
		answer_friend_search(extern_data, ret, NULL, 0);
	}

	friend_search_request__free_unpacked(req, NULL);

	return 0;
}

//扩展好友上限应答
static int handle_friend_extend_contact_answer(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	BagUseRequest *req = bag_use_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t pos = req->index;
	uint32_t use_all = req->use_all;

	bag_use_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		if (!(pos < player->data->bag_grid_num))
		{
			ret = ERROR_ID_BAG_POS;
			LOG_ERR("[%s:%d] player[%lu] bag pos error, pos:%u, grid_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, pos, player->data->bag_grid_num);
			break;
		}

		bag_grid_data& grid = player->data->bag[pos];
		player->add_task_progress(TCT_USE_PROP, grid.id, 1);
		uint32_t del_num = (use_all == 0 ? 1 : grid.num);
		player->del_item_by_pos(pos, del_num, MAGIC_TYPE_BAG_USE);
	} while(0);

	BagUseAnswer resp;
	bag_use_answer__init(&resp);
	resp.result = ret;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_BAG_USE_ANSWER, bag_use_answer__pack, resp);

	return 0;
}

//好友服请求扣除消耗
static int handle_friendsrv_check_and_cost_request(player_struct *player, EXTERN_DATA *extern_data)
{
	return handle_srv_check_and_cost_request(player, extern_data, SERVER_PROTO_FRIENDSRV_COST_ANSWER);
}

static int handle_xunbao_pos_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("%s: player[%lu] set", __FUNCTION__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	BagUseRequest *req = bag_use_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint32_t pos = req->index;
	bag_use_request__free_unpacked(req, NULL);

	if (!(pos < player->data->bag_grid_num))
	{
		LOG_ERR("[%s:%d] player[%lu] bag pos error, pos:%u, grid_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, pos, player->data->bag_grid_num);
		return -2;
	}
	if (player->data->bag[pos].num < 1)
	{
		return -3;
	}
	if (player->sight_space != NULL)
	{
		return -8;
	}

	ItemsConfigTable *prop_config = get_config_by_id(player->data->bag[pos].id, &item_config);
	if (prop_config == NULL)
	{
		return -5;
	}
	if (player->get_attr(PLAYER_ATTR_LEVEL) < prop_config->ItemLevel)
	{
		return -6;
	}

	SearchTable *sTable = get_config_by_id(player->data->bag[pos].id, &sg_xunbao);
	if (sTable == NULL)
	{
		return -4;
	}

	uint32_t empty_num = 0;
	for (uint32_t i = 0; i < player->data->bag_grid_num; ++i)
	{
		bag_grid_data& grid = player->data->bag[i];
		if (grid.id == 0)
		{
			empty_num++;
		}
	}
	if (empty_num < 3)
	{
		player->send_system_notice(190500284, NULL);
	}

	uint32_t mapId = 0;
	uint32_t x = 0;
	uint32_t z = 0;
	TreasureTable *tTable = NULL;
	std::map<uint32_t, uint32_t>::iterator itItem = player->xun_map_id.find(player->data->bag[pos].id);
	if (itItem == player->xun_map_id.end())
	{
		mapId = sTable->TreasureId[rand() % sTable->n_TreasureId];
		//uint32_t mapId = sTable->TreasureId[0];
		std::map<uint64_t, std::vector<uint64_t> >::iterator it = sg_xunbao_map.find(mapId);
		if (it == sg_xunbao_map.end())
		{
			return -5;
		}
		tTable = get_config_by_id(it->second.at(rand() % it->second.size()), &xunbao_map_config);
		if (tTable == NULL)
		{
			return -7;
		}
		player->xun_map_id[player->data->bag[pos].id] = tTable->ID;
	}
	else
	{
		tTable = get_config_by_id(itItem->second, &xunbao_map_config);
		if (tTable == NULL)
		{
			return -7;
		}
	}

	x = tTable->PointX / 100;
	z = tTable->PointZ / 100;
	mapId = tTable->MapId;

	XunbaoPos send;
	xunbao_pos__init(&send);
	send.mapid = mapId;
	send.x = x;
	send.z = z;
	send.transfer = tTable->TransferPoint;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_XUNBAO_POS_ANSWER, xunbao_pos__pack, send);

	return 0;
}

static int handle_auto_add_hp_set_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("%s: player[%lu] set", __FUNCTION__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	AutoAddHpSetData *data = auto_add_hp_set_data__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!data)
	{
		LOG_ERR("%s: player[%lu] can not unpack", __FUNCTION__, extern_data->player_id);
		return (-10);
	}

	player->data->auto_add_hp_item_id = data->auto_add_hp_item_id;
	player->data->auto_add_hp_percent = data->auto_add_hp_percent;
	player->data->open_auto_add_hp = data->open_auto_add_hp;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_AUTO_ADD_HP_SET_ANSWER, auto_add_hp_set_data__pack, *data);

	auto_add_hp_set_data__free_unpacked(data, NULL);
	return (0);
}

//好友服送礼消耗请求
static int handle_friend_gift_cost_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);

	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PROTO_COST_FRIEND_GIFT_REQ *req = (PROTO_COST_FRIEND_GIFT_REQ*)buf_head();
	uint32_t item_id = req->item_id;
	uint32_t item_num = req->gift_num;

	SRV_COST_INFO cost;
	memset(&cost, 0, sizeof(SRV_COST_INFO));
	int ret = 0;
	uint32_t add_closeness = 0;
	uint32_t send_item_id = 0;
	do
	{
		ItemsConfigTable *config = get_config_by_id(item_id, &item_config);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get itemconfig failed, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, item_id);
			break;
		}

		bool can_use_bind = false;
		if (config->ItemEffect == 9) //好感度道具，可以使用绑定道具
		{
			can_use_bind = true;
		}
		else //其他道具，只能使用非绑定道具
		{
			can_use_bind = false;
		}

		uint32_t item_bind_id = 0, item_unbind_id = 0;
		if (config->BindType == 0) //道具本身是非绑定道具
		{
			item_bind_id = config->ItemRelation;
			item_unbind_id = item_id;
		}
		else //道具是绑定道具
		{
			item_bind_id = item_id;
			item_unbind_id = config->ItemRelation;
		}

		uint32_t item_bind_num = player->get_item_num_by_id(item_bind_id);
		uint32_t item_unbind_num = player->get_item_num_by_id(item_unbind_id);
		uint32_t has_num = (can_use_bind ? item_bind_num + item_unbind_num : item_unbind_num);
		if (has_num < item_num)
		{
			uint32_t replace_num = item_num - has_num;
			uint32_t need_money = replace_num * req->currency_val;
			uint32_t has_money = 0;
			uint32_t error_id = 0;
			switch(req->currency_type)
			{
				case 1: //银币
					has_money = player->get_coin();
					error_id = ERROR_ID_COIN_NOT_ENOUGH;
					break;
				case 2: //绑定元宝，不够可以消耗非绑定
					has_money = player->get_comm_gold();
					error_id = ERROR_ID_GOLD_NOT_ENOUGH;
					break;
				case 3: //元宝
					has_money = player->get_attr(PLAYER_ATTR_GOLD);
					error_id = ERROR_ID_GOLD_NOT_ENOUGH;
					break;
			}

			if (has_money < need_money)
			{
				ret = error_id;
				break;
			}

			if (can_use_bind)
			{
				if (item_bind_num > 0)
				{
					player->del_item_by_id(item_bind_id, item_bind_num, MAGIC_TYPE_FRIEND_GIFT);
				}
				if (item_unbind_num > 0)
				{
					player->del_item_by_id(item_unbind_id, item_unbind_num, MAGIC_TYPE_FRIEND_GIFT);
				}
			}

			switch(req->currency_type)
			{
				case 1: //银币
					player->sub_coin(need_money, MAGIC_TYPE_FRIEND_GIFT);
					cost.coin = need_money;
					break;
				case 2: //绑定元宝，不够可以消耗非绑定
					{
						uint32_t has_bind_gold = player->get_attr(PLAYER_ATTR_BIND_GOLD);
						uint32_t has_unbind_gold = player->get_attr(PLAYER_ATTR_GOLD);
						player->sub_comm_gold(need_money, MAGIC_TYPE_FRIEND_GIFT);
						cost.gold = has_bind_gold - player->get_attr(PLAYER_ATTR_BIND_GOLD);
						cost.unbind_gold = has_unbind_gold - player->get_attr(PLAYER_ATTR_GOLD);
					}
					break;
				case 3: //元宝
					player->sub_unbind_gold(need_money, MAGIC_TYPE_FRIEND_GIFT);
					cost.unbind_gold = need_money;
					break;
			}
		}
		else
		{
			if (can_use_bind)
			{
				player->del_item(item_id, item_num, MAGIC_TYPE_FRIEND_GIFT);
			}
			else
			{
				player->del_item_by_id(item_unbind_id, item_num, MAGIC_TYPE_FRIEND_GIFT);
			}
		}

		uint32_t cost_bind_num = item_bind_num - player->get_item_num_by_id(item_bind_id);
		uint32_t cost_unbind_num = item_unbind_num - player->get_item_num_by_id(item_unbind_id);
		uint32_t item_idx = 0;
		if (cost_bind_num > 0)
		{
			cost.item_id[item_idx] = item_bind_id;
			cost.item_num[item_idx] = cost_bind_num;
			item_idx++;
		}
		if (cost_unbind_num > 0)
		{
			cost.item_id[item_idx] = item_unbind_id;
			cost.item_num[item_idx] = cost_unbind_num;
			item_idx++;
		}

		if (config->ItemEffect == 9)
		{
			if (config->n_ParameterEffect > 0)
			{
				add_closeness = config->ParameterEffect[0] * item_num;
			}
		}
		else
		{
			send_item_id = item_bind_id;
		}

		cost.statis_id = MAGIC_TYPE_FRIEND_GIFT;
	} while(0);

	PROTO_COST_FRIEND_GIFT_RES *res = (PROTO_COST_FRIEND_GIFT_RES *)get_send_buf(SERVER_PROTO_FRIEND_GIFT_COST_ANSWER, get_seq());
	res->head.len = ENDION_FUNC_4(sizeof(PROTO_COST_FRIEND_GIFT_RES));
	memset(res->head.data, 0, sizeof(PROTO_COST_FRIEND_GIFT_RES) - sizeof(PROTO_HEAD));
	res->result = ret;
	res->target_id = req->target_id;
	res->gift_id = req->gift_id;
	res->gift_num = req->gift_num;
	res->item_id = send_item_id;
	res->add_closeness = add_closeness;
	memcpy(&res->cost, &cost, sizeof(SRV_COST_INFO));
	add_extern_data(&res->head, extern_data);
	if (conn_node_gamesrv::connecter.send_one_msg(&res->head, 1) != (int)ENDION_FUNC_4(res->head.len))
	{
		LOG_ERR("[%s:%d] send to friendsrv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}

	return 0;
}

//好友服接收礼物
static int handle_friend_add_gift_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);

	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	FriendGiftData *req = friend_gift_data__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	std::map<uint32_t, uint32_t> item_map;
	item_map[req->item->id] = req->item->num;
	std::vector<char *> args;
	args.push_back(req->playername);
	player->add_item_list_otherwise_send_mail(item_map, MAGIC_TYPE_FRIEND_RECEIVE_GIFT, 270300031, &args);

	friend_gift_data__free_unpacked(req, NULL);

	return 0;
}

//好友服同步好友数
static int handle_friend_sync_friend_num(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);

	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PROTO_HEAD *head = (PROTO_HEAD*)buf_head();
	uint32_t *pData = (uint32_t*)head->data;
	uint32_t friend_num = *pData;

	player->data->friend_num = friend_num;
	player->add_task_progress(TCT_FRIEND_NUM, 0, friend_num);

	return 0;
}

//伙伴信息请求
static int handle_partner_info_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	notify_partner_info(player, extern_data);
	return 0;
}

static int notify_partner_info(player_struct *player, EXTERN_DATA *extern_data)
{
	PartnerInfoAnswer resp;
	partner_info_answer__init(&resp);

	PartnerData  partner_data[MAX_PARTNER_NUM];
	PartnerAttr partner_cur_attr[MAX_PARTNER_NUM];
	PartnerAttr partner_cur_flash[MAX_PARTNER_NUM];
	PartnerData* partner_point[MAX_PARTNER_NUM];
	AttrData  attr_data[MAX_PARTNER_NUM][MAX_PARTNER_ATTR];
	AttrData* attr_point[MAX_PARTNER_NUM][MAX_PARTNER_ATTR];
	PartnerSkillData  skill_data[MAX_PARTNER_NUM][MAX_PARTNER_SKILL_NUM];
	PartnerSkillData* skill_point[MAX_PARTNER_NUM][MAX_PARTNER_SKILL_NUM];
	PartnerSkillData  partner_skill_data_flash[MAX_PARTNER_NUM][MAX_PARTNER_SKILL_NUM];
	PartnerSkillData* partner_skill_point_flash[MAX_PARTNER_NUM][MAX_PARTNER_SKILL_NUM];

	resp.result = 0;

	uint32_t partner_num = 0;
	for (PartnerMap::iterator iter = player->m_partners.begin(); iter != player->m_partners.end() && partner_num < MAX_PARTNER_NUM; ++iter)
	{
		partner_struct *partner = iter->second;
		if (partner == NULL)
		{
			continue;
		}

		partner_point[partner_num] = &partner_data[partner_num];
		partner_data__init(&partner_data[partner_num]);
		partner_data[partner_num].uuid = partner->data->uuid;
		partner_data[partner_num].partnerid = partner->data->partner_id;
		partner_data[partner_num].relivetime = partner->data->relive_time;
		
		uint32_t attr_num = 0;
		for (int i = 1; i < MAX_PARTNER_ATTR; ++i)
		{
			attr_point[partner_num][attr_num] = &attr_data[partner_num][attr_num];
			attr_data__init(&attr_data[partner_num][attr_num]);
			attr_data[partner_num][attr_num].id = i;
			attr_data[partner_num][attr_num].val = partner->data->attrData[i];
			attr_num++;
		}
		partner_data[partner_num].attrs = attr_point[partner_num];
		partner_data[partner_num].n_attrs = attr_num;

		uint32_t skill_num = 0;
		if (partner->data->attr_cur.base_attr_id[0] != 0)
		{
			partner_data[partner_num].cur_attr = partner_cur_attr + partner_num;
			partner_attr__init(partner_cur_attr + partner_num);
			for (int i = 0; i < MAX_PARTNER_SKILL_NUM; ++i)
			{
				skill_point[partner_num][skill_num] = &skill_data[partner_num][skill_num];
				partner_skill_data__init(&skill_data[partner_num][skill_num]);
				skill_data[partner_num][skill_num].id = partner->data->attr_cur.skill_list[i].skill_id;
				skill_data[partner_num][skill_num].lv = partner->data->attr_cur.skill_list[i].lv;
				skill_num++;
			}
			partner_cur_attr[partner_num].skills = skill_point[partner_num];
			partner_cur_attr[partner_num].n_skills = skill_num;
			partner_cur_attr[partner_num].base_attr_id = partner->data->attr_cur.base_attr_id;
			partner_cur_attr[partner_num].n_base_attr_id = MAX_PARTNER_BASE_ATTR;
			partner_cur_attr[partner_num].base_attr_cur = partner->data->attr_cur.base_attr_vaual;
			partner_cur_attr[partner_num].n_base_attr_cur = MAX_PARTNER_BASE_ATTR;
			partner_cur_attr[partner_num].base_attr_up = partner->data->attr_cur.base_attr_up;
			partner_cur_attr[partner_num].n_base_attr_up = MAX_PARTNER_BASE_ATTR;
			partner_cur_attr[partner_num].base_attr_up = partner->data->attr_cur.base_attr_up;
			partner_cur_attr[partner_num].n_base_attr_up = MAX_PARTNER_BASE_ATTR;
			partner_cur_attr[partner_num].detail_attr_id = partner->data->attr_cur.detail_attr_id;
			partner_cur_attr[partner_num].n_detail_attr_id = partner->data->attr_cur.n_detail_attr;
			partner_cur_attr[partner_num].detail_attr_cur = partner->data->attr_cur.detail_attr_vaual;
			partner_cur_attr[partner_num].n_detail_attr_cur = partner->data->attr_cur.n_detail_attr;
			partner_cur_attr[partner_num].type = partner->data->attr_cur.type;
		}
		
		if (partner->data->attr_flash.base_attr_id[0] != 0)
		{
			partner_data[partner_num].flash_attr = partner_cur_flash + partner_num;
			partner_attr__init(partner_cur_flash + partner_num);
			skill_num = 0;
			for (int i = 0; i < MAX_PARTNER_SKILL_NUM; ++i)
			{
				partner_skill_point_flash[partner_num][skill_num] = &partner_skill_data_flash[partner_num][skill_num];
				partner_skill_data__init(&partner_skill_data_flash[partner_num][skill_num]);
				partner_skill_data_flash[partner_num][skill_num].id = partner->data->attr_flash.skill_list[i].skill_id;
				partner_skill_data_flash[partner_num][skill_num].lv = partner->data->attr_flash.skill_list[i].lv;
				skill_num++;
			}
			partner_cur_flash[partner_num].skills = partner_skill_point_flash[partner_num];
			partner_cur_flash[partner_num].n_skills = skill_num;
			partner_cur_flash[partner_num].base_attr_id = partner->data->attr_flash.base_attr_id;
			partner_cur_flash[partner_num].n_base_attr_id = MAX_PARTNER_BASE_ATTR;
			partner_cur_flash[partner_num].base_attr_cur = partner->data->attr_flash.base_attr_vaual;
			partner_cur_flash[partner_num].n_base_attr_cur = MAX_PARTNER_BASE_ATTR;
			partner_cur_flash[partner_num].base_attr_up = partner->data->attr_flash.base_attr_up;
			partner_cur_flash[partner_num].n_base_attr_up = MAX_PARTNER_BASE_ATTR;
			partner_cur_flash[partner_num].base_attr_up = partner->data->attr_flash.base_attr_up;
			partner_cur_flash[partner_num].n_base_attr_up = MAX_PARTNER_BASE_ATTR;
			partner_cur_flash[partner_num].detail_attr_id = partner->data->attr_flash.detail_attr_id;
			partner_cur_flash[partner_num].n_detail_attr_id = partner->data->attr_flash.n_detail_attr;
			partner_cur_flash[partner_num].detail_attr_cur = partner->data->attr_flash.detail_attr_vaual;
			partner_cur_flash[partner_num].n_detail_attr_cur = partner->data->attr_flash.n_detail_attr;
			partner_cur_flash[partner_num].type = partner->data->attr_flash.type;
			partner_data[partner_num].power = partner->data->attr_flash.power_refresh;
		}
		
		partner_data[partner_num].god_id = partner->data->god_id;
		partner_data[partner_num].n_god_id = partner->data->n_god;
		partner_data[partner_num].god_level = partner->data->god_level;
		partner_data[partner_num].n_god_level = partner->data->n_god;

		partner_num++;
	}
	resp.partners = partner_point;
	resp.n_partners = partner_num;

	resp.formation = player->data->partner_formation;
	resp.n_formation = MAX_PARTNER_FORMATION_NUM;

	resp.dictionary = player->data->partner_dictionary;
	resp.n_dictionary = 0;
	for (int i = 0; i < MAX_PARTNER_TYPE; ++i)
	{
		if (player->data->partner_dictionary[i] == 0)
		{
			break;
		}
		resp.n_dictionary++;
	}

	resp.recruitjuniortime = player->data->partner_recruit_junior_time;
	resp.recruitjuniorcount = player->data->partner_recruit_junior_count;
	resp.recruitseniortime = player->data->partner_recruit_senior_time;
	resp.recruitseniorcount = player->data->partner_recruit_senior_count;

	resp.bonds = player->data->partner_bond;
	resp.n_bonds = 0;
	for (int i = 0; i < MAX_PARTNER_BOND_NUM; ++i)
	{
		if (player->data->partner_bond[i] == 0)
		{
			break;
		}
		resp.n_bonds++;
	}

	resp.bondreward = player->data->partner_bond_reward;
	resp.n_bondreward = 0;
	for (int i = 0; i < MAX_PARTNER_TYPE; ++i)
	{
		if (player->data->partner_bond_reward[i] == 0)
		{
			break;
		}
		resp.n_bondreward++;
	}

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PARTNER_INFO_ANSWER, partner_info_answer__pack, resp);
	return 0;
}

//伙伴翻转开关请求 
static int handle_partner_turn_switch_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PartnerTurnSwitchRequest *req = partner_turn_switch_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t type = req->type;

	partner_turn_switch_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		if (!(type == PLAYER_ATTR_PARTNER_FIGHT || type == PLAYER_ATTR_PARTNER_PRECEDENCE))
		{
			ret = -1;
			LOG_ERR("[%s:%d] player[%lu] set type %u failed.", __FUNCTION__, __LINE__, extern_data->player_id, type);
			break;
		}

		uint32_t cur_val = player->get_attr(type);
		if (cur_val == 0)
		{
			player->set_attr(type, 1);
		}
		else
		{
			player->set_attr(type, 0);
		}

		player->notify_one_attr_changed(type, player->get_attr(type));
		player->adjust_battle_partner();
		if (type == PLAYER_ATTR_PARTNER_FIGHT)
		{
			player->calculate_attribute(true);
		}
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PARTNER_TURN_SWITCH_ANSWER, comm_answer__pack, resp);

	return 0;
}

//伙伴布阵请求
static int handle_partner_formation_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PartnerFormationRequest *req = partner_formation_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint64_t partner_uuid = req->uuid;
	uint32_t pos = req->position;

	partner_formation_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		partner_struct *partner = player->get_partner_by_uuid(partner_uuid);
		if (!partner)
		{
			ret = ERROR_ID_PARTNER_UUID;
			LOG_ERR("[%s:%d] player[%lu] can't find, partner:%lu, pos:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid, pos);
			break;
		}

		if (!(pos <= MAX_PARTNER_FORMATION_NUM))
		{
			ret = ERROR_ID_PARTNER_POS;
			LOG_ERR("[%s:%d] player[%lu] pos error, partner:%lu, pos:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid, pos);
			break;
		}

		int old_pos = -1;
		for (int i = 0; i < MAX_PARTNER_FORMATION_NUM; ++i)
		{
			if (player->data->partner_formation[i] == partner_uuid)
			{
				old_pos = i;
				break;
			}
		}

		if (pos == 0)
		{ //下阵
			if (old_pos < 0)
			{
				break;
			}

			player->data->partner_formation[old_pos] = 0;
		}
		else
		{
			uint64_t pos_partner_uuid = player->data->partner_formation[pos - 1];
			player->data->partner_formation[pos - 1] = partner_uuid;
			player->data->partner_formation[old_pos] = pos_partner_uuid;
		}

		partner->mark_bind();

		player->adjust_battle_partner();
		if (player->is_partner_battle())
		{
			player->calculate_attribute(true);
		}
	} while(0);

	PartnerFormationAnswer resp;
	partner_formation_answer__init(&resp);

	resp.result = ret;
	resp.uuids = player->data->partner_formation;
	resp.n_uuids = MAX_PARTNER_FORMATION_NUM;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PARTNER_FORMATION_ANSWER, partner_formation_answer__pack, resp);

	return 0;
}

//伙伴学习技能请求
static int handle_partner_learn_skill_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PartnerLearnSkillRequest *req = partner_learn_skill_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint64_t partner_uuid = req->uuid;
	uint32_t skill_idx = req->index;
	uint32_t book_id = req->bookid;

	partner_learn_skill_request__free_unpacked(req, NULL);

	int ret = 0;
	PartnerSkill *pSkill = NULL;
	do
	{
		partner_struct *partner = player->get_partner_by_uuid(partner_uuid);
		if (!partner)
		{
			ret = ERROR_ID_PARTNER_UUID;
			LOG_ERR("[%s:%d] player[%lu] can't find, partner:%lu, skill_idx:%u, book_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid, skill_idx, book_id);
			break;
		}

		int real_idx = (int)skill_idx - 1;
		if (!(real_idx >= 0 && real_idx < MAX_PARTNER_SKILL_NUM))
		{
			ret = ERROR_ID_PARTNER_SKILL_INDEX;
			LOG_ERR("[%s:%d] player[%lu] skill index error, partner:%lu, skill_idx:%u, book_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid, skill_idx, book_id);
			break;
		}

		uint32_t need_book_num = 1;
		uint32_t has_book_num = player->get_item_num_by_id(book_id);
		if (has_book_num < need_book_num)
		{
			ret = ERROR_ID_PROP_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] skill book not enough, partner:%lu, skill_idx:%u, book_id:%u, need_num:%u, has_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid, skill_idx, book_id, need_book_num, has_book_num);
			break;
		}

		ItemsConfigTable *book_config = get_config_by_id(book_id, &item_config);
		if (!book_config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get itemconfig failed, partner:%lu, skill_idx:%u, book_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid, skill_idx, book_id);
			break;
		}

		if (book_config->n_ParameterEffect < 1)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] param num error, partner:%lu, skill_idx:%u, book_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid, skill_idx, book_id);
			break;
		}

		uint32_t skill_id = book_config->ParameterEffect[0];
		if (get_config_by_id(skill_id, &skill_config) == NULL)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] param value error, not skill, partner:%lu, skill_idx:%u, book_id:%u, skill_id:%u",
				__FUNCTION__, __LINE__, extern_data->player_id, partner_uuid, skill_idx, book_id, skill_id);
			break;
		}

		pSkill = &partner->data->attr_cur.skill_list[real_idx];
		if (pSkill->skill_id == skill_id)
		{ //技能升级
			if (get_skill_level_config(skill_id, pSkill->lv + 1) == NULL)
			{
				ret = ERROR_ID_PARTNER_SKILL_LEVEL_MAX;
				LOG_ERR("[%s:%d] player[%lu] skill level max, partner:%lu, skill_idx:%u, book_id:%u, skill_id:%u, cur_level:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid, skill_idx, book_id, skill_id, pSkill->lv);
				break;
			}

			pSkill->lv++;
		}
		else
		{
			bool has_same_skill = false;
			for (int i = 0; i < MAX_PARTNER_SKILL_NUM; ++i)
			{
				if (i == real_idx)
				{
					continue;
				}

				if (partner->data->attr_cur.skill_list[i].skill_id > 0 && partner->data->attr_cur.skill_list[i].skill_id == skill_id)
				{
					has_same_skill = true;
					break;
				}
			}

			if (has_same_skill)
			{
				ret = ERROR_ID_PARTNER_CANT_LEARN_SAME_SKILL;
				LOG_ERR("[%s:%d] player[%lu] skill same, partner:%lu, skill_idx:%u, book_id:%u, skill_id:%u, cur_level:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid, skill_idx, book_id, skill_id, pSkill->lv);
				break;
			}

			pSkill->skill_id = skill_id;
			pSkill->lv = 1;
		}

		partner->mark_bind();
		player->del_item_by_id(book_id, need_book_num, MAGIC_TYPE_PARTNER_LEARN_SKILL);
	} while(0);

	PartnerLearnSkillAnswer resp;
	partner_learn_skill_answer__init(&resp);

	PartnerSkillData skill_data;
	partner_skill_data__init(&skill_data);

	resp.result = ret;
	resp.uuid = partner_uuid;
	resp.index = skill_idx;
	if (ret == 0 && pSkill != NULL)
	{
		resp.skill = &skill_data;
		skill_data.id = pSkill->skill_id;
		skill_data.lv = pSkill->lv;
	}

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PARTNER_LEARN_SKILL_ANSWER, partner_learn_skill_answer__pack, resp);

	return 0;
}

//伙伴使用经验丹请求
static int handle_partner_use_exp_item_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PartnerUseExpItemRequest *req = partner_use_exp_item_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint64_t partner_uuid = req->uuid;
	uint32_t item_id = req->itemid;

	partner_use_exp_item_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		partner_struct *partner = player->get_partner_by_uuid(partner_uuid);
		if (!partner)
		{
			ret = ERROR_ID_PARTNER_UUID;
			LOG_ERR("[%s:%d] player[%lu] can't find, partner:%lu, item_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid, item_id);
			break;
		}

		uint32_t need_item_num = 1;
		uint32_t has_item_num = player->get_item_num_by_id(item_id);
		if (has_item_num < need_item_num)
		{
			ret = ERROR_ID_PROP_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] skill book not enough, partner:%lu, item_id:%u, need_num:%u, has_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid, item_id, need_item_num, has_item_num);
			break;
		}

		ItemsConfigTable *prop_config = get_config_by_id(item_id, &item_config);
		if (!prop_config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get itemconfig failed, partner:%lu, item_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid, item_id);
			break;
		}

		if (prop_config->ItemEffect != 15)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] item effect error, partner:%lu, item_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid, item_id);
			break;
		}

		if (prop_config->n_ParameterEffect < 1)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] param num error, partner:%lu, item_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid, item_id);
			break;
		}

		uint32_t exp_val = prop_config->ParameterEffect[0];
		if (exp_val == 0)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] param val error, partner:%lu, item_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid, item_id);
			break;
		}

		uint32_t player_level = player->get_level();
		uint32_t partner_level = partner->get_level();
		if (partner_level >= player_level)
		{
			ret = ERROR_ID_PARTNER_LEVEL_LIMIT;
			LOG_ERR("[%s:%d] player[%lu] partner level limit, partner:%lu, item_id:%u, partner_lv:%u, player_lv:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid, item_id, partner_level, player_level);
			break;
		}

		if (!get_partner_level_config(partner_level + 1))
		{
			ret = ERROR_ID_PARTNER_LEVEL_MAX;
			LOG_ERR("[%s:%d] player[%lu] partner level max, partner:%lu, item_id:%u, partner_lv:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid, item_id, partner_level);
			break;
		}

		player->del_item(item_id, need_item_num, MAGIC_TYPE_PARTNER_USE_EXP_ITEM);

		partner->add_exp(exp_val, MAGIC_TYPE_PARTNER_USE_EXP_ITEM, player_level, true);
		partner->mark_bind();
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PARTNER_USE_EXP_ITEM_ANSWER, comm_answer__pack, resp);

	return 0;
}

//伙伴遣散请求
static int handle_partner_dismiss_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PartnerDismissRequest *req = partner_dismiss_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}



	int ret = 0;
	do
	{
		for (size_t i = 0; i < req->n_uuids; ++i)
		{
			uint64_t partner_uuid = req->uuids[i];
			partner_struct *partner = player->get_partner_by_uuid(partner_uuid);
			if (!partner)
			{
				ret = ERROR_ID_PARTNER_UUID;
				LOG_ERR("[%s:%d] player[%lu] can't find, partner:%lu", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid);
				break;
			}

			if (player->partner_is_in_formation(partner_uuid))
			{
				ret = ERROR_ID_PARTNER_IN_FORMATION;
				LOG_ERR("[%s:%d] player[%lu] in formation, partner:%lu", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid);
				break;
			}
		}

		if (ret != 0)
		{
			break;
		}

		std::map<uint32_t, uint32_t> recycle_item_map;
		for (size_t i = 0; i < req->n_uuids; ++i)
		{
			uint64_t partner_uuid = req->uuids[i];
			partner_struct *partner = player->get_partner_by_uuid(partner_uuid);
			PartnerTable *config = get_config_by_id(partner->data->partner_id, &partner_config);
			if (config)
			{
				for (uint32_t j = 0; j < config->n_SeveranceItem; ++j)
				{
					recycle_item_map[config->SeveranceItem[j]] += config->SeveranceNum[j];
				}
			}
		}

		if (!player->check_can_add_item_list(recycle_item_map))
		{
			ret = ERROR_ID_BAG_GRID_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] bag grid not enough", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		for (size_t i = 0; i < req->n_uuids; ++i)
		{
			uint64_t partner_uuid = req->uuids[i];
			player->remove_partner(partner_uuid);
		}
		player->add_item_list(recycle_item_map, MAGIC_TYPE_PARTNER_DISMISS);
	} while(0);


	PartnerDismissAnswer resp;
	partner_dismiss_answer__init(&resp);

	resp.result = ret;
	resp.uuids = req->uuids;
	resp.n_uuids = req->n_uuids;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PARTNER_DISMISS_ANSWER, partner_dismiss_answer__pack, resp);

	partner_dismiss_request__free_unpacked(req, NULL);

	return 0;
}

//伙伴兑换请求
static int handle_partner_exchange_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PartnerExchangeRequest *req = partner_exchange_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t partner_id = req->partnerid;

	partner_exchange_request__free_unpacked(req, NULL);

	int ret = 0;
	uint64_t uuid = 0;
	do
	{
		bool in_dictionary = false;
		for (int i = 0; i < MAX_PARTNER_TYPE; ++i)
		{
			if (player->data->partner_dictionary[i] == partner_id)
			{
				in_dictionary = true;
				break;
			}
		}

		if (!in_dictionary)
		{
			ret = ERROR_ID_PARTNER_NOT_IN_DICTIONARY;
			LOG_ERR("[%s:%d] player[%lu] not in dictionary, partner_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_id);
			break;
		}

		PartnerTable *config = get_config_by_id(partner_id, &partner_config);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get config failed, partner_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_id);
			break;
		}

		if (config->n_Recruit < 2)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] param num error, partner_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_id);
			break;
		}

		uint32_t item_id = config->Recruit[0];
		uint32_t need_item_num = config->Recruit[1];
		uint32_t has_item_num = player->get_item_num_by_id(item_id);
		if (has_item_num < need_item_num)
		{
			ret = ERROR_ID_PROP_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] item not enough, partner_id:%u, item_id:%u, need_num:%u, has_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_id, item_id, need_item_num, has_item_num);
			break;
		}

		if (player->m_partners.size() >= (size_t)MAX_PARTNER_NUM)
		{
			ret = ERROR_ID_PARTNER_NUM_MAX;
			LOG_ERR("[%s:%d] player[%lu] partner num max, partner_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_id);
			break;
		}

		ret = player->add_partner(partner_id, &uuid);
		if (ret != 0)
		{
			ret = ERROR_ID_PARTNER_NUM_MAX;
			LOG_ERR("[%s:%d] player[%lu] partner num max, partner_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_id);
			break;
		}

		player->del_item(item_id, need_item_num, MAGIC_TYPE_PARTNER_EXCHANGE);
	} while(0);

	PartnerExchangeAnswer resp;
	partner_exchange_answer__init(&resp);

	resp.result = ret;
	resp.uuid = uuid;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PARTNER_EXCHANGE_ANSWER, partner_exchange_answer__pack, resp);

	return 0;
}

enum
{
	PARTNER_RECRUIT_JUNIOR_ONE = 1,
	PARTNER_RECRUIT_JUNIOR_REPEAT = 2,
	PARTNER_RECRUIT_SENIOR_ONE = 3,
	PARTNER_RECRUIT_SENIOR_REPEAT = 4,
};

struct PartnerRecruitResult
{
	uint32_t type;
	union
	{
		uint64_t uuid;
		struct
		{
			uint32_t id;
			uint32_t num;
		} item;
	} data;
};

#define MAX_PARTNER_RECRUIT_NUM 5

//伙伴招募请求
static int handle_partner_recruit_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PartnerRecruitRequest *req = partner_recruit_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t type = req->type;

	partner_recruit_request__free_unpacked(req, NULL);

	int ret = 0;
	std::vector<PartnerRecruitResult> results;
	do
	{
		RecruitTable *config = get_partner_recruit_config(type);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] recruit type error, type:%u", __FUNCTION__, __LINE__, extern_data->player_id, type);
			break;
		}

		uint32_t *pRecruitCount = NULL;
		uint32_t *pRecruitTime = NULL;
		if (type == PARTNER_RECRUIT_JUNIOR_ONE || type == PARTNER_RECRUIT_JUNIOR_REPEAT)
		{
			pRecruitTime = &player->data->partner_recruit_junior_time;
			pRecruitCount = &player->data->partner_recruit_junior_count;
		}
		else if (type == PARTNER_RECRUIT_SENIOR_ONE || type == PARTNER_RECRUIT_SENIOR_REPEAT)
		{
			pRecruitTime = &player->data->partner_recruit_senior_time;
			pRecruitCount = &player->data->partner_recruit_senior_count;
		}

		if (!pRecruitCount || !pRecruitTime)
		{
			ret = ERROR_ID_PARTNER_RECRUIT_TYPE;
			LOG_ERR("[%s:%d] player[%lu] recruit type error, type:%u", __FUNCTION__, __LINE__, extern_data->player_id, type);
			break;
		}

		uint32_t need_coin = 0, need_gold = 0, need_time = 0;
		uint32_t now = time_helper::get_cached_time() / 1000;
		if ((type == PARTNER_RECRUIT_JUNIOR_ONE || type == PARTNER_RECRUIT_SENIOR_ONE) && (*pRecruitTime == 0 || *pRecruitTime <= now))
		{
			need_time = config->Time;
		}
		else
		{
			if (config->ConsumeType == 1)
			{ //银币
				need_coin = config->ConsumeNum;
			}
			else if (config->ConsumeType == 2)
			{ //元宝
				need_gold = config->ConsumeNum;
			}
		}

		if (need_coin > 0)
		{
			uint32_t has_coin = player->get_coin();
			if (has_coin < need_coin)
			{
				ret = ERROR_ID_COIN_NOT_ENOUGH;
				LOG_ERR("[%s:%d] player[%lu] coin not enough, type:%u, need_coin:%u, has_coin:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, need_coin, has_coin);
				break;
			}
		}
		if (need_gold > 0)
		{
			uint32_t has_gold = player->get_comm_gold();
			if (has_gold < need_gold)
			{
				ret = ERROR_ID_GOLD_NOT_ENOUGH;
				LOG_ERR("[%s:%d] player[%lu] gold not enough, type:%u, need_gold:%u, has_gold:%u", __FUNCTION__, __LINE__, extern_data->player_id, type, need_gold, has_gold);
				break;
			}
		}

		if (player->m_partners.size() + config->RecruitNum > MAX_PARTNER_NUM)
		{
			ret = ERROR_ID_PARTNER_NUM_MAX;
			LOG_ERR("[%s:%d] player[%lu] partner num, type:%u, cur_num:%lu", __FUNCTION__, __LINE__, extern_data->player_id, type, player->m_partners.size());
			break;
		}

		std::map<uint32_t, uint32_t> award_item_map;
		for (uint32_t i = 0; i + 1 < config->n_GetItem; i = i+2)
		{
			award_item_map[config->GetItem[i]] = config->GetItem[i + 1];
		}
//		if (award_item_map.size() > 0 && !player->check_can_add_item_list(award_item_map))
//		{
//			ret = ERROR_ID_BAG_GRID_NOT_ENOUGH;
//			LOG_ERR("[%s:%d] player[%lu] bag space not enough, type:%u", __FUNCTION__, __LINE__, extern_data->player_id, type);
//			break;
//		}

		for (uint64_t i = 0; i < config->RecruitNum; ++i)
		{
			(*pRecruitCount)++;
			if ((*pRecruitCount) % config->BottomTime == 0)
			{ //触发保底，必出伙伴
				*pRecruitCount = 0;
				uint64_t total_rate = 0;
				for (uint32_t j = 0; j < config->n_RecruitBottom; ++j)
				{
					total_rate += config->RecruitBottom[j];
				}

				if (total_rate == 0)
				{
					continue;
				}

				uint64_t rand_val = rand() % total_rate;
				uint64_t rate_count = 0;
				for (uint32_t j = 0; j < config->n_RecruitBottom; ++j)
				{
					if (rand_val >= rate_count && rand_val < rate_count + config->RecruitBottom[j])
					{
						//按概率随机出伙伴的品级，然后找出对应品级的伙伴，再平均随机出一个
						std::vector<uint64_t> rand_vec;
						for (uint32_t k = 0; k < config->n_Recruit; ++k)
						{
							PartnerTable *tmp_config = get_config_by_id(config->Recruit[k], &partner_config);
							if (tmp_config && tmp_config->Grade == config->BottomGrade[j])
							{
								rand_vec.push_back(config->Recruit[k]);
							}
						}

						if (rand_vec.empty())
						{
							break;
						}

						rand_val = rand() % rand_vec.size();
						uint32_t partner_id = rand_vec[rand_val];

						PartnerRecruitResult info;
						info.type = 2;
						info.data.uuid = 0;
						player->add_partner(partner_id, &info.data.uuid);
						results.push_back(info);
						break;
					}

					rate_count += config->RecruitBottom[j];
				}
			}
			else
			{
				//先随机到底是道具还是伙伴
				uint64_t total_rate = 0;
				for (uint32_t j = 0; j < config->n_TypeProbability; ++j)
				{
					total_rate += config->TypeProbability[j];
				}

				if (total_rate == 0)
				{
					continue;
				}

				uint64_t rand_val = rand() % total_rate;
				uint64_t rate_count = 0;
				uint32_t result_type = 0;
				for (uint32_t j = 0; j < config->n_TypeProbability; ++j)
				{
					if (rand_val >= rate_count && rand_val < rate_count + config->TypeProbability[j])
					{
						result_type = j + 1;
						break;
					}

					rate_count += config->TypeProbability[j];
				}

				if (result_type == 1)
				{
					if (config->n_Item == 0)
					{
						continue;
					}

					rand_val = rand() % config->n_Item;
					PartnerRecruitResult info;
					info.type = result_type;
					info.data.item.id = config->Item[rand_val];
					info.data.item.num = config->ItemNum[rand_val];
					results.push_back(info);
					award_item_map[config->Item[rand_val]] += config->ItemNum[rand_val];
				}
				else if (result_type == 2)
				{
					total_rate = 0;
					for (uint32_t j = 0; j < config->n_RecruitProbability; ++j)
					{
						total_rate += config->RecruitProbability[j];
					}

					if (total_rate == 0)
					{
						continue;
					}

					uint64_t rand_val = rand() % total_rate;
					uint64_t rate_count = 0;
					for (uint32_t j = 0; j < config->n_RecruitProbability; ++j)
					{
						if (rand_val >= rate_count && rand_val < rate_count + config->RecruitProbability[j])
						{
							//按概率随机出伙伴的品级，然后找出对应品级的伙伴，再平均随机出一个
							std::vector<uint64_t> rand_vec;
							for (uint32_t k = 0; k < config->n_Recruit; ++k)
							{
								PartnerTable *tmp_config = get_config_by_id(config->Recruit[k], &partner_config);
								if (tmp_config && tmp_config->Grade == config->RecruitGrade[j])
								{
									rand_vec.push_back(config->Recruit[k]);
								}
							}

							if (rand_vec.empty())
							{
								break;
							}

							rand_val = rand() % rand_vec.size();
							uint32_t partner_id = rand_vec[rand_val];

							PartnerRecruitResult info;
							info.type = 2;
							info.data.uuid = 0;
							player->add_partner(partner_id, &info.data.uuid);
							results.push_back(info);
							break;
						}

						rate_count += config->RecruitProbability[j];
					}
				}
			}
		}

		if (need_coin > 0)
		{
			player->sub_coin(need_coin, MAGIC_TYPE_PARTNER_RECRUIT);
		}
		if (need_gold > 0)
		{
			player->sub_comm_gold(need_gold, MAGIC_TYPE_PARTNER_RECRUIT);
		}
		if (need_time > 0)
		{
			*pRecruitTime = now + need_time;
		}
		if (award_item_map.size() > 0)
		{
			player->add_item_list_otherwise_send_mail(award_item_map, MAGIC_TYPE_PARTNER_RECRUIT, 270300030, NULL);
		}
		player->add_task_progress(TCT_PARTNER_RECRUIT, 0, config->RecruitNum);
	} while(0);

	PartnerRecruitAnswer resp;
	partner_recruit_answer__init(&resp);

	PartnerRecruitResultData result_data[MAX_PARTNER_RECRUIT_NUM];
	PartnerRecruitResultData* result_point[MAX_PARTNER_RECRUIT_NUM];
	ItemData  item_data[MAX_PARTNER_RECRUIT_NUM];

	resp.result = ret;
	resp.recruitjuniortime = player->data->partner_recruit_junior_time;
	resp.recruitjuniorcount = player->data->partner_recruit_junior_count;
	resp.recruitseniortime = player->data->partner_recruit_senior_time;
	resp.recruitseniorcount = player->data->partner_recruit_senior_count;
	for (size_t i = 0; i < results.size() && i < MAX_PARTNER_RECRUIT_NUM; ++i)
	{
		result_point[i] = &result_data[i];
		partner_recruit_result_data__init(&result_data[i]);
		result_data[i].type = results[i].type;
		if (results[i].type == 1)
		{
			result_data[i].item = &item_data[i];
			item_data__init(&item_data[i]);
			item_data[i].id = results[i].data.item.id;
			item_data[i].num = results[i].data.item.num;
		}
		else
		{
			result_data[i].has_uuid = true;
			result_data[i].uuid = results[i].data.uuid;
		}
	}
	resp.recruits = result_point;
	resp.n_recruits = results.size();

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PARTNER_RECRUIT_ANSWER, partner_recruit_answer__pack, resp);

	return 0;
}

static int handle_partner_dead_finish_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	player->adjust_battle_partner();

	return 0;
}

static int handle_partner_bond_active_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PartnerBondActiveRequest *req = partner_bond_active_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t bond_id = req->bondid;

	partner_bond_active_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		FetterTable *bond_config = get_config_by_id(bond_id, &partner_bond_config);
		if (!bond_config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get config failed, bond_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, bond_id);
			break;
		}

		int bond_idx = -1;
		for (int i = 0; i < MAX_PARTNER_BOND_NUM; ++i)
		{
			if (player->data->partner_bond[i] == 0)
			{
				bond_idx = i;
				break;
			}
			if (player->data->partner_bond[i] == bond_id)
			{
				bond_idx = i;
				break;
			}
		}

		if (bond_idx < 0)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] bond memory, bond_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, bond_id);
			break;
		}

		if (player->data->partner_bond[bond_idx] != 0)
		{
			ret = ERROR_ID_PARTNER_BOND_ACTIVED;
			LOG_ERR("[%s:%d] player[%lu] bond is actived, bond_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, bond_id);
			break;
		}

		if (bond_config->Relation == 0)
		{ //激活
			bool dictionary_active = true;
			for (uint32_t i = 0; i < bond_config->n_Partner; ++i)
			{
				if (!player->partner_dictionary_is_active(bond_config->Partner[i]))
				{
					dictionary_active = false;
					break;
				}
			}

			if (!dictionary_active)
			{
				ret = ERROR_ID_PARTNER_BOND_ACTIVE_CONDITION;
				LOG_ERR("[%s:%d] player[%lu] partner dictionary, bond_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, bond_id);
				break;
			}
		}
		else
		{ //升级
			if (!player->partner_bond_is_active(bond_config->Relation))
			{
				ret = ERROR_ID_PARTNER_BOND_ACTIVE_CONDITION;
				LOG_ERR("[%s:%d] player[%lu] partner bond, bond_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, bond_id);
				break;
			}

			uint32_t need_item_num = bond_config->ItemNum;
			uint32_t has_item_num = player->get_item_can_use_num(bond_config->LevelItem);
			if (has_item_num < need_item_num)
			{
				ret = ERROR_ID_PARTNER_BOND_ACTIVE_CONDITION;
				LOG_ERR("[%s:%d] player[%lu] item, bond_id:%u, item_id:%u, item_num:%u, has_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, bond_id, (uint32_t)bond_config->LevelItem, need_item_num, has_item_num);
				break;
			}
		}

		if (bond_config->Relation != 0)
		{
			player->del_item(bond_config->LevelItem, bond_config->ItemNum, MAGIC_TYPE_PARTNER_BOND_ACTIVE);
		}

		player->data->partner_bond[bond_idx] = bond_id;
		player->calculate_attribute(true);
	} while(0);

	PartnerBondActiveAnswer resp;
	partner_bond_active_answer__init(&resp);

	resp.result = ret;
	resp.bondid = bond_id;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PARTNER_BOND_ACTIVE_ANSWER, partner_bond_active_answer__pack, resp);

	return 0;
}

static int handle_partner_bond_reward_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PartnerBondRewardRequest *req = partner_bond_reward_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	uint32_t partner_id = req->partnerid;

	partner_bond_reward_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		PartnerTable *config = get_config_by_id(partner_id, &partner_config);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get config failed, partner_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_id);
			break;
		}

		int index = -1;
		for (int i = 0; i < MAX_PARTNER_TYPE; ++i)
		{
			if (player->data->partner_bond_reward[i] == 0)
			{
				index = i;
				break;
			}

			if (player->data->partner_bond_reward[i] == partner_id)
			{
				index = i;
				break;
			}
		}

		if (index < 0)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] bond reward memory, partner_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_id);
			break;
		}

		if (player->data->partner_bond_reward[index] > 0)
		{
			ret = ERROR_ID_PARTNER_BOND_REWARD_GET;
			LOG_ERR("[%s:%d] player[%lu] bond reward get, partner_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_id);
			break;
		}

		bool reward_can_get = true;
		for (uint32_t i = 0; i < config->n_Fetter; ++i)
		{
			if (!player->partner_bond_is_active(config->Fetter[i]))
			{
				reward_can_get = false;
				break;
			}
		}

		if (!reward_can_get)
		{
			ret = ERROR_ID_PARTNER_BOND_REWARD_CONDITION;
			LOG_ERR("[%s:%d] player[%lu] bond reward condition, partner_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_id);
			break;
		}

		std::map<uint32_t, uint32_t> award_map;
		award_map.insert(std::make_pair(config->Reward, 1));
		if (!player->check_can_add_item_list(award_map))
		{
			ret = ERROR_ID_BAG_GRID_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] bag not enough, partner_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, partner_id);
			break;
		}

		player->add_item_list(award_map, MAGIC_TYPE_PARTNER_BOND_REWARD);
		player->data->partner_bond_reward[index] = partner_id;
	} while(0);

	PartnerBondRewardAnswer resp;
	partner_bond_reward_answer__init(&resp);

	resp.result = ret;
	resp.partnerid = partner_id;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PARTNER_BOND_REWARD_ANSWER, partner_bond_reward_answer__pack, resp);

	return 0;
}

static int handle_partner_compose_stone_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PartnerComposeStoneRequest *req = partner_compose_stone_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	std::map<uint32_t, uint32_t> stone_map;
	for (size_t i = 0; i < req->n_stones; ++i)
	{
		stone_map[req->stones[i]->id] += req->stones[i]->num;
	}

	partner_compose_stone_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		uint32_t stone_type = 0, stone_score = 0;
		for (std::map<uint32_t, uint32_t>::iterator iter = stone_map.begin(); iter != stone_map.end(); ++iter)
		{
			uint32_t item_id = iter->first;
			uint32_t item_num = iter->second;
			ItemsConfigTable *config = get_config_by_id(item_id, &item_config);
			if (!config)
			{
				ret = ERROR_ID_NO_CONFIG;
				LOG_ERR("[%s:%d] player[%lu] get config failed, item_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, item_id);
				break;
			}

			if (stone_type == 0)
			{
				stone_type = config->ItemType;
			}
			else
			{
				if (stone_type != (uint32_t)config->ItemType)
				{
					ret = ERROR_ID_PARTNER_COMPOSE_STONE_TYPE;
					break;
				}
			}

			uint32_t has_num = player->get_item_can_use_num(item_id);
			if (has_num < item_num)
			{
				ret = ERROR_ID_PROP_NOT_ENOUGH;
				LOG_ERR("[%s:%d] player[%lu] prop not enough, item_id:%u, need_num:%u, has_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, item_id, item_num, has_num);
				break;
			}

			if (config->n_ParameterEffect >= 1)
			{
				stone_score += config->ParameterEffect[0] * item_num;
			}
		}

		if (ret != 0)
		{
			break;
		}

		uint32_t product_id = 0, product_score = 0, product_coin = 0;
		if (stone_type == 12)
		{
			product_id = sg_partner_sanshenshi_id;
			product_score = sg_partner_sanshenshi_score;
			product_coin = sg_partner_sanshenshi_coin;
		}
		else if (stone_type == 13)
		{
			product_id = sg_partner_qiyaoshi_id;
			product_score = sg_partner_qiyaoshi_score;
			product_coin = sg_partner_qiyaoshi_coin;
		}
		else
		{
			ret = ERROR_ID_NO_CONFIG;
			break;
		}

		if (stone_score < product_score)
		{
			ret = ERROR_ID_PARTNER_COMPOSE_STONE_SCORE;
			LOG_ERR("[%s:%d] player[%lu] score not enough, stone_type:%u, stone_score:%u, product_score:%u", __FUNCTION__, __LINE__, extern_data->player_id, stone_type, stone_score, product_score);
			break;
		}

		uint32_t product_num = stone_score / product_score;
		if (!player->check_can_add_item(product_id, product_num, NULL))
		{
			ret = ERROR_ID_BAG_GRID_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] bag not enough, item_id:%u, item_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, product_id, product_num);
			break;
		}

		uint32_t need_coin = product_coin * product_num;
		uint32_t has_coin = player->get_coin();
		if (has_coin < need_coin)
		{
			ret = ERROR_ID_COIN_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] coin not enough, need:%u, has:%u", __FUNCTION__, __LINE__, extern_data->player_id, need_coin, has_coin);
			break;
		}

		for (std::map<uint32_t, uint32_t>::iterator iter = stone_map.begin(); iter != stone_map.end(); ++iter)
		{
			player->del_item(iter->first, iter->second, MAGIC_TYPE_PARTNER_COMPOSE_STONE);
		}
		
		player->add_item(product_id, product_num, MAGIC_TYPE_PARTNER_COMPOSE_STONE);
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PARTNER_COMPOSE_STONE_ANSWER, comm_answer__pack, resp);

	return 0;
}

static int handle_partner_reset_attr_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PartnerUuid *req = partner_uuid__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint64_t partner_uuid = req->uuids;
	partner_struct *partner = player->get_partner_by_uuid(partner_uuid);
	partner_uuid__free_unpacked(req, NULL);
	if (partner == NULL)
	{
		LOG_ERR("[%s:%d] player[%lu] can't find, partner:%lu", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid);
		return -2;
	}
	PartnerTable *config = partner->config;
	if (config == NULL)
	{
		return -3;
	}
	ResResetAttr send;
	res_reset_attr__init(&send);
	send.ret = 0;
	if (player->del_item(config->WashMarrowItem[0], config->WashMarrowItem[1], MAGIC_TYPE_PARTNER_ATTR) != 0)
	{
		send.ret = 190400006;
	}
	else
	{
		partner->relesh_attr();

		double attrData[MAX_PARTNER_ATTR]; //战斗属性 需要战斗力属性
		for (int i = 0; i < MAX_PARTNER_ATTR; ++i)
		{
			attrData[i] = 0;
		}
		partner->calculate_attribute(attrData, partner->data->attr_flash);
		partner->data->attr_flash.power_refresh = calculate_fighting_capacity(attrData);
		send.power = partner->data->attr_flash.power_refresh;
		PartnerAttr sendAttr;
		partner_attr__init(&sendAttr);
		send.flash_attr = &sendAttr;
		uint32_t skill_num = 0;
		PartnerSkillData  skill_data[MAX_PARTNER_SKILL_NUM];
		PartnerSkillData* skill_point[MAX_PARTNER_SKILL_NUM];
		for (int i = 0; i < MAX_PARTNER_SKILL_NUM; ++i)
		{
			skill_point[skill_num] = &skill_data[skill_num];
			partner_skill_data__init(&skill_data[skill_num]);
			skill_data[skill_num].id = partner->data->attr_flash.skill_list[i].skill_id;
			skill_data[skill_num].lv = partner->data->attr_flash.skill_list[i].lv;
			skill_num++;
		}
		sendAttr.skills = skill_point;
		sendAttr.n_skills = skill_num;
		sendAttr.base_attr_id = partner->data->attr_flash.base_attr_id;
		sendAttr.n_base_attr_id = MAX_PARTNER_BASE_ATTR;
		sendAttr.base_attr_cur = partner->data->attr_flash.base_attr_vaual;
		sendAttr.n_base_attr_cur = MAX_PARTNER_BASE_ATTR;
		sendAttr.base_attr_up = partner->data->attr_flash.base_attr_up;
		sendAttr.n_base_attr_up = MAX_PARTNER_BASE_ATTR;
		sendAttr.base_attr_up = partner->data->attr_flash.base_attr_up;
		sendAttr.n_base_attr_up = MAX_PARTNER_BASE_ATTR;
		sendAttr.detail_attr_id = partner->data->attr_flash.detail_attr_id;
		sendAttr.n_detail_attr_id = partner->data->attr_flash.n_detail_attr;
		sendAttr.detail_attr_cur = partner->data->attr_flash.detail_attr_vaual;
		sendAttr.n_detail_attr_cur = partner->data->attr_flash.n_detail_attr;
		sendAttr.type = partner->data->attr_flash.type;

		player->add_task_progress(TCT_PARTNER_RESET_ATTR, 0, 1);
	}
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PARTNER_RESET_ATTR_ANSWER, res_reset_attr__pack, send);

	return 0;
}
static int handle_partner_save_attr_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PartnerUuid *req = partner_uuid__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint64_t partner_uuid = req->uuids;
	partner_struct *partner = player->get_partner_by_uuid(partner_uuid);
	partner_uuid__free_unpacked(req, NULL);
	if (partner == NULL)
	{
		LOG_ERR("[%s:%d] player[%lu] can't find, partner:%lu", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid);
		return -2;
	}
	if (partner->data->attr_flash.base_attr_id[0] ==0)
	{
		return -3;
	}

	std::map<uint32_t, uint32_t> item_list;
	for (int i = 0; i < MAX_PARTNER_SKILL_NUM; ++i)
	{
		if (partner->data->attr_cur.skill_list[i].lv > 0)
		{
			std::map<uint32_t, uint32_t>::iterator it = sg_partner_item_map.find(partner->data->attr_cur.skill_list[i].skill_id);
			if (it == sg_partner_item_map.end())
			{
				continue;
			}
			item_list.insert(std::make_pair(it->second, partner->data->attr_cur.skill_list[i].lv));
		}	
	}
	player->add_item_list_otherwise_send_mail(item_list, MAGIC_TYPE_PARTNER_ATTR, 0, NULL);
	memcpy(&(partner->data->attr_cur), &(partner->data->attr_flash), sizeof(partner_attr_data));
	memset(&(partner->data->attr_flash), 0, sizeof(partner_attr_data));
	partner->calculate_attribute(true);
	if (partner->data->strong_num > 0)
	{
		item_list.clear();
		item_list.insert(std::make_pair(partner->config->QualificationsItem[0], partner->config->QualificationsItem[1] * partner->data->strong_num));
		partner->data->strong_num = 0;
		player->add_item_list_otherwise_send_mail(item_list, MAGIC_TYPE_PARTNER_ATTR, 0, NULL);
	}
	send_comm_answer(MSG_ID_PARTNER_SAVE_ATTR_ANSWER, 0, extern_data);

	return 0;
}
static int handle_partner_add_attr_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	StrongPartner *req = strong_partner__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint64_t partner_uuid = req->uuids;
	uint32_t attrId = req->attr_id;
	uint32_t count = req->count;
	partner_struct *partner = player->get_partner_by_uuid(partner_uuid);
	strong_partner__free_unpacked(req, NULL);
	if (partner == NULL)
	{
		LOG_ERR("[%s:%d] player[%lu] can't find, partner:%lu", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid);
		return -2;
	}
	int i = 0;
	for (; i < MAX_PARTNER_BASE_ATTR; ++i)
	{
		if (attrId == partner->data->attr_cur.base_attr_id[i])
		{
			break;
		}
	}
	if (i == MAX_PARTNER_BASE_ATTR)
	{
		return -2;
	}
	ItemsConfigTable *table = get_config_by_id(partner->config->QualificationsItem[0], &item_config);
	if (table == NULL)
	{
		return -3;
	}
	ResStrongPartner send;
	res_strong_partner__init(&send);
	send.ret = 0;
	for (uint32_t c = 0; partner->data->attr_cur.base_attr_vaual[i] < partner->data->attr_cur.base_attr_up[i] && c < count; ++c)
	{
		if (player->del_item(partner->config->QualificationsItem[0], partner->config->QualificationsItem[1], MAGIC_TYPE_PARTNER_ATTR) != 0)
		{
			send.ret = 190400006;
			break;
		}
		partner->data->attr_cur.base_attr_vaual[i] += rand() % table->ParameterEffect[1] + 1;
		++partner->data->strong_num;
		player->add_task_progress(TCT_PARTNER_ADD_ATTR, 0, 1);
		if (partner->data->attr_cur.base_attr_vaual[i] >= partner->data->attr_cur.base_attr_up[i])
		{
			partner->data->attr_cur.base_attr_vaual[i] = partner->data->attr_cur.base_attr_up[i];
			break;
		}
	}

	send.uuids = partner_uuid;
	send.attr_id = attrId;
	send.attr_cur = partner->data->attr_cur.base_attr_vaual[i];

	partner->calculate_attribute(true);
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PARTNER_ADD_ATTR_ANSWER, res_strong_partner__pack, send);

	return 0;
}
static int handle_partner_add_god_request(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	StrongPartner *req = strong_partner__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	uint64_t partner_uuid = req->uuids;
	uint32_t attrId = req->attr_id;
	uint32_t count = req->count;
	partner_struct *partner = player->get_partner_by_uuid(partner_uuid);
	strong_partner__free_unpacked(req, NULL);
	if (partner == NULL)
	{
		LOG_ERR("[%s:%d] player[%lu] can't find, partner:%lu", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid);
		return -2;
	}
	uint32_t i = 0;
	for (; i < partner->data->n_god; ++i)
	{
		if (attrId == partner->data->god_id[i])
		{
			break;
		}
	}

	ResStrongPartner send;
	res_strong_partner__init(&send);
	send.ret = 0;

	uint32_t itemId = 0;
	uint32_t itemNum = 0;
	if (count == 0) //通用
	{
		if (attrId <= 3) //三神
		{
			itemId = partner->config->ThreeCurrencyItem;
			itemNum = partner->config->ThreeCurrencyItemNum;
		}
		else //七曜
		{
			itemId = partner->config->SevenCurrencyItem;
			itemNum = partner->config->SevenCurrencyItemNum;
		}
	}
	else //专属
	{
		if (attrId <= 3) //三神
		{
			itemId = partner->config->ThreeExclusiveItem;
			itemNum = partner->config->ThreeExclusiveItemNum;
		}
		else //七曜
		{
			itemId = partner->config->SevenExclusiveItem;
			itemNum = partner->config->SevenExclusiveItemNum;
		}
	}
	if ((uint32_t)player->get_item_can_use_num(itemId) < itemNum)
	{
		send.ret = 190400006;
	}
	else if (i <= partner->data->n_god && partner->data->god_level[i] == partner->get_attr(PLAYER_ATTR_LEVEL))
	{
		send.ret = 1;
	}
	
	if (send.ret == 0)
	{
		player->del_item(itemId, itemNum, MAGIC_TYPE_PARTNER_ATTR);
		if (i == partner->data->n_god)
		{
			uint32_t n = 0;
			for (; n < partner->config->n_GodYao; ++n)
			{
				if (attrId == partner->config->GodYao[n])
				{
					partner->data->god_id[partner->data->n_god] = attrId;
					partner->data->god_level[partner->data->n_god++] = 1;
					break;
				}
			}
			if (n == partner->config->n_GodYao)
			{
				return -3;
			}
		}
		else
		{
			++partner->data->god_level[i];
		}
		partner->calculate_attribute(true);
		player->add_task_progress(TCT_PARTNER_ADD_GOD, 0, 1);
	}

	send.uuids = partner_uuid;
	send.attr_id = attrId;
	send.attr_cur = partner->data->god_level[i];

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PARTNER_ADD_GOD_ANSWER, res_strong_partner__pack, send);

	return 0;
}

static int handle_partner_fabao_stone_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PartnerFabaoStoneRequest *req = partner_fabao_stone_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if(!req)
	{
		LOG_ERR("[%s:%d] can not upack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}
	
	uint64_t id = req->id;
	uint64_t mingti_id = req->mingti_id;
	partner_fabao_stone_request__free_unpacked(req, NULL);

	int ret = 0;
	do{
		LifeMagicTable * life_config = get_config_by_id(id, &lifemagic_config);
		if(!life_config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get config failed, id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, id);
			break;
		}
		

		//先判断精力值是否充足
		if(life_config->NeedJingli > player->get_attr(PLAYER_ATTR_ENERGY))
		{
			ret = 190410002;
			break;
		}
		
		//选择对应的命体
		uint64_t LifeNum = 0;
		uint32_t n_LifeProbability = 0;
		uint64_t *LifeProbability = 0;
		uint32_t n_Magic = 0;
		uint64_t *Magic =0;
		if(mingti_id == life_config->Life1)
		{
			LifeNum = life_config->LifeNum1;
			n_LifeProbability = life_config->n_LifeProbability1;
			LifeProbability = life_config->LifeProbability1;
			n_Magic = life_config->n_Magic1;
			Magic = life_config->Magic1;
		}
		else if(mingti_id == life_config->Life2)
		{
			LifeNum = life_config->LifeNum2;
			n_LifeProbability = life_config->n_LifeProbability2;
			LifeProbability = life_config->LifeProbability2;
			n_Magic = life_config->n_Magic2;
			Magic = life_config->Magic2;
		}
		else if(mingti_id == life_config->Life3)
		{
			LifeNum = life_config->LifeNum3;
			n_LifeProbability = life_config->n_LifeProbability3;
			LifeProbability = life_config->LifeProbability3;
			n_Magic = life_config->n_Magic3;
			Magic = life_config->Magic3;
		}
		else if(mingti_id == life_config->Life4)
		{
			LifeNum = life_config->LifeNum4;
			n_LifeProbability = life_config->n_LifeProbability4;
			LifeProbability = life_config->LifeProbability4;
			n_Magic = life_config->n_Magic4;
			Magic = life_config->Magic4;
		}
		else
		{			
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get config failed, item_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, life_config->NeedItem);
			break;
		}
		std::map<uint64_t, uint64_t> item_map;
		item_map[mingti_id] = LifeNum;
		item_map[life_config->NeedItem] = life_config->NeedItemNum;

		//判断要消耗的道具
		for(std::map<uint64_t, uint64_t>::iterator iter = item_map.begin(); iter != item_map.end(); ++iter)
		{
			uint64_t item_id = iter->first;
			uint64_t item_num = iter->second;
			ItemsConfigTable *config = get_config_by_id(item_id, &item_config);
			if (!config)
			{
				ret = ERROR_ID_NO_CONFIG;
				LOG_ERR("[%s:%d] player[%lu] get config failed, item_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, item_id);
				break;
			}
			uint32_t has_num = player->get_item_can_use_num(item_id);
			if (has_num < item_num)
			{
				ret = ERROR_ID_PROP_NOT_ENOUGH;
				LOG_ERR("[%s:%d] player[%lu] prop not enough, item_id:%u, need_num:%u, has_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, item_id, item_num, has_num);
				break;
			}
		}
		
		if(ret != 0)
		{
			break;
		}

		//根据概率获取对应的法宝
		uint64_t total_gailv = 0;
		for(size_t	i = 0; i < n_LifeProbability; ++i)
		{
			total_gailv += LifeProbability[i];
		}
		if(total_gailv <= 0)
		{
			ret = ERROR_ID_CONFIG;
			break;
		}

		total_gailv = rand() % total_gailv + 1;
		int flag = -1;
		uint64_t gailv_begin = 0;
		uint64_t gailv_end = 0;
		for(size_t j = 0; j < n_LifeProbability; ++j)
		{
			gailv_end = gailv_begin + LifeProbability[j];
			if(total_gailv > gailv_begin && total_gailv <= gailv_end)
			{
				flag = j;
				break;	
			}
			gailv_begin = gailv_end;
		}


		if(flag < 0 || flag >= (int)n_Magic)
		{
			ret = ERROR_ID_CONFIG;
			break;
		}
	
		uint64_t fabao_id = Magic[flag];
		if (!player->check_can_add_item(fabao_id, 1, NULL))
		{
			ret = ERROR_ID_BAG_GRID_NOT_ENOUGH;
			LOG_ERR("[%s:%d] player[%lu] bag not enough, item_id:%u, item_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, fabao_id, 1);
			break;
		}
		for (std::map<uint64_t, uint64_t>::iterator iter = item_map.begin(); iter != item_map.end(); ++iter)
		{
			player->del_item(iter->first, iter->second, MAGIC_TYPE_PARTNER_FABAO_STONE);
		}
		player->set_attr(PLAYER_ATTR_ENERGY, player->get_attr(PLAYER_ATTR_ENERGY) - life_config->NeedJingli);
		
		player->add_item(fabao_id, 1 , MAGIC_TYPE_PARTNER_FABAO_STONE);

	}while(0);


	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_PARTNER_FABAO_STONE_ANSWER, comm_answer__pack, resp);

	return 0;
}

//伙伴法宝佩戴或替换请求
static int handle_partner_fabao_change_request(player_struct *player, EXTERN_DATA *extern_data)
{	
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PartnerFabaoChangeRequest *req = partner_fabao_change_request__unpack(NULL, get_data_len(), (uint8_t*)get_data());
	if(!req)
	{
		LOG_ERR("[%s:%d] partner fabao change request unpack failed, player_id:[%lu]", __FUNCTION__, __LINE__, player->data->player_id);
	}

	//uint32_t bag_pos = req->bage_id;
	//uint32_t item_id = req->fabao_item_id;
	uint64_t partner_uuid = req->partner_uuid;
	partner_fabao_change_request__free_unpacked(req, NULL);
	partner_struct *partner = player->get_partner_by_uuid(partner_uuid);
	if( partner == NULL )
	{
		LOG_ERR("[%s:%d] player[%lu] can't find partner:lu", __FUNCTION__, __LINE__, extern_data->player_id, partner_uuid);
		return -2;
	}

	return 0;
}
//即将开启请求领奖
static int handle_gift_receive_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	GiftReceiveAnswer receive_answer;
	gift_receive_answer__init(&receive_answer);

	uint64_t receive_flag = player->data->Receive_type + 1;
	std::map<uint64_t, struct FunctionUnlockTable *>::iterator iter = sg_jijiangopen.find(receive_flag);
	

	if (iter != sg_jijiangopen.end())
	{
		if (iter->second->Level <= player->data->attrData[PLAYER_ATTR_LEVEL])
		{
			if (player->add_item(iter->second->ItemID, iter->second->ItemNum, MAGIC_TYPE_RECEIVE_JIJIANGOPEN) != 0)
			{
				receive_answer.gift_type = player->data->Receive_type;
				receive_answer.result = ERROR_ID_GIJIANGOPEN_GIFT_FAIL;
			}
			else
			{
				receive_answer.gift_type = receive_flag;
				receive_answer.result = 0;
				player->data->Receive_type = receive_flag;
			}
			
		}
		else
		{
			receive_answer.gift_type = player->data->Receive_type;
			receive_answer.result = 1;
		}
	}
	else
	{
		receive_answer.gift_type = player->data->Receive_type;
		receive_answer.result = 1;
	}
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_JIJIANGOP_GIFT_INFO_ANSWER, gift_receive_answer__pack, receive_answer);

	return(0);
}
static int notify_jijiangopen_gift_info(player_struct* player, EXTERN_DATA* extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}
	GiftCommNotify notify;
	gift_comm_notify__init(&notify);
	notify.gift_type = player->data->Receive_type;

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_JIJIANGOP_GIFT_INFO_NOTIFY, gift_comm_notify__pack, notify);

	return(0);
}

//其他服操作失败返还扣除的资源
static int handle_undo_cost(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);

	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-1);
	}

	PROTO_UNDO_COST *req = (PROTO_UNDO_COST*)buf_head();

	if (req->cost.gold > 0)
	{
		player->add_bind_gold(req->cost.gold, req->cost.statis_id);
	}
	if (req->cost.unbind_gold > 0)
	{
		player->add_unbind_gold(req->cost.unbind_gold, req->cost.statis_id);
	}
	if (req->cost.coin > 0)
	{
		player->add_coin(req->cost.coin, req->cost.statis_id);
	}
	std::map<uint32_t, uint32_t> item_map;
	for (size_t i = 0; i < sizeof(req->cost.item_id) / sizeof(req->cost.item_id[0]); ++i)
	{
		uint32_t item_id = req->cost.item_id[i];
		uint32_t item_num = req->cost.item_num[i];
		item_map[item_id] += item_num;
	}
	if (item_map.size() > 0)
	{
		player->add_item_list_otherwise_send_mail(item_map, req->cost.statis_id, 0, NULL);
	}

	return 0;
}
//客户端完成特定副本ai请求继续执行副本
static int handle_continue_raid_ai_request(player_struct *player, EXTERN_DATA *extern_data)
{
	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}
	RaidAiContinueRequest* req = raid_ai_continue_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());

	if(req == NULL)
	{
		LOG_ERR("[%s:%d] req == NULL", __FUNCTION__, __LINE__);
		return -2;
	}
	uint64_t type = req->ai_type;
	raid_ai_continue_request__free_unpacked(req, NULL);

	raid_struct *raid = player->get_raid();
	if(raid == NULL || raid->data == NULL)
	{
		LOG_ERR("[%s:%d] raid == NULL or raid->data == NULL", __FUNCTION__, __LINE__);
		return -3;
	}

	raid->data->raid_ai_event = type;

	return 0;
}

//客户端请求跳过新手副本
static int handle_skip_new_raid_request(player_struct *player, EXTERN_DATA *extern_data)
{

	if (!player || !player->is_online())
	{
		LOG_ERR("[%s:%d] can not find player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	raid_struct *raid = player->get_raid();
	if (NULL == raid || raid->data->ID != 20035)
	{
		LOG_ERR("[%s:%d] skip new raid fail", __FUNCTION__, __LINE__);
		return -2;
	}

	//将pk模式和阵营设置回去
	player->data->noviceraid_flag = 1;
	player->set_attr(PLAYER_ATTR_PK_TYPE, 0);
	player->broadcast_one_attr_changed(PLAYER_ATTR_PK_TYPE, 0, false, true);
	player->set_attr(PLAYER_ATTR_ZHENYING, 0);		
	player->broadcast_one_attr_changed(PLAYER_ATTR_ZHENYING, 0, false, true);
	player->send_zhenying_info();

	//将技能重置
	player->m_skill.clear(); //重新初始化技能
	player->m_skill.SendAllSkill();
	player->m_skill.Init();
	player->m_skill.SendAllSkill();
	

	raid->clear_monster();
	raid->m_player[0]->clear_one_buff(114400018);
	raid->player_leave_raid(raid->m_player[0]);		


	return 0;
}
void install_msg_handle()
{
	add_msg_handle(MSG_ID_MOVE_REQUEST, handle_move_request);
	add_msg_handle(MSG_ID_SKILL_CAST_REQUEST, handle_skill_cast_request);
	add_msg_handle(MSG_ID_PARTNER_SKILL_CAST_REQUEST, handle_partner_skill_cast_request);	
	add_msg_handle(MSG_ID_SKILL_HIT_REQUEST, handle_skill_hit_request);
//	add_msg_handle(MSG_ID_CALL_ATTACK_REQUEST, handle_skill_call_attack_request);
//	add_msg_handle(MSG_ID_SKILL_MOVE_REQUEST, handle_skill_move_request);
	add_msg_handle(MSG_ID_LEARN_SKILL_REQUEST, handle_learn_skill_request);
	add_msg_handle(MSG_ID_OPEN_SKILL_FOR_GUIDE, handle_open_skill_for_guide_request);
	add_msg_handle(MSG_ID_SET_FUWEN_REQUEST, handle_set_fuwen_request);
	add_msg_handle(MSG_ID_LEARN_LIVE_SKILL_REQUEST, handle_learn_live_skill_request);
	add_msg_handle(MSG_ID_LIVE_SKILL_BREAK_REQUEST, handle_live_skill_break_request);
	add_msg_handle(MSG_ID_PRODUCE_MEDICINE_REQUEST, handle_produce_medicine_request);
	add_msg_handle(MSG_ID_ADD_SPEED_BUFF_REQUEST, handle_add_speed_buff_request);
	add_msg_handle(MSG_ID_DEL_SPEED_BUFF_REQUEST, handle_del_speed_buff_request);		

	//镖车
	add_msg_handle(MSG_ID_ACCEPT_CASH_TRUCK_REQUEST, handle_accept_cash_truck_request);
	add_msg_handle(MSG_ID_GET_ON_CASH_TRUCK_REQUEST, handle_get_on_cash_truck_request);
	add_msg_handle(MSG_ID_GO_DOWND_CASH_TRUCK_REQUEST, handle_go_downd_cash_truck_request);
	//add_msg_handle(MSG_ID_SUBMIT_CASH_TRUCK_REQUEST, handle_submit_cash_truck_request);
	add_msg_handle(MSG_ID_CASH_TRUCK_SPEED_UP_REQUEST, handle_cash_truck_speed_up_request);

	//答题
	add_msg_handle(MSG_ID_GET_COMMON_QUESTION_REQUEST, handle_get_common_question_request);
	add_msg_handle(MSG_ID_ANSWER_COMMON_QUESTION_REQUEST, handle_answer_common_question_request);
	add_msg_handle(MSG_ID_COMMON_QUESTION_HINT_REQUEST, handle_common_question_hint_request);
	add_msg_handle(MSG_ID_COMMON_QUESTION_HELP_REQUEST, handle_common_question_help_request);
	add_msg_handle(MSG_ID_GET_AWARD_QUESTION_REQUEST, handle_get_award_question_request);
	add_msg_handle(MSG_ID_ANSWER_AWARD_QUESTION_REQUEST, handle_answer_award_question_request);
	add_msg_handle(MSG_ID_END_AWARD_QUESTION_REQUEST, handle_interupt_award_question_request);
	add_msg_handle(MSG_ID_FIRST_AWARD_QUESTION_REQUEST, handle_first_award_question_request);

	add_msg_handle(MSG_ID_XUNBAO_POS_REQUEST, handle_xunbao_pos_request);

	//阵营
	add_msg_handle(MSG_ID_CHOSE_ZHENYING_REQUEST, handle_chose_zhenying_request);
	add_msg_handle(MSG_ID_CHANGE_ZHENYING_REQUEST, handle_change_zhenying_request);
	add_msg_handle(MSG_ID_ZHENYING_POWER_REQUEST, handle_zhenying_power_request);
	add_msg_handle(MSG_ID_INTO_ZHENYING_BATTLE_REQUEST, handle_into_zhenying_battle_request);
	add_msg_handle(MSG_ID_ZHENYING_TEAM_INFO_REQUEST, handle_zhenying_team_info_request);
	add_msg_handle(MSG_ID_EXIT_ZHENYING_BATTLE_REQUEST, handle_exit_zhenying_battle_request);
	add_msg_handle(MSG_ID_GET_ZHENYING_TASK_AWARD_REQUEST, handle_get_zhenying_task_award_request);
	add_msg_handle(MSG_ID_ZHENYING_GET_LINE_INFO_REQUEST, handle_zhenying_get_line_info_request);
	add_msg_handle(MSG_ID_ZHENYING_CHANGE_LINE_REQUEST, handle_zhenying_change_line_request);

	//妖师国御
	add_msg_handle(MSG_ID_ACCECT_GUOYU_TASK_REQUEST, handle_accect_guoyu_task_request);
	add_msg_handle(MSG_ID_AGREED_GUOYU_TASK_REQUEST, handle_agreed_guoyu_task_request);
	add_msg_handle(MSG_ID_GIVEUP_GUOYU_TASK_REQUEST, handle_giveup_guoyu_task_request);
	add_msg_handle(MSG_ID_SET_SPECIAL_REQUEST, handle_set_special_request);
	add_msg_handle(MSG_ID_GUOYU_BOSS_APPEAR_REQUEST, handle_guoyu_boss_appear_request);
	//add_msg_handle(MSG_ID_ENTER_GUOYU_FB_REQUEST, handle_enter_guoyu_fb_request);
	//妖师惩戒
	add_msg_handle(MSG_ID_CHENGJIE_FIND_TARGET_REQUEST, handle_chengjie_find_target_request);
	add_msg_handle(MSG_ID_REFRESH_CHENGJIE_LIST_REQUEST, handle_refresh_chengjie_list_request);
	add_msg_handle(MSG_ID_ADD_CHENGJIE_TASK_REQUEST, handle_add_chengjie_task_request);
	add_msg_handle(MSG_ID_ACCEPT_CHENGJIE_TASK_REQUEST, handle_accept_chengjie_task_request);
	add_msg_handle(MSG_ID_SUBMIT_CHENGJIE_TASK_REQUEST, handle_submit_chengjie_task_request);
	//妖师赏金
	add_msg_handle(MSG_ID_ACCEPT_SHANGJIN_TASK_REQUEST, handle_accept_shangjin_task_request);
	add_msg_handle(MSG_ID_REFRESH_SHANGJIN_TASK_REQUEST, handle_refresh_shangjin_task_request);

	//坐骑
	add_msg_handle(MSG_ID_BUY_HORSE_REQUEST, handle_buy_horse_request);
	add_msg_handle(MSG_ID_SET_CUR_HORSE_REQUEST, handle_set_cur_horse_request);
	add_msg_handle(MSG_ID_ON_HORSE_REQUEST, handle_on_horse_request);
	add_msg_handle(MSG_ID_DOWN_HORSE_REQUEST, handle_down_horse_request);
	add_msg_handle(MSG_ID_ADD_HORSE_EXP_REQUEST, handle_add_horse_exp_request);
	add_msg_handle(MSG_ID_ADD_HORSE_STEP_REQUEST, handle_add_horse_step_request);
	add_msg_handle(MSG_ID_ADD_HORSE_SOUL_REQUEST, handle_add_horse_soul_request);
	add_msg_handle(MSG_ID_ADD_HORSE_SOUL_LEVEL_REQUEST, handle_add_horse_soul_level_request);
	add_msg_handle(MSG_ID_SET_HORSE_OLD_REQUEST, handle_set_horse_old_request);
	add_msg_handle(MSG_ID_SET_HORSE_FLY_REQUEST, handle_set_horse_fly_request);
	add_msg_handle(MSG_ID_HORSE_RESTORE_REQUEST, handle_horse_restory_request);

	//时装
	add_msg_handle(MSG_ID_BUY_FASHION_REQUEST, handle_buy_fashion_request);
	add_msg_handle(MSG_ID_UNLOCK_COLOR_REQUEST, handle_unlock_color_request);
	add_msg_handle(MSG_ID_UNLOCK_WEAPON_COLOR_REQUEST, handle_unlock_weapon_color_request);
	add_msg_handle(MSG_ID_SET_FASHION_COLOR_REQUEST, handle_set_fashion_color_request);
	add_msg_handle(MSG_ID_SET_WEAPON_COLOR_REQUEST, handle_set_weapon_color_request);
	add_msg_handle(MSG_ID_PUTON_FASHION_REQUEST, handle_puton_fashion_request);
	add_msg_handle(MSG_ID_TAKEOFF_FASHION_REQUEST, handle_takeoff_fashion_request);
	add_msg_handle(MSG_ID_FASHION_OLD_REQUEST, handle_set_fashion_old_request);
	add_msg_handle(MSG_ID_SET_COLOR_OLD_REQUEST, handle_set_color_old_request);
	add_msg_handle(MSG_ID_SET_WEAPON_COLOR_OLD_REQUEST, handle_set_weapon_color_old_request);

	//组队
	add_msg_handle(MSG_ID_CREATE_TEAM_REQUEST, handle_create_team_request);
	add_msg_handle(MSG_ID_APPLY_TEAM_REQUEST, handle_apply_team_request);
	add_msg_handle(MSG_ID_HANDLE_APPLY_TEAM_REQUEST, handle_handle_apply_team_request);
	add_msg_handle(MSG_ID_HANDLE_REFUCE_APPLY_TEAM_REQUEST, handle_handle_refuce_apply_team_request);
	add_msg_handle(MSG_ID_KICK_TEAM_REQUEST, handle_kick_team_mem_request);
	add_msg_handle(MSG_ID_QUIT_TEAM_REQUEST, handle_quit_team_request);
	add_msg_handle(MSG_ID_TEAM_BE_LEAD_REQUEST, handle_team_be_lead_request);
	add_msg_handle(MSG_ID_TEAM_CHANGE_LEAD_REQUEST, handle_team_change_lead_request);
	add_msg_handle(MSG_ID_TEAM_LIMITED_REQUEST, handle_team_setlimit_request);
	add_msg_handle(MSG_ID_INVITE_TEAM_REQUEST, handle_team_invite_request);
	add_msg_handle(MSG_ID_HANDLEINVITE_TEAM_REQUEST, handle_team_invite_handle_request);
	add_msg_handle(MSG_ID_TEAM_LIST_REQUEST, handle_team_list_request);
	add_msg_handle(MSG_ID_TEAM_INFO_REQUEST, handle_team_info_request);
	add_msg_handle(MSG_ID_TEAM_HANDLE_BE_LEAD_REQUEST, handle_be_team_lead_request);
	add_msg_handle(MSG_ID_TEAM_MATCH_REQUEST, handle_team_match_request);
	add_msg_handle(MSG_ID_TEAM_CANCEL_MATCH_REQUEST, handle_cancel_team_match_request);
	add_msg_handle(MSG_ID_TEAM_LEAD_POS_REQUEST, handle_team_lead_pos_request);
	add_msg_handle(MSG_ID_TEAM_SET_FOLLOW_REQUEST, handle_team_set_follow_request);
	add_msg_handle(MSG_ID_TEAM_SUMMON_MEM_REQUEST, handle_team_summon_mem_request);
	add_msg_handle(MSG_ID_TEAM_SPEEK_REQUEST, handle_team_speek_request);

	add_msg_handle(MSG_ID_TRANSFER_REQUEST, handle_transfer_request);
	add_msg_handle(MSG_ID_ENTER_SCENE_READY_REQUEST, handle_enter_scene_ready_request);
	add_msg_handle(MSG_ID_TRANSFER_TO_PLAYER_SCENE_REQUEST, handle_transfer_to_player_scene_request);

	add_msg_handle(SERVER_PROTO_ENTER_GAME_REQUEST, handle_enter_game);
	add_msg_handle(SERVER_PROTO_KICK_ROLE_NOTIFY, handle_kick_player);

	add_msg_handle(MSG_ID_MOVE_START_REQUEST, handle_move_start_request);
	add_msg_handle(MSG_ID_MOVE_STOP_REQUEST,  handle_move_stop_request);

	add_msg_handle(MSG_ID_MOVE_Y_START_REQUEST, handle_move_y_start_request);
	add_msg_handle(MSG_ID_MOVE_Y_STOP_REQUEST,  handle_move_y_stop_request);

	add_msg_handle(MSG_ID_BAG_INFO_REQUEST, handle_bag_info_request);
	add_msg_handle(MSG_ID_BAG_UNLOCK_GRID_REQUEST, handle_bag_unlock_grid_request);
	add_msg_handle(MSG_ID_BAG_SELL_REQUEST, handle_bag_sell_request);
	add_msg_handle(MSG_ID_BAG_USE_REQUEST, handle_bag_use_request);
	add_msg_handle(MSG_ID_BAG_STACK_REQUEST, handle_bag_stack_request);
	add_msg_handle(MSG_ID_BAG_TIDY_REQUEST, handle_bag_tidy_request);

	add_msg_handle(MSG_ID_RELIVE_REQUEST, handle_relive_request);
	add_msg_handle(MSG_ID_CHAT_REQUEST, handle_chat_request);
	add_msg_handle(MSG_ID_CHAT_BROADCAST_REQUEST, handle_chat_broadcast_request);

	add_msg_handle(MSG_ID_PLAYER_RENAME_REQUEST, handle_rename_request);

	add_msg_handle(MSG_ID_HEAD_ICON_INFO_REQUEST, handle_head_icon_info_request);
	add_msg_handle(MSG_ID_HEAD_ICON_REPLACE_REQUEST, handle_head_icon_replace_request);
	add_msg_handle(MSG_ID_HEAD_ICON_OLD_REQUEST, handle_head_icon_old_request);
	add_msg_handle(MSG_ID_COLLECT_BEGIN_REQUEST, handle_gather_request);
	add_msg_handle(MSG_ID_COLLECT_COMMPLETE_REQUEST, handle_gather_complete);
	add_msg_handle(MSG_ID_COLLECT_INTERRUPT_REQUEST, handle_gather_interupt);
//	add_msg_handle(MSG_ID_BAG_SHOW_REQUEST, handle_bag_show_request);

	add_msg_handle(MSG_ID_TASK_LIST_REQUEST, handle_task_list_request);
	add_msg_handle(MSG_ID_TASK_ACCEPT_REQUEST, handle_task_accept_request);
	add_msg_handle(MSG_ID_TASK_SUBMIT_REQUEST, handle_task_submit_request);
	add_msg_handle(MSG_ID_TASK_ABANDON_REQUEST, handle_task_abandon_request);
	add_msg_handle(MSG_ID_TASK_MONSTER_REQUEST, handle_task_monster_request);
	add_msg_handle(MSG_ID_TASK_COMPLETE_REQUEST, handle_task_complete_request);
	add_msg_handle(MSG_ID_TASK_FAIL_REQUEST, handle_task_fail_request);
	add_msg_handle(MSG_ID_TASK_CHAPTER_REWARD_REQUEST, handle_task_chapter_reward_request);

	//add_msg_handle(MSG_ID_SET_FASHION, handle_set_fashion_request);

	add_msg_handle(MSG_ID_SING_INTERRUPT_REQUEST, handle_sing_interrupt_request);
	add_msg_handle(MSG_ID_SING_END_REQUEST, handle_sing_end_request);
	add_msg_handle(MSG_ID_SING_BEGIN_REQUEST, handle_sing_begin_request);

	add_msg_handle(MSG_ID_ENTER_RAID_REQUEST, handle_enter_raid_request);
	add_msg_handle(MSG_ID_TRANSFER_TO_LEADER_REQUEST, handle_transfer_to_leader_request);
	add_msg_handle(MSG_ID_LEAVE_RAID_REQUEST, handle_leave_raid_request);
	add_msg_handle(MSG_ID_TRANSFER_FAR_TEAM_MEMBER_REQUEST, handle_transfer_far_team_member_request);
	add_msg_handle(MSG_ID_TEAM_RAID_READY_REQUEST, handle_team_raid_ready_request);
	add_msg_handle(MSG_ID_TEAM_RAID_CANCEL_REQUEST, handle_team_raid_cancel_request);

	add_msg_handle(MSG_ID_EQUIP_LIST_REQUEST, handle_equip_list_request);
	add_msg_handle(MSG_ID_EQUIP_STAR_UP_REQUEST, handle_equip_star_up_request);
	add_msg_handle(MSG_ID_EQUIP_STAIR_UP_REQUEST, handle_equip_stair_up_request);
	add_msg_handle(MSG_ID_EQUIP_ENCHANT_REQUEST, handle_equip_enchant_request);
	add_msg_handle(MSG_ID_EQUIP_ENCHANT_RETAIN_REQUEST, handle_equip_enchant_retain_request);
	add_msg_handle(MSG_ID_EQUIP_DRILL_REQUEST, handle_equip_drill_request);
	add_msg_handle(MSG_ID_EQUIP_INLAY_REQUEST, handle_equip_inlay_request);
	add_msg_handle(MSG_ID_EQUIP_STRIP_REQUEST, handle_equip_strip_request);
	add_msg_handle(MSG_ID_EQUIP_GEM_COMPOSE_REQUEST, handle_equip_gem_compose_request);

	add_msg_handle(MSG_ID_LEAVE_PLANES_RAID_REQUEST, handle_planes_raid_request);

	add_msg_handle(SERVER_PROTO_MAIL_GIVE_ATTACH_REQUEST, handle_mail_give_attach_request);

	add_msg_handle(MSG_ID_SHOP_INFO_REQUEST, handle_shop_info_request);
	add_msg_handle(MSG_ID_SHOP_BUY_REQUEST, handle_shop_buy_request);

	add_msg_handle(MSG_ID_YUQIDAO_INFO_REQUEST, handle_yuqidao_info_request);
	add_msg_handle(MSG_ID_YUQIDAO_FILL_REQUEST, handle_yuqidao_fill_request);
	add_msg_handle(MSG_ID_YUQIDAO_BREAK_REQUEST, handle_yuqidao_break_request);
	add_msg_handle(MSG_ID_YUQIDAO_BREAK_RETAIN_REQUEST, handle_yuqidao_break_retain_request);

	add_msg_handle(MSG_ID_SET_PK_TYPE_REQUEST, handle_set_pk_type_request);
	add_msg_handle(MSG_ID_QIECUO_REQUEST, handle_qiecuo_request);
	add_msg_handle(MSG_ID_QIECUO_START_REQUEST, handle_qiecuo_start_request);
	add_msg_handle(MSG_ID_QIECUO_REFUSE_REQUEST, handle_qiecuo_refuse_request);

	add_msg_handle(SERVER_PROTO_GET_OFFLINE_CACHE_ANSWER, handle_player_cache_answer);
	add_msg_handle(SERVER_PROTO_CHOSE_ZHENYING_REQUEST, handle_server_chose_zhenying);
	add_msg_handle(SERVER_PROTO_CHANGE_ZHENYING_REQUEST, handle_server_change_zhenying);

	add_msg_handle(MSG_ID_PVP_MATCH_START_REQUEST, handle_pvp_match_start_request);
	add_msg_handle(MSG_ID_PVP_MATCH_READY_REQUEST, handle_pvp_match_ready_request);
	add_msg_handle(MSG_ID_PVP_MATCH_CANCEL_REQUEST, handle_pvp_match_cancel_request);
	add_msg_handle(MSG_ID_PVP_RANK_REQUEST, handle_pvp_rank_request);
	add_msg_handle(MSG_ID_PVP_OPEN_LEVEL_REWARD_REQUEST, handle_pvp_open_level_reward_request);
	add_msg_handle(MSG_ID_PVP_OPEN_DAILY_BOX_REQUEST, handle_pvp_open_daily_box_request);
	add_msg_handle(MSG_ID_PVP_RAID_PRAISE_REQUEST, handle_pvp_raid_praise_request);

	add_msg_handle(MSG_ID_BAGUAPAI_INFO_REQUEST, handle_baguapai_info_request);
	add_msg_handle(MSG_ID_BAGUAPAI_SWITCH_REQUEST, handle_baguapai_switch_request);
	add_msg_handle(MSG_ID_BAGUAPAI_WEAR_REQUEST, handle_baguapai_wear_request);
	add_msg_handle(MSG_ID_BAGUAPAI_DECOMPOSE_REQUEST, handle_baguapai_decompose_request);
	add_msg_handle(MSG_ID_BAGUAPAI_REFINE_STAR_REQUEST, handle_baguapai_refine_star_request);
	add_msg_handle(MSG_ID_BAGUAPAI_REFINE_MAIN_ATTR_REQUEST, handle_baguapai_refine_main_attr_request);
	add_msg_handle(MSG_ID_BAGUAPAI_RETAIN_MAIN_ATTR_REQUEST, handle_baguapai_retain_main_attr_request);
	add_msg_handle(MSG_ID_BAGUAPAI_REFINE_MINOR_ATTR_REQUEST, handle_baguapai_refine_minor_attr_request);
	add_msg_handle(MSG_ID_BAGUAPAI_RETAIN_MINOR_ATTR_REQUEST, handle_baguapai_retain_minor_attr_request);

	add_msg_handle(MSG_ID_ACTIVE_REWARD_REQUEST, handle_active_reward_request);

	add_msg_handle(MSG_ID_SETTING_TURN_SWITCH_REQUEST, handle_setting_turn_switch_request);
	add_msg_handle(MSG_ID_TRANSFER_OUT_STUCK_REQUEST, handle_transfer_out_stuck_request);

	add_msg_handle(SERVER_PROTO_GUILDSRV_COST_REQUEST, handle_guildsrv_check_and_cost_request);
	add_msg_handle(SERVER_PROTO_GUILDSRV_REWARD_REQUEST, handle_guildsrv_reward_request);
	add_msg_handle(SERVER_PROTO_SYNC_GUILD_SKILL, handle_sync_guild_skill_request);
	add_msg_handle(SERVER_PROTO_SYNC_GUILD_INFO, handle_sync_guild_info_request);
	add_msg_handle(SERVER_PROTO_GUILD_DISBAND, handle_sync_guild_disband_request);
	add_msg_handle(SERVER_PROTO_GUILD_ANSWER_AWARD, handle_guild_answer_award);
	add_msg_handle(SERVER_PROTO_GUILD_PRODUCE_MEDICINE, handle_guild_prodece_medicine);
	add_msg_handle(MSG_ID_GUILD_BATTLE_CALL_REQUEST, handle_guild_battle_call_request);
	add_msg_handle(MSG_ID_GUILD_BATTLE_INFO_REQUEST, handle_guild_battle_info_request);
	add_msg_handle(SERVER_PROTO_GUILD_BATTLE_FINAL_LIST_ANSWER, handle_guild_battle_final_list_answer);
	add_msg_handle(SERVER_PROTO_GUILD_SYNC_DONATION, handle_guild_sync_donation);

	add_msg_handle(MSG_ID_PERSONALITY_INFO_REQUEST, handle_personality_info_request);
	add_msg_handle(MSG_ID_PERSONALITY_SET_GENERAL_REQUEST, handle_personality_set_general_request);
	add_msg_handle(MSG_ID_PERSONALITY_SET_TAGS_REQUEST, handle_personality_set_tags_request);
	add_msg_handle(MSG_ID_PERSONALITY_SET_INTRO_REQUEST, handle_personality_set_intro_request);
	add_msg_handle(MSG_ID_GET_OTHER_INFO_REQUEST, handle_get_other_info_request);

	add_msg_handle(MSG_ID_FRIEND_RECOMMEND_REQUEST, handle_friend_recommend_request);
	add_msg_handle(MSG_ID_FRIEND_SEARCH_REQUEST, handle_friend_search_request);
	add_msg_handle(SERVER_PROTO_FRIEND_EXTEND_CONTACT_ANSWER, handle_friend_extend_contact_answer);
	add_msg_handle(SERVER_PROTO_FRIENDSRV_COST_REQUEST, handle_friendsrv_check_and_cost_request);
	add_msg_handle(SERVER_PROTO_FRIEND_GIFT_COST_REQUEST, handle_friend_gift_cost_request);
	add_msg_handle(SERVER_PROTO_FRIEND_ADD_GIFT, handle_friend_add_gift_request);
	add_msg_handle(SERVER_PROTO_FRIEND_SYNC_FRIEND_NUM, handle_friend_sync_friend_num);

	add_msg_handle(MSG_ID_AUTO_ADD_HP_SET_REQUEST, handle_auto_add_hp_set_request);

	add_msg_handle(MSG_ID_PARTNER_INFO_REQUEST, handle_partner_info_request);
	add_msg_handle(MSG_ID_PARTNER_TURN_SWITCH_REQUEST, handle_partner_turn_switch_request);
	add_msg_handle(MSG_ID_PARTNER_FORMATION_REQUEST, handle_partner_formation_request);
	add_msg_handle(MSG_ID_PARTNER_LEARN_SKILL_REQUEST, handle_partner_learn_skill_request);
	add_msg_handle(MSG_ID_PARTNER_USE_EXP_ITEM_REQUEST, handle_partner_use_exp_item_request);
	add_msg_handle(MSG_ID_PARTNER_DISMISS_REQUEST, handle_partner_dismiss_request);
	add_msg_handle(MSG_ID_PARTNER_EXCHANGE_REQUEST, handle_partner_exchange_request);
	add_msg_handle(MSG_ID_PARTNER_RECRUIT_REQUEST, handle_partner_recruit_request);
	add_msg_handle(MSG_ID_PARTNER_RESET_ATTR_REQUEST, handle_partner_reset_attr_request);
	add_msg_handle(MSG_ID_PARTNER_ADD_ATTR_REQUEST, handle_partner_add_attr_request);
	add_msg_handle(MSG_ID_PARTNER_ADD_GOD_REQUEST, handle_partner_add_god_request);
	add_msg_handle(MSG_ID_PARTNER_SAVE_ATTR_REQUEST, handle_partner_save_attr_request);
	add_msg_handle(MSG_ID_PARTNER_DEAD_FINISH_REQUEST, handle_partner_dead_finish_request);
	add_msg_handle(MSG_ID_PARTNER_BOND_ACTIVE_REQUEST, handle_partner_bond_active_request);
	add_msg_handle(MSG_ID_PARTNER_BOND_REWARD_REQUEST, handle_partner_bond_reward_request);
	add_msg_handle(MSG_ID_PARTNER_COMPOSE_STONE_REQUEST, handle_partner_compose_stone_request);
	add_msg_handle(MSG_ID_PARTNER_FABAO_STONE_REQUEST, handle_partner_fabao_stone_request);
	add_msg_handle(MSG_ID_PARTNER_FABAO_CHANGE_REQUEST, handle_partner_fabao_change_request);

	add_msg_handle(MSG_ID_JIJIANGOP_GIFT_INFO_REQUEST, handle_gift_receive_request);

	add_msg_handle(SERVER_PROTO_UNDO_COST, handle_undo_cost);


	add_msg_handle(MSG_ID_RAID_AI_CONTINUE_REQUEST, handle_continue_raid_ai_request);
	add_msg_handle(MSG_ID_SKIP_NEW_RAID_REQUEST, handle_skip_new_raid_request);
}

void uninstall_msg_handle()
{
	m_game_handle_map.clear();
}
