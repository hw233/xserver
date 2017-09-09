#include <string.h>
#include <assert.h>
#include "buff.h"
#include "time_helper.h"
#include "buff_manager.h"
#include "game_config.h"
#include "game_event.h"
#include "count_skill_damage.h"
#include "unit.h"
#include "uuid.h"
#include "player_manager.h"
#include "monster_manager.h"

static double count_added_attr_value(double base_attr, struct SkillEffectTable *effect_config)
{
	return base_attr * ((int64_t)(effect_config->EffectAdd[0]) / 10000.0) + (int64_t)(effect_config->EffectNum[0]);
}

int buff_struct::init_buff(struct BuffTable *buffconfig, uint64_t end_time, unit_struct *attack, unit_struct *owner)
{
	assert(data);
	effect_config = NULL;
	m_owner = NULL;
//	m_attacker = NULL;
	config = buffconfig;//get_config_by_id(id, &buff_config);
	if (config->n_EffectID > 0)
	{
		effect_config = get_config_by_id(config->EffectID[0], &skill_effect_config);
	}
	else
	{
		LOG_ERR("%s: buff[%lu] config err", __FUNCTION__, config->ID);
		return (-1);
	}

	data->buff_id = config->ID;
	data->owner = owner->get_uuid();
	m_owner = owner;
	data->attacker = attack->get_uuid();
//	m_attacker = attack;

	uint32_t old_buff_state = owner->buff_state;
	
	data->start_time = time_helper::get_cached_time();
	data->end_time = end_time;

	if (is_hp_buff())
	{
//		data->effect.hp_effect.hp_delta = -(count_skill_effect(attack->get_all_attr(), owner->get_all_attr(),
//				attack->get_all_buff_fight_attr(), owner->get_all_buff_fight_attr(), effect_config));
		double *attr = owner->get_all_attr();		
		double base_attr = attr[PLAYER_ATTR_MAXHP];
		data->effect.hp_effect.hp_delta = count_added_attr_value(base_attr, effect_config);
		do_hp_buff_effect(false);
	}
	else if (is_recoverable_buff())
	{
		switch (effect_config->Type)
		{
			case 170000008: //-改变属性的buff
			{
				double *attr = owner->get_all_attr();
				double *fight_attr = owner->get_all_buff_fight_attr();				
				assert(attr && fight_attr);
				data->effect.attr_effect.attr_id = effect_config->Effect[0];
				double base_attr = attr[effect_config->Effect[0]];
				data->effect.attr_effect.added_attr_value = count_added_attr_value(base_attr, effect_config);
				
					//速度变化要特殊处理并且通知
				if (data->effect.attr_effect.attr_id == PLAYER_ATTR_MOVE_SPEED)
				{
					attr[effect_config->Effect[0]] += data->effect.attr_effect.added_attr_value;					
					owner->broadcast_one_attr_changed(PLAYER_ATTR_MOVE_SPEED, attr[PLAYER_ATTR_MOVE_SPEED], true, true);
					LOG_DEBUG("%s: player[%lu] add buff[%lu] attr[%lu] delta[%.1f] to[%.1f]",
						__FUNCTION__, owner->get_uuid(), config->ID, effect_config->Effect[0],
						data->effect.attr_effect.added_attr_value, attr[effect_config->Effect[0]]);					
				}
				else
				{
					assert(MAX_BUFF_FIGHT_ATTR > effect_config->Effect[0]);
					fight_attr[effect_config->Effect[0]] += data->effect.attr_effect.added_attr_value;
					LOG_DEBUG("%s: player[%lu] add buff[%lu] attr[%lu] delta[%.1f] to[%.1f]",
						__FUNCTION__, owner->get_uuid(), config->ID, effect_config->Effect[0],
						data->effect.attr_effect.added_attr_value, fight_attr[effect_config->Effect[0]]);
				}
			}
			break;
			case 170000004: //-眩晕，不能移动不能攻击
			{
				data->effect.buff_state.state = BUFF_STATE_STUN;
				m_owner->buff_state |= BUFF_STATE_STUN;
				m_owner->stop_move();
			}
			break;
			case 170000006: //-无敌，不受任何伤害
			{
				data->effect.buff_state.state = BUFF_STATE_GOD;
				m_owner->buff_state |= BUFF_STATE_GOD;				
			}
			break;
			case  170000022: //-嘲讽
			{
				data->effect.buff_state.state = BUFF_STATE_TAUNT;
				m_owner->buff_state |= BUFF_STATE_TAUNT;
				m_owner->stop_move();
				m_owner->clear_cur_skill();
			}
			break;
			case 170000024: //-免疫控制
			{
				data->effect.buff_state.state = BUFF_STATE_AVOID_TRAP;
				m_owner->buff_state |= BUFF_STATE_AVOID_TRAP;
				m_owner->clear_control_buff();
			}
			break;
			case 170000028: //-免疫PVP阴区域BUFF
			{
				data->effect.buff_state.state = BUFF_STATE_AVOID_BLUE_BUFF;
				m_owner->buff_state |= BUFF_STATE_AVOID_BLUE_BUFF;
			}
			break;
			case 170000026: //-免疫PVP阳区域BUFF
			{
				data->effect.buff_state.state = BUFF_STATE_AVOID_RED_BUFF;
				m_owner->buff_state |= BUFF_STATE_AVOID_RED_BUFF;
			}
			break;
			case 170000027: //-免疫PVP阴阳区域BUFF
			{
				data->effect.buff_state.state = (BUFF_STATE_AVOID_BLUE_BUFF | BUFF_STATE_AVOID_RED_BUFF);
				m_owner->buff_state |= (BUFF_STATE_AVOID_BLUE_BUFF | BUFF_STATE_AVOID_RED_BUFF);
			}
			break;
			case 170000029: //-剩余一点血不死
			{
				data->effect.buff_state.state = BUFF_STATE_ONEBLOOD;
				m_owner->buff_state |= BUFF_STATE_ONEBLOOD;
			}
			break;
/*			
			case 170000011: //吸血
			{
				if (effect_config->n_EffectAdd > 0)
				{
					data->effect.buff_state.state = BUFF_STATE_LIFE_STEAL;
					data->effect.buff_state.value = effect_config->EffectAdd[0];
					m_owner->buff_state |= BUFF_STATE_LIFE_STEAL;
					m_owner->life_steal += effect_config->EffectAdd[0];
				}
			}
			break;
			case 170000012: //反弹
			{
				if (effect_config->n_EffectAdd > 0)
				{
					data->effect.buff_state.state = BUFF_STATE_DAMAGE_RETURN;
					data->effect.buff_state.value = effect_config->EffectAdd[0];					
					m_owner->buff_state |= BUFF_STATE_DAMAGE_RETURN;
					m_owner->damage_return += effect_config->EffectAdd[0];					
				}
			}
			break;
*/			
		}
	}
	set_next_timer();

	if (owner->buff_state != old_buff_state)
	{
		owner->broadcast_buff_state_changed();
	}
	return (0);
}

uint32_t buff_struct::get_skill_effect_by_buff_state(int state)
{
	switch(state)
	{
		case BUFF_STATE_GOD:
			return 170000006;
		case BUFF_STATE_STUN:
			return 170000004;
		case BUFF_STATE_CRIT:
			return (0);
		case BUFF_STATE_TAUNT:
			return 170000022;
		case BUFF_STATE_AVOID_TRAP:
			return 170000024;
	}
	return (0);
}

bool buff_struct::is_hp_buff()
{
	if (effect_config && effect_config->Type == 170000001)
		return true;
	return false;
}

bool buff_struct::is_attr_buff()
{
	if (effect_config && effect_config->Type == 170000008)
		return true;
	return false;
}

bool buff_struct::is_recoverable_buff()
{
	if (config->Interval <= 0 && effect_config && effect_config->n_Effect > 0)
		return true;
	return false;
}

int buff_struct::reinit_type3_buff(struct BuffTable *buffconfig)
{
	effect_config = NULL;
	config = buffconfig;
	if (config->n_EffectID > 0)
	{
		effect_config = get_config_by_id(config->EffectID[0], &skill_effect_config);
	}
	else
	{
		LOG_ERR("%s: buff[%lu] config err", __FUNCTION__, config->ID);
		return (-1);
	}
	
	data->buff_id = config->ID;
	data->start_time = time_helper::get_cached_time();
	data->end_time = data->start_time + config->Time;
	buff_manager::buff_ontick_delete(this);		
	set_next_timer();
	return (0);
}

int buff_struct::reinit_buff(struct BuffTable *buffconfig, uint64_t end_time, unit_struct *attack)
{
	effect_config = NULL;
	config = buffconfig;
	if (config->n_EffectID > 0)
	{
		effect_config = get_config_by_id(config->EffectID[0], &skill_effect_config);
	}
	else
	{
		LOG_ERR("%s: buff[%lu] config err", __FUNCTION__, config->ID);
		return (-1);
	}
	data->buff_id = config->ID;
	data->start_time = time_helper::get_cached_time();
	data->end_time = end_time;
	if (is_hp_buff())	
//	if (config->Interval > 0)
	{
//		data->effect.hp_effect.hp_delta = -(count_skill_effect(attack->get_all_attr(), m_owner->get_all_attr(),
//				attack->get_all_buff_fight_attr(), m_owner->get_all_buff_fight_attr(), effect_config));
		double *attr = m_owner->get_all_attr();		
		double base_attr = attr[PLAYER_ATTR_MAXHP];
		data->effect.hp_effect.hp_delta = count_added_attr_value(base_attr, effect_config);		
		do_hp_buff_effect(false);
	}
	else if (is_recoverable_buff() && is_attr_buff())
	{
		assert(m_owner);
		double *attr = m_owner->get_all_attr();
		double *fight_attr = m_owner->get_all_buff_fight_attr();				
		assert(attr && fight_attr);
		
		data->effect.attr_effect.attr_id = effect_config->Effect[0];
		double base_attr = attr[effect_config->Effect[0]];

			//速度变化要通知
		if (data->effect.attr_effect.attr_id == PLAYER_ATTR_MOVE_SPEED)
		{
			uint32_t old_attr = attr[data->effect.attr_effect.attr_id];
			attr[data->effect.attr_effect.attr_id] -= data->effect.attr_effect.added_attr_value;
			base_attr = attr[effect_config->Effect[0]];			
			data->effect.attr_effect.added_attr_value = count_added_attr_value(base_attr, effect_config);
			
			attr[effect_config->Effect[0]] += data->effect.attr_effect.added_attr_value;			
			uint32_t new_attr = attr[data->effect.attr_effect.attr_id];
			if (old_attr != new_attr)
				m_owner->broadcast_one_attr_changed(PLAYER_ATTR_MOVE_SPEED, attr[PLAYER_ATTR_MOVE_SPEED], true, true);
			LOG_DEBUG("%s: player[%lu] reinit[%u] attr[%d] delta[%.1f] to[%.1f]",
				__FUNCTION__, m_owner->get_uuid(), data->buff_id, data->effect.attr_effect.attr_id,
				data->effect.attr_effect.added_attr_value, attr[data->effect.attr_effect.attr_id]);
			
		}
		else
		{
			assert(MAX_BUFF_FIGHT_ATTR > effect_config->Effect[0]);			
			fight_attr[data->effect.attr_effect.attr_id] -= data->effect.attr_effect.added_attr_value;
			data->effect.attr_effect.added_attr_value = count_added_attr_value(base_attr, effect_config);
			fight_attr[effect_config->Effect[0]] += data->effect.attr_effect.added_attr_value;
			LOG_DEBUG("%s: player[%lu] reinit[%u] attr[%d] delta[%.1f] to[%.1f]",
				__FUNCTION__, m_owner->get_uuid(), data->buff_id, data->effect.attr_effect.attr_id,
				data->effect.attr_effect.added_attr_value, fight_attr[data->effect.attr_effect.attr_id]);
		}
	}
	buff_manager::buff_ontick_delete(this);		
	set_next_timer();
	return (0);
}

unit_struct *buff_struct::get_attacker()
{
	unit_struct *ret = unit_struct::get_unit_by_uuid(data->attacker);
	if (ret)
		return ret;
	return m_owner;
}

bool buff_struct::do_hp_buff_effect(bool check_dead)
{
//	double *attr = unit_struct::get_attr_by_uuid(data->owner);
	if (m_owner->buff_state & BUFF_STATE_GOD)
		return true;

	if (data->effect.hp_effect.hp_delta == 0)
		return true;

	if (data->effect.hp_effect.hp_delta < 0
		&& m_owner->buff_state & BUFF_STATE_ONEBLOOD)
		return true;
	
	double *attr = m_owner->get_all_attr();
	assert(attr);
	int old = attr[PLAYER_ATTR_HP];
	attr[PLAYER_ATTR_HP] += data->effect.hp_effect.hp_delta;
	if (attr[PLAYER_ATTR_HP] > attr[PLAYER_ATTR_MAXHP])
		attr[PLAYER_ATTR_HP] = attr[PLAYER_ATTR_MAXHP];
	
	m_owner->on_hp_changed(-data->effect.hp_effect.hp_delta);

	if (attr[PLAYER_ATTR_HP] == old)
		return true;
	
	m_owner->broadcast_one_attr_changed(PLAYER_ATTR_HP, attr[PLAYER_ATTR_HP], true, true);

	LOG_DEBUG("%s: unit[%lu][%p] damage[%d] hp[%f] check_dead[%d]", __FUNCTION__, m_owner->get_uuid(),
		m_owner, data->effect.hp_effect.hp_delta, m_owner->get_attr(PLAYER_ATTR_HP), check_dead);	

	if (check_dead && !m_owner->is_alive())
	{
		unit_struct *attacker = get_attacker();
		m_owner->on_dead(attacker);
		return false;
	}
	return true;
}

void buff_struct::set_next_timer()
{
	if (!data)
		return;
	if (config->Interval == 0)
	{
		data->ontick_time = time_helper::get_cached_time() + config->Time;		
	}
	else
	{
		assert(is_hp_buff());
		data->ontick_time = time_helper::get_cached_time() + config->Interval;
	}
	buff_manager::buff_ontick_settimer(this);
}

void buff_struct::deal_with_del_effect()
{
	if (config->n_DelEffectID <= 0)
		return;
	if (config->DelEffectID[0] == 0)
		return;
	struct SkillEffectTable *del_config = get_config_by_id(config->DelEffectID[0], &skill_effect_config);
	assert(del_config);
	if (del_config->Type == 170000001)//-血量变化
	{
		double *attr = m_owner->get_all_attr();				
		double base_attr = attr[PLAYER_ATTR_MAXHP];
		int added_attr_value = count_added_attr_value(base_attr, del_config);
		data->effect.hp_effect.hp_delta = added_attr_value;
		do_hp_buff_effect(false);
	}
	else if (del_config->Type == 170000008)//-改变属性的buff
	{
		double *attr = m_owner->get_all_attr();
		double *fight_attr = m_owner->get_all_buff_fight_attr();				
		assert(attr && fight_attr);
		double base_attr = attr[del_config->Effect[0]];
		int added_attr_value = count_added_attr_value(base_attr, del_config);
				
			//速度变化要特殊处理并且通知
		if (data->effect.attr_effect.attr_id == PLAYER_ATTR_MOVE_SPEED)
		{
			attr[del_config->Effect[0]] += added_attr_value;
			m_owner->broadcast_one_attr_changed(PLAYER_ATTR_MOVE_SPEED, attr[PLAYER_ATTR_MOVE_SPEED], true, true);
			LOG_DEBUG("%s: player[%lu] add buff[%lu] attr[%lu] delta[%.1f] to[%.1f]",
				__FUNCTION__, m_owner->get_uuid(), config->ID, del_config->Effect[0],
				data->effect.attr_effect.added_attr_value, attr[del_config->Effect[0]]);					
		}
		else
		{
			assert(MAX_BUFF_FIGHT_ATTR > del_config->Effect[0]);
			fight_attr[del_config->Effect[0]] += added_attr_value;
			LOG_DEBUG("%s: player[%lu] add buff[%lu] attr[%lu] delta[%.1f] to[%.1f]",
				__FUNCTION__, m_owner->get_uuid(), config->ID, del_config->Effect[0],
				data->effect.attr_effect.added_attr_value, fight_attr[del_config->Effect[0]]);
		}
	}
}

void buff_struct::deal_with_recover_effect()
{
	switch (effect_config->Type)
	{
		case 170000008: //-改变属性的buff
		{
			double *attr = m_owner->get_all_attr();
			double *fight_attr = m_owner->get_all_buff_fight_attr();				
			assert(attr && fight_attr);

				//速度变化要通知
			if (data->effect.attr_effect.attr_id == PLAYER_ATTR_MOVE_SPEED)
			{
				attr[data->effect.attr_effect.attr_id] -= data->effect.attr_effect.added_attr_value;					
				m_owner->broadcast_one_attr_changed(PLAYER_ATTR_MOVE_SPEED, attr[PLAYER_ATTR_MOVE_SPEED], true, true);
				LOG_DEBUG("%s: player[%lu] add buff[%lu] attr[%lu] delta[%.1f] to[%.1f]",
					__FUNCTION__, m_owner->get_uuid(), config->ID, effect_config->Effect[0],
					data->effect.attr_effect.added_attr_value, attr[effect_config->Effect[0]]);					
			}
			else
			{
				assert(MAX_BUFF_FIGHT_ATTR > effect_config->Effect[0]);								
				fight_attr[data->effect.attr_effect.attr_id] -= data->effect.attr_effect.added_attr_value;
				LOG_DEBUG("%s: player[%lu] add buff[%lu] attr[%lu] delta[%.1f] to[%.1f]",
					__FUNCTION__, m_owner->get_uuid(), config->ID, effect_config->Effect[0],
					data->effect.attr_effect.added_attr_value, fight_attr[effect_config->Effect[0]]);
			}
		}
		break;
		case 170000004: //-眩晕，不能移动不能攻击
		case 170000006: //-无敌，不受任何伤害
		case 170000022: //-嘲讽
		case 170000024: //-免控
		case 170000028: //-免疫PVP阴区域BUFF
		case 170000026: //-免疫PVP阳区域BUFF
		case 170000027: //-免疫PVP阴阳区域BUFF
		{
			if (m_owner)
				m_owner->reset_unit_buff_state();
		}
		break;
		case 170000029: //-剩余一点血不死
		{
			if (m_owner)
				m_owner->reset_unit_buff_state();
			if (m_owner->is_alive())
			{
				uint32_t maxhp = m_owner->get_attr(PLAYER_ATTR_MAXHP);
				m_owner->broadcast_one_attr_changed(PLAYER_ATTR_HP, maxhp, true, true);
			}
		}
		break;
	}
}

void buff_struct::del_buff()
{
	if (m_owner)
	{
		m_owner->delete_one_buff(this);
		deal_with_del_effect();
	}
	
	if (is_recoverable_buff())
	{
		deal_with_recover_effect();
	}
	buff_manager::delete_buff(this);
}

void buff_struct::on_dead()
{
	if (config->DeleteType == 0)
		return del_buff();
}

void buff_struct::on_tick()
{
		//恢复属性,删除buff
	if (time_helper::get_cached_time() >= data->end_time)
	{
		return del_buff();
	}
	else		//buff间隔效果, 只有血量变化
	{
		if (!do_hp_buff_effect(true))
			return;
	}


	
	set_next_timer();
}




