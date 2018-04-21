#include <stdint.h>
#include "player_manager.h"
#include "monster_manager.h"
#include "uuid.h"
#include "unit.h"
//#include "team.h"
#include "buff_manager.h"
#include "partner_manager.h"
#include "cash_truck_manager.h"
#include "conn_node_gamesrv.h"
#include "msgid.h"
#include "camp_judge.h"
#include "role.pb-c.h"
#include "cast_skill.pb-c.h"
#include "move_direct.pb-c.h"
#include <algorithm>
#include <math.h>

unit_struct *unit_struct::get_unit_by_uuid(uint64_t uuid)
{
	unit_struct *T;
	int entity_type = get_entity_type(uuid);
	if (entity_type == ENTITY_TYPE_PLAYER || entity_type == ENTITY_TYPE_AI_PLAYER)
	{
		T = player_manager::get_player_by_id(uuid);
	}
	else if (entity_type == ENTITY_TYPE_MONSTER)
	{
		T = monster_manager::get_monster_by_id(uuid);
		if (!T)
		{
			T = cash_truck_manager::get_cash_truck_by_id(uuid);
		}
	}
	else if (entity_type == ENTITY_TYPE_PARTNER)
	{
		T = partner_manager::get_partner_by_uuid(uuid);
	}
	else if (entity_type == ENTITY_TYPE_TRUCK)
	{
		T = cash_truck_manager::get_cash_truck_by_id(uuid);
	}
	else
	{
		LOG_ERR("%s: entity type wrong[%lu][%d]", __FUNCTION__, uuid, entity_type);
		return NULL;
	}
	return T;
}

double *unit_struct::get_attr_by_uuid(uint64_t uuid)
{
	unit_struct *T = get_unit_by_uuid(uuid);
	if (!T)
		return NULL;
	return T->get_all_attr();
}

struct position *unit_struct::get_pos_by_uuid(uint64_t uuid)
{
	unit_struct *T = get_unit_by_uuid(uuid);
	if (!T)
		return NULL;
/*
	unit_struct *T;
	int entity_type = get_entity_type(uuid);
	if (entity_type == ENTITY_TYPE_PLAYER)
	{
		T = player_manager::get_player_by_id(uuid);
	}
	else if (entity_type == ENTITY_TYPE_MONSTER)
	{
		T = monster_manager::get_monster_by_id(uuid);
	}
	else
	{
		return NULL;
	}
	if (!T)
		return NULL;
*/
	return T->get_pos();
}

void unit_struct::init_unit_struct()
{
}

struct position *unit_struct::get_pos()
{
	struct unit_path *path = get_unit_path();
	return &path->pos[path->cur_pos];
}

raid_struct *unit_struct::get_raid()
{
	if (!scene || (scene->get_scene_type() != SCENE_TYPE_RAID))
		return NULL;
	return (raid_struct *)scene;
}

bool unit_struct::is_too_high_to_beattack()
{
	return false;
}

void unit_struct::on_beattack(unit_struct *player, uint32_t skill_id, int32_t damage)
{
	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (!m_buffs[i])
			continue;
		m_buffs[i]->on_beattack();
	}
}

bool unit_struct::give_drop_item(uint32_t drop_id, uint32_t statis_id, AddItemDealWay deal_way, bool isNty, uint32_t mail_id, std::vector<char *> *mail_args)
{
	return false;
}

void unit_struct::reset_pos()
{
	struct unit_path *path = get_unit_path();
	path->pos[0].pos_x = path->pos[path->cur_pos].pos_x;
	path->pos[0].pos_z = path->pos[path->cur_pos].pos_z;
	path->cur_pos = 0;
	path->max_pos = 0;

}

void unit_struct::set_pos(float pos_x, float pos_z)
{
	struct unit_path *path = get_unit_path();
	path->cur_pos = 0;
	path->max_pos = 0;
	path->pos[0].pos_x = pos_x;
	path->pos[0].pos_z = pos_z;
	path->direct_x = 0;
	path->direct_z = 0;
}

bool unit_struct::is_unit_in_move()
{
	struct unit_path *path = get_unit_path();
	if (path->direct_x != 0 || path->direct_z != 0)
		return true;
	if (path->cur_pos < path->max_pos)
		return true;
	return false;
}

unit_struct *unit_struct::get_taunt_target()
{
	uint32_t effect_id = buff_struct::get_skill_effect_by_buff_state(BUFF_STATE_TAUNT);
	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (!m_buffs[i])
			continue;
		if (!m_buffs[i]->effect_config)
			continue;		
		if (!m_buffs[i]->is_recoverable_buff())
			continue;
		if (m_buffs[i]->effect_config->Type != effect_id)
			continue;
		return unit_struct::get_unit_by_uuid(m_buffs[i]->data->attacker);
	}
	return NULL;
}

int unit_struct::clear_debuff()
{
	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (!m_buffs[i])
			continue;
		if (!m_buffs[i]->config && m_buffs[i]->config->IsDeBuff)
			continue;
		m_buffs[i]->del_buff();
	}
	return (0);
}

int unit_struct::clear_control_buff()
{
	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (!m_buffs[i])
			continue;
		if (!m_buffs[i]->config && m_buffs[i]->config->IsControl)
			continue;
		m_buffs[i]->del_buff();
	}
	return (0);
}

int unit_struct::stop_move()
{
	if (!scene)
		return (-1);
	struct position *cur_pos = get_pos();
	set_pos_with_broadcast(cur_pos->pos_x, cur_pos->pos_z);
	MoveStopNotify notify;
	move_stop_notify__init(&notify);
	notify.playerid = get_uuid();
	PosData pos_data;
	pos_data__init(&pos_data);
	pos_data.pos_x = cur_pos->pos_x;
	pos_data.pos_z = cur_pos->pos_z;
	notify.cur_pos = &pos_data;
	broadcast_to_sight(MSG_ID_MOVE_STOP_NOTIFY, &notify, (pack_func)move_stop_notify__pack, true);
	return (0);
}

int unit_struct::check_pos_distance(float pos_x, float pos_z)
{
	if (!scene || !scene->map_config)
		return (0);
	struct position *pos = get_pos();
	struct map_block *block = get_map_block(scene->map_config, pos->pos_x, pos->pos_z);
	if (!block || !block->can_walk)
	{
		LOG_INFO("%s: unit %lu in block[%p]", __FUNCTION__, get_uuid(), block);
		return (0);
	}
	
	float x = pos->pos_x - pos_x;
	float z = pos->pos_z - pos_z;
	if (x * x + z * z > 25)
	{
		LOG_ERR("%s: x = %.6f, z = %.6f, pos_x[%.6f][%.6f] pos_z[%.6f][%.6f]",
			__FUNCTION__, x, z, pos->pos_x, pos_x, pos->pos_z, pos_z);
		return (-1);
	}

	return (0);
}

int unit_struct::set_pos_with_broadcast(float pos_x, float pos_z)
{
	if (!scene)
		return (0);
	area_struct *old_area = area;
	set_pos(pos_x, pos_z);
	area_struct *new_area = scene->get_area_by_pos(pos_x, pos_z);
	if (old_area != new_area)
	{
		update_sight(old_area, new_area);
	}
	return (0);
}

bool unit_struct::is_fullhp()
{
	if (get_attr(PLAYER_ATTR_HP) >= get_attr(PLAYER_ATTR_MAXHP))
		return true;
	return false;
}

bool unit_struct::is_alive()
{
	if (get_attr(PLAYER_ATTR_HP) > 0)
		return true;
	return false;
}

bool unit_struct::add_watched_list(uint64_t player_id)
{
	std::list<uint64_t>::iterator it = std::find(watched_player_id.begin(), watched_player_id.end(), player_id);
	if (it != watched_player_id.end())
	{
		return false;
	}
	watched_player_id.push_back(player_id);
	return true;
}

void unit_struct::del_watched_list(uint64_t player_id)
{
	std::list<uint64_t>::iterator it = std::find(watched_player_id.begin(), watched_player_id.end(), player_id);
	if (it != watched_player_id.end())
	{
		watched_player_id.erase(it);
	}
}

bool unit_struct::in_watched_list(uint64_t player_id)
{
	std::list<uint64_t>::iterator it = std::find(watched_player_id.begin(), watched_player_id.end(), player_id);
	if (it != watched_player_id.end())
	{
		return true;
	}
	return false;
}
void unit_struct::clear_watched_list()
{
	watched_player_id.clear();
}

bool unit_struct::is_monster_in_sight(uint64_t uuid)
{
	int find;
	array_bsearch(&uuid, get_all_sight_monster(), *get_cur_sight_monster(), sizeof(uint64_t), &find, comp_uint64);
	return find;
}

int unit_struct::add_monster_to_sight(uint64_t uuid)
{
	int ret = 0;
	if (on_monster_enter_sight(uuid))
		ret = array_insert(&uuid, get_all_sight_monster(), get_cur_sight_monster(), sizeof(uint64_t), 1, comp_uint64);
	return ret;
}

int unit_struct::del_monster_from_sight(uint64_t uuid)
{
	int ret = 0;
	if (on_monster_leave_sight(uuid))
		ret = array_delete(&uuid, get_all_sight_monster(), get_cur_sight_monster(), sizeof(uint64_t), comp_uint64);
	return ret;
}

bool unit_struct::is_truck_in_sight(uint64_t uuid)
{
	int find;
	array_bsearch(&uuid, get_all_sight_truck(), *get_cur_sight_truck(), sizeof(uint64_t), &find, comp_uint64);
	return find;
}

int unit_struct::add_truck_to_sight(uint64_t uuid)
{
	int ret = 0;
	if (on_truck_enter_sight(uuid))
		ret = array_insert(&uuid, get_all_sight_truck(), get_cur_sight_truck(), sizeof(uint64_t), 1, comp_uint64);
	return ret;
}

int unit_struct::del_truck_from_sight(uint64_t uuid)
{
	int ret = 0;
	if (on_truck_leave_sight(uuid))
		ret = array_delete(&uuid, get_all_sight_truck(), get_cur_sight_truck(), sizeof(uint64_t), comp_uint64);
	return ret;
}

bool unit_struct::is_unit_in_sight(uint64_t uuid)
{
	switch (get_entity_type(uuid))
	{
		case ENTITY_TYPE_AI_PLAYER:
		case ENTITY_TYPE_PLAYER:
			return is_player_in_sight(uuid);
		case ENTITY_TYPE_MONSTER:
			return is_monster_in_sight(uuid);
		case ENTITY_TYPE_PARTNER:
			return is_partner_in_sight(uuid);
		case ENTITY_TYPE_TRUCK:
			return is_truck_in_sight(uuid);
		case ENTITY_TYPE_TRAP:
			return false;
	}
	return false;
}

bool unit_struct::is_partner_in_sight(uint64_t uuid)
{
	int find;
	array_bsearch(&uuid, get_all_sight_partner(), *get_cur_sight_partner(), sizeof(uint64_t), &find, comp_uint64);
	return find;
}

int unit_struct::add_partner_to_sight(uint64_t uuid)
{
	int ret = 0;
	if (on_partner_enter_sight(uuid))
		ret = array_insert(&uuid, get_all_sight_partner(), get_cur_sight_partner(), sizeof(uint64_t), 1, comp_uint64);
	return ret;
}

int unit_struct::del_partner_from_sight(uint64_t uuid)
{
	int ret = 0;
	if (on_partner_leave_sight(uuid))
		ret = array_delete(&uuid, get_all_sight_partner(), get_cur_sight_partner(), sizeof(uint64_t), comp_uint64);
	return ret;
}

bool unit_struct::is_player_in_sight(uint64_t player_id)
{
	int find;
	array_bsearch(&player_id, get_all_sight_player(), *get_cur_sight_player(), sizeof(uint64_t), &find, comp_uint64);
	return find;
}

int unit_struct::add_player_to_sight(uint64_t player_id)
{
	if (on_player_enter_sight(player_id))
		return array_insert(&player_id, get_all_sight_player(), get_cur_sight_player(), sizeof(uint64_t), 1, comp_uint64);
	return (0);
}

int unit_struct::del_player_from_sight(uint64_t player_id)
{
	if (on_player_leave_sight(player_id))
		return array_delete(&player_id, get_all_sight_player(), get_cur_sight_player(), sizeof(uint64_t), comp_uint64);
	return (0);
}

void unit_struct::broadcast_buff_state_changed()
{
	BuffStateChangedNotify nty;
	buff_state_changed_notify__init(&nty);
	nty.player_id = get_uuid();
	nty.buff_state = buff_state;
	broadcast_to_sight(MSG_ID_BUFF_STATE_NOTIFY, &nty, (pack_func)buff_state_changed_notify__pack, true);
}

void unit_struct::broadcast_one_attr_changed(uint32_t id, double value, bool send_team, bool include_myself)
{
	PlayerAttrNotify nty;
	player_attr_notify__init(&nty);
	AttrData attr_data[1];
	AttrData *attr_data_point[1];

	nty.player_id = get_uuid();
	nty.n_attrs = 1;
	nty.attrs = attr_data_point;
	attr_data_point[0] = &attr_data[0];
	attr_data__init(&attr_data[0]);
	attr_data[0].id = id;
	attr_data[0].val = value;

	broadcast_to_sight(MSG_ID_PLAYER_ATTR_NOTIFY, &nty, (pack_func)player_attr_notify__pack, include_myself);
}

void unit_struct::broadcast_to_sight(uint16_t msg_id, void *msg_data, pack_func func, bool include_myself)
{
#ifndef __AI_SRV__
	PROTO_HEAD_CONN_BROADCAST *head;
	PROTO_HEAD *real_head;

	/** ================广播数据============ **/
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
	head->msg_id = ENDION_FUNC_2(SERVER_PROTO_BROADCAST);
	real_head = &head->proto_head;

	real_head->msg_id = ENDION_FUNC_2(msg_id);
	real_head->seq = 0;
//	memcpy(real_head->data, msg_data, len);
	size_t len = func(msg_data, (uint8_t *)real_head->data);
	real_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + len);

	uint64_t *ppp = (uint64_t*)((char *)&head->player_id + len);

	int player_num = *get_cur_sight_player();
	head->num_player_id = 0;
//	memcpy(ppp, get_all_sight_player(), sizeof(uint64_t) * player_num);
	uint64_t *t_player_id = get_all_sight_player();
	for (int i = 0; i < player_num; ++i)
	{
		if (get_entity_type(t_player_id[i]) != ENTITY_TYPE_PLAYER)
			continue;
		ppp[head->num_player_id++] = t_player_id[i];
	}

//	head->num_player_id = player_num;
	if (include_myself && get_entity_type(get_uuid()) == ENTITY_TYPE_PLAYER)
	{
		ppp[head->num_player_id] = get_uuid();
		++head->num_player_id;
	}

	if (head->num_player_id == 0)
		return;

	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD_CONN_BROADCAST) + len + sizeof(uint64_t) * head->num_player_id);

	LOG_DEBUG("%s %d: broad [%u] to %d player", __FUNCTION__, __LINE__, ENDION_FUNC_2(head->proto_head.msg_id), head->num_player_id);

	if (conn_node_gamesrv::connecter.send_one_msg((PROTO_HEAD *)head, 1) != (int)(ENDION_FUNC_4(head->len))) {
		LOG_ERR("%s %d: send to all failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
#endif	
}

__attribute__((used)) static void add_unit_sight_player(unit_struct *unit, std::set<uint64_t> *player_ids)
{
	if (!unit->is_avaliable())
		return;
	int num = *unit->get_cur_sight_player();
	uint64_t *sight_ids = unit->get_all_sight_player();
	for (int i = 0; i < num; ++i)
	{
		if (player_ids->find(sight_ids[i]) != player_ids->end())
			continue;
		player_ids->insert(sight_ids[i]);
	}
	if (unit->get_unit_type() != UNIT_TYPE_PLAYER)
		return;

	if (player_ids->find(unit->get_uuid()) != player_ids->end())
		return;
	player_ids->insert(unit->get_uuid());
}

void unit_struct::broadcast_to_many_sight(uint16_t msg_id, void *msg_data, pack_func func, const std::vector<unit_struct *>& other)
{
#ifndef __AI_SRV__
	PROTO_HEAD_CONN_BROADCAST *head;
	PROTO_HEAD *real_head;
	std::set<uint64_t> player_ids;

	/** ================广播数据============ **/
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
	head->msg_id = ENDION_FUNC_2(SERVER_PROTO_BROADCAST);
	real_head = &head->proto_head;

	real_head->msg_id = ENDION_FUNC_2(msg_id);
	real_head->seq = 0;
//	memcpy(real_head->data, msg_data, len);
	size_t len = func(msg_data, (uint8_t *)real_head->data);
	real_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + len);

	for (std::vector<unit_struct *>::const_iterator iter = other.begin(); iter != other.end(); ++iter)
	{
		add_unit_sight_player(*iter, &player_ids);
	}

	head->num_player_id = 0;
	uint64_t *ppp = (uint64_t*)((char *)&head->player_id + len);
	for (std::set<uint64_t>::iterator iter = player_ids.begin(); iter != player_ids.end(); ++iter)
	{
		if (get_entity_type(*iter) != ENTITY_TYPE_PLAYER)
			continue;
		ppp[head->num_player_id] = *iter;
		++head->num_player_id;
	}

	if (head->num_player_id == 0)
		return;

	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD_CONN_BROADCAST) + len + sizeof(uint64_t) * head->num_player_id);

	LOG_DEBUG("%s %d: broad [%u] to %d player", __FUNCTION__, __LINE__, ENDION_FUNC_2(head->proto_head.msg_id), head->num_player_id);

	if (conn_node_gamesrv::connecter.send_one_msg((PROTO_HEAD *)head, 1) != (int)(ENDION_FUNC_4(head->len))) {
		LOG_ERR("%s %d: send to all failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
#endif	
}
/*
void unit_struct::broadcast_to_sight_with_other_unit(uint16_t msg_id, void *msg_data, pack_func func, bool include_myself, unit_struct *other, bool include_other)
{
	PROTO_HEAD_CONN_BROADCAST *head;
	PROTO_HEAD *real_head;

//	 ================广播数据============
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
	head->msg_id = ENDION_FUNC_2(SERVER_PROTO_BROADCAST);
	real_head = &head->proto_head;

	real_head->msg_id = ENDION_FUNC_2(msg_id);
	real_head->seq = 0;
//	memcpy(real_head->data, msg_data, len);
	size_t len = func(msg_data, (uint8_t *)real_head->data);
	real_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + len);

	uint64_t *ppp = (uint64_t*)((char *)&head->player_id + len);

	int player_num = *get_cur_sight_player();
	memcpy(ppp, get_all_sight_player(), sizeof(uint64_t) * player_num);

	int other_player_num = *other->get_cur_sight_player();
	uint64_t *other_player = other->get_all_sight_player();
	for (int i = 0; i < other_player_num; ++i)
	{
		if (is_player_in_sight(other_player[i]))
			continue;
		ppp[player_num] = other_player[i];
		++player_num;
	}

	if (include_myself)
	{
		if (!other->is_player_in_sight(get_uuid()))
		{
			ppp[player_num] = get_uuid();
			++player_num;
		}
	}
	if (include_other)
	{
		if (!is_player_in_sight(other->get_uuid()))
		{
			ppp[player_num] = other->get_uuid();
			++player_num;
		}
	}
	head->num_player_id = player_num;

	if (head->num_player_id == 0)
		return;

	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD_CONN_BROADCAST) + len + sizeof(uint64_t) * head->num_player_id);

	LOG_DEBUG("%s %d: broad [%u] to %d player", __FUNCTION__, __LINE__, ENDION_FUNC_2(head->proto_head.msg_id), head->num_player_id);

	if (conn_node_gamesrv::connecter.send_one_msg((PROTO_HEAD *)head, 1) != (int)(ENDION_FUNC_4(head->len))) {
		LOG_ERR("%s %d: send to all failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
}
*/
static void update_unit_position_impl(unit_struct *unit, struct unit_path *path, uint64_t escape_time, float speed);
static void update_unit_position_by_direct_impl(struct unit_path *path, float direct_x, float direct_z, uint64_t escape_time, float speed);

// 服务器Tick驱动单位行走, 返回0表示没有移动，返回1表示有移动
int unit_struct::update_unit_position()
{
	struct unit_path *path = get_unit_path();
	float speed = get_speed();
	if (speed <= 0)
		return 0;

	if (!is_unit_in_move())
		return 0;

	uint64_t now_time = time_helper::get_cached_time();
	uint64_t escape_time = now_time - path->start_time;
	if (escape_time == 0)
		return 0;
	if (path->direct_x != 0 || path->direct_z != 0)
		update_unit_position_by_direct_impl(path, path->direct_x, path->direct_z, escape_time, speed);
	else
		update_unit_position_impl(this, path, escape_time, speed);
	path->start_time = now_time;
	pos_changed = true;
	return 1;
}


// 按方向驱动单位走一定时间
static void update_unit_position_by_direct_impl(struct unit_path *path, float direct_x, float direct_z, uint64_t escape_time, float speed)
{
	struct position *cur_pos = &path->pos[path->cur_pos];
	float new_x = cur_pos->pos_x + direct_x * speed * escape_time / 1000.0;
	float new_z = cur_pos->pos_z + direct_z * speed * escape_time / 1000.0;

//	LOG_DEBUG("%s: escape[%lu] new_x[%f] new_z[%f]", __FUNCTION__, escape_time, new_x, new_z);
		// TODO: 判断阻挡

	cur_pos->pos_x = new_x;
	cur_pos->pos_z = new_z;
}

// 按路径点驱动单位走一定时间
static void update_unit_position_impl(unit_struct *unit, struct unit_path *path, uint64_t escape_time, float speed)
{
	double distance = speed * escape_time / 1000.0;

	for(int i = path->cur_pos; i < path->max_pos; ++i)
	{
		assert(i == path->cur_pos);
		if (distance == 0)
			break;
		//获取当前路径距离
		double cur_distance = getdistance(&path->pos[i], &path->pos[i + 1]);

		//移动过当前节点
		if(distance >= cur_distance)
		{
			distance -= cur_distance;
			++path->cur_pos;

			if (path->cur_pos < path->max_pos && unit->get_unit_type() == UNIT_TYPE_PLAYER)
			{
				player_struct *player = (player_struct *)unit;
				int i = path->cur_pos;
				player->data->m_angle = pos_to_angle(path->pos[i + 1].pos_x - path->pos[i].pos_x, path->pos[i + 1].pos_z - path->pos[i].pos_z);
//				float t = player->data->m_angle / M_PI * 180 * -1 + 90;
//				LOG_DEBUG("%s: m_angle = %.3f, %.3f, %.3f %.3f", __FUNCTION__, player->data->m_angle, player->data->m_angle / M_PI, player->data->m_angle / M_PI * 180, t);
			}

			continue;
		}
		// 移入当前节点
		else {

			// 计算坐标偏移, 并补偿浮点转整形时的数据丢失
			double rate = distance / cur_distance;
			float delta_x = rate * (path->pos[i + 1].pos_x - path->pos[i].pos_x);
			float delta_z = rate * (path->pos[i + 1].pos_z - path->pos[i].pos_z);

			path->pos[i].pos_x += delta_x;
			path->pos[i].pos_z += delta_z;
			break;
		}
	}

	return;
}

bool unit_struct::is_in_lock_time()
{
	if (lock_time > time_helper::get_cached_time())
		return true;
	return false;
}

void unit_struct::set_lock_time(uint64_t t)
{
	if (t > lock_time)
		lock_time = t;
}

void unit_struct::reset_pools()
{
	reset_buff_pool();
	reset_pos_pool();
}

#define MAX_SIGHT_BUFF_DATA_LEN 1000
static BuffInfo *buff_pool_buff_point[MAX_SIGHT_BUFF_DATA_LEN];
static BuffInfo buff_pool_buff[MAX_SIGHT_BUFF_DATA_LEN];
static int buff_pool_len;
void unit_struct::reset_buff_pool()
{
	buff_pool_len = 0;
}

void unit_struct::init_buff_pool()
{
	memset(&buff_pool_buff[0], 0, sizeof(buff_pool_buff));
	for (int i = 0; i < MAX_SIGHT_BUFF_DATA_LEN; ++i)
		buff_pool_buff_point[i] = &buff_pool_buff[i];
	reset_buff_pool();
}

BuffInfo **unit_struct::pack_unit_buff(size_t *n_data)
{
	BuffInfo **ret = &buff_pool_buff_point[buff_pool_len];
	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (m_buffs[i] == NULL)
			continue;
		buff_info__init(buff_pool_buff_point[buff_pool_len]);
		buff_pool_buff[buff_pool_len].id = m_buffs[i]->data->buff_id;
		buff_pool_buff[buff_pool_len].start_time = m_buffs[i]->data->start_time / 1000;
		buff_pool_buff[buff_pool_len].lv = m_buffs[i]->data->lv;
//		buff_pool_buff[buff_pool_len].end_time = m_buffs[i]->data->end_time / 1000;
		++buff_pool_len;
		++(*n_data);
	}
	return ret;
}


#define MAX_SIGHT_POS_DATA_LEN 300
static PosData *pos_pool_pos_point[MAX_PATH_POSITION * MAX_SIGHT_POS_DATA_LEN];
static PosData pos_pool_pos[MAX_PATH_POSITION * MAX_SIGHT_POS_DATA_LEN];
static int pos_pool_len;
void unit_struct::reset_pos_pool()
{
	pos_pool_len = 0;
}

void unit_struct::init_pos_pool()
{
	memset(&pos_pool_pos[0], 0, sizeof(pos_pool_pos));
	for (int i = 0; i < MAX_PATH_POSITION * MAX_SIGHT_POS_DATA_LEN; ++i)
		pos_pool_pos_point[i] = &pos_pool_pos[i];
	reset_pos_pool();
}

PosData **unit_struct::pack_unit_move_path(size_t *n_data)
{
	struct unit_path *path = get_unit_path();
	PosData **ret = &pos_pool_pos_point[pos_pool_len];
	if (!is_unit_in_move())
	{
		*n_data = 1;
//		pos_pool_pos_point[pos_pool_len] = &pos_pool_pos[pos_pool_len];
		pos_data__init(pos_pool_pos_point[pos_pool_len]);

//path->pos[path->cur_pos]

		pos_pool_pos[pos_pool_len].pos_x = path->pos[path->cur_pos].pos_x;
		pos_pool_pos[pos_pool_len].pos_z = path->pos[path->cur_pos].pos_z;
		++pos_pool_len;
	}
	else
	{
		*n_data = path->max_pos - path->cur_pos + 1;
//		if (*n_data == 0)
//			*n_data = 1;
		for (uint32_t i = 0; i < *n_data; ++i)
		{
//			pos_pool_pos_point[i + pos_pool_len] = &pos_pool_pos[i + pos_pool_len];
			pos_data__init(pos_pool_pos_point[i + pos_pool_len]);
			pos_pool_pos[i + pos_pool_len].pos_x = path->pos[path->cur_pos + i].pos_x;//data->move_path.pos[data->move_path.cur_pos + i].pos_x;
			pos_pool_pos[i + pos_pool_len].pos_z = path->pos[path->cur_pos + i].pos_z;//data->move_path.pos[data->move_path.cur_pos + i].pos_z;
		}
//		info->direct_x = data->move_path.speed_x;
//		info->direct_y = 0;
//		info->direct_z = data->move_path.speed_z;
		pos_pool_len += *n_data;
	}
	return ret;
}

void unit_struct::delete_state_buff(int state)
{
	uint32_t effect_id = buff_struct::get_skill_effect_by_buff_state(state);
	if (effect_id == 0)
		return;
	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (!m_buffs[i])
			continue;
		if (!m_buffs[i]->effect_config)
			continue;		
		if (!m_buffs[i]->is_recoverable_buff())
			continue;
		if (m_buffs[i]->effect_config->Type == effect_id)
		{
//			buff_manager::delete_buff(m_buffs[i]);
			m_buffs[i]->del_buff();
			return;
		}
	}
}

bool unit_struct::check_free_buff_pos(int index)
{
	return (m_buffs[index] == NULL);
}

int unit_struct::get_free_buff_pos()
{
	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (m_buffs[i] == NULL)
		{
			return i;
		}
	}
	return -1;
}

void unit_struct::set_one_buff(buff_struct *buff, int pos)
{
	assert(pos >= 0 && pos < MAX_BUFF_PER_UNIT);
	assert(m_buffs[pos] == NULL);
	m_buffs[pos] = buff;
}

void unit_struct::delete_one_buff(buff_struct *buff)
{
	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (m_buffs[i] == buff)
		{
			m_buffs[i] = NULL;
			return;
		}
	}
}

void unit_struct::delete_one_buff(uint32_t id, bool broadcast_msg)
{
	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (!m_buffs[i] || !m_buffs[i]->data)
			continue;
		if (m_buffs[i]->config->ID != id)
			continue;
		
		AddBuffNotify notify;
		add_buff_notify__init(&notify);
		notify.buff_id = m_buffs[i]->data->buff_id;
		notify.playerid = get_uuid();
		broadcast_to_sight(MSG_ID_DEL_BUFF_NOTIFY, &notify, (pack_func)add_buff_notify__pack, true);

		m_buffs[i]->del_buff();		
		return;
	}
}

void unit_struct::clear_god_buff()
{
	if (!(buff_state & BUFF_STATE_GOD))
		return;
	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (!m_buffs[i])
			continue;
		if (!m_buffs[i]->effect_config)
			continue;
		if (m_buffs[i]->effect_config->Type != 170000006)
			continue;

		AddBuffNotify notify;
		add_buff_notify__init(&notify);
		notify.buff_id = m_buffs[i]->data->buff_id;
		notify.playerid = get_uuid();
		broadcast_to_sight(MSG_ID_DEL_BUFF_NOTIFY, &notify, (pack_func)add_buff_notify__pack, true);
		m_buffs[i]->del_buff();
	}
}

void unit_struct::clear_all_buffs()
{
	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (!m_buffs[i])
			continue;
//		m_buffs[i]->data->owner = 0;
//		m_buffs[i]->m_owner = NULL;
//		m_buffs[i]->data->attacker = 0;
//		m_buffs[i]->m_attacker = NULL;
//		buff_manager::delete_buff(m_buffs[i]);
		m_buffs[i]->del_buff();
//		m_buffs[i] = NULL;
	}
}

bool unit_struct::is_in_buff3()
{
	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (!m_buffs[i])
			continue;
		if (m_buffs[i]->config->BuffType == 3)
			return true;
	}
	return false;
}

bool unit_struct::has_buff(uint32_t buff_id)
{
	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (!m_buffs[i])
			continue;
		if (m_buffs[i]->data->buff_id == buff_id)
			return true;
	}
	return false;
}

void unit_struct::clear_type3_buff()
{
	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (!m_buffs[i])
			continue;
		if (m_buffs[i]->config->BuffType != 3)
			continue;

		AddBuffNotify notify;
		add_buff_notify__init(&notify);
		notify.buff_id = m_buffs[i]->data->buff_id;
		notify.playerid = get_uuid();
		broadcast_to_sight(MSG_ID_DEL_BUFF_NOTIFY, &notify, (pack_func)add_buff_notify__pack, true);
		m_buffs[i]->del_buff();
	}
}


buff_struct *unit_struct::try_cover_duplicate_buff(struct BuffTable *buff_config, uint64_t end_time, unit_struct *attack, uint32_t *old_id)
{
//	if (buff_config->BuffType == 1)
	return try_cover_duplicate_skill_buff(buff_config, end_time, attack, old_id);
	// else if (buff_config->BuffType == 2)
	// 	return try_cover_duplicate_item_buff(buff_config, old_id);
	// else if (buff_config->BuffType == 3)
	// 	return try_cover_duplicate_type3_buff(buff_config, old_id);
	// return NULL;
}

// buff_struct *unit_struct::try_cover_duplicate_type3_buff(struct BuffTable *buff_config, uint32_t *old_id)
// {
// 	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
// 	{
// 		if (!m_buffs[i])
// 			continue;
// 		if (m_buffs[i]->config->BuffType == 3)
// 		{
// 			*old_id = m_buffs[i]->data->buff_id;
// 			m_buffs[i]->reinit_type3_buff(buff_config);
// 			return m_buffs[i];
// 		}
// 	}
// 	return NULL;
// }

buff_struct *unit_struct::try_cover_duplicate_skill_buff(struct BuffTable *buff_config, uint64_t end_time, unit_struct *attack, uint32_t *old_id)
{
	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (!m_buffs[i])
			continue;
		// if (m_buffs[i]->config->BuffType != 1)
		// 	continue;
		// if (m_buffs[i]->config->CoverType == buff_config->CoverType)
		// {
		// 	*old_id = m_buffs[i]->data->buff_id;			
		// 	m_buffs[i]->reinit_buff(buff_config, end_time, attack);
		// 	return m_buffs[i];
		// }
		if (m_buffs[i]->config->CoverType != buff_config->CoverType)
			continue;
		switch (buff_config->CoverType1)
		{
			case 0://覆盖
				m_buffs[i]->reinit_buff(buff_config, end_time, attack);
				break;
			case 1: //延长时间
				m_buffs[i]->data->end_time += buff_config->Time;				
				break;
			case 2: //叠加层数
				m_buffs[i]->add_lv();
				break;
		}
		return m_buffs[i];
	}
	return NULL;
}
// buff_struct *unit_struct::try_cover_duplicate_item_buff(struct BuffTable *buff_config, uint32_t *old_id)
// {
// 	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
// 	{
// 		if (!m_buffs[i])
// 			continue;
// 		if (m_buffs[i]->config->BuffType != 2)
// 			continue;
// 		if (m_buffs[i]->config->ID == buff_config->ID)
// 		{
// 			*old_id = m_buffs[i]->data->buff_id;			
// 			m_buffs[i]->data->end_time += buff_config->Time;
// 			return m_buffs[i];
// 		}
// 	}
// 	return NULL;
// }
void unit_struct::reset_unit_buff_state()
{
	uint32_t old_buff_state = buff_state;
	buff_state = 0;

	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (!m_buffs[i])
			continue;
		if (!m_buffs[i]->is_recoverable_buff())
			continue;
		if (m_buffs[i]->is_attr_buff())
			continue;
		buff_state |= m_buffs[i]->data->effect.buff_state.state;
/*
		switch (m_buffs[i]->data->effect.buff_state.state)
		{
			case BUFF_STATE_LIFE_STEAL:
				life_steal += m_buffs[i]->data->effect.buff_state.value;
				break;
			case BUFF_STATE_DAMAGE_RETURN:
				damage_return += m_buffs[i]->data->effect.buff_state.value;
				break;
		}
*/
	}

	if (buff_state != old_buff_state)
	{
			//如果玩家在红buff区域，而且消除了免疫红buff，那么触发区域变化
		if (get_unit_type() == UNIT_TYPE_PLAYER)
		{
			player_struct *player = (player_struct *)this;
			uint16_t region_id = player->get_attr(PLAYER_ATTR_REGION_ID);
			if (region_id == 14) //阳区域
			{
				if (!(buff_state & BUFF_STATE_AVOID_RED_BUFF) && (old_buff_state & BUFF_STATE_AVOID_RED_BUFF))
				{
					player->on_region_changed(region_id, region_id);
				}
			}
			else if (region_id == 13) //阴区域
			{
				if (!(buff_state & BUFF_STATE_AVOID_BLUE_BUFF) && (old_buff_state & BUFF_STATE_AVOID_BLUE_BUFF))
				{
					player->on_region_changed(region_id, region_id);
				}
			}
		}

		broadcast_buff_state_changed();
	}
}

void unit_struct::calculate_buff_fight_attr(bool isNty)
{
	double *attr = get_all_attr();
	double *fight_attr = get_all_buff_fight_attr();
	assert(attr && fight_attr);
	memcpy(fight_attr, attr, sizeof(double) * MAX_BUFF_FIGHT_ATTR);
	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (!m_buffs[i])
			continue;
		if (!m_buffs[i]->effect_config)
			continue;
		if (!m_buffs[i]->is_recoverable_buff())
			continue;
		if (!m_buffs[i]->is_attr_buff())
			continue;

			//沿用之前的变化值，因为有些效果是攻击者的属性算出来的，这个时候攻击者已经无法找到了
//		double base_attr = attr[m_buffs[i]->effect_config->Effect[0]];
//		m_buffs[i]->data->effect.attr_effect.added_attr_value = base_attr * (m_buffs[i]->effect_config->EffectAdd[0] / 10000.0 - 1) + m_buffs[i]->effect_config->EffectNum[0];

					//速度变化要特殊处理并通知
		if (m_buffs[i]->data->effect.attr_effect.attr_id == PLAYER_ATTR_MOVE_SPEED)
		{
			attr[m_buffs[i]->data->effect.attr_effect.attr_id] += m_buffs[i]->data->effect.attr_effect.added_attr_value;
			if (isNty)
				broadcast_one_attr_changed(PLAYER_ATTR_MOVE_SPEED, attr[PLAYER_ATTR_MOVE_SPEED], true, true);
			LOG_DEBUG("%s: player[%lu] add buff[%lu] attr[%lu] delta[%.1f] to[%.1f]",
				__FUNCTION__, get_uuid(), m_buffs[i]->config->ID, m_buffs[i]->effect_config->Effect[0],
				m_buffs[i]->data->effect.attr_effect.added_attr_value, attr[m_buffs[i]->effect_config->Effect[0]]);
		}
		else
		{
			fight_attr[m_buffs[i]->effect_config->Effect[0]] += m_buffs[i]->data->effect.attr_effect.added_attr_value;
			LOG_DEBUG("%s: player[%lu] add buff[%lu] attr[%lu] delta[%.1f] to[%.1f]",
				__FUNCTION__, get_uuid(), m_buffs[i]->config->ID, m_buffs[i]->effect_config->Effect[0],
				m_buffs[i]->data->effect.attr_effect.added_attr_value, fight_attr[m_buffs[i]->effect_config->Effect[0]]);
		}

	}
}

uint32_t unit_struct::count_life_steal_effect(int32_t damage)
{
	if (damage <= 0)
		return (0);
	double *all_attr = get_all_attr();
//	if (all_attr[PLAYER_ATTR_VAMPIRE] <= __DBL_EPSILON__)
//		return (0);

//	uint32_t ret = damage * all_attr[PLAYER_ATTR_VAMPIRE] / 100;
	uint32_t ret = 0;

	all_attr[PLAYER_ATTR_HP] += ret;
	if (all_attr[PLAYER_ATTR_HP] > all_attr[PLAYER_ATTR_MAXHP])
	{
//		ret -= (all_attr[PLAYER_ATTR_HP] - all_attr[PLAYER_ATTR_MAXHP]);
		all_attr[PLAYER_ATTR_HP] = all_attr[PLAYER_ATTR_MAXHP];
	}

	// if (get_unit_type() == UNIT_TYPE_PLAYER)
	// {
	// 	player_struct *player = (player_struct *)this;
	// 	if (player->m_team)
	// 		player->m_team->OnMemberHpChange(*player);
	// }

	return ret;
}

uint32_t unit_struct::count_damage_return(int32_t damage, unit_struct *unit)
{
	if (damage <= 0)
		return (0);
//	double bounce = unit->get_attr(PLAYER_ATTR_BOUNCE);
	double bounce = 0;
	if (bounce <= __DBL_EPSILON__)
		return (0);

	double *all_attr = get_all_attr();
	uint32_t ret = damage * bounce / 100;
	all_attr[PLAYER_ATTR_HP] -= ret;

	// if (get_unit_type() == UNIT_TYPE_PLAYER)
	// {
	// 	player_struct *player = (player_struct *)this;
	// 	if (player->m_team)
	// 		player->m_team->OnMemberHpChange(*player);
	// }

	return ret;
}

bool unit_struct::check_fight_type(unit_struct *player, bool bfriend)
{
	int fight_type = get_unit_fight_type(this, player);
	if (bfriend)
	{
		if (fight_type != UNIT_FIGHT_TYPE_FRIEND && fight_type != UNIT_FIGHT_TYPE_MYSELF)
		{
			return false;
		}
	}
	else
	{
		if (fight_type != UNIT_FIGHT_TYPE_ENEMY)
		{
			return false;
		}
	}
	return true;
}

int unit_struct::count_rect_unit_at_pos(double angle, struct position *start_pos,
	std::vector<unit_struct *> *ret, uint max, double length, double width, bool bfriend)
{
	if (!start_pos)
		return (0);
	
	double cos = qFastCos(angle);
	double sin = qFastSin(angle);
	double x1, x2;
	double z1, z2;
	x1 = 0;
	x2 = x1 + length;
	z2 = width / 2;
	z1 = -z2;

	int cur_sight_player = *get_cur_sight_player();
	uint64_t *sight_player = get_all_sight_player();
	for (int i = 0; i < cur_sight_player; ++i)
	{
		player_struct *player = player_manager::get_player_by_id(sight_player[i]);
		if (!player || !player->is_alive())
		{
			LOG_ERR("%s %d: unit[%lu] can't find player[%p][%lu] in sight", __FUNCTION__, __LINE__, get_uuid(), player, sight_player[i]);
			continue;
		}

		if (!check_fight_type(player, bfriend))
			continue;

		if (player->is_too_high_to_beattack())
			continue;

		struct position *pos = player->get_pos();

		double pos_x = pos->pos_x - start_pos->pos_x;
		double pos_z = pos->pos_z - start_pos->pos_z;
		double target_x1 = cos*(pos_x)-sin*(pos_z);
		double target_z1 = cos*(pos_z)+sin*(pos_x);

//		double target_x1 = cos*(pos->pos_x)-sin*(pos->pos_z);
//		double target_z1 = cos*(pos->pos_z)+sin*(pos->pos_x);

		if (target_x1 >= x1 && target_x1 <= x2 && target_z1 >= z1 && target_z1 <= z2)
		{
//			LOG_DEBUG("%s:  hit: angle[%.2f] x1[%.2f] x2[%.2f] x[%.2f] z1[%.2f] z2[%.2f] z[%.2f]", "jacktang", angle, x1, x2, target_x1, z1, z2, target_z1);
			ret->push_back(player);
			if (ret->size() >= max)
				return (0);
		}
//		else
//		{
//			LOG_DEBUG("%s: miss: angle[%.2f] x1[%.2f] x2[%.2f] x[%.2f] z1[%.2f] z2[%.2f] z[%.2f]", "jacktang", angle, x1, x2, target_x1, z1, z2, target_z1);
//		}
	}

	int cur_sight_monster = *get_cur_sight_monster();
	uint64_t *sight_monster = get_all_sight_monster();
	for (int i = 0; i < cur_sight_monster; ++i)
	{
		monster_struct *monster = monster_manager::get_monster_by_id(sight_monster[i]);
		if (!monster || monster->mark_delete || !monster->is_alive())
		{
			LOG_ERR("%s %d: player[%lu] in sight", __FUNCTION__, __LINE__, sight_monster[i]);
			continue;
		}

		if (!check_fight_type(monster, bfriend))
			continue;

		struct position *pos = monster->get_pos();

		double pos_x = pos->pos_x - start_pos->pos_x;
		double pos_z = pos->pos_z - start_pos->pos_z;
		double target_x1 = cos*(pos_x)-sin*(pos_z);
		double target_z1 = cos*(pos_z)+sin*(pos_x);

//		double target_x1 = cos*(pos->pos_x)-sin*(pos->pos_z);
//		double target_z1 = cos*(pos->pos_z)+sin*(pos->pos_x);

		if (target_x1 >= x1 && target_x1 <= x2 && target_z1 >= z1 && target_z1 <= z2)
		{
//			LOG_DEBUG("%s:  hit: angle[%.2f] x1[%.2f] x2[%.2f] x[%.2f] z1[%.2f] z2[%.2f] z[%.2f]", "jacktang", angle, x1, x2, target_x1, z1, z2, target_z1);
			ret->push_back(monster);
			if (ret->size() >= max)
				return (0);
		}
//		else
//		{
//			LOG_DEBUG("%s: miss: angle[%.2f] x1[%.2f] x2[%.2f] x[%.2f] z1[%.2f] z2[%.2f] z[%.2f]", "jacktang", angle, x1, x2, target_x1, z1, z2, target_z1);
//		}
	}
	
	return (0);
}

int unit_struct::count_rect_unit(double angle, std::vector<unit_struct *> *ret, uint max, double length, double width, bool bfriend)
{
	struct position *my_pos = get_pos();
	return count_rect_unit_at_pos(angle, my_pos, ret, max, length, width, bfriend);
}
int unit_struct::count_circle_unit(std::vector<unit_struct *> *ret, uint max, struct position *pos, double radius, bool bfriend)
{
	if (!pos)
		return (0);

	radius = radius * radius;
	int cur_sight_player = *get_cur_sight_player();
	uint64_t *sight_player = get_all_sight_player();
	for (int i = 0; i < cur_sight_player; ++i)
	{
		player_struct *player = player_manager::get_player_by_id(sight_player[i]);
		if (!player || !player->is_alive())
		{
			LOG_ERR("%s %d: unit[%lu] can't find player[%p][%lu] in sight", __FUNCTION__, __LINE__, get_uuid(), player, sight_player[i]);
			continue;
		}

		if (!check_fight_type(player, bfriend))
			continue;

		if (player->is_too_high_to_beattack())
			continue;

		double x = pos->pos_x - player->get_pos()->pos_x;
		double z = pos->pos_z - player->get_pos()->pos_z;
		if (x * x + z * z > radius)
			continue;
		ret->push_back(player);
		if (ret->size() >= max)
			return (0);
	}
	int cur_sight_monster = *get_cur_sight_monster();
	uint64_t *sight_monster = get_all_sight_monster();
	for (int i = 0; i < cur_sight_monster; ++i)
	{
		monster_struct *monster = monster_manager::get_monster_by_id(sight_monster[i]);
		if (!monster || monster->mark_delete || !monster->is_alive())
		{
			LOG_ERR("%s %d: player[%lu] in sight", __FUNCTION__, __LINE__, sight_monster[i]);
			continue;
		}

		if (!check_fight_type(monster, bfriend))
			continue;
		
		double x = pos->pos_x - monster->get_pos()->pos_x;
		double z = pos->pos_z - monster->get_pos()->pos_z;
		if (x * x + z * z > radius)
			continue;
		ret->push_back(monster);
		if (ret->size() >= max)
			return (0);
	}
	return (0);
}
int unit_struct::count_fan_unit(std::vector<unit_struct *> *ret, uint max, double radius, double angle, bool bfriend)
{
	struct position *my_pos = get_pos();
	radius = radius * radius;
	double my_angle = pos_to_angle(my_pos->pos_x, my_pos->pos_z);
	double angle_min = my_angle - angle;
	double angle_max = my_angle + angle;

	int cur_sight_player = *get_cur_sight_player();
	uint64_t *sight_player = get_all_sight_player();
	for (int i = 0; i < cur_sight_player; ++i)
	{
		player_struct *player = player_manager::get_player_by_id(sight_player[i]);
		if (!player || !player->is_alive())
		{
			LOG_ERR("%s %d: dead player[%lu][%p] in sight", __FUNCTION__, __LINE__, sight_player[i], player);
			continue;
		}

		if (!check_fight_type(player, bfriend))
			continue;
		
		if (player->is_too_high_to_beattack())
			continue;

		double x = my_pos->pos_x - player->get_pos()->pos_x;
		double z = my_pos->pos_z - player->get_pos()->pos_z;
		if (x * x + z * z > radius)
			continue;
		double angle_target = pos_to_angle(player->get_pos()->pos_x, player->get_pos()->pos_z);
		if (angle_target >= angle_min && angle_target <= angle_max)
		{
			ret->push_back(player);
			if (ret->size() >= max)
				return (0);
		}
	}
	return (0);
}

double unit_struct::get_skill_angle()
{
	return 0;
}
struct position *unit_struct::get_skill_target_pos()
{
	return NULL;
}

player_struct *unit_struct::get_owner()
{
	return NULL;
}

uint32_t unit_struct::get_skill_lv(uint32_t skillid)
{
	return (0);
}

int unit_struct::count_skill_hit_unit(std::vector<unit_struct *> *ret, struct SkillTable *config, bool bfriend)
{
	if (!config)
		return (-1);
	switch (config->RangeType)
	{
		case SKILL_RANGE_TYPE_RECT:
			return count_rect_unit(get_skill_angle(), ret, config->MaxCount, config->Radius, config->Angle, bfriend);
		case SKILL_RANGE_TYPE_CIRCLE:
			return count_circle_unit(ret, config->MaxCount, get_pos(), config->Radius, bfriend);						
		case SKILL_RANGE_TYPE_TARGET_CIRCLE:			
			return count_circle_unit(ret, config->MaxCount, get_skill_target_pos(), config->Radius, bfriend);			
		case SKILL_RANGE_TYPE_FAN:
			return count_fan_unit(ret, config->MaxCount, config->Radius, config->Angle, bfriend);
		case SKILL_RANGE_TYPE_TARGET_RECT:
			return count_rect_unit_at_pos(get_skill_angle(), get_skill_target_pos(), ret, config->MaxCount, config->Radius, config->Angle, bfriend);
		default:
			return -10;
	}
	
	return (0);
}
