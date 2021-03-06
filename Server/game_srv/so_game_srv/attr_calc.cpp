#ifndef ATTR_CALC_H
#define ATTR_CALC_H

#include "game_event.h"
#include "game_config.h"
#include "attr_id.h"
#include "attr_calc.h"
#include <math.h>

void get_attr_from_config(uint64_t attr_table_id, double *attr)
{
	ActorAttributeTable* config = get_config_by_id(attr_table_id, &actor_attribute_config);
	if (!config)
	{
		attr[PLAYER_ATTR_MAXHP] = 1;
		LOG_ERR("[%s:%d] get actor attribute config failed, id: %lu", __FUNCTION__, __LINE__, attr_table_id);
		return;
	}

	attr[PLAYER_ATTR_MAXHP] = config->Health;
//	attr[PLAYER_ATTR_HP] = config->Health;	
	attr[PLAYER_ATTR_ATTACK] = config->Attack;
//	attr[PLAYER_ATTR_DEFENSE] = config->Defense;
	attr[PLAYER_ATTR_ATK_METAL] = config->AtGold;
	attr[PLAYER_ATTR_ATK_WOOD] = config->AtWood;
	attr[PLAYER_ATTR_ATK_WATER] = config->AtWater;
	attr[PLAYER_ATTR_ATK_FIRE] = config->AtFire;
	attr[PLAYER_ATTR_ATK_EARTH] = config->AtEarth;
	attr[PLAYER_ATTR_DEF_METAL] = config->AtGoldDf;
	attr[PLAYER_ATTR_DEF_WOOD] = config->AtWoodDf;
	attr[PLAYER_ATTR_DEF_WATER] = config->AtWaterDf;
	attr[PLAYER_ATTR_DEF_FIRE] = config->AtFireDf;
	attr[PLAYER_ATTR_DEF_EARTH] = config->AtEarthDf;
	attr[PLAYER_ATTR_DODGE] = config->Dodge;
	attr[PLAYER_ATTR_HIT] = config->Hit;
	attr[PLAYER_ATTR_CRIT] = config->Critical;
	attr[PLAYER_ATTR_CRIT_DEF] = config->CriticalDf;
	attr[PLAYER_ATTR_CRT_DMG] = config->CtDmg;
	attr[PLAYER_ATTR_CRT_DMG_DEF] = config->CtDmgDf;
//	attr[PLAYER_ATTR_MOVE_SPEED] = config->MoveSpeed;
	attr[PLAYER_ATTR_DFWUDEL] = config->DfWuDel;
	attr[PLAYER_ATTR_DODGEDF] = config->DodgeDf;
	attr[PLAYER_ATTR_DIZZY] = config->Dizzy;
	attr[PLAYER_ATTR_SLOW] = config->Slow;
	attr[PLAYER_ATTR_MABI] = config->Mabi;
	attr[PLAYER_ATTR_HURT] = config->Hurt;
	attr[PLAYER_ATTR_CAN] = config->Can;
	attr[PLAYER_ATTR_DIZZYDF] = config->DizzyDf;
	attr[PLAYER_ATTR_SLOWDF] = config->SlowDf;
	attr[PLAYER_ATTR_MABIDF] = config->MabiDf;
	attr[PLAYER_ATTR_HURTDF] = config->HurtDf;
	attr[PLAYER_ATTR_CANDF] = config->CanDf;
	attr[PLAYER_ATTR_PVPAT] = config->PVPAt;
	attr[PLAYER_ATTR_PVPDF] = config->PVPDf;
	// attr[PLAYER_ATTR_HP_RECOVER] = config->Recover;
	// attr[PLAYER_ATTR_CURE] = config->Cure;
	// attr[PLAYER_ATTR_CURE_ADD] = config->CureAdd;
	// attr[PLAYER_ATTR_DMG_ADD] = config->DmgAdd;
	// attr[PLAYER_ATTR_DMG_DEF] = config->DmgDf;
	// attr[PLAYER_ATTR_DMG_ADD_PE] = config->DmgAddPE;
	// attr[PLAYER_ATTR_DMG_DEF_PP] = config->DmgDfPP;
	// attr[PLAYER_ATTR_STUN] = config->Dizzy;
	attr[PLAYER_ATTR_SLOW] = config->Slow;
	// attr[PLAYER_ATTR_CHAOS] = config->Chaos;
	// attr[PLAYER_ATTR_POISON] = config->Poison;
	// attr[PLAYER_ATTR_VAMPIRE] = config->Vampire;
	// attr[PLAYER_ATTR_BOUNCE] = config->Bounce;
	// attr[PLAYER_ATTR_HIT_REPEL] = config->HitRepel;
	// attr[PLAYER_ATTR_HIT_FLY] = config->Hitfly;
	// attr[PLAYER_ATTR_STUN_DEF] = config->DizzyDf;
	// attr[PLAYER_ATTR_SLOW_DEF] = config->SlowDf;
	// attr[PLAYER_ATTR_CHAOS_DEF] = config->ChaosDf;
	// attr[PLAYER_ATTR_POISON_DEF] = config->PoisonDf;
	// attr[PLAYER_ATTR_VAMPIRE_DEF] = config->VampireDf;
	// attr[PLAYER_ATTR_BOUNCE_DEF] = config->BounceDf;
	// attr[PLAYER_ATTR_HIT_REPEL_DEF] = config->HitRepelDf;
	// attr[PLAYER_ATTR_HIT_FLY_DEF] = config->HitflyDf;
}

void add_fight_attr(double *dest_attr, double *src_attr)
{
	for (int i = 1; i < PLAYER_ATTR_FIGHT_MAX; ++i)
	{
		dest_attr[i] += src_attr[i];
	}
	for (int i = PLAYER_ATTR_TI; i <= PLAYER_ATTR_ALLEFFDF; ++i)
	{
		dest_attr[i] += src_attr[i];		
	}
}

uint32_t calculate_fighting_capacity(double *attr, bool total)
{
	double t = 0;
	double tmp = 0.0;
	for (int i = 1; i < PLAYER_ATTR_FIGHT_MAX; ++i)
	{
		if (total && sg_fighting_capacity_count_in[i] == 0)
		{
			continue;
		}
		tmp = attr[i];
		if (i == PLAYER_ATTR_CRT_DMG)
		{
			tmp -= sg_fighting_capacity_crt_dmg_init_val;
			if (tmp < 0.0)
			{
				tmp = 0.0;
			}
		}
		t += tmp * sg_fighting_capacity_coefficient[i];
	}
	for (int i = PLAYER_ATTR_TI; i <= PLAYER_ATTR_ALLEFFDF; ++i)
	{
		if (total && sg_fighting_capacity_count_in[i] == 0)
		{
			continue;
		}
		t += attr[i] * sg_fighting_capacity_coefficient[i];
	}

	return ceil(t);
}

void calculate_lv3_attribute(double *attrData)
{
	// 面板生命=面板体质*（1+最终基础生命加成）*体质转生命系数+角色等级生命+其他系统附加属性
	attrData[PLAYER_ATTR_MAXHP] += attrData[PLAYER_ATTR_TI] * (1 + attrData[PLAYER_ATTR_HEALTHPRO]) * sg_fight_param_161000274;
	// 面板攻击=面板力量*（1+最终基础攻击加成）*力量转攻击系数+角色等级攻击+其他系统附加属性
	attrData[PLAYER_ATTR_ATTACK] += attrData[PLAYER_ATTR_LI] * (1 + attrData[PLAYER_ATTR_ATTACKPRO]) * sg_fight_param_161000275;
	// 最终全系抗性=面板敏捷*敏捷转全系抗性系数+其他系统附加属性
	attrData[PLAYER_ATTR_DFWU] += attrData[PLAYER_ATTR_MIN] * sg_fight_param_161000277;
		//五行攻击=力量*力量转五行攻击系数+角色五行攻击
	attrData[PLAYER_ATTR_WUXINGGONGJI] += attrData[PLAYER_ATTR_LI] * sg_fight_param_161000503;
	// 面板忽略全抗=角色忽略全抗+其他系统附加属性
	// 最终金系伤害=角色金系伤害+其他系统附加属性	
	// 最终木系伤害=角色木系伤害+其他系统附加属性	
	// 最终水系伤害=角色水系伤害+其他系统附加属性	
	// 最终火系伤害=角色火系伤害+其他系统附加属性	
	// 最终土系伤害=角色土系伤害+其他系统附加属性	
	// 面板命中=面板灵巧*灵巧转命中系数+角色命中+其他系统附加属性
	attrData[PLAYER_ATTR_HIT] += attrData[PLAYER_ATTR_LING] * sg_fight_param_161000278;
	// 面板闪避=面板敏捷*敏捷转闪避系数+角色闪避+其他系统附加属性
	attrData[PLAYER_ATTR_DODGE] += attrData[PLAYER_ATTR_MIN] * sg_fight_param_161000276;
	// 面板会心几率=面板灵巧*灵巧转会心几率系数+角色会心几率+其他系统附加属性
	attrData[PLAYER_ATTR_CRIT] += attrData[PLAYER_ATTR_LING] * sg_fight_param_161000279;
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

#endif /* ATTR_CALC_H */
