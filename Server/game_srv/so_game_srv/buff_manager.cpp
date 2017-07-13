#include "buff_manager.h"
#include "player.h"
#include "game_event.h"
#include "time_helper.h"
#include "game_config.h"
#include "../proto/cast_skill.pb-c.h"
#include "msgid.h"

buff_manager::buff_manager()
{
}

buff_manager::~buff_manager()
{
}

// struct minheap buff_manager::buff_manager_m_minheap;
// std::list<buff_struct *> buff_manager::buff_manager_buff_free_list;
// std::set<buff_struct *> buff_manager::buff_manager_buff_used_list;		
// comm_pool buff_manager::buff_manager_buff_data_pool;

//////////////////////////////////////////////
static bool minheap_cmp_buff_timeout(void *a, void *b)
{
	buff_struct *aa = (buff_struct *)a;
	buff_struct *bb = (buff_struct *)b;
	if (aa->data->ontick_time < bb->data->ontick_time)
		return true;
	return false;
}

static int minheap_get_buff_timeout_index(void *a)
{
	buff_struct *aa = (buff_struct *)a;
	return aa->data->heap_index;
}

static void minheap_set_buff_timeout_index(int index, void *a)
{
	buff_struct *aa = (buff_struct *)a;
	aa->data->heap_index = index;	
}

void buff_manager::buff_ontick_settimer(buff_struct *p)
{
	push_heap(&buff_manager_m_minheap, p);
}
buff_struct *buff_manager::get_ontick_buff(uint64_t now)
{
	if (buff_manager_m_minheap.cur_size == 0)
		return NULL;
	
	if (((buff_struct *)get_heap_first(&buff_manager_m_minheap))->data->ontick_time > now)
		return NULL;
	return (buff_struct *)pop_heap(&buff_manager_m_minheap);
}

void buff_manager::buff_ontick_delete(buff_struct *p)
{
	if (is_node_in_heap(&buff_manager_m_minheap, p))
		erase_heap_node(&buff_manager_m_minheap, p);
}

//////////////////////////////////////////


void buff_manager::on_tick_30()
{
	uint64_t now = time_helper::get_cached_time();
	buff_struct *buff = get_ontick_buff(now);
	while (buff != NULL)
	{
		buff->on_tick();
//		buff->set_next_timer();
		buff = get_ontick_buff(now);
	}
/*	
	for (std::set<buff_struct *>::iterator iter = buff_manager_buff_used_list.begin(); iter != buff_manager_buff_used_list.end(); ++iter)
	{
		(*iter)->on_tick();
	}
*/	
}

int buff_manager::reinit_min_heap()
{
	buff_manager_m_minheap.cmp = minheap_cmp_buff_timeout;
	buff_manager_m_minheap.get = minheap_get_buff_timeout_index;
	buff_manager_m_minheap.set = minheap_set_buff_timeout_index;
	return (0);
}

int buff_manager::init_buff_struct(int num, unsigned long key)
{
	init_heap(&buff_manager_m_minheap, num, minheap_cmp_buff_timeout, minheap_get_buff_timeout_index, minheap_set_buff_timeout_index);	
	buff_struct *buff;
	for (int i = 0; i < num; ++i) {
		buff = new buff_struct();
		buff_manager_buff_free_list.push_back(buff);
	}
	return init_mass_pool(0, sizeof(buff_data), num, key, &buff_manager_buff_data_pool);
}

int buff_manager::do_move_buff_effect(struct BuffTable *config, unit_struct *attack, unit_struct *owner)
{
	struct SkillEffectTable *effect_config;
	effect_config = get_config_by_id(config->EffectID[0], &skill_effect_config);
	assert(config && config->n_EffectID != 0);
	assert(effect_config);
	if (effect_config->n_EffectAdd != 1)
	{
		LOG_ERR("%s: buff[%lu] atk[%lu] owner[%p]", __FUNCTION__, config->ID, attack->get_uuid(), owner);
		return (-10);			
	}
	
	double distance = effect_config->EffectAdd[0] * config->Time / 1000.0;
	struct position *atk_pos = attack->get_pos();
	struct position *owner_pos = owner->get_pos();
	if (!atk_pos || !owner_pos)
	{
		LOG_ERR("%s: atk[%lu %p] owner[%lu %p]", __FUNCTION__, attack->get_uuid(), atk_pos, owner->get_uuid(), owner_pos);
		return (-20);
	}
	double cur_distance = getdistance(atk_pos, owner_pos);
	if (cur_distance > __DBL_EPSILON__)
	{
		double rate;
		if (effect_config->Type == 170000021)
			rate = distance / cur_distance;
		else
			rate = (distance + cur_distance) / cur_distance;
		double new_pos_x = (owner_pos->pos_x - atk_pos->pos_x) * rate + atk_pos->pos_x;
		double new_pos_z = (owner_pos->pos_z - atk_pos->pos_z) * rate + atk_pos->pos_z;
		struct map_block *block = get_map_block(owner->scene->map_config, new_pos_x, new_pos_z);
		if (block && block->can_walk)
		{
			LOG_DEBUG("%s: buff[%lu] attack[%lu][%.1f][%.1f] owner[%lu] from[%.1f][%.1f] to [%.1f][%.1f]",
				__FUNCTION__, config->ID, attack->get_uuid(), atk_pos->pos_x, atk_pos->pos_z,
				owner->get_uuid(), owner_pos->pos_x, owner_pos->pos_z, new_pos_x, new_pos_z);
			owner->set_pos_with_broadcast(new_pos_x, new_pos_z);
		}
	}

	owner->on_repel(attack);
	return (0);
}

buff_struct *buff_manager::create_default_buff(uint64_t id, unit_struct *attack, unit_struct *owner, bool notify)
{
	struct BuffTable *config = get_config_by_id(id, &buff_config);
	if (!config)
		return NULL;
	uint64_t now = time_helper::get_cached_time();
	
	return create_buff(id, config->Time + now, attack, owner, notify);
}

buff_struct *buff_manager::create_buff(uint64_t id, uint64_t end_time, unit_struct *attack, unit_struct *owner, bool notify)
{
	struct BuffTable *config = get_config_by_id(id, &buff_config);
	if (!config)
		return NULL;

	if (owner->buff_state & BUFF_STATE_AVOID_TRAP && config->IsControl)
		return NULL;

	uint64_t buff_effect_type = get_buff_first_effect_type(id);
	if (is_clear_debuff_buff_effect(buff_effect_type))
	{
		owner->clear_debuff();
		return NULL;
	}

	if (is_move_buff_effect(buff_effect_type))
	{
		if (config->TimeDelay > 0)
		{
			owner->set_lock_time(config->TimeDelay + time_helper::get_cached_time());
		}
		do_move_buff_effect(config, attack, owner);
		return NULL;
	}

	LOG_DEBUG("%s: player[%lu] add buff[%lu]", __FUNCTION__, owner->get_uuid(), id);

	buff_struct *ret;
	ret = owner->try_cover_duplicate_buff(config, end_time, attack);
	if (ret)
		return ret;

	int buff_pos = owner->get_free_buff_pos();
	if (buff_pos < 0)
		return NULL;
	
	ret = alloc_buff();
	if (!ret)
	{
		LOG_ERR("%s: alloc buff fail", __FUNCTION__);
		return ret;
	}
	
	if (ret->init_buff(config, end_time, attack, owner) != 0)
	{
		delete_buff(ret);
		return NULL;
	}
	owner->set_one_buff(ret, buff_pos);

	LOG_DEBUG("%s: %lu add buff %lu endtime[%lu]", __FUNCTION__, owner->get_uuid(), id, ret->data->end_time);

	if (notify)
	{
		AddBuffNotify notify;
		add_buff_notify__init(&notify);
		notify.buff_id = id;
		notify.start_time = ret->data->start_time / 1000;
//		notify.end_time = ret->data->end_time / 1000;		
		notify.playerid = owner->get_uuid();
		owner->broadcast_to_sight(MSG_ID_ADD_BUFF_NOTIFY, &notify, (pack_func)add_buff_notify__pack, true);
	}
	
	return ret;
}

uint64_t buff_manager::get_buff_first_effect_type(uint64_t id)
{
	struct BuffTable *config;
	struct SkillEffectTable *effect_config;
	
	config = get_config_by_id(id, &buff_config);
	if (!config || config->n_EffectID == 0)
		return 0;
	effect_config = get_config_by_id(config->EffectID[0], &skill_effect_config);
	if (!effect_config)
		return 0;
	return effect_config->Type;
}

bool buff_manager::is_clear_debuff_buff_effect(uint64_t effect_id)
{
	if (effect_id == 170000024)
		return true;
	return false;
}

bool buff_manager::is_move_buff_effect(uint64_t effect_id)
{
	switch (effect_id)
	{
		case 170000009:
		case 170000010:
		case 170000021:
			return true;
	}
	return false;
}

buff_struct *buff_manager::alloc_buff()
{
	buff_struct *ret = NULL;
	buff_data *data = NULL;
	if (buff_manager_buff_free_list.empty())
		return NULL;
	ret = buff_manager_buff_free_list.back();
	buff_manager_buff_free_list.pop_back();
	data = (buff_data *)mass_pool_alloc(&buff_manager_buff_data_pool);
	if (!data)
		goto fail;
	memset(data, 0, sizeof(buff_data));
	ret->data = data;
	buff_manager_buff_used_list.insert(ret);
	LOG_INFO("[%s:%d] buff:%p, data:%p", __FUNCTION__, __LINE__, ret, ret->data);	
	return ret;
fail:
	if (ret) {
		buff_manager_buff_used_list.erase(ret);
		buff_manager_buff_free_list.push_back(ret);
	}
	if (data) {
		mass_pool_free(&buff_manager_buff_data_pool, data);
	}
	return NULL;
}

void buff_manager::delete_buff(buff_struct *p)
{
	buff_manager_buff_used_list.erase(p);
	buff_manager_buff_free_list.push_back(p);

//	if (p->m_owner)
//	{
//		p->m_owner->delete_one_buff(p);
//	}

	buff_ontick_delete(p);	
	LOG_INFO("[%s:%d] buff:%p, data:%p [%u]", __FUNCTION__, __LINE__, p, p->data, p->data->buff_id);

	if (p->data) {
		mass_pool_free(&buff_manager_buff_data_pool, p->data);		
		p->data = NULL;
	}
}


// int buff_manager::add_skill_buff(unit_struct *attack, unit_struct *owner, int add_num, uint32_t *buff_id, uint32_t *buff_end_time)
// {
// 	for (int i = 0; i < add_num; ++i)
// 	{
// 		create_buff(buff_id[i], buff_end_time[i], attack, owner);
// 	}
// 	return (0);
// }

int buff_manager::load_item_buff(player_struct *player, ItemBuff *db_item_buff)
{
	if (time_helper::get_cached_time() >= db_item_buff->end_time)
		return (0);
	for (int i = 0; i < MAX_BUFF_PER_UNIT; ++i)
	{
		if (player->m_buffs[i])
			continue;
		player->m_buffs[i] = alloc_buff();
		if (!player->m_buffs[i])
		{
			LOG_ERR("%s: player[%lu] alloc buff[%u] failed", __FUNCTION__, player->get_uuid(), db_item_buff->id);
			return (-1);
		}
		struct BuffTable *config = get_config_by_id(db_item_buff->id, &buff_config);
		if (!config)
		{
			LOG_ERR("%s: player[%lu] get config  buff[%u] failed", __FUNCTION__, player->get_uuid(), db_item_buff->id);
			return (-10);			
		}
		player->m_buffs[i]->config = config;
		struct SkillEffectTable *effect_config;
		if (config->n_EffectID > 0)
		{
			effect_config = get_config_by_id(config->EffectID[0], &skill_effect_config);
			assert(effect_config);
			player->m_buffs[i]->effect_config = effect_config;
		}
		else
		{
			LOG_ERR("%s: buff[%lu] config err", __FUNCTION__, config->ID);
			return (-20);
		}

		
		player->m_buffs[i]->data->buff_id = config->ID;
		player->m_buffs[i]->data->owner = player->get_uuid();
		player->m_buffs[i]->m_owner = player;
		player->m_buffs[i]->data->attacker = player->m_buffs[i]->data->owner;
		player->m_buffs[i]->m_attacker = player;
		
		player->m_buffs[i]->data->start_time = time_helper::get_cached_time();
		player->m_buffs[i]->data->end_time = db_item_buff->end_time;
		assert(player->m_buffs[i]->is_recoverable_buff());
//		assert(effect_config->Type == 170000008 || effect_config->Type == 170000018 || effect_config->Type == 170000029);
		
		switch (effect_config->Type)
		{
			case 170000029:
				player->m_buffs[i]->data->effect.buff_state.state = db_item_buff->buff_state;
				player->buff_state |= player->m_buffs[i]->data->effect.buff_state.state;
				break;
			case 170000018:
				break;
			case 170000008:
			{
				double *attr = player->get_all_attr();
				double *fight_attr = player->get_all_buff_fight_attr();
				assert(attr && fight_attr);
				player->m_buffs[i]->data->effect.attr_effect.attr_id = effect_config->Effect[0];
				double base_attr = attr[effect_config->Effect[0]];
				player->m_buffs[i]->data->effect.attr_effect.added_attr_value = base_attr * (effect_config->EffectAdd[0] / 10000.0 - 1) + effect_config->EffectNum[0];


				assert(MAX_BUFF_FIGHT_ATTR > effect_config->Effect[0]);

				fight_attr[effect_config->Effect[0]] += player->m_buffs[i]->data->effect.attr_effect.added_attr_value;

				LOG_DEBUG("%s: player[%lu] add buff[%lu] attr[%lu] delta[%.1f] to[%.1f]",
					__FUNCTION__, player->get_uuid(), config->ID, effect_config->Effect[0],
					player->m_buffs[i]->data->effect.attr_effect.added_attr_value, fight_attr[effect_config->Effect[0]]);
			}
			break;
			default:
				assert(0);
		}
		break;
	}
	return (0);
}

uint32_t buff_manager::get_buff_count()
{
	return buff_manager_buff_used_list.size();
}

uint32_t buff_manager::get_buff_pool_max_num()
{
	return buff_manager_buff_data_pool.num;
}
