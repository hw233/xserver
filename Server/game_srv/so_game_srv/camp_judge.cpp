#include "camp_judge.h"
#include "monster.h"
#include "player.h"

// 是否有无敌
// 怪物是否可攻击
// 阵营ID
// 队友, 宠物
// 攻击模式，和平，杀戮，阵营

//pk模式
// | 攻击/受击 | 和平 | 阵营 | 杀戮 | 怪物 | 怪物2 | 怪物3 |
// |-----------+------+------+------+------+-------+-------|
// | 和平      | 中立 | 中立 | 敌对 | 敌对 | 中立  | 中立  |
// | 阵营      | 中立 | 阵营 | 敌对 | 敌对 | 阵营  | 中立  |
// | 杀戮      | 敌对 | 敌对 | 敌对 | 敌对 | 中立  | 中立  |
// | 怪物      | 敌对 | 敌对 | 敌对 | 友好 | 中立  | 敌对  |
// | 怪物2     | 中立 | 阵营 | 中立 | 中立 | 阵营  | 中立  |
// | 怪物3     | 中立 | 中立 | 中立 | 中立 | 中立  | 中立  |

//阵营关系
// | 攻击/受击 | 无   | 人   | 妖   |   |
// |-----------+------+------+------+---|
// | 无        | 中立 | 中立 | 中立 |   |
// | 人        | 中立 | 中立 | 敌对 |   |
// | 妖        | 中立 | 敌对 | 中立 |   |

// #define MAX_PK_TYPE 6
// static UNIT_FIGHT_TYPE pk_type_to_fight_type[MAX_PK_TYPE][MAX_PK_TYPE] = 
// {
// 	{UNIT_FIGHT_TYPE_NONE, UNIT_FIGHT_TYPE_NONE, UNIT_FIGHT_TYPE_ENEMY, UNIT_FIGHT_TYPE_ENEMY, UNIT_FIGHT_TYPE_NONE, UNIT_FIGHT_TYPE_NONE},
// 	{UNIT_FIGHT_TYPE_NONE, UNIT_FIGHT_TYPE_ZHENYING, UNIT_FIGHT_TYPE_ENEMY, UNIT_FIGHT_TYPE_ENEMY, UNIT_FIGHT_TYPE_ZHENYING, UNIT_FIGHT_TYPE_NONE},		
// 	{UNIT_FIGHT_TYPE_ENEMY, UNIT_FIGHT_TYPE_ENEMY, UNIT_FIGHT_TYPE_ENEMY, UNIT_FIGHT_TYPE_ENEMY, UNIT_FIGHT_TYPE_NONE, UNIT_FIGHT_TYPE_NONE},
// 	{UNIT_FIGHT_TYPE_ENEMY, UNIT_FIGHT_TYPE_ENEMY, UNIT_FIGHT_TYPE_ENEMY, UNIT_FIGHT_TYPE_FRIEND, UNIT_FIGHT_TYPE_NONE, UNIT_FIGHT_TYPE_ENEMY},
// 	{UNIT_FIGHT_TYPE_NONE, UNIT_FIGHT_TYPE_ZHENYING, UNIT_FIGHT_TYPE_NONE, UNIT_FIGHT_TYPE_NONE, UNIT_FIGHT_TYPE_ZHENYING, UNIT_FIGHT_TYPE_NONE},
// 	{UNIT_FIGHT_TYPE_NONE, UNIT_FIGHT_TYPE_NONE, UNIT_FIGHT_TYPE_NONE, UNIT_FIGHT_TYPE_NONE, UNIT_FIGHT_TYPE_NONE, UNIT_FIGHT_TYPE_NONE}, 
// };

#define MAX_ZHENYING_TYPE 3
static UNIT_FIGHT_TYPE zhenying_type_to_fight_type[MAX_ZHENYING_TYPE][MAX_ZHENYING_TYPE] =
{
	{UNIT_FIGHT_TYPE_NONE, UNIT_FIGHT_TYPE_NONE, UNIT_FIGHT_TYPE_NONE},
	{UNIT_FIGHT_TYPE_NONE, UNIT_FIGHT_TYPE_NONE, UNIT_FIGHT_TYPE_ENEMY},
	{UNIT_FIGHT_TYPE_NONE, UNIT_FIGHT_TYPE_ENEMY, UNIT_FIGHT_TYPE_NONE},	
};

static UNIT_FIGHT_TYPE adjust_unit_fight_type(unit_struct *attack, unit_struct *defence, UNIT_FIGHT_TYPE type)
{
		//如果玩家在安全区，那么敌对变成中立
	if (type == UNIT_FIGHT_TYPE_ENEMY)
	{
		if (defence->is_in_safe_region())
			return UNIT_FIGHT_TYPE_NONE;
	}

		//如果是阵营模式，看阵营关系
	if (type == UNIT_FIGHT_TYPE_ZHENYING)
	{
		int zy1 = attack->get_attr(PLAYER_ATTR_ZHENYING);
		int zy2 = defence->get_attr(PLAYER_ATTR_ZHENYING);
		assert(zy1 < MAX_ZHENYING_TYPE && zy2 < MAX_ZHENYING_TYPE);
		return zhenying_type_to_fight_type[zy1][zy2];
	}
	return type;
}

UNIT_FIGHT_TYPE get_unit_fight_type(unit_struct *attack, unit_struct *defence)
{
	if (attack == defence)
		return UNIT_FIGHT_TYPE_MYSELF;

	if (!defence->is_alive())
		return UNIT_FIGHT_TYPE_NONE;
	
		//无敌buff
		//被设置成不能攻击的怪物
	if (!defence->can_beattack())
		return adjust_unit_fight_type(attack, defence, UNIT_FIGHT_TYPE_NONE);

		//副本中的阵营关系，比如PVP副本
	int camp1 = attack->get_camp_id();
	int camp2 = defence->get_camp_id();
	if (camp1 != 0 && camp2 != 0)
	{
		if (camp1 == camp2)
			return adjust_unit_fight_type(attack, defence, UNIT_FIGHT_TYPE_FRIEND);
		else
			return adjust_unit_fight_type(attack, defence, UNIT_FIGHT_TYPE_ENEMY);
	}

	if (defence->get_unit_type() == UNIT_TYPE_PLAYER && attack->get_unit_type() == UNIT_TYPE_PLAYER)
	{
			//小于pk设置的等级，不能被玩家攻击
		if (defence->get_attr(PLAYER_ATTR_LEVEL) < sg_pk_level)
			return UNIT_FIGHT_TYPE_NONE;
		
			// 切磋		
		if (((player_struct *)attack)->is_qiecuo_target((player_struct *)(defence)))
			return adjust_unit_fight_type(attack, defence, UNIT_FIGHT_TYPE_ENEMY);

			//队友
		Team *team1, *team2;
		team1 = attack->get_team();
		team2 = defence->get_team();
		if (team1 && team1 == team2)
			return adjust_unit_fight_type(attack, defence, UNIT_FIGHT_TYPE_FRIEND);

			//悬赏目标
		if (((player_struct *)attack)->is_chengjie_target(defence->get_uuid()))
			return adjust_unit_fight_type(attack, defence, UNIT_FIGHT_TYPE_ENEMY);
		if (((player_struct *)defence)->is_chengjie_target(attack->get_uuid()))
			return adjust_unit_fight_type(attack, defence, UNIT_FIGHT_TYPE_ENEMY);

			//帮会
		if (((player_struct *)attack)->is_in_same_guild(((player_struct *)defence)))
		{
			return adjust_unit_fight_type(attack, defence, UNIT_FIGHT_TYPE_FRIEND);			
		}
	}

		//攻击模式
	int type1 = attack->get_attr(PLAYER_ATTR_PK_TYPE);
	int type2 = defence->get_attr(PLAYER_ATTR_PK_TYPE);
	assert(type1 < MAX_PK_TYPE && type2 < MAX_PK_TYPE);
	return adjust_unit_fight_type(attack, defence, pk_type_to_fight_type[type1][type2]);
}

// bool check_can_attack(unit_struct *attack, unit_struct *defence)
// {
// 	if (!defence->is_alive())
// 		return false;

// 	if (attack->scene != defence->scene)
// 		return false;
	
// 	if (defence->get_unit_type() != UNIT_TYPE_PLAYER)
// 	{
// 		monster_struct *monster = (monster_struct *)defence;
// 		if (monster->config->AttackType == 2)
// 			return false;
// 	}

// 	if (attack->get_unit_type() == UNIT_TYPE_PLAYER)
// 	{
// 		player_struct *player = (player_struct *)attack;

// 			//双方是玩家而且都是和平模式，而且不是切磋对象，那么不能打
// 		// if (defence->get_unit_type() == UNIT_TYPE_PLAYER)
// 		// {
// 		// 	if (player->get_attr(PLAYER_ATTR_PK_TYPE) == PK_TYPE_NORMAL)
// 		// 	{
// 		// 		if (((player_struct *)defence)->get_attr(PLAYER_ATTR_PK_TYPE) == PK_TYPE_NORMAL)
// 		// 		{
// 		// 			if (!player->is_qiecuo_target((player_struct *)(defence)))
// 		// 				return false;
// 		// 		}
// 		// 	}
// 		// }
		
// 		if (player->m_pet_list.find((monster_struct *)defence) != player->m_pet_list.end())
// 			return false;
// 		return true;
// 	}
// 	else
// 	{
// 		monster_struct *monster = (monster_struct *)attack;
// 		if (monster->config->AttackType == 2 || monster->data->owner)
// 		{
// 			if (monster->data->owner == defence->get_uuid() || attack == defence)
// 				return false;
// 			return true;
// 		}
// 	}
	
// 	if (defence->get_unit_type() == UNIT_TYPE_PLAYER)
// 		return true;

// 	return false;
// }
