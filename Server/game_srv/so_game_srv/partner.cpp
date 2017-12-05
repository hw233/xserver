#include "partner.h"
#include "msgid.h"
#include "uuid.h"
#include "path_algorithm.h"
#include "buff.h"
#include "attr_calc.h"
//#include "cached_hit_effect.h"
//#include "count_skill_damage.h"
//#include "skill_manager.h"
#include "player_manager.h"
#include "partner_manager.h"
#include "cash_truck_manager.h"
#include "monster_ai.h"
#include "monster_manager.h"
#include "check_range.h"
#include "camp_judge.h"
#include "cash_truck.h"
#include "global_param.h"
#include "monster.h"
#include "role.pb-c.h"
#include "partner.pb-c.h"
#include "cast_skill.pb-c.h"
#include <math.h>

partner_struct::partner_struct(void)
{
	config = NULL;
	data = NULL;
	partner_sight_space = NULL;
}

partner_struct::~partner_struct(void)
{
}

bool partner_struct::can_beattack()
{
	return true;
}

bool partner_struct::is_in_safe_region()
{
	return false;
}

int partner_struct::get_camp_id()         //红蓝阵营关系, 战斗关系优先级最高, 一方为0的时候被忽略
{
	return 0;
}

void partner_struct::set_camp_id(int id)
{
}

Team *partner_struct::get_team()
{
	return NULL;
}

void partner_struct::clear_cur_skill() //嘲讽的时候打断当前技能
{
}

UNIT_TYPE partner_struct::get_unit_type()
{
	return UNIT_TYPE_PARTNER;
}

uint64_t partner_struct::get_uuid()
{
	return (data ? data->uuid : 0);
}

double *partner_struct::get_all_attr()
{
	return (data ? data->attrData : NULL);
}

double *partner_struct::get_all_buff_fight_attr()
{
	return (data ? data->buff_fight_attr : NULL);
}

double partner_struct::get_attr(uint32_t id)
{
	return (data ? data->attrData[id] : 0);
}

double partner_struct::get_buff_fight_attr(uint32_t id)
{
	return (data ? data->buff_fight_attr[id] : 0);
}

void partner_struct::set_attr(uint32_t id, double value)
{
	data->attrData[id] = value;
}

struct unit_path *partner_struct::get_unit_path()
{
	return &data->move_path;
}

float partner_struct::get_speed()
{
	return data->attrData[PLAYER_ATTR_MOVE_SPEED];
}

void partner_struct::update_sight(area_struct *old_area, area_struct *new_area)
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
	uint64_t delete_player_id[MAX_PLAYER_IN_PLAYER_SIGHT];
//	int add_player_id_index = 0;
//	uint64_t add_player_id[MAX_PLAYER_IN_PLAYER_SIGHT];
	
	SightChangedNotify notify;
	sight_changed_notify__init(&notify);

	del_sight_player_in_area(n_del, &del_area[0], &delete_player_id_index, &delete_player_id[0]);
	del_sight_monster_in_area(n_del, &del_area[0]);
	del_sight_truck_in_area(n_del, &del_area[0]);
	del_sight_partner_in_area(n_del, &del_area[0]);	

		//发送给需要在视野里面删除怪物的通知
	notify.n_delete_monster = 1;
	uint64_t player_ids[1];
	player_ids[0] = data->uuid;
	notify.delete_monster = player_ids;
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
		add_area_truck_to_sight(add_area[i]);
		add_area_partner_to_sight(add_area[i]);				
	}
	if (add_player_id_index > 0)
	{
		sight_changed_notify__init(&notify);		
		SightPartnerInfo my_player_info[1];
		SightPartnerInfo *my_player_info_point[1];	
		my_player_info_point[0] = &my_player_info[0];
		notify.add_partner = my_player_info_point;
		pack_sight_partner_info(my_player_info_point[0]);
			//发送给需要在视野里面添加玩家的通知
		notify.n_add_partner = 1;
		
		ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
		for (int i = 0; i < add_player_id_index; ++i)
			conn_node_gamesrv::broadcast_msg_add_players(delete_player_id[i], ppp);
		conn_node_gamesrv::broadcast_msg_send();
	}

	reset_pools();

//	LOG_DEBUG("%s %d: monster[%lu] del player[%u] add player[%u] area from %d to %d", __FUNCTION__, __LINE__,
//		data->player_id, delete_player_id_index, add_player_id_index,
//		old_area - scene->m_area, new_area - scene->m_area);
	
	if (old_area->del_partner_from_area(data->uuid) != 0)
	{
		LOG_ERR("%s %d: can not del partner[%lu] from area[%ld %p]", __FUNCTION__, __LINE__, data->uuid, old_area - scene->m_area, old_area);		
	}
	new_area->add_partner_to_area(data->uuid);
	area = new_area;
}

bool partner_struct::on_truck_leave_sight(uint64_t player_id)
{
	return true;
}
bool partner_struct::on_truck_enter_sight(uint64_t player_id)
{
	return true;
}

bool partner_struct::on_player_leave_sight(uint64_t player_id)
{
	return true;
}

bool partner_struct::on_player_enter_sight(uint64_t player_id)
{
	return true;
}

bool partner_struct::on_monster_leave_sight(uint64_t uuid)
{
	return true;
}

bool partner_struct::on_monster_enter_sight(uint64_t uuid)
{
	return true;
}
bool partner_struct::on_partner_leave_sight(uint64_t uuid)
{
	return true;
}

bool partner_struct::on_partner_enter_sight(uint64_t uuid)
{
	return true;
}

int *partner_struct::get_cur_sight_player()
{
	return &data->cur_sight_player;
}

uint64_t *partner_struct::get_all_sight_player()
{
	return &data->sight_player[0];
}
int *partner_struct::get_cur_sight_partner()
{
	return &data->cur_sight_partner;
}

uint64_t *partner_struct::get_all_sight_partner()
{
	return &data->sight_partner[0];
}

int *partner_struct::get_cur_sight_monster()
{
	return &data->cur_sight_monster;
}

uint64_t *partner_struct::get_all_sight_monster()
{
	return &data->sight_monster[0];
}

int *partner_struct::get_cur_sight_truck()
{
	return &data->cur_sight_truck;
}

uint64_t *partner_struct::get_all_sight_truck()
{
	return &data->sight_truck[0];
}

void partner_struct::on_hp_changed(int damage)
{
	if (ai && ai->on_hp_changed)
		ai->on_hp_changed(this);
}

void partner_struct::on_dead(unit_struct *killer)
{
	LOG_DEBUG("[%s:%d] player[%lu] partner[%lu][%u]", __FUNCTION__, __LINE__, data->owner_id, data->uuid, data->partner_id);
	data->relive_time = time_helper::get_cached_time() / 1000 + sg_partner_relive_time;
	m_owner->del_partner_from_scene(this, false);

	PartnerReliveTimeNotify nty;
	partner_relive_time_notify__init(&nty);
	nty.uuid = data->uuid;
	nty.relivetime = data->relive_time;
	EXTERN_DATA ext_data;
	ext_data.player_id = data->owner_id;
	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_PARTNER_RELIVE_TIME_NOTIFY, partner_relive_time_notify__pack, nty);
}

void partner_struct::on_repel(unit_struct *player)  //击退
{
}

bool partner_struct::is_avaliable()
{
	return data != NULL && scene != NULL;
}

uint32_t partner_struct::get_skill_id()
{
	return 0;
}

JobDefine partner_struct::get_job()
{
	return JOB_DEFINE_MONSTER;
}

void partner_struct::do_taunt_action()  //嘲讽
{
}

void partner_struct::update_partner_pos_and_sight()
{
	if (!scene)
		return;
	if (get_speed() < __DBL_EPSILON__)
		return;
	if (!is_unit_in_move())
		return;

	area_struct *old_area = area;
	if (update_unit_position() == 0)
		return;
	if (!old_area)
		return;
	struct position *pos = get_pos();	
	area_struct *new_area = scene->get_area_by_pos(pos->pos_x, pos->pos_z);
		//检查是否越过area
	if (old_area == new_area || !new_area)
		return;

	update_sight(old_area, new_area);
}

void partner_struct::reset_timer(uint64_t time)
{
	data->ontick_time = time;
	partner_manager::partner_ontick_reset_timer(this);
}

void partner_struct::on_relive()
{
	data->relive_time = 0;
	data->attrData[PLAYER_ATTR_HP] = data->attrData[PLAYER_ATTR_MAXHP];
	notify_one_attr_changed(PLAYER_ATTR_HP, data->attrData[PLAYER_ATTR_HP]);
	on_hp_changed(0);

	m_owner->adjust_battle_partner();
}

void partner_struct::set_timer(uint64_t time)
{
	data->ontick_time = time;
	partner_manager::partner_ontick_settimer(this);
}

int partner_struct::broadcast_partner_move()
{
	if (!area && !partner_sight_space)
		return (0);
	
	MoveNotify notify;
	move_notify__init(&notify);
	notify.playerid = data->uuid;
	notify.data = pack_unit_move_path(&notify.n_data);

	broadcast_to_sight(MSG_ID_MOVE_NOTIFY, &notify, (pack_func)move_notify__pack, false);
	reset_pos_pool();
	return (0);
}

player_struct *partner_struct::get_owner()
{
	return m_owner;
}


void partner_struct::on_tick()
{
	if (!m_owner->scene)
		return;
	
	update_partner_pos_and_sight();

	if (buff_state & BUFF_STATE_TAUNT)
	{
		return do_taunt_action();
	}

	if (buff_state & BUFF_STATE_STUN)
	{
		return;
	}

	if (is_unit_in_move())
		return;

	if (is_player_in_sight(data->owner_id) &&  ai)
	{
		if (ai->on_tick(this) != 0)
			return;
	}
	

		// TODO: 间隔超过4米开始移动, 超过10米闪现
	struct position *owner_pos = m_owner->get_pos();
	struct position *cur_pos = get_pos();

	float x = owner_pos->pos_x - cur_pos->pos_x;
	float z = owner_pos->pos_z - cur_pos->pos_z;
	float d = x * x + z * z;
	
	if (!partner_sight_space && (d >= 24 * 24 || scene != m_owner->scene))
	{
			// 超过10米闪现
		struct position target_pos;
		calc_target_pos(&target_pos);
		if (scene)
			scene->delete_partner_from_scene(this, true);
		set_pos(target_pos.pos_x, target_pos.pos_z);
		m_owner->scene->add_partner_to_scene(this);
//		set_pos_with_broadcast(target_pos.pos_x, target_pos.pos_z);
	}
	else if (d >= 3 * 3)
	{
			// TODO: 间隔超过4米开始移动
		reset_pos();
		
		struct position target_pos;
		calc_target_pos(&target_pos);

		struct position last;
		int ret = get_last_can_walk(scene, &data->move_path.pos[0], &target_pos, &last);
		if (ret == 0)
		{
			data->move_path.pos[1].pos_x = last.pos_x;
			data->move_path.pos[1].pos_z = last.pos_z;					
		}
		else
		{
			data->move_path.pos[1].pos_x = target_pos.pos_x;
			data->move_path.pos[1].pos_z = target_pos.pos_z;		
		}
		
		data->move_path.start_time = time_helper::get_cached_time();
		data->move_path.max_pos = 1;
		broadcast_partner_move();
	}
}

void partner_struct::pack_sight_partner_info(SightPartnerInfo *info)
{
	sight_partner_info__init(info);
 	info->hp = data->attrData[PLAYER_ATTR_HP];
 	info->uuid = data->uuid;
 	info->partnerid = data->partner_id;
	info->owner = data->owner_id;
 	info->lv = data->attrData[PLAYER_ATTR_LEVEL];
 	info->pk_type = data->attrData[PLAYER_ATTR_PK_TYPE];
 	info->zhenying_id = data->attrData[PLAYER_ATTR_ZHENYING];	
 	info->data = pack_unit_move_path(&info->n_data);
 	info->speed = data->attrData[PLAYER_ATTR_MOVE_SPEED];
 	info->buff_info = pack_unit_buff(&info->n_buff_info);
	info->maxhp = data->attrData[PLAYER_ATTR_MAXHP];
}

void partner_struct::broadcast_to_sight_and_owner(uint16_t msg_id, void *msg_data, pack_func func, bool include_owner)
{
	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(msg_id, msg_data, func);
	PROTO_HEAD_CONN_BROADCAST *head;
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
	int player_num = *get_cur_sight_player();
	uint64_t *t_player_id = get_all_sight_player();
	bool has_owner = false;
	for (int i = 0; i < player_num; ++i)
	{
		conn_node_gamesrv::broadcast_msg_add_players(t_player_id[i], ppp);							
		if (t_player_id[i] == data->owner_id)
		{
			has_owner = true;
		}
	}
	
	if (include_owner && !has_owner)
	{
		conn_node_gamesrv::broadcast_msg_add_players(data->owner_id, ppp);									
	}

	if (head->num_player_id > 0)
	{
		conn_node_gamesrv::broadcast_msg_send();
	}
}

int partner_struct::init_partner(uint32_t partner_id, player_struct *owner)
{
	assert(data);
	init_unit_struct();
	
	data->partner_id = partner_id;
	data->owner_id = owner->get_uuid();
	m_owner = owner;
	config = get_config_by_id(partner_id, &partner_config);
	if (!config)
	{
		LOG_ERR("[%s:%u] can't find config, partner_id:%u", __FUNCTION__, __LINE__, partner_id);
		return -1;
	}
	
	scene = NULL;
	area = NULL;
//	sight_space = NULL;
	buff_state = 0;
	lock_time = 0;
	m_target = NULL;
	memset(&m_buffs[0], 0, sizeof(m_buffs));
	partner_sight_space = NULL;

	ai_type = config->Character;
	//ai_state = AI_PATROL_STATE;
	set_ai_interface(ai_type);
	for (int i = 0; i < MAX_PARTNER_ATTACK_UNIT; ++i)
	{
		attack_owner[i].uuid = 0;
		attack_partner[i].uuid = 0;
		owner_attack[i].uuid = 0;
	}
	

	return 0;
}

int partner_struct::init_create_data(void)
{
	data->attrData[PLAYER_ATTR_LEVEL] = 1;

	relesh_attr();
	memcpy(&(data->attr_cur), &(data->attr_flash), sizeof(partner_attr_data));
	memset(&(data->attr_flash), 0, sizeof(partner_attr_data));

	calculate_attribute(false);
	data->attrData[PLAYER_ATTR_HP] = data->attrData[PLAYER_ATTR_MAXHP];
	
	return 0;
}

int partner_struct::init_end(bool isNty)
{
	calculate_attribute(isNty);

	return 0;
}

// void partner_struct::clear(void)
// {
// 	clear_all_buffs();
// }

void partner_struct::send_patrol_move()
{
	data->move_path.start_time = time_helper::get_cached_time();
	data->move_path.max_pos = 1;
	data->move_path.cur_pos = 0;
	broadcast_partner_move();
}

void partner_struct::cast_skill_to_target(uint64_t skill_id, unit_struct *target)
{
	SkillCastNotify notify;
	skill_cast_notify__init(&notify);
	notify.skillid = skill_id;
	notify.playerid = data->uuid;
	PosData cur_pos;
	pos_data__init(&cur_pos);
	struct position *pos = get_pos();
	cur_pos.pos_x = pos->pos_x;
	cur_pos.pos_z = pos->pos_z;		
	notify.cur_pos = &cur_pos;

	if (target)
	{
		struct position *player_pos = target->get_pos();
		notify.direct_x = player_pos->pos_x - pos->pos_x;
		notify.direct_z = player_pos->pos_z - pos->pos_z;
	}
	broadcast_to_sight(MSG_ID_SKILL_CAST_NOTIFY, &notify, (pack_func)skill_cast_notify__pack, true);
}

// void partner_struct::cast_immediate_skill_to_target(uint64_t skill_id, int skill_index, unit_struct *target)
// {
// 	if (get_unit_fight_type(this, target) != UNIT_FIGHT_TYPE_ENEMY)			
// 		return;

// //	if (player->buff_state & BUFF_STATE_GOD)
// //	{
// //		return;
// //	}
	
// 	int n_hit_effect = 0;
// 	int n_buff = 0;
// 	assert(target && target->is_avaliable());
// 	cached_hit_effect_point[n_hit_effect] = &cached_hit_effect[n_hit_effect];
// 	skill_hit_effect__init(&cached_hit_effect[n_hit_effect]);
// 	uint32_t add_num = 0;
// 	int32_t damage = 0;

// 	struct SkillLvTable *lv_config1, *lv_config2;
// 	struct PassiveSkillTable *pas_config;
// 	struct SkillTable *ski_config;
	
// 	get_skill_configs(get_skill_level_byindex(skill_index), skill_id, &ski_config, &lv_config1, &pas_config, &lv_config2, NULL);
// 	if (!lv_config1 && !lv_config2)
// 	{
// 		LOG_ERR("%s %d: skill[%lu] no config", __FUNCTION__, __LINE__, skill_id);
// 		return;
// 	}
	
// 	int32_t other_rate = count_other_skill_damage_effect(this, target);					
// 	damage += count_skill_total_damage(UNIT_FIGHT_TYPE_ENEMY, ski_config, lv_config1,
// 		pas_config, lv_config2,
// 		this, target,
// 		&cached_hit_effect[n_hit_effect].effect,
// 		&cached_buff_id[n_buff],
// 		&cached_buff_end_time[n_buff],
// 		&add_num, other_rate);

// 	target->on_hp_changed(damage);

// 	// if (target->get_unit_type() == UNIT_TYPE_PLAYER)
// 	// {
// 	// 	player_struct *t = ((player_struct *)target);
// 	// 	raid_struct *raid = t->get_raid();
// 	// 	if (raid)
// 	// 		raid->on_monster_attack(monster, t, damage);
// 	// }

// 	LOG_DEBUG("%s: unit[%lu][%p] damage[%d] hp[%f]", __PRETTY_FUNCTION__, get_uuid(), this, damage, target->get_attr(PLAYER_ATTR_HP));

// 	uint32_t life_steal = count_life_steal_effect(damage);
// 	uint32_t damage_return = count_damage_return(damage, target);

// 	on_hp_changed(damage_return);

// 	PosData attack_pos;
// 	pos_data__init(&attack_pos);
// 	attack_pos.pos_x = get_pos()->pos_x;
// 	attack_pos.pos_z = get_pos()->pos_z;
	
// 	cached_hit_effect[n_hit_effect].playerid = target->get_uuid();
// 	cached_hit_effect[n_hit_effect].n_add_buff = add_num;
// 	cached_hit_effect[n_hit_effect].add_buff = &cached_buff_id[n_buff];
// //	cached_hit_effect[n_hit_effect].add_buff_end_time = &cached_buff_end_time[n_buff];
// 	cached_hit_effect[n_hit_effect].hp_delta = damage;
// 	cached_hit_effect[n_hit_effect].cur_hp = target->get_attr(PLAYER_ATTR_HP);
// //	cached_hit_effect.attack_pos = &attack_pos;
// 	cached_hit_effect[n_hit_effect].target_pos = &cached_target_pos[n_hit_effect];	
// //	pos_data__init(&cached_attack_pos[n_hit_effect]);
// 	pos_data__init(&cached_target_pos[n_hit_effect]);	
// //	cached_attack_pos[n_hit_effect].pos_x = monster->get_pos()->pos_x;
// //	cached_attack_pos[n_hit_effect].pos_z = monster->get_pos()->pos_z;
// 	cached_target_pos[n_hit_effect].pos_x = target->get_pos()->pos_x;
// 	cached_target_pos[n_hit_effect].pos_z = target->get_pos()->pos_z;			
	
// 	n_buff += add_num;
// 	++n_hit_effect;

// 	SkillHitImmediateNotify notify;
// 	skill_hit_immediate_notify__init(&notify);
// 	notify.playerid = data->uuid;
// 	notify.owneriid = data->uuid;
	
// 	notify.skillid = skill_id;
// 	notify.target_player = cached_hit_effect_point[0];
// 	notify.attack_pos = &attack_pos;
// 	notify.attack_cur_hp = get_attr(PLAYER_ATTR_HP);
// 	notify.life_steal = life_steal;
// 	notify.damage_return = damage_return;
// 	target->broadcast_to_sight(MSG_ID_SKILL_HIT_IMMEDIATE_NOTIFY, &notify, (pack_func)skill_hit_immediate_notify__pack, true);

// 	if (!target->is_alive())
// 	{
// 		target->on_dead(this);
// 	}
// 	else
// 	{
// 		target->on_beattack(this, skill_id, damage);
// 	}
// 	if (!is_alive())
// 	{
// 		on_dead(target);
// 	}	
// }

	//返回0表示没有继续执行跟随主人的AI，返回1表示不继续执行跟随主人
uint32_t partner_struct::attack_target(uint32_t skill_id, int skill_index, unit_struct *target)
{
	struct position *my_pos = get_pos();
	struct position *his_pos = target->get_pos();
	struct SkillTable *config = get_config_by_id(skill_id, &skill_config);
	if (config == NULL)
	{
		LOG_ERR("%s: partner can not find skill[%u] config", __FUNCTION__, skill_id);
		return 1;
	}
	if (!check_distance_in_range(my_pos, his_pos, config->SkillRange))
	{
			//追击
		reset_pos();
		if (get_circle_random_position_v2(scene, my_pos, his_pos, config->SkillRange, &data->move_path.pos[1]))		
		{
			send_patrol_move();
		}
		else
		{
			return (0);
		}
		data->ontick_time += random() % 1000;
		return 1;
	}

	uint64_t now = time_helper::get_cached_time();
		//主动技能
	if (config->SkillType == 2)
	{
		struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
		if (!act_config)
		{
			LOG_ERR("%s: can not find skillaffectid[%lu] config", __FUNCTION__, config->SkillAffectId);
			return 1;
		}
		if (act_config->ActionTime > 0)
		{
			add_skill_cd(skill_index, now);
			data->ontick_time = now + act_config->ActionTime;// + 1500;
			data->skill_id = skill_id;
			data->angle = -(pos_to_angle(his_pos->pos_x - my_pos->pos_x, his_pos->pos_z - my_pos->pos_z));
//			LOG_DEBUG("jacktang: mypos[%.2f][%.2f] hispos[%.2f][%.2f]", my_pos->pos_x, my_pos->pos_z, his_pos->pos_x, his_pos->pos_z);
			ai_state = AI_ATTACK_STATE;
			m_target = target;

			reset_pos();
			cast_skill_to_target(skill_id, target);		
			return 1;
		}
	}

	reset_pos();
	add_skill_cd(skill_index, now);
	uint32_t skill_lv = get_skill_level_byindex(skill_index);
	cast_immediate_skill_to_target(skill_id, skill_lv, this, target);

		//被反弹死了
	if (!data)
		return 1;
	
	data->skill_id = 0;

		//计算硬直时间
	data->ontick_time += count_skill_delay_time(config);
	
	return 1;
}

bool partner_struct::try_friend_skill(uint32_t skill_id, int skill_index)
{
	if (!m_owner || !m_owner->data)
		return false;
	
	struct SkillTable *config = get_config_by_id(skill_id, &skill_config);
	if (config == NULL)
		return false;
	if (config->TargetType[0] == 101 || config->TargetType[0] == 102)
	{
		unit_struct *target = NULL;
		if (!is_fullhp())
		{
			target = this;
		}
		else if (m_owner->is_fullhp())
		{
			target = m_owner;
		}
		if (!target)
			return false;

		struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
		if (!act_config)
		{
			LOG_ERR("%s: can not find skillaffectid[%lu] config", __FUNCTION__, config->SkillAffectId);
			return 1;
		}
		if (act_config->ActionTime > 0)
		{
			uint64_t now = time_helper::get_cached_time();			
			add_skill_cd(skill_index, now);
			data->ontick_time = now + act_config->ActionTime;// + 1500;
			data->skill_id = skill_id;
//			data->angle = -(pos_to_angle(his_pos->pos_x - my_pos->pos_x, his_pos->pos_z - my_pos->pos_z));
//			LOG_DEBUG("jacktang: mypos[%.2f][%.2f] hispos[%.2f][%.2f]", my_pos->pos_x, my_pos->pos_z, his_pos->pos_x, his_pos->pos_z);
			ai_state = AI_ATTACK_STATE;

			reset_pos();
			cast_skill_to_target(skill_id, target);		
			return 1;
		}
	
		return true;
	}
	return false;
}

uint32_t partner_struct::choose_skill(int *index)
{
	uint64_t now = time_helper::get_cached_time();
	
	for (int i = MAX_PARTNER_SKILL_NUM - 1; i >= 0; --i)
	{
		if (is_skill_in_cd(i, now))
			continue;
//		add_skill_cd(i, now);
		if (index)
			*index = i;
		return data->attr_cur.skill_list[i].skill_id;
	}
	if (index)
		*index = -1;
	return config->BaseSkill1;
//	return (0);
}

bool partner_struct::is_skill_in_cd(uint32_t index, uint64_t now)
{
	if (index >= MAX_PARTNER_SKILL_NUM)
		return true;
	if (data->attr_cur.skill_list[index].skill_id == 0)
		return true;

	// if (data->attr_cur.skill_list[index].skill_id == config->ProtectSkill)
	// 	return true;

	// if (data->attr_cur.skill_list[index].skill_id == config->Angerskill)
	// 	return true;	
	
	return now < data->attr_cur.skill_list[index].cd;
}

int partner_struct::add_skill_cd(int index, uint64_t now)
{
	if (index < 0)
		return (0);
	if (index >= MAX_PARTNER_SKILL_NUM)
		return (0);

	uint32_t skill_id = data->attr_cur.skill_list[index].skill_id;
	SkillTable *config = get_config_by_id(skill_id, &skill_config);
	if (!config)
		return (0);
	
	struct SkillLvTable *lv_config = get_config_by_id(config->SkillLv, &skill_lv_config);
	if (!lv_config)
		return (0);
	data->attr_cur.skill_list[index].cd = now + lv_config->CD;
	LOG_INFO("%s: %p add partner skill[%u] index[%u] cd[%lu]", __FUNCTION__, this, skill_id, index, data->attr_cur.skill_list[index].cd);
	return (0); 
}

extern struct partner_ai_interface *all_partner_ai_interface[MAX_PARTNER_AI_INTERFACE];
void partner_struct::set_ai_interface(int ai_type)
{
	if (ai_type >= 0 && ai_type < MAX_PARTNER_AI_INTERFACE)
		ai = all_partner_ai_interface[ai_type];
	else
		ai = NULL;
}

void partner_struct::add_ai_interface(int ai_type, struct partner_ai_interface *ai)
{
	assert(ai_type >= 0 && ai_type < MAX_PARTNER_AI_INTERFACE);
	if (ai)
	{	
		assert(ai->on_tick);
		assert(ai->choose_target);
	}
	all_partner_ai_interface[ai_type] = ai;
}

void partner_struct::calc_target_pos(struct position *pos)
{
	static float delta[] = {0.75, 0.7, 0.65, 0.6, 0.55, 0.5};

	assert(m_owner);
	struct position *cur_pos = m_owner->get_pos();
	
	if (!m_owner->scene)
	{
		pos->pos_x = cur_pos->pos_x;
		pos->pos_z = cur_pos->pos_z;
		return;
	}
		
	for (size_t i = 0; i < ARRAY_SIZE(delta); ++i)
	{
		double angle = m_owner->data->m_angle - (M_PI * delta[i]);
		double cos = qFastCos(angle);
		double sin = qFastSin(angle);
		float z = 2 * sin + cur_pos->pos_z;
		float x = 2 * cos + cur_pos->pos_x;		

		struct map_block *block_start = get_map_block(m_owner->scene->map_config, x, z);
		if (!block_start || !block_start->can_walk)
			continue;
		pos->pos_x = x;
		pos->pos_z = z;
		return;
	}
	pos->pos_x = cur_pos->pos_x;
	pos->pos_z = cur_pos->pos_z;
	return;
}

void partner_struct::calculate_attribute(double *attrData, partner_attr_data &attr_cur)
{
//	memset(&attrData[PLAYER_ATTR_MAXHP], 0, sizeof(double));
//	memset(&attrData[PLAYER_ATTR_ATTACK], 0, (PLAYER_ATTR_DETIMEDF - PLAYER_ATTR_ATTACK + 1) * sizeof(double));
	memset(&data->attrData[PLAYER_ATTR_MAXHP], 0, (PLAYER_ATTR_PVPDF - PLAYER_ATTR_MAXHP + 1) * sizeof(double));
	
	double module_attr[PLAYER_ATTR_MAX];
	memset(module_attr, 0, sizeof(double) * PLAYER_ATTR_MAX);

	for (int i = 0; i < MAX_PARTNER_BASE_ATTR; ++i)
	{
		uint32_t attr_id = attr_cur.base_attr_id[i];
		double attr_val = attr_cur.base_attr_vaual[i];
		if (attr_id > 0 && attr_id < PLAYER_ATTR_FIGHT_MAX && attr_val > 0.0)
		{
			module_attr[attr_id] += attr_val;
		}
	}
	for (uint32_t i = 0; i < attr_cur.n_detail_attr && i < MAX_PARTNER_DETAIL_ATTR; ++i)
	{
		uint32_t attr_id = attr_cur.detail_attr_id[i];
		double attr_val = attr_cur.detail_attr_vaual[i];
		if (attr_id > 0 && attr_id < PLAYER_ATTR_FIGHT_MAX && attr_val > 0.0)
		{
			module_attr[attr_id] += attr_val;
		}
	}
	for (uint32_t i = 0; i < data->n_god; ++i)
	{
		for (uint32_t n_attr = 0; n_attr < config->n_GodYao; ++n_attr)
		{
			if (config->GodYao[n_attr] == data->god_id[i])
			{
				GodYaoAttributeTable *table = get_config_by_id(config->GodYaoAttribute[n_attr], &partner_god_attr_config);
				if (table != NULL)
				{
					module_attr[table->AttributeType] += (table->AttributeNum * (1 + data->god_level[i] * table->Coefficient));
				}
				break;
			}
		}
	}
	
	//法宝加成属性
	if(data->cur_fabao.fabao_id !=0 )
	{
		uint32_t attr_id = data->cur_fabao.main_attr.id;
		double attr_val = data->cur_fabao.main_attr.val;
		if(attr_id > 0 && attr_id < MAX_PARTNER_ATTR && attr_val > 0.0)
		{
			module_attr[attr_id] += attr_val;
		}
		for(uint32_t i = 0; i < MAX_HUOBAN_FABAO_MINOR_ATTR_NUM; ++i)
		{	
			uint32_t attr_id = data->cur_fabao.minor_attr[i].id;
			double attr_val = data->cur_fabao.minor_attr[i].val;
			if(attr_id > 0 && attr_id < MAX_PARTNER_ATTR && attr_val > 0.0)
			{
				module_attr[attr_id] += attr_val;
			}
		}
	}
	add_fight_attr(attrData, module_attr);
}

void partner_struct::calculate_attribute(bool isNty)
{
	calculate_attribute(data->attrData, data->attr_cur);

	uint32_t prev_fp = get_attr(PLAYER_ATTR_FIGHTING_CAPACITY);
	uint32_t cur_fp = calculate_fighting_capacity(data->attrData, true);
	bool NtyFp = false;
	if (cur_fp != prev_fp)
	{
		NtyFp = true;
		data->attrData[PLAYER_ATTR_FIGHTING_CAPACITY] = cur_fp;
		if (m_owner->is_partner_battle() && (scene || m_owner->partner_is_in_formation(data->uuid)))
		{ //阵型上的伙伴战斗力变化会影响角色战斗力，正在战斗的伙伴战斗力变化还会影响护主技能等级，从而影响属性
			m_owner->calculate_attribute(isNty);
		}
		LOG_DEBUG("[%s:%d] player[%lu] partner[%lu] fighting capacity, %u --> %u", __FUNCTION__, __LINE__, data->owner_id, data->uuid, prev_fp, cur_fp);
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
}

void partner_struct::notify_attr(AttrMap& attr_list, bool broadcast, bool include_owner)
{
	PlayerAttrNotify nty;
	player_attr_notify__init(&nty);
	AttrData attr_data[PLAYER_ATTR_MAX + 2];
	AttrData *attr_data_point[PLAYER_ATTR_MAX + 2];

	nty.player_id = data->uuid;
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
		this->broadcast_to_sight_and_owner(MSG_ID_PLAYER_ATTR_NOTIFY, &nty, (pack_func)player_attr_notify__pack, include_owner);
	}
	else
	{
		EXTERN_DATA extern_data;
		extern_data.player_id = data->owner_id;
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_PLAYER_ATTR_NOTIFY, player_attr_notify__pack, nty);
	}
}

void partner_struct::notify_one_attr_changed(uint32_t attr_id, double attr_val)
{
	PlayerAttrNotify nty;
	player_attr_notify__init(&nty);
	AttrData attr_data[1];
	AttrData *attr_data_point[1];

	nty.player_id = data->uuid;
	nty.n_attrs = 1;
	nty.attrs = attr_data_point;

	attr_data_point[0] = &attr_data[0];
	attr_data__init(&attr_data[0]);
	attr_data[0].id = attr_id;
	attr_data[0].val = attr_val;

	EXTERN_DATA extern_data;
	extern_data.player_id = data->owner_id;
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_PLAYER_ATTR_NOTIFY, player_attr_notify__pack, nty);
}

int partner_struct::add_exp(uint32_t val, uint32_t statis_id, uint32_t owner_lv, bool isNty)
{
	if (val == 0)
	{
		return 0;
	}

	AttrMap attrs;
	uint32_t exp_old = data->attrData[PLAYER_ATTR_EXP];
	uint32_t exp_new = data->attrData[PLAYER_ATTR_EXP] + val;
	uint32_t level_old = data->attrData[PLAYER_ATTR_LEVEL];
	uint32_t level_new = level_old;

	while (true)
	{
		//partner level can't higher than its owner level
		if (level_new >= owner_lv)
		{
			exp_new = 0;
			break;
		}

		PartnerLevelTable* level_config = get_partner_level_config(level_new);
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

	if (exp_new != exp_old)
	{
		data->attrData[PLAYER_ATTR_EXP] = exp_new;
		attrs[PLAYER_ATTR_EXP] = exp_new;
	}

	if (level_new != level_old)
	{
		data->attrData[PLAYER_ATTR_LEVEL] = level_new;

		this->deal_level_up(level_old, level_new);
	}

	if (isNty && attrs.size() > 0)
	{
		this->notify_attr(attrs);
	}

	return 0;
}

int partner_struct::deal_level_up(uint32_t level_old, uint32_t level_new)
{
	uint32_t addBaseAttr[MAX_PARTNER_BASE_ATTR];
	for (uint32_t i = 0; i < config->n_AttributeType && i < MAX_PARTNER_BASE_ATTR; ++i)
	{
		addBaseAttr[i] = (level_new - level_old) * config->GrowthValue[i];
		data->attr_cur.base_attr_up[i] += addBaseAttr[i];
		data->attr_cur.base_attr_vaual[i] += addBaseAttr[i];
	}
	if (data->attr_flash.base_attr_id[0] != 0)
	{
		for (uint32_t i = 0; i < config->n_AttributeType && i < MAX_PARTNER_BASE_ATTR; ++i)
		{
			data->attr_flash.base_attr_up[i] += addBaseAttr[i];
			data->attr_flash.base_attr_vaual[i] += addBaseAttr[i];
		}
		double attrData[MAX_PARTNER_ATTR]; //战斗属性 需要战斗力属性
		calculate_attribute(attrData, data->attr_flash);
		data->attr_flash.power_refresh = calculate_fighting_capacity(attrData);
	}
	if (addBaseAttr[0] > 0)
	{
		PartnerAddBaseAttr send;
		partner_add_base_attr__init(&send);
		send.uuid = get_uuid();
		send.attr = addBaseAttr;
		send.n_attr = MAX_PARTNER_BASE_ATTR;
		send.power = data->attr_flash.power_refresh;
		EXTERN_DATA extern_data;
		extern_data.player_id = data->owner_id;
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_PARTNER_ADD_BASE_NOTIFY, partner_add_base_attr__pack, send);
	}

	this->calculate_attribute(true);

	//当前血量回满
	data->attrData[PLAYER_ATTR_HP] = data->attrData[PLAYER_ATTR_MAXHP];
	on_hp_changed(0);		
	
	AttrMap attrs;
	attrs[PLAYER_ATTR_HP] = data->attrData[PLAYER_ATTR_HP];
	attrs[PLAYER_ATTR_LEVEL] = level_new;
	attrs[PLAYER_ATTR_FIGHTING_CAPACITY] = get_attr(PLAYER_ATTR_FIGHTING_CAPACITY);
	this->notify_attr(attrs, true);

	return 0;
}

int partner_struct::get_total_exp(void)
{
	return 0;
}

uint32_t partner_struct::get_level()
{
	return (data ? data->attrData[PLAYER_ATTR_LEVEL] : 0);
}

void partner_struct::mark_bind(void)
{
	data->bind = 1;
}

int partner_struct::prepare_add_player_to_sight(player_struct *player)
{
	if (data->cur_sight_player < MAX_PLAYER_IN_PARTNER_SIGHT)
		return (0);

//todo 检查关系的优先级，然后删除低优先级的来腾出空间
	return (-1);
}
int partner_struct::prepare_add_truck_to_sight(cash_truck_struct * truck)
{
		//友好，中立关系的不用加
	if (get_unit_fight_type(this, truck) != UNIT_FIGHT_TYPE_ENEMY)
		return (-1);
	
	if (data->cur_sight_truck < MAX_TRUCK_IN_PARTNER_SIGHT)
		return (0);

//todo 检查关系的优先级，然后删除低优先级的来腾出空间
	return (-1);
}

int partner_struct::prepare_add_partner_to_sight(partner_struct *partner)
{
	// 	//友好，中立关系的不用加
	// if (get_unit_fight_type(this, partner) != UNIT_FIGHT_TYPE_ENEMY)
	// 	return (-1);
	
		//死了的不进入视野
//	if (partner->data->attrData[PLAYER_ATTR_HP] <= 0)
//		return (-1);
	
	if (data->cur_sight_partner < MAX_PARTNER_IN_PARTNER_SIGHT)
		return (0);

//todo 检查关系的优先级，然后删除低优先级的来腾出空间
	return (-1);
}

int partner_struct::prepare_add_monster_to_sight(monster_struct *monster)
{
	// 	//友好，中立关系的不用加
	// if (get_unit_fight_type(this, monster) != UNIT_FIGHT_TYPE_ENEMY)
	// 	return (-1);
	
		//死了的不进入视野
//	if (monster->data->attrData[PLAYER_ATTR_HP] <= 0)
//		return (-1);
	
	if (data->cur_sight_monster < MAX_MONSTER_IN_PARTNER_SIGHT)
		return (0);

//todo 检查关系的优先级，然后删除低优先级的来腾出空间
	return (-1);
}

int partner_struct::add_partner_to_sight_both(partner_struct *partner)
{
			//友好，中立关系的不用加
	if (get_unit_fight_type(this, partner) != UNIT_FIGHT_TYPE_ENEMY
		&& get_unit_fight_type(partner, this) != UNIT_FIGHT_TYPE_ENEMY)
		return (-1);
	
	if (prepare_add_partner_to_sight(partner) != 0 ||
		partner->prepare_add_partner_to_sight(this) != 0)
		return -1;
	
	int ret = add_partner_to_sight(partner->data->uuid);
	assert (ret >= 0);
	int ret1 = partner->add_partner_to_sight(data->uuid);
	assert(ret1 >= 0);		
	return ret;
}

int partner_struct::del_partner_from_sight_both(partner_struct *partner)
{
	int ret = del_partner_from_sight(partner->data->uuid);
	if (ret >= 0)
	{
		int ret1 = partner->del_partner_from_sight(data->uuid);
		assert(ret1 >= 0);
	}
	return ret;
}


int partner_struct::broadcast_partner_delete(bool send_msg)
{
	assert(data);

	for (int i = 0; i < data->cur_sight_player; ++i)
	{
		player_struct *player = player_manager::get_player_by_id(data->sight_player[i]);
		if (player)
			player->del_partner_from_sight(data->uuid);
	}
	for (int i = 0; i < data->cur_sight_monster; ++i)
	{
		monster_struct *monster = monster_manager::get_monster_by_id(data->sight_monster[i]);
		if (monster)
		{
			LOG_DEBUG("%s: %lu[%u] del monster %lu[%u]", __FUNCTION__, get_uuid(), data->cur_sight_monster,
				monster->get_uuid(), monster->data->cur_sight_monster);		
			monster->del_partner_from_sight(data->uuid);
		}
		else
		{
			LOG_ERR("%s: %lu can not find sight monster %lu", __FUNCTION__, get_uuid(), data->sight_monster[i]);
		}
	}
	for (int i = 0; i < data->cur_sight_truck; ++i)
	{
		cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(data->sight_truck[i]);
		if (truck)
		{
			truck->del_partner_from_sight(data->uuid);
		}
		else
		{
			LOG_ERR("%s: %lu can not find sight truck %lu", __FUNCTION__, get_uuid(), data->sight_truck[i]);
		}
	}
	for (int i = 0; i < data->cur_sight_partner; ++i)
	{
		partner_struct *partner = partner_manager::get_partner_by_uuid(data->sight_partner[i]);
		if (partner)
		{
			partner->del_partner_from_sight(data->uuid);
		}
		else
		{
			LOG_ERR("%s: %lu can not find sight partner %lu", __FUNCTION__, get_uuid(), data->sight_partner[i]);
		}
	}

	SightChangedNotify notify;
	uint64_t del_uuid[1];
	uint64_t *ppp;
	
	if (!send_msg)
		goto done;
	
	sight_changed_notify__init(&notify);
	del_uuid[0] = data->uuid;
	notify.delete_partner = del_uuid;
		//发送给需要在视野里面添加玩家的通知
	notify.n_delete_partner = 1;
	ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
	for (int i = 0; i < data->cur_sight_player; ++i)
		conn_node_gamesrv::broadcast_msg_add_players(data->sight_player[i], ppp);
	conn_node_gamesrv::broadcast_msg_send();

done:
	clear_partner_sight();
	return (0);
}

void partner_struct::clear_partner_sight()
{
	LOG_DEBUG("%s: %lu", __FUNCTION__, get_uuid());
	data->cur_sight_player = 0;
	data->cur_sight_monster = 0;
	data->cur_sight_truck = 0;
	data->cur_sight_partner = 0;		
}

int partner_struct::broadcast_partner_create()
{
	if (!area)
		return (0);

	LOG_DEBUG("%s %d: create partner %u %lu at area %ld[%.1f][%.1f]", __PRETTY_FUNCTION__, __LINE__, data->partner_id, get_uuid(),
		area - scene->m_area, get_pos()->pos_x, get_pos()->pos_z);
	
	SightChangedNotify notify;
	sight_changed_notify__init(&notify);
	SightPartnerInfo partner_info[1];
	SightPartnerInfo *partner_info_point[1];
	partner_info_point[0] = &partner_info[0];
	notify.n_add_partner = 1;
	notify.add_partner = partner_info_point;
	pack_sight_partner_info(partner_info_point[0]);

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

	add_area_truck_to_sight(area);
	for (int i = 0; i < MAX_NEIGHBOUR_AREA; ++i)
	{
		add_area_truck_to_sight(area->neighbour[i]);
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

void partner_struct::del_sight_player_in_area(int n_del, area_struct **del_area, int *delete_player_id_index, uint64_t *delete_player_id)
{
	int i, j;
	int index = 0;
	player_struct *del_sight_player[MAX_PLAYER_IN_PARTNER_SIGHT];	
	
	for (i = 0; i < data->cur_sight_player; ++i)
	{
		player_struct *player = player_manager::get_player_by_id(data->sight_player[i]);
		if (!player)
		{
				//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: %lu can not find sight player %lu", __FUNCTION__, __LINE__, data->uuid, data->sight_player[i]);
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
		del_sight_player[i]->del_partner_from_sight_both(this);
}

void partner_struct::del_sight_truck_in_area(int n_del, area_struct **del_area)
{
	int i, j;
	int index = 0;
	cash_truck_struct *del_sight_truck[MAX_TRUCK_IN_PARTNER_SIGHT];
	
	for (i = 0; i < data->cur_sight_truck; ++i)
	{
		cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(data->sight_truck[i]);
		if (!truck)
		{
				//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: %lu can not find sight truck %lu", __FUNCTION__, __LINE__, data->uuid, data->sight_truck[i]);
			continue;
		}
		for (j = 0; j < n_del; ++j)
		{
			if (truck->area != del_area[j])
				continue;
			del_sight_truck[index++] = truck;
		}
	}

	for (i = 0; i < index; ++i)
		del_sight_truck[i]->del_partner_from_sight_both(this);
}

void partner_struct::del_sight_monster_in_area(int n_del, area_struct **del_area)
{
	int i, j;
	int index = 0;
	monster_struct *del_sight_monster[MAX_MONSTER_IN_PARTNER_SIGHT];
	
	for (i = 0; i < data->cur_sight_monster; ++i)
	{
		monster_struct *monster = monster_manager::get_monster_by_id(data->sight_monster[i]);
		if (!monster)
		{
				//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: %lu can not find sight monster %lu", __FUNCTION__, __LINE__, data->uuid, data->sight_monster[i]);
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
		del_sight_monster[i]->del_partner_from_sight_both(this);
}
void partner_struct::del_sight_partner_in_area(int n_del, area_struct **del_area)
{
	int i, j;
	int index = 0;
	partner_struct *del_sight_partner[MAX_PARTNER_IN_PARTNER_SIGHT];
	
	for (i = 0; i < data->cur_sight_partner; ++i)
	{
		partner_struct *partner = partner_manager::get_partner_by_uuid(data->sight_partner[i]);
		if (!partner)
		{
				//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: %lu can not find sight partner %lu", __FUNCTION__, __LINE__, data->uuid, data->sight_partner[i]);
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
void partner_struct::add_area_player_to_sight(area_struct *area, uint16_t *add_player_id_index, uint64_t *add_player)
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
		if (player->add_partner_to_sight_both(this) >= 0)
		{
			if (get_entity_type(area->m_player_ids[j]) == ENTITY_TYPE_PLAYER)
			{
				add_player[*add_player_id_index] = player->data->player_id;
				(*add_player_id_index)++;
			}
		}
	}
}

void partner_struct::add_area_truck_to_sight(area_struct *area)
{
	if (!area)
		return;
	for (int j = 0; j < area->cur_truck_num; ++j)
	{
		cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(area->m_truck_uuid[j]);
		if (!truck)
		{
				//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: can not find sight truck %lu area[%p]", __FUNCTION__, __LINE__, area->m_truck_uuid[j], area);				
			continue;
		}
		truck->add_partner_to_sight_both(this);
	}
}

void partner_struct::add_area_monster_to_sight(area_struct *area)
{
	if (!area)
		return;
	for (int j = 0; j < area->cur_monster_num; ++j)
	{
		monster_struct *monster = monster_manager::get_monster_by_id(area->m_monster_uuid[j]);
		if (!monster)
		{
				//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: can not find sight player %lu area[%p]", __FUNCTION__, __LINE__, area->m_monster_uuid[j], area);				
			continue;
		}
		monster->add_partner_to_sight_both(this);
	}
}
void partner_struct::add_area_partner_to_sight(area_struct *area)
{
	if (!area)
		return;
	for (int j = 0; j < area->cur_partner_num; ++j)
	{
		if (area->m_partner_uuid[j] == data->uuid)
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

static void add_partner_hate_unit(uint64_t uuid, partner_attack_unit *unit)
{
	uint64_t now = time_helper::get_cached_time();
	for (int i = 0; i < MAX_PARTNER_ATTACK_UNIT; ++i)
	{
		if (unit[i].uuid == 0)
		{
			unit[i].uuid = uuid;
			unit[i].time = now;
			return;			
		}
		if (uuid == unit[i].uuid)
		{
			unit[i].time = now;
			return;
		}
		if (unit[i].time + 60 * 1000 >= now)
		{
			unit[i].uuid = uuid;
			unit[i].time = now;
			return;			
		}
	}
}

void partner_struct::on_owner_attack(uint64_t uuid)
{
	add_partner_hate_unit(uuid, &owner_attack[0]);
}
void partner_struct::on_owner_beattack(uint64_t uuid)
{
	add_partner_hate_unit(uuid, &attack_owner[0]);	
}
void partner_struct::on_beattack(unit_struct *unit, uint32_t skill_id, int32_t damage)
{
	add_partner_hate_unit(unit->get_uuid(), &attack_partner[0]);	
}

double partner_struct::get_skill_angle()
{
	return data->angle;
}
struct position *partner_struct::get_skill_target_pos()
{
	return &data->skill_target_pos;
}


// int partner_struct::count_skill_hit_unit(std::vector<unit_struct *> *ret, struct SkillTable *config, bool bfriend)
// {
// 	if (!config)
// 		return (-1);
// 	switch (config->RangeType)
// 	{
// 		case SKILL_RANGE_TYPE_RECT:
// 			return count_rect_unit(data->angle, ret, config->MaxCount, config->Radius, config->Angle, bfriend);
// 		case SKILL_RANGE_TYPE_CIRCLE:
// 			return count_circle_unit(ret, config->MaxCount, get_pos(), config->Radius, bfriend);			
// 		case SKILL_RANGE_TYPE_FAN:
// 			return count_fan_unit(ret, config->MaxCount, config->Radius, config->Angle, bfriend);
// 		default:
// 			return -10;
// 	}
	
// 	return (0);
// }

// void partner_struct::hit_notify_to_player(uint64_t skill_id, unit_struct *target)
// {
// 	std::vector<unit_struct *> targets;
// 	targets.push_back(target);
// 	hit_notify_to_many_target(skill_id, this, &targets);
// }

// void partner_struct::hit_notify_to_many_friend(uint64_t skill_id, std::vector<unit_struct *> *target)
// {
// 	int n_hit_effect = 0;
// 	int n_buff = 0;

// 	if (target->empty())
// 		return;

// 	struct SkillLvTable *lv_config1, *lv_config2;
// 	struct PassiveSkillTable *pas_config;
// 	struct SkillTable *ski_config;
// 	get_skill_configs(get_skill_level(skill_id), skill_id, &ski_config, &lv_config1, &pas_config, &lv_config2, NULL);
// 	if (!lv_config1 && !lv_config2)
// 	{
// 		LOG_ERR("%s %d: skill[%lu] no config", __FUNCTION__, __LINE__, skill_id);
// 		return;
// 	}

// 	for (std::vector<unit_struct *>::iterator ite = target->begin(); ite != target->end(); ++ite)
// 	{
// 		unit_struct *player = *ite;
// 		assert(player->is_avaliable());

// //		if (get_unit_fight_type(this, player) != UNIT_FIGHT_TYPE_ENEMY)								
// //			continue;			
		
// 		if (!player->is_alive())
// 			continue;

// 		if (player->is_fullhp())
// 			continue;

// 		if (player->is_too_high_to_beattack())
// 			continue;
		
// 		cached_hit_effect_point[n_hit_effect] = &cached_hit_effect[n_hit_effect];
// 		skill_hit_effect__init(&cached_hit_effect[n_hit_effect]);
// 		uint32_t add_num = 0;
// 		int32_t damage = 0;
// 		int32_t other_rate = count_other_skill_damage_effect(this, player);						
// 		damage += count_skill_total_damage(UNIT_FIGHT_TYPE_FRIEND, ski_config, lv_config1,
// 			pas_config, lv_config2,
// 			this, player,
// 			&cached_hit_effect[n_hit_effect].effect,
// 			&cached_buff_id[n_buff],
// 			&cached_buff_end_time[n_buff],
// 			&add_num, other_rate);

// 		LOG_DEBUG("%s: partner unit[%lu][%p] addhp[%d] hp[%f]", __FUNCTION__, player->get_uuid(), player, damage, player->get_attr(PLAYER_ATTR_HP));

// 		cached_hit_effect[n_hit_effect].playerid = player->get_uuid();
// 		cached_hit_effect[n_hit_effect].n_add_buff = add_num;
// 		cached_hit_effect[n_hit_effect].add_buff = &cached_buff_id[n_buff];
// 		cached_hit_effect[n_hit_effect].hp_delta = -damage;
// 		cached_hit_effect[n_hit_effect].cur_hp = player->get_attr(PLAYER_ATTR_HP);
// 		cached_hit_effect[n_hit_effect].target_pos = &cached_target_pos[n_hit_effect];	
// 		pos_data__init(&cached_target_pos[n_hit_effect]);
		
// 		cached_target_pos[n_hit_effect].pos_x = player->get_pos()->pos_x;
// 		cached_target_pos[n_hit_effect].pos_z = player->get_pos()->pos_z;
		
// 		n_buff += add_num;
// 		++n_hit_effect;
// 	}

// 	if (n_hit_effect == 0)
// 		return;

// 	target->push_back(this);

// 	SkillHitNotify notify;
// 	skill_hit_notify__init(&notify);
// 	notify.playerid = data->uuid;
// 	notify.owneriid = notify.playerid;
	
// 	notify.skillid = skill_id;
// 	notify.n_target_player = n_hit_effect;
// 	notify.target_player = cached_hit_effect_point;

// 	notify.attack_cur_hp = get_attr(PLAYER_ATTR_HP);

// 	PosData attack_pos;
// 	pos_data__init(&attack_pos);
// 	attack_pos.pos_x = get_pos()->pos_x;
// 	attack_pos.pos_z = get_pos()->pos_z;
// 	notify.attack_pos = &attack_pos;
// 	player_struct::broadcast_to_many_sight(MSG_ID_SKILL_HIT_NOTIFY, &notify, (pack_func)skill_hit_notify__pack, *target);
// }

// void partner_struct::hit_notify_to_many_player(uint64_t skill_id, std::vector<unit_struct *> *target)
// {
// 	int n_hit_effect = 0;
// 	int n_buff = 0;
// //	double *target_attr;

// 	if (target->empty())
// 		return;
// /*
// 	struct SkillLvTable *lv_config1, *lv_config2;
// 	get_skill_lv_config(skill_id, &lv_config1, &lv_config2);
// 	if (!lv_config1 && !lv_config2)
// 	{
// 		LOG_ERR("%s %d: skill[%u] no config", __FUNCTION__, __LINE__, skill_id);
// 		return;
// 	}
// */
// 	struct SkillLvTable *lv_config1, *lv_config2;
// 	struct PassiveSkillTable *pas_config;
// 	struct SkillTable *ski_config;
// 	get_skill_configs(get_skill_level(skill_id), skill_id, &ski_config, &lv_config1, &pas_config, &lv_config2, NULL);
// 	if (!lv_config1 && !lv_config2)
// 	{
// 		LOG_ERR("%s %d: skill[%lu] no config", __FUNCTION__, __LINE__, skill_id);
// 		return;
// 	}

// 	uint32_t life_steal = 0;
// 	uint32_t damage_return = 0;

// 	for (std::vector<unit_struct *>::iterator ite = target->begin(); ite != target->end(); ++ite)
// 	{
// 		unit_struct *player = *ite;
// 		assert(player->is_avaliable());

// 		if (get_unit_fight_type(this, player) != UNIT_FIGHT_TYPE_ENEMY)								
// 			continue;			
		
// 		if (!player->is_alive())
// 		{
// 			continue;
// 		}

// 		if (player->is_too_high_to_beattack())
// 			continue;
		
// 		cached_hit_effect_point[n_hit_effect] = &cached_hit_effect[n_hit_effect];
// 		skill_hit_effect__init(&cached_hit_effect[n_hit_effect]);
// 		uint32_t add_num = 0;
// 		int32_t damage = 0;
// 		int32_t other_rate = count_other_skill_damage_effect(this, player);						
// 		damage += count_skill_total_damage(UNIT_FIGHT_TYPE_ENEMY, ski_config, lv_config1,
// 			pas_config, lv_config2,
// 			this, player,
// 			&cached_hit_effect[n_hit_effect].effect,
// 			&cached_buff_id[n_buff],
// 			&cached_buff_end_time[n_buff],
// 			&add_num, other_rate);

// 		life_steal += count_life_steal_effect(damage);
// 		damage_return += count_damage_return(damage, player);

// 		// if (player->get_unit_type() == UNIT_TYPE_PLAYER)
// 		// {
// 		// 	player_struct *t = ((player_struct *)player);
// 		// 	raid_struct *raid = t->get_raid();
// 		// 	if (raid)
// 		// 		raid->on_monster_attack(monster, t, damage);

// 		// 	if (owner)
// 		// 	{
// 		// 		check_qiecuo_finished(owner, t);
// 		// 	}
// 		// }

// 		LOG_DEBUG("%s: unit[%lu][%p] damage[%d] hp[%f]", __FUNCTION__, player->get_uuid(), player, damage, player->get_attr(PLAYER_ATTR_HP));

// 		cached_hit_effect[n_hit_effect].playerid = player->get_uuid();
// 		cached_hit_effect[n_hit_effect].n_add_buff = add_num;
// 		cached_hit_effect[n_hit_effect].add_buff = &cached_buff_id[n_buff];
// 		cached_hit_effect[n_hit_effect].hp_delta = damage;
// 		cached_hit_effect[n_hit_effect].cur_hp = player->get_attr(PLAYER_ATTR_HP);
// //		cached_hit_effect[n_hit_effect].attack_pos = &cached_attack_pos[n_hit_effect];
// 		cached_hit_effect[n_hit_effect].target_pos = &cached_target_pos[n_hit_effect];	
// //		pos_data__init(&cached_attack_pos[n_hit_effect]);
// 		pos_data__init(&cached_target_pos[n_hit_effect]);
		
// //		cached_attack_pos[n_hit_effect].pos_x = monster->get_pos()->pos_x;
// //		cached_attack_pos[n_hit_effect].pos_z = monster->get_pos()->pos_z;		
// 		cached_target_pos[n_hit_effect].pos_x = player->get_pos()->pos_x;
// 		cached_target_pos[n_hit_effect].pos_z = player->get_pos()->pos_z;
		
// 		n_buff += add_num;
// 		++n_hit_effect;

// 		if (player->is_alive())
// 		{
// 			player->on_beattack(this, skill_id, damage);						
// 		}
// 		else
// 		{
// 			player->on_dead(this);
// 		}

// 	}

// 	if (n_hit_effect == 0)
// 		return;

// 	target->push_back(this);

// 	SkillHitNotify notify;
// 	skill_hit_notify__init(&notify);
// 	notify.playerid = data->uuid;
// 	notify.owneriid = notify.playerid;
	
// 	notify.skillid = skill_id;
// 	notify.n_target_player = n_hit_effect;
// 	notify.target_player = cached_hit_effect_point;

// 	notify.attack_cur_hp = get_attr(PLAYER_ATTR_HP);
// 	notify.life_steal = life_steal;
// 	notify.damage_return = damage_return;

// 	PosData attack_pos;
// 	pos_data__init(&attack_pos);
// 	attack_pos.pos_x = get_pos()->pos_x;
// 	attack_pos.pos_z = get_pos()->pos_z;
// 	notify.attack_pos = &attack_pos;
// 	player_struct::broadcast_to_many_sight(MSG_ID_SKILL_HIT_NOTIFY, &notify, (pack_func)skill_hit_notify__pack, *target);
	
// 	if (!is_alive())
// 	{
// 		on_dead(this);
// 	}
// }

void partner_struct::relesh_attr()
{
	data->attr_flash.type = rand() % 3;
	for (uint32_t i = 0; i < config->n_AttributeType && i < MAX_PARTNER_BASE_ATTR; ++i)
	{
		data->attr_flash.base_attr_id[i] = config->AttributeType[i];
		data->attr_flash.base_attr_up[i] =  (rand() % (int)(config->UpperLimitBase[i] - config->LowerLimitBase[i]) + config->LowerLimitBase[i]) * (config->GradeCoefficient[0] + config->TypeCoefficient[data->attr_flash.type]);
		data->attr_flash.base_attr_vaual[i] = rand() % (int)config->LowerLimitBase[i] * (config->GradeCoefficient[0] + config->TypeCoefficient[data->attr_flash.type]);

		data->attr_flash.base_attr_up[i] += (data->attrData[PLAYER_ATTR_LEVEL] * config->GrowthValue[i]);
		data->attr_flash.base_attr_vaual[i] += (data->attrData[PLAYER_ATTR_LEVEL] * config->GrowthValue[i]);
	}
	uint32_t drop_id;
	double attrData[MAX_PARTNER_ATTR]; //战斗属性 需要战斗力属性
	::get_attr_from_config((uint64_t)config->PartnerAttributeID[0], attrData, &drop_id);
	for (uint32_t i = 0; i < config->n_PartnerAttributeType && i < MAX_PARTNER_DETAIL_ATTR; ++i)
	{
		data->attr_flash.detail_attr_id[i] = config->PartnerAttributeType[i];
		data->attr_flash.detail_attr_vaual[i] = attrData[data->attr_flash.detail_attr_id[i]] * config->TypeCoefficient[data->attr_flash.type];

		++data->attr_flash.n_detail_attr;
	}

	memset(data->attr_flash.skill_list, 0, sizeof(data->attr_flash.skill_list));
	uint32_t r = rand() % 100000;
	uint32_t all = 0;
	uint32_t j = 0;
	for (; j < config->n_Skill; ++j)
	{
		all += config->SkillProbability[j];
		if (r < all)
		{
			break;
		}
	}
	std::vector<uint64_t> tmpSkill;
	for (uint32_t i = 0; i < config->n_Skill; ++i)
	{
		tmpSkill.push_back(config->Skill[i]);
	}
	int s = 0;
	for (uint32_t i = 0; i < j; ++i)
	{
		if (tmpSkill.size() == 0)
		{
			break;
		}
		uint32_t count = rand() % tmpSkill.size();
		data->attr_flash.skill_list[s].skill_id = tmpSkill[count];
		data->attr_flash.skill_list[s].lv = 1;
		tmpSkill.erase(tmpSkill.begin() + count);
		++s;
	}
	//memcpy(data->attr_flash.skill_list, data->attr_cur.skill_list, sizeof(PartnerSkill) * 3);
}

void partner_struct::do_normal_attack()
{
	if (!data)
		return;
	assert(data->skill_id != 0);

	struct SkillTable *config = get_config_by_id(data->skill_id, &skill_config);
	if (!config)
		return;

		//加血类技能
	if (config->TargetType[0] != 1)	
	{
			// if (config->MaxCount <= 1)
			// {
			// 	try_attack_target(monster, config);
			// }
			// else
		{
			try_attack_friend(this, config);
		}
		
		data->skill_id = 0;
		ai_state = AI_PURSUE_STATE;		
		 	//计算硬直时间
		data->ontick_time += count_skill_delay_time(config);
		return;
	}
	
	reset_pos();	
	
	if (!config)
		return;
	struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
	if (act_config)
	{
		data->ontick_time += act_config->TotalSkillDelay;
	}

	if (config->MaxCount <= 1)
	{
		try_attack_target(this, m_target, config);
	}
	else
	{
		try_attack(this, config);
	}
}

int partner_struct::get_skill_level_byindex(int skill_index)
{
	if (skill_index < 0 || skill_index >= MAX_PARTNER_SKILL_NUM)
		return 1;
	if (data->attr_cur.skill_list[skill_index].skill_id == 0)
		return 1;
	return data->attr_cur.skill_list[skill_index].lv;
}

int partner_struct::get_skill_level(uint32_t skill_id)
{
	for (int i = 0; i < MAX_PARTNER_SKILL_NUM; ++i)
	{
		if (data->attr_cur.skill_list[i].skill_id == skill_id)
			return data->attr_cur.skill_list[i].lv;
	}
	return 1;
}

// void partner_struct::try_attack_target(struct SkillTable *config)
// {
// 	if (!m_target || !m_target->is_avaliable() || !m_target->is_alive())
// 	{
// 		m_target = NULL;
// 		return;
// 	}
// 	struct position *my_pos = get_pos();
// 	struct position *his_pos = m_target->get_pos();

// 	assert(config && config->SkillType == 2);
// //	LOG_DEBUG("%s monster hit skill %u", __FUNCTION__, config->ID);
// 	if (check_distance_in_range(my_pos, his_pos, config->SkillRange/*monster->ai_config->ActiveAttackRange*/))
// 	{
// 		hit_notify_to_target(config->ID, this, m_target);
// 	}
// 	m_target = NULL;	
// }

// void partner_struct::try_attack_friend(struct SkillTable *config)
// {
// 	assert(config && config->SkillType == 2);
	
// 	std::vector<unit_struct *> target;
// 	target.push_back(this);
// 	if (count_skill_hit_unit(&target, config, true) != 0)
// 		return;
// 	hit_notify_to_many_friend(config->ID, this, &target);	
// }

// void partner_struct::cast_skill_to_friend(struct SkillTable *config)
// {
// 	// if (config->MaxCount <= 1)
// 	// {
// 	// 	try_attack_target(monster, config);
// 	// }
// 	// else
// 	{
// 		try_attack_friend(config);
// 	}
// }

// void partner_struct::try_attack(struct SkillTable *config)
// {
// 	assert(config && config->SkillType == 2);
	
// 	std::vector<unit_struct *> target;
// 	if (count_skill_hit_unit(&target, config, false) != 0)
// 		return;
// 	hit_notify_to_many_target(config->ID, this, &target);
// }

void partner_struct::add_sight_space_player_to_sight(sight_space_struct *sight_space, uint16_t *add_player_id_index, uint64_t *add_player)
{
	for (int j = 0; j < MAX_PLAYER_IN_SIGHT_SPACE; ++j)
	{
		player_struct *player = sight_space->players[j];
		if (!player)
			continue;
		if (player->add_partner_to_sight_both(this) >= 0)
		{
			add_player[*add_player_id_index] = player->data->player_id;
			(*add_player_id_index)++;
		}
	}
}

void partner_struct::add_sight_space_monster_to_sight(sight_space_struct *sight_space)
{
	for (int j = 0; j < MAX_MONSTER_IN_SIGHT_SPACE; ++j)
	{
		monster_struct *monster = sight_space->monsters[j];
		if (!monster)
			continue;

		monster->add_partner_to_sight_both(this);
	}
}
