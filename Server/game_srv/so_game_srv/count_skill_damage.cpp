#include <stdio.h>
#include <string.h>
#include <map>
#include <stdlib.h>
#include "count_skill_damage.h"
#include "attr_id.h"
#include "game_config.h"
#include "time_helper.h"
#include "excel_data.h"
#include "buff_manager.h"
#include "game_config.h"
#include "game_event.h"
#include "chengjie.h"
#include "monster.h"

static int32_t count_skill_effect_entry(const double *attack, const double *defence,
	const double *buff_fight_attack, const double *buff_fight_defence,
	uint64_t effect, uint64_t effect_add, uint64_t effect_num)
{
	double at, de;//, jianmian;
	double t1;
	int32_t ret = 0;
	switch (effect)
	{
		case PLAYER_ATTR_HP:
			ret = defence[PLAYER_ATTR_HP] * effect_add / 10000
				+ effect_num;
			break;
		case PLAYER_ATTR_MAXHP:
			ret = defence[PLAYER_ATTR_MAXHP] * effect_add / 10000
				+ effect_num;
			break;
		case PLAYER_ATTR_ATTACK: //攻击
			ret = buff_fight_attack[PLAYER_ATTR_ATTACK] * effect_add / 10000
				+ effect_num;
//			de = 0;//buff_fight_defence[PLAYER_ATTR_DEFENSE];
//			jianmian = de / (4 * at + de * 1.1) + 0.1;
//			ret = at * (1 - jianmian);
			break;
//守方五行抗性减免=（守方五行抗性-攻方忽略全抗）/（守方五行抗性-攻方忽略全抗+攻方等级*五行抗性等级系数+五行抗性基础值）
		case PLAYER_ATTR_ATK_METAL: //金
			t1 = buff_fight_defence[PLAYER_ATTR_DEF_METAL] - buff_fight_attack[PLAYER_ATTR_DFWUDEL];
			if (t1 < 0)
			{
				de = 1;
			}
			else
			{
				de = 1 - t1 / (t1 + attack[PLAYER_ATTR_LEVEL] * sg_fight_param_161000287 + sg_fight_param_161000288);
			}
			at = buff_fight_attack[PLAYER_ATTR_ATK_METAL] * effect_add / 10000
					+ effect_num;
			ret = at * de;
//			de = buff_fight_defence[PLAYER_ATTR_DEF_METAL];
//			ret = at - de;
			break;
		case PLAYER_ATTR_ATK_WOOD:
			at = buff_fight_attack[PLAYER_ATTR_ATK_WOOD] * effect_add / 10000
				+ effect_num;
//			de = buff_fight_defence[PLAYER_ATTR_DEF_WOOD];
//			ret = at - de;
			t1 = buff_fight_defence[PLAYER_ATTR_DEF_WOOD] - buff_fight_attack[PLAYER_ATTR_DFWUDEL];
			if (t1 < 0)
			{
				de = 1;
			}
			else
			{			
				de = 1 - t1 / (t1 + attack[PLAYER_ATTR_LEVEL] * sg_fight_param_161000287 + sg_fight_param_161000288);
			}
			ret = at * de;
			break;
		case PLAYER_ATTR_ATK_WATER:
			at = buff_fight_attack[PLAYER_ATTR_ATK_WATER] * effect_add / 10000
				+ effect_num;
//			de = buff_fight_defence[PLAYER_ATTR_DEF_WATER];
//			ret = at - de;
			t1 = buff_fight_defence[PLAYER_ATTR_DEF_WATER] - buff_fight_attack[PLAYER_ATTR_DFWUDEL];
			if (t1 < 0)
			{
				de = 1;
			}
			else
			{			
				de = 1 - t1 / (t1 + attack[PLAYER_ATTR_LEVEL] * sg_fight_param_161000287 + sg_fight_param_161000288);
			}
			ret=  at * de;
			break;
		case PLAYER_ATTR_ATK_FIRE: //火
			at = buff_fight_attack[PLAYER_ATTR_ATK_FIRE] * effect_add / 10000
				+ effect_num;
//			de = buff_fight_defence[PLAYER_ATTR_DEF_FIRE];
//			ret = at - de;
			t1 = buff_fight_defence[PLAYER_ATTR_DEF_FIRE] - buff_fight_attack[PLAYER_ATTR_DFWUDEL];
			if (t1 < 0)
			{
				de = 1;
			}
			else
			{			
				de = 1 - t1 / (t1 + attack[PLAYER_ATTR_LEVEL] * sg_fight_param_161000287 + sg_fight_param_161000288);
			}
			ret=  at * de;
			break;
		case PLAYER_ATTR_ATK_EARTH:
			at = buff_fight_attack[PLAYER_ATTR_ATK_EARTH] * effect_add / 10000
				+ effect_num;
			t1 = buff_fight_defence[PLAYER_ATTR_DEF_EARTH] - buff_fight_attack[PLAYER_ATTR_DFWUDEL];
			if (t1 < 0)
			{
				de = 1;
			}
			else
			{			
				de = 1 - t1 / (t1 + attack[PLAYER_ATTR_LEVEL] * sg_fight_param_161000287 + sg_fight_param_161000288);
			}
			ret = at * de;
//			de = buff_fight_defence[PLAYER_ATTR_DEF_EARTH];
//			ret = at - de;
			break;
	}
	if (ret < 0)
		ret = 0;
	return (ret);
}

int32_t count_skill_effect(const double *attack, const double *defence,
	const double *buff_fight_attack, const double *buff_fight_defence,
	struct SkillEffectTable *effectconfig, unit_struct *attack_unit)
{
	int32_t ret = 0;
	uint32_t other_skill_lv = 0;
	if (effectconfig->SkillID != 0)
	{
		other_skill_lv = attack_unit->get_skill_lv(effectconfig->SkillID);
	}
	for (size_t i = 0; i < effectconfig->n_Effect; ++i)
	{
		uint64_t effectadd = effectconfig->EffectAdd[i];
		uint64_t effectnum = effectconfig->EffectNum[i];
		if (other_skill_lv != 0)
		{
			effectadd += other_skill_lv * effectconfig->EffectAdd1[i];
			effectnum += other_skill_lv * effectconfig->EffectNum1[i];
		}
		
		ret += count_skill_effect_entry(attack, defence, buff_fight_attack, buff_fight_defence,
			effectconfig->Effect[i], effectadd, effectnum);
	}
	return ret;
}

	static bool check_can_add_buff(unit_struct *attack_unit, unit_struct *defence_unit, uint32_t buff_id)
{
	uint64_t type = buff_manager::get_buff_first_effect_type(buff_id);
	if (buff_manager::is_move_buff_effect(type) &&
		(defence_unit->is_in_lock_time() || (attack_unit->get_unit_type() == UNIT_TYPE_PLAYER && attack_unit->get_unit_type() == UNIT_TYPE_PLAYER )))
		return false;
	return true;
}

static void count_friend_buff(struct SkillTimeTable *timeconfig,
	uint32_t skill_lv,
	unit_struct *attack_unit,
	unit_struct *defence_unit,
	uint32_t buff_add[],
	uint32_t buff_add_end_time[],	
	uint32_t *n_buff_add)
{
	struct BuffTable *config;
//	double *attack = attack_unit->get_all_attr();
//	double *defence = defence_unit->get_all_attr();
	int n = 0;
//	uint64_t now = time_helper::get_cached_time();	

	for (size_t i = 0; i < timeconfig->n_BuffIdFriend; ++i)
	{
		if (!check_can_add_buff(attack_unit, defence_unit, timeconfig->BuffIdFriend[i]))
			continue;

		config = get_config_by_id(timeconfig->BuffIdFriend[i] + skill_lv - 1, &buff_config);
		if (!config)
			continue;

		int32_t rate;
//		if (config->AtPro == 0 || config->DfPro)
			rate = config->NeedPro;
//		else
//			rate = attack[config->AtPro] - defence[config->DfPro] + config->NeedPro;
		int32_t randnum = random() % 10000;

		if (randnum > rate)
			continue;

//		uint32_t time = config->Time;
		buff_manager::create_default_buff(timeconfig->BuffIdFriend[i], attack_unit, defence_unit);
		
		buff_add[*n_buff_add + n] = timeconfig->BuffIdFriend[i];
//		buff_add_end_time[*n_buff_add + n] = (now + time) / 1000;		
		++n;
	}
//	buff_manager::add_skill_buff(attack_unit, defence_unit, n, &buff_add[*n_buff_add], &buff_add_end_time[*n_buff_add]);
	(*n_buff_add) += n;
}
static void count_enemy_buff(struct SkillTimeTable *timeconfig,
	uint32_t skill_lv,
	unit_struct *attack_unit,
	unit_struct *defence_unit,
	uint32_t buff_add[],
	uint32_t buff_add_end_time[],
	uint32_t *n_buff_add)
{
	struct BuffTable *config;
	double *attack = attack_unit->get_all_attr();
	double *defence = defence_unit->get_all_attr();
	int n = 0;
//	uint64_t now = time_helper::get_cached_time();

	for (size_t i = 0; i < timeconfig->n_BuffIdEnemy; ++i)
	{
		if (!check_can_add_buff(attack_unit, defence_unit, timeconfig->BuffIdEnemy[i]))
			continue;

		config = get_config_by_id(timeconfig->BuffIdEnemy[i] + skill_lv - 1, &buff_config);
		if (!config)
			continue;

		if (defence_unit->buff_state & BUFF_STATE_AVOID_TRAP && config->IsControl)
			continue;

			  // 攻方实际受伤几率=攻方受伤几率/（攻方受伤几率+特殊属性基础值）
			  // 守方实际抗受伤几率=守方抗受伤几率/（守方抗受伤几率+特殊属性基础值）

			  // if（攻方技能几率+攻方实际受伤几率<守方实际抗受伤几率)
			  //      攻方受伤buff触发几率=0
			  // else
			  //      攻方受伤buff触发几率=攻方技能几率+攻方实际受伤几率-守方实际抗受伤几率

		int32_t rate;
		double attack_rate, defence_rate;
		switch (config->DfPro)
		{
			// case PLAYER_ATTR_DEEFFDF:
			// 	attack_rate = 0;
			// 	defence_rate = defence[config->DfPro];
			// 	break;
			case PLAYER_ATTR_DIZZYDF:
				attack_rate = attack[PLAYER_ATTR_DIZZY];
				defence_rate = defence[config->DfPro];
				break;
			case PLAYER_ATTR_SLOWDF:
				attack_rate = attack[PLAYER_ATTR_SLOW];
				defence_rate = defence[config->DfPro];
				break;
			case PLAYER_ATTR_MABIDF:
				attack_rate = attack[PLAYER_ATTR_MABI];
				defence_rate = defence[config->DfPro];
				break;
			case PLAYER_ATTR_HURTDF:
				attack_rate = attack[PLAYER_ATTR_HURT];
				defence_rate = defence[config->DfPro];
				break;
			case PLAYER_ATTR_CANDF:
				attack_rate = attack[PLAYER_ATTR_CAN];
				defence_rate = defence[config->DfPro];
				break;
			default:
				attack_rate = defence_rate = 0;
				break;
		}
		attack_rate = attack_rate / (attack_rate + sg_fight_param_161000289);
		defence_rate = defence_rate / (defence_rate + sg_fight_param_161000289);

		if (config->NeedPro + attack_rate < defence_rate)
			continue;

		rate = config->NeedPro + attack_rate - defence_rate;
		int32_t randnum = random() % 10000;

		if (randnum > rate)
			continue;

		// 	  // 攻方实际受伤时间比例=攻方受伤时间/（攻方受伤时间+特殊属性基础值）
		// 	  // 守方实际抗受伤时间比例=守方抗受伤时间/（守方抗受伤时间+特殊属性基础值）

		// 	  // if（1+攻方实际受伤时间比例-守方实际抗受伤时间比例<buff持续时间保底比例)
		// 	  //      int（攻方受伤buff持续时间=攻方技能时间*buff持续时间保底比例）
		// 	  // else
		// 	  //      int（攻方受伤buff持续时间=攻方技能时间*（1+攻方实际受伤时间比例-守方实际抗受伤时间比例））
		// switch (config->DfPro)
		// {
		// 	case PLAYER_ATTR_DEEFFDF:
		// 		attack_rate = 0;
		// 		defence_rate = defence[PLAYER_ATTR_DETIMEDF];
		// 		break;
		// 	case PLAYER_ATTR_DIZZYDF:
		// 		attack_rate = attack[PLAYER_ATTR_DIZZYTIME];
		// 		defence_rate = defence[PLAYER_ATTR_DIZZYTIMEDF];
		// 		break;
		// 	case PLAYER_ATTR_SLOWDF:
		// 		attack_rate = attack[PLAYER_ATTR_SLOWTIME];
		// 		defence_rate = defence[PLAYER_ATTR_SLOWTIMEDF];
		// 		break;
		// 	case PLAYER_ATTR_MABIDF:
		// 		attack_rate = attack[PLAYER_ATTR_MABITIME];
		// 		defence_rate = defence[PLAYER_ATTR_MABITIMEDF];
		// 		break;
		// 	case PLAYER_ATTR_HURTDF:
		// 		attack_rate = attack[PLAYER_ATTR_HURTTIME];
		// 		defence_rate = defence[PLAYER_ATTR_HURTTIMEDF];
		// 		break;
		// 	case PLAYER_ATTR_CANDF:
		// 		attack_rate = attack[PLAYER_ATTR_CANTIME];
		// 		defence_rate = defence[PLAYER_ATTR_CANTIMEDF];
		// 		break;
		// 	default:
		// 		attack_rate = defence_rate = 0;
		// 		break;
		// }
		// attack_rate = attack_rate / (attack_rate + sg_fight_param_161000289);
		// defence_rate = defence_rate / (defence_rate + sg_fight_param_161000289);		
		// uint32_t time = config->Time;
		// if (1 + attack_rate - defence_rate < sg_fight_param_161000290)
		// 	time = time * sg_fight_param_161000290;
		// else
		// 	time = time * (1 + attack_rate - defence_rate);

		//buff_manager::create_buff(lvconfig->BuffIdEnemy[i], now + time, attack_unit, defence_unit);
		buff_manager::create_default_buff(timeconfig->BuffIdEnemy[i], attack_unit, defence_unit);		

		buff_add[*n_buff_add + n] = timeconfig->BuffIdEnemy[i];
//		buff_add_end_time[*n_buff_add + n] = (now + time) / 1000;
		++n;
//		++(*n_buff_add);
	}
	(*n_buff_add) += n;
}

// // TODO: 阵营神马的
// bool is_friend(unit_struct *attack, unit_struct *defence)
// {
//	if (attack == defence)
//		return true;
//	return false;
// }

static int32_t count_friend_damage(struct SkillTimeTable *timeconfig,
	uint32_t skill_lv, unit_struct *attack_unit, unit_struct *defence_unit)
{
	if (timeconfig->n_EffectIdFriend == 0)
		return (0);
	double *attack = attack_unit->get_all_attr();
	double *defence = defence_unit->get_all_attr();
	double *buff_fight_attack = attack_unit->get_all_buff_fight_attr();
	double *buff_fight_defence = defence_unit->get_all_buff_fight_attr();

	int32_t damage = 0;
	for (size_t i = 0; i < timeconfig->n_EffectIdFriend; ++i)
	{
		struct SkillEffectTable *effectconfig = get_config_by_id(timeconfig->EffectIdFriend[i] + skill_lv - 1, &skill_effect_config);
		if (!effectconfig)
			return (0);
		damage += count_skill_effect(attack, defence, buff_fight_attack, buff_fight_defence, effectconfig, attack_unit);
	}
		//技能效果1=伤害*攻方暴击倍率*（1+攻方伤害加成-守方伤害减免+攻方惩戒-守方豁免）
	// double tmp_rate = 1 + attack[PLAYER_ATTR_DMG_ADD]
	//	- defence[PLAYER_ATTR_DMG_DEF]
	//	+ attack[PLAYER_ATTR_DMG_ADD_PE]
	//	- defence[PLAYER_ATTR_DMG_DEF_PP];
	double tmp_rate = 1;
	damage = damage * tmp_rate;
	if (damage < 0)
		damage = 0;

	defence[PLAYER_ATTR_HP] += damage;
	if (defence[PLAYER_ATTR_HP] > defence[PLAYER_ATTR_MAXHP])
	{
		defence[PLAYER_ATTR_HP] = defence[PLAYER_ATTR_MAXHP];
	}

	return damage;
}

static int32_t count_enemy_damage(struct SkillTable *skillconfig,
	struct SkillTimeTable *timeconfig,
	uint32_t skill_lv,
//	struct SkillLvTable *lvconfig,
	unit_struct *attack_unit,
	unit_struct *defence_unit,
	uint32_t effect,
	int32_t other_rate)
{
	if (timeconfig->n_EffectIdEnemy == 0 || effect == SKILL_EFFECT_MISS)
		return (0);
	double *attack = attack_unit->get_all_attr();
	double *defence = defence_unit->get_all_attr();
	double *buff_fight_attack = attack_unit->get_all_buff_fight_attr();
	double *buff_fight_defence = defence_unit->get_all_buff_fight_attr();
	double crit_attack_rate = 1.0;
	if (effect == SKILL_EFFECT_CRIT)
	{
	// if（攻方会心伤害-守方会心免伤<1)
	//      攻方实际会心伤害=1
	// else
	//      攻方实际会心伤害=攻方会心伤害-守方会心免伤
		crit_attack_rate = (attack[PLAYER_ATTR_CRT_DMG] - defence[PLAYER_ATTR_CRT_DMG_DEF]);// / 10000.0;
		if (crit_attack_rate < 1)
			crit_attack_rate = 1;
	}

	int32_t damage = 0;
	for (size_t i = 0; i < timeconfig->n_EffectIdEnemy; ++i)
	{
		struct SkillEffectTable *effectconfig = get_config_by_id(timeconfig->EffectIdEnemy[i] + skill_lv - 1, &skill_effect_config);
		if (!effectconfig)
			return (0);
		damage += count_skill_effect(attack, defence, buff_fight_attack, buff_fight_defence, effectconfig, attack_unit);
	}
		//技能效果1=伤害*攻方暴击倍率*（1+攻方伤害加成-守方伤害减免+攻方惩戒-守方豁免）
	// double tmp_rate = 1 + attack[PLAYER_ATTR_DMG_ADD]
	//	- defence[PLAYER_ATTR_DMG_DEF]
	//	+ attack[PLAYER_ATTR_DMG_ADD_PE]
	//	- defence[PLAYER_ATTR_DMG_DEF_PP];
	//double tmp_rate = 1;
	damage = damage * crit_attack_rate;// * tmp_rate;
	damage *= ((100 - sg_fight_rand + random() % (sg_fight_rand * 2)) / 100.0);
	if (damage <= 0)
		damage = 1;
	
	damage *= (other_rate / 10000.0);
	if (defence_unit->buff_state & BUFF_STATE_ONEBLOOD)
	{
		if (defence[PLAYER_ATTR_HP] - damage < 1)
		{
			defence[PLAYER_ATTR_HP] = 1;
		}
		else
		{
			defence[PLAYER_ATTR_HP] -= damage;
		}
	}
	else
	{
		defence[PLAYER_ATTR_HP] -= damage;
	}
	return damage;
}

//判断主动技能的被动效果是否被触发
// static bool check_skill_pas_active(struct PassiveSkillTable *pas_config, uint32_t effect)
// {
// 	switch (pas_config->TriggerType)
// 	{
// 		case 0://命中
// 			if (effect != SKILL_EFFECT_MISS)
// 				return true;
// 			return false;
// 		case 1: //暴击
// 			if (effect == SKILL_EFFECT_CRIT)
// 				return true;
// 			return false;
// 		case 2: //闪避
// 			if (effect == SKILL_EFFECT_MISS)
// 				return true;
// 			return false;
// 	}
// 	return false;
// }

static uint32_t get_skill_effect(unit_struct *attack_unit, unit_struct *defence_unit)
{
//	if (lvconfig->n_EffectIdEnemy == 0)
//		return (0);
//	double *attack = attack_unit->get_all_attr();
//	double *defence = defence_unit->get_all_attr();
	double *attack = attack_unit->get_all_buff_fight_attr();
	double *defence = defence_unit->get_all_buff_fight_attr();
	double lv_attack = attack_unit->get_attr(PLAYER_ATTR_LEVEL);
	double lv_defence = defence_unit->get_attr(PLAYER_ATTR_LEVEL);

		//攻方命中几率=攻方命中/(攻方命中+守方等级*命中等级系数+命中基础值)
	double attack_rate = attack[PLAYER_ATTR_HIT] / (attack[PLAYER_ATTR_HIT] + lv_defence * sg_fight_param_161000280 + sg_fight_param_161000281);
		// if（守方闪避<攻方忽略闪避)
		//  守方闪避几率=0
		// else
		//  守方闪避几率=（守方闪避-攻方忽略闪避）/（守方闪避-攻方忽略闪避+攻方等级*闪避等级系数+闪避基础值）
	double defence_rate = 0;
	if (defence[PLAYER_ATTR_DODGE] >= attack[PLAYER_ATTR_DODGEDF])
	{
		defence_rate = (defence[PLAYER_ATTR_DODGE] - attack[PLAYER_ATTR_DODGEDF]) / (defence[PLAYER_ATTR_DODGE] - attack[PLAYER_ATTR_DODGEDF]
			+ lv_attack * sg_fight_param_161000282 + sg_fight_param_161000283);
	}

	// if（攻方命中几率-守方闪避几率<实际命中率下限）
	//      攻方实际命中率=实际命中率下限
	// else
	//      攻方实际命中率=攻方命中几率-守方闪避几率
	if (attack_rate - defence_rate < sg_fight_param_161000284)
	{
		attack_rate = sg_fight_param_161000284;
	}
	else
	{
		attack_rate = attack_rate - defence_rate;
	}
	int randnum = random() % 100;
	if (randnum > (attack_rate) * 100)
	{
		return SKILL_EFFECT_MISS;
	}

	// if（攻方会心几率<守方抗会心几率)
	//      攻方实际会心几率=0
	// else
	//      攻方实际会心几率=（攻方会心几率-守方抗会心几率）/（攻方会心几率-守方抗会心几率+守方等级*会心等级系数+会心基础值）
	if (attack[PLAYER_ATTR_CRIT] <= defence[PLAYER_ATTR_CRIT_DEF])
	{
		return (0);
	}
	double crit_rate = (attack[PLAYER_ATTR_CRIT] - defence[PLAYER_ATTR_CRIT_DEF]) / (attack[PLAYER_ATTR_CRIT] - defence[PLAYER_ATTR_CRIT_DEF]
		+ lv_defence * sg_fight_param_161000285 + sg_fight_param_161000286);
	randnum = random() % 100;
	if (randnum < (crit_rate) * 100)
	{
		return SKILL_EFFECT_CRIT;
	}



//		//闪避率
//	double dodge_rate = defence[PLAYER_ATTR_DODGE] / (lv_defence * 500 + 2500) + 0.01;
//	if (dodge_rate > 0.2)
//		dodge_rate = 0.2;

//		//命中率
//	double hit_rate = attack[PLAYER_ATTR_HIT] / (lv_attack * 500 + 2500) + 0.8;
//	if (hit_rate > 1)
//		hit_rate = 1;

//	if (dodge_rate > hit_rate)
//	{
//		return SKILL_EFFECT_MISS;
//	}
//	int randnum = random() % 100;
//	if (randnum > (hit_rate - dodge_rate) * 100)
//	{
//		return SKILL_EFFECT_MISS;
//	}

//	if (attack_unit->buff_state & BUFF_STATE_CRIT)
//	{
//		attack_unit->delete_state_buff(BUFF_STATE_CRIT);
//		return SKILL_EFFECT_CRIT;
//	}

// //	double crit_attack_rate = 1.0;
//		//抗暴击率
//	double crit_def_rate = defence[PLAYER_ATTR_CRIT_DEF] / (lv_defence * 500 + 2500) + 0.01;
//	if (crit_def_rate > 0.2)
//		crit_def_rate = 0.2;
//		//暴击率
//	double crit_rate = attack[PLAYER_ATTR_CRIT] / (lv_attack * 500 + 2500) + 0.2;
//	if (crit_rate > 0.6)
//		crit_rate = 0.6;
//	if (crit_rate > crit_def_rate)
//	{
//		randnum = random() % 100;
//		if (randnum < (crit_rate - crit_def_rate) * 100)
//		{
//			return SKILL_EFFECT_CRIT;
// //			crit_attack_rate = (attack[PLAYER_ATTR_CRT_DMG] - defence[PLAYER_ATTR_CRT_DMG_DEF]) / 10000.0;
//		}
// //		if (crit_attack_rate < 1)
// //			crit_attack_rate = 1;
//	}
	return (0);
}

int32_t count_other_skill_damage_effect(unit_struct *attack, unit_struct *defence)
{
	int rate = 10000;
	if (attack->get_unit_type() == UNIT_TYPE_PLAYER)
	{
		if (defence->get_unit_type() == UNIT_TYPE_PLAYER)
		{
			player_struct *p1, *p2;
			p1 = (player_struct *)attack;
			p2 = (player_struct *)defence;
			
				//玩家打玩家, 算PVP部分数值
          // PVP伤害加成=1+（攻方穿刺-守方霸体）/（攻方穿刺-守方霸体+守方等级*PVP等级系数+PVP基础值）				
				
          // if（PVP伤害加成<PVP保底比例）				
          //         PVP伤害加成=PVP保底比例				
          // else				
          //         PVP伤害加成=PVP伤害加成
			double t = attack->get_attr(PLAYER_ATTR_PVPAT) - defence->get_attr(PLAYER_ATTR_PVPDF);
			if (t < 0)
				t = 1.0;
			else
				t = 1 + t / (t + defence->get_attr(PLAYER_ATTR_LEVEL) * sg_fight_param_161000291 + sg_fight_param_161000292);
			if (t > 1 + sg_fight_param_161000293)
				t = 1 + sg_fight_param_161000293;
			rate *= t; 
			
				//玩家打玩家，算悬赏
			rate += ChengJieTaskManage::ChengjieAddHurt(*p1, *p2);
			rate -= ChengJieTaskManage::ChengjieRedeuceHurt(*p1, *p2);

				//战斗力伤害加成
			// 技能效果之攻击=（攻方属性值*技能百分比/10000+技能值）*攻方实际会心伤害*PVP伤害加成*战斗力伤害加成
			// 技能效果之五行伤害=（攻方五行伤害*技能百分比/10000+技能值）*（1- 守方五行抗性减免）*攻方实际会心伤害*PVP伤害加成*战斗力伤害加成


			// 战斗力伤害加成公式：
			// 战斗力伤害加成 = 1 + MAX（MIN（INT(（攻方战斗力 - 守方战斗力）/ 守方战斗力*100)，A1），A2）* IF（攻方战斗力 > 守方战斗力，B1，B2）
			// A1 = 50       → 参数数据表（ParameterTable），ID：161000393
			// A2 = -50      →  161000394
			// B1 = 0.002   →  161000395
			// B2 = 0.003   →  161000396
			double attack_fight = attack->get_attr(PLAYER_ATTR_FIGHTING_CAPACITY);
			double defence_fight = defence->get_attr(PLAYER_ATTR_FIGHTING_CAPACITY);
			int t1 = (attack_fight - defence_fight ) / defence_fight * 100;
			if (t1 > sg_fight_param_161000393)
				t1 = sg_fight_param_161000393;
			else if (t1 < sg_fight_param_161000394)
				t1 = sg_fight_param_161000394;
			double t2;
			if (attack_fight > defence_fight)
				t2 = 1 + t1 * sg_fight_param_161000395;
			else
				t2 = 1 + t1 * sg_fight_param_161000396;
			rate *= t2;
		}
		else
		{
				//玩家打怪物，算国御
			player_struct *p1 = (player_struct *)attack;
			monster_struct *p2 = (monster_struct *)defence;
			rate += ChengJieTaskManage::GuoyuAddHurt(*p1, *p2);
		}
	}
	else if (defence->get_unit_type() == UNIT_TYPE_PLAYER)
	{
			//怪物打玩家，算国御
			monster_struct *p1 = (monster_struct *)attack;
			player_struct *p2 = (player_struct *)defence;
			rate -= ChengJieTaskManage::GuoyuRedeuceHurt(*p1, *p2);
	}
	return (rate);
}


int32_t count_skill_total_damage(UNIT_FIGHT_TYPE type, struct SkillTable *skillconfig, uint32_t skill_lv, unit_struct *attack_unit,
	unit_struct *defence_unit, uint32_t *effect, uint32_t buff_add[], uint32_t buff_end_time[], uint32_t *n_buff_add, int32_t other_rate)
{
//	uint32_t skill_id = attack->data->skill.skill_id;
//	if (skill_id == 0)
//		return 0;
	int32_t ret = 0;
//	bool pas_active = false; //被动技能是否触发

	if (!skillconfig)
		return (0);

//	bool b_is_friend = is_friend(attack_unit, defence_unit);

	double *defence_attr = defence_unit->get_all_attr();
	double defence_cur_hp = defence_attr[PLAYER_ATTR_HP];

	if (type == UNIT_FIGHT_TYPE_ENEMY)
	{
		*effect = get_skill_effect(attack_unit, defence_unit);
//		struct SkillTable *skillconfig = get_config_by_id(skill_id, &skill_config);
//		if (!skillconfig)
//			return (0);
		for (uint32_t i = 0; i < skillconfig->n_time_config; ++i)
		{
			ret += count_enemy_damage(skillconfig, skillconfig->time_config[i], skill_lv, attack_unit, defence_unit, *effect, other_rate);
		}
		// if (pas_config && pas_lvconfig)
		// {
		// 	pas_active = check_skill_pas_active(pas_config, *effect);
		// }
		// if (pas_active)
		// {
		// 	ret += count_enemy_damage(skillconfig, pas_lvconfig, attack_unit, defence_unit, 0, other_rate);
		// }
	}
	else
	{
		*effect = SKILL_EFFECT_ADDHP;
		for (uint32_t i = 0; i < skillconfig->n_time_config; ++i)
		{
			ret += count_friend_damage(skillconfig->time_config[i], skill_lv, attack_unit, defence_unit);
		}
		if (ret == 0)
			*effect = 0;
	}

	//世界boss数据处理
	if(defence_unit->get_unit_type() == UNIT_TYPE_MONSTER || defence_unit->get_unit_type() == UNIT_TYPE_BOSS)
	{
		((monster_struct*)defence_unit)->monster_suffer_damage(attack_unit, defence_cur_hp, ret);
	}
	if (!defence_unit->is_alive())
		return ret;

	if (type == UNIT_FIGHT_TYPE_ENEMY)
	{
		for (uint32_t i = 0; i < skillconfig->n_time_config; ++i)		
			count_enemy_buff(skillconfig->time_config[i], skill_lv, attack_unit, defence_unit, buff_add, buff_end_time, n_buff_add);
//		if (pas_active)
//			count_enemy_buff(pas_lvconfig, attack_unit, defence_unit, buff_add, buff_end_time, n_buff_add);
	}
	else
	{
		for (uint32_t i = 0; i < skillconfig->n_time_config; ++i)				
			count_friend_buff(skillconfig->time_config[i], skill_lv, attack_unit, defence_unit, buff_add, buff_end_time, n_buff_add);
	}

//	count_buff_add(attack, defence, buff_add, n_buff_add ,lvconfig);
//		bugff_manager::add_skill_buff(player, target, add_num, &cached_buff_id[n_buff]);

	return ret;
}
