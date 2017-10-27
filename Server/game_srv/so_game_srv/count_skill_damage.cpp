#include <stdio.h>
#include <string.h>
#include <map>
#include <stdlib.h>
#include "count_skill_damage.h"
#include "attr_id.h"
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
		case PLAYER_ATTR_ATK_METAL: //金
			at = buff_fight_attack[PLAYER_ATTR_ATK_METAL] * effect_add / 10000
				+ effect_num;
			de = buff_fight_defence[PLAYER_ATTR_DEF_METAL];
			ret = at - de;
			break;
		case PLAYER_ATTR_ATK_WOOD:
			at = buff_fight_attack[PLAYER_ATTR_ATK_WOOD] * effect_add / 10000
				+ effect_num;
			de = buff_fight_defence[PLAYER_ATTR_DEF_WOOD];
			ret = at - de;
			break;
		case PLAYER_ATTR_ATK_WATER:
			at = buff_fight_attack[PLAYER_ATTR_ATK_WATER] * effect_add / 10000
				+ effect_num;
			de = buff_fight_defence[PLAYER_ATTR_DEF_WATER];
			ret = at - de;
			break;
		case PLAYER_ATTR_ATK_FIRE: //火
			at = buff_fight_attack[PLAYER_ATTR_ATK_FIRE] * effect_add / 10000
				+ effect_num;
			de = buff_fight_defence[PLAYER_ATTR_DEF_FIRE];
			ret = at - de;
			break;
		case PLAYER_ATTR_ATK_EARTH:
			at = buff_fight_attack[PLAYER_ATTR_ATK_EARTH] * effect_add / 10000
				+ effect_num;
			de = buff_fight_defence[PLAYER_ATTR_DEF_EARTH];
			ret = at - de;
			break;
	}
	if (ret < 0)
		ret = 0;
	return (ret);
}

int32_t count_skill_effect(const double *attack, const double *defence,
	const double *buff_fight_attack, const double *buff_fight_defence,
	struct SkillEffectTable *effectconfig)
{
	int32_t ret = 0;
	for (size_t i = 0; i < effectconfig->n_Effect; ++i)
	{
		ret += count_skill_effect_entry(attack, defence, buff_fight_attack, buff_fight_defence,
			effectconfig->Effect[i], effectconfig->EffectAdd[i], effectconfig->EffectNum[i]);
	}
	return ret;
}

static bool check_can_add_buff(unit_struct *unit, uint32_t buff_id)
{
	uint64_t type = buff_manager::get_buff_first_effect_type(buff_id);
	if (unit->is_in_lock_time() && buff_manager::is_move_buff_effect(type))
		return false;
	return true;
}

static void count_friend_buff(struct SkillLvTable *lvconfig,
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

	for (size_t i = 0; i < lvconfig->n_BuffIdFriend; ++i)
	{
		if (!check_can_add_buff(defence_unit, lvconfig->BuffIdFriend[i]))
			continue;

		config = get_config_by_id(lvconfig->BuffIdFriend[i], &buff_config);
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
		buff_manager::create_default_buff(lvconfig->BuffIdFriend[i], attack_unit, defence_unit);
		
		buff_add[*n_buff_add + n] = lvconfig->BuffIdFriend[i];
//		buff_add_end_time[*n_buff_add + n] = (now + time) / 1000;		
		++n;
	}
//	buff_manager::add_skill_buff(attack_unit, defence_unit, n, &buff_add[*n_buff_add], &buff_add_end_time[*n_buff_add]);
	(*n_buff_add) += n;
}
static void count_enemy_buff(struct SkillLvTable *lvconfig,
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

	for (size_t i = 0; i < lvconfig->n_BuffIdEnemy; ++i)
	{
		if (!check_can_add_buff(defence_unit, lvconfig->BuffIdEnemy[i]))
			continue;

		config = get_config_by_id(lvconfig->BuffIdEnemy[i], &buff_config);
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
		buff_manager::create_default_buff(lvconfig->BuffIdEnemy[i], attack_unit, defence_unit);		

		buff_add[*n_buff_add + n] = lvconfig->BuffIdEnemy[i];
//		buff_add_end_time[*n_buff_add + n] = (now + time) / 1000;
		++n;
//		++(*n_buff_add);
	}
	(*n_buff_add) += n;
}

void get_skill_configs(uint32_t skill_lv, uint32_t skill_id, struct SkillTable **ski_config, struct SkillLvTable **lv_config1, struct PassiveSkillTable **pas_config, struct SkillLvTable **lv_config2, struct ActiveSkillTable **act_config)
{
	*lv_config1 = NULL;
	*lv_config2 = NULL;
	*pas_config = NULL;
	*ski_config = get_config_by_id(skill_id, &skill_config);
	if (!ski_config)
		return;

	if (act_config)
	{
		*act_config = get_config_by_id((*ski_config)->SkillAffectId, &active_skill_config);
	}

	std::map<uint64_t, struct SkillLvTable *>::iterator iter = skill_lv_config.find((*ski_config)->SkillLv + skill_lv - 1);
	if (iter != skill_lv_config.end())
		*lv_config1 = iter->second;

	if ((*ski_config)->PassiveID)
	{
		*pas_config = get_config_by_id((uint32_t)((*ski_config)->PassiveID), &passive_skill_config);
		if (!pas_config)
			return;
		iter = skill_lv_config.find((*ski_config)->PassiveLv + skill_lv - 1);
		if (iter != skill_lv_config.end())
			*lv_config2 = iter->second;
	}
}

// // TODO: 阵营神马的
// bool is_friend(unit_struct *attack, unit_struct *defence)
// {
//	if (attack == defence)
//		return true;
//	return false;
// }

static int32_t count_friend_damage(struct SkillLvTable *lvconfig,
	unit_struct *attack_unit, unit_struct *defence_unit)
{
	if (lvconfig->n_EffectIdFriend == 0)
		return (0);
	double *attack = attack_unit->get_all_attr();
	double *defence = defence_unit->get_all_attr();
	double *buff_fight_attack = attack_unit->get_all_buff_fight_attr();
	double *buff_fight_defence = defence_unit->get_all_buff_fight_attr();

	int32_t damage = 0;
	for (size_t i = 0; i < lvconfig->n_EffectIdFriend; ++i)
	{
		struct SkillEffectTable *effectconfig = get_config_by_id(lvconfig->EffectIdFriend[i], &skill_effect_config);
		if (!effectconfig)
			return (0);
		damage += count_skill_effect(attack, defence, buff_fight_attack, buff_fight_defence, effectconfig);
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

static int32_t count_enemy_damage(struct SkillTable *skillconfig,
	struct SkillLvTable *lvconfig,
	unit_struct *attack_unit,
	unit_struct *defence_unit,
	uint32_t effect,
	int32_t other_rate)
{
	if (lvconfig->n_EffectIdEnemy == 0 || effect == SKILL_EFFECT_MISS)
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
	for (size_t i = 0; i < lvconfig->n_EffectIdEnemy; ++i)
	{
		struct SkillEffectTable *effectconfig = get_config_by_id(lvconfig->EffectIdEnemy[i], &skill_effect_config);
		if (!effectconfig)
			return (0);
		damage += count_skill_effect(attack, defence, buff_fight_attack, buff_fight_defence, effectconfig);
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
static bool check_skill_pas_active(struct PassiveSkillTable *pas_config, uint32_t effect)
{
	switch (pas_config->TriggerType)
	{
		case 0://命中
			if (effect != SKILL_EFFECT_MISS)
				return true;
			return false;
		case 1: //暴击
			if (effect == SKILL_EFFECT_CRIT)
				return true;
			return false;
		case 2: //闪避
			if (effect == SKILL_EFFECT_MISS)
				return true;
			return false;
	}
	return false;
}

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

		//攻方命中几率=攻方命中/(攻方命中+攻方等级*命中等级系数+命中基础值)
	double attack_rate = attack[PLAYER_ATTR_HIT] / (attack[PLAYER_ATTR_HIT] + lv_attack * sg_fight_param_161000280 + sg_fight_param_161000281);
		// if（守方闪避<攻方忽略闪避)
		//  守方闪避几率=0
		// else
		//  守方闪避几率=（守方闪避-攻方忽略闪避）/（守方闪避-攻方忽略闪避+守方等级*闪避等级系数+闪避基础值）
	double defence_rate = 0;
	if (defence[PLAYER_ATTR_DODGE] >= attack[PLAYER_ATTR_DODGEDF])
	{
		defence_rate = (defence[PLAYER_ATTR_DODGE] - attack[PLAYER_ATTR_DODGEDF]) / (defence[PLAYER_ATTR_DODGE] - attack[PLAYER_ATTR_DODGEDF]
			+ lv_defence * sg_fight_param_161000282 + sg_fight_param_161000283);
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
	//      攻方实际会心几率=（攻方会心几率-守方抗会心几率）/（攻方会心几率-守方抗会心几率+攻方等级*会心等级系数+会心基础值）
	if (attack[PLAYER_ATTR_CRIT] <= defence[PLAYER_ATTR_CRIT_DEF])
	{
		return (0);
	}
	double crit_rate = (attack[PLAYER_ATTR_CRIT] - defence[PLAYER_ATTR_CRIT_DEF]) / (attack[PLAYER_ATTR_CRIT] - defence[PLAYER_ATTR_CRIT_DEF]
		+ lv_attack * sg_fight_param_161000285 + sg_fight_param_161000286);
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
          // PVP伤害加成=1+（攻方穿刺-守方霸体）/（攻方穿刺-守方霸体+攻方等级*PVP等级系数+PVP基础值）				
				
          // if（PVP伤害加成<PVP保底比例）				
          //         PVP伤害加成=PVP保底比例				
          // else				
          //         PVP伤害加成=PVP伤害加成
			double t = attack->get_attr(PLAYER_ATTR_PVPAT) - defence->get_attr(PLAYER_ATTR_PVPDF);
			t = 1 + t / (t + attack->get_attr(PLAYER_ATTR_LEVEL) * sg_fight_param_161000291 + sg_fight_param_161000292);
			if (t < sg_fight_param_161000293)
				t = sg_fight_param_161000293;
			rate *= t; 
			
				//玩家打玩家，算悬赏
			rate += ChengJieTaskManage::ChengjieAddHurt(*p1, *p2);
			rate -= ChengJieTaskManage::ChengjieRedeuceHurt(*p1, *p2);
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


int32_t count_skill_total_damage(UNIT_FIGHT_TYPE type, struct SkillTable *skillconfig, struct SkillLvTable *act_lvconfig,
	struct PassiveSkillTable *pas_config, struct SkillLvTable *pas_lvconfig, unit_struct *attack_unit,
	unit_struct *defence_unit, uint32_t *effect, uint32_t buff_add[], uint32_t buff_end_time[], uint32_t *n_buff_add, int32_t other_rate)
{
//	uint32_t skill_id = attack->data->skill.skill_id;
//	if (skill_id == 0)
//		return 0;
	int32_t ret;
	bool pas_active = false; //被动技能是否触发

	if (!skillconfig || !act_lvconfig)
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
		ret = count_enemy_damage(skillconfig, act_lvconfig, attack_unit, defence_unit, *effect, other_rate);
		if (pas_config && pas_lvconfig)
		{
			pas_active = check_skill_pas_active(pas_config, *effect);
		}
		if (pas_active)
		{
			ret += count_enemy_damage(skillconfig, pas_lvconfig, attack_unit, defence_unit, 0, other_rate);
		}
	}
	else
	{
		ret = count_friend_damage(act_lvconfig, attack_unit, defence_unit);
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
		count_enemy_buff(act_lvconfig, attack_unit, defence_unit, buff_add, buff_end_time, n_buff_add);
		if (pas_active)
			count_enemy_buff(pas_lvconfig, attack_unit, defence_unit, buff_add, buff_end_time, n_buff_add);
	}
	else
	{
		count_friend_buff(act_lvconfig, attack_unit, defence_unit, buff_add, buff_end_time, n_buff_add);
	}

//	count_buff_add(attack, defence, buff_add, n_buff_add ,lvconfig);
//		bugff_manager::add_skill_buff(player, target, add_num, &cached_buff_id[n_buff]);

	return ret;
}
