#include "cash_truck.h"
#include "check_range.h"
#include "game_config.h"
#include "scene_manager.h"
#include "time_helper.h"
#include "camp_judge.h"
#include "player_manager.h"
#include "monster_manager.h"
#include "partner_manager.h"
#include "team.h"
#include "uuid.h"
#include "../../proto/xunbao.pb-c.h"
#include "../../proto/role.pb-c.h"
#include "msgid.h"
#include "scene.h"
#include "collect.h"
#include "attr_calc.h"
#include "sortarray.h"
#include "unit.h"
#include "buff.h"
#include "app_data_statis.h"
#include "cached_hit_effect.h"
#include "count_skill_damage.h"
#include "sight_space_manager.h"

UNIT_TYPE cash_truck_struct::get_unit_type()
{
	return UNIT_TYPE_CASH_TRUCK;
}

bool cash_truck_struct::is_avaliable()
{
	return data != NULL && scene != NULL;
}

uint64_t cash_truck_struct::get_uuid()
{
	return data->player_id;
}

double *cash_truck_struct::get_all_attr()
{
	return &data->attrData[0];
}

double cash_truck_struct::get_attr(uint32_t id)
{
	return data->attrData[id];
}

double *cash_truck_struct::get_all_buff_fight_attr()
{
	return &data->buff_fight_attr[0];
}

double cash_truck_struct::get_buff_fight_attr(uint32_t id)
{
	assert(id < MAX_BUFF_FIGHT_ATTR);
	return data->buff_fight_attr[id];
}

player_struct *cash_truck_struct::get_owner()
{
	if (!data)
		return NULL;
	return player_manager::get_player_by_id(data->owner);
}

bool cash_truck_struct::can_beattack()
{
	if (buff_state & BUFF_STATE_GOD)
		return false;

	return true;
}

bool cash_truck_struct::is_in_safe_region()
{
	return false;
}

int cash_truck_struct::get_camp_id()
{
	return data->camp_id;
}

void cash_truck_struct::set_camp_id(int id)
{
	data->camp_id = id;
}

// int cash_truck_struct::get_pk_type()
// {
// 	//怪物模式	
// 	return 3;
// }

void cash_truck_struct::set_attr(uint32_t id, double value)
{
	assert(id < PLAYER_ATTR_MAX);
	data->attrData[id] = value;
}

struct unit_path *cash_truck_struct::get_unit_path()
{
	return &data->move_path;
}

float cash_truck_struct::get_speed()
{
	ParameterTable *table = get_config_by_id(161000225, &parameter_config);
	if (table == NULL)
	{
		return data->attrData[PLAYER_ATTR_MOVE_SPEED];
	}
	if (data->n_speed_up > 0)
	{
		return data->attrData[PLAYER_ATTR_MOVE_SPEED] * (1 + table->parameter1[3] * data->n_speed_up / 10000.0);
	}
	else if (time_helper::get_cached_time() < data->speed_reduce)
	{
		return data->attrData[PLAYER_ATTR_MOVE_SPEED] * (table->parameter1[5] / 10000.0);
	}
	else
	{
		return data->attrData[PLAYER_ATTR_MOVE_SPEED];
	}
	
}

int *cash_truck_struct::get_cur_sight_player()
{
	return &data->cur_sight_player;
}

uint64_t *cash_truck_struct::get_all_sight_player()
{
	return &data->sight_player[0];
}

int *cash_truck_struct::get_cur_sight_monster()
{
	return &data->cur_sight_monster;
}

uint64_t *cash_truck_struct::get_all_sight_monster()
{
	return &data->sight_monster[0];
}
int *cash_truck_struct::get_cur_sight_partner()
{
	return &data->cur_sight_partner;
}

uint64_t *cash_truck_struct::get_all_sight_partner()
{
	return &data->sight_partner[0];
}
int *cash_truck_struct::get_cur_sight_truck()
{
	return NULL;
}

uint64_t *cash_truck_struct::get_all_sight_truck()
{
	return NULL;
}

JobDefine cash_truck_struct::get_job() //不用的话删掉
{
	return JOB_DEFINE_MONSTER;
}

void cash_truck_struct::calculate_attribute(void)
{
	std::map<uint64_t, struct MonsterTable *>::iterator ite = monster_config.find(data->monster_id);
	if (ite == monster_config.end())
		return;
	config = ite->second;
	::get_attr_from_config(config->BaseAttribute * 1000 + get_attr(PLAYER_ATTR_LEVEL), data->attrData);
	drop_id = config->DropID * 1000 + 1;
	data->attrData[PLAYER_ATTR_MOVE_SPEED] = config->MoveSpeed;	
	data->attrData[PLAYER_ATTR_HP] = data->attrData[PLAYER_ATTR_MAXHP];
	//data->attrData[PLAYER_ATTR_MOVE_SPEED] = 2;

	calculate_buff_fight_attr(true);
}

cash_truck_struct::~cash_truck_struct()
{
}

void cash_truck_struct::init_cash_truck()
{
	init_unit_struct();
	scene = NULL;
	area = NULL;
	config = NULL;
	lock_time = 0;
	drop_id = 0;
	buff_state = 0;
	data->fb_time = 0;
	data->endurance = 100;

	sight_space = NULL;
	memset(&m_buffs[0], 0, sizeof(m_buffs));
}

void cash_truck_struct::on_tick()
{
	bool noteSpeed = false;
	if (data->n_speed_up > 0)
	{
		if (data->speed_up[0] <= time_helper::get_cached_time())
		{
			--data->n_speed_up;
			if (data->n_speed_up > 0)
			{
				memmove(data->speed_up, data->speed_up + 1, data->n_speed_up * sizeof(uint64_t));
			}
			noteSpeed = true;
		}
	}
	else if (data->speed_reduce != 0 && time_helper::get_cached_time() > data->speed_reduce)
	{
		data->speed_reduce = 0;
		noteSpeed = true;
	}
	
	player_struct *player = player_manager::get_player_by_id(data->owner);
	EXTERN_DATA extern_data;
	if (noteSpeed)
	{
		//通知客户端减速
		PlayerAttrNotify nty;
		player_attr_notify__init(&nty);
		AttrData attr_data;
		AttrData *attr_data_point[2] = {&attr_data};
		nty.player_id = get_uuid();
		nty.attrs = attr_data_point;
		attr_data__init(&attr_data);
		attr_data.id = PLAYER_ATTR_MOVE_SPEED;
		attr_data.val = get_speed();
		nty.n_attrs = 1;
		if (player != NULL && player->sight_space != NULL)
		{
			extern_data.player_id = player->get_uuid();
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_PLAYER_ATTR_NOTIFY, player_attr_notify__pack, nty);
		}
		else
		{
			broadcast_to_sight(MSG_ID_PLAYER_ATTR_NOTIFY, (void *)&nty, (pack_func)player_attr_notify__pack, true);
		}
		
	}
	
	if (player == NULL)
	{
		return;
	}
	if (player->data->truck.on_truck)
	{
		ParameterTable *table = get_config_by_id(161000225, &parameter_config);
		if (table != NULL && data->next_add_endruance < time_helper::get_cached_time())
		{
			uint32_t old = data->endurance;
			
			data->next_add_endruance = time_helper::get_cached_time() + 1000;
			if (data->endurance < table->parameter1[0])
			{
				data->endurance += table->parameter1[1];
			}
			
			if (data->endurance > table->parameter1[0])
			{
				data->endurance = table->parameter1[0];
			}

			if (old != data->endurance)
			{
				TruckEndurance send;
				truck_endurance__init(&send);
				send.endurance = data->endurance;
				extern_data.player_id = player->get_uuid();
				fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_CASH_TRUCK_ENDURANCE_NOTIFY, truck_endurance__pack, send);
			}
		}
	}
	if (player->sight_space == NULL && data->fb_time < time_helper::get_cached_time() && player->data->truck.jiefei < truck_config->Time)
	{
		data->fb_time = time_helper::get_cached_time() + truck_config->Interval * 1000;

		if (!player->data->truck.on_truck)
		{
			return;
		}
		uint64_t r = rand();
		if (r % 10000 > truck_config->Probability)
		{
			return;
		}
		player->go_down_cash_truck();
		player->sight_space = sight_space_manager::create_sight_space(player, 2);

		if (!player->sight_space)
			return;
		
		//player->stop_move();
		for (uint32_t num = 0; num < truck_config->Number[player->data->truck.jiefei]; ++num)
		{
			int lv = player->get_attr(PLAYER_ATTR_LEVEL) + truck_config->level[0] - rand() % (truck_config->level[0] * 2);
			if (lv < 1)
			{
				lv = 1;
			}
			ParameterTable *tableLv = get_config_by_id(161000020, &parameter_config);
			if (tableLv != NULL && tableLv->parameter1[0] < lv)
			{
				lv = tableLv->parameter1[0];
			}
			float x = player->get_pos()->pos_x + truck_config->Range - rand() % (truck_config->Range * 2);
			float z = player->get_pos()->pos_z + truck_config->Range - rand() % (truck_config->Range * 2);
			while (true)
			{
				struct map_block *block_start = get_map_block(this->scene->map_config, x, z);
				if (block_start != NULL && block_start->can_walk == true)
					break;
				x = player->get_pos()->pos_x + truck_config->Range - rand() % (truck_config->Range * 2);
				z = player->get_pos()->pos_z + truck_config->Range - rand() % (truck_config->Range * 2);
			}
			monster_manager::create_sight_space_monster(player->sight_space, player->scene,
				truck_config->shanfeiId[rand() % truck_config->n_shanfeiId], lv, x, z, NULL);
		}
		++player->data->truck.jiefei;
	}
	
}

void cash_truck_struct::on_dead(unit_struct *killer)
{
	player_struct *pOwner = player_manager::get_player_by_id(data->owner);
	if (pOwner == NULL)
	{
		return;
	}
	
	BiaocheRewardTable *reward_config = get_config_by_id(truck_config->Reward, &cash_truck_reward_config);
	if (reward_config == NULL)
	{
		return;
	}
	if (get_truck_type() == 2)
	{
		Collect *pCollect = Collect::CreateCollectByPos(this->scene, reward_config->Collection, this->get_pos()->pos_x, 57, this->get_pos()->pos_z, 0);
		if (pCollect != NULL)
		{
			pCollect->m_ownerLv = pOwner->get_attr(PLAYER_ATTR_LEVEL);
			pCollect->m_active = truck_config->ID;
		}
	}

	pOwner->set_task_fail(pOwner->get_task_info(truck_config->TaskId));
}

void cash_truck_struct::on_beattack(unit_struct *player, uint32_t skill_id, int32_t damage)
{
	unit_struct::on_beattack(player, skill_id, damage);	
	player_struct *pOwner = player_manager::get_player_by_id(data->owner);
	if (pOwner == NULL)
	{
		return;
	}
	pOwner->send_system_notice(190500299, NULL);
	if (pOwner->data->truck.on_truck)
	{
		pOwner->go_down_cash_truck();
	}
}

void cash_truck_struct::pack_sight_cash_truck_info(SightCashTruckInfo *info)
{
	sight_cash_truck_info__init(info);
	info->hp = data->attrData[PLAYER_ATTR_HP];
	info->uuid = data->player_id;
	info->monsterid = data->monster_id;
	info->lv = data->attrData[PLAYER_ATTR_LEVEL];
	info->pos_y = data->pos_y;
	info->maxhp = data->attrData[PLAYER_ATTR_MAXHP];

	
	info->speed = data->attrData[PLAYER_ATTR_MOVE_SPEED];
	info->buff_info = pack_unit_buff(&info->n_buff_info);
	info->pk_type = data->attrData[PLAYER_ATTR_PK_TYPE];; //pk模式

		

	info->owner = this->data->owner;
	//info->truck_player = NULL;
	//info->n_truck_player = 0;
	bool self_path = true;
	player_struct *player = player_manager::get_player_by_id(info->owner);
	if (player != NULL)
	{
		info->owner_name = player->get_name();
		if (player->get_team() != NULL)
		{
			info->team_id = player->get_team()->m_data->m_id;
		}
		info->zhenying_id = get_attr(PLAYER_ATTR_ZHENYING);

		if (player->data->truck.on_truck)
		{
			info->data = player->pack_unit_move_path(&info->n_data);
			info->direct_x = player->data->move_path.direct_x;
			info->direct_z = player->data->move_path.direct_z; 
			self_path = false;
			info->on = true;
		}
	}
	if (self_path)
	{
		info->data = pack_unit_move_path(&info->n_data);
		info->direct_x = data->move_path.direct_x;
		info->direct_z = data->move_path.direct_z;
	}
}

int cash_truck_struct::broadcast_cash_truck_delete(bool send)
{
	assert(data);

	SightChangedNotify notify;
	uint64_t del_uuid[1];
	uint64_t *ppp;
	sight_changed_notify__init(&notify);
	del_uuid[0] = data->player_id;
	notify.delete_cash_truck = del_uuid;
	//发送给需要在视野里面添加玩家的通知
	notify.n_delete_cash_truck = 1;
	
	ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);

	for (int i = 0; i < data->cur_sight_player; ++i)
	{
		player_struct *player = player_manager::get_player_by_id(data->sight_player[i]);
		if (player)
		{
			player->del_truck_from_sight(data->player_id);
			if (player->get_uuid() == data->owner && player->data->truck.on_truck)
			{
				//在镖车上不发离开视野给前端
			}
			else
			{
				conn_node_gamesrv::broadcast_msg_add_players(data->sight_player[i], ppp);	
			}
		}
		else
		{
			LOG_ERR("%s: %lu can not find sight player %lu", __FUNCTION__, get_uuid(), data->sight_player[i]);			
		}
	}
	for (int i = 0; i < data->cur_sight_monster; ++i)
	{
		monster_struct *monster = monster_manager::get_monster_by_id(data->sight_monster[i]);
		if (monster)
		{
			monster->del_truck_from_sight(data->player_id);
		}
		else
		{
			LOG_ERR("%s: %lu can not find sight monster %lu", __FUNCTION__, get_uuid(), data->sight_monster[i]);
		}
	}
	for (int i = 0; i < data->cur_sight_partner; ++i)
	{
		partner_struct *partner = partner_manager::get_partner_by_uuid(data->sight_partner[i]);
		if (partner)
		{
			partner->del_truck_from_sight(data->player_id);
		}
		else
		{
			LOG_ERR("%s: %lu can not find sight partner %lu", __FUNCTION__, get_uuid(), data->sight_partner[i]);
		}
	}
	
	if (send)
	{
		conn_node_gamesrv::broadcast_msg_send();
	}

	clear_cash_truck_sight();
	return (0);
}

int cash_truck_struct::broadcast_cash_truck_create()
{
	if (!area)
		return (0);

	LOG_DEBUG("%s %d: create monster %u %lu at area %ld[%.1f][%.1f]", __FUNCTION__, __LINE__, data->monster_id, get_uuid(),
		area - scene->m_area, get_pos()->pos_x, get_pos()->pos_z);

	SightChangedNotify notify;
	sight_changed_notify__init(&notify);
	SightCashTruckInfo truck_info[1];
	SightCashTruckInfo *truck_info_point[1];
	truck_info_point[0] = &truck_info[0];
	notify.n_add_cash_truck = 1;
	notify.add_cash_truck = truck_info_point;
	pack_sight_cash_truck_info(truck_info_point[0]);

	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
	PROTO_HEAD_CONN_BROADCAST *head;
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;

	add_area_player_to_sight(area, &head->num_player_id, ppp);
	for (int i = 0; i < MAX_NEIGHBOUR_AREA; ++i)
	{
		add_area_player_to_sight(area->neighbour[i], &head->num_player_id, ppp);
	}

	add_area_monster_to_sight(area);
	for (int i = 0; i < MAX_NEIGHBOUR_AREA; ++i)
	{
		add_area_monster_to_sight(area->neighbour[i]);
	}
	add_area_partner_to_sight(area);
	for (int i = 0; i < MAX_NEIGHBOUR_AREA; ++i)
	{
		add_area_partner_to_sight(area->neighbour[i]);
	}

	if (head->num_player_id > 0)
	{
		head->len += sizeof(uint64_t) * head->num_player_id;
		conn_node_gamesrv::broadcast_msg_send();
	}
	reset_pools();

	return (0);
}

int cash_truck_struct::broadcast_cash_truck_move()
{
	if (!area && !sight_space)
		return (0);

	MoveNotify notify;
	move_notify__init(&notify);
	notify.playerid = data->player_id;
	notify.data = pack_unit_move_path(&notify.n_data);

	broadcast_to_sight(MSG_ID_MOVE_NOTIFY, &notify, (pack_func)move_notify__pack, false);
	reset_pos_pool();
	return (0);
}

void cash_truck_struct::clear_cash_truck_sight()
{
	LOG_DEBUG("%s: %lu", __FUNCTION__, get_uuid());
	data->cur_sight_player = 0;
	data->cur_sight_monster = 0;
	data->cur_sight_partner = 0;
}

void cash_truck_struct::update_sight(area_struct *old_area, area_struct *new_area)
{
	if (!old_area || !new_area || old_area == new_area)
		return;
	area_struct *del_area[MAX_NEIGHBOUR_AREA + 1];
	area_struct *add_area[MAX_NEIGHBOUR_AREA + 1];
	int n_add, n_del;
	area_struct::area_neighbour_diff(old_area, new_area, del_area, &n_del, add_area, &n_add);
	assert(n_add <= MAX_NEIGHBOUR_AREA + 1);
	assert(n_del <= MAX_NEIGHBOUR_AREA + 1);

	int delete_player_id_index = 0;
	uint64_t delete_player_id[MAX_PLAYER_IN_CASH_TRUCK_SIGHT];

	SightChangedNotify notify;
	sight_changed_notify__init(&notify);

	del_sight_player_in_area(n_del, &del_area[0], &delete_player_id_index, &delete_player_id[0]);
	del_sight_monster_in_area(n_del, &del_area[0]);
	del_sight_partner_in_area(n_del, &del_area[0]);	

	//发送给需要在视野里面删除怪物的通知
	notify.n_delete_cash_truck = 1;
	uint64_t player_ids[1];
	player_ids[0] = data->player_id;
	notify.delete_cash_truck = player_ids;
	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
	for (int i = 0; i < delete_player_id_index; ++i)
		conn_node_gamesrv::broadcast_msg_add_players(delete_player_id[i], ppp);
	conn_node_gamesrv::broadcast_msg_send();

	sight_changed_notify__init(&notify);
	uint16_t add_player_id_index = 0;
	for (int i = 0; i < n_add; ++i)
	{
		add_area_player_to_sight(add_area[i], &add_player_id_index, delete_player_id);
		add_area_monster_to_sight(add_area[i]);
		add_area_partner_to_sight(add_area[i]);
	}
	if (add_player_id_index > 0)
	{
		SightChangedNotify notify;
		sight_changed_notify__init(&notify);
		SightCashTruckInfo truck_info[1];
		SightCashTruckInfo *truck_info_point[1];
		truck_info_point[0] = &truck_info[0];
		notify.n_add_cash_truck = 1;
		notify.add_cash_truck = truck_info_point;
		pack_sight_cash_truck_info(truck_info_point[0]);

		ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
		for (int i = 0; i < add_player_id_index; ++i)
			conn_node_gamesrv::broadcast_msg_add_players(delete_player_id[i], ppp);
		conn_node_gamesrv::broadcast_msg_send();
	}

	reset_pools();

	if (old_area->del_truck_from_area(data->player_id) != 0)
	{
		LOG_ERR("%s %d: can not del monster[%lu] from area[%ld %p]", __FUNCTION__, __LINE__, data->player_id, old_area - scene->m_area, old_area);
	}
	new_area->add_truck_to_area(data->player_id);
	area = new_area;
}

int cash_truck_struct::prepare_add_player_to_sight(player_struct *player)
{
	if (data->cur_sight_player < MAX_PLAYER_IN_CASH_TRUCK_SIGHT)
		return (0);

	//todo 检查关系的优先级，然后删除低优先级的来腾出空间
	return (-1);
}
int cash_truck_struct::prepare_add_monster_to_sight(monster_struct *monster)
{
	if (data->cur_sight_monster < MAX_MONSTER_IN_CASH_TRUCK_SIGHT)
		return (0);

	//todo 检查关系的优先级，然后删除低优先级的来腾出空间
	return (-1);
}
int cash_truck_struct::prepare_add_partner_to_sight(partner_struct *partner)
{
	if (data->cur_sight_partner < MAX_PARTNER_IN_CASH_TRUCK_SIGHT)
		return (0);

	//todo 检查关系的优先级，然后删除低优先级的来腾出空间
	return (-1);
}

bool cash_truck_struct::on_truck_leave_sight(uint64_t player_id)
{
//	LOG_DEBUG("%s player[%lu] monster[%p][%lu]", __FUNCTION__, player_id, this, get_uuid());

	return true;
}
bool cash_truck_struct::on_truck_enter_sight(uint64_t player_id)
{
//	LOG_DEBUG("%s player[%lu] monster[%p][%lu]", __FUNCTION__, player_id, this, get_uuid());
	return true;
}

bool cash_truck_struct::on_player_leave_sight(uint64_t player_id)
{
//	LOG_DEBUG("%s player[%lu] monster[%p][%lu]", __FUNCTION__, player_id, this, get_uuid());

	return true;
}
bool cash_truck_struct::on_player_enter_sight(uint64_t player_id)
{
//	LOG_DEBUG("%s player[%lu] monster[%p][%lu]", __FUNCTION__, player_id, this, get_uuid());
	return true;
}

bool cash_truck_struct::on_monster_leave_sight(uint64_t uuid)
{
	return true;
}
bool cash_truck_struct::on_monster_enter_sight(uint64_t uuid)
{
	return true;
}

bool cash_truck_struct::on_partner_leave_sight(uint64_t uuid)
{
	return true;
}
bool cash_truck_struct::on_partner_enter_sight(uint64_t uuid)
{
	return true;
}

void cash_truck_struct::add_sight_space_player_to_sight(sight_space_struct *sight_space, uint16_t *add_player_id_index, uint64_t *add_player)
{
	for (int j = 0; j < MAX_PLAYER_IN_SIGHT_SPACE; ++j)
	{
		player_struct *player = sight_space->players[j];
		if (!player)
			continue;
		if (player->add_cash_truck_to_sight_both(this) >= 0)
		{
			add_player[*add_player_id_index] = player->data->player_id;
			(*add_player_id_index)++;
		}
	}
}

void cash_truck_struct::add_sight_space_monster_to_sight(sight_space_struct *sight_space)
{
	for (int j = 0; j < MAX_MONSTER_IN_SIGHT_SPACE; ++j)
	{
		monster_struct *monster = sight_space->monsters[j];
		if (!monster)
			continue;
		monster->add_truck_to_sight_both(this);
	}
}

bool cash_truck_struct::can_see_player(player_struct *player)
{
	return player->is_monster_in_sight(data->player_id);
}

void cash_truck_struct::del_sight_player_in_area(int n_del, area_struct **del_area, int *delete_player_id_index, uint64_t *delete_player_id)
{
	int i, j;
	int index = 0;
	player_struct *del_sight_player[MAX_PLAYER_IN_CASH_TRUCK_SIGHT];

	for (i = 0; i < data->cur_sight_player; ++i)
	{
		player_struct *player = player_manager::get_player_by_id(data->sight_player[i]);
		if (!player)
		{
			//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: %lu can not find sight player %lu", __FUNCTION__, __LINE__, data->player_id, data->sight_player[i]);
			continue;
		}
		for (j = 0; j < n_del; ++j)
		{
			if (player->area != del_area[j])
				continue;
			delete_player_id[(*delete_player_id_index)++] = player->data->player_id;
			del_sight_player[index++] = player;
		}
	}
	for (i = 0; i < index; ++i)
		del_sight_player[i]->del_cash_truck_from_sight_both(this);
}

void cash_truck_struct::del_sight_monster_in_area(int n_del, area_struct **del_area)
{
	int i, j;
	int index = 0;
	monster_struct *del_sight_monster[MAX_MONSTER_IN_CASH_TRUCK_SIGHT];

	for (i = 0; i < data->cur_sight_monster; ++i)
	{
		monster_struct *monster = monster_manager::get_monster_by_id(data->sight_monster[i]);
		if (!monster)
		{
			//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: %lu can not find sight monster %lu", __FUNCTION__, __LINE__, data->player_id, data->sight_monster[i]);
			continue;
		}
		for (j = 0; j < n_del; ++j)
		{
			if (monster->area != del_area[j])
				continue;
			del_sight_monster[index++] = monster;
		}
	}

	for (i = 0; i < index; ++i)
		del_sight_monster[i]->del_truck_from_sight_both(this);
}
void cash_truck_struct::del_sight_partner_in_area(int n_del, area_struct **del_area)
{
	int i, j;
	int index = 0;
	partner_struct *del_sight_partner[MAX_PARTNER_IN_CASH_TRUCK_SIGHT];

	for (i = 0; i < data->cur_sight_partner; ++i)
	{
		partner_struct *partner = partner_manager::get_partner_by_uuid(data->sight_partner[i]);
		if (!partner)
		{
			//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: %lu can not find sight partner %lu", __FUNCTION__, __LINE__, data->player_id, data->sight_partner[i]);
			continue;
		}
		for (j = 0; j < n_del; ++j)
		{
			if (partner->area != del_area[j])
				continue;
			del_sight_partner[index++] = partner;
		}
	}

	for (i = 0; i < index; ++i)
		del_partner_from_sight_both(del_sight_partner[i]);

}

int cash_truck_struct::del_partner_from_sight_both(partner_struct *partner)
{
	int ret = del_partner_from_sight(partner->data->uuid);
	if (ret >= 0)
	{
		int ret1 = partner->del_truck_from_sight(data->player_id);
		assert(ret1 >= 0);
	}
	return ret;
}

int cash_truck_struct::add_partner_to_sight_both(partner_struct *partner)
{
	if (prepare_add_partner_to_sight(partner) != 0 ||
		partner->prepare_add_truck_to_sight(this) != 0)
		return -1;
	
	int ret = add_partner_to_sight(partner->data->uuid);
	assert(ret >= 0);
	int ret1 = partner->add_truck_to_sight(data->player_id);
	assert(ret1 >= 0);
	return ret;
}
// int cash_truck_struct::del_monster_from_sight_both(monster_struct *monster)
// {
// 	int ret = del_monster_from_sight(monster->data->player_id);
// 	if (ret >= 0)
// 	{
// 		int ret1 = monster->del_monster_from_sight(data->player_id);
// 		//		LOG_DEBUG("%s: %lu[%u] del monster %lu[%u]",
// 		//			__FUNCTION__, get_uuid(), data->cur_sight_monster,
// 		//			monster->get_uuid(), monster->data->cur_sight_monster);		
// 		assert(ret1 >= 0);
// 	}
// 	return ret;
// }
// int cash_truck_struct::add_monster_to_sight_both(monster_struct *monster)
// {
// 	int ret = add_monster_to_sight(monster->data->player_id);
// 	assert(ret >= 0);
// 	int ret1 = monster->add_truck_to_sight(data->player_id);
// 	assert(ret1 >= 0);
// 	return ret;
// }

void cash_truck_struct::add_area_player_to_sight(area_struct *area, uint16_t *add_player_id_index, uint64_t *add_player)
{
	if (!area)
		return;
	for (int j = 0; j < area->cur_player_num; ++j)
	{
		player_struct *player = player_manager::get_player_by_id(area->m_player_ids[j]);
		if (!player)
		{
			//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: can not find sight player %lu area[%p]", __FUNCTION__, __LINE__, area->m_player_ids[j], area);
			continue;
		}
		if (player->prepare_add_truck_to_sight(this) != 0) 
			continue;
		if (player->add_cash_truck_to_sight_both(this) >= 0)
		{
			if (get_entity_type(area->m_player_ids[j]) == ENTITY_TYPE_PLAYER)
			{
				add_player[*add_player_id_index] = player->data->player_id;
				(*add_player_id_index)++;
			}
		}
	}
}

void cash_truck_struct::add_area_partner_to_sight(area_struct *area)
{
	if (!area)
		return;
	for (int j = 0; j < area->cur_partner_num; ++j)
	{
		if (area->m_partner_uuid[j] == data->player_id)
			continue;

		partner_struct *partner = partner_manager::get_partner_by_uuid(area->m_partner_uuid[j]);
		if (!partner)
		{
			//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: can not find sight player %lu area[%p]", __FUNCTION__, __LINE__, area->m_partner_uuid[j], area);
			continue;
		}

		add_partner_to_sight_both(partner);
	}
}

void cash_truck_struct::add_area_monster_to_sight(area_struct *area)
{
	if (!area)
		return;
	for (int j = 0; j < area->cur_monster_num; ++j)
	{
		if (area->m_monster_uuid[j] == data->player_id)
			continue;

		monster_struct *monster = monster_manager::get_monster_by_id(area->m_monster_uuid[j]);
		if (!monster)
		{
			//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: can not find sight player %lu area[%p]", __FUNCTION__, __LINE__, area->m_monster_uuid[j], area);
			continue;
		}

		monster->add_truck_to_sight_both(this);
	}
}

void cash_truck_struct::on_hp_changed(int damage)
{

}

void cash_truck_struct::on_repel(unit_struct *player)
{
}

uint32_t cash_truck_struct::get_skill_id()
{
	return 0;
}

Team *cash_truck_struct::get_team()
{
	return NULL;
}

void cash_truck_struct::clear_cur_skill()
{
	
}

void cash_truck_struct::do_taunt_action()
{

}

