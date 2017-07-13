//
#include <unistd.h>
#include "comm.h"
#include "player.h"
#include "raid.h"
#include "uuid.h"
#include "game_event.h"
#include "server_proto.h"
#include "player_manager.h"
#include "raid_manager.h"
#include "partner_manager.h"
#include "zhenying_raid_manager.h"
#include "monster_manager.h"
#include "cash_truck_manager.h"
#include "partner_manager.h"
#include "collect.h"
#include "cash_truck.h"
#include "sortarray.h"
#include "area.h"
#include "buff.h"
#include "buff_manager.h"
#include "scene.h"
#include "scene_manager.h"
#include "msgid.h"
#include "unit.h"
#include "conn_node.h"
#include "conn_node_dbsrv.h"
#include "check_range.h"
#include "player_db.pb-c.h"
#include "time_helper.h"
#include <stdio.h>
#include <vector>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <algorithm>
#include <sstream>
#include <math.h>
#include <stdlib.h>
#include "game_config.h"
#include "attr_calc.h"
#include "../proto/role.pb-c.h"
#include "../proto/task.pb-c.h"
#include "../proto/move_direct.pb-c.h"
#include "../proto/scene_transfer.pb-c.h"
#include "../proto/collect.pb-c.h"
#include "../proto/player_redis_info.pb-c.h"
#include "../proto/relive.pb-c.h"
#include "../proto/cast_skill.pb-c.h"
#include "../proto/raid.pb-c.h"
#include "../proto/fashion.pb-c.h"
#include "../proto/horse.pb-c.h"
#include "../proto/equip.pb-c.h"
#include "../proto/pk.pb-c.h"
#include "../proto/pvp_raid.pb-c.h"
#include "../proto/hotel.pb-c.h"
#include "../proto/mail_db.pb-c.h"
#include "../proto/shop.pb-c.h"
#include "../proto/yuqidao.pb-c.h"
#include "../proto/activity.pb-c.h"
#include "../proto/chat.pb-c.h"
#include "../proto/bag.pb-c.h"
#include "../proto/xunbao.pb-c.h"
#include "auto_add_hp.pb-c.h"
#include "error_code.h"
#include "app_data_statis.h"
#include "sight_space_manager.h"
#include "buff.h"
#include "buff_manager.h"
#include "check_range.h"
#include "send_mail.h"
#include "camp_judge.h"
#include "pvp_match_manager.h"
#include "guild_battle_manager.h"
#include "chengjie.h"
#include "guild_wait_raid_manager.h"
#include "partner_manager.h"
#include "partner.pb-c.h"

ItemUseEffectInfo::~ItemUseEffectInfo()
{
//	LOG_ERR("[%s:%d]", __FUNCTION__, __LINE__);
	pos = 0;
	use_all = 0;
	is_easy = 0;
	items.clear();
}

uint32_t FightingCapacity::get_total(void)
{
	return (level + equip + horse + yuqidao + bagua + guild_skill + partner + fashion);
}

player_struct::player_struct() : m_skill(this)
{
	data = NULL;
	init_player();
}

player_struct::~player_struct()
{
	data = NULL;
}

raid_struct *player_struct::get_raid()
{
	if (!scene || (scene->get_scene_type() != SCENE_TYPE_RAID))
		return NULL;
	return (raid_struct *)scene;
}

UNIT_TYPE player_struct::get_unit_type()
{
	return UNIT_TYPE_PLAYER;
}

bool player_struct::is_avaliable()
{
	return data != NULL && scene != NULL;
}
uint32_t player_struct::get_skill_id()
{
	return data->cur_skill.skill_id;
}

uint64_t player_struct::get_uuid()
{
	return data->player_id;
}
double *player_struct::get_all_attr()
{
	return &data->attrData[0];
}
double player_struct::get_attr(uint32_t id)
{
	return data->attrData[id];
}
double *player_struct::get_all_buff_fight_attr()
{
	return &data->buff_fight_attr[0];
}
double player_struct::get_buff_fight_attr(uint32_t id)
{
	assert(id < MAX_BUFF_FIGHT_ATTR);
	return data->buff_fight_attr[id];
}

void player_struct::add_attr(uint32_t id, double value)
{
	assert(id < PLAYER_ATTR_MAX);
	data->attrData[id] += value;
}
void player_struct::set_attr(uint32_t id, double value)
{
	assert(id < PLAYER_ATTR_MAX);
	data->attrData[id] = value;
}
void player_struct::clear_cur_skill()
{
	data->cur_skill.skill_id = 0;
	data->cur_skill.start_time = 0;
}

struct unit_path *player_struct::get_unit_path()
{
	return &data->move_path;
}
float player_struct::get_speed()
{
	if (data->truck.on_truck)
	{
		cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(data->truck.truck_id);
		if (truck != NULL)
		{
			return truck->get_speed();
		}
	}
	return data->attrData[PLAYER_ATTR_MOVE_SPEED];
}
int *player_struct::get_cur_sight_player()
{
	return &data->cur_sight_player;
}
uint64_t *player_struct::get_all_sight_player()
{
	return &data->sight_player[0];
}
int *player_struct::get_cur_sight_monster()
{
	return &data->cur_sight_monster;
}
uint64_t *player_struct::get_all_sight_monster()
{
	return &data->sight_monster[0];
}
int *player_struct::get_cur_sight_truck()
{
	return &data->cur_sight_truck;
}

uint64_t *player_struct::get_all_sight_truck()
{
	return &data->sight_truck[0];
}
int *player_struct::get_cur_sight_partner()
{
	return &data->cur_sight_partner;
}
uint64_t *player_struct::get_all_sight_partner()
{
	return &data->sight_partner[0];
}

void player_struct::try_out_raid()
{
	if (is_raid_scene_id(data->scene_id))
	{
		// struct DungeonTable* r_config = get_config_by_id(data->scene_id, &all_raid_config);
		// if (r_config->DengeonRank == DUNGEON_TYPE_RAND_MASTER)
		// {
		//	r_config = get_config_by_id(r_config->DungeonID, &all_raid_config);
		// }
		set_out_raid_pos();
	}
}

void player_struct::send_raid_hit_statis(raid_struct *raid)
{
	RaidHitStatisChangedNotify notify;
	raid_hit_statis_changed_notify__init(&notify);
	EXTERN_DATA extern_data;
	extern_data.player_id = data->player_id;

	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (raid->data->player_info[i].player_id == 0)
			continue;
		notify.player_id = raid->data->player_info[i].player_id;
		notify.cure = raid->data->player_info[i].cure;
		notify.damage = raid->data->player_info[i].damage;
		notify.injured = raid->data->player_info[i].injured;
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data,
			MSG_ID_RAID_HIT_STATIS_CHANGED_NOTIFY, raid_hit_statis_changed_notify__pack, notify);
	}
}

void player_struct::try_return_zhenying_raid()
{
		// TODO: 检查是否还在活动时间
	zhenying_raid_struct *raid = zhenying_raid_manager::get_avaliable_zhenying_raid();
	if (!raid)
	{
		return try_out_raid();
	}
	scene = raid;

	raid->on_player_enter_raid(this);
}

void player_struct::try_return_guild_wait_raid()
{
	do
	{
		if (!is_guild_battle_opening())
		{
			break;
		}

		if (player_can_participate_guild_battle(this) != 0)
		{
			break;
		}

		guild_wait_raid_struct *raid = guild_wait_raid_manager::get_avaliable_guild_wait_raid(data->guild_id);
		if (!raid)
		{
			break;
		}

		scene = raid;

		raid->on_player_enter_raid(this);

		return;
	} while(0);

	try_out_raid();
}

void player_struct::try_return_guild_battle_raid()
{
	bool to_wait = true;
	do
	{
		if (!is_guild_battle_opening())
		{
			to_wait = false;
			break;
		}

		if (player_can_participate_guild_battle(this) != 0)
		{
			to_wait = false;
			break;
		}

		if (!m_team)
		{
			break;
		}

		if (m_team->m_data->m_raid_uuid == 0)
		{
			break;
		}

		raid_struct *raid = raid_manager::get_raid_by_uuid(m_team->m_data->m_raid_uuid);
		if (!raid)
		{
			break;
		}
		if (raid->data->state != RAID_STATE_START)
		{
			break;
		}
		if (raid->player_return_raid(this) != 0)
		{
			break;
		}

		scene = raid;

		return;
	} while(0);

	if (to_wait)
	{ //回准备区
		data->scene_id = sg_guild_wait_raid_id;
		float pos_x = 0.0, pos_z = 0.0;
		get_scene_birth_pos(data->scene_id, &pos_x, NULL, &pos_z, NULL);
		set_pos(pos_x, pos_z);

		try_return_guild_wait_raid();
	}
	else
	{ //回帮会领地
		data->scene_id = sg_guild_scene_id;
		float pos_x = 0.0, pos_z = 0.0;
		get_scene_birth_pos(data->scene_id, &pos_x, NULL, &pos_z, NULL);
		set_pos(pos_x, pos_z);
	}
}

void player_struct::try_return_raid()
{
	if (data->scene_id == ZHENYING_RAID_ID)
	{
		return try_return_zhenying_raid();
	}
	else if (is_guild_wait_raid(data->scene_id))
	{
		return try_return_guild_wait_raid();
	}
	else if (is_guild_battle_raid(data->scene_id))
	{
		return try_return_guild_battle_raid();
	}

	if (!m_team)
		return try_out_raid();

	if (m_team->m_data->m_raid_uuid == 0)
		return try_out_raid();

	raid_struct *raid = raid_manager::get_raid_by_uuid(m_team->m_data->m_raid_uuid);
	if (!raid)
	{
		return try_out_raid();
	}
	if (raid->data->state != RAID_STATE_START)
	{
		return try_out_raid();
	}

	if (raid->player_return_raid(this) != 0)
	{
		return try_out_raid();
	}
	scene = raid;
	send_raid_hit_statis(raid);
/*
	if (data->raid_uuid == 0)
		return;
	raid_struct *raid = raid_manager::get_raid_by_uuid(data->raid_uuid);
	if (!raid)
	{
		struct DungeonTable* r_config = get_config_by_id(data->scene_id, &all_raid_config);
		assert(r_config);
		set_pos(r_config->ExitPointX, r_config->BirthPointZ);
		data->scene_id = r_config->ExitScene;
		data->raid_uuid = 0;
		return;
	}
	if (raid->player_enter_raid(this, false) != 0)
	{
		return;
	}
*/
}

bool player_struct::is_chengjie_target(uint64_t player_id)
{
	if (data->chengjie.cur_task != 0 && data->chengjie.target == player_id)
		return true;
	return false;
}

JobDefine player_struct::get_job()
{
	return (JobDefine)(data->attrData[PLAYER_ATTR_JOB]);
}

uint32_t player_struct::get_level()
{
	return (uint32_t)(data->attrData[PLAYER_ATTR_LEVEL]);
}

void player_struct::init_player()
{
	init_unit_struct();
	scene = NULL;
	area = NULL;
	srtt = 0;
	lock_time = 0;
	buff_state = 0;
	last_change_area_time = 0;
	item_pos_cache.clear();
	m_team = NULL;
	memset(&m_buffs[0], 0, sizeof(m_buffs));
//	memset(&sing_info, 0, sizeof(SingInfo));
//	login_notify = false;
	sight_space = NULL;
	m_pet_list.clear();
	bagua_buffs.clear();
//	guild_id = 0;
//	memset(guild_name, 0, sizeof(guild_name));
	ai_patrol_config = NULL;
//	memset(&fc_data, 0, sizeof(FightingCapacity));
	m_partners.clear();
	chengjie_kill = 0;
}

void player_struct::clear(void)
{
	if (data)
	{
		if (data->sing_info.args)
		{
			switch (data->sing_info.type)
			{
				case SING_TYPE__USE_PROP:
				case SING_TYPE__XUNBAO:
					delete (ItemUseEffectInfo*)(data->sing_info.args);
					break;
			}
		}
	}
	clear_all_buffs();
	clear_temp_data();
	clear_all_partners();
	clear_all_escort();
}

bool player_struct::is_in_pvp_raid()
{
	if (!is_in_raid())
		return false;
	if (!((raid_struct *)scene)->m_config)
		return false;
	int type = ((raid_struct *)scene)->m_config->DengeonRank;
	if (type != 5 && type != 6)
		return false;
	return true;
}

bool player_struct::is_in_raid()
{
	if (!data || !scene)
		return false;
	if (scene->get_scene_type() == SCENE_TYPE_RAID)
		return true;
	return false;
}

bool player_struct::is_online(void)
{
	return (data && data->status == ONLINE);
}

void player_struct::update_rtt(uint32_t stamp)
{
	time_t t;
	time(&t);
	int64_t m = t - stamp;
	if (unlikely(srtt == 0)) {
		srtt = (m << 3);
	}
	else {
		m -= (srtt >> 3);
		srtt += m;
	}
}
/*
bool player_struct::is_player_in_sight(uint64_t player_id)
{
	int find;
	array_bsearch(&player_id, &data->sight_player[0], data->cur_sight_player, sizeof(uint64_t), &find, comp_uint64);
	return find;
}

int player_struct::add_player_to_sight(uint64_t player_id)
{
	return array_insert(&player_id, &data->sight_player[0], &data->cur_sight_player, sizeof(uint64_t), 1, comp_uint64);
}

int player_struct::del_player_from_sight(uint64_t player_id)
{
	return array_delete(&player_id, &data->sight_player[0], &data->cur_sight_player, sizeof(uint64_t), comp_uint64);
}
*/

Team *player_struct::get_team()
{
	return m_team;
}

int player_struct::get_camp_id()
{
	return data->camp_id;
}

void player_struct::set_camp_id(int id)
{
	data->camp_id = id;
}

// int player_struct::get_pk_type()
// {
//	return get_attr(PLAYER_ATTR_PK_TYPE);
// }

bool player_struct::is_in_safe_region()
{
	if (get_attr(PLAYER_ATTR_REGION_ID) == 11)
		return true;
	return false;
}

bool player_struct::can_beattack()
{
	if (buff_state & BUFF_STATE_GOD)
		return false;

	// if (get_attr(PLAYER_ATTR_REGION_ID) == 11)
	//	return false;

	return true;
}

int player_struct::add_monster_to_sight_both(monster_struct *monster)
{
	if (prepare_add_monster_to_sight(monster) != 0 ||
		monster->prepare_add_player_to_sight(this) != 0)
		return -1;

	int ret = add_monster_to_sight(monster->data->player_id);
	assert(ret >= 0);
	int ret1 = monster->add_player_to_sight(data->player_id);
	assert(ret1 >= 0);
	return ret;
}
int player_struct::add_partner_to_sight_both(partner_struct *partner)
{
	if (prepare_add_partner_to_sight(partner) != 0 ||
		partner->prepare_add_player_to_sight(this) != 0)
		return -1;

	int ret = add_partner_to_sight(partner->data->uuid);
	assert(ret >= 0);
	int ret1 = partner->add_player_to_sight(data->player_id);
	assert(ret1 >= 0);
	return ret;
}

int player_struct::del_monster_from_sight_both(monster_struct *monster)
{
	int ret = del_monster_from_sight(monster->data->player_id);
	if (ret >= 0)
	{
		int ret1 = monster->del_player_from_sight(data->player_id);
//		LOG_DEBUG("%s: %lu del monster %lu", __PRETTY_FUNCTION__, get_uuid(), monster->get_uuid());
		assert(ret1 >= 0);
	}
	return ret;
}
int player_struct::del_partner_from_sight_both(partner_struct *partner)
{
	int ret = del_partner_from_sight(partner->data->uuid);
	if (ret >= 0)
	{
		int ret1 = partner->del_player_from_sight(data->player_id);
//		LOG_DEBUG("%s: %lu del partner %lu", __PRETTY_FUNCTION__, get_uuid(), partner->get_uuid());
		assert(ret1 >= 0);
	}
	return ret;
}

int player_struct::add_cash_truck_to_sight_both(cash_truck_struct *truck)
{
	if (prepare_add_truck_to_sight(truck) != 0 ||
		truck->prepare_add_player_to_sight(this) != 0)
		return -1;

	int ret = add_truck_to_sight(truck->data->player_id);
	assert(ret >= 0);
	int ret1 = truck->add_player_to_sight(data->player_id);
	assert(ret1 >= 0);

	return ret;
}
int player_struct::del_cash_truck_from_sight_both(cash_truck_struct *truck)
{
	int ret = del_truck_from_sight(truck->data->player_id);
	if (ret >= 0)
	{
		int ret1 = truck->del_player_from_sight(data->player_id);
		assert(ret1 >= 0);
	}

	return ret;
}

static void print_player_sight_player(player_struct *player)
{
	// static char buf[1024];
	// char *p = buf;
	// p += sprintf(p, "[%lu] cur = %d ", player->data->player_id, player->data->cur_sight_player);
	// for (int i = 0; i < player->data->cur_sight_player; ++i)
	//	p += sprintf(p, "[%lu] ", player->data->sight_player[i]);
	// LOG_INFO("%s: %s", __FUNCTION__, buf);
}

int player_struct::add_player_to_sight_both(player_struct *player)
{
	if (prepare_add_player_to_sight(player) != 0 ||
		player->prepare_add_player_to_sight(this) != 0)
		return -1;

	LOG_INFO("%s: player[%lu] enter player[%lu] sight", __FUNCTION__, data->player_id, player->data->player_id);
	print_player_sight_player(player);
	print_player_sight_player(this);

	int ret = add_player_to_sight(player->data->player_id);
	assert (ret >= 0);
	int ret1 = player->add_player_to_sight(data->player_id);
	assert(ret1 >= 0);

	return ret;
}

int player_struct::del_player_from_sight_both(player_struct *player)
{
	LOG_INFO("%s: player[%lu] leave player[%lu] sight", __FUNCTION__, data->player_id, player->data->player_id);
	print_player_sight_player(player);
	print_player_sight_player(this);

	int ret = del_player_from_sight(player->data->player_id);
	if (ret >= 0)
	{
		int ret1 = player->del_player_from_sight(data->player_id);
		assert(ret1 >= 0);
		print_player_sight_player(player);
		print_player_sight_player(this);

	}
	else
	{
		LOG_INFO("%s: fail", __FUNCTION__);
	}

	return ret;
}

bool player_struct::on_player_leave_sight(uint64_t player_id)
{
	return true;
}
bool player_struct::on_player_enter_sight(uint64_t player_id)
{
	return true;
}

bool player_struct::on_partner_leave_sight(uint64_t player_id)
{
	return true;
}
bool player_struct::on_partner_enter_sight(uint64_t player_id)
{
	return true;
}
bool player_struct::on_monster_leave_sight(uint64_t uuid)
{
	return true;
}
bool player_struct::on_monster_enter_sight(uint64_t uuid)
{
	return true;
}

int player_struct::prepare_add_monster_to_sight(monster_struct *monster)
{
		//死了的不进入视野
//	if (monster->data->attrData[PLAYER_ATTR_HP] <= 0)
//		return (-1);

	if (data->cur_sight_monster < MAX_MONSTER_IN_PLAYER_SIGHT)
		return (0);

//todo 检查关系的优先级，然后删除低优先级的来腾出空间
	return (-1);
}
int player_struct::prepare_add_partner_to_sight(partner_struct *partner)
{
		//死了的不进入视野
//	if (partner->data->attrData[PLAYER_ATTR_HP] <= 0)
//		return (-1);

	if (data->cur_sight_partner < MAX_PARTNER_IN_PLAYER_SIGHT)
		return (0);

//todo 检查关系的优先级，然后删除低优先级的来腾出空间
	return (-1);
}

int player_struct::prepare_add_truck_to_sight(cash_truck_struct *truck)
{
	if (data->cur_sight_truck < MAX_TRUCK_IN_PLAYER_SIGHT)
		return (0);
	//todo 检查关系的优先级，然后删除低优先级的来腾出空间
	return (-1);
}

int player_struct::prepare_add_player_to_sight(player_struct *player)
{
	if (data->cur_sight_player < MAX_PLAYER_IN_PLAYER_SIGHT)
		return (0);
//todo 检查关系的优先级，然后删除低优先级的来腾出空间
	return (-1);
}

int player_struct::get_sight_priority(player_struct *player)
{
	return (SIGHT_PRIORITY_NORMAL);
}
#if 0
void player_struct::broadcast_to_sight(uint16_t msg_id, void *msg_data, pack_func func, bool include_myself)
{
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

	memcpy(ppp, &(data->sight_player[0]), sizeof(uint64_t) * data->cur_sight_player);
	head->num_player_id = data->cur_sight_player;
	if (include_myself)
	{
		ppp[head->num_player_id] = data->player_id;
		++head->num_player_id;
	}

	if (head->num_player_id == 0)
		return;

	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD_CONN_BROADCAST) + len + sizeof(uint64_t) * head->num_player_id);

	LOG_DEBUG("%s %d: broad to %d player", __FUNCTION__, __LINE__, head->num_player_id);

	if (conn_node_gamesrv::connecter.send_one_msg((PROTO_HEAD *)head, 1) != (int)(ENDION_FUNC_4(head->len))) {
		LOG_ERR("%s %d: send to all failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
}
#endif
void player_struct::process_offline(bool again/* = false*/, EXTERN_DATA *ext_data/* = NULL*/)
{
	if (is_in_qiecuo())
	{
		player_struct *target = player_manager::get_player_by_id(data->qiecuo_target);
		assert(target);
		target->finish_qiecuo();

		QiecuoFinishNotify nty;
		EXTERN_DATA ext;
		qiecuo_finish_notify__init(&nty);

		nty.result = 0;
		ext.player_id = target->get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_FINISH_NOTIFY, qiecuo_finish_notify__pack, nty);
	}

	data->status = OFFLINE_SAVING;

	pvp_match_on_player_offline(this);
//	del_from_guild_battle_waiting(this);

	std::vector<monster_struct *> t;
	for (std::set<monster_struct *>::iterator ite = m_pet_list.begin(); ite != m_pet_list.end(); ++ite)
	{
		t.push_back(*ite);
	}
	for (std::vector<monster_struct *>::iterator ite = t.begin(); ite != t.end(); ++ite)
	{
		if ((*ite)->scene)
			(*ite)->scene->delete_monster_from_scene(*ite, true);
		monster_manager::delete_monster(*ite);
	}
	m_pet_list.clear();

	if (scene)
	{
		if (!is_alive())
		{
			struct position *pos = get_pos();
			int32_t pos_x = 0, pos_z = 0, direct = 0;
			scene->get_relive_pos(pos->pos_x, pos->pos_z, &pos_x, &pos_z, &direct);
			set_pos(pos_x, pos_z);
			data->attrData[PLAYER_ATTR_HP] = data->attrData[PLAYER_ATTR_MAXHP];
			++data->attrData[PLAYER_ATTR_RELIVE_TYPE2];
		}
		if (scene->get_scene_type() == SCENE_TYPE_RAID)
		{
			raid_struct *raid = (raid_struct *)scene;
				//放在上线的时候做
			// if (raid->m_config->DengeonRank == DUNGEON_TYPE_RAND_MASTER)
			// {
			//	data->scene_id = raid->m_config->DungeonID;
			// }
			raid->player_offline(this);
		}
		else
		{
			scene->delete_player_from_scene(this);
		}
	}

	if (sight_space)
	{
		sight_space_manager::del_player_from_sight_space(sight_space, this, false);
	}

	if (m_team != NULL)
	{
		m_team->MemberOffLine(*this);
	}
	//要放在组队的处理后面，因为队长下线需要拷贝数据
	stop_all_escort();
	clear_team_task();



	logout_check_task_time();
	logout_check_award_question();
	cache_to_dbserver(again, ext_data);

	if (data->truck.truck_id != 0)
	{
		cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(data->truck.truck_id);
		if (truck != NULL)
		{
			truck->scene->delete_cash_truck_from_scene(truck);
			cash_truck_manager::delete_cash_truck(truck);
		}
	}

	//同步信息到redis，放在最后
	refresh_player_redis_info(true);
}

void player_struct::cache_to_dbserver(bool again/* = false*/, EXTERN_DATA *ext_data/* = NULL*/)
{
	if (!data)
		return;

	LOG_DEBUG("%s %d: player[%lu]", __FUNCTION__, __LINE__, data->player_id);

	PROTO_SAVE_PLAYER_REQ *req = (PROTO_SAVE_PLAYER_REQ *)&conn_node_base::global_send_buf[0];

	req->level = data->attrData[PLAYER_ATTR_LEVEL];
	req->data_size = pack_playerinfo_to_dbinfo(&req->data[0]);
	req->again = again ? 1 : 0;
	req->plug = 0;
	req->chengjie_cd = data->chengjie.rest;

	size_t name_len = std::min((uint32_t)sizeof(req->name) - 1, (uint32_t)strlen(data->name));
	memcpy(req->name, data->name, name_len);
	req->name[name_len] = '\0';

	req->head.msg_id = ENDION_FUNC_2(SERVER_PROTO_SAVE_PLAYER);
	req->head.len = ENDION_FUNC_4(sizeof(PROTO_SAVE_PLAYER_REQ) + req->data_size);

	EXTERN_DATA extern_data;
	if (ext_data) {
		extern_data.player_id = ext_data->player_id;
		extern_data.open_id = ext_data->open_id;
		extern_data.fd = ext_data->fd;
		extern_data.port = ext_data->port;
	}
	else {
		extern_data.player_id = data->player_id;
	}
	conn_node_base::add_extern_data(&req->head, &extern_data);


	if (conn_node_dbsrv::connecter.send_one_msg(&req->head, 1) != (int)ENDION_FUNC_4(req->head.len)) {
		LOG_ERR("%s %d: send to dbsrv err[%d]", __FUNCTION__, __LINE__, errno);
	}
}

int player_struct::pack_playerinfo_to_dbinfo(uint8_t *out_data)
{
	PlayerDBInfo db_info;
	player_dbinfo__init(&db_info);

	db_info.exp = data->attrData[PLAYER_ATTR_EXP];
	db_info.scene_id = data->scene_id;
	db_info.last_scene_id = data->last_scene_id;
	db_info.pos_x = get_pos()->pos_x;
	db_info.pos_z = get_pos()->pos_z;
	db_info.pos_y = data->pos_y;
	db_info.next_time_refresh_oneday_job = data->next_time_refresh_oneday_job / 1000;
	db_info.cur_hp = data->attrData[PLAYER_ATTR_HP];
//	db_info.raid_uuid = data->raid_uuid;

	LOG_DEBUG("%s: player[%lu] pos_y = %.1f", __FUNCTION__, data->player_id, db_info.pos_y);

	pack_answer_db(db_info);

	int i;
	for (i = 0; i < MAX_RAID_NUM; ++i)
	{
		if (data->raid_reward_id[i] == 0)
			break;
	}
	db_info.n_raid_reward_id = db_info.n_raid_reward_num = i;
	db_info.raid_reward_id = &data->raid_reward_id[0];
	db_info.raid_reward_num = &data->raid_reward_num[0];

	db_info.angle = data->m_angle;

	//背包
	DBBagGrid bag_data[MAX_BAG_GRID_NUM];
	DBBagGrid *bag_data_point[MAX_BAG_GRID_NUM];
	DBItemBagua item_bagua_data[MAX_BAG_GRID_NUM];
	DBAttr item_bagua_attr[MAX_BAG_GRID_NUM][MAX_BAGUAPAI_MINOR_ATTR_NUM];
	DBAttr* item_bagua_attr_point[MAX_BAG_GRID_NUM][MAX_BAGUAPAI_MINOR_ATTR_NUM];
	for (uint32_t i = 0; i < data->bag_grid_num; ++i)
	{
		bag_data_point[i] = &bag_data[i];
		dbbag_grid__init(&bag_data[i]);
		bag_data[i].id = data->bag[i].id;
		bag_data[i].num = data->bag[i].num;
		bag_data[i].used_count = data->bag[i].used_count;
		bag_data[i].expire_time = data->bag[i].expire_time;
		if (item_is_baguapai(data->bag[i].id))
		{
			bag_data[i].bagua = &item_bagua_data[i];
			dbitem_bagua__init(&item_bagua_data[i]);
			item_bagua_data[i].star = data->bag[i].baguapai.star;
			item_bagua_data[i].main_attr_val = data->bag[i].baguapai.main_attr_val;
			uint32_t attr_num = 0;
			for (int j = 0; j < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++j)
			{
				item_bagua_attr_point[i][attr_num] = &item_bagua_attr[i][attr_num];
				dbattr__init(&item_bagua_attr[i][attr_num]);
				item_bagua_attr[i][attr_num].id = data->bag[i].baguapai.minor_attrs[j].id;
				item_bagua_attr[i][attr_num].val = data->bag[i].baguapai.minor_attrs[j].val;
				attr_num++;
			}
			item_bagua_data[i].minor_attrs = item_bagua_attr_point[i];
			item_bagua_data[i].n_minor_attrs = attr_num;
		}
	}
	db_info.bag = bag_data_point;
	db_info.n_bag = data->bag_grid_num;
	db_info.bag_grid_num = data->bag_grid_num;
	db_info.bag_unlock_num = data->bag_unlock_num;

	//货币
	db_info.gold = data->attrData[PLAYER_ATTR_GOLD];
	db_info.bind_gold = data->attrData[PLAYER_ATTR_BIND_GOLD];
	db_info.coin = data->attrData[PLAYER_ATTR_COIN];
	db_info.teamid = data->teamid;

	//头像
	DBHeadIcon head_data[MAX_HEAD_ICON_NUM];
	DBHeadIcon* head_data_point[MAX_HEAD_ICON_NUM];
	db_info.head_icon = data->attrData[PLAYER_ATTR_HEAD];
	db_info.n_head_icon_list = 0;
	db_info.head_icon_list = head_data_point;
	for (int i = 0; i < MAX_HEAD_ICON_NUM; ++i)
	{
		if (data->head_icon_list[i].id == 0)
		{
			continue;
		}

		head_data_point[db_info.n_head_icon_list] = &head_data[db_info.n_head_icon_list];
		dbhead_icon__init(&head_data[db_info.n_head_icon_list]);
		head_data[db_info.n_head_icon_list].id = data->head_icon_list[i].id;
		head_data[db_info.n_head_icon_list].status = data->head_icon_list[i].status;
		db_info.n_head_icon_list++;
	}

	const static int MAX_CREATE_ATTR = PLAYER_ATTR_MAX - PLAYER_ATTR_CLOTHES + 1;
	int arrNum = 0;
	uint32_t arrId[MAX_CREATE_ATTR];
	uint32_t arrVaual[MAX_CREATE_ATTR];
	for (int i = PLAYER_ATTR_CLOTHES; i < PLAYER_ATTR_MAX; ++i, ++arrNum)
	{
		arrId[arrNum] = i;
		arrVaual[arrNum] = data->attrData[i];
	}
	arrId[arrNum] = PLAYER_ATTR_ZHENYING;
	arrVaual[arrNum] = data->attrData[PLAYER_ATTR_ZHENYING];
	++arrNum;

	db_info.n_attr_id = arrNum;
	db_info.n_attr = arrNum;
	db_info.attr_id = arrId;
	db_info.attr = arrVaual;

	//任务
	uint32_t task_num = 0;
	DBTask task_data[MAX_TASK_ACCEPTED_NUM];
	DBTask* task_data_point[MAX_TASK_ACCEPTED_NUM];
	DBTaskCount task_count_data[MAX_TASK_ACCEPTED_NUM][MAX_TASK_TARGET_NUM];
	DBTaskCount* task_count_data_point[MAX_TASK_ACCEPTED_NUM][MAX_TASK_TARGET_NUM];
	for (int i = 0; i < MAX_TASK_ACCEPTED_NUM; ++i)
	{
		TaskInfo& task = data->task_list[i];
		if (task.id == 0)
		{
			continue;
		}

		task_data_point[task_num] = &task_data[task_num];
		dbtask__init(&task_data[task_num]);
		task_data[task_num].id = task.id;
		task_data[task_num].status = task.status;
		task_data[task_num].accept_ts = task.accept_ts;
		task_data[task_num].accu_time = task.accu_time;
		uint32_t target_num = 0;
		for (int j = 0; j < MAX_TASK_TARGET_NUM; ++j)
		{
			TaskCountInfo& count_info = task.progress[j];
			if (count_info.id == 0)
			{
				continue;
			}

			task_count_data_point[task_num][target_num] = &task_count_data[task_num][target_num];
			dbtask_count__init(&task_count_data[task_num][target_num]);
			task_count_data[task_num][target_num].id = count_info.id;
			task_count_data[task_num][target_num].num = count_info.num;
			target_num++;
		}
		task_data[task_num].progress = task_count_data_point[task_num];
		task_data[task_num].n_progress = target_num;
		task_num++;
	}
	db_info.task_list = task_data_point;
	db_info.n_task_list = task_num;

	uint32_t task_finish_num = 1;
	for (int i = 1; i < MAX_TASK_NUM; ++i)
	{
		if (data->task_finish[i] > 0)
		{
			task_finish_num++;
		}
		else
		{
			break;
		}
	}
	db_info.task_finish = data->task_finish;
	db_info.n_task_finish = task_finish_num;
	db_info.task_chapter_reward = data->task_chapter_reward;

	ItemBuff item_buffs[MAX_BUFF_PER_UNIT];
	ItemBuff *item_buffs_point[MAX_BUFF_PER_UNIT];
	db_info.item_buffs = &item_buffs_point[0];
		//下线不删除的buff
	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (!m_buffs[i])
			continue;
		if (m_buffs[i]->config->DeleteType != 1)
			continue;

//		if (m_buffs[i]->config->BuffType != 2)
//			continue;

//		assert(m_buffs[i]->is_recoverable_buff());
		assert(m_buffs[i]->effect_config->Type == 170000008 || m_buffs[i]->effect_config->Type == 170000018 || m_buffs[i]->effect_config->Type == 170000029);

		item_buffs_point[db_info.n_item_buffs] = &item_buffs[db_info.n_item_buffs];
		item_buff__init(&item_buffs[db_info.n_item_buffs]);
		item_buffs[db_info.n_item_buffs].id = m_buffs[i]->config->ID;
		item_buffs[db_info.n_item_buffs].end_time = m_buffs[i]->data->end_time;
		item_buffs[db_info.n_item_buffs].buff_state = m_buffs[i]->data->effect.buff_state.state;
//		item_buffs[db_info.n_item_buffs].attr_id = m_buffs[i]->data->effect.attr_effect.attr_id;
//		item_buffs[db_info.n_item_buffs].added_attr_value = m_buffs[i]->data->effect.attr_effect.added_attr_value;
		db_info.n_item_buffs++;
	}

	//装备
	DBEquip equip_data[MAX_EQUIP_NUM];
	DBEquip* equip_data_point[MAX_EQUIP_NUM];
	DBEquipEnchant enchant_data[MAX_EQUIP_NUM][MAX_EQUIP_ENCHANT_NUM];
	DBEquipEnchant* enchant_data_point[MAX_EQUIP_NUM][MAX_EQUIP_ENCHANT_NUM];
	DBAttr cur_attr[MAX_EQUIP_NUM][MAX_EQUIP_ENCHANT_NUM];
	DBAttr* cur_attr_point[MAX_EQUIP_NUM][MAX_EQUIP_ENCHANT_NUM];
	DBAttr rand_attr[MAX_EQUIP_NUM][MAX_EQUIP_ENCHANT_NUM][MAX_EQUIP_ENCHANT_RAND_NUM];
	DBAttr* rand_attr_point[MAX_EQUIP_NUM][MAX_EQUIP_ENCHANT_NUM][MAX_EQUIP_ENCHANT_RAND_NUM];

	size_t equip_num = 0;
	for (int k = 0; k < MAX_EQUIP_NUM; ++k)
	{
		EquipInfo &equip_info = data->equip_list[k];

		equip_data_point[equip_num] = &equip_data[equip_num];
		dbequip__init(&equip_data[equip_num]);

		equip_data[equip_num].stair = equip_info.stair;
		equip_data[equip_num].star_lv = equip_info.star_lv;
		equip_data[equip_num].star_exp = equip_info.star_exp;
		size_t enchant_num = 0;
		for (int i = 0; i < MAX_EQUIP_ENCHANT_NUM; ++i)
		{
			EquipEnchantInfo &enchant_info = equip_info.enchant[i];

			enchant_data_point[equip_num][enchant_num] = &enchant_data[equip_num][enchant_num];
			dbequip_enchant__init(&enchant_data[equip_num][enchant_num]);

			cur_attr_point[equip_num][enchant_num] = &cur_attr[equip_num][enchant_num];
			dbattr__init(&cur_attr[equip_num][enchant_num]);
			cur_attr[equip_num][enchant_num].id = enchant_info.cur_attr.id;
			cur_attr[equip_num][enchant_num].val = enchant_info.cur_attr.val;
			enchant_data[equip_num][enchant_num].cur_attr = cur_attr_point[equip_num][enchant_num];

			size_t rand_num = 0;
			for (int j = 0; j < MAX_EQUIP_ENCHANT_RAND_NUM; ++j)
			{
				rand_attr_point[equip_num][enchant_num][rand_num] = &rand_attr[equip_num][enchant_num][rand_num];
				dbattr__init(&rand_attr[equip_num][enchant_num][rand_num]);
				rand_attr[equip_num][enchant_num][rand_num].id = enchant_info.rand_attr[j].id;
				rand_attr[equip_num][enchant_num][rand_num].val = enchant_info.rand_attr[j].val;
				rand_num++;
			}
			enchant_data[equip_num][enchant_num].rand_attr = rand_attr_point[equip_num][enchant_num];
			enchant_data[equip_num][enchant_num].n_rand_attr = rand_num;

			enchant_num++;
		}
		equip_data[equip_num].n_enchant = enchant_num;
		equip_data[equip_num].enchant = enchant_data_point[equip_num];

		equip_data[equip_num].inlay = &equip_info.inlay[0];
		equip_data[equip_num].n_inlay = MAX_EQUIP_INLAY_NUM;
		equip_num++;
	}
	db_info.equip_list = equip_data_point;
	db_info.n_equip_list = MAX_EQUIP_NUM;

	m_skill.PackAllSkill(db_info);
	//时装
	FashionDbData fashion[MAX_FASHION_NUM];
	FashionDbData *fashionPoint[MAX_FASHION_NUM];
	for (uint32_t i = 0; i < data->n_fashion; ++i)
	{
		fashion_db_data__init(&fashion[i]);
		fashion[i].id = data->fashion[i].id;
		fashion[i].color =data->fashion[i].color;
		fashion[i].isnew = data->fashion[i].isNew;
		fashion[i].cd = data->fashion[i].timeout;

		fashionPoint[i] = &fashion[i];
	}
	db_info.n_fashion = data->n_fashion;
	db_info.fashion = fashionPoint;
	//颜色
	db_info.n_color = db_info.n_color_isnew = data->n_color;
	db_info.color = data->color;
	db_info.color_isnew = data->color_is_new;
	db_info.charm_total = data->charm_total;
	db_info.charm_level = data->charm_level;
	db_info.n_weapon_color = db_info.n_weapon_color_isnew = data->n_weapon_color;
	db_info.weapon_color = data->weapon_color;
	db_info.weapon_color_isnew = data->weapon_color_is_new;

	//商城
	DBGoods goods_data[MAX_SHOP_GOODS_NUM];
	DBGoods* goods_data_point[MAX_SHOP_GOODS_NUM];
	uint32_t goods_num = 0;
	for (int i = 0; i < MAX_SHOP_GOODS_NUM; ++i)
	{
		if (data->shop_goods[i].goods_id == 0)
		{
			break;
		}

		goods_data_point[goods_num] = &goods_data[goods_num];
		dbgoods__init(&goods_data[goods_num]);
		goods_data[goods_num].goods_id = data->shop_goods[i].goods_id;
		goods_data[goods_num].bought_num = data->shop_goods[i].bought_num;
		goods_num++;
	}
	db_info.n_shop_goods = goods_num;
	db_info.shop_goods = goods_data_point;

	DBShopReset shop_reset_data;
	dbshop_reset__init(&shop_reset_data);
	db_info.shop_reset = &shop_reset_data;
	shop_reset_data.next_day_time = data->shop_reset.next_day_time;
	shop_reset_data.next_week_time = data->shop_reset.next_week_time;
	shop_reset_data.next_month_time = data->shop_reset.next_month_time;

	//horse
	DbHorseData horse[MAX_HORSE_NUM];
	DbHorseData *horsePoint[MAX_HORSE_NUM];
	for (uint32_t i = 0; i < data->n_horse; ++i)
	{
		db_horse_data__init(&horse[i]);
		horse[i].id = data->horse[i].id;
		horse[i].isnew = data->horse[i].isNew;
		horse[i].cd = data->horse[i].timeout;
		horsePoint[i] = &horse[i];
	}
	DbHorseCommonAttr sendAttr;
	db_horse_common_attr__init(&sendAttr);
	sendAttr.step = data->horse_attr.step;
	sendAttr.cur_soul = data->horse_attr.cur_soul;
	sendAttr.soul_level = data->horse_attr.soul;
	sendAttr.soul_num = &(data->horse_attr.soul_exp[1]);
	sendAttr.n_soul_num = MAX_HORSE_SOUL;
	sendAttr.attr = data->horse_attr.attr;
	sendAttr.attr_level = data->horse_attr.attr_exp;
	sendAttr.n_attr = sendAttr.n_attr_level = MAX_HORSE_ATTR_NUM;
	sendAttr.power = 0;
	sendAttr.fly = data->horse_attr.fly;
	sendAttr.soul_full = data->horse_attr.soul_full;

	db_info.n_horse_data = data->n_horse;
	db_info.horse_data = horsePoint;
	db_info.horse_attr = &sendAttr;

	db_info.pk_type = data->attrData[PLAYER_ATTR_PK_TYPE];
	db_info.murder_num = data->attrData[PLAYER_ATTR_MURDER];

	//御气道
	DBYuqidaoMai mai_data[MAX_YUQIDAO_MAI_NUM];
	DBYuqidaoMai* mai_data_point[MAX_YUQIDAO_MAI_NUM];
	db_info.n_yuqidao_mais = 0;
	db_info.yuqidao_mais = mai_data_point;
	for (int i = 0; i < MAX_YUQIDAO_MAI_NUM; ++i)
	{
		if (data->yuqidao_mais[i].mai_id == 0)
		{
			continue;
		}

		mai_data_point[db_info.n_yuqidao_mais] = &mai_data[db_info.n_yuqidao_mais];
		dbyuqidao_mai__init(&mai_data[db_info.n_yuqidao_mais]);
		mai_data[db_info.n_yuqidao_mais].mai_id = data->yuqidao_mais[i].mai_id;
		mai_data[db_info.n_yuqidao_mais].acupoint_id = data->yuqidao_mais[i].acupoint_id;
		mai_data[db_info.n_yuqidao_mais].fill_lv = data->yuqidao_mais[i].fill_lv;
		db_info.n_yuqidao_mais++;
	}

	DBYuqidaoBreak break_data[MAX_YUQIDAO_BREAK_NUM];
	DBYuqidaoBreak* break_data_point[MAX_YUQIDAO_BREAK_NUM];
	db_info.n_yuqidao_breaks = 0;
	db_info.yuqidao_breaks = break_data_point;
	for (int i = 0; i < MAX_YUQIDAO_BREAK_NUM; ++i)
	{
		if (data->yuqidao_breaks[i].id == 0)
		{
			continue;
		}

		break_data_point[db_info.n_yuqidao_breaks] = &break_data[db_info.n_yuqidao_breaks];
		dbyuqidao_break__init(&break_data[db_info.n_yuqidao_breaks]);
		break_data[db_info.n_yuqidao_breaks].id = data->yuqidao_breaks[i].id;
		break_data[db_info.n_yuqidao_breaks].cur_val = data->yuqidao_breaks[i].cur_val;
		break_data[db_info.n_yuqidao_breaks].n_cur_val = MAX_YUQIDAO_BREAK_ATTR_NUM;
		break_data[db_info.n_yuqidao_breaks].new_val = data->yuqidao_breaks[i].new_val;
		break_data[db_info.n_yuqidao_breaks].n_new_val = MAX_YUQIDAO_BREAK_ATTR_NUM;
		break_data[db_info.n_yuqidao_breaks].new_addn = data->yuqidao_breaks[i].new_addn;
		break_data[db_info.n_yuqidao_breaks].n_new_addn = MAX_YUQIDAO_BREAK_ATTR_NUM;
		break_data[db_info.n_yuqidao_breaks].count = data->yuqidao_breaks[i].count;
		db_info.n_yuqidao_breaks++;
	}

	//八卦牌
	DBBaguapaiDress bagua_dress[MAX_BAGUAPAI_STYLE_NUM];
	DBBaguapaiDress* bagua_dress_point[MAX_BAGUAPAI_STYLE_NUM];
	DBBaguapaiCard bagua_card[MAX_BAGUAPAI_STYLE_NUM][MAX_BAGUAPAI_DRESS_NUM];
	DBBaguapaiCard* bagua_card_point[MAX_BAGUAPAI_STYLE_NUM][MAX_BAGUAPAI_DRESS_NUM];
	DBAttr bagua_cur_attr[MAX_BAGUAPAI_STYLE_NUM][MAX_BAGUAPAI_DRESS_NUM][MAX_BAGUAPAI_MINOR_ATTR_NUM];
	DBAttr* bagua_cur_attr_point[MAX_BAGUAPAI_STYLE_NUM][MAX_BAGUAPAI_DRESS_NUM][MAX_BAGUAPAI_MINOR_ATTR_NUM];
	DBAttr bagua_new_attr[MAX_BAGUAPAI_STYLE_NUM][MAX_BAGUAPAI_DRESS_NUM][MAX_BAGUAPAI_MINOR_ATTR_NUM];
	DBAttr* bagua_new_attr_point[MAX_BAGUAPAI_STYLE_NUM][MAX_BAGUAPAI_DRESS_NUM][MAX_BAGUAPAI_MINOR_ATTR_NUM];
	for (int i = 0; i < MAX_BAGUAPAI_STYLE_NUM; ++i)
	{
		bagua_dress_point[i] = &bagua_dress[i];
		dbbaguapai_dress__init(&bagua_dress[i]);
		for (int j = 0; j < MAX_BAGUAPAI_DRESS_NUM; ++j)
		{
			bagua_card_point[i][j] = &bagua_card[i][j];
			dbbaguapai_card__init(&bagua_card[i][j]);
			bagua_card[i][j].id = data->baguapai_dress[i].card_list[j].id;
			bagua_card[i][j].star = data->baguapai_dress[i].card_list[j].star;
			bagua_card[i][j].main_attr_val = data->baguapai_dress[i].card_list[j].main_attr_val;
			bagua_card[i][j].main_attr_val_new = data->baguapai_dress[i].card_list[j].main_attr_val_new;
			for (int k = 0; k < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++k)
			{
				bagua_cur_attr_point[i][j][k] = &bagua_cur_attr[i][j][k];
				dbattr__init(&bagua_cur_attr[i][j][k]);
				bagua_cur_attr[i][j][k].id = data->baguapai_dress[i].card_list[j].minor_attrs[k].id;
				bagua_cur_attr[i][j][k].val = data->baguapai_dress[i].card_list[j].minor_attrs[k].val;
			}
			bagua_card[i][j].minor_attrs = bagua_cur_attr_point[i][j];
			bagua_card[i][j].n_minor_attrs = MAX_BAGUAPAI_MINOR_ATTR_NUM;
			for (int k = 0; k < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++k)
			{
				bagua_new_attr_point[i][j][k] = &bagua_new_attr[i][j][k];
				dbattr__init(&bagua_new_attr[i][j][k]);
				bagua_new_attr[i][j][k].id = data->baguapai_dress[i].card_list[j].minor_attrs_new[k].id;
				bagua_new_attr[i][j][k].val = data->baguapai_dress[i].card_list[j].minor_attrs_new[k].val;
			}
			bagua_card[i][j].minor_attrs_new = bagua_new_attr_point[i][j];
			bagua_card[i][j].n_minor_attrs_new = MAX_BAGUAPAI_MINOR_ATTR_NUM;
		}
		bagua_dress[i].cards = bagua_card_point[i];
		bagua_dress[i].n_cards = MAX_BAGUAPAI_DRESS_NUM;
	}
	db_info.baguapai_dress = bagua_dress_point;
	db_info.n_baguapai_dress = MAX_BAGUAPAI_STYLE_NUM;

	db_info.baguapai_style = data->attrData[PLAYER_ATTR_BAGUA];

	ChengJieTaskManage::pack_yaoshi_to_dbinfo(this, db_info);

	db_info.qiecuo_invite_switch = data->qiecuo_invite_switch;
	db_info.team_invite_switch = data->team_invite_switch;
	db_info.out_stuck_time = data->out_stuck_time;

		//pvp副本
	PvpRaidDb pvp3, pvp5;
	pvp_raid_db__init(&pvp3);
	pvp_raid_db__init(&pvp5);
	db_info.pvp_3 = &pvp3;
	db_info.pvp_5 = &pvp5;
	pvp3.oneday_win_num = data->pvp_raid_data.oneday_win_num_3;
	pvp3.cur_level_id = data->pvp_raid_data.cur_level_id_3;
	pvp3.max_level_id = data->pvp_raid_data.max_level_id_3;
	pvp3.max_score = data->pvp_raid_data.max_score_3;
	pvp3.score = data->pvp_raid_data.score_3;
	pvp3.avaliable_reward_level = data->pvp_raid_data.avaliable_reward_level_3;
	pvp3.n_avaliable_box = MAX_ONEDAY_PVP_BOX;
	pvp3.avaliable_box = data->pvp_raid_data.avaliable_box_3;

	pvp5.oneday_win_num = data->pvp_raid_data.oneday_win_num_5;
	pvp5.cur_level_id = data->pvp_raid_data.cur_level_id_5;
	pvp5.max_level_id = data->pvp_raid_data.max_level_id_5;
	pvp5.max_score = data->pvp_raid_data.max_score_5;
	pvp5.score = data->pvp_raid_data.score_5;
	pvp5.avaliable_reward_level = data->pvp_raid_data.avaliable_reward_level_5;
	pvp5.n_avaliable_box = MAX_ONEDAY_PVP_BOX;
	pvp5.avaliable_box = data->pvp_raid_data.avaliable_box_5;

		//自动补血
	DBAutoAddHp auto_add_hp;
	dbauto_add_hp__init(&auto_add_hp);
	auto_add_hp.auto_add_hp_item_id = data->auto_add_hp_item_id;
	auto_add_hp.auto_add_hp_percent = data->auto_add_hp_percent;
	auto_add_hp.open_auto_add_hp = data->open_auto_add_hp;
	auto_add_hp.hp_pool_num = data->hp_pool_num;
	db_info.auto_add_hp = &auto_add_hp;

	//活动大厅
	db_info.n_active_reward = 0;
	for (int i = 0; i < MAX_ACTIVE_REWARD_NUM; ++i)
	{
		if (data->active_reward[i] == 0)
		{
			break;
		}

		db_info.n_active_reward++;
	}
	db_info.active_reward = data->active_reward;

	DBDailyActivity daily_act[MAX_DAILY_ACTIVITY_NUM];
	DBDailyActivity* daily_act_point[MAX_DAILY_ACTIVITY_NUM];
	DBChivalryActivity chivalry_act[MAX_CHIVALRY_ACTIVITY_NUM];
	DBChivalryActivity* chivalry_act_point[MAX_CHIVALRY_ACTIVITY_NUM];
	db_info.n_daily_activity = 0;
	db_info.daily_activity = daily_act_point;
	for (int i = 0; i < MAX_DAILY_ACTIVITY_NUM; ++i)
	{
		if (data->daily_activity[i].act_id == 0)
		{
			break;
		}

		daily_act_point[db_info.n_daily_activity] = &daily_act[db_info.n_daily_activity];
		dbdaily_activity__init(&daily_act[db_info.n_daily_activity]);
		daily_act[db_info.n_daily_activity].id = data->daily_activity[i].act_id;
		daily_act[db_info.n_daily_activity].count = data->daily_activity[i].count;
		db_info.n_daily_activity++;
	}

	db_info.n_chivalry_activity = 0;
	db_info.chivalry_activity = chivalry_act_point;
	for (int i = 0; i < MAX_CHIVALRY_ACTIVITY_NUM; ++i)
	{
		if (data->chivalry_activity[i].act_id == 0)
		{
			break;
		}

		chivalry_act_point[db_info.n_chivalry_activity] = &chivalry_act[db_info.n_chivalry_activity];
		dbchivalry_activity__init(&chivalry_act[db_info.n_chivalry_activity]);
		chivalry_act[db_info.n_chivalry_activity].id = data->chivalry_activity[i].act_id;
		chivalry_act[db_info.n_chivalry_activity].val = data->chivalry_activity[i].val;
		db_info.n_chivalry_activity++;
	}

	DBPersonality personality;
	dbpersonality__init(&personality);
	personality.sex = data->personality_sex;
	personality.birthday = data->personality_birthday;
	personality.location = data->personality_location;
	personality.text_intro = data->personality_text_intro;
	personality.voice_intro = data->personality_voice_intro;
	personality.tags = data->personality_tags;
	personality.n_tags = 0;
	for (int i = 0; i < MAX_PERSONALITY_TAG_NUM; ++i)
	{
		if (data->personality_tags[i] == 0)
		{
			break;
		}
		personality.n_tags++;
	}
	db_info.personality = &personality;

	DBLiveSkill liveSkill;
	dblive_skill__init(&liveSkill);
	liveSkill.level = data->live_skill.level + 1;
	liveSkill.exp = data->live_skill.exp + 1;
	liveSkill.book = data->live_skill.book + 1;
	liveSkill.broken = data->live_skill.broken + 1;
	liveSkill.n_level = MAX_LIVE_SKILL_NUM;
	liveSkill.n_exp = MAX_LIVE_SKILL_NUM;
	liveSkill.n_book = MAX_LIVE_SKILL_NUM;
	liveSkill.n_broken = MAX_LIVE_SKILL_NUM;
	db_info.live_skill = &liveSkill;

	//保存离开副本后要到的场景的位置
	db_info.leaveraid_sceneid = data->leaveraid.scene_id;
	db_info.exitpointx = data->leaveraid.ExitPointX;
	db_info.exitpointy = data->leaveraid.ExitPointY;
	db_info.exitpointz = data->leaveraid.ExitPointZ;
	db_info.facey = data->leaveraid.direct;
	db_info.noviceraid = data->noviceraid_flag;
	db_info.receivegift = data->Receive_type;

	//伙伴
	DBPartner  partner_data[MAX_PARTNER_NUM];
	DBPartner* partner_point[MAX_PARTNER_NUM];
	DBPartnerAttr partner_cur_attr[MAX_PARTNER_NUM];
	DBPartnerAttr partner_cur_flash[MAX_PARTNER_NUM];
	DBPartnerSkill  partner_skill_data[MAX_PARTNER_NUM][MAX_PARTNER_SKILL_NUM];
	DBPartnerSkill* partner_skill_point[MAX_PARTNER_NUM][MAX_PARTNER_SKILL_NUM];
	DBPartnerSkill  partner_skill_data_flash[MAX_PARTNER_NUM][MAX_PARTNER_SKILL_NUM];
	DBPartnerSkill* partner_skill_point_flash[MAX_PARTNER_NUM][MAX_PARTNER_SKILL_NUM];
	DBAttr  partner_attr_data[MAX_PARTNER_NUM][PLAYER_ATTR_MAX];
	DBAttr* partner_attr_point[MAX_PARTNER_NUM][PLAYER_ATTR_MAX];
	uint32_t partner_num = 0;
	for (PartnerMap::iterator iter = m_partners.begin(); iter != m_partners.end() && partner_num < MAX_PARTNER_NUM; ++iter)
	{
		partner_struct *partner = iter->second;
		partner_point[partner_num] = &partner_data[partner_num];
		dbpartner__init(&partner_data[partner_num]);
		partner_data[partner_num].uuid = partner->data->uuid;
		partner_data[partner_num].partner_id = partner->data->partner_id;
		partner_data[partner_num].bind = partner->data->bind;
		partner_data[partner_num].relive_time = partner->data->relive_time;
		partner_data[partner_num].strong_num = partner->data->strong_num;

		uint32_t attr_num = 0;
		for (int i = 1; i < MAX_PARTNER_ATTR; ++i)
		{
			if (!(i == PLAYER_ATTR_HP || i == PLAYER_ATTR_LEVEL || i == PLAYER_ATTR_EXP))
			{
				continue;
			}

			partner_attr_point[partner_num][attr_num] = &partner_attr_data[partner_num][attr_num];
			dbattr__init(&partner_attr_data[partner_num][attr_num]);
			partner_attr_data[partner_num][attr_num].id = i;
			partner_attr_data[partner_num][attr_num].val = partner->data->attrData[i];
			attr_num++;
		}
		partner_data[partner_num].attrs = partner_attr_point[partner_num];
		partner_data[partner_num].n_attrs = attr_num;
		partner_data[partner_num].n_god_id = partner_data[partner_num].n_god_lv = partner->data->n_god;
		partner_data[partner_num].god_id = partner->data->god_id;
		partner_data[partner_num].god_lv = partner->data->god_level;

		partner_data[partner_num].attr_cur = partner_cur_attr + partner_num;
		dbpartner_attr__init(partner_cur_attr + partner_num);
		uint32_t skill_num = 0;
		for (int i = 0; i < MAX_PARTNER_SKILL_NUM; ++i)
		{
			partner_skill_point[partner_num][skill_num] = &partner_skill_data[partner_num][skill_num];
			dbpartner_skill__init(&partner_skill_data[partner_num][skill_num]);
			partner_skill_data[partner_num][skill_num].skill_id = partner->data->attr_cur.skill_list[i].skill_id;
			partner_skill_data[partner_num][skill_num].lv = partner->data->attr_cur.skill_list[i].lv;
			skill_num++;
		}
		partner_cur_attr[partner_num].skills = partner_skill_point[partner_num];
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

		if (partner->data->attr_flash.base_attr_id[0] != 0)
		{
			partner_data[partner_num].attr_flash = partner_cur_flash + partner_num;
			dbpartner_attr__init(partner_cur_flash + partner_num);
			skill_num = 0;
			for (int i = 0; i < MAX_PARTNER_SKILL_NUM; ++i)
			{
				partner_skill_point_flash[partner_num][skill_num] = &partner_skill_data_flash[partner_num][skill_num];
				dbpartner_skill__init(&partner_skill_data_flash[partner_num][skill_num]);
				partner_skill_data_flash[partner_num][skill_num].skill_id = partner->data->attr_flash.skill_list[i].skill_id;
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
			partner_cur_flash[partner_num].power_refresh = partner->data->attr_flash.power_refresh;
		}


		partner_num++;
	}
	db_info.partner_list = partner_point;
	db_info.n_partner_list = partner_num;
	db_info.partner_dictionary = data->partner_dictionary;
	db_info.n_partner_dictionary = 0;
	for (int i = 0; i < MAX_PARTNER_TYPE; ++i)
	{
		if (data->partner_dictionary[i] == 0)
		{
			break;
		}
		db_info.n_partner_dictionary++;
	}
	db_info.partner_formation = data->partner_formation;
	db_info.n_partner_formation = MAX_PARTNER_FORMATION_NUM;
	db_info.partner_battle = data->partner_battle;
	db_info.n_partner_battle = MAX_PARTNER_BATTLE_NUM;
	db_info.partner_recruit_junior_time = data->partner_recruit_junior_time;
	db_info.partner_recruit_junior_count = data->partner_recruit_junior_count;
	db_info.partner_recruit_senior_time = data->partner_recruit_senior_time;
	db_info.partner_recruit_senior_count = data->partner_recruit_senior_count;

	db_info.partner_bond = data->partner_bond;
	db_info.n_partner_bond = 0;
	for (int i = 0; i < MAX_PARTNER_BOND_NUM; ++i)
	{
		if (data->partner_bond[i] == 0)
		{
			break;
		}
		db_info.n_partner_bond++;
	}
	db_info.partner_bond_reward = data->partner_bond_reward;
	db_info.n_partner_bond_reward = 0;
	for (int i = 0; i < MAX_PARTNER_TYPE; ++i)
	{
		if (data->partner_bond_reward[i] == 0)
		{
			break;
		}
		db_info.n_partner_bond_reward++;
	}

	DBTruck truckData;
	dbtruck__init(&truckData);
	truckData.truck_id = data->truck.truck_id;
	truckData.active_id = data->truck.active_id;
	truckData.num_coin = data->truck.num_coin;
	truckData.num_gold = data->truck.num_gold;
	cash_truck_struct *pTruck = cash_truck_manager::get_cash_truck_by_id(data->truck.truck_id);
	if (pTruck != NULL)
	{
		truckData.pos_x = pTruck->get_pos()->pos_x;
		truckData.pos_z = pTruck->get_pos()->pos_z;
		truckData.scene_id = pTruck->data->scene_id;
		truckData.hp = pTruck->get_attr(PLAYER_ATTR_HP);
	}
	db_info.truck = &truckData;

	DBZhenying zhenying;
	dbzhenying__init(&zhenying);
	db_info.zhenying = &zhenying;
	zhenying.level = data->zhenying.level;
	zhenying.exp = data->zhenying.exp;
	zhenying.step = data->zhenying.step;
	zhenying.task = data->zhenying.task;
	zhenying.task_type = data->zhenying.task_type;
	zhenying.task_num = data->zhenying.task_num;
	zhenying.exp_day = data->zhenying.exp_day;
	zhenying.free_change = data->zhenying.free;
	zhenying.kill_week = data->zhenying.kill_week;
	zhenying.score_week = data->zhenying.score_week;
	zhenying.last_week = data->zhenying.last_week;
	zhenying.week = data->zhenying.week;
	zhenying.kill = data->zhenying.kill;
	zhenying.death = data->zhenying.death;
	zhenying.help = data->zhenying.help;
	zhenying.score = data->zhenying.score;
	zhenying.mine = data->zhenying.mine;


	return player_dbinfo__pack(&db_info, out_data);
}

int player_struct::unpack_dbinfo_to_playerinfo(uint8_t *packed_data, int len)
{
	PlayerDBInfo *db_info = NULL;
	db_info = player_dbinfo__unpack(NULL, len, packed_data);
	if (!db_info)
	{
		LOG_ERR("[%s:%d] unpack dbinfo fail, data_len:%d", __FUNCTION__, __LINE__, len);
		return -1;
	}

	data->attrData[PLAYER_ATTR_EXP] = db_info->exp;
	data->scene_id = db_info->scene_id;
	data->last_scene_id = db_info->last_scene_id;
	data->move_path.pos[0].pos_x = db_info->pos_x;
	data->move_path.pos[0].pos_z = db_info->pos_z;
	data->pos_y = db_info->pos_y;
	data->next_time_refresh_oneday_job = db_info->next_time_refresh_oneday_job * 1000ULL;
	data->attrData[PLAYER_ATTR_HP] = (int)(db_info->cur_hp);
//	data->raid_uuid = db_info->raid_uuid;

	data->attrData[PLAYER_ATTR_PK_TYPE] = db_info->pk_type;
	data->attrData[PLAYER_ATTR_MURDER] = db_info->murder_num;

	data->m_angle = db_info->angle;

	for (uint32_t i = 0; i < db_info->n_raid_reward_id; ++i)
	{
		data->raid_reward_id[i] = db_info->raid_reward_id[i];
		data->raid_reward_num[i] = db_info->raid_reward_num[i];
	}

	if (db_info->zhenying != NULL)
	{
		 data->zhenying.level = db_info->zhenying->level;
		 data->zhenying.exp = db_info->zhenying->exp;
		 data->zhenying.step = db_info->zhenying->step;
		 data->zhenying.task = db_info->zhenying->task;
		 data->zhenying.task_type = db_info->zhenying->task_type;
		 data->zhenying.task_num = db_info->zhenying->task_num;
		 data->zhenying.exp_day = db_info->zhenying->exp_day;
		 data->zhenying.free = db_info->zhenying->free_change;
		 data->zhenying.kill_week = db_info->zhenying->kill_week;
		 data->zhenying.score_week = db_info->zhenying->score_week;
		 data->zhenying.last_week = db_info->zhenying->last_week;
		 data->zhenying.week = db_info->zhenying->week;
		 data->zhenying.kill = db_info->zhenying->kill;
		 data->zhenying.death = db_info->zhenying->death;
		 data->zhenying.help = db_info->zhenying->help;
		 data->zhenying.score = db_info->zhenying->score;
		 data->zhenying.mine = db_info->zhenying->mine;
	}

	//背包
	for (size_t i = 0; i < db_info->n_bag && i < MAX_BAG_GRID_NUM; ++i)
	{
		data->bag[i].id = db_info->bag[i]->id;
		data->bag[i].num = db_info->bag[i]->num;
		data->bag[i].used_count = db_info->bag[i]->used_count;
		data->bag[i].expire_time = db_info->bag[i]->expire_time;
		if (db_info->bag[i]->bagua)
		{
			data->bag[i].baguapai.star = db_info->bag[i]->bagua->star;
			data->bag[i].baguapai.main_attr_val = db_info->bag[i]->bagua->main_attr_val;
			for (size_t j = 0; j < db_info->bag[i]->bagua->n_minor_attrs && j < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++j)
			{
				data->bag[i].baguapai.minor_attrs[j].id = db_info->bag[i]->bagua->minor_attrs[j]->id;
				data->bag[i].baguapai.minor_attrs[j].val = db_info->bag[i]->bagua->minor_attrs[j]->val;
			}
		}
	}
	data->bag_grid_num = db_info->bag_grid_num;
	data->bag_unlock_num = db_info->bag_unlock_num;

	//货币
	data->attrData[PLAYER_ATTR_GOLD] = db_info->gold;
	data->attrData[PLAYER_ATTR_BIND_GOLD] = db_info->bind_gold;
	data->attrData[PLAYER_ATTR_COIN] = db_info->coin;
	data->teamid = db_info->teamid;

	//头像
	data->attrData[PLAYER_ATTR_HEAD] = db_info->head_icon;
	for (size_t i = 0; i < db_info->n_head_icon_list && i < MAX_HEAD_ICON_NUM; ++i)
	{
		data->head_icon_list[i].id = db_info->head_icon_list[i]->id;
		data->head_icon_list[i].status = db_info->head_icon_list[i]->status;
	}

	for (size_t i = 0; i < db_info->n_attr_id; ++i)
	{
		data->attrData[db_info->attr_id[i]] = db_info->attr[i];
	}

	//任务
	for (size_t i = 0; i < db_info->n_task_list && i < MAX_TASK_ACCEPTED_NUM; ++i)
	{
		data->task_list[i].id = db_info->task_list[i]->id;
		data->task_list[i].status = db_info->task_list[i]->status;
		data->task_list[i].accept_ts = db_info->task_list[i]->accept_ts;
		data->task_list[i].accu_time = db_info->task_list[i]->accu_time;
		for (size_t j = 0; j < db_info->task_list[i]->n_progress && j < MAX_TASK_TARGET_NUM; ++j)
		{
			data->task_list[i].progress[j].id = db_info->task_list[i]->progress[j]->id;
			data->task_list[i].progress[j].num = db_info->task_list[i]->progress[j]->num;
		}
	}

	for (size_t i = 0; i < db_info->n_task_finish && i < MAX_TASK_NUM; ++i)
	{
		data->task_finish[i] = db_info->task_finish[i];
	}
	data->task_chapter_reward = db_info->task_chapter_reward;

	for (size_t i = 0; i < db_info->n_item_buffs && i < MAX_BUFF_PER_UNIT; ++i)
	{
		buff_manager::load_item_buff(this, db_info->item_buffs[i]);
	}

	//装备
	for (size_t i = 0; i < db_info->n_equip_list && i < MAX_EQUIP_NUM; ++i)
	{
		data->equip_list[i].stair = db_info->equip_list[i]->stair;
		data->equip_list[i].star_lv = db_info->equip_list[i]->star_lv;
		data->equip_list[i].star_exp = db_info->equip_list[i]->star_exp;
		for (size_t j = 0; j < db_info->equip_list[i]->n_enchant && j < MAX_EQUIP_ENCHANT_NUM; ++j)
		{
			data->equip_list[i].enchant[j].cur_attr.id = db_info->equip_list[i]->enchant[j]->cur_attr->id;
			data->equip_list[i].enchant[j].cur_attr.val = db_info->equip_list[i]->enchant[j]->cur_attr->val;
			for (size_t k = 0; k < db_info->equip_list[i]->enchant[j]->n_rand_attr && k < MAX_EQUIP_ENCHANT_RAND_NUM; ++k)
			{
				data->equip_list[i].enchant[j].rand_attr[k].id = db_info->equip_list[i]->enchant[j]->rand_attr[k]->id;
				data->equip_list[i].enchant[j].rand_attr[k].val = db_info->equip_list[i]->enchant[j]->rand_attr[k]->val;
			}
		}

		for (size_t j = 0; j < db_info->equip_list[i]->n_inlay && j < MAX_EQUIP_INLAY_NUM; ++j)
		{
			data->equip_list[i].inlay[j] = db_info->equip_list[i]->inlay[j];
		}
	}

	m_skill.UnPackAllSkill(*db_info);

	//时装
	for (uint32_t i = 0; i < db_info->n_fashion; ++i)
	{
		data->fashion[i].id = db_info->fashion[i]->id;
		data->fashion[i].color = db_info->fashion[i]->color;
		data->fashion[i].isNew = db_info->fashion[i]->isnew;
		data->fashion[i].timeout = db_info->fashion[i]->cd;
	}
	data->n_fashion = db_info->n_fashion;
	data->charm_level = db_info->charm_level;
	data->charm_total = db_info->charm_total;
	if (db_info->n_color > 0)
	{
		data->n_color = db_info->n_color;
		memcpy(data->color, db_info->color, data->n_color * sizeof(uint32_t));
		memcpy(data->color_is_new, db_info->color_isnew, data->n_color * sizeof(uint32_t));
	}
	if (db_info->n_weapon_color > 0)
	{
		data->n_weapon_color = db_info->n_weapon_color;
		memcpy(data->weapon_color, db_info->weapon_color, data->n_weapon_color * sizeof(uint32_t));
		memcpy(data->weapon_color_is_new, db_info->weapon_color_isnew, data->n_color * sizeof(uint32_t));
	}


	//商城
	for (size_t i = 0; i < db_info->n_shop_goods && i < MAX_SHOP_GOODS_NUM; ++i)
	{
		data->shop_goods[i].goods_id = db_info->shop_goods[i]->goods_id;
		data->shop_goods[i].bought_num = db_info->shop_goods[i]->bought_num;
	}

	if (db_info->shop_reset)
	{
		data->shop_reset.next_day_time = db_info->shop_reset->next_day_time;
		data->shop_reset.next_week_time = db_info->shop_reset->next_week_time;
		data->shop_reset.next_month_time = db_info->shop_reset->next_month_time;
	}

	unpack_horse(db_info);

	//御气道
	for (size_t i = 0; i < db_info->n_yuqidao_mais && i < MAX_YUQIDAO_MAI_NUM; ++i)
	{
		data->yuqidao_mais[i].mai_id = db_info->yuqidao_mais[i]->mai_id;
		data->yuqidao_mais[i].acupoint_id = db_info->yuqidao_mais[i]->acupoint_id;
		data->yuqidao_mais[i].fill_lv = db_info->yuqidao_mais[i]->fill_lv;
	}

	for (size_t i = 0; i < db_info->n_yuqidao_breaks && i < MAX_YUQIDAO_BREAK_NUM; ++i)
	{
		data->yuqidao_breaks[i].id = db_info->yuqidao_breaks[i]->id;
		data->yuqidao_breaks[i].count = db_info->yuqidao_breaks[i]->count;

		for (size_t j = 0; j < db_info->yuqidao_breaks[i]->n_cur_val && j < MAX_YUQIDAO_BREAK_ATTR_NUM; ++j)
		{
			data->yuqidao_breaks[i].cur_val[j] = db_info->yuqidao_breaks[i]->cur_val[j];
		}
		for (size_t j = 0; j < db_info->yuqidao_breaks[i]->n_new_val && j < MAX_YUQIDAO_BREAK_ATTR_NUM; ++j)
		{
			data->yuqidao_breaks[i].new_val[j] = db_info->yuqidao_breaks[i]->new_val[j];
		}
		for (size_t j = 0; j < db_info->yuqidao_breaks[i]->n_new_addn && j < MAX_YUQIDAO_BREAK_ATTR_NUM; ++j)
		{
			data->yuqidao_breaks[i].new_addn[j] = db_info->yuqidao_breaks[i]->new_addn[j];
		}
	}

	//八卦牌
	for (size_t i = 0; i < db_info->n_baguapai_dress && i < MAX_BAGUAPAI_STYLE_NUM; ++i)
	{
		for (size_t j = 0; j < db_info->baguapai_dress[i]->n_cards && j < MAX_BAGUAPAI_DRESS_NUM; ++j)
		{
			data->baguapai_dress[i].card_list[j].id = db_info->baguapai_dress[i]->cards[j]->id;
			data->baguapai_dress[i].card_list[j].star = db_info->baguapai_dress[i]->cards[j]->star;
			data->baguapai_dress[i].card_list[j].main_attr_val = db_info->baguapai_dress[i]->cards[j]->main_attr_val;
			data->baguapai_dress[i].card_list[j].main_attr_val_new = db_info->baguapai_dress[i]->cards[j]->main_attr_val_new;
			for (size_t k = 0; k < db_info->baguapai_dress[i]->cards[j]->n_minor_attrs && k < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++k)
			{
				data->baguapai_dress[i].card_list[j].minor_attrs[k].id = db_info->baguapai_dress[i]->cards[j]->minor_attrs[k]->id;
				data->baguapai_dress[i].card_list[j].minor_attrs[k].val = db_info->baguapai_dress[i]->cards[j]->minor_attrs[k]->val;
			}
			for (size_t k = 0; k < db_info->baguapai_dress[i]->cards[j]->n_minor_attrs_new && k < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++k)
			{
				data->baguapai_dress[i].card_list[j].minor_attrs_new[k].id = db_info->baguapai_dress[i]->cards[j]->minor_attrs_new[k]->id;
				data->baguapai_dress[i].card_list[j].minor_attrs_new[k].val = db_info->baguapai_dress[i]->cards[j]->minor_attrs_new[k]->val;
			}
		}
	}

	data->attrData[PLAYER_ATTR_BAGUA] = db_info->baguapai_style;

	ChengJieTaskManage::unpack_yaoshi(this, db_info);

	data->qiecuo_invite_switch = db_info->qiecuo_invite_switch;
	data->team_invite_switch = db_info->team_invite_switch;
	data->out_stuck_time = db_info->out_stuck_time;

			//自动补血
	if (db_info->auto_add_hp)
	{
		data->auto_add_hp_item_id = db_info->auto_add_hp->auto_add_hp_item_id;
		data->auto_add_hp_percent = db_info->auto_add_hp->auto_add_hp_percent;
		data->open_auto_add_hp = db_info->auto_add_hp->open_auto_add_hp;
		data->hp_pool_num = db_info->auto_add_hp->hp_pool_num;
	}

		//pvp副本
	if (db_info->pvp_3)
	{
	data->pvp_raid_data.oneday_win_num_3 = db_info->pvp_3->oneday_win_num;
	data->pvp_raid_data.cur_level_id_3 = db_info->pvp_3->cur_level_id;
	data->pvp_raid_data.max_level_id_3 = db_info->pvp_3->max_level_id;
	data->pvp_raid_data.max_score_3 = db_info->pvp_3->max_score;
	data->pvp_raid_data.score_3 = db_info->pvp_3->score;
	data->pvp_raid_data.avaliable_reward_level_3 = db_info->pvp_3->avaliable_reward_level;
	assert(db_info->pvp_3->n_avaliable_box == MAX_ONEDAY_PVP_BOX);
	for (int i = 0; i < MAX_ONEDAY_PVP_BOX; ++i)
		data->pvp_raid_data.avaliable_box_3[i] = db_info->pvp_3->avaliable_box[i];
	struct StageTable* config = get_config_by_id(data->pvp_raid_data.cur_level_id_3, &pvp_raid_config);
	assert(config);
	data->pvp_raid_data.level_3 = config->Stage;
	data->pvp_raid_data.star_3 = config->StageLevel;
	}
	if (db_info->pvp_5)
	{
	data->pvp_raid_data.oneday_win_num_5 = db_info->pvp_5->oneday_win_num;
	data->pvp_raid_data.cur_level_id_5 = db_info->pvp_5->cur_level_id;
	data->pvp_raid_data.max_level_id_5 = db_info->pvp_5->max_level_id;
	data->pvp_raid_data.max_score_5 = db_info->pvp_5->max_score;
	data->pvp_raid_data.score_5 = db_info->pvp_5->score;
	data->pvp_raid_data.avaliable_reward_level_5 = db_info->pvp_5->avaliable_reward_level;
	assert(db_info->pvp_5->n_avaliable_box == MAX_ONEDAY_PVP_BOX);
	for (int i = 0; i < MAX_ONEDAY_PVP_BOX; ++i)
		data->pvp_raid_data.avaliable_box_5[i] = db_info->pvp_5->avaliable_box[i];
	struct StageTable* config = get_config_by_id(data->pvp_raid_data.cur_level_id_5, &pvp_raid_config);
	assert(config);
	data->pvp_raid_data.level_5 = config->Stage;
	data->pvp_raid_data.star_5 = config->StageLevel;
	}

		//初始化
	if (data->pvp_raid_data.cur_level_id_3 == 0 || data->pvp_raid_data.level_3 == 0)
	{
		std::map<uint64_t, struct StageTable*>::iterator first_ite = pvp_raid_config.begin();
		std::map<uint64_t, struct StageTable*>::iterator second_ite = first_ite;
		++second_ite;

		data->pvp_raid_data.cur_level_id_3 = first_ite->first;
		data->pvp_raid_data.cur_level_id_5 = data->pvp_raid_data.cur_level_id_3;
		data->pvp_raid_data.max_level_id_3 = data->pvp_raid_data.cur_level_id_3;
		data->pvp_raid_data.max_level_id_5 = data->pvp_raid_data.cur_level_id_3;

		data->pvp_raid_data.level_3 = first_ite->second->Stage;
		data->pvp_raid_data.star_3 = first_ite->second->StageLevel;
		data->pvp_raid_data.level_5 = data->pvp_raid_data.level_3;
		data->pvp_raid_data.star_5 = data->pvp_raid_data.star_3;

		data->pvp_raid_data.avaliable_reward_level_3 = second_ite->first;//  data->pvp_raid_data.cur_level_id_3;
		data->pvp_raid_data.avaliable_reward_level_5 = data->pvp_raid_data.avaliable_reward_level_3;
		for (int i = 0; i < MAX_ONEDAY_PVP_BOX; ++i)
		{
			data->pvp_raid_data.avaliable_box_3[i] = i;
			data->pvp_raid_data.avaliable_box_5[i] = i;
		}
	}

	//活动大厅
	for (size_t i = 0; i < db_info->n_active_reward && i < MAX_ACTIVE_REWARD_NUM; ++i)
	{
		data->active_reward[i] = db_info->active_reward[i];
	}
	for (size_t i = 0; i < db_info->n_daily_activity && i < MAX_DAILY_ACTIVITY_NUM; ++i)
	{
		data->daily_activity[i].act_id = db_info->daily_activity[i]->id;
		data->daily_activity[i].count = db_info->daily_activity[i]->count;
	}
	for (size_t i = 0; i < db_info->n_chivalry_activity && i < MAX_CHIVALRY_ACTIVITY_NUM; ++i)
	{
		data->chivalry_activity[i].act_id = db_info->chivalry_activity[i]->id;
		data->chivalry_activity[i].val = db_info->chivalry_activity[i]->val;
	}

	unpack_answer_db(db_info);

	if (db_info->personality)
	{
		data->personality_sex = db_info->personality->sex;
		data->personality_birthday = db_info->personality->birthday;
		strncpy(data->personality_location, db_info->personality->location, MAX_PERSONALITY_LOCATION_LEN);
		strncpy(data->personality_text_intro, db_info->personality->text_intro, MAX_PERSONALITY_TEXT_INTRO_LEN);
		strncpy(data->personality_voice_intro, db_info->personality->voice_intro, MAX_PERSONALITY_VOICE_INTRO_LEN);
		for (size_t i = 0; i < db_info->personality->n_tags && i < MAX_PERSONALITY_TAG_NUM; ++i)
		{
			data->personality_tags[i] = db_info->personality->tags[i];
		}
	}

	if (db_info->live_skill)
	{
		for (uint32_t i = 0; i < db_info->live_skill->n_level; ++i)
		{
			data->live_skill.level[i + 1] = db_info->live_skill->level[i];
			data->live_skill.exp[i + 1] = db_info->live_skill->exp[i];
			data->live_skill.book[i + 1] = db_info->live_skill->book[i];
			data->live_skill.broken[i + 1] = db_info->live_skill->broken[i];
		}
	}
	//离开副本后要到的场景的位置
	data->leaveraid.scene_id = db_info->leaveraid_sceneid;
	data->leaveraid.ExitPointX = db_info->exitpointx;
	data->leaveraid.ExitPointY = db_info->exitpointy;
	data->leaveraid.ExitPointZ = db_info->exitpointz;
	data->leaveraid.direct = db_info->facey;
	data->noviceraid_flag = db_info->noviceraid;
	data->Receive_type = db_info->receivegift;

	//伙伴
	for (size_t i = 0; i < db_info->n_partner_list && i < MAX_PARTNER_NUM; ++i)
	{
		partner_struct *partner = partner_manager::create_partner(db_info->partner_list[i]->partner_id, this, db_info->partner_list[i]->uuid);
		if (partner == NULL)
		{
			LOG_ERR("[%s:%d] player[%lu] create partner failed, partner_id:%u, uuid:%lu", __FUNCTION__, __LINE__, data->player_id, db_info->partner_list[i]->partner_id, db_info->partner_list[i]->uuid);
			return -1;
		}

		partner->data->bind = db_info->partner_list[i]->bind;
		partner->data->relive_time = db_info->partner_list[i]->relive_time;
		partner->data->strong_num = db_info->partner_list[i]->strong_num;
		for (size_t j = 0; j < db_info->partner_list[i]->n_attrs; ++j)
		{
			partner->data->attrData[db_info->partner_list[i]->attrs[j]->id] = db_info->partner_list[i]->attrs[j]->val;
		}
		size_t tmp = MAX_PARTNER_DETAIL_ATTR;
		if (db_info->partner_list[i]->attr_flash != NULL)
		{
			for (uint32_t nAtt = 0; nAtt < db_info->partner_list[i]->attr_flash->n_skills; ++nAtt)
			{
				partner->data->attr_flash.skill_list[nAtt].skill_id = db_info->partner_list[i]->attr_flash->skills[nAtt]->skill_id;
				partner->data->attr_flash.skill_list[nAtt].lv = db_info->partner_list[i]->attr_flash->skills[nAtt]->lv;
			}
			memcpy(partner->data->attr_flash.base_attr_id, db_info->partner_list[i]->attr_flash->base_attr_id, sizeof(partner->data->attr_flash.base_attr_id));
			memcpy(partner->data->attr_flash.base_attr_vaual, db_info->partner_list[i]->attr_flash->base_attr_cur, sizeof(partner->data->attr_flash.base_attr_vaual));
			memcpy(partner->data->attr_flash.base_attr_up, db_info->partner_list[i]->attr_flash->base_attr_up, sizeof(partner->data->attr_flash.base_attr_up));
			memcpy(partner->data->attr_flash.detail_attr_id, db_info->partner_list[i]->attr_flash->detail_attr_id, sizeof(uint32_t) * std::min(tmp, db_info->partner_list[i]->attr_flash->n_detail_attr_id));
			memcpy(partner->data->attr_flash.detail_attr_vaual, db_info->partner_list[i]->attr_flash->detail_attr_cur, sizeof(uint32_t) * std::min(tmp, db_info->partner_list[i]->attr_flash->n_detail_attr_cur));
			partner->data->attr_flash.n_detail_attr = db_info->partner_list[i]->attr_flash->n_detail_attr_id;
			partner->data->attr_flash.type = db_info->partner_list[i]->attr_flash->type;
			partner->data->attr_flash.power_refresh = db_info->partner_list[i]->attr_flash->power_refresh;
		}
		if (db_info->partner_list[i]->attr_cur!= NULL)
		{
			for (uint32_t nAtt = 0; nAtt < db_info->partner_list[i]->attr_cur->n_skills; ++nAtt)
			{
				partner->data->attr_cur.skill_list[nAtt].skill_id = db_info->partner_list[i]->attr_cur->skills[nAtt]->skill_id;
				partner->data->attr_cur.skill_list[nAtt].lv = db_info->partner_list[i]->attr_cur->skills[nAtt]->lv;
			}
			memcpy(partner->data->attr_cur.base_attr_id, db_info->partner_list[i]->attr_cur->base_attr_id, sizeof(partner->data->attr_cur.base_attr_id));
			memcpy(partner->data->attr_cur.base_attr_vaual, db_info->partner_list[i]->attr_cur->base_attr_cur, sizeof(partner->data->attr_cur.base_attr_vaual));
			memcpy(partner->data->attr_cur.base_attr_up, db_info->partner_list[i]->attr_cur->base_attr_up, sizeof(partner->data->attr_cur.base_attr_up));
			memcpy(partner->data->attr_cur.detail_attr_id, db_info->partner_list[i]->attr_cur->detail_attr_id, sizeof(uint32_t) * std::min(tmp, db_info->partner_list[i]->attr_cur->n_detail_attr_id));
			memcpy(partner->data->attr_cur.detail_attr_vaual, db_info->partner_list[i]->attr_cur->detail_attr_cur, sizeof(uint32_t) * std::min(tmp, db_info->partner_list[i]->attr_cur->n_detail_attr_cur));
			partner->data->attr_cur.n_detail_attr = db_info->partner_list[i]->attr_cur->n_detail_attr_id;
			partner->data->attr_cur.type = db_info->partner_list[i]->attr_cur->type;
		}
		for (uint32_t n_g = 0; n_g < db_info->partner_list[i]->n_god_id; ++n_g)
		{
			partner->data->god_id[n_g] = db_info->partner_list[i]->god_id[n_g];
			partner->data->god_level[n_g] = db_info->partner_list[i]->god_lv[n_g];
		}
		partner->data->n_god = db_info->partner_list[i]->n_god_id;

		m_partners.insert(std::make_pair(partner->data->uuid, partner));
	}
	for (size_t i = 0; i < db_info->n_partner_dictionary && i < MAX_PARTNER_TYPE; ++i)
	{
		data->partner_dictionary[i] = db_info->partner_dictionary[i];
	}
	for (size_t i = 0; i < db_info->n_partner_formation && i < MAX_PARTNER_FORMATION_NUM; ++i)
	{
		data->partner_formation[i] = db_info->partner_formation[i];
	}
	for (size_t i = 0; i < db_info->n_partner_battle && i < MAX_PARTNER_BATTLE_NUM; ++i)
	{
		data->partner_battle[i] = db_info->partner_battle[i];
	}
	data->partner_recruit_junior_time = db_info->partner_recruit_junior_time;
	data->partner_recruit_junior_count = db_info->partner_recruit_junior_count;
	data->partner_recruit_senior_time = db_info->partner_recruit_senior_time;
	data->partner_recruit_senior_count = db_info->partner_recruit_senior_count;
	for (size_t i = 0; i < db_info->n_partner_bond && i < MAX_PARTNER_BOND_NUM; ++i)
	{
		data->partner_bond[i] = db_info->partner_bond[i];
	}
	for (size_t i = 0; i < db_info->n_partner_bond_reward && i < MAX_PARTNER_TYPE; ++i)
	{
		data->partner_bond_reward[i] = db_info->partner_bond_reward[i];
	}

	if (db_info->truck != NULL)
	{
		data->truck.truck_id = db_info->truck->truck_id;
		data->truck.active_id = db_info->truck->active_id;
		data->truck.num_coin = db_info->truck->num_coin;
		data->truck.num_gold = db_info->truck->num_gold;

		data->truck.pos.pos_x = db_info->truck->pos_x;
		data->truck.pos.pos_z = db_info->truck->pos_z;
		data->truck.scene_id = db_info->truck->scene_id;
		data->truck.hp = db_info->truck->hp;

	}

	player_dbinfo__free_unpacked(db_info, NULL);

	return 0;
}

/*
static void pack_sight_player_base_info(player_struct *player, SightPlayerBaseInfo *info)
{
	sight_player_base_info__init(info);
	info->name = player->data->name;
	info->playerid = player->data->player_id;
}

static void pack_sight_monster_info(monster_struct *monster, SightMonsterInfo *info)
{
	sight_monster_info__init(info);
	info->hp = 100;
}
*/

void player_struct::add_area_partner_to_sight(area_struct *area, int *add_partner_id_index, SightPartnerInfo *add_partner)
{
	if (!area)
		return;
	for (int j = 0; j < area->cur_partner_num; ++j)
	{
		partner_struct *partner = partner_manager::get_partner_by_uuid(area->m_partner_uuid[j]);
		if (!partner)
		{
				//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: %lu can not find sight partner %lu", __FUNCTION__, __LINE__, data->player_id, area->m_partner_uuid[j]);
			continue;
		}
		if (add_partner_to_sight_both(partner) < 0)
			continue;

		partner->pack_sight_partner_info(&add_partner[*add_partner_id_index]);
		(*add_partner_id_index)++;
	}
}

void player_struct::add_area_monster_to_sight(area_struct *area, int *add_monster_id_index, SightMonsterInfo *add_monster)
{
	if (!area)
		return;
	for (int j = 0; j < area->cur_monster_num; ++j)
	{
		monster_struct *monster = monster_manager::get_monster_by_id(area->m_monster_uuid[j]);
		if (!monster)
		{
				//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: %lu can not find sight monster %lu", __FUNCTION__, __LINE__, data->player_id, area->m_monster_uuid[j]);
			continue;
		}
		if (add_monster_to_sight_both(monster) < 0)
			continue;

		monster->pack_sight_monster_info(&add_monster[*add_monster_id_index]);
		(*add_monster_id_index)++;
	}
}

void player_struct::add_area_truck_to_sight(area_struct *area, int *add_truck_id_index, SightCashTruckInfo *add_truck)
{
	if (!area)
		return;
	for (int j = 0; j < area->cur_truck_num; ++j)
	{
		cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(area->m_truck_uuid[j]);
		if (!truck)
		{
			//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: %lu can not find sight monster %lu", __FUNCTION__, __LINE__, data->player_id, area->m_truck_uuid[j]);
			continue;
		}
		if (add_cash_truck_to_sight_both(truck) < 0)
			continue;

		truck->pack_sight_cash_truck_info(&add_truck[*add_truck_id_index]);
		(*add_truck_id_index)++;
	}
}

// bool player_struct::is_player_in_sight(uint64_t other)
// {
//	for (int j = 0; j < area->cur_player_num; ++j)
//	{
//		if (area->m_player_ids[j] == other)
//			return true;
//	}
//	return false;
// }

void player_struct::add_area_player_to_sight(area_struct *area, int *add_player_id_index, SightPlayerBaseInfo *add_player)
{
	if (!area)
		return;
	for (int j = 0; j < area->cur_player_num; ++j)
	{
		if (area->m_player_ids[j] == data->player_id)
			continue;
		player_struct *player = player_manager::get_player_by_id(area->m_player_ids[j]);
		if (!player)
		{
				//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: %lu can not find sight player %lu area[%p]", __FUNCTION__, __LINE__, data->player_id, area->m_player_ids[j], area);
			continue;
		}
		if (player->data->truck.on_truck)
		{
			continue;
		}

		if (add_player_to_sight_both(player) >= 0)
		{
//			if (get_entity_type(area->m_player_ids[j]) == ENTITY_TYPE_PLAYER)
			{
				player->pack_sight_player_base_info(&add_player[*add_player_id_index]);
				(*add_player_id_index)++;
			}
		}
	}
}

static PosData collect_pos[MAX_COLLECT_IN_PLAYER_SIGHT];
void player_struct::add_area_collect_to_sight(area_struct *area, int *add_collect_id_index, SightCollectInfo *add_collect)
{
	if (!area)
		return;
	for (int j = 0; j < area->cur_collect_num; ++j)
	{
		Collect *pCollect = Collect::GetById(area->m_collect_ids[j]);
		if (pCollect == NULL)
		{
			continue;
		}
		sight_collect_info__init(&add_collect[*add_collect_id_index]);
		add_collect[*add_collect_id_index].uuid = pCollect->m_uuid;
		add_collect[*add_collect_id_index].collectid = pCollect->m_collectId;
		add_collect[*add_collect_id_index].y = pCollect->m_y;
		add_collect[*add_collect_id_index].yaw = pCollect->m_yaw;
		pos_data__init(&collect_pos[*add_collect_id_index]);
		collect_pos[*add_collect_id_index].pos_x = pCollect->m_pos.pos_x;
		collect_pos[*add_collect_id_index].pos_z = pCollect->m_pos.pos_z;
		add_collect[*add_collect_id_index].data = &collect_pos[*add_collect_id_index];

		//PosData pos;
		//pos_data__init(&pos);
		//pos.pos_x = pCollect->m_pos.pos_x;
		//pos.pos_z = pCollect->m_pos.pos_z;
		//add_collect[*add_collect_id_index].data = &pos;

		(*add_collect_id_index)++;
	}
}


	//在栈上的话怕溢出
static SightPlayerBaseInfo player_info[MAX_PLAYER_IN_PLAYER_SIGHT];
static SightPlayerBaseInfo *player_info_point[MAX_PLAYER_IN_PLAYER_SIGHT];
static SightMonsterInfo monster_info[MAX_MONSTER_IN_PLAYER_SIGHT];
static SightMonsterInfo *monster_info_point[MAX_MONSTER_IN_PLAYER_SIGHT];
static SightCollectInfo collect_info[MAX_COLLECT_IN_PLAYER_SIGHT];
static SightCollectInfo *collect_info_point[MAX_COLLECT_IN_PLAYER_SIGHT];
static SightCashTruckInfo cash_truck_info[MAX_TRUCK_IN_PLAYER_SIGHT];
static SightCashTruckInfo *cash_truck_info_point[MAX_TRUCK_IN_PLAYER_SIGHT];
static SightPartnerInfo partner_info[MAX_PARTNER_IN_PLAYER_SIGHT];
static SightPartnerInfo *partner_info_point[MAX_PARTNER_IN_PLAYER_SIGHT];

void init_sight_unit_info_point()
{
	for (int i = 0; i < MAX_PLAYER_IN_PLAYER_SIGHT; ++i)
		player_info_point[i] = &player_info[i];
	for (int i = 0; i < MAX_MONSTER_IN_PLAYER_SIGHT; ++i)
		monster_info_point[i] = &monster_info[i];
	for (int i = 0; i < MAX_COLLECT_IN_PLAYER_SIGHT; ++i)
		collect_info_point[i] = &collect_info[i];
	for (int i = 0; i < MAX_TRUCK_IN_PLAYER_SIGHT; ++i)
		cash_truck_info_point[i] = &cash_truck_info[i];
	for (int i = 0; i < MAX_PARTNER_IN_PLAYER_SIGHT; ++i)
		partner_info_point[i] = &partner_info[i];
}

void player_struct::clear_player_sight()
{
	data->cur_sight_player = 0;
	data->cur_sight_monster = 0;
	data->cur_sight_truck = 0;
	data->cur_sight_partner = 0;
}

void player_struct::send_scene_transfer(float direct, float pos_x, float pos_y, float pos_z, uint32_t scene_id, int32_t result)
{
	if (get_entity_type(data->player_id) == ENTITY_TYPE_AI_PLAYER)
		return;

	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (!m_buffs[i])
			continue;
		m_buffs[i]->on_dead();
	}
	clear_watched_list();

	SceneTransferAnswer resp;
	scene_transfer_answer__init(&resp);
	resp.direct = direct;
	resp.new_scene_id = scene_id;
	resp.pos_x = pos_x;
	resp.pos_y = pos_y;
	resp.pos_z = pos_z;
	resp.result = result;
	EXTERN_DATA  extern_data;
	extern_data.player_id = get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_TRANSFER_ANSWER, scene_transfer_answer__pack, resp);
}

void player_struct::send_clear_sight_monster()
{
	if (get_entity_type(data->player_id) == ENTITY_TYPE_AI_PLAYER)
		return;
	SightChangedNotify notify;
	sight_changed_notify__init(&notify);
	notify.n_delete_monster = data->cur_sight_monster;
	notify.delete_monster = data->sight_monster;
	EXTERN_DATA extern_data;
	extern_data.player_id = data->player_id;
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_SIGHT_CHANGED_NOTIFY, sight_changed_notify__pack, notify);
}

void player_struct::send_clear_sight()
{
	if (get_entity_type(data->player_id) == ENTITY_TYPE_AI_PLAYER)
		return;
	SightChangedNotify notify;
	sight_changed_notify__init(&notify);
	notify.n_delete_monster = data->cur_sight_monster;
	notify.delete_monster = data->sight_monster;
	notify.n_delete_player = data->cur_sight_player;
	notify.delete_player = data->sight_player;
	uint32_t delete_collect_uuid[MAX_COLLECT_IN_PLAYER_SIGHT];
	int del_collect_uuid_index = 0;
	notify.n_delete_cash_truck = data->cur_sight_truck;
	notify.delete_cash_truck = data->sight_truck;

	if (data->truck.on_truck)
	{
		for (int t = 0; t < data->cur_sight_truck; ++t)
		{
			if (data->sight_truck[t] == data->truck.truck_id) //在镖车上 自己的镖车不删
			{
				data->sight_truck[t] = 0; //note 调这个函数之后会把视野全清除所以先置0了
			}
		}
	}


	if (area)
	{
		for (int j = 0; j < area->cur_collect_num; ++j)
		{
			delete_collect_uuid[del_collect_uuid_index++] = area->m_collect_ids[j];
		}

		for (int i = 0; i < MAX_NEIGHBOUR_AREA; ++i)
		{
			if (!area->neighbour[i])
				continue;
			for (int j = 0; j < area->neighbour[i]->cur_collect_num; ++j)
			{
				delete_collect_uuid[del_collect_uuid_index++] = area->neighbour[i]->m_collect_ids[j];
			}
		}
		notify.n_delete_collect = del_collect_uuid_index;
		notify.delete_collect = &delete_collect_uuid[0];
/*
		char log_buf[256];
		log_buf[0] = '\0';
		char *p = &log_buf[0];
		for (size_t i = 0; i < notify.n_delete_collect; ++i)
		{
			p += sprintf(p, "%u ", notify.delete_collect[i]);
		}

		LOG_DEBUG("%s: delete collect uuid %s", __FUNCTION__, log_buf);
*/
	}

	EXTERN_DATA extern_data;
	extern_data.player_id = data->player_id;
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_SIGHT_CHANGED_NOTIFY, sight_changed_notify__pack, notify);
}

int player_struct::broadcast_player_delete()
{
	for (int i = 0; i < data->cur_sight_player; ++i)
	{
		player_struct *player = player_manager::get_player_by_id(data->sight_player[i]);
		if (player)
			player->del_player_from_sight(data->player_id);
	}
	for (int i = 0; i < data->cur_sight_monster; ++i)
	{
		monster_struct *monster = monster_manager::get_monster_by_id(data->sight_monster[i]);
		if (monster)
			monster->del_player_from_sight(data->player_id);
	}
	for (int i = 0; i < data->cur_sight_truck; ++i)
	{
		cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(data->sight_truck[i]);
		if (truck)
			truck->del_player_from_sight(data->player_id);
	}
	for (int i = 0; i < data->cur_sight_partner; ++i)
	{
		partner_struct *partner = partner_manager::get_partner_by_uuid(data->sight_partner[i]);
		if (partner)
			partner->del_player_from_sight(data->player_id);
	}

	//	area_struct *area = scene->get_area_by_pos(data->pos.pos_x, data->pos.pos_z);
	//	area->del_player_from_area(data->player_id);
	if (!data->truck.on_truck) //不在镖车上
	{
		SightChangedNotify notify;
		sight_changed_notify__init(&notify);
		uint64_t del_player_id[1];
		del_player_id[0] = data->player_id;
		notify.delete_player = del_player_id;
		//发送给需要在视野里面添加玩家的通知
		notify.n_delete_player = 1;
		uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
		for (int i = 0; i < data->cur_sight_player; ++i)
			conn_node_gamesrv::broadcast_msg_add_players(data->sight_player[i], ppp);
		conn_node_gamesrv::broadcast_msg_send();
	}

	clear_player_sight();
	return (0);
}

int player_struct::broadcast_player_create(scene_struct *scene)
{
	SightChangedNotify notify;
	int add_player_id_index = 0;
	int add_monster_uuid_index = 0;
	int add_collect_uuid_index = 0;
	int add_truck_uuid_index = 0;
	int add_partner_uuid_index = 0;
	if (!scene || !area)
		return (0);

	add_area_player_to_sight(area, &add_player_id_index, player_info);
	add_area_monster_to_sight(area, &add_monster_uuid_index, monster_info);
	add_area_collect_to_sight(area, &add_collect_uuid_index, collect_info);
	add_area_truck_to_sight(area, &add_truck_uuid_index, cash_truck_info);
	add_area_partner_to_sight(area, &add_partner_uuid_index, partner_info);
	for (int i = 0; i < MAX_NEIGHBOUR_AREA; ++i)
	{
		add_area_player_to_sight(area->neighbour[i], &add_player_id_index, player_info);
		add_area_monster_to_sight(area->neighbour[i], &add_monster_uuid_index, monster_info);
		add_area_collect_to_sight(area->neighbour[i], &add_collect_uuid_index, collect_info);
		add_area_truck_to_sight(area->neighbour[i], &add_truck_uuid_index, cash_truck_info);
		add_area_partner_to_sight(area->neighbour[i], &add_partner_uuid_index, partner_info);
	}

	if (get_entity_type(data->player_id) != ENTITY_TYPE_AI_PLAYER &&
		(add_player_id_index > 0 || add_monster_uuid_index > 0 || add_collect_uuid_index > 0
		|| add_truck_uuid_index > 0 || add_partner_uuid_index > 0)
		)
	{
//发送别人的信息给自己
		sight_changed_notify__init(&notify);
		notify.add_player = player_info_point;
		notify.n_add_player = add_player_id_index;
		notify.add_monster = monster_info_point;
		notify.n_add_monster = add_monster_uuid_index;
		notify.n_add_collect = add_collect_uuid_index;
		notify.add_collect = collect_info_point;
		notify.add_cash_truck = cash_truck_info_point;
		notify.n_add_cash_truck = add_truck_uuid_index;
		notify.add_partner = partner_info_point;
		notify.n_add_partner = add_partner_uuid_index;
		EXTERN_DATA extern_data;
		extern_data.player_id = data->player_id;
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_SIGHT_CHANGED_NOTIFY, sight_changed_notify__pack, notify);
	}

	if (add_player_id_index > 0 && !data->truck.on_truck)
	{
//广播自己信息给别人
		sight_changed_notify__init(&notify);
		SightPlayerBaseInfo my_player_info[1];
		SightPlayerBaseInfo *my_player_info_point[1];
		my_player_info_point[0] = &my_player_info[0];
		notify.add_player = my_player_info_point;
		pack_sight_player_base_info(&my_player_info[0]);
			//发送给需要在视野里面添加玩家的通知
		notify.n_add_player = 1;

		uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
		for (int i = 0; i < add_player_id_index; ++i)
			conn_node_gamesrv::broadcast_msg_add_players(player_info[i].playerid, ppp);
		conn_node_gamesrv::broadcast_msg_send();
	}
	reset_pools();
	return (0);
}

void player_struct::del_sight_player_in_area(int n_del, area_struct **del_area, int *delete_player_id_index, uint64_t *delete_player_id)
{
	int i, j;
	int index = 0;
	player_struct *del_sight_player[MAX_PLAYER_IN_PLAYER_SIGHT];
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
		del_player_from_sight_both(del_sight_player[i]);
}

void player_struct::del_sight_partner_in_area(int n_del, area_struct **del_area, int *delete_partner_uuid_index, uint64_t *delete_partner_uuid)
{
	int i, j;
	int index = 0;
	partner_struct *del_sight_partner[MAX_PARTNER_IN_PLAYER_SIGHT];
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
			delete_partner_uuid[(*delete_partner_uuid_index)++] = partner->data->uuid;
			del_sight_partner[index++] = partner;
		}
	}

	for (i = 0; i < index; ++i)
		del_partner_from_sight_both(del_sight_partner[i]);
}

void player_struct::del_sight_monster_in_area(int n_del, area_struct **del_area, int *delete_monster_uuid_index, uint64_t *delete_monster_uuid)
{
	int i, j;
	int index = 0;
	monster_struct *del_sight_monster[MAX_MONSTER_IN_PLAYER_SIGHT];
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
			delete_monster_uuid[(*delete_monster_uuid_index)++] = monster->data->player_id;
			del_sight_monster[index++] = monster;
		}
	}

	for (i = 0; i < index; ++i)
		del_monster_from_sight_both(del_sight_monster[i]);
}

void player_struct::del_sight_truck_in_area(int n_del, area_struct **del_area, int *delete_truck_uuid_index, uint64_t *delete_truck_uuid)
{
	int i, j;
	int index = 0;
	cash_truck_struct *del_sight_truck[MAX_TRUCK_IN_PLAYER_SIGHT];
	for (i = 0; i < data->cur_sight_truck; ++i)
	{
		cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(data->sight_truck[i]);
		if (!truck)
		{
			//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: %lu can not find sight monster %lu", __FUNCTION__, __LINE__, data->player_id, data->sight_truck[i]);
			continue;
		}
		for (j = 0; j < n_del; ++j)
		{
			if (truck->area != del_area[j])
				continue;
			delete_truck_uuid[(*delete_truck_uuid_index)++] = truck->data->player_id;
			del_sight_truck[index++] = truck;
		}
	}

	for (i = 0; i < index; ++i)
		del_cash_truck_from_sight_both(del_sight_truck[i]);
}

void player_struct::update_sight(area_struct *old_area, area_struct *new_area)
{
	if (!old_area || !new_area || old_area == new_area)
		return;

//	uint64_t now = time_helper::get_cached_time();
//	if (now - last_change_area_time < 1500)
//	{
//		LOG_DEBUG("%s %d: player[%lu] update sight too much[%d]", __FUNCTION__, __LINE__, data->player_id, now - last_change_area_time)
//	}
//		return;
//	last_change_area_time = now;

	area_struct *del_area[MAX_NEIGHBOUR_AREA + 1];
	area_struct *add_area[MAX_NEIGHBOUR_AREA + 1];
	int n_add, n_del;
	area_struct::area_neighbour_diff(old_area, new_area, del_area, &n_del, add_area, &n_add);
	assert(n_add <= MAX_NEIGHBOUR_AREA + 1);
	assert(n_del <= MAX_NEIGHBOUR_AREA + 1);

	int delete_player_id_index = 0;
	uint64_t delete_player_id[MAX_PLAYER_IN_PLAYER_SIGHT];
	int add_player_id_index = 0;
	int delete_monster_uuid_index = 0;
	uint64_t delete_monster_uuid[MAX_MONSTER_IN_PLAYER_SIGHT];
	int add_monster_uuid_index = 0;
	uint32_t delete_collect_uuid[MAX_COLLECT_IN_PLAYER_SIGHT];
	int del_collect_uuid_index = 0;
	int add_collect_uuid_index = 0;
	int delete_truck_uuid_index = 0;
	int add_truck_uuid_index = 0;
	uint64_t delete_truck_uuid[MAX_TRUCK_IN_PLAYER_SIGHT];
	int delete_partner_uuid_index = 0;
	uint64_t delete_partner_uuid[MAX_PARTNER_IN_PLAYER_SIGHT];
	int add_partner_uuid_index = 0;


	SightChangedNotify notify;
	sight_changed_notify__init(&notify);

	del_sight_player_in_area(n_del, &del_area[0], &delete_player_id_index, &delete_player_id[0]);
	del_sight_monster_in_area(n_del, &del_area[0], &delete_monster_uuid_index, &delete_monster_uuid[0]);
	del_sight_truck_in_area(n_del, &del_area[0], &delete_truck_uuid_index, &delete_truck_uuid[0]);
	del_sight_partner_in_area(n_del, &del_area[0], &delete_partner_uuid_index, &delete_partner_uuid[0]);

	for (int i = 0; i < n_del; ++i)
	{
		for (int j = 0; j < del_area[i]->cur_collect_num; ++j)
		{
			delete_collect_uuid[del_collect_uuid_index++] = del_area[i]->m_collect_ids[j];
		}
	}
		//发送给需要在视野里面删除玩家的通知
	notify.n_delete_player = 1;
	uint64_t player_ids[1];
	player_ids[0] = data->player_id;
	notify.delete_player = player_ids;
	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
	for (int i = 0; i < delete_player_id_index; ++i)
		conn_node_gamesrv::broadcast_msg_add_players(delete_player_id[i], ppp);
	conn_node_gamesrv::broadcast_msg_send();

	sight_changed_notify__init(&notify);
	for (int i = 0; i < n_add; ++i)
	{
		add_area_player_to_sight(add_area[i], &add_player_id_index, player_info);
		add_area_monster_to_sight(add_area[i], &add_monster_uuid_index, monster_info);
		add_area_collect_to_sight(add_area[i], &add_collect_uuid_index, collect_info);
		add_area_truck_to_sight(add_area[i], &add_truck_uuid_index, cash_truck_info);
		add_area_partner_to_sight(add_area[i], &add_partner_uuid_index, partner_info);
	}

	if ((get_entity_type(data->player_id) == ENTITY_TYPE_PLAYER) &&
		(delete_player_id_index > 0 ||
			add_player_id_index > 0 ||
			delete_monster_uuid_index > 0 || add_monster_uuid_index > 0 ||
			delete_truck_uuid_index > 0 || add_truck_uuid_index > 0 ||
			delete_partner_uuid_index > 0 || add_partner_uuid_index > 0 ||
			del_collect_uuid_index > 0 || add_collect_uuid_index > 0))
	{
			//把所有要删除和要添加的玩家数据发送给玩家自己
		sight_changed_notify__init(&notify);
		notify.n_delete_player = delete_player_id_index;
		notify.delete_player = delete_player_id;
		notify.n_add_player = add_player_id_index;
		notify.add_player = player_info_point;

		notify.n_delete_monster = delete_monster_uuid_index;
		notify.delete_monster = delete_monster_uuid;
		notify.n_add_monster = add_monster_uuid_index;
		notify.add_monster = monster_info_point;

		notify.n_delete_collect = del_collect_uuid_index;
		notify.delete_collect = delete_collect_uuid;
		notify.n_add_collect = add_collect_uuid_index;
		notify.add_collect = collect_info_point;

		notify.n_delete_cash_truck = delete_truck_uuid_index;
		notify.delete_cash_truck = delete_truck_uuid;
		notify.n_add_cash_truck = add_truck_uuid_index;
		notify.add_cash_truck = cash_truck_info_point;

		notify.n_delete_partner = delete_partner_uuid_index;
		notify.delete_partner = delete_partner_uuid;
		notify.n_add_partner = add_partner_uuid_index;
		notify.add_partner = partner_info_point;

		size_t size = sight_changed_notify__pack(&notify, conn_node_base::get_send_data());
		if (size != (size_t)-1)
		{
			PROTO_HEAD *head = conn_node_base::get_send_buf(MSG_ID_SIGHT_CHANGED_NOTIFY, 0);
			head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);
			head->seq = conn_node_gamesrv::connecter.get_seq();
			EXTERN_DATA extern_data;
			extern_data.player_id = data->player_id;
			conn_node_base::add_extern_data(head, &extern_data);
			size_t ret = conn_node_gamesrv::connecter.send_one_msg(head, 1);
			if (ret != ENDION_FUNC_4(head->len)) {
				LOG_ERR("%s: send to client failed err[%d]", __FUNCTION__, errno);
			}
		}
	}


		//把自己发送给别的玩家
	if (add_player_id_index > 0 && !data->truck.on_truck)
	{
		sight_changed_notify__init(&notify);
		SightPlayerBaseInfo my_player_info[1];
		SightPlayerBaseInfo *my_player_info_point[1];
		my_player_info_point[0] = &my_player_info[0];
		notify.add_player = my_player_info_point;
		pack_sight_player_base_info(my_player_info_point[0]);
			//发送给需要在视野里面添加玩家的通知
		notify.n_add_player = 1;

		ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
		for (int i = 0; i < add_player_id_index; ++i)
			conn_node_gamesrv::broadcast_msg_add_players(player_info[i].playerid, ppp);
		conn_node_gamesrv::broadcast_msg_send();
	}
	reset_pools();

//	LOG_DEBUG("%s %d: player[%lu] del player[%u] add player[%u] del monster[%u] add monster[%u] area from %d to %d",
//		__FUNCTION__, __LINE__,	data->player_id,
//		delete_player_id_index, add_player_id_index,
//		delete_monster_uuid_index, add_monster_uuid_index,
//		old_area - scene->m_area, new_area - scene->m_area);

	if (old_area->del_player_from_area(data->player_id) != 0)
	{
		LOG_ERR("%s %d: can not del player[%lu] from area[%ld %p]", __FUNCTION__, __LINE__, data->player_id, old_area - scene->m_area, old_area);
	}
	new_area->add_player_to_area(data->player_id);
	area = new_area;
}

void player_struct::check_qiecuo_range()
{
	if (!is_in_qiecuo())
		return;
	struct position *pos = get_pos();
//	if (pos->pos_x - data->qiecuo_pos.pos_x < sg_qiecuo_range &&
//		pos->pos_x - data->qiecuo_pos.pos_x > -sg_qiecuo_range)
	if (check_circle_in_range(pos, &data->qiecuo_pos, sg_qiecuo_range))
	{
		if (data->qiecuo_out_range_fail_time != 0)
		{
			EXTERN_DATA extern_data;
			extern_data.player_id = get_uuid();
			fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_QIECUO_IN_RANGE_NOTIFY, 0, 0);
			data->qiecuo_out_range_fail_time = 0;
		}
		return;
	}

	uint64_t now = time_helper::get_cached_time() / 1000;
	if (data->qiecuo_out_range_fail_time == 0)
	{
		EXTERN_DATA extern_data;
		extern_data.player_id = get_uuid();
		fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_QIECUO_OUT_RANGE_NOTIFY, 0, 0);
		data->qiecuo_out_range_fail_time = now + sg_qiecuo_out_range_timeout;
		return;
	}

	if (now < data->qiecuo_out_range_fail_time)
		return;

	LOG_INFO("%s: player[%lu] out range qiecuo failed", __FUNCTION__, get_uuid());

	player_struct *target = player_manager::get_player_by_id(data->qiecuo_target);
	assert(target);
	finish_qiecuo();
	target->finish_qiecuo();

	QiecuoFinishNotify nty;
	EXTERN_DATA ext;
	qiecuo_finish_notify__init(&nty);

	nty.result = 0;
	ext.player_id = target->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_FINISH_NOTIFY, qiecuo_finish_notify__pack, nty);

	nty.result = 1;
	ext.player_id = get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_FINISH_NOTIFY, qiecuo_finish_notify__pack, nty);
}

void player_struct::update_player_pos_and_sight()
{
	check_qiecuo_range();

//	if (!scene || !area)
//		return;
	if (!scene)
		return;

//	if (data->attrData[PLAYER_ATTR_MOVE_SPEED] == 0)
	if (get_speed() < __DBL_EPSILON__)
		return;
	if (!is_unit_in_move())
		return;

	if (scene->get_scene_type() == SCENE_TYPE_RAID
		&& ((raid_struct *)scene)->data->state != RAID_STATE_START)
		return;

	struct position *pos = get_pos();
	float pos_x = pos->pos_x;
	float pos_z = pos->pos_z;

	area_struct *old_area = area;
	if (update_unit_position() == 0)
		return;
	pos = get_pos();
	if (!check_pos_valid(scene->map_config, pos->pos_x, pos->pos_z))
	{
			//新的路径点无效，那么停止移动
		set_pos(pos_x, pos_z);
//		check_qiecuo_range();
		return;
	}

	update_region_id();
//	check_qiecuo_range();

	cash_truck_struct *truck = NULL;
	if (data->truck.on_truck)
	{
		truck = cash_truck_manager::get_cash_truck_by_id(data->truck.truck_id);
	}
	if (truck != NULL)
	{
		truck->set_pos(pos->pos_x, pos->pos_z);
	}

	if (!old_area)
		return;

	area_struct *new_area = scene->get_area_by_pos(pos->pos_x, pos->pos_z);
		//检查是否越过area
	if (old_area == new_area || !new_area)
		return;

	update_sight(old_area, new_area);

	if (truck != NULL && truck->area)
	{
		truck->update_sight(truck->area, new_area);
	}
}

void player_struct::on_region_changed(uint16_t old_region_id, uint16_t new_region_id)
{
	switch (new_region_id)
	{
		case 11:
			if (is_in_qiecuo())
			{
				LOG_INFO("%s: player[%lu] enter safe region range qiecuo failed", __FUNCTION__, get_uuid());
				player_struct *target = player_manager::get_player_by_id(data->qiecuo_target);
				assert(target);
				finish_qiecuo();
				target->finish_qiecuo();

				QiecuoFinishNotify nty;
				EXTERN_DATA ext;
				qiecuo_finish_notify__init(&nty);

				nty.result = 0;
				ext.player_id = target->get_uuid();
				fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_FINISH_NOTIFY, qiecuo_finish_notify__pack, nty);

				nty.result = 1;
				ext.player_id = get_uuid();
				fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_FINISH_NOTIFY, qiecuo_finish_notify__pack, nty);
			}
			set_attr(PLAYER_ATTR_PK_TYPE, PK_TYPE_NORMAL);
			broadcast_one_attr_changed(PLAYER_ATTR_PK_TYPE, PK_TYPE_NORMAL, false, true);
			break;
		case 12:
			//		//检查等级
			// if (get_attr(PLAYER_ATTR_LEVEL) < sg_pk_level)
			//	return;
			//没有选择阵营
			if (get_attr(PLAYER_ATTR_ZHENYING) <= __DBL_EPSILON__)
				return;

			set_attr(PLAYER_ATTR_PK_TYPE, PK_TYPE_CAMP);
			broadcast_one_attr_changed(PLAYER_ATTR_PK_TYPE, PK_TYPE_CAMP, false, true);
			break;
		// case 15:
		// {
		//	if (!is_alive())
		//		return;
		//	raid_struct *raid = get_raid();
		//	if (raid && raid->ai && raid->ai->raid_on_fall)
		//	{
		//		raid->ai->raid_on_fall(raid, this);
		//	}
		// }
		//	break;
	}

	if (old_region_id == 11 || old_region_id == 12)
	{
		if (get_attr(PLAYER_ATTR_MURDER) > sg_muder_cant_set_pktype)
		{
			set_attr(PLAYER_ATTR_PK_TYPE, PK_TYPE_MURDER);
			broadcast_one_attr_changed(PLAYER_ATTR_PK_TYPE, PK_TYPE_MURDER, false, true);
		}
	}

	raid_struct *raid = get_raid();
	if (raid && raid->ai && raid->ai->raid_on_player_region_changed)
	{
		raid->ai->raid_on_player_region_changed(raid, this, new_region_id);
	}

}

void player_struct::update_region_id()
{
	if (!is_avaliable())
		return;
	struct position *pos = get_pos();
	uint16_t old_region_id = get_attr(PLAYER_ATTR_REGION_ID);
	uint16_t new_region_id = get_region_id(scene->map_config, scene->region_config, pos->pos_x, pos->pos_z);
	if (new_region_id != old_region_id)
	{
//		data->region_id = new_region_id;
		set_attr(PLAYER_ATTR_REGION_ID, new_region_id);
		send_enter_region_notify(new_region_id);
		on_region_changed(old_region_id, new_region_id);
	}
}

void player_struct::pack_sight_player_base_info(SightPlayerBaseInfo *info)
{
//	PosData *pos_point[MAX_PATH_POSITION];
//	PosData pos[MAX_PATH_POSITION];
	sight_player_base_info__init(info);
	info->name = data->name;
	info->playerid = data->player_id;
	info->job = data->attrData[PLAYER_ATTR_JOB];
	info->speed = data->attrData[PLAYER_ATTR_MOVE_SPEED];
	info->hp = data->attrData[PLAYER_ATTR_HP];
	info->icon = data->attrData[PLAYER_ATTR_HEAD];
	info->lv = data->attrData[PLAYER_ATTR_LEVEL];
	info->maxhp = data->attrData[PLAYER_ATTR_MAXHP];
	info->clothes = data->attrData[PLAYER_ATTR_CLOTHES];
	info->clothes_color_up = data->attrData[PLAYER_ATTR_CLOTHES_COLOR_UP];
	info->clothes_color_down = data->attrData[PLAYER_ATTR_CLOTHES_COLOR_DOWN];
	info->hat = data->attrData[PLAYER_ATTR_HAT];
	info->hat_color = data->attrData[PLAYER_ATTR_HAT_COLOR];
	info->team_id = data->teamid;
	info->fashion_weapon = data->attrData[PLAYER_ATTR_WEAPON];
	info->horse = get_on_horse_id();
	info->pos_y = data->pos_y;
	info->pk_type = data->attrData[PLAYER_ATTR_PK_TYPE];
	info->zhenying_id = data->attrData[PLAYER_ATTR_ZHENYING];
	info->region_id = data->attrData[PLAYER_ATTR_REGION_ID];
	info->guild_id = data->guild_id;
	info->fashion_weapon_color = data->attrData[PLAYER_ATTR_WEAPON_COLOR];
/*
	info->data = &pos_pool_pos_point[pos_pool_len];
	if (!is_unit_in_move(&data->move_path))
	{
		info->n_data = 1;
//		pos_pool_pos_point[pos_pool_len] = &pos_pool_pos[pos_pool_len];
		pos_data__init(pos_pool_pos_point[pos_pool_len]);
		pos_pool_pos[pos_pool_len].pos_x = get_player_pos()->pos_x;
		pos_pool_pos[pos_pool_len].pos_z = get_player_pos()->pos_z;
		++pos_pool_len;
	}
	else
	{
		info->n_data = data->move_path.max_pos - data->move_path.cur_pos;
		if (info->n_data == 0)
			info->n_data = 1;
		for (uint32_t i = 0; i < info->n_data; ++i)
		{
//			pos_pool_pos_point[i + pos_pool_len] = &pos_pool_pos[i + pos_pool_len];
			pos_data__init(pos_pool_pos_point[i + pos_pool_len]);
			pos_pool_pos[i + pos_pool_len].pos_x = data->move_path.pos[data->move_path.cur_pos + i].pos_x;
			pos_pool_pos[i + pos_pool_len].pos_z = data->move_path.pos[data->move_path.cur_pos + i].pos_z;
		}
		info->direct_x = data->move_path.speed_x;
		info->direct_y = 0;
		info->direct_z = data->move_path.speed_z;
		pos_pool_len += info->n_data;
	}
*/
	info->direct_x = data->move_path.speed_x;
	info->direct_y = 0;
	info->direct_z = data->move_path.speed_z;
	info->data = pack_unit_move_path(&info->n_data);
	info->buff_info = pack_unit_buff(&info->n_buff_info);
}

void player_struct::calculate_lv4_attribute()
{
// 面板金系抗性=最终全系抗性+角色金系抗性+其他系统附加属性
	data->attrData[PLAYER_ATTR_DEF_METAL] += data->attrData[PLAYER_ATTR_DFWU];
// 面板木系抗性=最终全系抗性+角色木系抗性+其他系统附加属性
	data->attrData[PLAYER_ATTR_DEF_WOOD] += data->attrData[PLAYER_ATTR_DFWU];
// 面板水系抗性=最终全系抗性+角色水系抗性+其他系统附加属性
	data->attrData[PLAYER_ATTR_DEF_WATER] += data->attrData[PLAYER_ATTR_DFWU];
// 面板火系抗性=最终全系抗性+角色火系抗性+其他系统附加属性
	data->attrData[PLAYER_ATTR_DEF_FIRE] += data->attrData[PLAYER_ATTR_DFWU];
// 面板土系抗性=最终全系抗性+角色土系抗性+其他系统附加属性
	data->attrData[PLAYER_ATTR_DEF_EARTH] += data->attrData[PLAYER_ATTR_DFWU];

// 面板眩晕几率=最终属性效果几率+角色眩晕几率+其他系统附加属性			
// 面板迟缓几率=最终属性效果几率+角色迟缓几率+其他系统附加属性			
// 面板麻痹几率=最终属性效果几率+角色麻痹几率+其他系统附加属性			
// 面板受伤几率=最终属性效果几率+角色受伤几率+其他系统附加属性			
// 面板致残几率=最终属性效果几率+角色致残几率+其他系统附加属性
	data->attrData[PLAYER_ATTR_DIZZY] += data->attrData[PLAYER_ATTR_ALLEFF];
	data->attrData[PLAYER_ATTR_SLOW] += data->attrData[PLAYER_ATTR_ALLEFF];
	data->attrData[PLAYER_ATTR_MABI] += data->attrData[PLAYER_ATTR_ALLEFF];
	data->attrData[PLAYER_ATTR_HURT] += data->attrData[PLAYER_ATTR_ALLEFF];
	data->attrData[PLAYER_ATTR_CAN] += data->attrData[PLAYER_ATTR_ALLEFF];	

// 面板抗眩晕几率=最终抗属性效果几率+角色抗眩晕几率+其他系统附加属性			
// 面板抗迟缓几率=最终抗属性效果几率+角色抗迟缓几率+其他系统附加属性			
// 面板抗麻痹几率=最终抗属性效果几率+角色抗麻痹几率+其他系统附加属性			
// 面板抗受伤几率=最终抗属性效果几率+角色抗受伤几率+其他系统附加属性			
// 面板抗致残几率=最终抗属性效果几率+角色抗致残几率+其他系统附加属性
	data->attrData[PLAYER_ATTR_DIZZYDF] += data->attrData[PLAYER_ATTR_ALLEFFDF];
	data->attrData[PLAYER_ATTR_SLOWDF] += data->attrData[PLAYER_ATTR_ALLEFFDF];
	data->attrData[PLAYER_ATTR_MABIDF] += data->attrData[PLAYER_ATTR_ALLEFFDF];
	data->attrData[PLAYER_ATTR_HURTDF] += data->attrData[PLAYER_ATTR_ALLEFFDF];
	data->attrData[PLAYER_ATTR_CANDF] += data->attrData[PLAYER_ATTR_ALLEFFDF];	
}

void player_struct::calculate_lv3_attribute()
{
// 面板生命=面板体质*（1+最终基础生命加成）*体质转生命系数+角色等级生命+其他系统附加属性
	data->attrData[PLAYER_ATTR_MAXHP] += data->attrData[PLAYER_ATTR_TI] * (1 + data->attrData[PLAYER_ATTR_HEALTHPRO]) * sg_fight_param_161000274;
// 面板攻击=面板力量*（1+最终基础攻击加成）*力量转攻击系数+角色等级攻击+其他系统附加属性
	data->attrData[PLAYER_ATTR_ATTACK] += data->attrData[PLAYER_ATTR_LI] * (1 + data->attrData[PLAYER_ATTR_ATTACKPRO]) * sg_fight_param_161000275;
// 最终全系抗性=面板敏捷*敏捷转全系抗性系数+其他系统附加属性
	data->attrData[PLAYER_ATTR_DFWU] += data->attrData[PLAYER_ATTR_MIN] * sg_fight_param_161000277;
// 面板忽略全抗=角色忽略全抗+其他系统附加属性
// 最终金系伤害=角色金系伤害+其他系统附加属性	
// 最终木系伤害=角色木系伤害+其他系统附加属性	
// 最终水系伤害=角色水系伤害+其他系统附加属性	
// 最终火系伤害=角色火系伤害+其他系统附加属性	
// 最终土系伤害=角色土系伤害+其他系统附加属性	
// 面板命中=面板灵巧*灵巧转命中系数+角色命中+其他系统附加属性
	data->attrData[PLAYER_ATTR_HIT] += data->attrData[PLAYER_ATTR_LING] * sg_fight_param_161000278;
// 面板闪避=面板敏捷*敏捷转闪避系数+角色闪避+其他系统附加属性
	data->attrData[PLAYER_ATTR_DODGE] += data->attrData[PLAYER_ATTR_MIN] * sg_fight_param_161000276;
// 面板会心几率=面板灵巧*灵巧转会心几率系数+角色会心几率+其他系统附加属性
	data->attrData[PLAYER_ATTR_CRIT] += data->attrData[PLAYER_ATTR_LING] * sg_fight_param_161000279;
// 面板忽略闪避=角色忽略闪避+其他系统附加属性
// 面板抗会心几率=角色抗会心几率+其他系统附加属性	
// 面板会心伤害=角色会心伤害+其他系统附加属性	
// 面板会心免伤=角色会心免伤+其他系统附加属性	
// 面板移动速度=角色移动速度+其他系统附加属性	
// 最终属性效果几率=其他系统附加属性	
// 最终抗属性效果几率=其他系统附加属性	
// 最终属性效果时间=其他系统附加属性	
// 最终抗属性效果时间=其他系统附加属性	
}

void player_struct::calculate_lv2_attribute()
{
// 面板体质=角色等级*角色体质成长+角色体质+其他系统附加属性		
// 面板力量=角色等级*角色力量成长+角色力量+其他系统附加属性		
// 面板敏捷=角色等级*角色敏捷成长+角色敏捷+其他系统附加属性		
// 面板灵巧=角色等级*角色灵巧成长+角色灵巧+其他系统附加属性
	uint32_t player_job = data->attrData[PLAYER_ATTR_JOB];
	ActorTable *config = get_actor_config(player_job);
	assert(config);
	data->attrData[PLAYER_ATTR_TI] += data->attrData[PLAYER_ATTR_LEVEL] * config->TiLv + config->Ti;
	data->attrData[PLAYER_ATTR_LI] += data->attrData[PLAYER_ATTR_LEVEL] * config->LiLv + config->Li;
	data->attrData[PLAYER_ATTR_MIN] += data->attrData[PLAYER_ATTR_LEVEL] * config->MinLv + config->Min;
	data->attrData[PLAYER_ATTR_LING] += data->attrData[PLAYER_ATTR_LEVEL] * config->LingLv + config->Ling;		
}

void player_struct::calculate_attribute(bool isNty)
{
//	memset(&data->attrData[PLAYER_ATTR_MAXHP], 0, sizeof(double));
//	data->attrData[PLAYER_ATTR_MAXHP] = 0;
//	memset(&data->attrData[PLAYER_ATTR_ATTACK], 0, (PLAYER_ATTR_DETIMEDF - PLAYER_ATTR_ATTACK + 1) * sizeof(double));
	memset(&data->attrData[PLAYER_ATTR_MAXHP], 0, (PLAYER_ATTR_PVPDF - PLAYER_ATTR_MAXHP + 1) * sizeof(double));	

	for (int i = PLAYER_ATTR_TI; i <= PLAYER_ATTR_ALLEFFDF; ++i)
		data->attrData[i] = 0;
	// data->attrData[PLAYER_ATTR_DFWU] = 0;
	// data->attrData[PLAYER_ATTR_TI] = 0;
	// data->attrData[PLAYER_ATTR_LI] = 0;
	// data->attrData[PLAYER_ATTR_MIN] = 0;
	// data->attrData[PLAYER_ATTR_LING] = 0;

	double module_attr[PLAYER_ATTR_MAX];
	memset(module_attr, 0, sizeof(double) * PLAYER_ATTR_MAX);
	memset(&data->fc_data, 0, sizeof(FightingCapacity));

	calcu_level_attr(module_attr);
	add_fight_attr(data->attrData, module_attr);
	data->fc_data.level = calculate_fighting_capacity(module_attr);

	calcu_equip_attr(module_attr);
	add_fight_attr(data->attrData, module_attr);
	data->fc_data.equip = calculate_fighting_capacity(module_attr);

	calc_horse_attr(module_attr);
	add_fight_attr(data->attrData, module_attr);
	data->fc_data.horse = calculate_fighting_capacity(module_attr);

	calcu_yuqidao_attr(module_attr);
	add_fight_attr(data->attrData, module_attr);
	data->fc_data.yuqidao = calculate_fighting_capacity(module_attr);

	calcu_baguapai_attr(module_attr);
	add_fight_attr(data->attrData, module_attr);
	data->fc_data.bagua = calculate_fighting_capacity(module_attr);

	calcu_guild_skill_attr(module_attr);
	add_fight_attr(data->attrData, module_attr);
	data->fc_data.guild_skill = calculate_fighting_capacity(module_attr);

	calcu_partner_attr(module_attr);
	add_fight_attr(data->attrData, module_attr);
	data->fc_data.partner = get_partner_fc() + calculate_fighting_capacity(module_attr);

	if (data->charm_level > 1)
	{
		calcu_fashion_attr(module_attr);
		add_fight_attr(data->attrData, module_attr);
		data->fc_data.fashion = calculate_fighting_capacity(module_attr);
	}

	calculate_lv2_attribute();
	calculate_lv3_attribute();
	calculate_lv4_attribute();		

//	data->speed = data->attrData[PLAYER_ATTR_MOVE_SPEED];// / 10.0;

	print_attribute("total", data->attrData);

	uint32_t prev_fp = get_attr(PLAYER_ATTR_FIGHTING_CAPACITY);
	uint32_t cur_fp = data->fc_data.get_total();
	bool NtyFp = false;
	if (cur_fp != prev_fp)
	{
		NtyFp = true;
		data->attrData[PLAYER_ATTR_FIGHTING_CAPACITY] = cur_fp;
		refresh_player_redis_info();
		LOG_DEBUG("[%s:%d] player[%lu] fighting capacity, %u --> %u", __FUNCTION__, __LINE__, data->player_id, prev_fp, cur_fp);
	}

	if (isNty)
	{
		AttrMap nty_list;
		for (uint32_t i = 1; i < PLAYER_ATTR_FIGHT_MAX; ++i)
		{
			//当前生命值，由外部进行视野广播
			if (i == PLAYER_ATTR_HP || i > PLAYER_ATTR_PVPDF)
			{
				continue;
			}
			nty_list[i] = data->attrData[i];
		}

		if (NtyFp)
		{
			nty_list[PLAYER_ATTR_FIGHTING_CAPACITY] = data->attrData[PLAYER_ATTR_FIGHTING_CAPACITY];
		}

		this->notify_attr(nty_list);
	}
	calculate_buff_fight_attr(isNty);
}

void player_struct::notify_attr(AttrMap& attr_list, bool broadcast, bool include_myself)
{
	if (!broadcast && get_entity_type(data->player_id) != ENTITY_TYPE_PLAYER)
		return;

	PlayerAttrNotify nty;
	player_attr_notify__init(&nty);
	AttrData attr_data[PLAYER_ATTR_MAX + 2];
	AttrData *attr_data_point[PLAYER_ATTR_MAX + 2];

	nty.player_id = data->player_id;
	nty.n_attrs = 0;
	nty.attrs = attr_data_point;
	for (AttrMap::iterator iter = attr_list.begin(); iter != attr_list.end(); ++iter)
	{
		attr_data_point[nty.n_attrs] = &attr_data[nty.n_attrs];
		attr_data__init(&attr_data[nty.n_attrs]);
		attr_data[nty.n_attrs].id = iter->first;
		attr_data[nty.n_attrs].val = iter->second;
		nty.n_attrs++;
	}

	if (broadcast)
	{
		this->broadcast_to_sight_and_team(MSG_ID_PLAYER_ATTR_NOTIFY, &nty, (pack_func)player_attr_notify__pack, include_myself);
//		if (m_team != NULL)
//		{
//			m_team->BroadcastToTeamNotinSight(*this, MSG_ID_PLAYER_ATTR_NOTIFY, &nty, (pack_func)player_attr_notify__pack);
//		}
	}
	else
	{
		EXTERN_DATA extern_data;
		extern_data.player_id = data->player_id;
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_PLAYER_ATTR_NOTIFY, player_attr_notify__pack, nty);
	}
}

void player_struct::notify_attr_changed(uint32_t num, uint32_t* id, double *value)
{
	if (get_entity_type(data->player_id) != ENTITY_TYPE_PLAYER)
		return;

	PlayerAttrNotify nty;
	player_attr_notify__init(&nty);
	AttrData attr_data[PLAYER_ATTR_MAX + 2];
	AttrData *attr_data_point[PLAYER_ATTR_MAX + 2];

	nty.player_id = data->player_id;
	nty.n_attrs = num;
	nty.attrs = attr_data_point;

	for (size_t i = 0; i < num; ++i)
	{
		attr_data_point[i] = &attr_data[i];
		attr_data__init(&attr_data[i]);
		attr_data[i].id = id[i];
		attr_data[i].val = value[i];
	}
	EXTERN_DATA extern_data;
	extern_data.player_id = data->player_id;
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_PLAYER_ATTR_NOTIFY, player_attr_notify__pack, nty);
}

void player_struct::notify_one_attr_changed(uint32_t attr_id, double attr_val)
{
	if (get_entity_type(data->player_id) != ENTITY_TYPE_PLAYER)
		return;

	PlayerAttrNotify nty;
	player_attr_notify__init(&nty);
	AttrData attr_data[1];
	AttrData *attr_data_point[1];

	nty.player_id = data->player_id;
	nty.n_attrs = 1;
	nty.attrs = attr_data_point;

	attr_data_point[0] = &attr_data[0];
	attr_data__init(&attr_data[0]);
	attr_data[0].id = attr_id;
	attr_data[0].val = attr_val;

	EXTERN_DATA extern_data;
	extern_data.player_id = data->player_id;
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_PLAYER_ATTR_NOTIFY, player_attr_notify__pack, nty);
}

void player_struct::send_enter_region_notify(int region_id)
{
	// EnterRegionNotify nty;
	// enter_region_notify__init(&nty);
	// nty.region_id = region_id;
	// EXTERN_DATA extern_data;
	// extern_data.player_id = data->player_id;
	// fast_send_msg(&conn_node_gamesrv::connecter, &extern_data,
	//	MSG_ID_ENTER_REGION_NOTIFY, enter_region_notify__pack, nty);
	broadcast_one_attr_changed(PLAYER_ATTR_REGION_ID, region_id, false, true);
}

void player_struct::broadcast_one_attr_changed(uint32_t id, double value, bool send_team, bool include_myself)
{
	PlayerAttrNotify nty;
	player_attr_notify__init(&nty);
	AttrData attr_data[1];
	AttrData *attr_data_point[1];

	nty.player_id = data->player_id;
	nty.n_attrs = 1;
	nty.attrs = attr_data_point;
	attr_data_point[0] = &attr_data[0];
	attr_data__init(&attr_data[0]);
	attr_data[0].id = id;
	attr_data[0].val = value;

	broadcast_to_sight(MSG_ID_PLAYER_ATTR_NOTIFY, &nty, (pack_func)player_attr_notify__pack, include_myself);
	if (m_team != NULL && send_team)
	{
		uint64_t except = 0;
		if (include_myself)
			except = get_uuid();
		m_team->BroadcastToTeamNotinSight(*this, MSG_ID_PLAYER_ATTR_NOTIFY, &nty, (pack_func)player_attr_notify__pack, except);
	}
}

void player_struct::calcu_level_attr(double *attr)
{
	uint32_t player_job = data->attrData[PLAYER_ATTR_JOB];
	uint32_t player_level = data->attrData[PLAYER_ATTR_LEVEL];
	ActorLevelTable *level_config = get_actor_level_config(player_job, player_level);
	if (!level_config)
	{
		LOG_ERR("[%s:%d] get level config failed, job: %u, level:%u", __FUNCTION__, __LINE__, player_job, player_level);
		return;
	}

	memset(attr, 0, sizeof(double) * PLAYER_ATTR_MAX);
	::get_attr_from_config(level_config->ActorLvAttri, attr, NULL);
	print_attribute("level", attr);
}

void player_struct::calcu_equip_attr(double *attr)
{
	memset(attr, 0, sizeof(double) * PLAYER_ATTR_MAX);

	uint32_t player_job = data->attrData[PLAYER_ATTR_JOB];
	for (int equip_i = 0; equip_i < MAX_EQUIP_NUM; ++equip_i)
	{
		EquipInfo &equip_info = data->equip_list[equip_i];
		uint32_t type = equip_i + 1;

		//星级属性
		EquipmentTable *equip_config = get_equip_config(player_job, type, equip_info.stair, equip_info.star_lv);
		if (equip_config)
		{
			attr[equip_config->AttriEquipType] += equip_config->AttriEquipValue;
		}
		if (equip_info.star_exp > 0)
		{
			EquipmentTable *next_equip_config = get_equip_config(player_job, type, equip_info.stair, equip_info.star_lv + 1);
			EquipStarLv *next_star_config = get_equip_star_config(player_job, type, equip_info.stair, equip_info.star_lv + 1);
			if (next_equip_config && next_star_config)
			{
				double value = (next_equip_config->AttriEquipValue - equip_config->AttriEquipValue) * ((double)equip_info.star_exp / next_star_config->StarSchedule);
				attr[equip_config->AttriEquipType] += value;
			}
		}

		//附魔属性
		for (int enchant_i = 0; enchant_i < MAX_EQUIP_ENCHANT_NUM; ++enchant_i)
		{
			EquipEnchantInfo &enchant_info = equip_info.enchant[enchant_i];
			if (enchant_info.cur_attr.id > 0)
			{
				attr[enchant_info.cur_attr.id] += enchant_info.cur_attr.val;
			}
		}

		//镶嵌属性
		for (int inlay_i = 0; inlay_i < MAX_EQUIP_INLAY_NUM; ++inlay_i)
		{
			int32_t gem_id = equip_info.inlay[inlay_i];
			if (gem_id <= 0)
			{
				continue;
			}

			GemAttribute *gem_config = get_gem_config(gem_id);
			if (!gem_config)
			{
				continue;
			}

			attr[gem_config->AttributeType] += gem_config->AttributeValue;
		}
	}

	print_attribute("equip", attr);
}

void player_struct::calcu_yuqidao_attr(double *attr)
{
	memset(attr, 0, sizeof(double) * PLAYER_ATTR_MAX);

	//穴脉属性
	for (int mai_i = 0; mai_i < MAX_YUQIDAO_MAI_NUM; ++mai_i)
	{
		YuqidaoMaiInfo &mai_info = data->yuqidao_mais[mai_i];
		if (mai_info.mai_id == 0)
		{
			continue;
		}

		PulseTable *mai_config = get_config_by_id(mai_info.mai_id, &yuqidao_jingmai_config);
		if (!mai_config)
		{
			continue;
		}

		for (uint32_t acupoint_id = mai_config->AcupunctureType; mai_info.acupoint_id == 0 || acupoint_id <= mai_info.acupoint_id; acupoint_id++)
		{
			AcupunctureTable *acupoint_config = get_config_by_id(acupoint_id, &yuqidao_acupoint_config);
			if (!acupoint_config)
			{
				break;
			}

			double percent = 1.0;
			if (acupoint_id == mai_info.acupoint_id)
			{
				percent = (double)mai_info.fill_lv / (double)acupoint_config->GradeNum;
			}

			for (int attr_i = 0; attr_i < (int)acupoint_config->n_AcupunctureAttribute; ++attr_i)
			{
				attr[acupoint_config->AcupunctureAttribute[attr_i]] += acupoint_config->AttributeCeiling[attr_i] * percent;
			}
		}
	}

	//冲脉属性
	for (int break_i = 0; break_i < MAX_YUQIDAO_BREAK_NUM; ++break_i)
	{
		YuqidaoBreakInfo &break_info = data->yuqidao_breaks[break_i];
		if (break_info.id == 0)
		{
			continue;
		}

		BreakTable *break_config = get_config_by_id(break_info.id, &yuqidao_break_config);
		if (!break_config)
		{
			continue;
		}

		for (int j = 0; j < MAX_YUQIDAO_BREAK_ATTR_NUM && j < (int)break_config->n_PulseAttribute; ++j)
		{
			attr[break_config->PulseAttribute[j]] += break_info.cur_val[j];
		}
	}

	print_attribute("yuqidao", attr);
}

void player_struct::calcu_baguapai_attr(double *attr)
{
	memset(attr, 0, sizeof(double) * PLAYER_ATTR_MAX);

	uint32_t dress_id = get_attr(PLAYER_ATTR_BAGUA);
	BaguapaiDressInfo *dress_info = get_baguapai_dress(dress_id);
	if (!dress_info)
	{
		LOG_ERR("[%s:%d] player[%lu] get baguapai dress failed, dress_id:%u", __FUNCTION__, __LINE__, data->player_id, dress_id);
		return ;
	}

	std::map<uint32_t, std::vector<uint32_t> > suit_map;
	//卡牌属性
	for (int i = 0; i < MAX_BAGUAPAI_DRESS_NUM; ++i)
	{
		BaguapaiCardInfo *card_info = &dress_info->card_list[i];
		double star_addn_rate = 0.0;
		BaguaStarTable *star_config = get_bagua_star_config(card_info->star);
		if (star_config)
		{
			star_addn_rate = star_config->AttributeValue;
		}

		//主属性
		BaguaTable *card_config = get_config_by_id(card_info->id, &bagua_config);
		if (card_config)
		{
			attr[card_config->PrimaryAttributeType] += card_info->main_attr_val * (1.0 + star_addn_rate);

			uint32_t suit_id = card_config->Suit;
			suit_map[suit_id].push_back(card_info->star);
		}

		//副属性
		for (int j = 0; j < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++j)
		{
			AttrInfo *minor_attr = &card_info->minor_attrs[j];
			if (minor_attr->id > 0)
			{
				attr[minor_attr->id] += minor_attr->val;
			}
		}
	}

	//清除旧的buff
	for (std::vector<uint64_t>::iterator iter = bagua_buffs.begin(); iter != bagua_buffs.end(); ++iter)
	{
		uint64_t buff_id = *iter;
		buff_struct *buff = NULL;
		for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
		{
			if (m_buffs[i] != NULL && m_buffs[i]->data != NULL && m_buffs[i]->data->buff_id == (uint32_t)buff_id)
			{
				buff = m_buffs[i];
			}
		}

		if (buff)
		{
			buff->del_buff();
			buff = NULL;
		}
	}
	bagua_buffs.clear();

	//套装属性
	for (std::map<uint32_t, std::vector<uint32_t> >::iterator iter = suit_map.begin(); iter != suit_map.end(); ++iter)
	{
		uint32_t suit_id = iter->first;
		std::vector<uint32_t> &star_vec = iter->second;
		uint32_t suit_num = star_vec.size();
		uint32_t min_star = 0;
		for (uint32_t i = 0; i < suit_num; ++i)
		{
			if (i == 0)
			{
				min_star = star_vec[i];
			}
			else
			{
				min_star = std::min(min_star, star_vec[i]);
			}
		}

		BaguaSuitTable *suit_config = get_config_by_id(suit_id, &bagua_suit_config);
		if (suit_config)
		{
			for (size_t i = 0; i < suit_config->n_SuitNum; ++i)
			{
				uint32_t need_num = suit_config->SuitNum[i];
				uint32_t effect_id = suit_config->SuitAttributeType[i];
				uint32_t attr_id = suit_config->Classification[i];
				double attr_val = suit_config->AttributeValue[i];
				double star_suit_addn = 0;
				switch (min_star)
				{
					case 1:
						star_suit_addn = suit_config->SuitPlus1[i];
						break;
					case 2:
						star_suit_addn = suit_config->SuitPlus2[i];
						break;
					case 3:
						star_suit_addn = suit_config->SuitPlus3[i];
						break;
					case 4:
						star_suit_addn = suit_config->SuitPlus4[i];
						break;
					case 5:
						star_suit_addn = suit_config->SuitPlus5[i];
						break;
					case 6:
						star_suit_addn = suit_config->SuitPlus6[i];
						break;
				}

				if (suit_num >= need_num)
				{
					if (effect_id == 1)
					{ //属性
						attr[attr_id] += attr_val;
						if (suit_num == MAX_BAGUAPAI_DRESS_NUM && min_star > 0)
						{ //炼星套装加成
							attr[attr_id] += star_suit_addn;
						}
					}
					else if (effect_id == 2)
					{ //BUFF
						uint32_t buff_id = 0;
						if (suit_num == MAX_BAGUAPAI_DRESS_NUM && min_star > 0)
						{
							buff_id = star_suit_addn;
						}
						else
						{
							buff_id = attr_id;
						}
						bagua_buffs.push_back(buff_id);
						buff_struct *buff = buff_manager::create_default_buff(buff_id, this, this, true);
						if (!buff)
						{
							LOG_ERR("[%s:%d] player[%lu] create buff failed, create_id:%u", __FUNCTION__, __LINE__, data->player_id, buff_id);
						}
					}
				}
			}
		}
	}

	print_attribute("baguapai", attr);
}

void player_struct::calcu_guild_skill_attr(double *attr)
{
	memset(attr, 0, sizeof(double) * PLAYER_ATTR_MAX);
	memcpy(attr, data->guild_skill_attr, sizeof(double) * PLAYER_ATTR_FIGHT_MAX);

	print_attribute("guild_skill", attr);
}

void player_struct::calcu_fashion_attr(double *attr)
{
	memset(attr, 0, sizeof(double) * PLAYER_ATTR_MAX);
	CharmTable *table = get_charm_table(data->charm_level);
	if (table == NULL)
	{
		return;
	}
	for (uint32_t i = 0; i < table->n_AttributeType; ++i)
	{
		attr[table->AttributeType[i]] = table->AttributeNum[i];
	}
}

void player_struct::calcu_partner_attr(double *attr)
{
	memset(attr, 0, sizeof(double) * PLAYER_ATTR_MAX);

	for (int i = 0; i < MAX_PARTNER_BATTLE_NUM; ++i)
	{
		uint64_t partner_uuid = data->partner_battle[i];
		if (partner_uuid == 0)
		{
			continue;
		}

		partner_struct *partner = get_partner_by_uuid(partner_uuid);
		if (partner && partner->scene)
		{
			do
			{
				SkillTable *main_config = get_config_by_id(partner->config->ProtectSkill, &skill_config);
				if (!main_config)
				{
					break;
				}

				uint64_t fc = partner->get_attr(PLAYER_ATTR_FIGHTING_CAPACITY);
				uint32_t lv_id = main_config->SkillLv;
				uint32_t effect_id = 0;
				while (true)
				{
					SkillLvTable *lv_config = get_config_by_id(lv_id, &skill_lv_config);
					if (!lv_config)
					{
						break;
					}

					if (fc < lv_config->NeedFight)
					{
						break;
					}

					if (lv_config->n_EffectIdFriend > 0)
					{
						effect_id = lv_config->EffectIdFriend[0];
					}

					lv_id++;
				}

				SkillEffectTable *effect_config = get_config_by_id(effect_id, &skill_effect_config);
				if (!effect_config)
				{
					break;
				}

				if (effect_config->Type != 170000008)
				{
					break;
				}

				for (uint32_t j = 0; j < effect_config->n_Effect; ++j)
				{
					attr[effect_config->Effect[j]] += effect_config->EffectNum[j];
				}
			} while(0);
		}
	}

	print_attribute("partner", attr);
}

uint32_t player_struct::get_partner_fc(void)
{
	if (!is_partner_battle())
	{
		return 0;
	}

	uint32_t fc = 0;
	for (int i = 0; i < MAX_PARTNER_FORMATION_NUM; ++i)
	{
		if (data->partner_formation[i] == 0)
		{
			continue;
		}

		partner_struct *partner = get_partner_by_uuid(data->partner_formation[i]);
		if (!partner)
		{
			continue;
		}

		if (!partner->is_alive())
		{
			continue;
		}

		double percent = (i == 0 ? 1.0 : sg_partner_assist_percent);
		fc += partner->get_attr(PLAYER_ATTR_FIGHTING_CAPACITY) * percent;
	}

	return fc;
}

char *player_struct::get_name()
{
	return &data->name[0];
}

void player_struct::set_qiecuo(uint32_t pos_x, uint32_t pos_z, uint64_t target)
{
	data->qiecuo_start_time = time_helper::get_cached_time();
	data->qiecuo_target = target;

	data->qiecuo_pos.pos_x = pos_x;
	data->qiecuo_pos.pos_z = pos_z;
	data->qiecuo_out_range_fail_time = 0;
}
void player_struct::finish_qiecuo()
{
	data->qiecuo_start_time = 0;
	data->qiecuo_target = 0;
}

//1表示P1胜利，2表示P2胜利，3表示平局，0表示无结果
int check_qiecuo_finished(player_struct *p1, player_struct *p2)
{
	if (!p1->is_in_qiecuo() || !p1->is_qiecuo_target(p2))
		return (0);

	int ret = 0;
	if (!p1->is_alive())
	{
		p1->set_attr(PLAYER_ATTR_HP, 1);
		p1->on_hp_changed(0);
		ret += 2;
	}

	if (!p2->is_alive())
	{
		p2->set_attr(PLAYER_ATTR_HP, 1);
		p2->on_hp_changed(0);
		ret += 1;
	}

	if (ret != 0)
	{
		LOG_INFO("%s: player[%lu][%lu] qiecuo result %d", __FUNCTION__, p1->get_uuid(), p2->get_uuid(), ret);

		p1->clear_debuff();
		p2->clear_debuff();
		p1->finish_qiecuo();
		p2->finish_qiecuo();

		QiecuoFinishNotify nty;
		EXTERN_DATA ext;
		qiecuo_finish_notify__init(&nty);

		switch (ret)
		{
			case 1:
				nty.result = 0;
				ext.player_id = p1->get_uuid();
				fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_FINISH_NOTIFY, qiecuo_finish_notify__pack, nty);

				nty.result = 1;
				ext.player_id = p2->get_uuid();
				fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_FINISH_NOTIFY, qiecuo_finish_notify__pack, nty);
				break;
			case 2:
				nty.result = 1;
				ext.player_id = p1->get_uuid();
				fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_FINISH_NOTIFY, qiecuo_finish_notify__pack, nty);

				nty.result = 0;
				ext.player_id = p2->get_uuid();
				fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_FINISH_NOTIFY, qiecuo_finish_notify__pack, nty);
				break;
			case 3:
				nty.result = 2;
				ext.player_id = p1->get_uuid();
				fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_FINISH_NOTIFY, qiecuo_finish_notify__pack, nty);
				ext.player_id = p2->get_uuid();
				fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_FINISH_NOTIFY, qiecuo_finish_notify__pack, nty);
				break;
		}
	}


	return (ret);
}

void xunbao_drop(player_struct &player, uint32_t itemid)
{
	SearchTable *sTable = get_config_by_id(itemid, &sg_xunbao);
	if (sTable == NULL)
	{
		return;
	}
	std::map<uint32_t, uint32_t>::iterator it = player.xun_map_id.find(itemid);
	if (it == player.xun_map_id.end())
	{
		return;
	}
	player.xun_map_id.erase(it);
	TreasureTable *tTable = get_config_by_id(it->second, &xunbao_map_config);
	if (tTable == NULL)
	{
		return;
	}
	float x = tTable->PointX / 100;
	float y = tTable->PointY / 100;
	float z = tTable->PointZ / 100;
	uint32_t r = rand() % 10000;
	uint32_t all = 0;
	uint32_t event = 0;
	uint32_t i = 0;
	for (; i < sTable->n_Probability; ++i)
	{
		all += sTable->Probability[i];
		if (r < all)
		{
			event = sTable->Event[i];
			break;;
		}
	}
	EXTERN_DATA extern_data;
	extern_data.player_id = player.get_uuid();
	//系统提示
	std::vector<char *> args;
	switch (event)
	{
	case 1:
		player.give_drop_item(sTable->Parameter1[i], MAGIC_TYPE_XUNBAO, ADD_ITEM_SEND_MAIL_WHEN_BAG_FULL);

		player.data->xunbao.send_next = true;
		break;
	case 2:
	{
		CollectTable *tCollet = get_config_by_id(sTable->Parameter1[i], &collect_config);
		if (tCollet != NULL)
		{
			args.push_back(tCollet->NameId);
			player.send_system_notice(190500286, &args);
			Collect *pCollect = Collect::CreateCollectByPos(player.scene, sTable->Parameter1[i], x, y, z, 0);
			if (pCollect != NULL)
			{
				pCollect->m_minType = 1;
			}
		}
	}
		break;
	case 4:
	{
		player.send_system_notice(190500288, &args);
		monster_struct *mon = monster_manager::create_monster_at_pos(player.scene, sTable->Parameter1[i], player.get_attr(PLAYER_ATTR_LEVEL), x, z, 0, NULL);
		if (mon != NULL)
		{
			mon->data->owner = player.get_uuid();

			ParameterTable * config = get_config_by_id(161000221, &parameter_config);
			char str[512] = "bbbbbbbbbbbbbbbbbbbbbbbbb";
			Chat send;
			chat__init(&send);
			if (config != NULL)
			{
				sprintf(str, config->parameter2, player.scene->res_config->SceneName);
			}
			send.contain = str;
			send.channel = CHANNEL__world;
			send.sendname = player.data->name;
			send.sendplayerid = player.get_uuid();
			send.sendplayerlv = player.get_attr(PLAYER_ATTR_LEVEL);
			send.sendplayerjob = player.get_attr(PLAYER_ATTR_JOB);
			conn_node_gamesrv::send_to_all_player(MSG_ID_CHAT_NOTIFY, (void *)&send, (pack_func)chat__pack);
		}
	}
	break;
	case 3:
	{
		player.send_system_notice(190500287, &args);
		if (!player.sight_space)
		{
			player.sight_space = sight_space_manager::create_sight_space(&player, 1);
		}
		for (uint32_t num = 0; num < sTable->Parameter2[i]; ++num)
		{
			monster_manager::create_sight_space_monster(player.sight_space, player.scene, sTable->Parameter1[i], player.get_attr(PLAYER_ATTR_LEVEL), x + sTable->Parameter2[i] - rand()% 2*sTable->Parameter2[i], z + sTable->Parameter2[i] - rand() % 2*sTable->Parameter2[i]);
		}

	}
		break;
	case 5:
	{
		SightNpcInfo send;
		sight_npc_info__init(&send);
		send.npcid = sTable->Parameter1[i];
		PosData pos;
		pos_data__init(&pos);
		pos.pos_x = x;
		pos.pos_z = z;
		send.data = &pos;
		send.cd = 1800;

		player.data->xunbao.cd = time_helper::get_cached_time() / 1000 + 1800;
		player.data->xunbao.door_map = player.data->scene_id;
		player.data->xunbao.door_x = x;
		player.data->xunbao.door_z = z;
		player.data->xunbao.door_y = y;
		send.y = player.data->xunbao.door_y;
		player.data->xunbao.door_id = sTable->Parameter1[i];

		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ADD_NPC_NOTIFY, sight_npc_info__pack, send);


		player.send_system_notice(190500290, &args);

		ParameterTable * config = get_config_by_id(161000222, &parameter_config);
		char str[512] = "aaaaaaaaaaaaaaaaaaa";
		Chat send1;
		chat__init(&send1);
		if (config != NULL)
		{
			sprintf(str, config->parameter2, player.scene->res_config->SceneName);
		}
		send1.contain = str;
		send1.channel = CHANNEL__world;
		send1.sendname = player.data->name;
		send1.sendplayerid = player.get_uuid();
		send1.sendplayerlv = player.get_attr(PLAYER_ATTR_LEVEL);
		send1.sendplayerjob = player.get_attr(PLAYER_ATTR_JOB);
		conn_node_gamesrv::send_to_all_player(MSG_ID_CHAT_NOTIFY, (void *)&send1, (pack_func)chat__pack);
	}
		break;
	}

	player.check_activity_progress(AM_XUNBAO, 0);
	player.add_task_progress(TCT_XUNBAO, 0, 1);
}

bool player_struct::is_qiecuo_target(player_struct *target)
{
	if (!is_in_qiecuo() || !target->is_in_qiecuo())
		return false;
	if (data->qiecuo_target != target->get_uuid())
		return false;
	assert(target->data->qiecuo_target == get_uuid());

	if (time_helper::get_cached_time() - data->qiecuo_start_time < sg_qiecuo_god_time)
		return false;

	return true;
}

bool player_struct::is_in_qiecuo()
{
	if (data->qiecuo_start_time == 0)
		return false;
	return true;
}

void player_struct::print_attribute(const char * stype, double *attr)
{
	return ;
	for (uint32_t i = 1; i < PLAYER_ATTR_FIGHT_MAX; ++i)
	{
		LOG_DEBUG("[%s:%d] player[%lu] %s attr %u : %f", __FUNCTION__, __LINE__, data->player_id, stype, i, attr[i]);
	}
}


int player_struct::get_comm_gold(void) //获取绑定+非绑定元宝
{
	return (data->attrData[PLAYER_ATTR_GOLD] + data->attrData[PLAYER_ATTR_BIND_GOLD]);
}

int player_struct::add_unbind_gold(uint32_t num, uint32_t statis_id, bool isNty) //增加非绑定元宝
{
	if (num == 0)
	{
		return 0;
	}

	uint32_t prevVal = data->attrData[PLAYER_ATTR_GOLD];
	if (prevVal >= (uint32_t)MAX_MONEY_VALUE)
	{
		return 0;
	}

	data->attrData[PLAYER_ATTR_GOLD] = std::min(prevVal + num, (uint32_t)MAX_MONEY_VALUE);
	uint32_t curVal = data->attrData[PLAYER_ATTR_GOLD];
	uint32_t realNum = curVal - prevVal;
	LOG_INFO("[%s:%d] player[%lu] prevVal:%u, curVal:%u, num:%u", __FUNCTION__, __LINE__, data->player_id, prevVal, curVal, num);

	this->add_task_progress(TCT_BASIC, TBC_GOLD, get_comm_gold());
	if (isNty)
	{
		AttrMap attrs;
		attrs[PLAYER_ATTR_GOLD] = data->attrData[PLAYER_ATTR_GOLD];
		this->notify_attr(attrs);
	}

	//系统提示
	std::vector<char *> args;
	std::stringstream ss_num;
	ss_num << realNum;
	args.push_back(const_cast<char*>(ss_num.str().c_str()));
	uint32_t notice_id = SNT_ADD_GOLD_TEXT;
	send_system_notice(notice_id, &args);

	if (realNum > 0)
	{
		refresh_player_redis_info();
	}

	return 0;
}

int player_struct::add_bind_gold(uint32_t num, uint32_t statis_id, bool isNty)
{
	if (num == 0)
	{
		return 0;
	}

	uint32_t prevVal = data->attrData[PLAYER_ATTR_BIND_GOLD];
	if (prevVal >= (uint32_t)MAX_MONEY_VALUE)
	{
		return 0;
	}

	data->attrData[PLAYER_ATTR_BIND_GOLD] = std::min(prevVal + num, (uint32_t)MAX_MONEY_VALUE);
	uint32_t curVal = data->attrData[PLAYER_ATTR_BIND_GOLD];
	uint32_t realNum = curVal - prevVal;
	LOG_INFO("[%s:%d] player[%lu] prevVal:%u, curVal:%u, num:%u", __FUNCTION__, __LINE__, data->player_id, prevVal, curVal, num);

	this->add_task_progress(TCT_BASIC, TBC_GOLD, get_comm_gold());
	if (isNty)
	{
		AttrMap attrs;
		attrs[PLAYER_ATTR_BIND_GOLD] = data->attrData[PLAYER_ATTR_BIND_GOLD];
		this->notify_attr(attrs);
	}

	//系统提示
	std::vector<char *> args;
	std::stringstream ss_num;
	ss_num << realNum;
	args.push_back(const_cast<char*>(ss_num.str().c_str()));
	uint32_t notice_id = SNT_ADD_GOLD_TEXT;
	send_system_notice(notice_id, &args);

	if (realNum > 0)
	{
		refresh_player_redis_info();
	}

	return 0;
}

int player_struct::sub_unbind_gold(uint32_t num, uint32_t statis_id, bool isNty)
{
	if (num == 0)
	{
		return 0;
	}

	uint32_t prevGold = data->attrData[PLAYER_ATTR_GOLD];
	if (prevGold < num)
	{
		LOG_ERR("[%s:%d] player[%lu] unbind gold not enough, prevGold:%u, num:%u", __FUNCTION__, __LINE__, data->player_id, prevGold, num);
		return -1;
	}

	data->attrData[PLAYER_ATTR_GOLD] -= num;
	uint32_t curGold = data->attrData[PLAYER_ATTR_GOLD];
	LOG_INFO("[%s:%d] player[%lu] unbind gold, prevVal:%u, curVal:%u, num:%u", __FUNCTION__, __LINE__, data->player_id,  prevGold, curGold, num);

	this->add_task_progress(TCT_BASIC, TBC_GOLD, get_comm_gold());
	if (isNty)
	{
		AttrMap attrs;
		attrs[PLAYER_ATTR_GOLD] = data->attrData[PLAYER_ATTR_GOLD];
		this->notify_attr(attrs);
	}

	refresh_player_redis_info();

	return 0;
}

int player_struct::sub_comm_gold(uint32_t num, uint32_t statis_id, bool isNty)
{
	if (num == 0)
	{
		return 0;
	}

	uint32_t prevBindGold = data->attrData[PLAYER_ATTR_BIND_GOLD];
	uint32_t prevGold = data->attrData[PLAYER_ATTR_GOLD];
	if (prevBindGold + prevGold < num)
	{
		LOG_ERR("[%s:%d] player[%lu] common gold not enough, prevBindGold:%u, prevGold:%u, num:%u", __FUNCTION__, __LINE__, data->player_id, prevBindGold, prevGold, num);
		return -1;
	}

	AttrMap attrs;
	if (prevBindGold >= num)
	{
		data->attrData[PLAYER_ATTR_BIND_GOLD] -= num;
		uint32_t curBindGold = data->attrData[PLAYER_ATTR_BIND_GOLD];
		LOG_INFO("[%s:%d] player[%lu] bind gold, prevVal:%u, curVal:%u, num:%u", __FUNCTION__, __LINE__, data->player_id,  prevBindGold, curBindGold, num);

		attrs[PLAYER_ATTR_BIND_GOLD] = data->attrData[PLAYER_ATTR_BIND_GOLD];
	}
	else
	{
		uint32_t left_num = num;
		if (prevBindGold > 0)
		{
			left_num = num - prevBindGold;
			data->attrData[PLAYER_ATTR_BIND_GOLD] = 0;
			LOG_INFO("[%s:%d] player[%lu] bind gold, prevVal:%u, curVal:%u, num:%u", __FUNCTION__, __LINE__, data->player_id,  prevBindGold, 0, prevBindGold);
			attrs[PLAYER_ATTR_BIND_GOLD] = 0;
		}

		data->attrData[PLAYER_ATTR_GOLD] -= left_num;
		uint32_t curGold = data->attrData[PLAYER_ATTR_GOLD];
		LOG_INFO("[%s:%d] player[%lu] gold, prevVal:%u, curVal:%u, num:%u", __FUNCTION__, __LINE__, data->player_id,  prevGold, curGold, left_num);
		attrs[PLAYER_ATTR_GOLD] = data->attrData[PLAYER_ATTR_GOLD];
	}

	this->add_task_progress(TCT_BASIC, TBC_GOLD, get_comm_gold());
	if (isNty)
	{
		this->notify_attr(attrs);
	}

	refresh_player_redis_info();

	return 0;
}

int player_struct::add_coin(uint32_t num, uint32_t statis_id, bool isNty)
{
	if (num == 0)
	{
		return 0;
	}

	uint32_t prevVal = data->attrData[PLAYER_ATTR_COIN];
	if (prevVal >= (uint32_t)MAX_MONEY_VALUE)
	{
		return 0;
	}

	data->attrData[PLAYER_ATTR_COIN] = std::min(prevVal + num, (uint32_t)MAX_MONEY_VALUE);
	uint32_t curVal = data->attrData[PLAYER_ATTR_COIN];
	uint32_t realNum = curVal - prevVal;
	LOG_INFO("[%s:%d] player[%lu] prevVal:%u, curVal:%u, num:%u", __FUNCTION__, __LINE__, data->player_id, prevVal, curVal, num);

	this->add_task_progress(TCT_BASIC, TBC_COIN, curVal);
	if (isNty)
	{
		AttrMap attrs;
		attrs[PLAYER_ATTR_COIN] = data->attrData[PLAYER_ATTR_COIN];
		this->notify_attr(attrs);

		//系统提示
		std::vector<char *> args;
		std::stringstream ss_num;
		ss_num << realNum;
		args.push_back(const_cast<char*>(ss_num.str().c_str()));
		uint32_t notice_id = (notice_use_art(statis_id) ? SNT_ADD_COIN_ART : SNT_ADD_COIN_TEXT);
		send_system_notice(notice_id, &args);
	}

	if (realNum > 0)
	{
		refresh_player_redis_info();
	}

	return 0;
}

int player_struct::sub_coin(uint32_t num, uint32_t statis_id, bool isNty)
{
	if (num == 0)
	{
		return 0;
	}

	uint32_t prevVal = data->attrData[PLAYER_ATTR_COIN];
	if (prevVal < num)
	{
		LOG_ERR("[%s:%d] player[%lu] coin not enough, prevVal:%u, num:%u", __FUNCTION__, __LINE__, data->player_id, prevVal, num);
		return -1;
	}

	data->attrData[PLAYER_ATTR_COIN] -= num;
	uint32_t curVal = data->attrData[PLAYER_ATTR_COIN];
	LOG_INFO("[%s:%d] player[%lu] prevVal:%u, curVal:%u, num:%u", __FUNCTION__, __LINE__, data->player_id, prevVal, curVal, num);

	this->add_task_progress(TCT_BASIC, TBC_COIN, curVal);
	if (isNty)
	{
		AttrMap attrs;
		attrs[PLAYER_ATTR_COIN] = data->attrData[PLAYER_ATTR_COIN];
		this->notify_attr(attrs);
	}

	refresh_player_redis_info();

	return 0;
}

int player_struct::add_chengjie_coin(uint32_t num, uint32_t statis_id, bool isNty)
{
	if (num == 0)
	{
		return 0;
	}

	data->attrData[PLAYER_ATTR_CHENGJIE_COIN] += num;

	if (isNty)
	{
		AttrMap attrs;
		attrs[PLAYER_ATTR_CHENGJIE_COIN] = data->attrData[PLAYER_ATTR_CHENGJIE_COIN];
		this->notify_attr(attrs);

		std::vector<char *> args;
		std::stringstream ss_num;
		ss_num << num;
		args.push_back(const_cast<char*>(ss_num.str().c_str()));
		send_system_notice(190500170, &args);
	}


	return 0;
}
int player_struct::sub_chengjie_coin(uint32_t num, uint32_t statis_id, bool isNty)
{
	if (num == 0)
	{
		return 0;
	}

	if (data->attrData[PLAYER_ATTR_CHENGJIE_COIN] < num)
	{
		return -1;
	}

	data->attrData[PLAYER_ATTR_CHENGJIE_COIN] -= num;

	if (isNty)
	{
		AttrMap attrs;
		attrs[PLAYER_ATTR_CHENGJIE_COIN] = data->attrData[PLAYER_ATTR_CHENGJIE_COIN];
		this->notify_attr(attrs);
	}

	return 0;
}
int player_struct::add_guoyu_coin(uint32_t num, uint32_t statis_id, bool isNty)
{
	if (num == 0)
	{
		return 0;
	}

	data->attrData[PLAYER_ATTR_GUOYU_COIN] += num;

	if (isNty)
	{
		AttrMap attrs;
		attrs[PLAYER_ATTR_GUOYU_COIN] = data->attrData[PLAYER_ATTR_GUOYU_COIN];
		this->notify_attr(attrs);

		std::vector<char *> args;
		std::stringstream ss_num;
		ss_num << num;
		args.push_back(const_cast<char*>(ss_num.str().c_str()));
		send_system_notice(190500169, &args);
	}


	return 0;
}
int player_struct::sub_guoyu_coin(uint32_t num, uint32_t statis_id, bool isNty)
{
	if (num == 0)
	{
		return 0;
	}

	if (data->attrData[PLAYER_ATTR_GUOYU_COIN] < num)
	{
		return -1;
	}

	data->attrData[PLAYER_ATTR_GUOYU_COIN] -= num;

	if (isNty)
	{
		AttrMap attrs;
		attrs[PLAYER_ATTR_GUOYU_COIN] = data->attrData[PLAYER_ATTR_GUOYU_COIN];
		this->notify_attr(attrs);
	}

	return 0;
}
int player_struct::add_shangjin_coin(uint32_t num, uint32_t statis_id, bool isNty)
{
	if (num == 0)
	{
		return 0;
	}

	data->attrData[PLAYER_ATTR_SHANGJIN_COIN] += num;

	if (isNty)
	{
		AttrMap attrs;
		attrs[PLAYER_ATTR_SHANGJIN_COIN] = data->attrData[PLAYER_ATTR_SHANGJIN_COIN];
		this->notify_attr(attrs);

		std::vector<char *> args;
		std::stringstream ss_num;
		ss_num << num;
		args.push_back(const_cast<char*>(ss_num.str().c_str()));
		send_system_notice(190500171, &args);
	}


	return 0;
}
int player_struct::sub_shangjin_coin(uint32_t num, uint32_t statis_id, bool isNty)
{
	if (num == 0)
	{
		return 0;
	}

	if (data->attrData[PLAYER_ATTR_SHANGJIN_COIN] < num)
	{
		return -1;
	}

	data->attrData[PLAYER_ATTR_SHANGJIN_COIN] -= num;

	if (isNty)
	{
		AttrMap attrs;
		attrs[PLAYER_ATTR_SHANGJIN_COIN] = data->attrData[PLAYER_ATTR_SHANGJIN_COIN];
		this->notify_attr(attrs);
	}

	return 0;
}

uint32_t player_struct::get_coin(void)
{
	return (data ? data->attrData[PLAYER_ATTR_COIN] : 0);
}

int player_struct::add_zhenqi(uint32_t num, uint32_t statis_id, bool isNty)
{
	if (num == 0)
	{
		return 0;
	}

	uint32_t prevVal = data->attrData[PLAYER_ATTR_ZHENQI];
	if (prevVal >= (uint32_t)MAX_MONEY_VALUE)
	{
		return 0;
	}

	data->attrData[PLAYER_ATTR_ZHENQI] = std::min(prevVal + num, (uint32_t)MAX_MONEY_VALUE);
	uint32_t curVal = data->attrData[PLAYER_ATTR_ZHENQI];
//	uint32_t realNum = curVal - prevVal;
	LOG_INFO("[%s:%d] player[%lu] prevVal:%u, curVal:%u, num:%u", __FUNCTION__, __LINE__, data->player_id, prevVal, curVal, num);

	if (isNty)
	{
		AttrMap attrs;
		attrs[PLAYER_ATTR_ZHENQI] = data->attrData[PLAYER_ATTR_ZHENQI];
		this->notify_attr(attrs);
	}

	return 0;
}

int player_struct::sub_zhenqi(uint32_t num, uint32_t statis_id, bool isNty)
{
	if (num == 0)
	{
		return 0;
	}

	uint32_t prevVal = data->attrData[PLAYER_ATTR_ZHENQI];
	if (prevVal < num)
	{
		LOG_ERR("[%s:%d] player[%lu] zhenqi not enough, prevVal:%u, num:%u", __FUNCTION__, __LINE__, data->player_id, prevVal, num);
		return -1;
	}

	data->attrData[PLAYER_ATTR_ZHENQI] -= num;
	uint32_t curVal = data->attrData[PLAYER_ATTR_ZHENQI];
	LOG_INFO("[%s:%d] player[%lu] prevVal:%u, curVal:%u, num:%u", __FUNCTION__, __LINE__, data->player_id, prevVal, curVal, num);

	if (isNty)
	{
		AttrMap attrs;
		attrs[PLAYER_ATTR_ZHENQI] = data->attrData[PLAYER_ATTR_ZHENQI];
		this->notify_attr(attrs);
	}

	return 0;
}

uint32_t player_struct::get_zhenqi(void)
{
	return (data ? data->attrData[PLAYER_ATTR_ZHENQI] : 0);
}

void player_struct::item_find_pos_by_cache(uint32_t id, std::vector<uint32_t>& pos_list)
{
	std::pair<ItemPosMap::iterator, ItemPosMap::iterator> range;
	range = item_pos_cache.equal_range(id);

	for (ItemPosMap::iterator iter = range.first; iter != range.second; ++iter)
	{
		if (iter->first == id)
		{
			pos_list.push_back(iter->second);
		}
	}
}

void player_struct::add_item_pos_cache(uint32_t id, uint32_t pos)
{
	item_pos_cache.insert(std::make_pair(id, pos));
}

void player_struct::del_item_pos_cache(uint32_t id, uint32_t pos)
{
	std::pair<ItemPosMap::iterator, ItemPosMap::iterator> range;
	range = item_pos_cache.equal_range(id);
	for (ItemPosMap::iterator iter = range.first; iter != range.second; ++iter)
	{
		if (iter->second == pos)
		{
			item_pos_cache.erase(iter);
			break;
		}
	}
}

void player_struct::create_item_cache(void)
{
	item_pos_cache.clear();
	for (uint32_t i = 0; i < data->bag_grid_num; ++i)
	{
		if (data->bag[i].id == 0)
		{
			continue;
		}

		this->add_item_pos_cache(data->bag[i].id, i);
	}
}

int player_struct::check_can_transfer()
{
	if (is_in_raid())
	{
		return ERROR_ID_CAN_NOT_TRANSFER;
	}

	if (sight_space)
	{
		return ERROR_ID_CAN_NOT_TRANSFER;
	}

	if (data->truck.truck_id != 0)
	{
		return 190500305;
	}
	return (0);
}

void player_struct::fit_bag_grid_num(void)
{
	ActorLevelTable *level_config = get_actor_level_config(data->attrData[PLAYER_ATTR_JOB], data->attrData[PLAYER_ATTR_LEVEL]);
	if (!level_config)
	{
		return ;
	}

	data->bag_grid_num = level_config->FreeGrid + data->bag_unlock_num;
}

uint32_t player_struct::set_item_cd(ItemsConfigTable *config)
{
	uint32_t cd = config->ItemCD;
	if (cd == 0)
		return (0);
	cd += time_helper::get_cached_time() / 1000;
	switch (config->ItemEffect)
	{
		case IUE_ADD_HP:
			set_attr(PLAYER_ATTR_ITEM_HP_CD, cd);
			break;
		case IUE_ADD_HP_POOL:
			set_attr(PLAYER_ATTR_ITEM_HP_POOL_CD, cd);
			break;
		default:
			return (0);
	}
	return (cd);
}

int player_struct::check_item_cd(ItemsConfigTable *config)
{
	uint32_t cd;
	switch (config->ItemEffect)
	{
		case IUE_ADD_HP:
			cd = get_attr(PLAYER_ATTR_ITEM_HP_CD);
			break;
		case IUE_ADD_HP_POOL:
			cd = get_attr(PLAYER_ATTR_ITEM_HP_POOL_CD);
			break;
		default:
			return (0);
	}
	if (time_helper::get_cached_time() / 1000 < cd)
		return (-1);
	return (0);
}

bool player_struct::check_can_add_item(uint32_t id, uint32_t num, std::map<uint32_t, uint32_t> *out_add_list)
{
	if (get_item_type(id) != ITEM_TYPE_ITEM)
	{
		return true;
	}

	uint32_t stack_num = get_item_stack_num(id);
	std::vector<uint32_t> pos_list;
	item_find_pos_by_cache(id, pos_list);

	uint32_t tmp_num = num;
	for (std::vector<uint32_t>::iterator iter = pos_list.begin(); iter != pos_list.end() && tmp_num > 0; ++iter)
	{
		bag_grid_data *grid = &data->bag[*iter];
		if (grid->id != id)
		{
			continue;
		}

		int can_put = stack_num - grid->num;
		if (can_put <= 0)
		{
			continue;
		}

		uint32_t put_num = ((uint32_t)can_put > tmp_num ? tmp_num : can_put);
		tmp_num -= put_num;
		if (out_add_list)
		{
			(*out_add_list)[*iter] = put_num;
		}
	}

	for (uint32_t i = 0; i < data->bag_grid_num && tmp_num > 0; ++i)
	{
		bag_grid_data *grid = &data->bag[i];
		if (grid->id > 0)
		{
			continue;
		}

		uint32_t put_num = (stack_num > tmp_num ? tmp_num : stack_num);
		tmp_num -= put_num;
		if (out_add_list)
		{
			(*out_add_list)[i] = put_num;
		}
	}

	if (tmp_num > 0)
	{
		return false;
	}

	return true;
}

bool player_struct::check_can_add_item_list(std::map<uint32_t, uint32_t>& item_list)
{
	std::map<uint32_t, uint32_t> stackable_map;
	uint32_t empty_num = 0;
	for (uint32_t i = 0; i < data->bag_grid_num; ++i)
	{
		bag_grid_data& grid = data->bag[i];
		if (grid.id > 0)
		{
			uint32_t stack_num = get_item_stack_num(grid.id);
			if (stack_num > grid.num)
			{
				stackable_map[grid.id] += stack_num - grid.num;
			}
		}
		else
		{
			empty_num++;
		}
	}

	//空格子足够
	if (item_list.size() <= empty_num)
	{
		return true;
	}

	for (std::map<uint32_t, uint32_t>::iterator iter = item_list.begin(); iter != item_list.end(); ++iter)
	{
		uint32_t item_id = iter->first;
		uint32_t item_num = iter->second;
		if (get_item_type(item_id) != ITEM_TYPE_ITEM)
		{
			continue;
		}

		std::map<uint32_t, uint32_t>::iterator iter_stack = stackable_map.find(item_id);
		if (iter_stack != stackable_map.end())
		{
			if (iter_stack->second > item_num)
			{
				item_num = 0;
				iter_stack->second -= item_num;
			}
			else
			{
				item_num -= iter_stack->second;
				stackable_map.erase(item_id);
			}
		}

		if (item_num > 0)
		{
			uint32_t stack_num = get_item_stack_num(item_id);
			uint32_t use_grid = ceil((double)item_num / stack_num);
			if (empty_num < use_grid)
			{
				return false;
			}

			empty_num -= use_grid;
			if (stack_num > stack_num * use_grid - item_num)
			{
				stackable_map[item_id] = stack_num * use_grid - item_num;
			}
		}
	}

	return true;
}

int player_struct::add_item(uint32_t id, uint32_t num, uint32_t statis_id, bool isNty)
{
	int ret = 0;
	switch (get_item_type(id))
	{
		case ITEM_TYPE_COIN: //银两
			{
				add_coin(num, statis_id, isNty);
			}
			break;
		case ITEM_TYPE_BIND_GOLD: //绑定元宝
			{
				add_bind_gold(num, statis_id, isNty);
			}
			break;
		case ITEM_TYPE_GUOYU_EXP:
			add_guoyu_exp(num);
			break;
		case ITEM_TYPE_CHENGJIE_EXP:
			add_chengjie_exp(num);
			break;
		case ITEM_TYPE_CHENGJIE_COIN:
		{
			add_chengjie_coin(num, statis_id, isNty);
		}
			break;
		case ITEM_TYPE_SHANGJIN_COIN:
		{
			add_shangjin_coin(num, statis_id, isNty);
		}
			break;
		case ITEM_TYPE_GUOYU_COIN:
		{
			add_guoyu_coin(num, statis_id, isNty);
		}
		break;
		case ITEM_TYPE_SHANGJIN_EXP:
		{
			add_shangjin_exp(num);
		}
			break;
		case ITEM_TYPE_GOLD: //元宝
			{
				add_unbind_gold(num, statis_id, isNty);
			}
			break;
		case ITEM_TYPE_EXP: //经验
			{
				add_exp(num, statis_id, isNty);
			}
			break;
		case ITEM_TYPE_GUILD_TREASURE:
			{
				add_guild_resource(2, num);
			}
			break;
		case ITEM_TYPE_GUILD_DONATION:
			{
				add_guild_resource(4, num);
			}
			break;
		case ITEM_TYPE_PARTNER_EXP:
			{
				add_partner_exp(num, statis_id, isNty);
			}
			break;
		case ITEM_TYPE_EQUIP:
			{
				ItemsConfigTable* config = get_config_by_id(id, &::item_config);
				if (!config)
				{
					LOG_ERR("[%s:%d] player[%lu] get item config failed, id:%u", __FUNCTION__, __LINE__, data->player_id, id);
					ret = ERROR_ID_NO_CONFIG;
					break;
				}

				if (config->n_ParameterEffect < 1)
				{
					LOG_ERR("[%s:%d] player[%lu] item config error, id:%u", __FUNCTION__, __LINE__, data->player_id, id);
					ret = ERROR_ID_NO_CONFIG;
					break;
				}

				add_equip(config->ParameterEffect[0], statis_id);
			}
			break;
		case ITEM_TYPE_ITEM: //普通道具
			{
				ItemsConfigTable* config = get_config_by_id(id, &::item_config);
				if (!config)
				{
					LOG_ERR("[%s:%d] player[%lu] get item config failed, id:%u", __FUNCTION__, __LINE__, data->player_id, id);
					ret = ERROR_ID_NO_CONFIG;
					break;
				}

				std::map<uint32_t, uint32_t> add_list;
				if (!check_can_add_item(id, num, &add_list))
				{
					LOG_ERR("[%s:%d] player[%lu] bag grid not enough, id:%u, num:%u", __FUNCTION__, __LINE__, data->player_id, id, num);
					ret = ERROR_ID_BAG_GRID_NOT_ENOUGH;
					break;
				}

				for (std::map<uint32_t, uint32_t>::iterator iter = add_list.begin(); iter != add_list.end(); ++iter)
				{
					bag_grid_data *grid = &data->bag[iter->first];
					if (grid->id > 0)
					{
						grid->num += iter->second;
					}
					else
					{
						grid->id = id;
						grid->num = iter->second;
						if (config->ItemLimit > 0) //时限道具
						{
							uint32_t now = time_helper::get_cached_time() / 1000;
							grid->expire_time = now + config->ItemLimit;
						}
						if (config->ItemType == 10) //八卦牌
						{
							if (config->n_ParameterEffect < 1)
							{
								LOG_ERR("[%s:%d] player[%lu] item config miss baguapai id, item_id:%u", __FUNCTION__, __LINE__, data->player_id, id);
							}
							else
							{
								uint32_t card_id = config->ParameterEffect[0];
								if (generate_baguapai_main_attr(card_id, grid->baguapai.main_attr_val) != 0)
								{
									LOG_ERR("[%s:%d] player[%lu] generate main attr failed, item_id:%u", __FUNCTION__, __LINE__, data->player_id, id);
								}
								if (generate_baguapai_minor_attr(card_id, grid->baguapai.minor_attrs) != 0)
								{
									LOG_ERR("[%s:%d] player[%lu] generate minor attr failed, item_id:%u", __FUNCTION__, __LINE__, data->player_id, id);
								}
							}
						}
						this->add_item_pos_cache(id, iter->first);
					}

					if (isNty)
					{
						this->update_bag_grid(iter->first);
					}
				}

				this->add_task_progress(TCT_CARRY_ITEM, id, num);

				//系统提示
				std::vector<char *> args;
				std::stringstream ss_id, ss_num;
				ss_id << id;
				ss_num << num;
				args.push_back(const_cast<char*>(ss_id.str().c_str()));
				args.push_back(const_cast<char*>(ss_num.str().c_str()));
				send_system_notice(SNT_ADD_ITEM_TEXT, &args);

				//道具飞背包
				switch (statis_id)
				{
					case MAGIC_TYPE_XUNBAO:
					case MAGIC_TYPE_GATHER:
					case MAGIC_TYPE_MONSTER_DEAD:
						notify_one_item_flow_to_bag(id, num);
						break;
				}
			}
			break;
	}

	return ret;
}

bool player_struct::add_item_list(std::map<uint32_t, uint32_t>& item_list, uint32_t statis_id, bool isNty)
{
	if (!check_can_add_item_list(item_list))
	{
		return false;
	}

	for (std::map<uint32_t, uint32_t>::iterator iter = item_list.begin(); iter != item_list.end(); ++iter)
	{
		add_item(iter->first, iter->second, statis_id, isNty);
	}

	return true;
}

bool player_struct::add_item_list_as_much_as_possible(std::map<uint32_t, uint32_t>& item_list, uint32_t statis_id, bool isNty) //增加一堆道具，尽可能放入背包，满了丢弃
{
	std::map<uint32_t, uint32_t> stackable_map;
	uint32_t empty_num = 0;
	for (uint32_t i = 0; i < data->bag_grid_num; ++i)
	{
		bag_grid_data& grid = data->bag[i];
		if (grid.id > 0)
		{
			uint32_t stack_num = get_item_stack_num(grid.id);
			if (stack_num > grid.num)
			{
				stackable_map[grid.id] += stack_num - grid.num;
			}
		}
		else
		{
			empty_num++;
		}
	}

	std::map<uint32_t, uint32_t> add_list;
	for (std::map<uint32_t, uint32_t>::iterator iter = item_list.begin(); iter != item_list.end(); ++iter)
	{
		uint32_t item_id = iter->first;
		uint32_t item_num = iter->second;
		if (get_item_type(item_id) != ITEM_TYPE_ITEM)
		{
			add_list[item_id] += item_num;
			continue;
		}

		std::map<uint32_t, uint32_t>::iterator iter_stack = stackable_map.find(item_id);
		if (iter_stack != stackable_map.end())
		{
			if (iter_stack->second > item_num)
			{
				add_list[item_id] += item_num;
				item_num = 0;
				iter_stack->second -= item_num;
			}
			else
			{
				add_list[item_id] += iter_stack->second;
				item_num -= iter_stack->second;
				stackable_map.erase(item_id);
			}
		}

		if (item_num > 0)
		{
			uint32_t stack_num = get_item_stack_num(item_id);
			while (item_num > 0 && empty_num > 0)
			{
				empty_num--;
				if (item_num >= stack_num)
				{
					add_list[item_id] += stack_num;
					item_num -= stack_num;
				}
				else
				{
					add_list[item_id] += item_num;
					stackable_map[item_id] = stack_num - item_num;
					item_num = 0;
				}
			}
		}
	}

	for (std::map<uint32_t, uint32_t>::iterator iter = add_list.begin(); iter != add_list.end(); ++iter)
	{
		this->add_item(iter->first, iter->second, statis_id, isNty);
	}

	return true;
}

bool player_struct::add_item_list_otherwise_send_mail(std::map<uint32_t, uint32_t>& item_list, uint32_t statis_id, uint32_t mail_id, std::vector<char *> *mail_args, bool isNty) //增加一堆道具，背包满后发邮件
{
	std::map<uint32_t, uint32_t> failed_list;
	for (std::map<uint32_t, uint32_t>::iterator iter = item_list.begin(); iter != item_list.end(); ++iter)
	{
		if (add_item(iter->first, iter->second, statis_id, isNty) != 0)
		{
			failed_list[iter->first] = iter->second;
		}
	}

	if (failed_list.size() > 0)
	{
		if (mail_id > 0)
		{
			send_mail_by_id(mail_id, mail_args, &failed_list, statis_id);
			send_system_notice(ERROR_ID_BAG_NOT_ABLE_ADD_PASSIVE, NULL);
		}
		else
		{
			LOG_ERR("[%s:%d] player[%lu] bag full, but has't mail_id, statis_id:%u", __FUNCTION__, __LINE__, data->player_id, statis_id);
			return false;
		}
	}

	return true;
}

int player_struct::get_item_num_by_id(uint32_t id)
{
	uint32_t total_num = 0;
	std::vector<uint32_t> pos_list;
	this->item_find_pos_by_cache(id, pos_list);
	if (pos_list.size() == 0)
	{
		return 0;
	}

	for (size_t i = 0; i < pos_list.size(); ++i)
	{
		uint32_t pos = pos_list[i];
		if (pos >= data->bag_grid_num)
		{
			continue;
		}

		bag_grid_data *grid = &data->bag[pos];
		total_num += grid->num;
	}

	return total_num;
}

int player_struct::get_item_can_use_num(uint32_t id)
{
	uint32_t can_use_num = this->get_item_num_by_id(id);
	uint32_t relate_id = get_item_relate_id(id);
	if (relate_id > 0)
	{
		can_use_num += this->get_item_num_by_id(relate_id);
	}

	return can_use_num;
}

int player_struct::del_item_grid(uint32_t pos, bool isNty)
{
	if (pos >= data->bag_grid_num)
	{
		return -1;
	}

	this->del_item_pos_cache(data->bag[pos].id, pos);

	memset(&data->bag[pos], 0, sizeof(bag_grid_data));

	if (isNty)
	{
		this->update_bag_grid(pos);
	}

	return 0;
}

int player_struct::del_item_by_pos(uint32_t pos, uint32_t num, uint32_t statis_id, bool isNty)
{
	if (pos >= data->bag_grid_num)
	{
		LOG_ERR("[%s:%d] player[%lu] bag pos illegal, pos:%u, open_num:%u", __FUNCTION__, __LINE__, data->player_id, pos, data->bag_grid_num);
		return -1;
	}

	bag_grid_data *grid = &data->bag[pos];
	if (grid->num < num)
	{
		return -1;
	}

	grid->num -= num;
	if (grid->num == 0)
	{
		this->del_item_grid(pos, false);
	}

	if (isNty)
	{
		this->update_bag_grid(pos);
	}

	return 0;
}

static void del_num_from_list(bag_grid_data* bag_data, uint32_t pos, int& tmp_num, std::map<uint32_t, uint32_t> &del_list)
{
	bag_grid_data& grid = bag_data[pos];
	uint32_t del_num = std::min(grid.num, (uint32_t)tmp_num);
	del_list.insert(std::make_pair(pos, del_num));
	tmp_num -= del_num;
}

int player_struct::del_item_by_id(uint32_t id, uint32_t num, uint32_t statis_id, bool isNty)
{
	ItemsConfigTable *item_config = get_config_by_id(id, &::item_config);
	if (!item_config)
	{
		LOG_ERR("[%s:%d] player[%lu] get item config failed, id:%u", __FUNCTION__, __LINE__, data->player_id, id);
		return -1;
	}

	uint32_t total_num = 0;
	std::vector<uint32_t> origin_pos_list;
	uint32_t origin_min_pos = 0x7fffffff;
	uint32_t origin_min_num = 0x7fffffff;
	this->item_find_pos_by_cache(id, origin_pos_list);
	for (size_t i = 0; i < origin_pos_list.size(); ++i)
	{
		uint32_t pos = origin_pos_list[i];
		if (pos >= data->bag_grid_num)
		{
			continue;
		}

		total_num += data->bag[pos].num;
		if (data->bag[pos].num < origin_min_num)
		{
			origin_min_num = data->bag[pos].num;
			origin_min_pos = pos;
		}
	}

	if (total_num < num)
	{
		return -1;
	}

	int tmp_num = num;
	std::map<uint32_t, uint32_t> del_list;
	if (tmp_num > 0 && origin_pos_list.size() > 0)
	{
		if (origin_min_pos != 0x7fffffff)
		{
			del_num_from_list(data->bag, origin_min_pos, tmp_num, del_list);
		}

		for (size_t i = 0; i < origin_pos_list.size() && tmp_num > 0; ++i)
		{
			uint32_t pos = origin_pos_list[i];
			if (pos == origin_min_pos)
			{
				continue;
			}

			del_num_from_list(data->bag, pos, tmp_num, del_list);
		}
	}

	for (std::map<uint32_t, uint32_t>::iterator iter = del_list.begin(); iter != del_list.end(); ++iter)
	{
		this->del_item_by_pos(iter->first, iter->second, 0, isNty);
	}

	return 0;
}

int player_struct::del_item(uint32_t id, uint32_t num, uint32_t statis_id, bool isNty)
{
	ItemsConfigTable *item_config = get_config_by_id(id, &::item_config);
	if (!item_config)
	{
		LOG_ERR("[%s:%d] player[%lu] get item config failed, id:%u", __FUNCTION__, __LINE__, data->player_id, id);
		return -1;
	}

	uint32_t total_num = 0;
	std::vector<uint32_t> origin_pos_list;
	uint32_t origin_min_pos = 0x7fffffff;
	uint32_t origin_min_num = 0x7fffffff;
	this->item_find_pos_by_cache(id, origin_pos_list);
	for (size_t i = 0; i < origin_pos_list.size(); ++i)
	{
		uint32_t pos = origin_pos_list[i];
		if (pos >= data->bag_grid_num)
		{
			continue;
		}

		total_num += data->bag[pos].num;
		if (data->bag[pos].num < origin_min_num)
		{
			origin_min_num = data->bag[pos].num;
			origin_min_pos = pos;
		}
	}

	std::vector<uint32_t> relate_pos_list;
	uint32_t relate_min_pos = 0x7fffffff;
	uint32_t relate_min_num = 0x7fffffff;
	if (item_config->ItemRelation > 0)
	{
		uint32_t relate_id = item_config->ItemRelation;
		this->item_find_pos_by_cache(relate_id, relate_pos_list);
		for (size_t i = 0; i < relate_pos_list.size(); ++i)
		{
			uint32_t pos = relate_pos_list[i];
			if (pos >= data->bag_grid_num)
			{
				continue;
			}

			total_num += data->bag[pos].num;
			if (data->bag[pos].num < relate_min_num)
			{
				relate_min_num = data->bag[pos].num;
				relate_min_pos = pos;
			}
		}
	}

	if (total_num < num)
	{
		return -1;
	}

	std::vector<uint32_t> *bind_pos_list = NULL, *unbind_pos_list = NULL;
	uint32_t bind_min_pos = 0, unbind_min_pos = 0;
	if (item_config->BindType == 0) //道具本身是非绑定道具
	{
		bind_pos_list = &relate_pos_list;
		bind_min_pos = relate_min_pos;
		unbind_pos_list = &origin_pos_list;
		unbind_min_pos = origin_min_pos;
	}
	else //道具是绑定道具
	{
		bind_pos_list = &origin_pos_list;
		bind_min_pos = origin_min_pos;
		unbind_pos_list = &relate_pos_list;
		unbind_min_pos = relate_min_pos;
	}

	int tmp_num = num;
	std::map<uint32_t, uint32_t> del_list;
	if (tmp_num > 0 && bind_pos_list->size() > 0)
	{
		if (bind_min_pos != 0x7fffffff)
		{
			del_num_from_list(data->bag, bind_min_pos, tmp_num, del_list);
		}

		for (size_t i = 0; i < bind_pos_list->size() && tmp_num > 0; ++i)
		{
			uint32_t pos = (*bind_pos_list)[i];
			if (pos == bind_min_pos)
			{
				continue;
			}

			del_num_from_list(data->bag, pos, tmp_num, del_list);
		}
	}

	if (tmp_num > 0 && unbind_pos_list->size() > 0)
	{
		if (tmp_num > 0 && unbind_min_pos != 0x7fffffff)
		{
			del_num_from_list(data->bag, unbind_min_pos, tmp_num, del_list);
		}

		for (size_t i = 0; i < unbind_pos_list->size() && tmp_num > 0; ++i)
		{
			uint32_t pos = (*unbind_pos_list)[i];
			if (pos == unbind_min_pos)
			{
				continue;
			}

			del_num_from_list(data->bag, pos, tmp_num, del_list);
		}
	}

	for (std::map<uint32_t, uint32_t>::iterator iter = del_list.begin(); iter != del_list.end(); ++iter)
	{
		this->del_item_by_pos(iter->first, iter->second, 0, isNty);
	}

	return 0;
}

int player_struct::merge_item(uint32_t id)
{
	ItemsConfigTable *config = get_config_by_id(id, &item_config);
	if (!config)
	{
		return -1;
	}

	uint32_t bind_id = 0, unbind_id = 0;
	if (config->BindType == 0)
	{
		bind_id = config->ItemRelation;
		unbind_id = id;
	}
	else
	{
		bind_id = id;
		unbind_id = config->ItemRelation;
	}

	if (bind_id == 0 || unbind_id == 0)
	{
		return -1;
	}

	static bag_grid_data tmp_grid;
	std::vector<uint32_t> unbind_pos_list;
	this->item_find_pos_by_cache(unbind_id, unbind_pos_list);
	for (std::vector<uint32_t>::iterator iter = unbind_pos_list.begin(); iter != unbind_pos_list.end(); ++iter)
	{
		uint32_t pos = *iter;
		if (pos >= data->bag_grid_num)
		{
			continue;
		}

		bag_grid_data& grid = data->bag[pos];
		if (grid.id != unbind_id)
		{
			continue;
		}

		memcpy(&tmp_grid, &grid, sizeof(bag_grid_data));
		tmp_grid.id = bind_id;
		this->del_item_grid(pos);
		memcpy(&grid, &tmp_grid, sizeof(bag_grid_data));
		this->add_item_pos_cache(bind_id, pos);
		this->update_bag_grid(pos);
	}

	//TODO:非绑定变绑定，要写流水

	return 0;
}

struct BagGridChange
{
	uint32_t type; //0:减少 1:增加
	uint32_t num;
};
int player_struct::stack_item(uint32_t id)
{
	uint32_t stack_max = get_item_stack_num(id);
	if (stack_max == 0)
	{
		return -1;
	}

	std::vector<uint32_t> item_pos_list;
	this->item_find_pos_by_cache(id, item_pos_list);

	//只有一个格子，没得堆叠
	if (item_pos_list.size() <= 1)
	{
		return -1;
	}

	std::map<uint32_t, BagGridChange> change_map;

	std::vector<uint32_t>::iterator add_iter = item_pos_list.begin();
	std::vector<uint32_t>::iterator sub_iter = --item_pos_list.end();
	while (add_iter != sub_iter)
	{
		uint32_t add_pos = *add_iter;
		uint32_t sub_pos = *sub_iter;
		bag_grid_data& add_grid = data->bag[add_pos];
		bag_grid_data& sub_grid = data->bag[sub_pos];
		if (add_grid.num >= stack_max)
		{
			++add_iter;
			continue;
		}
		else
		{
			std::map<uint32_t, BagGridChange>::iterator change_add_iter = change_map.find(add_pos);
			if (change_add_iter == change_map.end())
			{
				BagGridChange change_info;
				change_info.type = 1;
				change_info.num = 0;
				change_map.insert(std::make_pair(add_pos, change_info));
				change_add_iter = change_map.find(add_pos);
			}
			std::map<uint32_t, BagGridChange>::iterator change_sub_iter = change_map.find(sub_pos);
			if (change_sub_iter == change_map.end())
			{
				BagGridChange change_info;
				change_info.type = 0;
				change_info.num = 0;
				change_map.insert(std::make_pair(sub_pos, change_info));
				change_sub_iter = change_map.find(sub_pos);
			}

			uint32_t add_grid_num = add_grid.num + change_add_iter->second.num;
			uint32_t sub_grid_num = sub_grid.num - change_sub_iter->second.num;
			if (add_grid_num + sub_grid_num > stack_max)
			{
				//只能堆一部分到前面格子，堆满前面格子，再向后递增
				uint32_t move_num = stack_max - add_grid_num;
				change_add_iter->second.num += move_num;
				change_sub_iter->second.num += move_num;

				++add_iter;
				continue;
			}
			else
			{
				//可以全部堆到前面格子，全部堆到前面格子，后面格子向前递增
				uint32_t move_num = sub_grid_num;
				change_add_iter->second.num += move_num;
				change_sub_iter->second.num += move_num;

				--sub_iter;
				continue;
			}
		}
	}

	//改变格子数量，执行堆叠
	for (std::map<uint32_t, BagGridChange>::iterator iter = change_map.begin(); iter != change_map.end(); ++iter)
	{
		bag_grid_data& grid = data->bag[iter->first];
		if (iter->second.type == 0)
		{
			if (grid.num <= iter->second.num)
			{
				this->del_item_pos_cache(grid.id, iter->first);
				memset(&grid, 0, sizeof(bag_grid_data));
			}
			else
			{
				grid.num -= iter->second.num;
			}
		}
		else
		{
			grid.num += iter->second.num;
		}
		this->update_bag_grid(iter->first);
	}

	return 0;
}

void player_struct::tidy_bag(void)
{
	this->check_bag_expire(true);
	std::set<uint32_t> all_item_id;
	for (uint32_t i = 0; i < data->bag_grid_num; ++i)
	{
		if (data->bag[i].id > 0)
		{
			all_item_id.insert(data->bag[i].id);
		}
	}

	for (std::set<uint32_t>::iterator iter = all_item_id.begin(); iter != all_item_id.end(); ++iter)
	{
		this->stack_item(*iter);
	}
}

int player_struct::check_use_prop(uint32_t item_id, uint32_t use_count, ItemUseEffectInfo *info)
{
	ItemsConfigTable *config = get_config_by_id(item_id, &item_config);
	if (!config)
	{
		return ERROR_ID_NO_CONFIG;
	}

	if (config->UseDegree == 0)
	{
		return ERROR_ID_PROP_CAN_NOT_USE;
	}

	if (config->SkillId > 0)
	{
	}

	if (config->TaskId > 0 && (task_is_finish(config->TaskId) || get_task_info(config->TaskId) != NULL))
	{
		return ERROR_ID_PROP_EFFECT_TASK;
	}

	if (config->DropId > 0)
	{
		int ret = 0;
		for (uint32_t i = 0; i < use_count; ++i)
		{
			ret = get_drop_item(config->DropId, info->items);
			if (ret != 0)
			{
				return ERROR_ID_NO_CONFIG;
			}
		}

		if (!check_can_add_item_list(info->items))
		{
			return ERROR_ID_BAG_NOT_ABLE_ADD_USE;
		}
	}

	switch (config->ItemEffect)
	{
		case IUE_TRANSFER_SCENE:
			{
				if (!scene || !scene->can_transfer(2))
				{
					return ERROR_ID_CAN_NOT_TRANSFER_IN_RAID;
				}
				if (is_in_raid())
				{
					return ERROR_ID_CAN_NOT_TRANSFER_IN_RAID;
				}
				if (data->truck.truck_id != 0)
				{
					return 190500305;
				}

				if (config->n_ParameterEffect < 1)
				{
					return ERROR_ID_NO_CONFIG;
				}
			}
			break;
		case IUE_FIND_TARGET:
			{
				int ret = ChengJieTaskManage::CanUseFollowItem(this);
				if (ret != 190500154)
				{
					return ret;
				}
			}
			break;
		case IUE_SUB_MURDER:
			{
				if (config->n_ParameterEffect < 1)
				{
					return ERROR_ID_NO_CONFIG;
				}
			}
			break;
		case IUE_ADD_HP:
			{
				if (config->n_ParameterEffect < 1)
				{
					return ERROR_ID_NO_CONFIG;
				}
			}
			break;
		case IUE_ADD_HP_POOL:
			{
				if (config->n_ParameterEffect < 1)
				{
					return ERROR_ID_NO_CONFIG;
				}

				uint32_t value = data->hp_pool_num + config->ParameterEffect[0] * use_count;
				if (value > sg_hp_pool_max)
				{
					LOG_ERR("%s: player[%lu] use hp pool too much", __FUNCTION__, data->player_id);
					return (ERROR_ID_PROP_CAN_NOT_USE);
				}
			}
			break;
		case IUE_ADD_ENERGY:
			break;
		case IUE_XUNBAO:
			break;
		case IUE_ADD_HORSE:
			{
				if (config->n_ParameterEffect < 2)
				{
					return ERROR_ID_NO_CONFIG;
				}
			}
			break;
	}

	return 0;
}

int player_struct::use_prop_effect(uint32_t id, uint32_t use_count, ItemUseEffectInfo *info)
{
	ItemsConfigTable *config = get_config_by_id(id, &item_config);
	if (!config)
	{
		return ERROR_ID_NO_CONFIG;
	}

	if (config->UseDegree == 0)
	{
		return ERROR_ID_PROP_CAN_NOT_USE;
	}

	if (config->SkillId > 0)
	{
	}

	if (config->TaskId > 0)
	{
		int ret = 0;
		for (uint32_t i = 0; i < use_count; ++i)
		{
			ret = accept_task(config->TaskId, false);
			if (ret != 0)
			{
				return ret;
			}
		}
	}

	if (info && info->items.size() > 0)
	{
		add_item_list_as_much_as_possible(info->items, MAGIC_TYPE_BAG_USE, true);
		if (info->is_easy)
		{
			noitfy_item_flow_to_bag(info->items);
		}
	}

	switch (config->ItemEffect)
	{
		case IUE_UNLOCK_HEAD:
			{
				this->check_head_condition(HUCT_ITEM_USE, id);
			}
			break;
		case IUE_TRANSFER_SCENE:
			{
				if (!scene || !scene->can_transfer(2))
				{
					LOG_ERR("%s player[%lu] can not transfer at this scene", __FUNCTION__, data->player_id);
					return (-1);
				}

				EXTERN_DATA ext_data;
				ext_data.player_id = data->player_id;
				if (config->n_ParameterEffect >= 1)
				{
					uint32_t trans_id = config->ParameterEffect[0];
					transfer_to_new_scene_by_config(trans_id, &ext_data);
				}
			}
			break;
		case IUE_FIND_TARGET:
			if (data->chengjie.target != 0)
			{
				ChengJieTaskManage::NotifyTargetPos(this);
			}
			break;
		case IUE_SUB_MURDER:
			{
				if (config->n_ParameterEffect >= 1)
				{
					this->sub_murder_num(config->ParameterEffect[0] * use_count);
				}
			}
			break;
		case IUE_ADD_HP:
		{
			if (config->n_ParameterEffect >= 1)
			{
				uint32_t cur_hp = get_attr(PLAYER_ATTR_HP);
				uint32_t max_hp = get_attr(PLAYER_ATTR_MAXHP);
				uint32_t add_hp = config->ParameterEffect[0] * use_count;
				cur_hp += add_hp;
				if (cur_hp > max_hp)
					cur_hp = max_hp;
				set_attr(PLAYER_ATTR_HP, cur_hp);
				broadcast_one_attr_changed(PLAYER_ATTR_HP, cur_hp, true, true);
				on_hp_changed(0);
			}
		}
		break;
		case IUE_ADD_HP_POOL:
		{
			if (config->n_ParameterEffect >= 1)
			{
				uint32_t value = data->hp_pool_num + config->ParameterEffect[0] * use_count;
				if (value > sg_hp_pool_max)
				{
					LOG_ERR("%s: player[%lu] use hp pool too much", __FUNCTION__, data->player_id);
					return (ERROR_ID_PROP_CAN_NOT_USE);
				}
				data->hp_pool_num = value;
				send_hp_pool_changed_notify();
			}
		}
		break;
		case IUE_ADD_ENERGY:
		{
			add_attr(PLAYER_ATTR_ENERGY, config->ParameterEffect[0] * use_count);
			AttrMap attrs;
			attrs[PLAYER_ATTR_ENERGY] = data->attrData[PLAYER_ATTR_ENERGY];
			this->notify_attr(attrs);
		}
			break;
		case IUE_XUNBAO:
			xunbao_drop(*this, id);
			break;
		case IUE_ADD_HORSE:
			{
				if (config->n_ParameterEffect < 2)
				{
					return ERROR_ID_NO_CONFIG;
				}
				int i = add_horse(config->ParameterEffect[0], config->ParameterEffect[1]);
				if (i >= 0)
				{
					notify_add_horse(i);
				}
			}
			break;
	}

	return 0;
}

int player_struct::try_use_prop(uint32_t pos, uint32_t use_all, ItemUseEffectInfo *info)
{
	if (pos >= data->bag_grid_num)
	{
		return ERROR_ID_BAG_POS;
	}

	bag_grid_data& grid = data->bag[pos];
	ItemsConfigTable *prop_config = get_config_by_id(grid.id, &item_config);
	if (!prop_config)
	{
		return ERROR_ID_NO_CONFIG;
	}

	uint32_t item_id = grid.id;
	uint32_t use_count = (use_all == 0 ? 1 : (prop_config->UseDegree * grid.num - grid.used_count));

	return check_use_prop(item_id, use_count, info);
}

int player_struct::use_prop(uint32_t pos, uint32_t use_all, ItemUseEffectInfo *info)
{
	int ret = 0;
	if (pos >= data->bag_grid_num)
	{
		return ERROR_ID_BAG_POS;
	}

	bag_grid_data& grid = data->bag[pos];
	ItemsConfigTable *prop_config = get_config_by_id(grid.id, &item_config);
	if (!prop_config)
	{
		return ERROR_ID_NO_CONFIG;
	}

	uint32_t item_id = grid.id;
	uint32_t use_count = (use_all == 0 ? 1 : (prop_config->UseDegree * grid.num - grid.used_count));

	uint32_t del_num = 0;
	if (use_all == 0)
	{
		grid.used_count++;
		if (grid.used_count >= prop_config->UseDegree)
		{
			grid.used_count = 0;
			del_num++;
		}
	}
	else
	{
		del_num = grid.num;
	}

	if (del_num > 0)
	{
		del_item_by_pos(pos, del_num, MAGIC_TYPE_BAG_USE);
	}
	else
	{
		update_bag_grid(pos);
	}

	this->add_task_progress(TCT_USE_PROP, item_id, use_count);

	if (this->data->xunbao.send_next)
	{
		EXTERN_DATA extern_data;
		extern_data.player_id = data->player_id;
		fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_XUNBAO_USE_NEXT_NOTIFY, 0, 0);
		this->data->xunbao.send_next = false;
	}

	ret = use_prop_effect(item_id, use_count, info);
	if (ret != 0)
	{
		return ret;
	}

	return ret;
}

void player_struct::update_bag_grid(uint32_t pos)
{
	if (pos >= MAX_BAG_GRID_NUM)
	{
		return ;
	}

	bag_grid_data *grid = &data->bag[pos];

	BagGrid grid_data;
	bag_grid__init(&grid_data);

	ItemBaguaData bagua_data;
	AttrData bagua_attr[MAX_BAGUAPAI_MINOR_ATTR_NUM];
	AttrData* bagua_attr_point[MAX_BAGUAPAI_MINOR_ATTR_NUM];

	grid_data.index = pos;
	grid_data.id = grid->id;
	grid_data.num = grid->num;
	grid_data.usedcount = grid->used_count;
	grid_data.expiretime = grid->expire_time;
	if (item_is_baguapai(grid->id))
	{
		grid_data.bagua = &bagua_data;
		item_bagua_data__init(&bagua_data);
		bagua_data.star = grid->baguapai.star;
		bagua_data.main_attr_val = grid->baguapai.main_attr_val;
		uint32_t attr_num = 0;
		for (int j = 0; j < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++j)
		{
			if (grid->baguapai.minor_attrs[j].id == 0)
			{
				break;
			}

			bagua_attr_point[attr_num] = &bagua_attr[attr_num];
			attr_data__init(&bagua_attr[attr_num]);
			bagua_attr[attr_num].id = grid->baguapai.minor_attrs[j].id;
			bagua_attr[attr_num].val = grid->baguapai.minor_attrs[j].val;
			attr_num++;
		}
		bagua_data.minor_attrs = bagua_attr_point;
		bagua_data.n_minor_attrs = attr_num;
	}

	EXTERN_DATA extern_data;
	extern_data.player_id = data->player_id;
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_BAG_GRID_UPDATE_NOTIFY, bag_grid__pack, grid_data);
}

bool player_struct::is_item_expire(uint32_t pos)
{
	if (pos >= data->bag_grid_num)
	{
		return true;
	}

	bag_grid_data& grid = data->bag[pos];
	ItemsConfigTable* config = get_config_by_id(grid.id, &::item_config);
	if (!config)
	{
		return true;
	}

	if (config->ItemLimit > 0)
	{
		uint32_t now = time_helper::get_cached_time() / 1000;
		if (now >= grid.expire_time)
		{
			return true;
		}
	}

	return false;
}

void player_struct::check_bag_expire(bool isNty)
{
	std::vector<uint32_t> del_list;
	for (uint32_t i = 0; i < data->bag_grid_num; ++i)
	{
		if (data->bag[i].id && data->bag[i].expire_time > 0)
		{
			if (is_item_expire(i))
			{
				del_list.push_back(i);
			}
		}
	}

	for (std::vector<uint32_t>::iterator iter = del_list.begin(); iter != del_list.end(); ++iter)
	{
		del_item_grid(*iter, isNty);
	}
}

int player_struct::move_baguapai_to_bag(BaguapaiCardInfo &card)
{
	for (uint32_t i = 0; i < data->bag_grid_num; ++i)
	{
		bag_grid_data& grid = data->bag[i];
		if (grid.id == 0)
		{
			grid.id = bagua_card_to_bind_item(card.id);
			grid.num = 1;
			grid.baguapai.star = card.star;
			grid.baguapai.main_attr_val = card.main_attr_val;
			memcpy(&grid.baguapai.minor_attrs, &card.minor_attrs, sizeof(AttrInfo) * MAX_BAGUAPAI_MINOR_ATTR_NUM);

			this->add_item_pos_cache(grid.id, i);
			this->update_bag_grid(i);
			this->add_task_progress(TCT_CARRY_ITEM, grid.id, grid.num);

			return 0;
		}
	}

	return -1;
}

void player_struct::noitfy_item_flow_to_bag(std::map<uint32_t, uint32_t> &item_list)
{
	if (item_list.empty())
	{
		return;
	}
	ItemFlowToBagNotify nty;
	item_flow_to_bag_notify__init(&nty);

	ItemData item_data[50];
	ItemData* item_point[50];

	nty.itemlist = item_point;
	nty.n_itemlist = 0;
	for (std::map<uint32_t, uint32_t>::iterator iter = item_list.begin(); iter != item_list.end() && nty.n_itemlist < 50; ++iter)
	{
		item_point[nty.n_itemlist] = &item_data[nty.n_itemlist];
		item_data__init(&item_data[nty.n_itemlist]);
		item_data[nty.n_itemlist].id = iter->first;
		item_data[nty.n_itemlist].num = iter->second;
		nty.n_itemlist++;
	}

	EXTERN_DATA ext_data;
	ext_data.player_id = data->player_id;
	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_ITEM_FLOW_TO_BAG_NOTIFY, item_flow_to_bag_notify__pack, nty);
}

void player_struct::notify_one_item_flow_to_bag(uint32_t id, uint32_t num)
{
	ItemFlowToBagNotify nty;
	item_flow_to_bag_notify__init(&nty);

	ItemData item_data[1];
	ItemData* item_point[1];

	nty.itemlist = item_point;
	nty.n_itemlist = 0;
	item_point[nty.n_itemlist] = &item_data[nty.n_itemlist];
	item_data__init(&item_data[nty.n_itemlist]);
	item_data[nty.n_itemlist].id = id;
	item_data[nty.n_itemlist].num = num;
	nty.n_itemlist++;

	EXTERN_DATA ext_data;
	ext_data.player_id = data->player_id;
	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_ITEM_FLOW_TO_BAG_NOTIFY, item_flow_to_bag_notify__pack, nty);
}

bool player_struct::give_drop_item(uint32_t drop_id, uint32_t statis_id, AddItemDealWay deal_way, bool isNty, uint32_t mail_id, std::vector<char *> *mail_args)
{
	std::map<uint32_t, uint32_t> item_list;
	int ret = get_drop_item(drop_id, item_list);
	if (ret != 0)
	{
		return false;
	}

	LOG_INFO("%s: player[%lu] id[%u] statis_id[%u]", __FUNCTION__, get_uuid(), drop_id, statis_id);

	bool ret2 = false;
	switch (deal_way)
	{
		case ADD_ITEM_WILL_FAIL:
			ret2 = this->add_item_list(item_list, statis_id, isNty);
			break;
		case ADD_ITEM_AS_MUCH_AS_POSSIBLE:
			ret2 = this->add_item_list_as_much_as_possible(item_list, statis_id, isNty);
			break;
		case ADD_ITEM_SEND_MAIL_WHEN_BAG_FULL:
			ret2 = this->add_item_list_otherwise_send_mail(item_list, statis_id, mail_id, mail_args, isNty);
			break;
	}
	return ret2;
}

int player_struct::add_exp(uint32_t val, uint32_t statis_id, bool isNty)
{
	if (val == 0)
	{
		return 0;
	}

	AttrMap attrs;
	uint32_t exp_new = data->attrData[PLAYER_ATTR_EXP] + val;
	uint32_t level_old = data->attrData[PLAYER_ATTR_LEVEL];
	uint32_t level_new = level_old;

	while (true)
	{
		if (level_new >= (uint32_t)sg_player_level_limit)
		{
			break;
		}

		ActorLevelTable* level_config = get_actor_level_config(data->attrData[PLAYER_ATTR_JOB], level_new);
		if (!level_config)
		{
			break;
		}

		if ((uint64_t)exp_new < level_config->NeedExp)
		{
			break;
		}

		exp_new -= level_config->NeedExp;
		level_new++;
	}

	data->attrData[PLAYER_ATTR_EXP] = exp_new;
	attrs[PLAYER_ATTR_EXP] = exp_new;

	if (level_new != level_old)
	{
		data->attrData[PLAYER_ATTR_LEVEL] = level_new;
		this->deal_level_up(level_old, level_new);
	}
	else if (isNty)
	{
		this->notify_attr(attrs);
	}

	//系统提示
	std::vector<char *> args;
	std::stringstream ss_num;
	ss_num << val;
	args.push_back(const_cast<char*>(ss_num.str().c_str()));
	uint32_t notice_id = (notice_use_art(statis_id) ? SNT_ADD_EXP_ART : SNT_ADD_EXP_TEXT);
	send_system_notice(notice_id, &args);

	return 0;
}

int player_struct::deal_level_up(uint32_t level_old, uint32_t level_new)
{
	this->calculate_attribute(false);

	//当前血量回满
	data->attrData[PLAYER_ATTR_HP] = data->attrData[PLAYER_ATTR_MAXHP];
	on_hp_changed(0);

		//视野广播属性
	AttrMap attrs;
	attrs[PLAYER_ATTR_HP] = data->attrData[PLAYER_ATTR_HP];
	attrs[PLAYER_ATTR_LEVEL] = level_new;
	attrs[PLAYER_ATTR_FIGHTING_CAPACITY] = get_attr(PLAYER_ATTR_FIGHTING_CAPACITY);
	notify_attr(attrs, true, false);

////  给玩家自己发送属性
	// AttrMap nty_list;
	// for (uint32_t i = PLAYER_ATTR_FIGHT_MAX; i >=1; --i)
	// {
	// 	nty_list[i] = data->attrData[i];
	// }
	
	// nty_list[PLAYER_ATTR_DFWU] = data->attrData[PLAYER_ATTR_DFWU];
	// nty_list[PLAYER_ATTR_TI] = data->attrData[PLAYER_ATTR_TI];
	// nty_list[PLAYER_ATTR_LI] = data->attrData[PLAYER_ATTR_LI];
	// nty_list[PLAYER_ATTR_MIN] = data->attrData[PLAYER_ATTR_MIN];
	// nty_list[PLAYER_ATTR_LING] = data->attrData[PLAYER_ATTR_LING];
	
	// nty_list[PLAYER_ATTR_FIGHTING_CAPACITY] = data->attrData[PLAYER_ATTR_FIGHTING_CAPACITY];
	// nty_list[PLAYER_ATTR_EXP] = data->attrData[PLAYER_ATTR_EXP];
	// notify_attr(nty_list);
	std::vector<uint32_t> attr_id;
	std::vector<double> attr_value;	
	for (uint32_t i = PLAYER_ATTR_FIGHT_MAX; i >=1; --i)
	{
		attr_id.push_back(i);
		attr_value.push_back(data->attrData[i]);
	}
	attr_id.push_back(PLAYER_ATTR_DFWU);
	attr_id.push_back(PLAYER_ATTR_TI);
	attr_id.push_back(PLAYER_ATTR_LI);
	attr_id.push_back(PLAYER_ATTR_MIN);
	attr_id.push_back(PLAYER_ATTR_LING);
	attr_id.push_back(PLAYER_ATTR_FIGHTING_CAPACITY);
	attr_id.push_back(PLAYER_ATTR_EXP);
	attr_value.push_back(data->attrData[PLAYER_ATTR_DFWU]);
	attr_value.push_back(data->attrData[PLAYER_ATTR_TI]);
	attr_value.push_back(data->attrData[PLAYER_ATTR_LI]);
	attr_value.push_back(data->attrData[PLAYER_ATTR_MIN]);
	attr_value.push_back(data->attrData[PLAYER_ATTR_LING]);
	attr_value.push_back(data->attrData[PLAYER_ATTR_FIGHTING_CAPACITY]);
	attr_value.push_back(data->attrData[PLAYER_ATTR_EXP]);	

	notify_attr_changed(attr_id.size(), attr_id.data(), attr_value.data());
///

	this->check_head_condition(HUCT_LEVEL_UP, level_new);
	this->add_task_progress(TCT_BASIC, TBC_LEVEL, level_new);

	if (ChengJieTaskManage::GetRoleLevel(get_uuid()) != 0)
	{
		ChengJieTaskManage::AddRoleLevel(get_uuid(), get_attr(PLAYER_ATTR_LEVEL), data->chengjie.rest);
	}
	refresh_player_redis_info();
	m_team == NULL ? 0 : m_team->OnMemberHpChange(*this);
	m_skill.OnPlayerLevelUp(level_new);
	open_new_fashion(level_old, level_new);
	cache_to_dbserver();

	if (level_new >= 100)
	{
		send_rock_notice(*this, 330510001);
	}

	return 0;
}

int player_struct::get_total_exp(void)
{
	uint32_t total_exp = 0;
	uint32_t player_job = data->attrData[PLAYER_ATTR_JOB];
	uint32_t player_level = (uint32_t)data->attrData[PLAYER_ATTR_LEVEL];
	for (uint32_t level = 1; level < player_level; ++level)
	{
		ActorLevelTable *config = get_actor_level_config(player_job, player_level);
		if (!config)
		{
			break;
		}

		total_exp += (uint32_t)config->NeedExp;
	}

	total_exp += (uint32_t)data->attrData[PLAYER_ATTR_EXP];

	return total_exp;
}

int player_struct::sub_exp(uint32_t val, uint32_t statis_id, bool isNty)
{
	if (val == 0)
	{
		return 0;
	}

	data->attrData[PLAYER_ATTR_EXP] = std::max((int32_t)data->attrData[PLAYER_ATTR_EXP] - (int32_t)val, 0);

	if (isNty)
	{
		AttrMap attrs;
		attrs[PLAYER_ATTR_EXP] = data->attrData[PLAYER_ATTR_EXP];
		this->notify_attr(attrs);
	}

	return 0;
}

int player_struct::add_head_icon(uint32_t icon_id)
{
	ActorHeadTable *config = get_config_by_id(icon_id, &actor_head_config);
	if (!config)
	{
		return ERROR_ID_NO_CONFIG;
	}

	uint32_t player_job = data->attrData[PLAYER_ATTR_JOB];
	bool job_fit = false;
	for (uint32_t i = 0; i < config->n_Vocation; ++i)
	{
		if (config->Vocation[i] == player_job)
		{
			job_fit = true;
			break;
		}
	}

	if (!job_fit)
	{
		return -1;
	}

	for (int i = 0; i < MAX_HEAD_ICON_NUM; ++i)
	{
		if (data->head_icon_list[i].id == icon_id)
		{
			return 0;
		}

		if (data->head_icon_list[i].id == 0)
		{
			data->head_icon_list[i].id = icon_id;
			if (config->HeadLock == 2)
			{
				data->head_icon_list[i].status = HIS_NEW_UNLOCK;
				HeadIconUnlockNotify nty;
				head_icon_unlock_notify__init(&nty);
				nty.icon_id = icon_id;

				EXTERN_DATA ext_data;
				ext_data.player_id = data->player_id;
				fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_HEAD_ICON_UNLOCK_NOTIFY, head_icon_unlock_notify__pack, nty);
			}

			break;
		}
	}

	return 0;
}

int player_struct::init_head_icon(void)
{
	uint32_t player_job = data->attrData[PLAYER_ATTR_JOB];
	if (data->attrData[PLAYER_ATTR_HEAD] == 0)
	{
		ActorTable *config = get_actor_config(player_job);
		if (config)
		{
			data->attrData[PLAYER_ATTR_HEAD] = config->InitialHead;
		}
	}

	if (data->head_icon_list[0].id == 0)
	{
		for (std::map<uint64_t, ActorHeadTable *>::iterator iter = actor_head_config.begin(); iter != actor_head_config.end(); ++iter)
		{
			ActorHeadTable *config = iter->second;
			if (config == NULL)
			{
				continue;
			}

			if (config->HeadLock != 1)
			{
				continue;
			}

			bool job_fit = false;
			for (uint32_t i = 0; i < config->n_Vocation; ++i)
			{
				if (config->Vocation[i] == (uint64_t)player_job)
				{
					job_fit = true;
					break;
				}
			}

			if (!job_fit)
			{
				continue;
			}

			this->add_head_icon(config->ID);
		}
	}

	return 0;
}

HeadIconInfo *player_struct::get_head_icon(uint32_t icon_id)
{
	for (int i = 0; i < MAX_HEAD_ICON_NUM; ++i)
	{
		if (data->head_icon_list[i].id == icon_id)
		{
			return &data->head_icon_list[i];
		}
	}

	return NULL;
}

void player_struct::check_head_condition(uint32_t condition_id, uint32_t condition_val)
{
	std::set<uint32_t> unlocked_set;
	for (int i = 0; i < MAX_HEAD_ICON_NUM; ++i)
	{
		if (data->head_icon_list[i].id > 0)
		{
			unlocked_set.insert(data->head_icon_list[i].id);
		}
	}

	uint32_t player_job = data->attrData[PLAYER_ATTR_JOB];
	for (std::map<uint64_t, ActorHeadTable *>::iterator iter = actor_head_config.begin(); iter != actor_head_config.end(); ++iter)
	{
		ActorHeadTable *config = iter->second;
		if (config == NULL)
		{
			continue;
		}

		if (unlocked_set.find(config->ID) != unlocked_set.end())
		{
			continue;
		}

		if (config->HeadLock != 2)
		{
			continue;
		}

		if (config->UnlockType != condition_id)
		{
			continue;
		}

		if (config->UnlockType == HUCT_ITEM_USE)
		{
			if (config->UnlockCondition1 != condition_val)
			{
				continue;
			}
		}
		else
		{
			if (condition_val < config->UnlockCondition1)
			{
				continue;
			}
		}

		bool job_fit = false;
		for (uint32_t i = 0; i < config->n_Vocation; ++i)
		{
			if (config->Vocation[i] == (uint64_t)player_job)
			{
				job_fit = true;
				break;
			}
		}

		if (!job_fit)
		{
			continue;
		}

		this->add_head_icon(config->ID);
	}
}

bool player_struct::check_task_accept_condition(uint32_t type, uint32_t target, uint32_t val)
{
	switch (type)
	{
		case TCT_BASIC:
			{
				switch(target)
				{
					case TBC_JOB:
						return ((uint32_t)data->attrData[PLAYER_ATTR_JOB] == val);
					case TBC_LEVEL:
						return ((uint32_t)data->attrData[PLAYER_ATTR_LEVEL] >= val);
					case TBC_SEX:
						return ((uint32_t)get_player_sex(data->attrData[PLAYER_ATTR_JOB]) == val);
					case TBC_EXP:
						return ((uint32_t)get_total_exp() >= val);
					case TBC_COIN:
						return ((uint32_t)data->attrData[PLAYER_ATTR_COIN] >= val);
					case TBC_GOLD:
						return ((uint32_t)get_comm_gold() >= val);
				}
			}
			break;
		case TCT_ACCEPT:
			return task_is_finish(target);
		case TCT_TEAM:
			{
				if (!m_team)
				{
					return false;
				}
				switch (target)
				{
					case 1: //1.队伍人数	数值
						return ((uint32_t)m_team->GetMemberSize() >= val);
					case 2: //2.是否队长	0否，1是
						{
							if (val == 0)
							{
								return true;
							}
							else if (val == 1)
							{
								return (m_team->GetLeadId() == data->player_id);
							}
						}
						break;
				}
			}
			break;
		case TCT_CARRY_ITEM:
			return ((uint32_t)get_item_num_by_id(target) >= val);
		case TCT_TRUE:
			return true;
	}

	return false;
}

bool player_struct::check_task_accept_condition_by_id(uint32_t id)
{
	TaskConditionTable *config = get_config_by_id(id, &task_condition_config);
	if (!config)
	{
		return false;
	}

	return check_task_accept_condition(config->ConditionType, config->ConditionTarget, config->ConditionNum);
}

bool player_struct::task_is_finish(uint32_t task_id)
{
	return (task_finish_set.find(task_id) != task_finish_set.end());
}

int player_struct::task_is_acceptable(uint32_t task_id)
{
	uint64_t player_id = this->data->player_id;
	TaskTable *main_config = get_config_by_id(task_id, &task_config);
	if (!main_config)
	{
		LOG_ERR("[%s:%d] player[%lu] get task config failed, id:%u", __FUNCTION__, __LINE__, player_id, task_id);
		return ERROR_ID_NO_CONFIG;
	}

	uint32_t player_level = this->data->attrData[PLAYER_ATTR_LEVEL];
	if (player_level < (uint32_t)main_config->Level)
	{
		LOG_ERR("[%s:%d] player[%lu] level not enough, need_level:%u, player_level:%u", __FUNCTION__, __LINE__, player_id, (uint32_t)main_config->Level, player_level);
		return ERROR_ID_LEVEL_NOT_ENOUGH;
	}

	for (uint32_t i = 0; i < main_config->n_AccessConID; ++i)
	{
		if (!this->check_task_accept_condition_by_id(main_config->AccessConID[i]))
		{
			LOG_ERR("[%s:%d] player[%lu] accept condition, id:%u", __FUNCTION__, __LINE__, player_id, (uint32_t)main_config->AccessConID[i]);
			return ERROR_ID_TASK_CONDITION;
		}
	}

	std::map<uint32_t, uint32_t> item_list;
	this->get_task_event_item(task_id, TEC_ACCEPT, item_list);
	if (item_list.size() > 0 && !this->check_can_add_item_list(item_list))
	{
		LOG_ERR("[%s:%d] player[%lu] bag grid not enough, id:%u", __FUNCTION__, __LINE__, player_id, task_id);
		return ERROR_ID_BAG_GRID_NOT_ENOUGH;
	}

	return 0;
}

TaskInfo *player_struct::get_task_info(uint32_t task_id)
{
	for (int i = 0; i < MAX_TASK_ACCEPTED_NUM; ++i)
	{
		if (data->task_list[i].id == task_id)
		{
			return &data->task_list[i];
		}
	}

	return NULL;
}

int player_struct::add_task(uint32_t task_id, uint32_t status, bool isNty)
{
	if (status == TASK_STATUS__FINISH)
	{
		return -1;
	}

	if (task_is_finish(task_id))
	{
		return -1;
	}

	TaskInfo *info = get_task_info(task_id);
	if (info != NULL)
	{
		return -1;
	}

	int free_idx = -1;
	for (int i = 0; i < MAX_TASK_ACCEPTED_NUM; ++i)
	{
		if (data->task_list[i].id == 0)
		{
			free_idx = i;
			break;
		}
	}

	if (free_idx < 0)
	{
		return -1;
	}

	TaskTable *main_config = get_config_by_id(task_id, &task_config);
	if (!main_config)
	{
		return -1;
	}

	info = &data->task_list[free_idx];
	info->id = task_id;
	info->status = status;
	for (uint32_t i = 0; i < main_config->n_EndConID && i < MAX_TASK_TARGET_NUM; ++i)
	{
		TaskConditionTable *condition_config = get_config_by_id(main_config->EndConID[i], &task_condition_config);
		if (!condition_config)
		{
			memset(info, 0, sizeof(TaskInfo));
			return -1;
		}

		info->progress[i].id = condition_config->ID;
		info->progress[i].num = 0;
	}

	if (status == TASK_STATUS__ACCEPTED)
	{
		info->accept_ts = time_helper::get_cached_time() / 1000;
	}

	if (isNty)
	{
		this->task_update_notify(info);
	}

	return 0;
}

void player_struct::task_update_notify(TaskInfo *info)
{
	TaskUpdateNotify nty;
	task_update_notify__init(&nty);

	TaskData task_data;
	task_data__init(&task_data);
	TaskCount count_data[MAX_TASK_TARGET_NUM];
	TaskCount* count_data_point[MAX_TASK_TARGET_NUM];

	std::ostringstream os;

	nty.data = &task_data;
	task_data.id = info->id;
	task_data.status = info->status;
	task_data.expiretime = get_task_expire_time(info);
	task_data.n_progress = 0;
	task_data.progress = count_data_point;
	for (int i = 0; i < MAX_TASK_TARGET_NUM; ++i)
	{
		os << info->progress[i].num << " ";
		if (info->progress[i].id == 0)
		{
			continue;
		}

		count_data_point[task_data.n_progress] = &count_data[task_data.n_progress];
		task_count__init(&count_data[task_data.n_progress]);
		count_data[task_data.n_progress].type = info->progress[i].id;
		count_data[task_data.n_progress].count = info->progress[i].num;
		task_data.n_progress++;
	}

	TaskRewardData reward_data;
	ItemData item_data[MAX_SHANGJIN_AWARD_NUM];
	ItemData *item_data_point[MAX_SHANGJIN_AWARD_NUM];
	if (get_task_type(info->id) == TT_SHANGJIN)
	{
		task_reward_data__init(&reward_data);
		task_data.reward = &reward_data;
		reward_data.exp = data->shangjin.task[data->shangjin.cur_task].exp;
		reward_data.coin =data->shangjin.task[data->shangjin.cur_task].coin;
		reward_data.items = item_data_point;
		size_t &n_item = reward_data.n_items;
		for (; n_item < data->shangjin.task[data->shangjin.cur_task].n_award; ++n_item)
		{
			item_data_point[n_item] = &(item_data[n_item]);
			item_data__init(item_data_point[n_item]);
			item_data[n_item].id = data->shangjin.task[data->shangjin.cur_task].award[n_item].id;
			item_data[n_item].num =data->shangjin.task[data->shangjin.cur_task].award[n_item].val;
		}
	}
	else if (get_task_type(info->id) == TT_CASH_TRUCK)
	{
		BiaocheTable *table = get_config_by_id(data->truck.active_id, &cash_truck_config);
		if (table != NULL)
		{
			BiaocheRewardTable *reward_config = get_config_by_id(table->Reward, &cash_truck_reward_config);
			if (reward_config != NULL)
			{
				task_reward_data__init(&reward_data);
				task_data.reward = &reward_data;
				reward_data.exp = reward_config->RewardExp1 * get_attr(PLAYER_ATTR_LEVEL);
				reward_data.coin = reward_config->RewardMoney1 * get_attr(PLAYER_ATTR_LEVEL) * (reward_config->Deposit + 10000) / 10000.0;
				reward_data.items = item_data_point;
				size_t &n_item = reward_data.n_items;
				for (uint32_t i = 0; i < reward_config->n_RewardItem1; ++n_item,++i)
				{
					item_data_point[n_item] = &(item_data[n_item]);
					item_data__init(item_data_point[n_item]);
					item_data[n_item].id = reward_config->RewardItem1[i];
					item_data[n_item].num = reward_config->RewardNum1[i];
				}
				item_data_point[n_item] = &(item_data[n_item]);
				item_data__init(item_data_point[n_item]);
				item_data[n_item].id = 201010002;
				item_data[n_item].num = reward_config->RewardLv1 * get_attr(PLAYER_ATTR_LEVEL);
				++n_item;
			}
		}
	}

	if (task_is_team(info->id) && m_team && m_team->GetLeadId() == data->player_id)
	{
		m_team->BroadcastToTeam(MSG_ID_TASK_UPDATE_NOTIFY, &nty, (pack_func)task_update_notify__pack, 0);
		LOG_DEBUG("[%s:%d] player[%lu] id:%u, status:%u, progress:[%s], team_id:%lu", __FUNCTION__, __LINE__, data->player_id, nty.data->id, nty.data->status, os.str().c_str(), m_team->GetId());
	}
	else
	{
		EXTERN_DATA ext_data;
		ext_data.player_id = data->player_id;

		LOG_DEBUG("[%s:%d] player[%lu] id:%u, status:%u, progress:[%s]", __FUNCTION__, __LINE__, data->player_id, nty.data->id, nty.data->status, os.str().c_str());
		fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_TASK_UPDATE_NOTIFY, task_update_notify__pack, nty);
	}
}

void player_struct::get_task_event_item(uint32_t task_id, uint32_t event_class, std::map<uint32_t, uint32_t> &item_list)
{
	TaskTable *config = get_config_by_id(task_id, &task_config);
	if (!config)
	{
		return ;
	}

	for (uint32_t i =0; i < config->n_EventID; ++i)
	{
		uint32_t event_id = config->EventID[i];
		TaskEventTable *event_config = get_config_by_id(event_id, &task_event_config);
		if (!event_config)
		{
			continue;
		}

		if (event_config->EventClass != event_class)
		{
			continue;
		}

		if (event_config->EventType != TET_ADD_ITEM)
		{
			continue;
		}

		item_list[event_config->EventTarget] += event_config->EventNum;
	}
}

int player_struct::touch_task_event(uint32_t task_id, uint32_t event_class)
{
	TaskTable *config = get_config_by_id(task_id, &task_config);
	if (!config)
	{
		return -1;
	}

	for (uint32_t i = 0; i < config->n_EventID; ++i)
	{
		execute_task_event(config->EventID[i], event_class, true);
	}

	return 0;
}

int player_struct::execute_task_event(uint32_t event_id, uint32_t event_class, bool internal)
{
	TaskEventTable *config = get_config_by_id(event_id, &task_event_config);
	if (!config)
	{
		return -1;
	}

	if ((uint32_t)config->EventClass != event_class)
	{
		return -1;
	}

	switch(config->EventType)
	{
		case TET_ADD_ITEM:
			this->add_item(config->EventTarget, config->EventNum, MAGIC_TYPE_TASK_EVENT);
			break;
		case TET_DEL_ITEM:
			{
				uint32_t del_num = (config->EventNum > 0 ? config->EventNum : get_item_num_by_id(config->EventTarget));
				this->del_item_by_id(config->EventTarget, del_num, MAGIC_TYPE_TASK_EVENT);
			}
			break;
		case TET_ADD_MONSTER:
			{
				if (internal)
				{
					break;
				}

				TaskMonsterTable *monster_config = get_config_by_id(config->EventTarget, &task_monster_config);
				if (!monster_config)
				{
					break;
				}

				if (!sight_space)
				{
					sight_space = sight_space_manager::create_sight_space(this);
				}

				if (sight_space->is_task_event_exist(event_id))
				{
					break;
				}

				bool success = false;
				for (uint64_t i = 0; i < config->EventNum; ++i)
				{
					monster_struct *monster = monster_manager::create_sight_space_monster(sight_space, scene, monster_config->MonsterID, monster_config->MonsterLevel, monster_config->PointX, monster_config->PointZ);
					if (!monster)
					{
						LOG_ERR("[%s:%d] player[%lu] create monster failed, create_id:%lu", __FUNCTION__, __LINE__, data->player_id, config->EventTarget);
						break;
					}

					success = true;
				}

				if (success)
				{
					sight_space->insert_task_event(event_id);
				}
			}
			break;
		case TET_DEL_MONSTER:
			{
				if (sight_space)
				{
					sight_space_manager::del_player_from_sight_space(sight_space, this, true);
				}
			}
			break;
		case TET_ADD_BUFF:
			{
				buff_struct *buff = buff_manager::create_default_buff(config->EventTarget, this, this, true);
				if (!buff)
				{
					LOG_ERR("[%s:%d] player[%lu] create buff failed, create_id:%lu", __FUNCTION__, __LINE__, data->player_id, config->EventTarget);
					break;
				}
			}
			break;
		case TET_DEL_BUFF:
			{
				buff_struct *buff = NULL;
				for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
				{
					if (m_buffs[i] != NULL && m_buffs[i]->data != NULL && m_buffs[i]->data->buff_id == (uint32_t)config->EventTarget)
					{
						buff = m_buffs[i];
						break;
					}
				}

				if (buff)
				{
					AddBuffNotify notify;
					add_buff_notify__init(&notify);
					notify.buff_id = buff->data->buff_id;
					notify.playerid = data->player_id;
					broadcast_to_sight(MSG_ID_DEL_BUFF_NOTIFY, &notify, (pack_func)add_buff_notify__pack, true);
					buff->del_buff();
					buff = NULL;
				}
				else
				{
					LOG_ERR("[%s:%d] player[%lu] delete buff[%u] failed", __FUNCTION__, __LINE__, data->player_id, (uint32_t)config->EventTarget);
				}
			}
			break;
		case TET_ESCORT:
			start_escort(config->EventTarget);
			break;
	}

	LOG_DEBUG("[%s:%d] player[%lu], event_id:%u, event_class:%u, event_type:%lu", __FUNCTION__, __LINE__, data->player_id, event_id, event_class, config->EventType);

	return 0;
}

int player_struct::add_finish_task(uint32_t task_id)
{
	uint32_t task_type = get_task_type(task_id);
	int empty_idx = -1;
	switch (task_type)
	{
		case TT_TRUNK:
			{
				empty_idx = 0;
			}
			break;
		case TT_BRANCH:
		case TT_QUESTION:
		case TT_RAID:
			{
				for (int i = 1; i < MAX_TASK_NUM; ++i)
				{
					if (data->task_finish[i] == 0)
					{
						empty_idx = i;
						break;
					}
				}
			}
			break;
	}

	if (empty_idx >= 0)
	{
		data->task_finish[empty_idx] = task_id;
		task_finish_set.insert(task_id);

		task_finish_add_notify(task_id);
	}

	add_task_progress(TCT_FINISH, task_id, 1);

	return 0;
}

int player_struct::del_finish_task(uint32_t task_id)
{
	int idx = -1;
	for (int i = 1; i < MAX_TASK_NUM; ++i)
	{
		if (data->task_finish[i] == 0)
		{
			break;
		}
		if (data->task_finish[i] == task_id)
		{
			idx = i;
			break;
		}
	}

	if (idx >= 0)
	{
		int last_idx = MAX_TASK_NUM - 1;
		if (idx < last_idx)
		{
			memmove(&data->task_finish[idx], &data->task_finish[idx+1], (last_idx - idx) * sizeof(uint32_t));
		}
		memset(&data->task_finish[last_idx], 0, sizeof(uint32_t));
		task_finish_set.erase(task_id);

		task_finish_del_notify(task_id);
	}
	return 0;
}


int player_struct::submit_task(uint32_t task_id)
{
	TaskInfo *info = get_task_info(task_id);
	if (!info)
	{
		return -1;
	}


	return 0;
}

void player_struct::get_task_reward_item(uint32_t task_id, std::map<uint32_t, uint32_t> &item_list)
{
	TaskTable *config = get_config_by_id(task_id, &task_config);
	if (!config)
	{
		return ;
	}

	for (uint32_t i = 0; i < config->n_RewardID; ++i)
	{
		get_task_reward_item_from_config(config->RewardID[i], item_list);
	}
}

int player_struct::give_task_reward(uint32_t task_id)
{
	TaskTable *config = get_config_by_id(task_id, &task_config);
	if (!config)
	{
		return -1;
	}

	if (config->TaskType == TT_CASH_TRUCK)
	{
		cash_truck_manager::cash_truck_drop(*this);
		return 0;
	}

	for (uint32_t i = 0; i < config->n_RewardID; ++i)
	{
		give_task_reward_by_reward_id(config->RewardID[i], MAGIC_TYPE_TASK_REWARD);
	}

	return 0;
}

int player_struct::give_task_reward_by_reward_id(uint32_t reward_id, uint32_t statis_id)
{
	TaskRewardTable *reward_config = get_config_by_id(reward_id, &task_reward_config);
	if (!reward_config)
	{
		return -1;
	}

	if (reward_config->RewardEXP > 0)
	{
		this->add_exp(reward_config->RewardEXP, statis_id);
	}
	if (reward_config->RewardMoney > 0)
	{
		this->add_coin(reward_config->RewardMoney, statis_id);
	}

	for (uint32_t j = 0; j < reward_config->n_RewardType && j < reward_config->n_RewardTarget
			&& j < reward_config->n_RewardNum; ++j)
	{
		uint32_t type = reward_config->RewardType[j];
		uint32_t target = reward_config->RewardTarget[j];
		uint32_t num = reward_config->RewardNum[j];
		switch(type)
		{
			case TRT_ITEM:
				this->add_item(target, num, statis_id);
				break;
			case TRT_BUFF:
				break;
			case TRT_GOLD:
				this->add_unbind_gold(num, statis_id);
				break;
			case TRT_BIND_GOLD:
				this->add_bind_gold(num, statis_id);
				break;
		}
	}

	return 0;
}

int player_struct::touch_task_drop(uint32_t scene_id, uint32_t monster_id)
{
	for (int i = 0; i < MAX_TASK_ACCEPTED_NUM; ++i)
	{
		TaskInfo &info = data->task_list[i];
		if (info.id == 0)
		{
			continue;
		}
		if (info.status != TASK_STATUS__ACCEPTED)
		{
			continue;
		}

		TaskTable *config = get_config_by_id(info.id, &task_config);
		if (!config)
		{
			continue;
		}

		for (uint32_t j = 0; j < config->n_DropID; ++j)
		{
			uint32_t drop_id = config->DropID[j];
			TaskDropTable *drop_config = get_config_by_id(drop_id, &task_drop_config);
			if (!drop_config)
			{
				continue;
			}
			if (drop_config->SceneID != scene_id || drop_config->MonsterID != monster_id)
			{
				continue;
			}

			uint32_t rand_val = rand() % RAND_RATE_BASE;
			if (rand_val < drop_config->DropPro)
			{
				if (this->add_item(drop_config->DropItem, 1, MAGIC_TYPE_TASK_DROP) != 0)
				{
					send_system_notice(ERROR_ID_BAG_NOT_ABLE_ADD_TASK_DROP, NULL);
				}
				LOG_DEBUG("[%s:%d] player[%lu], task_id:%u, drop_id:%u, scene_id:%u, monster_id:%u, item_id:%lu", __FUNCTION__, __LINE__, data->player_id, info.id, drop_id, scene_id, monster_id, drop_config->DropItem);
			}
			break;
		}
	}
	return 0;
}

void player_struct::load_task_end(void)
{
	uint32_t cur_trunk_task_id = 0;
	for (int i = 0; i < MAX_TASK_ACCEPTED_NUM; ++i)
	{
		TaskInfo &info = data->task_list[i];
		if (info.id == 0)
		{
			continue;
		}

		if (task_is_trunk(info.id))
		{
			cur_trunk_task_id = info.id;
		}

		for (int j = 0; j < MAX_TASK_TARGET_NUM; ++j)
		{
			TaskCountInfo &count_info = info.progress[j];
			if (count_info.id == 0)
			{
				continue;
			}

			TaskConditionTable *config = get_config_by_id(count_info.id, &task_condition_config);
			if (!config)
			{
				continue;
			}

			if ((uint32_t)config->ConditionType == TCT_ESCORT)
			{
				set_task_fail(&info); //所有护送的任务都失败
			}
		}
	}

//	if (cur_trunk_task_id == 0)
//	{
//		cur_trunk_task_id = sg_first_trunk_task_id;
//	}

	uint32_t trunk_task_finish_last = task_is_trunk(data->task_finish[0]) ? data->task_finish[0] : 0;

	task_finish_set.clear();
	if (sg_first_trunk_task_id != 0)
	{
		uint32_t task_id = sg_first_trunk_task_id;
		while (true)
		{
			if (trunk_task_finish_last > 0)
			{
				task_finish_set.insert(task_id);
				if (task_id == trunk_task_finish_last)
				{
					break;
				}
			}
			else if (cur_trunk_task_id != 0)
			{
				if (task_id == cur_trunk_task_id)
				{
					break;
				}
				this->add_finish_task(task_id);
			}
			else
			{
				break;
			}
			TaskTable *config = get_config_by_id(task_id, &task_config);
			if (!config)
			{
				break;
			}

			task_id = config->FollowTask;
		}
	}

	for (int i = 0; i < MAX_TASK_NUM; ++i)
	{
		if (data->task_finish[i] > 0)
		{
			task_finish_set.insert(data->task_finish[i]);
		}
	}

	if (task_is_finish(sg_first_trunk_task_id) == false && get_task_info(sg_first_trunk_task_id) == NULL)
	{
		add_task(sg_first_trunk_task_id, TASK_STATUS__NOT_ACCEPT_YET);
	}
	else if (cur_trunk_task_id == 0 && trunk_task_finish_last != 0)
	{
		TaskTable *config = get_config_by_id(trunk_task_finish_last, &task_config);
		if (config)
		{
			add_task(config->FollowTask, TASK_STATUS__NOT_ACCEPT_YET);
		}
	}
}

void player_struct::init_task_progress(TaskInfo *info)
{
	for (int j = 0; j < MAX_TASK_TARGET_NUM; ++j)
	{
		TaskCountInfo &count_info = info->progress[j];
		if (count_info.id == 0)
		{
			break;
		}

		TaskConditionTable *config = get_config_by_id(count_info.id, &task_condition_config);
		if (!config)
		{
			break;
		}

		uint32_t num = 0;
		switch(config->ConditionType)
		{
			case TCT_BASIC:
				{
					switch(config->ConditionTarget)
					{
						case TBC_LEVEL:
							num = data->attrData[PLAYER_ATTR_LEVEL];
							break;
						case TBC_COIN:
							num = data->attrData[PLAYER_ATTR_COIN];
							break;
						case TBC_GOLD:
							num = get_comm_gold();
							break;
					}
				}
				break;
			case TCT_FINISH:
				num = (task_is_finish(config->ConditionTarget) ? 1 : 0);
				break;
			case TCT_CARRY_ITEM:
				num = get_item_num_by_id(config->ConditionTarget);
				break;
			case TCT_JOIN_CAMP:
				{
					uint32_t camp_id = get_attr(PLAYER_ATTR_ZHENYING);
					if (camp_id == 0)
					{
						break;
					}

					if (config->ConditionTarget == 0 || camp_id == (uint32_t)config->ConditionTarget)
					{
						num++;
					}
				}
				break;
			case TCT_GUILD_JOIN:
				{
					if (data->guild_id == 0)
					{
						break;
					}

					uint32_t join_type = (data->guild_office == 1 ? 2 : 1);
					if (config->ConditionTarget == 0 || join_type == (uint32_t)config->ConditionTarget)
					{
						num++;
					}
				}
				break;
			case TCT_GUILD_DONATION:
				num = data->guild_donation;
				break;
			case TCT_BAGUA_WEAR:
				{
					if (config->ConditionTarget != 0)
					{
						bool wear = false;
						for (int i = 0; i < MAX_BAGUAPAI_STYLE_NUM; ++i)
						{
							BaguapaiDressInfo *dress = &data->baguapai_dress[i];
							for (int j = 0; j < MAX_BAGUAPAI_DRESS_NUM; ++j)
							{
								BaguapaiCardInfo *card = &dress->card_list[j];
								if (card->id == (uint32_t)config->ConditionTarget)
								{
									wear = true;
									break;
								}
							}
							if (wear)
							{
								break;
							}
						}

						if (wear)
						{
							num++;
						}
					}
				}
				break;
			case TCT_PARTNER_OUT_FIGHT:
				{
					for (int i = 0; i < MAX_PARTNER_BATTLE_NUM; ++i)
					{
						uint64_t uuid = data->partner_battle[i];
						if (uuid == 0)
						{
							continue;
						}

						partner_struct *partner = get_partner_by_uuid(uuid);
						if (partner && partner->scene)
						{
							if (config->ConditionTarget == 0 || partner->data->partner_id == (uint32_t)config->ConditionTarget)
							{
								num++;
							}
						}
					}
				}
				break;
			case TCT_EQUIP_STAR:
				{
					if (config->ConditionTarget == 0)
					{
						uint32_t max_val = 0;
						for (int i = 0; i < MAX_EQUIP_NUM; ++i)
						{
							EquipInfo *info = &data->equip_list[i];
							uint32_t val = info->stair * 10 + info->star_lv;
							if (val > max_val)
							{
								max_val = val;
							}
						}

						num = max_val;
					}
					else
					{
						EquipInfo *info = get_equip(config->ConditionTarget);
						if (info)
						{
							num = info->stair * 10 + info->star_lv;
						}
					}
				}
				break;
			case TCT_FASHION_WEAR:
				{
					if (config->ConditionTarget > 0 && (config->ConditionTarget == (uint64_t)get_attr(PLAYER_ATTR_WEAPON) || config->ConditionTarget == (uint64_t)get_attr(PLAYER_ATTR_CLOTHES) || config->ConditionTarget == (uint64_t)get_attr(PLAYER_ATTR_HAT)))
					{
						num++;
					}
				}
				break;
			case TCT_FRIEND_NUM:
				num = data->friend_num;
				break;
			case TCT_JOIN_TEAM:
				num = (m_team ? m_team->m_data->m_memSize : 0);
				break;
		}

		count_info.num = num;
		if (get_task_type(info->id) == TT_SHANGJIN)
		{
			count_info.num += data->shangjin.task[data->shangjin.cur_task].reduce;
		}
	}
}

void player_struct::add_task_progress(uint32_t type, uint32_t target, uint32_t num, uint32_t task_id, uint32_t cond_id, uint64_t teammate_id)
{
	if (teammate_id > 0 && (teammate_id == data->player_id || m_team == NULL))
	{
		return;
	}

	for (int i = 0; i < MAX_TASK_ACCEPTED_NUM; ++i)
	{
		TaskInfo &info = data->task_list[i];
		TaskTable *main_config = get_config_by_id(info.id, &task_config);
		if (!main_config)
		{
			continue;
		}
		if (info.id == 0)
		{
			continue;
		}
		if (info.status != TASK_STATUS__ACCEPTED)
		{
			continue;
		}
		if (task_id > 0 && task_id != info.id)
		{
			continue;
		}
		if (teammate_id > 0)
		{
			if (main_config->Team == 0)
			{
				continue;
			}
			else if (main_config->Team == 1)
			{ //整个队伍的任务
				if (data->player_id != m_team->GetLeadId())
				{
					continue;
				}
			}
			else if (main_config->Team == 2)
			{ //任务计数共享
			}
		}

		bool count_change = false;
		for (int j = 0; j < MAX_TASK_TARGET_NUM; ++j)
		{
			TaskCountInfo &count_info = info.progress[j];
			if (cond_id > 0 && count_info.id != cond_id)
			{
				continue;
			}

			TaskConditionTable *config = get_config_by_id(count_info.id, &task_condition_config);
			if (!config)
			{
				continue;
			}

			if (config->ConditionType != type)
			{
				continue;
			}

			if (config->ConditionTarget > 0 && config->ConditionTarget != target)
			{
				continue;
			}

			if (count_info.num >= config->ConditionNum)
			{
				continue;
			}

			switch(type)
			{
				case TCT_BASIC:
				case TCT_FRIEND_NUM:
				case TCT_GUILD_DONATION:
				case TCT_JOIN_TEAM:
					count_info.num = num;
					break;
				case TCT_EQUIP_STAR:
					{
						if (num > count_info.num)
						{
							count_info.num = num;
						}
					}
					break;
				default:
					count_info.num += num;
			}

			count_change = true;
			break;
		}

		if (count_change)
		{
			if (task_is_achieved(&info))
			{
				info.status = TASK_STATUS__ACHIEVED;
				touch_task_event(info.id, TEC_ACHIEVE);
			}

			this->task_update_notify(&info);
		}
	}

	//只有原始玩家会向其他组员传递
	if (teammate_id == 0 && m_team != NULL)
	{
		for (int i = 0; i < MAX_TEAM_MEM; ++i)
		{
			MEM_INFO *member = &m_team->m_data->m_mem[i];
			if (member->id == 0 || member->timeremove > 0 || member->id == data->player_id)
			{
				continue;
			}

			player_struct *teammate = player_manager::get_player_by_id(member->id);
			if (!teammate)
			{
				continue;
			}

			teammate->add_task_progress(type, target, num, task_id, cond_id, data->player_id);
		}
	}
}

bool player_struct::task_is_achieved(TaskInfo *info)
{
	bool achieved = true;
	for (int j = 0; j < MAX_TASK_TARGET_NUM; ++j)
	{
		TaskCountInfo &count_info = info->progress[j];
		if (count_info.id == 0)
		{
			break;
		}
		TaskConditionTable *config = get_config_by_id(count_info.id, &task_condition_config);
		if (!config)
		{
			achieved = false;
			break;
		}

		if (count_info.num < config->ConditionNum)
		{
			achieved = false;
			break;
		}
	}

	return achieved;
}

bool player_struct::go_down_cash_truck()
{
	if (data->truck.truck_id == 0)
	{
		return false;
	}
	if (!data->truck.on_truck)
	{
		return false;
	}
	if (is_unit_in_move())
	{
		stop_move();
	}

	data->truck.on_truck = false;

	EXTERN_DATA extern_data;
	extern_data.player_id = get_uuid();
	CommAnswer resp;
	comm_answer__init(&resp);
	resp.result = 0;
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_GO_DOWND_CASH_TRUCK_ANSWER, comm_answer__pack, resp);

	//UpDownCashTruck send;
	//up_down_cash_truck__init(&send);
	//send.cash_truck = player->data->truck_id;
	//send.playerid = player->get_uuid();
	//conn_node_gamesrv::send_to_all_player(MSG_ID_GO_DOWND_CASH_TRUCK_NOTIFY, (void *)&send, (pack_func)up_down_cash_truck__pack);

	//广播自己信息给别人
	SightChangedNotify notify;
	sight_changed_notify__init(&notify);
	SightPlayerBaseInfo my_player_info[1];
	SightPlayerBaseInfo *my_player_info_point[1];
	my_player_info_point[0] = &my_player_info[0];
	notify.add_player = my_player_info_point;
	this->pack_sight_player_base_info(&my_player_info[0]);
	//发送给需要在视野里面添加玩家的通知
	notify.n_add_player = 1;
	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
	for (int i = 0; i < data->cur_sight_player; ++i)
		conn_node_gamesrv::broadcast_msg_add_players(data->sight_player[i], ppp);
	conn_node_gamesrv::broadcast_msg_send();

	adjust_battle_partner();

	return true;
}

int player_struct::set_task_fail(TaskInfo *info)
{
	if (!info || info->status != TASK_STATUS__ACCEPTED)
	{
		return -1;
	}

	if (task_is_trunk(info->id))
	{
		return -1;
	}

	if (get_task_type(info->id) == TT_CASH_TRUCK)
	{
		if (data->truck.on_truck)
		{
			stop_move();
			go_down_cash_truck();
		}
		cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(data->truck.truck_id);
		if (truck != NULL)
		{
			//系统提示
			uint64_t noId = 0;
			if (truck->get_attr(PLAYER_ATTR_HP) < 1)
			{
				noId = 190500298;
			}
			else
			{
				noId = 190500295;
			}
			CommAnswer resp;
			comm_answer__init(&resp);
			resp.result = noId;
			EXTERN_DATA ext_data;
			ext_data.player_id = data->player_id;
			fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_CASH_TRUCK_TASK_FAIL_NOTIFY, comm_answer__pack, resp);
			data->truck.truck_id = 0;
			if (this->sight_space != NULL)
			{
				sight_space_manager::del_player_from_sight_space(this->sight_space, this, true);
			}
			else
			{
				truck->scene->delete_cash_truck_from_scene(truck);
			}

			cash_truck_manager::delete_cash_truck(truck);
		}
		data->truck.truck_id = 0;
		data->truck.active_id = 0;
	}

	info->status = TASK_STATUS__UNACHIEVABLE;
	task_update_notify(info);

	return 0;
}

bool player_struct::task_condition_can_fail(uint32_t task_id)
{
	TaskTable *main_config = get_config_by_id(task_id, &task_config);
	if (!main_config)
	{
		return false;
	}

	for (uint32_t i = 0; i < main_config->n_EndConID; ++i)
	{
		uint64_t cond_id = main_config->EndConID[i];
		TaskConditionTable *cond_config = get_config_by_id(cond_id, &task_condition_config);
		if (!cond_config)
		{
			continue;
		}

		switch (cond_config->ConditionType)
		{
			case TCT_EXPLORE:
			case TCT_TRACK:
				return true;
		}
	}

	return false;
}


void player_struct::logout_check_task_time(void)
{
	uint32_t cur_time = time_helper::get_cached_time() / 1000;
	for (int i = 0; i < MAX_TASK_ACCEPTED_NUM; ++i)
	{
		TaskInfo &info = data->task_list[i];
		if (info.id == 0)
		{
			continue;
		}

		TaskTable *config = get_config_by_id(info.id, &task_config);
		if (!config)
		{
			continue;
		}

		if (config->TaskTime == 0)
		{
			continue;
		}

		if (config->TimeRule != 1)
		{
			continue;
		}

		info.accu_time += cur_time - info.accept_ts;
	}

	//有NPC事件的任务，下线后重置
	for (int i = 0; i < MAX_TASK_ACCEPTED_NUM; ++i)
	{
		TaskInfo &info = data->task_list[i];
		if (info.id == 0 || info.status == TASK_STATUS__NOT_ACCEPT_YET)
		{
			continue;
		}

		TaskTable *config = get_config_by_id(info.id, &task_config);
		if (!config)
		{
			continue;
		}

		for (size_t j = 0; j < config->n_EventID; ++j)
		{
			uint32_t event_id = config->EventID[j];
			TaskEventTable *event_config = get_config_by_id(event_id, &task_event_config);
			if (!event_config)
			{
				continue;
			}

			if (event_config->EventType == TET_ADD_NPC || event_config->EventType == TET_NPC_FOLLOW)
			{
				info.accept_ts = 0;
				info.accu_time = 0;
				info.status = TASK_STATUS__NOT_ACCEPT_YET;
				for (int k = 0; k < MAX_TASK_TARGET_NUM; ++k)
				{
					info.progress[k].num = 0;
				}
			}
		}
	}
}

void player_struct::login_check_task_time(void)
{
	uint32_t cur_time = time_helper::get_cached_time() / 1000;
	for (int i = 0; i < MAX_TASK_ACCEPTED_NUM; ++i)
	{
		TaskInfo &info = data->task_list[i];
		if (info.id == 0)
		{
			continue;
		}

		TaskTable *config = get_config_by_id(info.id, &task_config);
		if (!config)
		{
			continue;
		}

		if (config->TaskTime == 0)
		{
			continue;
		}

		if (config->TimeRule != 1)
		{
			continue;
		}

		info.accept_ts = cur_time;
	}
}

void player_struct::check_task_time(void)
{
	uint32_t cur_time = time_helper::get_cached_time() / 1000;
	for (int i = 0; i < MAX_TASK_ACCEPTED_NUM; ++i)
	{
		TaskInfo &info = data->task_list[i];
		if (info.id == 0)
		{
			continue;
		}

		TaskTable *config = get_config_by_id(info.id, &task_config);
		if (!config)
		{
			continue;
		}

		if (config->TaskTime == 0)
		{
			continue;
		}

		bool bFail = false;
		if (config->TimeRule == 0)
		{
			if (cur_time - info.accept_ts >= (uint32_t)config->TaskTime)
			{
				bFail = true;
			}
		}
		else if (config->TimeRule == 1)
		{
			if (cur_time - info.accept_ts + info.accu_time >= (uint32_t)config->TaskTime)
			{
				bFail = true;
			}
		}

		if (bFail)
		{
			set_task_fail(&info);
		}
	}
}

uint32_t player_struct::get_task_expire_time(TaskInfo *info)
{
	uint32_t expire_time = 0;
	do
	{
		if (info == NULL || info->id == 0)
		{
			break;
		}

		TaskTable *config = get_config_by_id(info->id, &task_config);
		if (!config)
		{
			break;
		}

		if (config->TaskTime == 0)
		{
			break;
		}

		if (config->TimeRule == 0)
		{
			expire_time = info->accept_ts + (uint32_t)config->TaskTime;
		}
		else if (config->TimeRule == 1)
		{
			expire_time = info->accept_ts + (uint32_t)config->TaskTime - info->accu_time;
		}
	} while(0);

	return expire_time;
}

int player_struct::accept_task(uint32_t task_id, bool check_condition)
{
	int ret = 0;
	if (task_is_team(task_id))
	{
		if (!this->m_team)
		{
			return ERROR_ID_TASK_TEAM_LEADER;
		}

		if (m_team->GetLeadId() != get_uuid())
		{
			return ERROR_ID_TASK_TEAM_LEADER;
		}
	}

	if (check_condition)
	{
		ret = this->task_is_acceptable(task_id);
		if (ret != 0)
		{
			return ret;
		}
	}

	TaskInfo *info = this->get_task_info(task_id);
	if (info)
	{
		if (info->status != TASK_STATUS__NOT_ACCEPT_YET)
		{
			return ERROR_ID_TASK_ACCEPTED;
		}

		info->status = TASK_STATUS__ACCEPTED;
		info->accept_ts = time_helper::get_cached_time() / 1000;
	}
	else
	{
		ret = this->add_task(task_id, TASK_STATUS__ACCEPTED);
		if (ret != 0)
		{
			return ret;
		}

		info = this->get_task_info(task_id);
	}

	this->init_task_progress(info);
	bool achieved = task_is_achieved(info);
	if (achieved)
	{
		info->status = TASK_STATUS__ACHIEVED;
	}
	this->task_update_notify(info);
	this->touch_task_event(task_id, TEC_ACCEPT);
	if (achieved)
	{
		touch_task_event(info->id, TEC_ACHIEVE);
	}
	return 0;
}

void player_struct::clear_team_task(void)
{
	for (int i = 0; i < MAX_TASK_ACCEPTED_NUM; ++i)
	{
		TaskInfo *task = &data->task_list[i];
		if (task->id == 0)
		{
			continue;
		}

		if (!task_is_team(task->id))
		{
			continue;
		}

		memset(task, 0, sizeof(TaskInfo));
	}

	for (int i = 1; i < MAX_TASK_NUM; ++i)
	{
		if (data->task_finish[i] == 0)
		{
			break;
		}

		if (!task_is_team(data->task_finish[i]))
		{
			continue;
		}

		task_finish_set.erase(data->task_finish[i]);

		int last_idx = MAX_TASK_NUM - 1;
		if (i < last_idx)
		{
			memmove(&data->task_finish[i], &data->task_finish[i+1], (last_idx - i) * sizeof(uint32_t));
		}
		memset(&data->task_finish[last_idx], 0, sizeof(uint32_t));
		i--;
	}
}

void player_struct::task_remove_notify(uint32_t task_id)
{
	TaskRemoveNotify nty;
	task_remove_notify__init(&nty);

	nty.task_id = task_id;

	EXTERN_DATA ext_data;
	ext_data.player_id = data->player_id;

	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_TASK_REMOVE_NOTIFY, task_remove_notify__pack, nty);
}

void player_struct::task_finish_add_notify(uint32_t task_id)
{
	TaskUpdateFinishNotify nty;
	task_update_finish_notify__init(&nty);

	uint32_t tmp_data[1];
	tmp_data[0] = task_id;

	nty.adds = tmp_data;
	nty.n_adds = 1;

	if (task_is_team(task_id) && m_team && m_team->GetLeadId() == data->player_id)
	{
		m_team->BroadcastToTeam(MSG_ID_TASK_UPDATE_FINISH_NOTIFY, &nty, (pack_func)task_update_finish_notify__pack, 0);
		LOG_DEBUG("[%s:%d] player[%lu] task[%u] team_id:%lu", __FUNCTION__, __LINE__, data->player_id, task_id, m_team->GetId());
	}
	else
	{
		EXTERN_DATA ext_data;
		ext_data.player_id = data->player_id;

		LOG_DEBUG("[%s:%d] player[%lu] id:%u", __FUNCTION__, __LINE__, data->player_id, task_id);
		fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_TASK_UPDATE_FINISH_NOTIFY, task_update_finish_notify__pack, nty);
	}
}

void player_struct::task_finish_del_notify(uint32_t task_id)
{
	TaskUpdateFinishNotify nty;
	task_update_finish_notify__init(&nty);

	uint32_t tmp_data[1];
	tmp_data[0] = task_id;

	nty.dels = tmp_data;
	nty.n_dels = 1;

	if (task_is_team(task_id) && m_team && m_team->GetLeadId() == data->player_id)
	{
		m_team->BroadcastToTeam(MSG_ID_TASK_UPDATE_FINISH_NOTIFY, &nty, (pack_func)task_update_finish_notify__pack, 0);
		LOG_DEBUG("[%s:%d] player[%lu] id:%u, team_id:%lu", __FUNCTION__, __LINE__, data->player_id, task_id, m_team->GetId());
	}
	else
	{
		EXTERN_DATA ext_data;
		ext_data.player_id = data->player_id;

		LOG_DEBUG("[%s:%d] player[%lu] id:%u", __FUNCTION__, __LINE__, data->player_id, task_id);
		fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_TASK_UPDATE_FINISH_NOTIFY, task_update_finish_notify__pack, nty);
	}
}

void player_struct::leave_team(player_struct *leader)
{
	if (leader == NULL)
	{
		return ;
	}

	for (int i = 0; i < MAX_TASK_ACCEPTED_NUM; ++i)
	{
		TaskInfo &info = leader->data->task_list[i];
		if (info.id == 0)
		{
			continue;
		}

		if (!task_is_team(info.id))
		{
			continue;
		}

		task_remove_notify(info.id);
	}

	std::vector<uint32_t> finish_del;
	for (int i = 1; i < MAX_TASK_NUM; ++i)
	{
		uint32_t task_id = leader->data->task_finish[i];
		if (task_id == 0)
		{
			break;
		}
		if (!task_is_team(task_id))
		{
			continue;
		}

		finish_del.push_back(task_id);
	}

	if (!finish_del.empty())
	{
		TaskUpdateFinishNotify nty;
		task_update_finish_notify__init(&nty);

		nty.dels = &finish_del[0];
		nty.n_dels = finish_del.size();

		EXTERN_DATA ext_data;
		ext_data.player_id = get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_TASK_UPDATE_FINISH_NOTIFY, task_update_finish_notify__pack, nty);
	}
}

void player_struct::hand_out_team_leader(player_struct *leader)
{
	do
	{
		if (!leader)
		{
			break;
		}

		if (leader == this)
		{
			break;
		}

		for (int i = 0; i < MAX_TASK_ACCEPTED_NUM; ++i)
		{
			TaskInfo *task = &this->data->task_list[i];
			if (task->id > 0 && task_is_team(task->id))
			{
				int free_idx = -1;
				bool task_exist = false;
				for (int j = 0; j < MAX_TASK_ACCEPTED_NUM; ++j)
				{
					if (leader->data->task_list[j].id == 0 && free_idx < 0)
					{
						free_idx = i;
					}
					if (leader->data->task_list[j].id == task->id)
					{
						task_exist = true;
						break;
					}
				}

				if (!(free_idx < 0 || task_exist))
				{
					memcpy(&leader->data->task_list[free_idx], task, sizeof(TaskInfo));
				}

				memset(task, 0, sizeof(TaskInfo));
			}
		}

		for (int i = 1; i < MAX_TASK_NUM; ++i)
		{
			uint32_t task_id = data->task_finish[i];
			if (task_id == 0)
			{
				break;
			}
			if (!task_is_team(task_id))
			{
				continue;
			}

			task_finish_set.erase(task_id);

			int last_idx = MAX_TASK_NUM - 1;
			if (i < last_idx)
			{
				memmove(&data->task_finish[i], &data->task_finish[i+1], (last_idx - i) * sizeof(uint32_t));
			}
			memset(&data->task_finish[last_idx], 0, sizeof(uint32_t));
			i--;

			//把数据拷贝给leader
			int free_idx = -1;
			bool task_exist = false;
			for (int j = 1; j < MAX_TASK_NUM; ++j)
			{
				if (leader->data->task_finish[j] == 0)
				{
					free_idx = j;
					break;
				}
				else if (leader->data->task_finish[j] == task_id)
				{
					task_exist = true;
					break;
				}
			}

			if (!task_exist && free_idx >= 0)
			{
				leader->data->task_finish[free_idx] = task_id;
				leader->task_finish_set.insert(task_id);
			}
		}

		for (int i = 0; i < MAX_ESCORT_NUM; ++i)
		{
			EscortInfo *escort = &this->data->escort_list[i];
			if (escort->escort_id == 0 || !escort_is_team(escort->escort_id))
			{
				continue;
			}

			int free_idx = -1;
			bool exist = false;
			for (int j = 0; j < MAX_ESCORT_NUM; ++j)
			{
				if (leader->data->escort_list[j].escort_id == 0 && free_idx < 0)
				{
					free_idx = i;
				}
				if (leader->data->escort_list[j].escort_id == escort->escort_id)
				{
					exist = true;
					break;
				}
			}

			if (free_idx < 0 || exist)
			{
				stop_escort(escort->escort_id, false);
				clear_escort_by_index(i);
				continue;
			}

			memcpy(&leader->data->escort_list[free_idx], escort, sizeof(EscortInfo));
			monster_struct *monster = monster_manager::get_monster_by_id(escort->summon_monster_uuids[0]);
			if (monster)
			{
				monster->ai_data.escort_ai.owner_player_id = leader->get_uuid();
			}
			memset(escort, 0, sizeof(EscortInfo));
		}
	} while(0);

	//无论有没有新队长，自己身上的数据要清掉
	this->clear_team_task();
	for (int i = 0; i < MAX_ESCORT_NUM; ++i)
	{
		EscortInfo *escort = &this->data->escort_list[i];
		if (escort->escort_id == 0 || !escort_is_team(escort->escort_id))
		{
			continue;
		}

		stop_escort(escort->escort_id, false);
		clear_escort_by_index(i);
		memset(escort, 0, sizeof(EscortInfo));
	}
}

int player_struct::get_task_chapter_info(uint32_t &id, uint32_t &state)
{
	uint32_t cur_trunk_task_id = 0;
	for (int i = 0; i < MAX_TASK_ACCEPTED_NUM; ++i)
	{
		TaskInfo &info = data->task_list[i];
		if (info.id == 0)
		{
			continue;
		}

		if (task_is_trunk(info.id))
		{
			cur_trunk_task_id = info.id;
			break;
		}
	}

	TaskTable *config = get_config_by_id(cur_trunk_task_id, &task_config);
	if (!config)
	{
		return -1;
	}

	id = data->task_chapter_reward + 1;
	if (data->task_chapter_reward + 1 < (uint32_t)config->ChapterId)
	{
		state = 1;
	}
	else
	{
		state = 0;
	}

	return 0;
}

void player_struct::update_task_chapter_info(void)
{
	TaskUpdateChapterRewardNotify nty;
	task_update_chapter_reward_notify__init(&nty);

	get_task_chapter_info(nty.chapterid, nty.chapterstate);

	EXTERN_DATA ext_data;
	ext_data.player_id = get_uuid();

	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_TASK_UPDATE_CHAPTER_REWARD_NOTIFY, task_update_chapter_reward_notify__pack, nty);
}

void player_struct::sing_notify(uint32_t msg_id, uint32_t type, uint32_t time, bool broadcast, bool include_myself)
{
	LOG_DEBUG("[%s:%d] player[%lu], type:%u, time:%u, broadcast:%u, include_myself:%u, msg_id:%u", __FUNCTION__, __LINE__, data->player_id, type, time, broadcast, include_myself, msg_id);
	SingNotify nty;
	sing_notify__init(&nty);
	nty.playerid = data->player_id;
	nty.singtype = type;
	nty.singtime = time;

	if (broadcast)
	{
		broadcast_to_sight(msg_id, &nty, (pack_func)sing_notify__pack, include_myself);
	}
	else
	{
		EXTERN_DATA extern_data;
		extern_data.player_id = data->player_id;
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, msg_id, sing_notify__pack, nty);
	}
}

int player_struct::begin_sing(uint32_t type, uint32_t time, bool broadcast, bool include_myself, void *args)
{
	LOG_DEBUG("[%s:%d] player[%lu], type:%u, time:%u, broadcast:%u, include_myself:%u", __FUNCTION__, __LINE__, data->player_id, type, time, broadcast, include_myself);
	interrupt();
	data->sing_info.type = type;
	data->sing_info.time = time;
	data->sing_info.broad = broadcast;
	data->sing_info.include_myself = include_myself;
	data->sing_info.start_ts = time_helper::get_cached_time();
	data->sing_info.args = args;

	sing_notify(MSG_ID_SING_BEGIN_NOTIFY, type, time, broadcast, include_myself);
	return 0;
}

int player_struct::interrupt_sing(void)
{
	if (data->sing_info.type != 0)
	{
		sing_notify(MSG_ID_SING_INTERRUPT_NOTIFY, data->sing_info.type, data->sing_info.time, data->sing_info.broad, data->sing_info.include_myself);
		switch(data->sing_info.type)
		{
			case SING_TYPE__USE_PROP:
			case SING_TYPE__XUNBAO:
				{
					delete (ItemUseEffectInfo*)data->sing_info.args;
				}
				break;
		}
		memset(&data->sing_info, 0, sizeof(SingInfo));
	}
	return 0;
}

int player_struct::end_sing(void)
{
	if (data->sing_info.type == 0)
	{
		return 0;
	}

	sing_notify(MSG_ID_SING_END_NOTIFY, data->sing_info.type, data->sing_info.time, data->sing_info.broad, data->sing_info.include_myself);

	uint32_t type = data->sing_info.type;
	//吟唱结束，触发效果
	switch(type)
	{
		case SING_TYPE__USE_PROP:
		case SING_TYPE__XUNBAO:
			{
				ItemUseEffectInfo *effect_info = (ItemUseEffectInfo*)data->sing_info.args;
				use_prop(effect_info->pos, effect_info->use_all, effect_info);
				if (data->sing_info.args)
				{
					delete effect_info;
				}
			}
			break;
	}
	memset(&data->sing_info, 0, sizeof(SingInfo));

	return 0;
}

void player_struct::send_system_notice(uint32_t id, std::vector<char*> *args)
{
	if (get_entity_type(data->player_id) == ENTITY_TYPE_AI_PLAYER)
		return;

	SystemNoticeNotify nty;
	system_notice_notify__init(&nty);

	nty.id = id;
	if (args)
	{
		nty.n_args = args->size();
		nty.args = &(*args)[0];
	}

	EXTERN_DATA ext_data;
	ext_data.player_id = data->player_id;

	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_SYSTEM_NOTICE_NOTIFY, system_notice_notify__pack, nty);
}

void player_struct::send_rock_notice(player_struct &player, uint32_t notify_id)
{
	NoticeTable *table = get_config_by_id(notify_id, &notify_config);
	if (table == NULL)
	{
		return;
	}
	char buff[512];
	ChatHorse send;
	chat_horse__init(&send);
	send.id = notify_id;
	send.prior = table->Priority;
	send.content = buff;
	switch (notify_id)
	{
	case 330510001:
		snprintf(buff, 510, table->NoticeTxt, player.get_name());
		break;
	}
	conn_node_gamesrv::send_to_all_player(MSG_ID_CHAT_HORSE_NOTIFY, &send, (pack_func)chat_horse__pack);
}

EquipInfo *player_struct::get_equip(uint32_t type)
{
	if (data && type >= 1 && type <= MAX_EQUIP_NUM)
	{
		return &data->equip_list[type - 1];
	}

	return NULL;
}

void player_struct::add_pet(monster_struct *pet)
{
	LOG_DEBUG("%s: player[%lu][%p] pet[%lu][%p]", __FUNCTION__, get_uuid(), this, pet->get_uuid(), pet);
	pet->data->owner = get_uuid();
	m_pet_list.insert(pet);
}

void player_struct::del_pet(monster_struct *pet)
{
	LOG_DEBUG("%s: player[%lu][%p] pet[%lu][%p]", __FUNCTION__, get_uuid(), this, pet->get_uuid(), pet);
	pet->data->owner = 0;
	m_pet_list.erase(pet);
}

uint32_t player_struct::add_murder_num(uint32_t num)
{
	assert(num > 0);
	int murder = get_attr(PLAYER_ATTR_MURDER);
	int old_murder = murder;
	if (murder == sg_muder_num_max)
		return murder;
	murder += num;
	if (murder > sg_muder_num_max)
		murder = sg_muder_num_max;
	set_attr(PLAYER_ATTR_MURDER, murder);

	if (old_murder < sg_muder_debuff[0] && murder >= sg_muder_debuff[0])
	{
		buff_manager::create_default_buff(sg_muder_debuff[1], this, this, true);
	}

	uint32_t id[1];
	double value[1];
	id[0] = PLAYER_ATTR_MURDER;
	value[0] = murder;
	notify_attr_changed(1, id, value);
	return (murder);
}
uint32_t player_struct::sub_murder_num(uint32_t num)
{
	assert(num > 0);

	int murder = get_attr(PLAYER_ATTR_MURDER);
	int old_murder = murder;
	murder -= num;
	if (murder < 0)
		murder = 0;

	if (old_murder >= sg_muder_debuff[0] && murder < sg_muder_debuff[0])
	{
		clear_one_buff(sg_muder_debuff[1]);
	}

	uint32_t id[1];
	double value[1];
	id[0] = PLAYER_ATTR_MURDER;
	value[0] = murder;
	set_attr(PLAYER_ATTR_MURDER, murder);
	notify_attr_changed(1, id, value);
	return (murder);
}


void player_struct::clear_god_buff()
{
	if (!(buff_state & BUFF_STATE_GOD))
		return;
	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (!m_buffs[i])
			continue;
		if (m_buffs[i]->effect_config->Type != 170000006)
			continue;

		AddBuffNotify notify;
		add_buff_notify__init(&notify);
		notify.buff_id = m_buffs[i]->data->buff_id;
		notify.playerid = data->player_id;
		broadcast_to_sight(MSG_ID_DEL_BUFF_NOTIFY, &notify, (pack_func)add_buff_notify__pack, true);
		m_buffs[i]->del_buff();
	}
}

void player_struct::clear_one_buff(uint32_t id)
{
	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (!m_buffs[i] || !m_buffs[i]->data)
			continue;
		if (m_buffs[i]->data->buff_id != id)
			continue;

		AddBuffNotify notify;
		add_buff_notify__init(&notify);
		notify.buff_id = m_buffs[i]->data->buff_id;
		notify.playerid = data->player_id;
		broadcast_to_sight(MSG_ID_DEL_BUFF_NOTIFY, &notify, (pack_func)add_buff_notify__pack, true);
		m_buffs[i]->del_buff();
	}
}

bool player_struct::is_in_buff3()
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

void player_struct::clear_type3_buff()
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
		notify.playerid = data->player_id;
		broadcast_to_sight(MSG_ID_DEL_BUFF_NOTIFY, &notify, (pack_func)add_buff_notify__pack, true);
		m_buffs[i]->del_buff();
	}
}

int player_struct::add_equip(uint32_t type, uint32_t statis_id)
{
	EquipInfo *info = get_equip(type);
	if (!info)
	{
		return -1;
	}

	if (info->stair > 0)
	{
		return -1;
	}

	info->stair = 1;
	for (int i = 0; i < MAX_EQUIP_INLAY_NUM; ++i)
	{
		info->inlay[i] = -1;
	}

	//刷新属性
	calculate_attribute(true);

	//通知前端
	EquipData nty;
	equip_data__init(&nty);
	EquipEnchantData enchant_data[MAX_EQUIP_ENCHANT_NUM];
	EquipEnchantData* enchant_data_point[MAX_EQUIP_ENCHANT_NUM];
	AttrData cur_attr[MAX_EQUIP_ENCHANT_NUM];
	AttrData* cur_attr_point[MAX_EQUIP_ENCHANT_NUM];
	AttrData rand_attr[MAX_EQUIP_ENCHANT_NUM][MAX_EQUIP_ENCHANT_RAND_NUM];
	AttrData* rand_attr_point[MAX_EQUIP_ENCHANT_NUM][MAX_EQUIP_ENCHANT_RAND_NUM];

	nty.type = type;
	nty.stair = info->stair;
	nty.starlv = info->star_lv;
	nty.starexp = info->star_exp;
	size_t enchant_num = 0;
	for (int i = 0; i < MAX_EQUIP_ENCHANT_NUM; ++i)
	{
		EquipEnchantInfo &enchant_info = info->enchant[i];

		enchant_data_point[enchant_num] = &enchant_data[enchant_num];
		equip_enchant_data__init(&enchant_data[enchant_num]);
		enchant_data[enchant_num].index = i;

		cur_attr_point[enchant_num] = &cur_attr[enchant_num];
		attr_data__init(&cur_attr[enchant_num]);
		cur_attr[enchant_num].id = enchant_info.cur_attr.id;
		cur_attr[enchant_num].val = enchant_info.cur_attr.val;
		enchant_data[enchant_num].curattr = cur_attr_point[enchant_num];

		size_t rand_num = 0;
		if (enchant_info.rand_attr[0].id > 0)
		{
			for (int j = 0; j < MAX_EQUIP_ENCHANT_RAND_NUM; ++j)
			{
				rand_attr_point[enchant_num][rand_num] = &rand_attr[enchant_num][rand_num];
				attr_data__init(&rand_attr[enchant_num][rand_num]);
				rand_attr[enchant_num][rand_num].id = enchant_info.rand_attr[j].id;
				rand_attr[enchant_num][rand_num].val = enchant_info.rand_attr[j].val;
				rand_num++;
			}
		}
		enchant_data[enchant_num].n_randattr = rand_num;
		enchant_data[enchant_num].randattr = rand_attr_point[enchant_num];

		enchant_num++;
	}
	nty.n_enchant = enchant_num;
	nty.enchant = enchant_data_point;

	nty.inlay = &info->inlay[0];
	nty.n_inlay = MAX_EQUIP_INLAY_NUM;

	EXTERN_DATA ext_data;
	ext_data.player_id = data->player_id;

	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_EQUIP_ADD_NOTIFY, equip_data__pack, nty);

	//如果是武器，更新场景信息
	if (type == ET_WEAPON)
	{
		this->update_weapon_skin(true);
	}

	return 0;
}

int player_struct::add_equip_exp(uint32_t type, uint32_t val)
{
	if (val == 0)
	{
		return 0;
	}

	EquipInfo *equip_info = get_equip(type);
	if (!equip_info)
	{
		return -1;
	}

	uint32_t player_job = get_attr(PLAYER_ATTR_JOB);
	EquipStarLv *config = get_equip_star_config(player_job, type, equip_info->stair, equip_info->star_lv + 1);
	if (!config)
	{
		return -1;
	}

	equip_info->star_exp += val;

	bool level_up = false;
	while (true)
	{
		EquipStarLv *config = get_equip_star_config(player_job, type, equip_info->stair, equip_info->star_lv + 1);
		if (!config)
		{
			equip_info->star_exp = 0;
			break;
		}

		uint32_t need_exp = (uint32_t)config->StarSchedule;
		if (equip_info->star_exp < need_exp)
		{
			break;
		}

		equip_info->star_lv++;
		equip_info->star_exp -= need_exp;
		level_up = true;
	}

	this->calculate_attribute(true);
	if (level_up)
	{
		add_task_progress(TCT_EQUIP_STAR, type, equip_info->stair * 10 + equip_info->star_lv);
		add_task_progress(TCT_EQUIP_STAR_UP, type, 1);
	}

	return 0;
}

bool player_struct::equip_is_max_star(uint32_t type)
{
	EquipInfo *equip_info = get_equip(type);
	if (!equip_info)
	{
		return false;
	}

	uint32_t player_job = get_attr(PLAYER_ATTR_JOB);
	EquipStarLv *config = get_equip_star_config(player_job, type, equip_info->stair, equip_info->star_lv + 1);
	if (!config)
	{
		return true;
	}

	return false;
}

uint32_t player_struct::get_equip_max_star_need_exp(uint32_t type)
{
	EquipInfo *equip_info = get_equip(type);
	if (!equip_info)
	{
		return 0;
	}

	uint32_t need_exp = 0;
	uint32_t player_job = get_attr(PLAYER_ATTR_JOB);
	uint32_t star = equip_info->star_lv + 1;
	while (true)
	{
		EquipStarLv *config = get_equip_star_config(player_job, type, equip_info->stair, star);
		if (!config)
		{
			break;
		}

		need_exp += (uint32_t)config->StarSchedule;
		star++;
	}

	need_exp -= equip_info->star_exp;

	return need_exp;
}

uint64_t player_struct::reset_pvp_raid_level(uint16_t score, uint8_t *level, uint8_t *star)
{
	for (std::map<uint64_t, struct StageTable*>::iterator ite = pvp_raid_config.begin();
		 ite != pvp_raid_config.end(); ++ite)
	{
		if (score < ite->second->StageScore)
		{
			*level = ite->second->Stage;
			*star = ite->second->StageLevel;
			return ite->first;
		}
	}
	std::map<uint64_t, struct StageTable*>::reverse_iterator ite = pvp_raid_config.rbegin();
	*level = ite->second->Stage;
	*star = ite->second->StageLevel;
	return ite->first;
}

int player_struct::send_pvp_raid_score_changed(int type)
{
	if (get_entity_type(data->player_id) == ENTITY_TYPE_AI_PLAYER)
		return 0;

	//	//初始化
	// if (data->pvp_raid_data.cur_level_id_3 == 0 || data->pvp_raid_data.level_3 == 0)
	// {
	//	data->pvp_raid_data.cur_level_id_3 = pvp_raid_config.begin()->first;
	//	data->pvp_raid_data.cur_level_id_5 = data->pvp_raid_data.cur_level_id_3;
	//	data->pvp_raid_data.max_level_id_3 = data->pvp_raid_data.cur_level_id_3;
	//	data->pvp_raid_data.max_level_id_5 = data->pvp_raid_data.cur_level_id_3;

	//	data->pvp_raid_data.level_3 = pvp_raid_config.begin()->second->Stage;
	//	data->pvp_raid_data.star_3 = pvp_raid_config.begin()->second->StageLevel;
	//	data->pvp_raid_data.level_5 = data->pvp_raid_data.level_3;
	//	data->pvp_raid_data.star_5 = data->pvp_raid_data.star_3;

	//	data->pvp_raid_data.avaliable_reward_level_3 = data->pvp_raid_data.cur_level_id_3;
	//	data->pvp_raid_data.avaliable_reward_level_5 = data->pvp_raid_data.avaliable_reward_level_3;
	//	for (int i = 0; i < MAX_ONEDAY_PVP_BOX; ++i)
	//	{
	//		data->pvp_raid_data.avaliable_box_3[i] = i;
	//		data->pvp_raid_data.avaliable_box_5[i] = i;
	//	}
	// }

	uint32_t box[MAX_ONEDAY_PVP_BOX];
	PvpScoreChangedNotify nty;
	pvp_score_changed_notify__init(&nty);
	nty.type = type;
	if (type == PVP_TYPE_DEFINE_3)
	{
		nty.level = data->pvp_raid_data.cur_level_id_3;
		nty.score = data->pvp_raid_data.score_3;
		nty.today_win_num = data->pvp_raid_data.oneday_win_num_3;
		nty.max_level = data->pvp_raid_data.max_level_id_3;
		nty.max_score = data->pvp_raid_data.max_score_3;
		nty.avaliable_reward_level = data->pvp_raid_data.avaliable_reward_level_3;

		for (int i = 0; i < MAX_ONEDAY_PVP_BOX; ++i)
		{
			if (data->pvp_raid_data.avaliable_box_3[i] == (uint8_t)(-1))
				continue;
//			if (data->pvp_raid_data.oneday_win_num_3 <= data->pvp_raid_data.avaliable_box_3[i])
//				continue;
//			nty.avaliable_box_id[nty.n_avaliable_box_id++] = data->pvp_raid_data.avaliable_box_3[i];
			box[nty.n_avaliable_box_id++] = data->pvp_raid_data.avaliable_box_3[i];
		}
		nty.avaliable_box_id = box;
	}
	else
	{
		nty.level = data->pvp_raid_data.cur_level_id_5;
		nty.score = data->pvp_raid_data.score_5;
		nty.today_win_num = data->pvp_raid_data.oneday_win_num_5;
		nty.max_level = data->pvp_raid_data.max_level_id_5;
		nty.max_score = data->pvp_raid_data.max_score_5;
		nty.avaliable_reward_level = data->pvp_raid_data.avaliable_reward_level_5;

		for (int i = 0; i < MAX_ONEDAY_PVP_BOX; ++i)
		{
			if (data->pvp_raid_data.avaliable_box_5[i] == (uint8_t)-1)
				continue;
//			if (data->pvp_raid_data.oneday_win_num_5 <= data->pvp_raid_data.avaliable_box_5[i])
//				continue;
//			nty.avaliable_box_id[nty.n_avaliable_box_id++] = data->pvp_raid_data.avaliable_box_5[i];
			box[nty.n_avaliable_box_id++] = data->pvp_raid_data.avaliable_box_5[i];
		}
		nty.avaliable_box_id = box;
	}
	EXTERN_DATA ext;
	ext.player_id = get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_PVP_SCORE_CHANGED_NOTIFY, pvp_score_changed_notify__pack, nty);
	return (0);
}

int player_struct::add_pvp_raid_score(int type, int value)
{
	assert(value >= 0);
	if (!is_online())
	{
			// 发送离线数据
		PROTO_PLAYER_CACHE_INSERT *proto = (PROTO_PLAYER_CACHE_INSERT *)conn_node_base::global_send_buf;
		UNUSED(proto);
		proto->player_id = get_uuid();
		proto->type = CACHE_PVP_RAID_WIN;

		CachePvpRaidWin lose;
		cache_pvp_raid_win__init(&lose);
		lose.score = value;
		lose.type = type;
		lose.time = time_helper::get_cached_time() / 1000;
		int len = cache_pvp_raid_win__pack(&lose, proto->data);
		EXTERN_DATA ext;
		ext.player_id = proto->player_id;
		size_t size = sizeof(*proto) - sizeof(PROTO_HEAD) + len;
		fast_send_msg_base(&conn_node_gamesrv::connecter, &ext, SERVER_PROTO_INSERT_OFFLINE_CACHE, size, 0);

		return (0);
	}

	if (type == PVP_TYPE_DEFINE_3)
	{
//		++data->pvp_raid_data.oneday_win_num_3;
		data->pvp_raid_data.score_3 += value;
		uint64_t id = reset_pvp_raid_level(data->pvp_raid_data.score_3, &data->pvp_raid_data.level_3, &data->pvp_raid_data.star_3);
		LOG_INFO("%s: player[%lu] pvp3 lv from[%u] to [%lu]", __FUNCTION__, data->player_id, data->pvp_raid_data.cur_level_id_3, id);
		data->pvp_raid_data.cur_level_id_3 = id;
		if (data->pvp_raid_data.score_3 > data->pvp_raid_data.max_score_3)
		{
			data->pvp_raid_data.max_score_3 = data->pvp_raid_data.score_3;
			if (id > data->pvp_raid_data.max_level_id_3)
				data->pvp_raid_data.max_level_id_3 = id;
		}
		refresh_player_redis_info(false);
	}
	else
	{
//		++data->pvp_raid_data.oneday_win_num_5;
		data->pvp_raid_data.score_5 += value;
		uint64_t id = reset_pvp_raid_level(data->pvp_raid_data.score_5, &data->pvp_raid_data.level_5, &data->pvp_raid_data.star_5);
		LOG_INFO("%s: player[%lu] pvp5 lv from[%u] to [%lu]", __FUNCTION__, data->player_id, data->pvp_raid_data.cur_level_id_5, id);
		data->pvp_raid_data.cur_level_id_5 = id;
		if (data->pvp_raid_data.score_5 > data->pvp_raid_data.max_score_5)
		{
			data->pvp_raid_data.max_score_5 = data->pvp_raid_data.score_5;
			if (id > data->pvp_raid_data.max_level_id_5)
				data->pvp_raid_data.max_level_id_5 = id;
		}

	}
	send_pvp_raid_score_changed(type);
	return (0);
}

int player_struct::sub_pvp_raid_score(int type, int value)
{
	assert(value >= 0);

	if (!is_online())
	{
			// 发送离线数据
		PROTO_PLAYER_CACHE_INSERT *proto = (PROTO_PLAYER_CACHE_INSERT *)conn_node_base::global_send_buf;
		UNUSED(proto);
		proto->player_id = get_uuid();
		proto->type = CACHE_PVP_RAID_LOSE;

		CachePvpRaidLose lose;
		cache_pvp_raid_lose__init(&lose);
		lose.score = value;
		lose.type = type;
		lose.time = time_helper::get_cached_time() / 1000;
		int len = cache_pvp_raid_lose__pack(&lose, proto->data);
		EXTERN_DATA ext;
		ext.player_id = proto->player_id;
		size_t size = sizeof(*proto) - sizeof(PROTO_HEAD) + len;
		fast_send_msg_base(&conn_node_gamesrv::connecter, &ext, SERVER_PROTO_INSERT_OFFLINE_CACHE, size, 0);
		return (0);
	}

	if (type == PVP_TYPE_DEFINE_3)
	{
//			//新手段位不掉分
//		if (data->pvp_raid_data.level_3 == 1)
//			return (0);
		assert(data->pvp_raid_data.level_3 != 1);

		data->pvp_raid_data.score_3 -= value;
		uint8_t level, star;
		uint64_t id = reset_pvp_raid_level(data->pvp_raid_data.score_3, &level, &star);
			//段位不会掉回新手
		if (level != 1)
		{
			LOG_INFO("%s: player[%lu] pvp3 lv from[%u] to [%lu]", __FUNCTION__, data->player_id, data->pvp_raid_data.cur_level_id_3, id);
			data->pvp_raid_data.cur_level_id_3 = id;
			data->pvp_raid_data.level_3 = level;
			data->pvp_raid_data.star_3 = star;
		}
		refresh_player_redis_info(false);
	}
	else
	{
//			//新手段位不掉分
//		if (data->pvp_raid_data.level_5 == 1)
//			return (0);
		assert(data->pvp_raid_data.level_5 != 1);

		data->pvp_raid_data.score_5 -= value;
		uint8_t level, star;
		uint64_t id = reset_pvp_raid_level(data->pvp_raid_data.score_5, &level, &star);
			//段位不会掉回新手
		if (level != 1)
		{
			LOG_INFO("%s: player[%lu] pvp5 lv from[%u] to [%lu]", __FUNCTION__, data->player_id, data->pvp_raid_data.cur_level_id_5, id);
			data->pvp_raid_data.cur_level_id_5 = id;
			data->pvp_raid_data.level_5 = level;
			data->pvp_raid_data.star_5 = star;
		}
	}
	send_pvp_raid_score_changed(type);
	return (0);
}

int player_struct::change_pvp_raid_score(int type, int value)
{
	if (value == 0)
		return 0;
	if (value > 0)
		return add_pvp_raid_score(type, value);
	return sub_pvp_raid_score(type, -value);
}

int player_struct::add_today_pvp_win_num(int type)
{
	if (type == PVP_TYPE_DEFINE_3)
	{
		++data->pvp_raid_data.oneday_win_num_3;
	}
	else
	{
		++data->pvp_raid_data.oneday_win_num_5;
	}
	return (0);
}

void player_struct::update_weapon_skin(bool isNty)
{
	return ;
	EquipInfo *weapon = get_equip(ET_WEAPON);
	if (!weapon || weapon->stair == 0)
	{
		return;
	}

	data->attrData[PLAYER_ATTR_WEAPON] = weapon->stair;

	AttrMap attrs;
	attrs[PLAYER_ATTR_WEAPON] = data->attrData[PLAYER_ATTR_WEAPON];

	if (isNty)
	{
		notify_attr(attrs, true, true);
	}
}

int player_struct::send_mail(uint32_t type, char *title, char *sender_name, char *content, std::vector<char *> *args, std::map<uint32_t, uint32_t> *attachs, uint32_t statis_id)
{
	if (get_entity_type(data->player_id) == ENTITY_TYPE_AI_PLAYER)
		return 0;

	return ::send_mail(&conn_node_gamesrv::connecter, data->player_id, type, title, sender_name, content, args, attachs, statis_id);
	// EXTERN_DATA ext_data;
	// ext_data.player_id = data->player_id;

	// MailDBInfo resp;
	// mail_dbinfo__init(&resp);

	// MailAttach attach_data[MAX_MAIL_ATTACH_NUM];
	// MailAttach* attach_data_point[MAX_MAIL_ATTACH_NUM];

	// resp.type = type;
	// resp.title = title;
	// resp.sendername = sender_name;
	// resp.content = content;
	// if (args)
	// {
	//	resp.args = &(*args)[0];
	//	resp.n_args = args->size();
	// }

	// if (attachs)
	// {
	//	resp.n_attach = 0;
	//	resp.attach = attach_data_point;
	//	std::map<uint32_t, uint32_t>::iterator iter = attachs->begin();
	//	for (int j = 0; j < MAX_MAIL_ATTACH_NUM && iter != attachs->end(); ++j, ++iter)
	//	{
	//		attach_data_point[resp.n_attach] = &attach_data[resp.n_attach];
	//		mail_attach__init(&attach_data[resp.n_attach]);
	//		attach_data[resp.n_attach].id = iter->first;
	//		attach_data[resp.n_attach].num = iter->second;
	//		resp.n_attach++;
	//	}
	//	resp.statisid = statis_id;
	// }

	// PROTO_MAIL_INSERT* proto = (PROTO_MAIL_INSERT*)conn_node_base::get_send_buf(SERVER_PROTO_MAIL_INSERT, 0);
	// proto->player_id = data->player_id;
	// size_t pack_size = mail_dbinfo__pack(&resp, (uint8_t*)proto->data);
	// if (pack_size != (size_t)-1)
	// {
	//	proto->head.len = ENDION_FUNC_4(sizeof(PROTO_MAIL_INSERT) + pack_size);
	//	proto->data_size = pack_size;
	//	conn_node_base::add_extern_data(&proto->head, &ext_data);
	//	int ret = conn_node_gamesrv::connecter.send_one_msg(&proto->head, 1);
	//	if (ret != (int)ENDION_FUNC_4(proto->head.len))
	//	{
	//		LOG_ERR("[%s:%d] send to mail_srv failed err[%d]", __FUNCTION__, __LINE__, errno);
	//		return -1;
	//	}
	// }

	// return 0;
}

int player_struct::send_mail_by_id(uint32_t type, std::vector<char *> *args, std::map<uint32_t, uint32_t> *attachs, uint32_t statis_id)
{
	if (get_entity_type(data->player_id) == ENTITY_TYPE_AI_PLAYER)
		return 0;

	if (attachs != NULL && attachs->size() > MAX_MAIL_ATTACH_NUM)
	{
		size_t attach_num = attachs->size();
		size_t count = 0;
		std::map<uint32_t, uint32_t> send_map;
		for (std::map<uint32_t,uint32_t>::iterator iter = attachs->begin(); iter != attachs->end(); ++iter)
		{
			send_map[iter->first] = iter->second;
			count++;
			if (send_map.size() == MAX_MAIL_ATTACH_NUM || count == attach_num)
			{
				send_mail(type, NULL, NULL, NULL, args, &send_map, statis_id);
				send_map.clear();
			}
		}
	}
	else
	{
		send_mail(type, NULL, NULL, NULL, args, attachs, statis_id);
	}

	return 0;
}

int player_struct::init_yuqidao_mai(uint32_t break_id, bool isNty)
{
	YuqidaoMaiOpenNotify nty;
	yuqidao_mai_open_notify__init(&nty);

	YuqidaoMaiData mai_data[MAX_YUQIDAO_MAI_NUM];
	YuqidaoMaiData* mai_data_point[MAX_YUQIDAO_MAI_NUM];

	nty.n_mais = 0;
	nty.mais = mai_data_point;
	for (std::map<uint64_t, struct PulseTable*>::iterator iter = yuqidao_jingmai_config.begin(); iter != yuqidao_jingmai_config.end(); ++iter)
	{
		PulseTable *config = iter->second;
		if (config->BreakCondition != break_id)
		{
			continue;
		}

		int idx = -1;
		for (int i = 0; i < MAX_YUQIDAO_MAI_NUM; ++i)
		{
			YuqidaoMaiInfo &info = data->yuqidao_mais[i];
			if (info.mai_id == 0 || (info.mai_id > 0 && info.mai_id == (uint32_t)config->ID))
			{
				idx = i;
				break;
			}
		}

		if (idx < 0)
		{
			LOG_ERR("[%s:%d] player[%lu] yuqidao mai memory not enough, mai_id:%lu", __FUNCTION__, __LINE__, data->player_id, config->ID);
			continue;
		}

		if (data->yuqidao_mais[idx].mai_id > 0) //已经开启
		{
			continue;
		}

		data->yuqidao_mais[idx].mai_id = config->ID;
		data->yuqidao_mais[idx].acupoint_id = config->AcupunctureType;
		data->yuqidao_mais[idx].fill_lv = 0;

		mai_data_point[nty.n_mais] = &mai_data[nty.n_mais];
		yuqidao_mai_data__init(&mai_data[nty.n_mais]);
		mai_data[nty.n_mais].maiid = data->yuqidao_mais[idx].mai_id;
		mai_data[nty.n_mais].acupointid = data->yuqidao_mais[idx].acupoint_id;
		mai_data[nty.n_mais].filllv = data->yuqidao_mais[idx].fill_lv;
		nty.n_mais++;
	}

	if (isNty && nty.n_mais > 0)
	{
		EXTERN_DATA ext_data;
		ext_data.player_id = data->player_id;
		fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_YUQIDAO_MAI_OPEN_NOTIFY, yuqidao_mai_open_notify__pack, nty);
	}

	return 0;
}

YuqidaoMaiInfo *player_struct::get_yuqidao_mai(uint32_t mai_id)
{
	for (int i = 0; i < MAX_YUQIDAO_MAI_NUM; ++i)
	{
		YuqidaoMaiInfo &info = data->yuqidao_mais[i];
		if (info.mai_id > 0 && info.mai_id == mai_id)
		{
			return &info;
		}
	}

	return NULL;
}

int player_struct::init_yuqidao_break(uint32_t break_id)
{
	int idx = -1;
	for (int i = 0; i < MAX_YUQIDAO_BREAK_NUM; ++i)
	{
		YuqidaoBreakInfo &info = data->yuqidao_breaks[i];
		if (info.id == 0 || (info.id > 0 && info.id == break_id))
		{
			idx = i;
			break;
		}
	}

	if (idx < 0)
	{
		LOG_ERR("[%s:%d] player[%lu] yuqidao break memory not enough, break_id:%u", __FUNCTION__, __LINE__, data->player_id, break_id);
		return -1;
	}

	if (data->yuqidao_breaks[idx].id > 0) //已经开启
	{
		return -1;
	}

	memset(&data->yuqidao_breaks[idx], 0, sizeof(YuqidaoBreakInfo));
	data->yuqidao_breaks[idx].id = break_id;

	return 0;
}

YuqidaoBreakInfo *player_struct::get_yuqidao_break(uint32_t break_id)
{
	for (int i = 0; i < MAX_YUQIDAO_BREAK_NUM; ++i)
	{
		YuqidaoBreakInfo &info = data->yuqidao_breaks[i];
		if (info.id > 0 && info.id == break_id)
		{
			return &info;
		}
	}

	return NULL;
}

BaguapaiDressInfo *player_struct::get_baguapai_dress(uint32_t style_id)
{
	return (style_id == 0 || style_id > MAX_BAGUAPAI_STYLE_NUM ? NULL : &data->baguapai_dress[style_id - 1]);
}

BaguapaiCardInfo *player_struct::get_baguapai_card(uint32_t style_id, uint32_t part_id)
{
	if (style_id > 0 && style_id <= MAX_BAGUAPAI_STYLE_NUM && part_id > 0 && part_id <= MAX_BAGUAPAI_DRESS_NUM)
	{
		return &data->baguapai_dress[style_id - 1].card_list[part_id - 1];
	}

	return NULL;
}

int player_struct::generate_baguapai_main_attr(uint32_t card_id, double &attr_val)
{
	BaguaTable *config = get_config_by_id(card_id, &bagua_config);
	if (!config)
	{
		LOG_ERR("[%s:%d] player[%lu] get bagua config failed, id:%u", __FUNCTION__, __LINE__, data->player_id, card_id);
		return -1;
	}

	if (config->PrimaryAttributeCeiling < config->PrimaryAttributeLimit)
	{
		LOG_ERR("[%s:%d] player[%lu] bagua main attr range error, id:%u", __FUNCTION__, __LINE__, data->player_id, card_id);
		return -1;
	}

	double range_val = config->PrimaryAttributeCeiling - config->PrimaryAttributeLimit;
	if (range_val > 1.00)
	{
		uint32_t rand_num = rand() % (uint32_t)(range_val + 1);
		attr_val = config->PrimaryAttributeLimit + rand_num;
	}
	else //随机0-1的小数
	{
		uint32_t rand_num = rand() % (uint32_t)(range_val * 10000 + 1);
		attr_val = config->PrimaryAttributeCeiling + (double)rand_num / 10000.0;
	}

	return 0;
}

int player_struct::generate_baguapai_minor_attr(uint32_t card_id, AttrInfo *attrs)
{
	BaguaTable *card_config = get_config_by_id(card_id, &bagua_config);
	if (!card_config)
	{
		LOG_ERR("[%s:%d] player[%lu] get bagua config failed, id:%u", __FUNCTION__, __LINE__, data->player_id, card_id);
		return -1;
	}

	uint32_t attr_num = 0;
	uint32_t rand_num = rand() % card_config->ViceAttributeEntry[card_config->n_ViceAttributeEntry - 1];
	for (uint32_t i = 0; i < card_config->n_ViceAttributeEntry; ++i)
	{
		if (rand_num < card_config->ViceAttributeEntry[i])
		{
			attr_num = i + 1;
			break;
		}
	}

	AttrInfo tmp_attr[MAX_BAGUAPAI_MINOR_ATTR_NUM];
	memset(tmp_attr, 0, sizeof(AttrInfo) * MAX_BAGUAPAI_MINOR_ATTR_NUM);
	for (uint32_t i = 0; i < attr_num && i < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++i)
	{
		rand_num = rand() % card_config->n_ViceAttributeType;
		uint32_t attr_type = card_config->ViceAttributeType[rand_num];

		BaguaViceAttributeTable *attr_config = get_config_by_id(attr_type, &bagua_vice_attr_config);
		if (!attr_config)
		{
			LOG_ERR("[%s:%d] player[%lu] get attr config failed, card_id:%u, attr_id:%u", __FUNCTION__, __LINE__, data->player_id, card_id, attr_type);
			return -1;
		}

		if (attr_config->n_Rand < card_config->BaguaQuality)
		{
			LOG_ERR("[%s:%d] player[%lu] attr rand size error, card_id:%u, attr_id:%u", __FUNCTION__, __LINE__, data->player_id, card_id, attr_type);
			return -1;
		}

		double lower_val = (card_config->BaguaQuality == 1 ? 1 : attr_config->Rand[card_config->BaguaQuality - 2]) * card_config->coefficient;
		double upper_val = attr_config->Rand[card_config->BaguaQuality - 1] * card_config->coefficient;
		if (upper_val < lower_val)
		{
			LOG_ERR("[%s:%d] player[%lu] attr rand range error, card_id:%u, attr_id:%u", __FUNCTION__, __LINE__, data->player_id, card_id, attr_type);
			return -1;
		}

		double range_val = upper_val - lower_val;
		double attr_val = 0;
		if (range_val > 1.00)
		{
			rand_num = rand() % (uint32_t)(range_val + 1);
			attr_val = lower_val + rand_num;
		}
		else //随机0-1的小数
		{
			rand_num = rand() % (uint32_t)(range_val * 10000 + 1);
			attr_val = lower_val + (double)rand_num / 10000.0;
		}

		tmp_attr[i].id = attr_type;
		tmp_attr[i].val = attr_val;
	}

	memcpy(attrs, tmp_attr, sizeof(AttrInfo) * MAX_BAGUAPAI_MINOR_ATTR_NUM);

	return 0;
}

int player_struct::add_activeness(uint32_t num, uint32_t statis_id, bool isNty)
{
	if (num == 0)
	{
		return 0;
	}

	uint32_t prevVal = data->attrData[PLAYER_ATTR_ACTIVENESS];
	data->attrData[PLAYER_ATTR_ACTIVENESS] = std::min(prevVal + num, UINT32_MAX);
	uint32_t curVal = data->attrData[PLAYER_ATTR_ACTIVENESS];
	uint32_t realNum = curVal - prevVal;
	LOG_INFO("[%s:%d] player[%lu] prevVal:%u, curVal:%u, num:%u, realNum:%u", __FUNCTION__, __LINE__, data->player_id, prevVal, curVal, num, realNum);

	if (isNty)
	{
		AttrMap attrs;
		attrs[PLAYER_ATTR_ACTIVENESS] = data->attrData[PLAYER_ATTR_ACTIVENESS];
		this->notify_attr(attrs);
	}

	return 0;
}

uint32_t player_struct::get_activeness(void)
{
	return (data ? data->attrData[PLAYER_ATTR_ACTIVENESS] : 0);
}

static bool activity_is_unlock_by_config(player_struct *player, EventCalendarTable *config)
{
	switch (config->SubtabCondition)
	{
		case 1: //等级
			{
				uint32_t need_level = config->SubtabValue;
				uint32_t player_level = player->get_attr(PLAYER_ATTR_LEVEL);
				if (player_level < need_level)
				{
					return false;
				}
			}
			break;
		case 2: //任务
			{
				uint32_t need_task = config->SubtabValue;
				return player->task_is_finish(need_task);
			}
			break;
		case 3: //登陆天数
			{
				return false;
			}
			break;
	}

	return true;
}

bool player_struct::activity_is_unlock(uint32_t act_id)
{
	EventCalendarTable *config = get_config_by_id(act_id, &activity_config);
	if (!config)
	{
		return false;
	}

	return activity_is_unlock_by_config(this, config);
}

int player_struct::check_activity_progress(uint32_t matter, uint32_t value)
{
	for (std::map<uint64_t, EventCalendarTable*>::iterator iter = activity_config.begin(); iter != activity_config.end(); ++iter)
	{
		EventCalendarTable *config = iter->second;
		if (!activity_is_unlock_by_config(this, config))
		{
			continue;
		}

		if (config->ActivityType == 0 || config->ActivityType != matter)
		{
			continue;
		}

		if (config->ActivityValue > 0 && config->ActivityValue != value)
		{
			continue;
		}

		add_task_progress(TCT_ACTIVITY, config->ID, 1);
		if (config->Active > 0)
		{
			if (config->Sum == 0)
			{ //无限次数
				add_activeness(config->Active, MAGIC_TYPE_ACTIVITY_REWARD);
			}
			else
			{
				DailyActivityInfo *pDaily = NULL;
				for (int i = 0; i < MAX_DAILY_ACTIVITY_NUM; ++i)
				{
					if (data->daily_activity[i].act_id == (uint32_t)config->ID || data->daily_activity[i].act_id == 0)
					{
						pDaily = &data->daily_activity[i];
						break;
					}
				}

				if (pDaily && pDaily->count < (uint32_t)config->Sum)
				{
					if (pDaily->act_id == 0)
					{
						pDaily->act_id = config->ID;
					}
					pDaily->count++;
					update_daily_activity_item(pDaily);
					add_activeness(config->Active, MAGIC_TYPE_ACTIVITY_REWARD);
				}
			}
		}

		if (config->ChivalrousID > 0)
		{
			activity_finish_check_chivalry(config->ChivalrousID);
		}
	}

	return 0;
}

int player_struct::activity_finish_check_chivalry(uint32_t chivalry_id)
{
	ChivalrousTable *chivalry_config = get_config_by_id(chivalry_id, &activity_chivalry_config);
	if (!chivalry_config)
	{
		return -1;
	}

	ChivalryActivityInfo *pChivalry = NULL;
	for (int i = 0; i < MAX_CHIVALRY_ACTIVITY_NUM; ++i)
	{
		if (data->chivalry_activity[i].act_id == chivalry_id || data->chivalry_activity[i].act_id == 0)
		{
			pChivalry = &data->chivalry_activity[i];
			break;
		}
	}

	if (!pChivalry)
	{
		return -1;
	}

	if (pChivalry->val >= (uint32_t)chivalry_config->MaxNum)
	{
		return 0;
	}

	if (!m_team)
	{
		return -1;
	}

	uint32_t min_level = 0;
	for (int i = 0; i < m_team->m_data->m_memSize; ++i)
	{
		player_struct *tmp_player = player_manager::get_player_by_id(m_team->m_data->m_mem[i].id);
		if (!tmp_player)
		{
			continue;
		}

		if (min_level == 0)
		{
			min_level = tmp_player->get_level();
		}
		else
		{
			min_level = std::min(min_level, tmp_player->get_level());
		}
	}

	uint32_t player_level = get_level();
	if (player_level >= min_level && player_level - min_level >= (uint32_t)chivalry_config->Condition1)
	{
		if (pChivalry->act_id == 0)
		{
			pChivalry->act_id = chivalry_id;
		}
		pChivalry->val += chivalry_config->SingleNum;
		update_chivalry_activity_item(pChivalry);
		add_chivalry(chivalry_config->SingleNum, MAGIC_TYPE_ACTIVITY_REWARD);
		add_task_progress(TCT_ACTIVITY, chivalry_id, 1);
	}

	return 0;
}

int player_struct::add_chivalry(uint32_t num, uint32_t statis_id, bool isNty)
{
	if (num == 0)
	{
		return 0;
	}

	uint32_t prevVal = data->attrData[PLAYER_ATTR_CHIVALRY];
	data->attrData[PLAYER_ATTR_CHIVALRY] = std::min(prevVal + num, UINT32_MAX);
	uint32_t curVal = data->attrData[PLAYER_ATTR_CHIVALRY];
	uint32_t realNum = curVal - prevVal;
	LOG_INFO("[%s:%d] player[%lu] prevVal:%u, curVal:%u, num:%u, realNum:%u", __FUNCTION__, __LINE__, data->player_id, prevVal, curVal, num, realNum);

	if (isNty)
	{
		AttrMap attrs;
		attrs[PLAYER_ATTR_CHIVALRY] = data->attrData[PLAYER_ATTR_CHIVALRY];
		this->notify_attr(attrs);
	}

	return 0;
}

int player_struct::sub_chivalry(uint32_t num, uint32_t statis_id, bool isNty)
{
	if (num == 0)
	{
		return 0;
	}

	uint32_t prevVal = data->attrData[PLAYER_ATTR_CHIVALRY];
	if (prevVal < num)
	{
		LOG_ERR("[%s:%d] player[%lu] chivalry not enough, prevVal:%u, num:%u", __FUNCTION__, __LINE__, data->player_id, prevVal, num);
		return -1;
	}

	data->attrData[PLAYER_ATTR_CHIVALRY] -= num;
	uint32_t curVal = data->attrData[PLAYER_ATTR_CHIVALRY];
	LOG_INFO("[%s:%d] player[%lu] prevVal:%u, curVal:%u, num:%u", __FUNCTION__, __LINE__, data->player_id, prevVal, curVal, num);

	if (isNty)
	{
		AttrMap attrs;
		attrs[PLAYER_ATTR_CHIVALRY] = data->attrData[PLAYER_ATTR_CHIVALRY];
		this->notify_attr(attrs);
	}

	return 0;
}

uint32_t player_struct::get_chivalry(void)
{
	return (data ? (uint32_t)data->attrData[PLAYER_ATTR_CHIVALRY] : 0);
}

void player_struct::update_daily_activity_item(DailyActivityInfo *info)
{
	DailyActivityData act_data;
	daily_activity_data__init(&act_data);

	act_data.id = info->act_id;
	act_data.count = info->count;

	EXTERN_DATA ext_data;
	ext_data.player_id = data->player_id;

	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_ACTIVITY_UPDATE_DAILY_NOTIFY, daily_activity_data__pack, act_data);
}

void player_struct::update_chivalry_activity_item(ChivalryActivityInfo *info)
{
	ChivalryActivityData act_data;
	chivalry_activity_data__init(&act_data);

	act_data.id = info->act_id;
	act_data.val = info->val;

	EXTERN_DATA ext_data;
	ext_data.player_id = data->player_id;

	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_ACTIVITY_UPDATE_CHIVALRY_NOTIFY, chivalry_activity_data__pack, act_data);
}

void player_struct::refresh_activity_daily(void)
{
	EXTERN_DATA ext_data;
	ext_data.player_id = data->player_id;

	memset(data->active_reward, 0, sizeof(data->active_reward));
	memset(data->daily_activity, 0, sizeof(data->daily_activity));
	memset(data->chivalry_activity, 0, sizeof(data->chivalry_activity));
	notify_activity_info(&ext_data);
}

void player_struct::notify_activity_info(EXTERN_DATA *extern_data)
{
	if (get_entity_type(data->player_id) == ENTITY_TYPE_AI_PLAYER)
		return;

	ActivityInfoNotify nty;
	activity_info_notify__init(&nty);

	DailyActivityData  daily_data[MAX_DAILY_ACTIVITY_NUM];
	DailyActivityData* daily_point[MAX_DAILY_ACTIVITY_NUM];
	ChivalryActivityData  chivalry_data[MAX_CHIVALRY_ACTIVITY_NUM];
	ChivalryActivityData* chivalry_point[MAX_CHIVALRY_ACTIVITY_NUM];

	nty.dailys = daily_point;
	nty.n_dailys = 0;
	for (int i = 0; i< MAX_DAILY_ACTIVITY_NUM; ++i)
	{
		if (this->data->daily_activity[i].act_id == 0)
		{
			break;
		}

		daily_point[nty.n_dailys] = &daily_data[nty.n_dailys];
		daily_activity_data__init(&daily_data[nty.n_dailys]);
		daily_data[nty.n_dailys].id = this->data->daily_activity[i].act_id;
		daily_data[nty.n_dailys].count = this->data->daily_activity[i].count;
		nty.n_dailys++;
	}

	nty.chivalrys = chivalry_point;
	nty.n_chivalrys = 0;
	for (int i = 0; i< MAX_CHIVALRY_ACTIVITY_NUM; ++i)
	{
		if (this->data->chivalry_activity[i].act_id == 0)
		{
			break;
		}

		chivalry_point[nty.n_chivalrys] = &chivalry_data[nty.n_chivalrys];
		chivalry_activity_data__init(&chivalry_data[nty.n_chivalrys]);
		chivalry_data[nty.n_chivalrys].id = this->data->chivalry_activity[i].act_id;
		chivalry_data[nty.n_chivalrys].val = this->data->chivalry_activity[i].val;
		nty.n_chivalrys++;
	}

	nty.activerewardids = data->active_reward;
	nty.n_activerewardids = 0;
	for (int i = 0; i < MAX_ACTIVE_REWARD_NUM; ++i)
	{
		if (data->active_reward[i] == 0)
		{
			break;
		}
		nty.n_activerewardids++;
	}

	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_ACTIVITY_INFO_NOTIFY, activity_info_notify__pack, nty);
}

void player_struct::add_wanyaoka(uint32_t *id, uint32_t n_id)
{
	PROTO_HEAD *proto_head;
	PROTO_HEAD *real_head;
	proto_head = (PROTO_HEAD *)conn_node_base::global_send_buf;
	proto_head->msg_id = ENDION_FUNC_2(SERVER_PROTO_GAME_TO_FRIEND);
	proto_head->seq = 0;

	real_head = (PROTO_HEAD *)proto_head->data;
	PROTO_ADD_WANYAOKA *info = (PROTO_ADD_WANYAOKA *)real_head->data;
	size_t size = sizeof(PROTO_ADD_WANYAOKA);
	real_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);
	real_head->msg_id = ENDION_FUNC_2(SERVER_PROTO_ADD_WANYAOKA);

	memset(info, 0, sizeof(*info));
	info->player_id = get_uuid();
	for (size_t i = 0; i < n_id; ++i)
		info->wanyaoka[i] = id[i];

	proto_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + real_head->len);
	if (conn_node_gamesrv::connecter.send_one_msg(proto_head, 1) != (int)(ENDION_FUNC_4(proto_head->len))) {
		LOG_ERR("%s %d: send to friend failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
}

void player_struct::add_guild_resource(uint32_t type, uint32_t num)
{
	EXTERN_DATA ext_data;
	ext_data.player_id = data->player_id;

	PROTO_HEAD *head = conn_node_base::get_send_buf(SERVER_PROTO_ADD_GUILD_RESOURCE, 0);
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + sizeof(uint32_t) * 2);
	uint32_t *pData = (uint32_t *)head->data;
	*(pData++) = type;
	*(pData++) = num;
	conn_node_base::add_extern_data(head, &ext_data);
	if (conn_node_gamesrv::connecter.send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len))
	{
		LOG_ERR("[%s:%d] send to conn_srv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
}

void player_struct::sub_guild_building_time(uint32_t time)
{
	EXTERN_DATA ext_data;
	ext_data.player_id = data->player_id;

	PROTO_HEAD *head = conn_node_base::get_send_buf(SERVER_PROTO_SUB_GUILD_BUILDING_TIME, 0);
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + sizeof(uint32_t));
	uint32_t *pData = (uint32_t *)head->data;
	*(pData++) = time;
	conn_node_base::add_extern_data(head, &ext_data);
	if (conn_node_gamesrv::connecter.send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len))
	{
		LOG_ERR("[%s:%d] send to conn_srv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
}

void player_struct::disband_guild(uint32_t guild_id)
{
	EXTERN_DATA ext_data;
	ext_data.player_id = data->player_id;

	PROTO_HEAD *head = conn_node_base::get_send_buf(SERVER_PROTO_GM_DISBAND_GUILD, 0);
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + sizeof(uint32_t));
	uint32_t *pData = (uint32_t *)head->data;
	*(pData++) = guild_id;
	conn_node_base::add_extern_data(head, &ext_data);
	if (conn_node_gamesrv::connecter.send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len))
	{
		LOG_ERR("[%s:%d] send to conn_srv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
}

int player_struct::start_escort(uint32_t escort_id)
{
	int free_idx = -1;
	for (int i = 0; i < MAX_ESCORT_NUM; ++i)
	{
		if (free_idx < 0 && data->escort_list[i].escort_id == 0)
		{
			free_idx = i;
		}
		else if (data->escort_list[i].escort_id == escort_id)
		{
			return -1;
		}
	}

	if (free_idx < 0)
	{
		return -1;
	}

	EscortInfo &info = data->escort_list[free_idx];
	memset(&data->escort_list[free_idx], 0, sizeof(EscortInfo));

	EscortTask *config = get_config_by_id(escort_id, &escort_config);
	if (!config)
	{
		return -1;
	}

	if (sight_space)
	{
		return -1;
	}

	if (!scene || scene->m_id != config->Scene)
	{
		return -1;
	}

	monster_struct *escort_monster = monster_manager::create_monster_by_config(scene, config->MonsterIndex);
	if (!escort_monster)
	{
		LOG_ERR("[%s:%d] player[%lu] create escort monster failed, escort_id:%u", __FUNCTION__, __LINE__, data->player_id, escort_id);
		return -1;
	}

	escort_monster->ai_data.escort_ai.owner_player_id = data->player_id;
	escort_monster->ai_data.escort_ai.escort_id = escort_id;
	info.escort_id = escort_id;
	info.start_time = time_helper::get_cached_time() / 1000;
	info.summon_monster_uuids[0] = escort_monster->get_uuid();
	info.summon_num = 1;

	LOG_INFO("[%s:%d] player[%lu] , escort_id:%u", __FUNCTION__, __LINE__, data->player_id, escort_id);

	return 0;
}

int player_struct::stop_escort(uint32_t escort_id, bool success)
{
	LOG_INFO("[%s:%d] player[%lu] , escort_id:%u, result:%d", __FUNCTION__, __LINE__, data->player_id, escort_id, success);

	EscortInfo *escort_info = get_escort_info(escort_id);

	if (!escort_info)
	{
		LOG_ERR("[%s:%d] player[%lu] can't find info, escort_id:%u, result:%d", __FUNCTION__, __LINE__, data->player_id, escort_id, success);
		return -1;
	}

	if (escort_info->mark_delete)
	{
		return 0;
	}

	if (success)
	{
		add_task_progress(TCT_ESCORT, escort_info->escort_id, 1);
	}
	else
	{
		for (int i = 0; i < MAX_TASK_ACCEPTED_NUM; ++i)
		{
			TaskInfo &info = data->task_list[i];
			if (info.id == 0)
			{
				continue;
			}

			for (int j = 0; j < MAX_TASK_TARGET_NUM; ++j)
			{
				TaskCountInfo &count_info = info.progress[j];
				if (count_info.id == 0)
				{
					continue;
				}

				TaskConditionTable *config = get_config_by_id(count_info.id, &task_condition_config);
				if (!config)
				{
					continue;
				}

				if ((uint32_t)config->ConditionType == TCT_ESCORT && (uint32_t)config->ConditionTarget == escort_info->escort_id)
				{
					set_task_fail(&info);
				}
			}

		}
	}

	raid_struct *raid = get_raid();
	if (raid && raid->ai && raid->ai->raid_on_escort_stop)
	{
		raid->ai->raid_on_escort_stop(raid, this, escort_info->escort_id, success);
	}

	escort_info->mark_delete = true;
	for (int i = 0; i < MAX_ESCORT_MONSTER_NUM; ++i)
	{
		monster_struct *monster = monster_manager::get_monster_by_id(escort_info->summon_monster_uuids[i]);
		if (monster)
		{
			if (monster->scene)
			{
				monster->scene->delete_monster_from_scene(monster, true);
			}
			monster->data->stop_ai = true;
		}
	}

	return 0;
}

void player_struct::stop_all_escort(void)
{
	for (int i = 0; i < MAX_ESCORT_NUM; ++i)
	{
		if (data->escort_list[i].escort_id > 0)
		{
			stop_escort(data->escort_list[i].escort_id, false);
		}
	}
}

EscortInfo *player_struct::get_escort_info(uint32_t escort_id)
{
	for (int i = 0; i < MAX_ESCORT_NUM; ++i)
	{
		if (data->escort_list[i].escort_id > 0 && data->escort_list[i].escort_id == escort_id)
		{
			return &data->escort_list[i];
		}
	}
	return NULL;
}

int player_struct::clear_escort_by_id(uint32_t escort_id)
{
	for (int i = 0; i < MAX_ESCORT_NUM; ++i)
	{
		EscortInfo &info = data->escort_list[i];
		if (info.escort_id == 0)
		{
			continue;
		}

		if (escort_id > 0 && info.escort_id != escort_id)
		{
			continue;
		}

		clear_escort_by_index(i);
	}
	return 0;
}

int player_struct::clear_escort_by_index(uint32_t idx)
{
	if (idx >= MAX_ESCORT_NUM)
	{
		return -1;
	}

	EscortInfo &info = data->escort_list[idx];

	for (uint32_t j = 0; j < info.summon_num; ++j)
	{
		monster_struct *monster = monster_manager::get_monster_by_id(info.summon_monster_uuids[j]);
		if (monster)
		{
			if (monster->scene)
			{
				monster->scene->delete_monster_from_scene(monster, true);
			}
			monster_manager::delete_monster(monster);
		}
	}

	memset(&data->escort_list[idx], 0, sizeof(EscortInfo));
	return 0;
}

void player_struct::clear_all_escort(void)
{
	for (int i = 0; i < MAX_ESCORT_NUM; ++i)
	{
		clear_escort_by_index(i);
	}
}

static void escort_summon_monster(player_struct *player, monster_struct *monster, EscortInfo *info, EscortTask *config, uint32_t idx)
{
	uint32_t n_Point = 0;
	double *Point = NULL;
	uint32_t n_MonsterID = 0;
	uint64_t *MonsterID = NULL;
	if (idx == 1)
	{
		n_Point = config->n_PointXZ1;
		Point = config->PointXZ1;
		n_MonsterID = config->n_MonsterID1;
		MonsterID = config->MonsterID1;
	}
	else if (idx == 2)
	{
		n_Point = config->n_PointXZ2;
		Point = config->PointXZ2;
		n_MonsterID = config->n_MonsterID2;
		MonsterID = config->MonsterID2;
	}
	else if (idx == 3)
	{
		n_Point = config->n_PointXZ3;
		Point = config->PointXZ3;
		n_MonsterID = config->n_MonsterID3;
		MonsterID = config->MonsterID3;
	}
	else if (idx == 4)
	{
		n_Point = config->n_PointXZ4;
		Point = config->PointXZ4;
		n_MonsterID = config->n_MonsterID4;
		MonsterID = config->MonsterID4;
	}
	else
	{
		return;
	}

	position *monster_pos = monster->get_pos();
	position target_pos;
	target_pos.pos_x = Point[0];
	target_pos.pos_z = Point[1];
	if (!(n_Point >= 2 && check_circle_in_range(monster_pos, &target_pos, 1)))
	{
		return;
	}

	int empty_idx = -1;
	bool find_mark = false;
	for (int i = 0; i < MAX_ESCORT_MONSTER_WAVE; ++i)
	{
		uint32_t wave = info->summon_monster_waves[i];
		if (empty_idx < 0 && wave == 0)
		{
			empty_idx = i;
		}
		else if (wave == idx)
		{
			find_mark = true;
			break;
		}
	}

	if (find_mark)
	{
		return;
	}
	if (empty_idx < 0)
	{
		return;
	}

	info->summon_monster_waves[empty_idx] = idx;
	for (uint32_t i = 0; i < n_MonsterID && info->summon_num < MAX_ESCORT_MONSTER_NUM; ++i)
	{
		uint64_t tmp_id = MonsterID[i];
		TaskMonsterTable *create_config = get_config_by_id(tmp_id, &task_monster_config);
		if (!create_config)
		{
			continue;
		}

		monster_struct *summon_monster = monster_manager::create_monster_at_pos(monster->scene, create_config->MonsterID, create_config->MonsterLevel, create_config->PointX, create_config->PointZ, 0, NULL);
		if (!summon_monster)
		{
			continue;
		}

		info->summon_monster_uuids[info->summon_num] = summon_monster->get_uuid();
		info->summon_num++;
	}

	//触发冒泡
	for (uint32_t i = 0; i < config->n_talkID; ++i)
	{
		NpcTalkTable *talk_config = get_config_by_id(config->talkID[i], &monster_talk_config);
		if (!talk_config)
		{
			continue;
		}

		if (talk_config->Type != 3)
		{
			continue;
		}

		if (talk_config->EventNum1 == (uint64_t)idx)
		{
			MonsterTalkNotify nty;
			monster_talk_notify__init(&nty);
			nty.uuid = monster->get_uuid();
			nty.talkid = talk_config->ID;
			monster->broadcast_to_sight(MSG_ID_MONSTER_TALK_NOTIFY, &nty, (pack_func)monster_talk_notify__pack, false);
		}
	}
}

void player_struct::check_escort(void)
{
	uint32_t now = time_helper::get_cached_time() / 1000;
	for (int i = 0; i < MAX_ESCORT_NUM; ++i)
	{
		EscortInfo *info = &data->escort_list[i];
		if (info->escort_id == 0)
		{
			continue;
		}
		if (info->mark_delete)
		{
			clear_escort_by_index(i);
			continue;
		}

		do
		{
			monster_struct *monster = monster_manager::get_monster_by_id(info->summon_monster_uuids[0]);
			if (!monster)
			{
				stop_escort(info->escort_id, false);
				clear_escort_by_index(i);
				break;
			}

			EscortTask *config = get_config_by_id(info->escort_id, &escort_config);
			if (!config)
			{
				break;
			}

			//检查护送时间
			if (config->Time != 0 && info->start_time + (uint32_t)config->Time <= now)
			{
				this->stop_escort(info->escort_id, false);
				break;
			}

			position *monster_pos = monster->get_pos();
			//检查距离过远
			if (check_circle_in_range(this->get_pos(), monster_pos, config->Distance) == false)
			{
				if (info->too_far_time == 0)
				{
					info->too_far_time = now;
				}
				else if (now - info->too_far_time >= 10)
				{
					this->stop_escort(info->escort_id, false);
					break;
				}
			}
			else
			{
				if (info->too_far_time != 0)
				{
					info->too_far_time = 0;
				}
			}

			//检查召唤
			for (int i = 0; i < 4; ++i)
			{
				escort_summon_monster(this, monster, info, config, i);
			}
		} while(0);
	}
}

partner_struct *player_struct::get_battle_partner()
{
	if (data->partner_battle[0] == 0)
		return NULL;
	partner_struct *ret = get_partner_by_uuid(data->partner_battle[0]);
	if (!ret || !ret->scene || !ret->is_alive())
		return (NULL);
	return ret;
}

int player_struct::add_partner(uint32_t partner_id, uint64_t *uuid)
{
	PartnerTable *config = get_config_by_id(partner_id, &partner_config);
	if (!config)
	{
		return -1;
	}

	if (m_partners.size() >= (size_t)MAX_PARTNER_NUM)
	{
		return -1;
	}

	partner_struct *partner = partner_manager::create_partner(partner_id, this);
	if (!partner)
	{
		return -1;
	}

	partner->init_create_data();
	partner->init_end(false);
	m_partners[partner->data->uuid] = partner;
	if (uuid)
	{
		*uuid = partner->data->uuid;
	}
	add_partner_dictionary(partner_id);

	PartnerData partner_data;
	partner_data__init(&partner_data);

	PartnerAttr partner_cur_attr;
	PartnerAttr partner_cur_flash;
	AttrData  attr_data[MAX_PARTNER_ATTR];
	AttrData* attr_point[MAX_PARTNER_ATTR];
	PartnerSkillData  skill_data[MAX_PARTNER_SKILL_NUM];
	PartnerSkillData* skill_point[MAX_PARTNER_SKILL_NUM];
	PartnerSkillData  partner_skill_data_flash[MAX_PARTNER_SKILL_NUM];
	PartnerSkillData* partner_skill_point_flash[MAX_PARTNER_SKILL_NUM];

	partner_data.uuid = partner->data->uuid;
	partner_data.partnerid = partner->data->partner_id;

	uint32_t attr_num = 0;
	for (int i = 1; i < MAX_PARTNER_ATTR; ++i)
	{
		attr_point[attr_num] = &attr_data[attr_num];
		attr_data__init(&attr_data[attr_num]);
		attr_data[attr_num].id = i;
		attr_data[attr_num].val = partner->data->attrData[i];
		attr_num++;
	}
	partner_data.attrs = attr_point;
	partner_data.n_attrs = attr_num;

	uint32_t skill_num = 0;
	if (partner->data->attr_cur.base_attr_id[0] != 0)
	{
		partner_data.cur_attr = &partner_cur_attr;
		partner_attr__init(&partner_cur_attr);
		for (int i = 0; i < MAX_PARTNER_SKILL_NUM; ++i)
		{
			skill_point[skill_num] = &skill_data[skill_num];
			partner_skill_data__init(&skill_data[skill_num]);
			skill_data[skill_num].id = partner->data->attr_cur.skill_list[i].skill_id;
			skill_data[skill_num].lv = partner->data->attr_cur.skill_list[i].lv;
			skill_num++;
		}
		partner_cur_attr.skills = skill_point;
		partner_cur_attr.n_skills = skill_num;
		partner_cur_attr.base_attr_id = partner->data->attr_cur.base_attr_id;
		partner_cur_attr.n_base_attr_id = MAX_PARTNER_BASE_ATTR;
		partner_cur_attr.base_attr_cur = partner->data->attr_cur.base_attr_vaual;
		partner_cur_attr.n_base_attr_cur = MAX_PARTNER_BASE_ATTR;
		partner_cur_attr.base_attr_up = partner->data->attr_cur.base_attr_up;
		partner_cur_attr.n_base_attr_up = MAX_PARTNER_BASE_ATTR;
		partner_cur_attr.base_attr_up = partner->data->attr_cur.base_attr_up;
		partner_cur_attr.n_base_attr_up = MAX_PARTNER_BASE_ATTR;
		partner_cur_attr.detail_attr_id = partner->data->attr_cur.detail_attr_id;
		partner_cur_attr.n_detail_attr_id = partner->data->attr_cur.n_detail_attr;
		partner_cur_attr.detail_attr_cur = partner->data->attr_cur.detail_attr_vaual;
		partner_cur_attr.n_detail_attr_cur = partner->data->attr_cur.n_detail_attr;
		partner_cur_attr.type = partner->data->attr_cur.type;
	}

	if (partner->data->attr_flash.base_attr_id[0] != 0)
	{
		partner_data.flash_attr = &partner_cur_flash;
		partner_attr__init(&partner_cur_flash);
		skill_num = 0;
		for (int i = 0; i < MAX_PARTNER_SKILL_NUM; ++i)
		{
			partner_skill_point_flash[skill_num] = &partner_skill_data_flash[skill_num];
			partner_skill_data__init(&partner_skill_data_flash[skill_num]);
			partner_skill_data_flash[skill_num].id = partner->data->attr_flash.skill_list[i].skill_id;
			partner_skill_data_flash[skill_num].lv = partner->data->attr_flash.skill_list[i].lv;
			skill_num++;
		}
		partner_cur_flash.skills = partner_skill_point_flash;
		partner_cur_flash.n_skills = skill_num;
		partner_cur_flash.base_attr_id = partner->data->attr_flash.base_attr_id;
		partner_cur_flash.n_base_attr_id = MAX_PARTNER_BASE_ATTR;
		partner_cur_flash.base_attr_cur = partner->data->attr_flash.base_attr_vaual;
		partner_cur_flash.n_base_attr_cur = MAX_PARTNER_BASE_ATTR;
		partner_cur_flash.base_attr_up = partner->data->attr_flash.base_attr_up;
		partner_cur_flash.n_base_attr_up = MAX_PARTNER_BASE_ATTR;
		partner_cur_flash.base_attr_up = partner->data->attr_flash.base_attr_up;
		partner_cur_flash.n_base_attr_up = MAX_PARTNER_BASE_ATTR;
		partner_cur_flash.detail_attr_id = partner->data->attr_flash.detail_attr_id;
		partner_cur_flash.n_detail_attr_id = partner->data->attr_flash.n_detail_attr;
		partner_cur_flash.detail_attr_cur = partner->data->attr_flash.detail_attr_vaual;
		partner_cur_flash.n_detail_attr_cur = partner->data->attr_flash.n_detail_attr;
		partner_cur_flash.type = partner->data->attr_flash.type;
	}

	partner_data.god_id = partner->data->god_id;
	partner_data.n_god_id = partner->data->n_god;
	partner_data.god_level = partner->data->god_level;
	partner_data.n_god_level = partner->data->n_god;

	EXTERN_DATA ext_data;
	ext_data.player_id = data->player_id;

	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_PARTNER_ADD_NOTIFY, partner_data__pack, partner_data);

	return 0;
}

int player_struct::add_partner_dictionary(uint32_t partner_id)
{
	int free_idx = -1;
	for (int i = 0; i < MAX_PARTNER_TYPE; ++i)
	{
		if (data->partner_dictionary[i] == 0)
		{
			free_idx = i;
			break;
		}
		else if (data->partner_dictionary[i] == partner_id)
		{
			return 0;
		}
	}

	if (free_idx < 0)
	{
		return -1;
	}

	data->partner_dictionary[free_idx] = partner_id;

	PartnerDictionaryAddNotify nty;
	partner_dictionary_add_notify__init(&nty);

	nty.partnerid = partner_id;

	EXTERN_DATA ext_data;
	ext_data.player_id = data->player_id;

	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_PARTNER_DICTIONARY_ADD_NOTIFY, partner_dictionary_add_notify__pack, nty);

	return 0;
}

int player_struct::remove_partner(uint64_t partner_uuid)
{
	partner_struct *partner = get_partner_by_uuid(partner_uuid);
	if (!partner)
	{
		return -1;
	}

	m_partners.erase(partner_uuid);
	partner_manager::delete_partner(partner);

	return 0;
}

void player_struct::clear_all_partners(void)
{
	for (PartnerMap::iterator iter = m_partners.begin(); iter != m_partners.end(); ++iter)
	{
		partner_struct *partner = iter->second;
		if (partner)
		{
			partner_manager::delete_partner(partner);
		}
	}
	m_partners.clear();
}

partner_struct *player_struct::get_partner_by_uuid(uint64_t partner_uuid)
{
	PartnerMap::iterator iter = m_partners.find(partner_uuid);
	if (iter != m_partners.end())
	{
		return iter->second;
	}

	return NULL;
}

bool player_struct::partner_is_in_formation(uint64_t partner_uuid)
{
	for (int i = 0; i < MAX_PARTNER_FORMATION_NUM; ++i)
	{
		if (data->partner_formation[i] == partner_uuid)
		{
			return true;
		}
	}
	return false;
}

bool player_struct::partner_is_in_battle(uint64_t partner_uuid)
{
	for (int i = 0; i < MAX_PARTNER_BATTLE_NUM; ++i)
	{
		if (data->partner_battle[i] == partner_uuid)
		{
			return true;
		}
	}
	return false;
}

void player_struct::load_partner_end(void)
{
	for (PartnerMap::iterator iter = m_partners.begin(); iter != m_partners.end(); ++iter)
	{
		partner_struct *partner = iter->second;
		partner->init_end(false);
	}
}

bool player_struct::is_partner_battle(void)
{
	return ((int)get_attr(PLAYER_ATTR_PARTNER_FIGHT) != 0);
}

bool player_struct::is_partner_precedence(void)
{
	return ((int)get_attr(PLAYER_ATTR_PARTNER_PRECEDENCE) != 0);
}

int player_struct::del_partner_from_scene(partner_struct *partner, bool send_msg)
{
	if (partner->scene)
	{
		partner->scene->delete_partner_from_scene(partner, send_msg);
	}
	if (is_node_in_heap(&partner_manager_minheap, partner))
		partner_manager::partner_ontick_delete(partner);
	return (0);
}

int player_struct::del_partner_from_scene(uint64_t partner_uuid, bool send_msg)
{
	partner_struct *partner = get_partner_by_uuid(partner_uuid);
	if (!partner)
		return (-1);
	return del_partner_from_scene(partner, send_msg);
}

int player_struct::add_partner_to_scene(uint64_t partner_uuid)
{
	if (!scene)
	{
		return -1;
	}

	partner_struct *partner = get_partner_by_uuid(partner_uuid);
	if (!partner)
	{
		return -1;
	}

	// if (partner->scene)
	// {
	//	partner->scene->delete_partner_from_scene(partner, true);
	// }
	del_partner_from_scene(partner, true);

	struct position pos;
	partner->calc_target_pos(&pos);
	partner->set_pos(pos.pos_x, pos.pos_z);
	partner->data->attrData[PLAYER_ATTR_PK_TYPE] = partner->m_owner->get_attr(PLAYER_ATTR_PK_TYPE);
	partner->data->attrData[PLAYER_ATTR_ZHENYING] = partner->m_owner->get_attr(PLAYER_ATTR_ZHENYING);
	scene->add_partner_to_scene(partner);

		//开启定时器
	partner->set_timer(time_helper::get_cached_time() + 1000 + random() % 1500);

	return 0;
}

void player_struct::take_partner_into_scene(void)
{
	if (!is_partner_battle())
	{
		return ;
	}

	for (int i = 0; i < MAX_PARTNER_BATTLE_NUM; ++i)
	{
		if (data->partner_battle[i] > 0)
		{
			add_partner_to_scene(data->partner_battle[i]);
		}
	}
}

void player_struct::del_battle_partner_from_scene()
{
	for (int i = 0; i < MAX_PARTNER_BATTLE_NUM; ++i)
	{
		if (data->partner_battle[i] > 0)
		{
			del_partner_from_scene(data->partner_battle[i], false);
//			add_partner_to_scene(data->partner_battle[i]);
		}
	}
}

void player_struct::adjust_battle_partner(void)
{
//	LOG_DEBUG("[%s:%u] player[%lu] ", __FUNCTION__, __LINE__, data->player_id);
	uint64_t old_uuid = get_fighting_partner();
	do
	{
		//在角色上坐骑、上镖车、死亡的时候，要把伙伴隐藏起来
		if (!is_partner_battle() || is_on_horse() || is_on_truck() || !is_alive())
		{
			for (int i = 0; i < MAX_PARTNER_BATTLE_NUM; ++i)
			{
				if (data->partner_battle[i] == 0)
				{
					continue;
				}

				del_partner_from_scene(data->partner_battle[i], true);
				// partner_struct *partner = get_partner_by_uuid(data->partner_battle[i]);
				// if (partner && partner->scene)
				// {
				//	partner->scene->delete_partner_from_scene(partner, true);
				// }
			}
			break;
		}

		//主战优先出战的时候，确保主战伙伴出战
		do
		{
			uint64_t main_partner_uuid = data->partner_formation[0];
			if (!is_partner_precedence() || main_partner_uuid == 0)
			{
				break;
			}

			partner_struct *main_partner = get_partner_by_uuid(main_partner_uuid);
			if (!main_partner || !main_partner->is_alive())
			{
				break;
			}
			if (partner_is_in_battle(main_partner_uuid))
			{
				break;
			}

			if (data->partner_battle[0] > 0)
			{
				del_partner_from_scene(data->partner_battle[0], true);
				// partner_struct *down_partner = get_partner_by_uuid(data->partner_battle[0]);
				// if (down_partner && down_partner->scene)
				// {
				//	down_partner->scene->delete_partner_from_scene(down_partner, true);
				// }
			}

			data->partner_battle[0] = main_partner_uuid;
			add_partner_to_scene(data->partner_battle[0]);
			add_task_progress(TCT_PARTNER_OUT_FIGHT, main_partner->data->partner_id, 1);
			reset_partner_anger();
		} while(0);

		for (int i = 0; i < MAX_PARTNER_BATTLE_NUM; ++i)
		{
			if (data->partner_battle[i] == 0 && i != 0)
			{
				continue;
			}

			partner_struct *partner = get_partner_by_uuid(data->partner_battle[i]);
			if (!partner_is_in_formation(data->partner_battle[i]) || !partner || !partner->is_alive())
			{
				if (partner)
					del_partner_from_scene(partner, true);
				// if (partner && partner->scene)
				// {
				//	partner->scene->delete_partner_from_scene(partner, true);
				// }

				data->partner_battle[i] = 0;
				data->partner_battle[i] = get_next_can_battle_partner();
				if (data->partner_battle[i] > 0)
				{
					partner_struct *partner_on = get_partner_by_uuid(data->partner_battle[i]);
					if (partner_on && partner_on->is_alive())
					{
						add_partner_to_scene(data->partner_battle[i]);
						add_task_progress(TCT_PARTNER_OUT_FIGHT, partner_on->data->partner_id, 1);
					}
				}
				if (i == 0)
				{
					reset_partner_anger();
				}
			}
			else if (partner_is_in_formation(data->partner_battle[i]) && partner && partner->is_alive() && !partner->scene)
			{
				add_partner_to_scene(data->partner_battle[i]);
				add_task_progress(TCT_PARTNER_OUT_FIGHT, partner->data->partner_id, 1);
			}
		}
	} while(0);

	uint64_t new_uuid = get_fighting_partner();
	if (new_uuid != old_uuid)
	{
		notify_fighting_partner();
	}
}

uint64_t player_struct::get_next_can_battle_partner(void)
{
	uint64_t fast_relive_uuid = 0, relive_time = UINT64_MAX;
	for (int i = 0; i < MAX_PARTNER_FORMATION_NUM; ++i)
	{
		uint64_t partner_uuid = data->partner_formation[i];
		if (partner_uuid == 0)
		{
			continue;
		}

		if (partner_is_in_battle(partner_uuid))
		{
			continue;
		}

		partner_struct *partner = get_partner_by_uuid(partner_uuid);
		if (!partner)
		{
			continue;
		}

		if (!partner->is_alive())
		{
			if (partner->data->relive_time < relive_time)
			{
				relive_time = partner->data->relive_time;
				fast_relive_uuid = partner->get_uuid();
			}
			continue;
		}

		return partner_uuid;
	}

	return fast_relive_uuid;
}

uint64_t player_struct::get_fighting_partner(void)
{
	uint64_t uuid = data->partner_battle[0];
//	if (uuid > 0)
//	{
//		partner_struct *partner = get_partner_by_uuid(uuid);
//		if (!partner || !partner->scene)
//		{
//			uuid = 0;
//		}
//	}

	return uuid;
}

void player_struct::on_leave_scene(scene_struct *old_scene)
{
	for (int i = 0; i < MAX_PARTNER_BATTLE_NUM; ++i)
	{
		if (data->partner_battle[i] == 0)
		{
			continue;
		}

		del_partner_from_scene(data->partner_battle[i], true);
		// partner_struct *partner = get_partner_by_uuid(data->partner_battle[i]);
		// if (partner && partner->scene)
		// {
		//	partner->scene->delete_partner_from_scene(partner, true);
		// }
	}
}

int player_struct::add_partner_anger(uint32_t num, bool isNty)
{
	if (num == 0)
	{
		return 0;
	}

	uint32_t prevVal = data->attrData[PLAYER_ATTR_PARTNER_ANGER];
	if (prevVal >= sg_partner_anger_max)
	{
		return 0;
	}

	data->attrData[PLAYER_ATTR_PARTNER_ANGER] = std::min(prevVal + num, sg_partner_anger_max);
	uint32_t curVal = data->attrData[PLAYER_ATTR_PARTNER_ANGER];
	uint32_t realNum = curVal - prevVal;
	LOG_INFO("[%s:%d] player[%lu] prevVal:%u, curVal:%u, num:%u, realNum:%u", __FUNCTION__, __LINE__, data->player_id, prevVal, curVal, num, realNum);

	if (isNty && curVal != prevVal)
	{
		this->notify_one_attr_changed(PLAYER_ATTR_PARTNER_ANGER, data->attrData[PLAYER_ATTR_PARTNER_ANGER]);
	}

	return 0;
}

int player_struct::reset_partner_anger(bool isNty)
{
	uint32_t prevVal = data->attrData[PLAYER_ATTR_PARTNER_ANGER];
	data->attrData[PLAYER_ATTR_PARTNER_ANGER] = 0;
	LOG_INFO("[%s:%d] player[%lu] prevVal:%u", __FUNCTION__, __LINE__, data->player_id, prevVal);

	if (isNty && 0 != prevVal)
	{
		this->notify_one_attr_changed(PLAYER_ATTR_PARTNER_ANGER, data->attrData[PLAYER_ATTR_PARTNER_ANGER]);
	}

	return 0;
}

int player_struct::add_partner_exp(uint32_t num, uint32_t statis_id, bool isNty)
{
	uint32_t player_level = get_level();
	for (int i = 0; i < MAX_PARTNER_BATTLE_NUM; ++i)
	{
		if (data->partner_battle[i] == 0)
		{
			continue;
		}

		partner_struct *partner = get_partner_by_uuid(data->partner_battle[i]);
		if (partner && partner->is_alive())
		{
			partner->add_exp(num, statis_id, player_level, isNty);
		}
	}
	return 0;
}

void player_struct::notify_fighting_partner(void)
{
	PartnerUuid nty;
	partner_uuid__init(&nty);

	nty.uuids = get_fighting_partner();

	EXTERN_DATA ext_data;
	ext_data.player_id = get_uuid();

	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_PARTNER_FIGHTING_NOTIFY, partner_uuid__pack, nty);
}

void player_struct::check_partner_relive(void)
{
	if (!data->login_notify)
	{
		return;
	}
	uint32_t now = time_helper::get_cached_time() / 1000;
	for (PartnerMap::iterator iter = m_partners.begin(); iter != m_partners.end(); ++iter)
	{
		partner_struct *partner = iter->second;
		if (partner->data->relive_time > 0 && partner->data->relive_time <= now)
		{
			partner->on_relive();
		}
	}
}

bool player_struct::partner_dictionary_is_active(uint32_t partner_id)
{
	for (int i = 0; i < MAX_PARTNER_TYPE; ++i)
	{
		if (data->partner_dictionary[i] == 0)
		{
			break;
		}

		if (data->partner_dictionary[i] == partner_id)
		{
			return true;
		}
	}

	return false;
}

bool player_struct::partner_bond_is_active(uint32_t bond_id)
{
	for (int i = 0; i < MAX_PARTNER_BOND_NUM; ++i)
	{
		if (data->partner_bond[i] == 0)
		{
			break;
		}

		if (data->partner_bond[i] == bond_id)
		{
			return true;
		}
	}

	return false;
}

bool player_struct::partner_bond_reward_is_get(uint32_t partner_id)
{
	for (int i = 0; i < MAX_PARTNER_TYPE; ++i)
	{
		if (data->partner_bond_reward[i] == 0)
		{
			break;
		}

		if (data->partner_bond_reward[i] == partner_id)
		{
			return true;
		}
	}

	return false;
}



#if 0
struct position *player_struct::get_player_pos()
{
	assert(data);
	assert(data->move_path.cur_pos <= data->move_path.max_pos);
	assert(data->move_path.cur_pos < MAX_PATH_POSITION);
	return &data->move_path.pos[data->move_path.cur_pos];
}
#endif

void player_struct::use_hp_pool_add_hp()
{
	if (data->hp_pool_num == 0)
		return;
	if (!is_alive())
		return;

	uint32_t cur_hp = get_attr(PLAYER_ATTR_HP);
	uint32_t max_hp = get_attr(PLAYER_ATTR_MAXHP);

	if (max_hp <= cur_hp)
		return;

	uint32_t add_hp = max_hp * sg_hp_pool_percent;
	if (add_hp > data->hp_pool_num)
		add_hp = data->hp_pool_num;
	if (add_hp + cur_hp > max_hp)
		add_hp = max_hp - cur_hp;

	cur_hp += add_hp;
	set_attr(PLAYER_ATTR_HP, cur_hp);
	data->hp_pool_num -= add_hp;
	send_hp_pool_changed_notify();
	broadcast_one_attr_changed(PLAYER_ATTR_HP, cur_hp, true, true);
	on_hp_changed(add_hp);
}

void player_struct::do_auto_add_hp()
{
	if (time_helper::get_cached_time() < data->next_auto_add_hp_time)
		return;
	if (data->on_fight_state)
	{
		leave_fight_state();
	}
	else
	{
		use_hp_pool_add_hp();
		data->next_auto_add_hp_time = time_helper::get_cached_time() + 1000;
	}
}

void player_struct::on_tick_10()
{
	//todo 代码优化
	check_fashion_expire();
	check_horse_expire();
	check_guoyu_expire();
	check_escort();
	check_partner_relive();

	if (data->zhenying.last_week < time_helper::get_cached_time() / 1000)
	{
		data->zhenying.last_week = time_helper::nextWeek(5 * 3600);
		data->zhenying.kill_week = 0;
		data->zhenying.score_week = 0;
		data->zhenying.week = 0;
		data->zhenying.task_type = 1;
		WeekTable *table = get_rand_week_config();
		if (table != NULL)
		{
			data->zhenying.task = table->ID;
			data->zhenying.task_type = table->Type;
		}
		data->zhenying.task_num = 0;

		send_zhenying_info();
	}

	do_auto_add_hp();

	if (!pos_changed)
		return;
	pos_changed = false;

	SpecialPlayerPosNotify nty;
	EXTERN_DATA extern_data;
	special_player_pos_notify__init(&nty);

	for (std::list<uint64_t>::iterator ite = watched_player_id.begin(); ite != watched_player_id.end(); ++ite)
	{
		player_struct *player = player_manager::get_player_by_id(*ite);
		if (!player)
			continue;
		struct position *pos = get_pos();
		nty.scene_id = data->scene_id;
		nty.uuid = data->player_id;
		nty.pos_x = pos->pos_x;
		nty.pos_z = pos->pos_z;
		extern_data.player_id = player->get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data,
			MSG_ID_SPECIAL_PLAYER_POS_NOTIFY, special_player_pos_notify__pack, nty);
	}
}

void player_struct::on_tick()
{
	update_player_pos_and_sight();

	if (buff_state & BUFF_STATE_TAUNT)
	{
		do_taunt_action();
	}

	if (get_entity_type(data->player_id) != ENTITY_TYPE_PLAYER)
		return;
	if (time_helper::get_cached_time() > data->next_time_refresh_oneday_job)
	{
		refresh_oneday_job();
	}
	check_task_time();

	if (data->truck.truck_id != 0)
	{
		cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(data->truck.truck_id);
		if (truck != NULL)
		{
			truck->on_tick();
		}
	}
}

void player_struct::on_kill_player(player_struct *dead)
{
	if (!is_online())
		return;


		//玩家开启杀戮模式，击杀任意模式下的玩家会增加1点的杀戮值，非杀戮模式击杀都不会增加杀戮值；
	if (get_attr(PLAYER_ATTR_PK_TYPE) == PK_TYPE_MURDER)
	{
		add_murder_num(sg_muder_add_num);

		// int murder = get_attr(PLAYER_ATTR_MURDER);
		// if (murder + sg_muder_add_num <= sg_muder_num_max)
		// {
		//	uint32_t id[1];
		//	double value[1];
		//	id[0] = PLAYER_ATTR_MURDER;
		//	value[0] = murder + sg_muder_add_num;
		//	set_attr(PLAYER_ATTR_MURDER, value[0]);
		//	notify_attr_changed(1, id, value);
		// }

		//通知好友服增加仇人
		PROTO_HEAD *head = conn_node_base::get_send_buf(SERVER_PROTO_FRIEND_ADD_ENEMY, 0);
		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + sizeof(uint64_t));
		uint64_t *pData = (uint64_t*)head->data;
		*pData++ = data->player_id;

		EXTERN_DATA ext_data;
		ext_data.player_id = dead->data->player_id;
		conn_node_base::add_extern_data(head, &ext_data);
		if (conn_node_gamesrv::connecter.send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len))
		{
			LOG_ERR("[%s:%d] player[%lu] send to friend_srv failed, err:%u", __FUNCTION__, __LINE__, ext_data.player_id, errno);
		}
	}

		//当玩家杀戮值超过10点时，死亡后会掉落经验，杀戮值越高，掉落经验越多，具体点数以及掉落经验读取配置表；
	int muder_num = dead->get_attr(PLAYER_ATTR_MURDER);
	if (muder_num > sg_muder_punish_point)
	{
		int sub_exp = dead->get_attr(PLAYER_ATTR_EXP);
		sub_exp = sub_exp * ((muder_num - sg_muder_punish_point) / sg_muder_punish_inc[0] * sg_muder_punish_inc[1] + sg_muder_punish_base) / 10000.0;
		dead->sub_exp(sub_exp, MAGIC_TYPE_MUDER_PUNISH, true);
	}

		//有杀戮值的玩家只有开启杀戮模式并且被任意模式玩家击杀后，会降低1点的杀戮值；
	if (dead->get_attr(PLAYER_ATTR_PK_TYPE) == PK_TYPE_MURDER)
	{
		dead->sub_murder_num(sg_muder_sub_num);
		// int murder = dead->get_attr(PLAYER_ATTR_MURDER);
		// if (murder - sg_muder_sub_num >= 0)
		// {
		//	uint32_t id[1];
		//	double value[1];
		//	id[0] = PLAYER_ATTR_MURDER;
		//	value[0] = murder - sg_muder_sub_num;
		//	dead->set_attr(PLAYER_ATTR_MURDER, value[0]);
		//	dead->notify_attr_changed(1, id, value);
		// }
	}


	// if (get_attr(PLAYER_ATTR_PK_TYPE) == PK_TYPE_MURDER && murder < sg_muder_num_max)
	// {
	//	if (murder < sg_muder_num_max)
	//	{
	//		uint32_t id[1];
	//		double value[1];
	//		id[0] = PLAYER_ATTR_MURDER;
	//		value[0] = murder + sg_muder_add_num;
	//		set_attr(PLAYER_ATTR_MURDER, value[0]);
	//		notify_attr_changed(1, id, value);
	//	}
	//	murder = dead->get_attr(PLAYER_ATTR_MURDER);
	//	if (murder >= sg_muder_sub_num)
	//	{
	//		uint32_t id[1];
	//		double value[1];
	//		id[0] = PLAYER_ATTR_MURDER;
	//		value[0] = murder - sg_muder_sub_num;
	//		dead->set_attr(PLAYER_ATTR_MURDER, value[0]);
	//		dead->notify_attr_changed(1, id, value);
	//	}
	// }

	add_task_progress(TCT_KILL_PLAYER, data->player_id, 1);

	if (data->chengjie.cur_task != 0 && data->chengjie.target == dead->get_uuid())
	{
		STChengJie *pTask = ChengJieTaskManage::FindTask(data->chengjie.cur_task);
		if (pTask != NULL && pTask->complete == false && data->scene_id < SCENCE_DEPART)
		{
			pTask->complete = true;
			ChengJieTaskManage::CommpleteTask(this, dead, *pTask);
		}
	}
	if (dead->data->chengjie.cur_task != 0 && dead->data->chengjie.target == this->get_uuid() && data->scene_id < SCENCE_DEPART)
	{
		ChengJieTaskManage::TaskFail(dead, this);
	}
}

void player_struct::on_hp_changed(int damage)
{
}

void player_struct::on_dead(unit_struct *killer)
{
//	clear_all_buffs();
	if (!scene || !data)
		return;

	if (data->truck.on_truck)
	{
		go_down_cash_truck();
	}

	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (!m_buffs[i])
			continue;
		m_buffs[i]->on_dead();
	}

	if (killer && killer->get_unit_type() == UNIT_TYPE_PLAYER)
	{
		((player_struct *)killer)->on_kill_player(this);
	}
	if (scene)
		scene->on_player_dead(this, killer);

	m_team == NULL ? 0 : m_team->OnMemberHpChange(*this);

	if (is_in_qiecuo())
	{
		LOG_INFO("%s: player[%lu] dead, qiecuo failed", __FUNCTION__, get_uuid());

		player_struct *target = player_manager::get_player_by_id(data->qiecuo_target);
		assert(target);
		finish_qiecuo();
		target->finish_qiecuo();

		QiecuoFinishNotify nty;
		EXTERN_DATA ext;
		qiecuo_finish_notify__init(&nty);

		nty.result = 0;
		ext.player_id = target->get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_FINISH_NOTIFY, qiecuo_finish_notify__pack, nty);

		nty.result = 1;
		ext.player_id = get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_QIECUO_FINISH_NOTIFY, qiecuo_finish_notify__pack, nty);
	}

	if (sight_space) //&& sight_space->data->type != 2)
		sight_space_manager::mark_sight_space_delete(sight_space);

	interrupt();
	adjust_battle_partner();
}

void player_struct::on_relive_in_raid(raid_struct *raid, uint32_t type)
{
	if (raid->ai && raid->ai->raid_on_player_relive)
		return raid->ai->raid_on_player_relive(raid, this, type);

	ReliveNotify nty;
	relive_notify__init(&nty);
	nty.playerid = get_uuid();
	nty.type = type;

	if (type == 1)	//原地复活
	{
		if (raid->m_config->InstantRelive == 0)
		{
			LOG_ERR("%s player[%lu] can not relive type 1 in raid %u", __FUNCTION__, get_uuid(), raid->m_id);
			return;
		}

		int relive_times = get_attr(PLAYER_ATTR_RELIVE_TYPE1);
		if (relive_times >= sg_relive_free_times)
		{
			int fin_cost = (relive_times - sg_relive_free_times) * sg_relive_grow_cost + sg_relive_first_cost;
			if (fin_cost > sg_relive_max_cost)
				fin_cost = sg_relive_max_cost;
			if (get_comm_gold() < fin_cost)
			{
				LOG_ERR("%s: player[%lu] do not have enough gold", __FUNCTION__, get_uuid());
				return;
			}
			sub_comm_gold(fin_cost, MAGIC_TYPE_RELIVE);
		}

		++data->attrData[PLAYER_ATTR_RELIVE_TYPE1];
		++data->attrData[PLAYER_ATTR_RELIVE_TYPE2];
		broadcast_to_sight(MSG_ID_RELIVE_NOTIFY, &nty, (pack_func)relive_notify__pack, true);
	}
	else //复活点复活
	{
		struct position *pos = get_pos();
		scene->get_relive_pos(pos->pos_x, pos->pos_z, &nty.pos_x, &nty.pos_z, &nty.direct);
		LOG_DEBUG("%s: player[%lu] relive to pos[%d][%d][%d]", __FUNCTION__, get_uuid(), nty.pos_x, nty.pos_z, nty.direct);

		EXTERN_DATA extern_data;
		extern_data.player_id = get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_RELIVE_NOTIFY, relive_notify__pack, nty);

		send_clear_sight();
		scene_struct *t_scene = scene;
		scene->delete_player_from_scene(this);
		set_pos(nty.pos_x, nty.pos_z);
		t_scene->add_player_to_scene(this);
	}

	data->attrData[PLAYER_ATTR_HP] = data->attrData[PLAYER_ATTR_MAXHP];
	on_hp_changed(0);

		//复活的时候加上一个无敌buff
	buff_manager::create_default_buff(114400001, this, this, false);

	m_team == NULL ? true : m_team->OnMemberHpChange(*this);
}

void player_struct::on_relive(uint32_t type)
{
	if (!data)
		return;
	if (!scene)
	{
		LOG_ERR("%s: player %lu can not relive without scene", __FUNCTION__, get_uuid());
		return;
	}

	if (scene->get_scene_type() == SCENE_TYPE_RAID)
	{
		raid_struct *raid = (raid_struct *)scene;
		on_relive_in_raid(raid, type);
	}
	else
	{
		ReliveNotify nty;
		relive_notify__init(&nty);
		nty.playerid = get_uuid();
		nty.type = type;

		if (type == 1)	//原地复活
		{
			int relive_times = get_attr(PLAYER_ATTR_RELIVE_TYPE1);
	//		int free_times = get_config_by_id(PARAM_ID_RELIVE_FREE_TIMES, &parameter_config)->parameter1[0];
			if (relive_times >= sg_relive_free_times)
			{
	//			int first_cost = get_config_by_id(PARAM_ID_RELIVE_FIRST_COST, &parameter_config)->parameter1[0];
	//			int grow_cost = get_config_by_id(PARAM_ID_RELIVE_GROW_COST, &parameter_config)->parameter1[0];
	//			int max_cost = get_config_by_id(PARAM_ID_RELIVE_MAX_COST, &parameter_config)->parameter1[0];
				int fin_cost = (relive_times - sg_relive_free_times) * sg_relive_grow_cost + sg_relive_first_cost;
				if (fin_cost > sg_relive_max_cost)
					fin_cost = sg_relive_max_cost;
				if (get_comm_gold() < fin_cost)
				{
					LOG_ERR("%s: player[%lu] do not have enough gold", __FUNCTION__, get_uuid());
					return;
				}
				sub_comm_gold(fin_cost, MAGIC_TYPE_RELIVE);
			}

			++data->attrData[PLAYER_ATTR_RELIVE_TYPE1];
			broadcast_to_sight(MSG_ID_RELIVE_NOTIFY, &nty, (pack_func)relive_notify__pack, true);
		}
		else //复活点复活
		{
			struct position *pos = get_pos();
			scene->get_relive_pos(pos->pos_x, pos->pos_z, &nty.pos_x, &nty.pos_z, &nty.direct);
			LOG_DEBUG("%s: player[%lu] relive to pos[%d][%d][%d]", __FUNCTION__, get_uuid(), nty.pos_x, nty.pos_z, nty.direct);

			EXTERN_DATA extern_data;
			extern_data.player_id = get_uuid();
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_RELIVE_NOTIFY, relive_notify__pack, nty);

			send_clear_sight();
			scene_struct *t_scene = scene;
			scene->delete_player_from_scene(this);
			set_pos(nty.pos_x, nty.pos_z);
			t_scene->add_player_to_scene(this);
		}

		data->attrData[PLAYER_ATTR_HP] = data->attrData[PLAYER_ATTR_MAXHP];
		++data->attrData[PLAYER_ATTR_RELIVE_TYPE2];
		on_hp_changed(0);

		//复活的时候加上一个无敌buff
		buff_manager::create_default_buff(114400001, this, this, false);
		if (chengjie_kill != 0)
		{
			ParameterTable *config = get_config_by_id(161000098, &parameter_config);
			if (config != NULL)
			{
				buff_manager::create_default_buff(config->parameter1[chengjie_kill - 1], this, this, true);
			}
			chengjie_kill = 0;
		}


		m_team == NULL ? true : m_team->OnMemberHpChange(*this);
	}

	adjust_battle_partner();
}

void player_struct::on_repel(unit_struct *player)
{
	if (!scene || !data)
		return;
}

void player_struct::refresh_oneday_job()
{
	if (get_entity_type(data->player_id) != ENTITY_TYPE_PLAYER)
		return;

	data->next_time_refresh_oneday_job = (time_helper::get_cached_time() + 24 * 3600 * 1000) / (24 * 3600 * 1000) * (24 * 3600 * 1000) + timezone * 1000;
//	LOG_DEBUG("%s: timezone = %ld, nextday = %lu", __FUNCTION__, timezone, data->next_time_refresh_oneday_job);

	data->attrData[PLAYER_ATTR_RELIVE_TYPE1] = 0;
	data->attrData[PLAYER_ATTR_RELIVE_TYPE2] = 0;
	memset(data->raid_reward_id, 0, sizeof(data->raid_reward_id));
	send_raid_earning_time_notify();
//	memset(data->raid_reward_num, 0, sizeof());
	data->attrData[PLAYER_ATTR_ACTIVENESS] = 0;
	data->attrData[PLAYER_ATTR_BRAVE] = sg_guild_battle_brave_init;
	AttrMap nty_list;
	nty_list[PLAYER_ATTR_RELIVE_TYPE1] = data->attrData[PLAYER_ATTR_RELIVE_TYPE1];
	nty_list[PLAYER_ATTR_RELIVE_TYPE2] = data->attrData[PLAYER_ATTR_RELIVE_TYPE2];
	nty_list[PLAYER_ATTR_ACTIVENESS] = data->attrData[PLAYER_ATTR_ACTIVENESS];
	notify_attr(nty_list);
	refresh_shop_daily();

	data->pvp_raid_data.oneday_win_num_3 = 0;
	data->pvp_raid_data.oneday_win_num_5 = 0;
	for (int i = 0; i < MAX_ONEDAY_PVP_BOX; ++i)
	{
		data->pvp_raid_data.avaliable_box_3[i] = i;
		data->pvp_raid_data.avaliable_box_5[i] = i;
	}

	refresh_yaoshi_oneday();
	refresh_activity_daily();
	data->zhenying.exp_day = 0;
	FactionBattleTable *table = get_zhenying_battle_table(get_attr(PLAYER_ATTR_LEVEL));
	if (table != NULL)
	{
		data->zhenying.mine = table->BoxOpenNum;
	}

	add_zhenying_exp(0);
	data->live_skill.book[LIVE__SKILL__TYPE__COOK] = 20; //todo 读配置
	data->live_skill.book[LIVE__SKILL__TYPE__MEDICINE] = 20;
}

void player_struct::refresh_shop_daily(void)
{
	uint32_t now = time_helper::get_cached_time() / 1000;
	uint32_t hour = time_helper::get_cur_hour(data->next_time_refresh_oneday_job);
	bool day_reset = false, week_reset = false, month_reset = false;
	if (now >= data->shop_reset.next_day_time) //每天
	{
		data->shop_reset.next_day_time = time_helper::nextOffsetTime(hour * 3600, now);
		day_reset = true;
	}
	if (now >= data->shop_reset.next_week_time) //每周一
	{
		data->shop_reset.next_week_time = time_helper::get_next_timestamp_by_week_old(1, hour, 0, now);
		week_reset = true;
	}
	if (now >= data->shop_reset.next_month_time) //每月一号
	{
		data->shop_reset.next_month_time = time_helper::get_next_timestamp_by_month_old(1, hour, 0, now);
		month_reset = true;
	}
	for (int i = 0; i < MAX_SHOP_GOODS_NUM; ++i)
	{
		GoodsInfo *info = &data->shop_goods[i];
		if (info->goods_id == 0)
		{
			continue;
		}

		ShopTable *config = get_config_by_id(info->goods_id, &shop_config);
		if (!config)
		{
			continue;
		}

		if (config->Reset != 1)
		{
			continue;
		}

		if ((config->RestrictionTime == 1 && day_reset) || (config->RestrictionTime == 2 && week_reset) || (config->RestrictionTime == 3 && month_reset))
		{
			info->bought_num = 0;
		}
	}

	ShopGoodsNotify nty;
	shop_goods_notify__init(&nty);

	GoodsData goods_data[MAX_SHOP_GOODS_NUM];
	GoodsData* goods_data_point[MAX_SHOP_GOODS_NUM];

	nty.n_goods = 0;
	nty.goods = goods_data_point;
	for (int i = 0; i < MAX_SHOP_GOODS_NUM; ++i)
	{
		if (data->shop_goods[i].goods_id == 0)
		{
			break;
		}

		goods_data_point[nty.n_goods] = &goods_data[nty.n_goods];
		goods_data__init(&goods_data[nty.n_goods]);
		goods_data[nty.n_goods].goodsid = data->shop_goods[i].goods_id;
		goods_data[nty.n_goods].boughtnum = data->shop_goods[i].bought_num;
		nty.n_goods++;
	}

	EXTERN_DATA ext_data;
	ext_data.player_id = data->player_id;

	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_SHOP_GOODS_NOTIFY, shop_goods_notify__pack, nty);
}

void player_struct::interrupt()
{
	interrupt_sing();
	if (data->m_collect_uuid == 0)
	{
		return;
	}
	Collect *pCollect = Collect::GetById(data->m_collect_uuid);
	if (pCollect == NULL)
	{
		return;
	}
	pCollect->GatherInterupt(this);
}

void player_struct::on_attack(unit_struct *target)
{
	if (!scene || !data)
		return;
	for (std::set<monster_struct *>::iterator ite = m_pet_list.begin(); ite != m_pet_list.end(); ++ite)
	{
		monster_struct *pet = (*ite);
		if (pet->ai && pet->ai->on_owner_attack)
			pet->ai->on_owner_attack(pet, this, target);
	}

	for (int i = 0; i < MAX_PARTNER_BATTLE_NUM; ++i)
	{
		if (data->partner_battle[i] == 0)
			continue;
		partner_struct *partner = partner_manager::get_partner_by_uuid(data->partner_battle[i]);
		if (!partner)
			continue;
		partner->on_owner_attack(target->get_uuid());
	}
}

void player_struct::enter_fight_state()
{
	data->next_auto_add_hp_time = time_helper::get_cached_time() + 5000;
	if (data->on_fight_state)
		return;
	data->on_fight_state = true;

	EXTERN_DATA extern_data;
	extern_data.player_id = data->player_id;
	fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ENTER_FIGHT_STATE_NOTIFY, 0, 0);
}

void player_struct::leave_fight_state()
{
	assert(data->on_fight_state);

	data->on_fight_state = false;
	data->next_auto_add_hp_time = time_helper::get_cached_time() + 1000;

	EXTERN_DATA extern_data;
	extern_data.player_id = data->player_id;
	fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_LEAVE_FIGHT_STATE_NOTIFY, 0, 0);
}

void player_struct::on_beattack(unit_struct *player, uint32_t skill_id, int32_t damage)
{
	if (!scene || !data)
		return;
	interrupt();
	if (data->truck.on_truck)
	{
		go_down_cash_truck();
	}
	//if (m_team != NULL)
	//{
	//	m_team->OnMemberHpChange(*this);
	//}
	m_team == NULL ? 0 : m_team->OnMemberHpChange(*this);
	for (std::set<monster_struct *>::iterator ite = m_pet_list.begin(); ite != m_pet_list.end(); ++ite)
	{
		monster_struct *pet = (*ite);
		if (pet->ai && pet->ai->on_owner_beattack)
			pet->ai->on_owner_beattack(pet, this, player);
	}

	for (int i = 0; i < MAX_PARTNER_BATTLE_NUM; ++i)
	{
		if (data->partner_battle[i] == 0)
			continue;
		partner_struct *partner = partner_manager::get_partner_by_uuid(data->partner_battle[i]);
		if (!partner)
			continue;
		partner->on_owner_beattack(player->get_uuid());
	}

	enter_fight_state();
}

void player_struct::broadcast_to_sight_and_team(uint16_t msg_id, void *msg_data, pack_func func, bool include_myself)
{
	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(msg_id, msg_data, func);
	PROTO_HEAD_CONN_BROADCAST *head;
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
	int player_num = *get_cur_sight_player();
	uint64_t *t_player_id = get_all_sight_player();
	for (int i = 0; i < player_num; ++i)
	{
		conn_node_gamesrv::broadcast_msg_add_players(t_player_id[i], ppp);
	}

	if (include_myself && get_entity_type(get_uuid()) == ENTITY_TYPE_PLAYER)
	{
		conn_node_gamesrv::broadcast_msg_add_players(data->player_id, ppp);
	}

	if (m_team != NULL)
	{
		for (int i = 0; i < m_team->m_data->m_memSize; ++i)
		{
			if (m_team->m_data->m_mem[i].id != data->player_id
				&& !is_player_in_sight(m_team->m_data->m_mem[i].id))
			{
					//检查在同一个场景
				player_struct *player = player_manager::get_player_by_id(m_team->m_data->m_mem[i].id);
				if (player && player->scene == scene)
					conn_node_gamesrv::broadcast_msg_add_players(m_team->m_data->m_mem[i].id, ppp);
			}
		}
	}

	if (head->num_player_id > 0)
	{
		conn_node_gamesrv::broadcast_msg_send();
	}
}

int player_struct::transfer_to_new_scene_impl(scene_struct *new_scene, double pos_x, double pos_y, double pos_z, double direct, EXTERN_DATA *extern_data)
{
	if (sight_space)
	{
		LOG_ERR("%s: player[%lu] in sightspace, can not transfer", __FUNCTION__, data->player_id);
		return (-1);
	}
	if (!scene)
	{
		LOG_ERR("%s: player[%lu] not in scene, can not transfer", __FUNCTION__, data->player_id);
		return (-10);
	}

	uint32_t old_scence = data->scene_id;
	if (new_scene->m_id == data->scene_id)
	{
		assert(scene == new_scene);
//		set_pos_with_broadcast(pos_x, pos_z);
		send_clear_sight();
		scene->delete_player_from_scene(this);
		set_pos(pos_x, pos_z);
		new_scene->add_player_to_scene(this);
	}
	else
	{
		if (scene->get_scene_type() == SCENE_TYPE_WILD)
		{
			data->last_scene_id = data->scene_id;
		}
		scene->delete_player_from_scene(this);
		data->scene_id = new_scene->m_id;
		set_pos(pos_x, pos_z);
	}
	data->pos_y = pos_y;

	data->m_angle = unity_angle_to_c_angle(direct);
	send_scene_transfer(direct, pos_x, pos_y, pos_z, new_scene->m_id, 0);

	interrupt();

	if (old_scence > SCENCE_DEPART && new_scene->m_id < SCENCE_DEPART)
	{
		return 0;
	}
	if (m_team !=  NULL)
	{
		if (m_team->GetLeadId() == this->get_uuid())
		{
			m_team->FollowLeadTrans(new_scene->m_id, pos_x, pos_y, pos_z, direct);
		}
		//if (new_scene->m_id < SCENCE_DEPART)
		//{
			m_team->broadcast_leader_pos(this->get_pos(), this->data->scene_id, this->get_uuid());
		//}
	}
	return (0);
}

int player_struct::transfer_to_new_scene_by_config(uint32_t transfer_id, EXTERN_DATA *extern_data)
{
	LOG_INFO("%s: player[%lu] transfer to id[%u]", __FUNCTION__, extern_data->player_id, transfer_id);
	struct TransferPointTable* t_config = get_config_by_id(transfer_id, &transfer_config);
	if (!t_config || t_config->n_MapId == 0)
	{
		LOG_ERR("%s: player[%lu] transfer to %u, no config", __FUNCTION__, get_uuid(), transfer_id);
		return (-1);
	}
	scene_struct *new_scene = scene_manager::get_scene(t_config->MapId[0]);
	if (!new_scene)
	{
		LOG_ERR("%s %d: player[%lu] transfer to the wrong scene[%lu]", __FUNCTION__, __LINE__, data->player_id, t_config->MapId[0]);
		return (-30);
	}

	if (t_config->n_MapId >= 5)
	{
		double pos_x = (int64_t)t_config->MapId[1] / 1000.0;
		double pos_y = (int64_t)t_config->MapId[2] / 1000.0;
		double pos_z = (int64_t)t_config->MapId[3] / 1000.0;
		double direct = (int64_t)t_config->MapId[4];
		return transfer_to_new_scene_impl(new_scene, pos_x, pos_y, pos_z, direct, extern_data);
	}
	else
	{
		return transfer_to_new_scene_impl(new_scene, new_scene->m_born_x,
			new_scene->m_born_y, new_scene->m_born_z, new_scene->m_born_direct, extern_data);
	}

}

// int player_struct::transfer_to_new_scene(uint32_t scene_id, EXTERN_DATA *extern_data)
// {
//	scene_struct *new_scene = scene_manager::get_scene(scene_id);
//		if (!new_scene)
//		{
//		LOG_ERR("%s %d: player[%lu] transfer to the wrong scene[%u]", __FUNCTION__, __LINE__, data->player_id, scene_id);
//			return (-30);
//		}
//	return transfer_to_new_scene_impl(new_scene, new_scene->m_born_x,
//		new_scene->m_born_y, new_scene->m_born_z, new_scene->m_born_direct, extern_data);
// }

int player_struct::transfer_to_new_scene(uint32_t scene_id, double pos_x, double pos_y, double pos_z, double direct, EXTERN_DATA *extern_data)
{
	scene_struct *new_scene = scene_manager::get_scene(scene_id);
	if (!new_scene)
	{
		LOG_ERR("%s %d: player[%lu] transfer to the wrong scene[%u]", __FUNCTION__, __LINE__, data->player_id, scene_id);
		return (-30);
	}
	return transfer_to_new_scene_impl(new_scene, pos_x, pos_y, pos_z, direct, extern_data);
}

int player_struct::transfer_to_birth_position(EXTERN_DATA *extern_data)
{
	if (!scene)
	{
		LOG_ERR("[%s:%d] player[%lu] isn't in a scene, scene_id:%u", __FUNCTION__, __LINE__, data->player_id, data->scene_id);
		return -1;
	}

	return transfer_to_new_scene_impl(scene, scene->m_born_x, scene->m_born_y, scene->m_born_z, scene->m_born_direct, extern_data);
}

int player_struct::transfer_to_guild_scene(EXTERN_DATA *extern_data)
{
	if (data->guild_id == 0)
	{
		LOG_ERR("%s: %lu not join guild", __FUNCTION__, data->player_id);
		return ERROR_ID_GUILD_PLAYER_NOT_JOIN;
	}

	scene_struct *scene = scene_manager::get_guild_scene(data->guild_id);
	if (!scene)
	{
		LOG_ERR("[%s:%d] player[%lu] get guild_scene failed, guild_id:%u", __FUNCTION__, __LINE__, data->player_id, data->guild_id);
		return (-30);
	}
	return transfer_to_new_scene_impl(scene, scene->m_born_x, scene->m_born_y, scene->m_born_z, scene->m_born_direct, extern_data);
}

int player_struct::transfer_out_guild_scene(EXTERN_DATA *extern_data)
{
	if (!is_guild_scene_id(data->scene_id))
	{
		return 0;
	}

	scene_struct *scene = scene_manager::get_scene(data->last_scene_id);
	if (!scene)
	{
		LOG_ERR("[%s:%d] player[%lu] get last scene failed, last_scene_id:%u", __FUNCTION__, __LINE__, data->player_id, data->last_scene_id);
		return -1;
	}

	return transfer_to_new_scene_impl(scene, scene->m_born_x, scene->m_born_y, scene->m_born_z, scene->m_born_direct, extern_data);
}


uint32_t player_struct::get_raid_reward_count(uint32_t raid_id)
{
	for (int i = 0; i < MAX_RAID_NUM; ++i)
	{
		if (data->raid_reward_id[i] == 0)
		{
			return 0;
		}
		if (data->raid_reward_id[i] == raid_id)
		{
			return data->raid_reward_num[i];
		}
	}
	return (0);
}

void player_struct::add_raid_reward_count(uint32_t raid_id)
{
	int i;

	if (is_wanyaogu_raid(raid_id))
	{
		for (i = 0; i < MAX_RAID_NUM; ++i)
		{
			if (data->raid_reward_id[i] == 0)
			{
				for (std::vector<uint32_t>::iterator ite = sg_vec_wanyaogu_raid_id.begin();
					 ite != sg_vec_wanyaogu_raid_id.end(); ++ite)
				{
					data->raid_reward_id[i] = (*ite);
					data->raid_reward_num[i] = 1;
					++i;
				}
				return;
			}
			else if (is_wanyaogu_raid(data->raid_reward_id[i]))
			{
				for (std::vector<uint32_t>::iterator ite = sg_vec_wanyaogu_raid_id.begin();
					 ite != sg_vec_wanyaogu_raid_id.end(); ++ite)
				{
					assert(is_wanyaogu_raid(data->raid_reward_id[i]));
					data->raid_reward_num[i]++;
					++i;
				}
				return;
			}
		}
		assert(0);
	}

	for (i = 0; i < MAX_RAID_NUM; ++i)
	{
		if (data->raid_reward_id[i] == 0)
		{
			data->raid_reward_id[i] = raid_id;
			data->raid_reward_num[i] = 1;
			return;
		}

		if (data->raid_reward_id[i] == raid_id)
		{
			data->raid_reward_num[i]++;
			return;
		}
	}
	assert(0);
}

void player_struct::send_hp_pool_changed_notify()
{
	if (get_entity_type(data->player_id) == ENTITY_TYPE_AI_PLAYER)
		return;
	HpPoolChangedNotify nty;
	hp_pool_changed_notify__init(&nty);
	nty.hp_pool_num = data->hp_pool_num;
	EXTERN_DATA extern_data;
	extern_data.player_id = data->player_id;
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_HP_POOL_CHANGED_NOTIFY, hp_pool_changed_notify__pack, nty);
}

void player_struct::send_buff_info()
{
	if (get_entity_type(data->player_id) == ENTITY_TYPE_AI_PLAYER)
		return;

	AddBuffNotify notify;
	add_buff_notify__init(&notify);
	notify.playerid = get_uuid();

	EXTERN_DATA extern_data;
	extern_data.player_id = data->player_id;

	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (m_buffs[i] && m_buffs[i]->data)
		{
			notify.start_time = m_buffs[i]->data->start_time / 1000;
//			notify.end_time = m_buffs[i]->data->end_time / 1000;
			notify.buff_id = m_buffs[i]->data->buff_id;
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ADD_BUFF_NOTIFY, add_buff_notify__pack, notify);
//			owner->broadcast_to_sight(MSG_ID_ADD_BUFF_NOTIFY, &notify, (pack_func)add_buff_notify__pack, true);
		}
	}
}

void player_struct::send_raid_earning_time_notify()
{
	if (get_entity_type(data->player_id) == ENTITY_TYPE_AI_PLAYER)
		return;

	RaidEarningTimesChangedNotify notify;
	raid_earning_times_changed_notify__init(&notify);
	int i;
	for (i = 0; i < MAX_RAID_NUM; ++i)
	{
		if (data->raid_reward_id[i] == 0)
			break;
	}
	notify.n_raid_id = notify.n_earning_times = i;
	notify.raid_id = &data->raid_reward_id[0];
	notify.earning_times = &data->raid_reward_num[0];

	EXTERN_DATA extern_data;
	extern_data.player_id = data->player_id;

	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_RAID_EARNING_TIMES_CHANGED_NOTIFY, raid_earning_times_changed_notify__pack, notify);
}

bool player_struct::is_in_same_guild(player_struct *player)
{
	if (data->guild_id == 0)
		return false;
	if (player->data->guild_id == 0)
		return false;
	if (data->guild_id != player->data->guild_id)
		return false;
	return true;
}

bool player_struct::is_on_horse(void)
{
	return ((uint32_t)data->attrData[PLAYER_ATTR_ON_HORSE_STATE] == 1);
}

bool player_struct::is_on_truck(void)
{
	return (data->truck.on_truck);
}

void player_struct::refresh_player_redis_info(bool offline)
{
	if (!data || get_entity_type(data->player_id) != ENTITY_TYPE_PLAYER)
		return;
/*
	PROTO_HEAD *req = (PROTO_HEAD *)conn_node_base::global_send_buf;
	req->msg_id = ENDION_FUNC_2(SERVER_PROTO_GAME_TO_FRIEND);

	PlayerRedisInfo info;
	player_redis_info__init(&info);
	info.head_icon = get_attr(PLAYER_ATTR_HEAD);
	info.job = get_attr(PLAYER_ATTR_JOB);
	info.lv = get_attr(PLAYER_ATTR_LEVEL);
	info.name = data->name;
	info.scene_id = data->scene_id;
	size_t size = player_redis_info__pack(&info, (uint8_t *)&req->data[0]);
	req->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);

	LOG_DEBUG("%s: data len %d", __FUNCTION__, size);


	conn_node_base::add_extern_data(req, &extern_data);
	if (conn_node_gamesrv::connecter.send_one_msg(req, 1) != (int)ENDION_FUNC_4(req->len)) {
		LOG_ERR("%s %d: send to dbsrv err[%d]", __FUNCTION__, __LINE__, errno);
	}
	*/

	PlayerRedisInfo info;
	player_redis_info__init(&info);
	info.head_icon = get_attr(PLAYER_ATTR_HEAD);
	info.job = get_attr(PLAYER_ATTR_JOB);
	info.lv = get_attr(PLAYER_ATTR_LEVEL);
	info.name = data->name;
	info.scene_id = data->scene_id;
	info.hp = get_attr(PLAYER_ATTR_HP);
	info.max_hp = get_attr(PLAYER_ATTR_MAXHP);
	info.clothes = data->attrData[PLAYER_ATTR_CLOTHES];
	info.clothes_color_up = data->attrData[PLAYER_ATTR_CLOTHES_COLOR_UP];
	info.clothes_color_down = data->attrData[PLAYER_ATTR_CLOTHES_COLOR_DOWN];
	info.hat = data->attrData[PLAYER_ATTR_HAT];
	info.hat_color = data->attrData[PLAYER_ATTR_HAT_COLOR];
	info.weapon = data->attrData[PLAYER_ATTR_WEAPON];
	info.weapon_color = data->attrData[PLAYER_ATTR_WEAPON_COLOR];
	info.fighting_capacity = data->attrData[PLAYER_ATTR_FIGHTING_CAPACITY];
	info.status = (offline ? time_helper::get_cached_time() / 1000 : 0); //保存下线时间，0表示在线
	info.zhenying = data->attrData[PLAYER_ATTR_ZHENYING];
//	info.guild_id = guild_id;
//	info.guild_name = guild_name;
	info.n_tags = 0;
	info.tags = data->personality_tags;
	for (int i = 0; i < MAX_PERSONALITY_TAG_NUM; ++i)
	{
		if (data->personality_tags[i] == 0)
		{
			break;
		}
		info.n_tags++;
	}
	info.textintro = data->personality_text_intro;
	info.coin = get_coin();
	info.gold = get_attr(PLAYER_ATTR_GOLD);
	info.bind_gold = get_attr(PLAYER_ATTR_BIND_GOLD);
	info.equip_fc = data->fc_data.equip;
	info.bagua_fc = data->fc_data.bagua;
	info.pvp_division_3 = data->pvp_raid_data.level_3;
	info.pvp_score_3 = data->pvp_raid_data.score_3;

	EXTERN_DATA extern_data;
	extern_data.player_id = data->player_id;

	//在数据开头增加一个类型
	uint8_t *pData = conn_node_base::get_send_data();
	size_t data_len = player_redis_info__pack(&info, pData + sizeof(uint32_t));
	if (data_len != (size_t)-1)
	{
		*((uint32_t*)pData) = 1;
		data_len += sizeof(uint32_t);
		fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, SERVER_PROTO_REFRESH_PLAYER_REDIS_INFO, data_len, 0);
	}
}

int player_struct::set_enter_raid_pos_and_scene(raid_struct *raid, double pos_x, double pos_z)
//int player_struct::enter_raid(raid_struct *raid)
{
	if (scene)
		scene->delete_player_from_scene(this);
	scene = raid;
	data->scene_id = scene->m_id;
//	set_pos(raid->res_config->BirthPointX, raid->res_config->BirthPointZ);
	set_pos(pos_x, pos_z);
//	data->raid_uuid = raid->data->uuid;
	interrupt();
	return (0);
}

int player_struct::set_out_raid_pos()
{
	data->scene_id = data->leaveraid.scene_id;
	set_pos(data->leaveraid.ExitPointX, data->leaveraid.ExitPointZ);
	return (0);
}

int player_struct::set_out_raid_pos_and_clear_scene()
{
//	raid_struct *raid = (raid_struct *)scene;
//	data->raid_uuid = 0;
	set_out_raid_pos();
	scene = NULL;
	return (0);
}
//进入副本前保存离开副本后所到的场景id以及位置
int player_struct::conserve_out_raid_pos_and_scene(raid_struct *raid)
{
	assert(raid);
	assert(raid->m_config);
	data->leaveraid.scene_id = raid->m_config->ExitScene;
	data->leaveraid.ExitPointX = raid->m_config->ExitPointX;
	data->leaveraid.ExitPointY = raid->m_config->BirthPointY;
	data->leaveraid.ExitPointZ = raid->m_config->BirthPointZ;
	data->leaveraid.direct = raid->m_config->FaceY;
	return (0);
}
bool notice_use_art(uint32_t statis_id)
{
	switch(statis_id)
	{
		case MAGIC_TYPE_GATHER:
		case MAGIC_TYPE_MONSTER_DEAD:
		case MAGIC_TYPE_TASK_REWARD:
		case MAGIC_TYPE_WANYAOGU_BBQ:
			return true;
	}

	return false;
}

int player_struct::get_fashion(uint32_t id)
{
	for (uint32_t i = 0; i < data->n_fashion; ++i)
	{
		if (data->fashion[i].id == id)
		{
			return i;
		}
	}
	return -1;
}

int player_struct::add_fashion(uint32_t id, uint32_t color, time_t expire)
{
	int ret = get_fashion(id);
	if (ret >= 0) //已买
	{
		if (data->fashion[ret].timeout > 0)
		{
			if (expire  >0)
			{
				if (data->fashion[ret].timeout <= (time_t)time_helper::get_cached_time() / 1000)
				{
					data->fashion[ret].timeout = time_helper::get_cached_time() / 1000 + expire;
				}
				else
				{
					data->fashion[ret].timeout += expire;
				}
			}
			else
			{
				data->fashion[ret].timeout = 0;
			}
		}
		return ret;
	}

	if (data->n_fashion >= MAX_FASHION_NUM)
	{
		return -1;
	}
	data->fashion[data->n_fashion].id = id;
	data->fashion[data->n_fashion].color = color;
	data->fashion[data->n_fashion].isNew = true;
	data->fashion[data->n_fashion].timeout = expire;
	data->fashion[data->n_fashion].colordown = color;
	if (expire > 0)//新买
	{
		data->fashion[data->n_fashion].timeout = time_helper::get_cached_time() / 1000 + expire;
	}
	ret = data->n_fashion;
	++data->n_fashion;

	ActorFashionTable *table = get_config_by_id(id, &fashion_config);
	if (table != NULL)
	{
		data->charm_total += table->Charm;

		CharmTable *tableCharm = get_charm_table(data->charm_level);
		while (tableCharm != NULL && data->charm_total >= tableCharm->Exp)
		{
			if (data->charm_level == charm_config.size())
			{
				break;
			}
			++data->charm_level;
			data->charm_total -= tableCharm->Exp;
			tableCharm = get_charm_table(data->charm_level);
		}

		FashionCharm send;
		fashion_charm__init(&send);
		send.level = data->charm_level;
		send.charm = data->charm_total;

		EXTERN_DATA extern_data;
		extern_data.player_id = get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_FACTION_CHARM_NOTIFY, fashion_charm__pack, send);
	}
	return  ret;
}

void player_struct::unlock_color(uint32_t color)
{
	if (data->n_color >= MAX_COLOR_NUM)
	{
		return;
	}
	if (get_color(color) >= 0)
	{
		return;
	}
	data->color[data->n_color] = color;
	data->color_is_new[data->n_color] = true;
	++data->n_color;
}

void player_struct::unlock_weapon_color(uint32_t color)
{
	if (data->n_color >= MAX_WEAPON_COLOR_NUM)
	{
		return;
	}
	if (get_weapon_color(color) >= 0)
	{
		return;
	}
	data->weapon_color[data->n_weapon_color] = color;
	data->weapon_color_is_new[data->n_weapon_color] = true;
	++data->n_weapon_color;
}

int player_struct::get_color(uint32_t color)
{
	for (uint32_t i = 0; i < data->n_color; ++i)
	{
		if (data->color[i] == color)
		{
			return i;
		}
	}
	return -1;
}

int player_struct::get_weapon_color(uint32_t color)
{
	for (uint32_t i = 0; i < data->n_weapon_color; ++i)
	{
		if (data->weapon_color[i] == color)
		{
			return i;
		}
	}
	return -1;
}

void player_struct::set_fashion_color(uint32_t id, uint32_t color, bool down)
{
	int i = get_fashion(id);
	if (i < 0)
	{
		return;
	}
	if (down)
	{
		data->fashion[i].colordown = color;
	}
	else
	{
		data->fashion[i].color = color;
	}

}

void player_struct::set_fashion_old(uint32_t id)
{
	int i = get_fashion(id);
	if (i < 0)
	{
		return;
	}
	data->fashion[i].isNew = false;
}

void pack_fashion_info(FashionData &data, player_struct *player, int ind);
void player_struct::open_new_fashion(uint32_t level_old, uint32_t level_new)
{
	std::map<uint64_t, struct ActorFashionTable *>::iterator it = fashion_config.begin();
	for (; it != fashion_config.end(); ++it)
	{
		if (it->second->UnlockType != 3 || it->second->Occupation != get_attr(PLAYER_ATTR_JOB))
		{
			continue;
		}

		if (it->second->UnlockEffect1 > level_old &&  it->second->UnlockEffect1 <= level_new)
		{
			BuyFashionAnswer send;
			buy_fashion_answer__init(&send);
			FashionData data;
			send.data = &data;
			send.ret = 0;
			fashion_data__init(&data);

			int ret = add_fashion(it->first, it->second->Colour, 0);
			if (ret >= 0)
			{
				pack_fashion_info(data, this, ret);
			}
			else
			{
				send.ret = 190300008; //时装包裹满了
			}
			EXTERN_DATA extern_data;
			extern_data.player_id = get_uuid();
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_BUY_FASHION_ANSWER, buy_fashion_answer__pack, send);
		}
	}
}

void player_struct::check_fashion_expire()
{
	uint32_t carrerid = 101000000 + get_job();
	std::map<uint64_t, struct ActorTable *>::iterator it = actor_config.find(carrerid);
	if (it == actor_config.end())
	{
		return ;
	}
	AttrMap attrs;
	PutonFashionAns send;
	puton_fashion_ans__init(&send);
	send.ret = 0;
	bool have = false;
	for (uint32_t i = 0; i < data->n_fashion; )
	{
		if (data->fashion[i].timeout != 0
			&& data->fashion[i].timeout <= (time_t)time_helper::get_cached_time()/1000)
		{
			uint32_t factionId = data->fashion[i].id;
			if (i + 1 != data->n_fashion)
			{
				memcpy(data->fashion + i, data->fashion + data->n_fashion - 1, sizeof(FashionInfo));
			}
			--data->n_fashion;
			std::map<uint64_t, struct ActorFashionTable *>::iterator itFashion = fashion_config.find(factionId);
			if (itFashion == fashion_config.end())
			{
				continue;
			}

			have = true;
			CharmTable *tableCharm = get_charm_table(data->charm_level - 1);
			while (tableCharm != NULL && data->charm_total < itFashion->second->Charm  && data->charm_level > 1)
			{
				--data->charm_level;
				data->charm_total += tableCharm->Exp;
				tableCharm = get_charm_table(data->charm_level - 1);
			}
			if (data->charm_total < itFashion->second->Charm)
			{
				data->charm_total = 0;
			}
			else
			{
				data->charm_total -= itFashion->second->Charm;
			}

			send.id = factionId;
			if (itFashion->second->Type == FASHION_TYPE_WEAPON)
			{
				if (factionId != get_attr(PLAYER_ATTR_WEAPON))
				{
					continue;
				}
				attrs[PLAYER_ATTR_WEAPON] = it->second->WeaponId;
				set_attr(PLAYER_ATTR_WEAPON, it->second->WeaponId);
				send.newid = attrs[PLAYER_ATTR_WEAPON];
			}
			else if (itFashion->second->Type == FASHION_TYPE_HAT)
			{
				if (factionId != get_attr(PLAYER_ATTR_HAT))
				{
					continue;
				}

				attrs[PLAYER_ATTR_HAT] = it->second->HairResId[0];
				set_attr(PLAYER_ATTR_HAT, it->second->HairResId[0]);
				//data->fashion[i].color = itFashion->second->Colour;
				send.newid = attrs[PLAYER_ATTR_HAT];
				std::map<uint64_t, struct ActorFashionTable *>::iterator itFNew = fashion_config.find(it->second->HairResId[0]);
				if (itFNew != fashion_config.end())
				{
					if (itFNew->second->n_ColourID1 > 0)
					{
						attrs[PLAYER_ATTR_HAT_COLOR] = itFNew->second->ColourID1[0];
						set_attr(PLAYER_ATTR_HAT_COLOR, itFNew->second->ColourID1[0]);
					}
				}
			}
			else if (itFashion->second->Type == FASHION_TYPE_CLOTHES)
			{
				if (factionId != get_attr(PLAYER_ATTR_CLOTHES))
				{
					continue;
				}

				//data->fashion[i].color = itFashion->second->Colour;
				//data->fashion[i].colordown = itFashion->second->Colour;
				attrs[PLAYER_ATTR_CLOTHES] = it->second->ResId[0];
				set_attr(PLAYER_ATTR_CLOTHES, it->second->ResId[0]);
				send.newid = attrs[PLAYER_ATTR_CLOTHES];
				std::map<uint64_t, struct ActorFashionTable *>::iterator itFNew = fashion_config.find(it->second->ResId[0]);
				if (itFNew != fashion_config.end())
				{
					if (itFNew->second->n_ColourID1 > 0)
					{
						attrs[PLAYER_ATTR_CLOTHES_COLOR_UP] = itFNew->second->ColourID1[0];
						set_attr(PLAYER_ATTR_CLOTHES_COLOR_UP, itFNew->second->ColourID1[0]);
					}
					if (itFNew->second->n_ColourID2 > 0)
					{
						attrs[PLAYER_ATTR_CLOTHES_COLOR_DOWN] = itFNew->second->ColourID2[0];
						set_attr(PLAYER_ATTR_CLOTHES_COLOR_DOWN, itFNew->second->ColourID2[0]);
					}
				}
			}
		}
		else {
			++i;
		}
	}

	EXTERN_DATA extern_data;
	extern_data.player_id = get_uuid();
	if (have)
	{
		FashionCharm sendCharm;
		fashion_charm__init(&sendCharm);
		sendCharm.level = data->charm_level;
		sendCharm.charm = data->charm_total;
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_FACTION_CHARM_NOTIFY, fashion_charm__pack, sendCharm);
	}

	if (attrs.size() > 0)
	{
		notify_attr(attrs, true,true);
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_TAKEOFF_FASHION_ANSWER, puton_fashion_ans__pack, send);
	}
}

int player_struct::get_horse(uint32_t id)
{
	for (uint32_t i = 0; i < data->n_horse; ++i)
	{
		if (data->horse[i].id == id)
		{
			return i;
		}
	}
	return -1;
}

int player_struct::add_horse(uint32_t id, time_t expire)
{
	if (data->n_horse >= MAX_HORSE_NUM)
	{
		return -1;
	}
	int ret = get_horse(id);
	if (ret >= 0) //已买
	{
		if (data->horse[ret].timeout > 0)
		{
			if (expire > 0)
			{
				if (data->horse[ret].timeout <= (time_t)time_helper::get_cached_time() / 1000)
				{
					data->horse[ret].timeout = time_helper::get_cached_time() / 1000 + expire;
				}
				else
				{
					data->horse[ret].timeout += expire;
				}
			}
			else
			{
				data->horse[ret].timeout = 0;
			}
		}
		return ret;
	}


	data->horse[data->n_horse].id = id;
	data->horse[data->n_horse].isNew = true;
	data->horse[data->n_horse].timeout = expire;
	if (expire > 0)//新买
	{
		data->horse[data->n_horse].timeout = time_helper::get_cached_time() / 1000 + expire;
	}
	int pos = data->n_horse;
	++data->n_horse;

	return pos;
}

void player_struct::notify_add_horse(int i)
{
	BuyHorseAns send;
	buy_horse_ans__init(&send);
	HorseData hData;
	send.data = &hData;
	horse_data__init(&hData);
	hData.id = data->horse[i].id;
	hData.isnew = data->horse[i].isNew;

	if (data->horse[i].timeout != 0)
	{
		if (data->horse[i].timeout <= (time_t)time_helper::get_cached_time() / 1000)
		{
			hData.isexpire = true;
		}
		else
		{
			hData.cd = data->horse[i].timeout - time_helper::get_cached_time() / 1000;
			hData.isexpire = false;
		}
	}
	else
	{
		hData.isexpire = false;
		hData.cd = 0;
	}
	hData.is_current = false;

	EXTERN_DATA extern_data;
	extern_data.player_id = get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_BUY_HORSE_ANSWER, buy_horse_ans__pack, send);
}

int player_struct::get_on_horse_id()
{
	if (data->attrData[PLAYER_ATTR_CUR_HORSE] != 0 && data->attrData[PLAYER_ATTR_ON_HORSE_STATE] != 0)
	{
		return data->attrData[PLAYER_ATTR_CUR_HORSE];
	}
	return 0;
}

void player_struct::unpack_horse(_PlayerDBInfo *db_info)
{
	for (uint32_t i = 0; i < db_info->n_horse_data; ++i)
	{

		data->horse[i].id = db_info->horse_data[i]->id;
		data->horse[i].isNew = db_info->horse_data[i]->isnew;
		data->horse[i].timeout = db_info->horse_data[i]->cd;
		++data->n_horse;
	}

	data->horse_attr.soul = db_info->horse_attr->soul_level;
	data->horse_attr.cur_soul = db_info->horse_attr->cur_soul;
	data->horse_attr.fly = db_info->horse_attr->fly;
	for (uint32_t i = 0; i < db_info->horse_attr->n_soul_num; ++i)
	{
		data->horse_attr.soul_exp[i + 1] = db_info->horse_attr->soul_num[i];
	}

	data->horse_attr.step = db_info->horse_attr->step;
	for (uint32_t i = 0; i < db_info->horse_attr->n_attr; ++i)
	{
		data->horse_attr.attr[i] = db_info->horse_attr->attr[i];
	}
	for (uint32_t i = 0; i < db_info->horse_attr->n_attr_level; ++i)
	{
		data->horse_attr.attr_exp[i] = db_info->horse_attr->attr_level[i];
	}
	data->horse_attr.n_attr = db_info->horse_attr->n_attr;
	data->horse_attr.soul_full = db_info->horse_attr->soul_full;
}

void player_struct::calc_horse_attr()
{
	memset(data->horse_attr.total, 0, sizeof(data->horse_attr.total));
	std::map<uint64_t, struct MountsTable*>::iterator itBase = horse_config.find(get_attr(PLAYER_ATTR_CUR_HORSE));
	if (itBase != horse_config.end())
	{
		for (uint32_t i = 0; i < itBase->second->n_MountsAttribute; ++i)
		{
			data->horse_attr.total[itBase->second->MountsAttribute[i]] += itBase->second->AttributeCeiling[i];
		}
	}
	std::map<uint64_t, struct SpiritTable*>::iterator itSp = spirit_config.find(data->horse_attr.step);
	if (itSp != spirit_config.end())
	{
		if (data->horse_attr.step > 1)
		{
			std::map<uint64_t, struct SpiritTable*>::iterator itSp1 = spirit_config.find(data->horse_attr.step - 1);
			if (itSp1 != spirit_config.end())
			{
				for (uint32_t i = 0; i < itSp->second->n_SpiritAttribute; ++i)
				{
					data->horse_attr.total[itSp->second->SpiritAttribute[i]] += (itSp->second->OrderAttribute[data->horse_attr.step - 2] +
						(itSp->second->AttributeCeiling[i] - itSp1->second->AttributeCeiling[i]) * data->horse_attr.attr_exp[i] / itSp->second->GradeNum[i] +
						itSp1->second->AttributeCeiling[i]);
				}
			}
		}
		else
		{
			for (uint32_t i = 0; i < itSp->second->n_SpiritAttribute; ++i)
			{
				data->horse_attr.total[itSp->second->SpiritAttribute[i]] += (itSp->second->AttributeCeiling[i] * data->horse_attr.attr_exp[i] / itSp->second->GradeNum[i]);
			}
		}
	}
	for (uint32_t i = 1; i <= MAX_HORSE_SOUL; ++i)
	{
		std::map<uint64_t, struct CastSpiritTable*>::iterator itCa = horse_soul_config.find(i);
		if (itCa == horse_soul_config.end())
		{
			continue;
		}
		if (data->horse_attr.soul <= 1)
		{
			data->horse_attr.total[itCa->second->CastSpiritAttribute[0]] += (itCa->second->CastAttributeCeiling[0] * data->horse_attr.soul_exp[i] / itCa->second->GradeNum[0]);
		}
		else
		{
			data->horse_attr.total[itCa->second->CastSpiritAttribute[0]] += (itCa->second->OrderAttribute[data->horse_attr.soul - 2] +
				(itCa->second->CastAttributeCeiling[data->horse_attr.soul] - itCa->second->CastAttributeCeiling[data->horse_attr.soul - 1]) * data->horse_attr.soul_exp[i] / itCa->second->GradeNum[data->horse_attr.soul - 1] +
				itCa->second->CastAttributeCeiling[data->horse_attr.soul - 1]);
		}
	}
	for (uint32_t i = 1; i < PLAYER_ATTR_FIGHT_MAX; ++i)
	{
		data->horse_attr.power += data->horse_attr.total[i] * sg_fighting_capacity_coefficient[i];
	}
}

void player_struct::calc_horse_attr(double *attr)
{
	memset(attr, 0, sizeof(double) * PLAYER_ATTR_MAX);
	for (uint32_t i = 1; i < PLAYER_ATTR_FIGHT_MAX; ++i)
	{
		if (i == PLAYER_ATTR_MOVE_SPEED)
		{
			continue;
		}
		attr[i] += data->horse_attr.total[i];
	}
	if (get_attr(PLAYER_ATTR_CUR_HORSE) != 0 && get_attr(PLAYER_ATTR_ON_HORSE_STATE) != 0)
	{
		set_attr(PLAYER_ATTR_MOVE_SPEED, data->horse_attr.total[PLAYER_ATTR_MOVE_SPEED]);
		if (data->horse_attr.fly == 2)
		{
			set_attr(PLAYER_ATTR_MOVE_SPEED, data->horse_attr.total[PLAYER_ATTR_FLY_SPEED]);
		}
	}
}

void player_struct::check_horse_expire()
{
	for (uint32_t i = 0; i < data->n_horse;)
	{
		if (data->horse[i].timeout == 0)
		{
			++i;
			continue;
		}
		if (time_helper::get_cached_time() / 1000 >= (uint64_t)data->horse[i].timeout)
		{
			if (data->horse[i].id == get_attr(PLAYER_ATTR_CUR_HORSE))
			{
				int oldHorse = data->attrData[PLAYER_ATTR_CUR_HORSE];
				data->attrData[PLAYER_ATTR_CUR_HORSE] = DEFAULT_HORSE;
				SetCurHorseAns send;
				set_cur_horse_ans__init(&send);
				send.ret = 0;
				send.id = DEFAULT_HORSE;
				calc_horse_attr();
				send.power = data->horse_attr.power;
				send.playerid = this->get_uuid();
				send.old_id = oldHorse;
				broadcast_to_sight(MSG_ID_SET_CUR_HORSE_ANSWER, &send, (pack_func)set_cur_horse_ans__pack, true);
				calculate_attribute(true);
			}


			MountsTable *table = get_config_by_id(data->horse[i].id, &horse_config);
			std::vector<char *> args;
			if (table != NULL)
			{
				args.push_back(table->Name);
			}
			send_mail(270300029, NULL, NULL, NULL, &args, NULL, 0);

			if (i + 1 != data->n_horse)
			{
				memcpy(data->horse + i, data->horse + data->n_horse - 1, sizeof(HorseInfo));
			}
			--data->n_horse;
		}
		else
		{
			++i;
		}
	}
}
void player_struct::check_guoyu_expire()
{
	if (data->guoyu.cur_task != 0)
	{
		if (data->guoyu.task_timeout < time_helper::get_cached_time() / 1000)
		{
			data->guoyu.cur_task = 0; //过期
			GuoyuSucc send;
			guoyu_succ__init(&send);
			send.succ = false;
			EXTERN_DATA extern_data;
			extern_data.player_id = get_uuid();
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_GUOYU_TASK_SUCC_NOTIFY, guoyu_succ__pack, send);

			if (data->scene_id < SCENCE_DEPART)
			{
				return;
			}
			if (scene == NULL)
			{
				return;
			}
			raid_struct *raid = (raid_struct *)scene;
			if (raid->m_config->DengeonRank != DUNGEON_TYPE_YAOSHI)
			{
				return;
			}
			GuoyuFbSucc notify;
			guoyu_fb_succ__init(&notify);
			notify.succ = 1;
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_GUOYU_FB_SUCC_NOTIFY, guoyu_fb_succ__pack, notify);
		}
	}
}

uint32_t player_struct::get_first_skill_id()
{
	return m_skill.GetFirstSkillId();
}

void player_struct::do_taunt_action()
{
	if (!area || !scene || !data)
		return;

	if (is_unit_in_move())
		return;

	unit_struct *target = get_taunt_target();
	if (!target || !target->is_avaliable()
		|| !target->is_alive())
		return;

	struct position *my_pos = get_pos();
	struct position *his_pos = target->get_pos();

//	data->cur_skill.skill_id = 0;

	uint32_t skill_id = get_first_skill_id();
	if (skill_id == 0)
		return;
	struct SkillTable *config = get_config_by_id(skill_id, &skill_config);
	if (config == NULL)
	{
		return;
	}
	if (!check_distance_in_range(my_pos, his_pos, config->SkillRange))
	{
		reset_pos();

		data->move_path.pos[1].pos_x = his_pos->pos_x;
		data->move_path.pos[1].pos_z = his_pos->pos_z;

		data->move_path.start_time = time_helper::get_cached_time();
		data->move_path.max_pos = 1;
		data->move_path.cur_pos = 0;

		MoveNotify notify;
		move_notify__init(&notify);
		notify.playerid = data->player_id;
		notify.data = pack_unit_move_path(&notify.n_data);

		broadcast_to_sight(MSG_ID_MOVE_NOTIFY, &notify, (pack_func)move_notify__pack, true);
		reset_pos_pool();
	}
	else
	{
		if (data->cur_skill.skill_id == 0)
		{
			data->cur_skill.start_time = time_helper::get_cached_time();
			data->cur_skill.skill_id = skill_id;
			data->cur_skill.direct_x = his_pos->pos_x - my_pos->pos_x;
			data->cur_skill.direct_z = his_pos->pos_z - my_pos->pos_z;

			SkillCastNotify notify;
			PosData pos_data;
			struct position *pos = get_pos();
			pos_data__init(&pos_data);
			pos_data.pos_x = pos->pos_x;
			pos_data.pos_z = pos->pos_z;
			skill_cast_notify__init(&notify);
			notify.playerid = data->player_id;
			notify.skillid = skill_id;
			notify.cur_pos = &pos_data;
			notify.direct_x = data->cur_skill.direct_x;
			notify.direct_z = data->cur_skill.direct_z;
			broadcast_to_sight(MSG_ID_SKILL_CAST_NOTIFY, &notify, (pack_func)skill_cast_notify__pack, true);
		}
		else
		{
			struct ActiveSkillTable *active_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
			if (!active_config)
				return;
			uint32_t delay_time = active_config->TotalSkillDelay;
//			for (uint32_t i = 0; i < active_config->n_SkillLength; ++i)
//				delay_time += active_config->SkillLength[i];
			if (delay_time > time_helper::get_cached_time() - data->cur_skill.start_time)
				return;
			clear_cur_skill();
		}
	}

}

void player_struct::add_guoyu_exp(uint32_t num)
{
	SpecialtyLevelTable *table = get_specialty_level_table(MAJOR__TYPE__GUOYU, data->guoyu.guoyu_level + 1);
	if (table == NULL)
	{
		return;
	}
	//if (data->cur_yaoshi == MAJOR__TYPE__GUOYU)
	//{
	//	SpecialTitleTable *title = get_yaoshi_title_table(data->cur_yaoshi, data->guoyu.guoyu_level);
	//	if (title != NULL)
	//	{
	//		num = num * (10000 + title->TitleEffect2) / 10000;
	//	}
	//}
	data->guoyu.cur_exp += num;
	table = get_specialty_level_table(MAJOR__TYPE__GUOYU, data->guoyu.guoyu_level);
	while (table != NULL && data->guoyu.cur_exp >= (int32_t)table->LevelExp)
	{
		data->guoyu.cur_exp -= table->LevelExp;
		++data->guoyu.guoyu_level;
		table = get_specialty_level_table(MAJOR__TYPE__GUOYU, data->guoyu.guoyu_level + 1);
		if (table == NULL)
		{
			break;
		}
		table = get_specialty_level_table(MAJOR__TYPE__GUOYU, data->guoyu.guoyu_level);
	}

	YaoshiLevelExp send;
	yaoshi_level_exp__init(&send);
	send.type = MAJOR__TYPE__GUOYU;
	send.level = data->guoyu.guoyu_level;
	send.exp = data->guoyu.cur_exp;
	EXTERN_DATA extern_data;
	extern_data.player_id = get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_YAOSHI_EXP_NOTIFY, yaoshi_level_exp__pack, send);
}

void player_struct::add_chengjie_exp(uint32_t num)
{
	SpecialtyLevelTable *table = get_specialty_level_table(MAJOR__TYPE__CHENGJIE, data->chengjie.level + 1);
	if (table == NULL)
	{
		return;
	}
	//if (data->cur_yaoshi == MAJOR__TYPE__CHENGJIE)
	//{
	//	SpecialTitleTable *title = get_yaoshi_title_table(data->cur_yaoshi, data->chengjie.level);
	//	if (title != NULL)
	//	{
	//		num = num * (10000 + title->TitleEffect2) / 10000;
	//	}
	//}
	data->chengjie.cur_exp += num;
	table = get_specialty_level_table(MAJOR__TYPE__CHENGJIE, data->chengjie.level);
	while (table != NULL && data->chengjie.cur_exp >= (int32_t)table->LevelExp)
	{
		data->chengjie.cur_exp -= table->LevelExp;
		++data->chengjie.level;
		table = get_specialty_level_table(MAJOR__TYPE__CHENGJIE, data->chengjie.level + 1);
		if (table == NULL)
		{
			break;
		}
		table = get_specialty_level_table(MAJOR__TYPE__CHENGJIE, data->chengjie.level);
	}

	YaoshiLevelExp send;
	yaoshi_level_exp__init(&send);
	send.type = MAJOR__TYPE__CHENGJIE;
	send.level = data->chengjie.level;
	send.exp = data->chengjie.cur_exp;
	EXTERN_DATA extern_data;
	extern_data.player_id = get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_YAOSHI_EXP_NOTIFY, yaoshi_level_exp__pack, send);

	//系统提示
	std::vector<char *> args;
	std::stringstream ss_num;
	ss_num << num;
	args.push_back(const_cast<char*>(ss_num.str().c_str()));
	send_system_notice(190500167, &args);
}

void player_struct::add_chengjie_courage(uint32_t num)
{

	data->attrData[PLAYER_ATTR_COURAGE_GOLD] += num;

	AttrMap attrs;
	attrs[PLAYER_ATTR_COURAGE_GOLD] = data->attrData[PLAYER_ATTR_COURAGE_GOLD];
	this->notify_attr(attrs);

	//系统提示
	std::vector<char *> args;
	std::stringstream ss_num;
	ss_num << num;
	args.push_back(const_cast<char*>(ss_num.str().c_str()));
	send_system_notice(190500170, &args);

}

void player_struct::add_shangjin_exp(uint32_t num)
{
	SpecialtyLevelTable *table = get_specialty_level_table(MAJOR__TYPE__SHUANGJIN, data->shangjin.level + 1);
	if (table == NULL)
	{
		return;
	}
	//if (data->cur_yaoshi == MAJOR__TYPE__SHUANGJIN)  //专精加成
	//{
	//	SpecialTitleTable *title = get_yaoshi_title_table(data->cur_yaoshi, data->shangjin.level);
	//	if (title != NULL)
	//	{
	//		num = num * (10000 + title->TitleEffect2) / 10000;
	//	}
	//}
	SpecialtySkillTable *tableSkillOld = get_yaoshi_skill_config(SHANGJIN_THREE, data->shangjin.level);
	data->shangjin.cur_exp += num;
	table = get_specialty_level_table(MAJOR__TYPE__SHUANGJIN, data->shangjin.level);
	bool up = false;
	while (table != NULL && data->shangjin.cur_exp >= (int32_t)table->LevelExp)
	{
		data->shangjin.cur_exp -= table->LevelExp;
		++data->shangjin.level;
		up = true;
		table = get_specialty_level_table(MAJOR__TYPE__SHUANGJIN, data->shangjin.level + 1);
		if (table == NULL)
		{
			break;
		}
		table = get_specialty_level_table(MAJOR__TYPE__SHUANGJIN, data->shangjin.level);
	}
	SpecialtySkillTable *tableSkill = get_yaoshi_skill_config(SHANGJIN_THREE, data->shangjin.level);
	int add = 0;
	if (up && tableSkill != NULL && tableSkillOld != NULL)
	{
		 add = tableSkill->EffectValue[0] - tableSkillOld->EffectValue[0];
	}
	else if (up && tableSkill != NULL && tableSkillOld == NULL)
	{
		 add = tableSkill->EffectValue[0];
	}
	if (add > 0)
	{
		data->shangjin.free += add;
		send_all_yaoshi_num();
	}
	YaoshiLevelExp send;
	yaoshi_level_exp__init(&send);
	send.type = MAJOR__TYPE__SHUANGJIN;
	send.level = data->shangjin.level;
	send.exp = data->shangjin.cur_exp;
	EXTERN_DATA extern_data;
	extern_data.player_id = get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_YAOSHI_EXP_NOTIFY, yaoshi_level_exp__pack, send);

	//系统提示
	std::vector<char *> args;
	std::stringstream ss_num;
	ss_num << num;
	args.push_back(const_cast<char*>(ss_num.str().c_str()));
	send_system_notice(190500168, &args);
}

void player_struct::send_all_yaoshi_num()
{
	AllYaoshiNum send;
	all_yaoshi_num__init(&send);
	send.critical_num = data->guoyu.critical_num;
	send.guoyu_num = data->guoyu.guoyu_num;
	send.shangjin_num = data->shangjin.shangjin_num;
	send.chengjie_num = data->chengjie.chengjie_num;
	send.shangjin_free = data->shangjin.free;
	EXTERN_DATA extern_data;
	extern_data.player_id = get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_YAOSHI_ALL_NUM_NOTIFY, all_yaoshi_num__pack, send);
}

void player_struct::refresh_yaoshi_oneday()
{
	TypeLevelTable *table = get_guoyu_level_table(GUOYU__TASK__TYPE__CRITICAL);
	if (table != NULL)
	{
		data->guoyu.guoyu_num = table->RewardTime;
		data->guoyu.critical_num = table->ShowTimes;
	}

	ParameterTable *param_config = get_config_by_id(161000070, &parameter_config);
	if (param_config != NULL)
	{
		data->chengjie.chengjie_num = param_config->parameter1[0];
	}

	param_config = get_config_by_id(161000106, &parameter_config);
	if (param_config != NULL)
	{
		data->shangjin.shangjin_num = param_config->parameter1[0];
	}
	SpecialtySkillTable *tableSkill = get_yaoshi_skill_config(SHANGJIN_THREE, data->shangjin.level);
	if (tableSkill != NULL)
	{
		data->shangjin.free = tableSkill->EffectValue[0];
	}

	send_all_yaoshi_num();

	//镖车
	for (std::map<uint64_t, struct BiaocheTable*>::iterator it = cash_truck_config.begin(); it != cash_truck_config.end(); ++it)
	{
		ControlTable *tableAct = get_config_by_id(it->second->ActivityControl, &all_control_config);
		if (it->second->Type == 1)
		{
			data->truck.num_coin = tableAct->RewardTime;
		}
		else
		{
			data->truck.num_gold = tableAct->RewardTime;
		}
	}
	CashTruckInfo note;
	cash_truck_info__init(&note);
	note.num_cion = data->truck.num_coin;
	note.num_gold = data->truck.num_gold;
	note.cash_truck = data->truck.truck_id;
	note.mapid = data->truck.scene_id;
	note.x = data->truck.pos.pos_x;
	note.z = data->truck.pos.pos_z;
	EXTERN_DATA extern_data;
	extern_data.player_id = get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_CASH_TRUCK_INFO_NOTIFY, cash_truck_info__pack, note);
}

void player_struct::logout_check_award_question()
{
	if (data->award_answer.bOpenWin)
	{
		ParameterTable * config1 = get_config_by_id(161000170, &parameter_config);
		ParameterTable * config2 = get_config_by_id(161000169, &parameter_config);
		ParameterTable * config3 = get_config_by_id(161000168, &parameter_config);
		add_task_progress(TCT_QUESTION, 0, config2->parameter1[0] - data->award_answer.number + 1);
		data->award_answer.timer += (config1->parameter1[0] * (config2->parameter1[0] - data->award_answer.number + 1));
		data->award_answer.number = 1;
		if (data->award_answer.trun == config3->parameter1[0])
		{
			data->award_answer.question = 0;
		}
		else
		{
			data->award_answer.number = 1;
			++data->award_answer.trun;
			data->award_answer.question = get_rand_question(sg_award_question);
		}
	}
}

void player_struct::pack_answer_db(_PlayerDBInfo &db_info)
{
	static DBCommonQuestion send;
	dbcommon_question__init(&send);
	send.question = data->common_answer.question;
	send.contin = data->common_answer.contin;
	send.right = data->common_answer.right;
	send.money = data->common_answer.money;
	send.exp = data->common_answer.exp;
	send.tip = data->common_answer.tip;
	send.help = data->common_answer.help;
	send.btip = data->common_answer.btip;
	send.bhelp = data->common_answer.bhelp;
	send.number = data->common_answer.number;
	send.answer = data->common_answer.answer;
	send.anstip = data->common_answer.answer_tip;
	send.next_open = data->common_answer.next_open;
	send.n_answer = MAX_QUESTION_ANSWER;
	send.n_anstip = 2;

	static DBAwardQuestion send1;
	dbaward_question__init(&send1);
	send1.question = data->award_answer.question;
	send1.contin = data->award_answer.contin;
	send1.right = data->award_answer.right;
	send1.money = data->award_answer.money;
	send1.exp = data->award_answer.exp;
	send1.number = data->award_answer.number;
	send1.trun = data->award_answer.trun;
	send1.timer = data->award_answer.timer;
	send1.next_open = data->award_answer.next_open;

	db_info.award_question = &send1;
	db_info.common_question = &send;
}

void player_struct::unpack_answer_db(_PlayerDBInfo *db_info)
{
	if (db_info->common_question != NULL)
	{
		data->common_answer.question = db_info->common_question->question;
		data->common_answer.contin = db_info->common_question->contin;
		data->common_answer.right = db_info->common_question->right;
		data->common_answer.money = db_info->common_question->money;
		data->common_answer.exp = db_info->common_question->exp;
		data->common_answer.tip = db_info->common_question->tip;
		data->common_answer.help = db_info->common_question->help;
		data->common_answer.btip = db_info->common_question->btip;
		data->common_answer.bhelp = db_info->common_question->bhelp;
		data->common_answer.number = db_info->common_question->number;
		data->common_answer.next_open = db_info->common_question->next_open;
		memcpy(data->common_answer.answer, db_info->common_question->answer, sizeof(uint32_t) * 4);
		memcpy(data->common_answer.answer_tip, db_info->common_question->anstip, sizeof(uint32_t) * 2);
	}

	if (db_info->award_question != NULL)
	{
		data->award_answer.question = db_info->award_question->question;
		data->award_answer.contin = db_info->award_question->contin;
		data->award_answer.right = db_info->award_question->right;
		data->award_answer.money = db_info->award_question->money;
		data->award_answer.exp = db_info->award_question->exp;
		data->award_answer.number = db_info->award_question->number;
		data->award_answer.trun = db_info->award_question->trun;
		data->award_answer.timer = db_info->award_question->timer;
		data->award_answer.next_open = db_info->award_question->next_open;
	}
}

void player_struct::clear_award_question()
{
	EventCalendarTable *tableAct = get_config_by_id(AWARD_QUESTION_ACTIVE_ID, &activity_config);
	if (tableAct == NULL)
	{
		return;
	}
	uint32_t cd = 0;
	if (data->award_answer.question != 0 && !check_active_open(tableAct->RelationID, cd))
	{
		data->award_answer.question = 0;
		data->award_answer.contin = 0;
		data->award_answer.exp = 0;
		data->award_answer.money = 0;
		data->award_answer.number = 1;
		data->award_answer.timer = 0;
		data->award_answer.trun = 1;

		for (int i = 0; i < MAX_TASK_ACCEPTED_NUM; ++i)
		{
			if (data->task_list[i].id != 0)
			{
				TaskTable *config = get_config_by_id(data->task_list[i].id, &task_config);
				if (config->TaskType == TT_QUESTION)
				{
					set_task_fail(&data->task_list[i]);
					break;
				}

			}
		}
	}
}


void player_struct::set_team_raid_id_wait_ready(uint32_t raid_id)
{
	LOG_INFO("%s: player[%lu] old[%u] new[%u]", __FUNCTION__, data->player_id, data->team_raid_id_wait_ready, raid_id);
	data->team_raid_id_wait_ready = raid_id;
	data->is_team_raid_ready = false;
}
void player_struct::unset_team_raid_id_wait_ready()
{
	LOG_INFO("%s: player[%lu] old[%u]", __FUNCTION__, data->player_id, data->team_raid_id_wait_ready);
	data->team_raid_id_wait_ready = 0;
	data->is_team_raid_ready = false;
}

void player_struct::clear_temp_data()
{
	m_skill.clear();
	m_team = NULL;
	data->teamid = 0;
	m_hitMe.clear();
	m_meHit.clear();
	xun_map_id.clear();
	chengjie_kill = 0;
	watched_player_id.clear();
}
