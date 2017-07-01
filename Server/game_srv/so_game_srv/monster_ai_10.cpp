//墓穴副本僵尸头目
#include <math.h>
#include <stdlib.h>
#include "game_event.h"
#include "monster_ai.h"
#include "monster_manager.h"
#include "path_algorithm.h"
#include "player_manager.h"
#include "time_helper.h"
#include "unit.h"
#include "collect.h"
#include "check_range.h"
#include "cached_hit_effect.h"
#include "path_algorithm.h"
#include "camp_judge.h"
#include "count_skill_damage.h"
#include "msgid.h"
#include "buff.h"
#include "buff_manager.h"

// 111300501,111300502,111300503,111300504
// 僵尸boss的怪物ID    151001007
// 石棺技能id          111300503
// 被动招小怪技能id    111300504
#define SHIGUAN_SKILL_ID 111300503
#define CALL_SKILL_ID 111300504
#define CALL_SKILL_CD_TIME (15 * 1000)
static void start_use_shiguan_skill(monster_struct *monster)
{
	for (int i = 0; i < monster->data->cur_sight_player; ++i)
	{
		player_struct *player = player_manager::get_player_by_id(monster->data->sight_player[i]);
		if (!player)
			continue;
		if (get_unit_fight_type(monster, player) != UNIT_FIGHT_TYPE_ENEMY)							
			continue;
		monster->target = player;
		monster->reset_pos();
		monster->data->skill_id = SHIGUAN_SKILL_ID;
		struct position *my_pos = monster->get_pos();
		struct position *his_pos = monster->target->get_pos();
		
		monster->data->angle = -(pos_to_angle(his_pos->pos_x - my_pos->pos_x, his_pos->pos_z - my_pos->pos_z));
		monster->data->skill_target_pos.pos_x = his_pos->pos_x;
		monster->data->skill_target_pos.pos_z = his_pos->pos_z;		
		
		monster->ai_state = AI_ATTACK_STATE;
		monster_cast_skill_to_player(SHIGUAN_SKILL_ID, monster, monster->target, true);

		SkillTable *config = get_config_by_id(SHIGUAN_SKILL_ID, &skill_config);
		struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
		if (act_config)
		{
			monster->data->ontick_time = time_helper::get_cached_time() + act_config->TotalSkillDelay - act_config->ActionTime;
		}
		return;
	}	
}

static void ai_dead_10(monster_struct *monster, scene_struct *scene)
{
	monster->ai_state = AI_DEAD_STATE;
}

extern void normal_ai_tick(monster_struct *monster);
static void ai_tick_10(monster_struct *monster)
{
	if (monster->ai_state == AI_DEAD_STATE)
		return;

	if (monster->is_in_lock_time())
		return;
	if (monster->buff_state & BUFF_STATE_STUN)
		return;

	if (!monster->is_alive())
		assert(0);
	
	if (monster->ai_data.muxue_jiangshi_ai.state == 1 && monster->data->next_skill_id == 0)
	{
		uint64_t now = time_helper::get_cached_time();
		if (now >= monster->ai_data.muxue_jiangshi_ai.call_skill_time &&
			monster->ai_data.muxue_jiangshi_ai.call_monster_num <= 20)
		{
			monster->ai_data.muxue_jiangshi_ai.call_skill_time = now + CALL_SKILL_CD_TIME;
			monster->ai_data.muxue_jiangshi_ai.call_monster_num++;
			monster->data->next_skill_id = CALL_SKILL_ID;

//			monster_cast_skill_to_player(CALL_SKILL_ID, monster, NULL, false);

///
	// if (config->SkillType == 2)
	// {
	// 	struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
	// 	if (!act_config)
	// 		return;

	// 	if (act_config->ActionTime > 0)
	// 	{
	// 		uint64_t now = time_helper::get_cached_time();		
	// 		monster->data->ontick_time = now + act_config->ActionTime;// + 1500;
	// 		monster->data->skill_id = skill_id;
	// 		monster->data->angle = -(pos_to_angle(his_pos->pos_x - my_pos->pos_x, his_pos->pos_z - my_pos->pos_z));
	// 		monster->ai_state = AI_ATTACK_STATE;

	// 		monster->reset_pos();
	// 		monster_cast_skill_to_player(skill_id, monster, monster->target, false);		
	// 		return;
	// 	}
	// }
///			

			// SkillTable *config = get_config_by_id(CALL_SKILL_ID, &skill_config);
			// struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
			// if (act_config)
			// {
			// 	monster->data->ontick_time += act_config->TotalSkillDelay;
			// }
			
			// SkillLvTable *lv_config = get_config_by_id(config->SkillLv, &skill_lv_config);
			// if (lv_config && lv_config->MonsterID != 0 && lv_config->MonsterLv != 0)
			// {
			// 	struct position *t_pos = monster->get_pos();
			// 	struct position pos;

			// 	if (get_circle_random_position_v3(monster->scene, t_pos, CALL_SKILL_RADIUS, &pos))
			// 	{
			// 		monster_struct *t_monster = monster_manager::create_monster_at_pos(monster->scene, lv_config->MonsterID,
			// 			lv_config->MonsterLv, pos.pos_x, pos.pos_z, lv_config->MonsterEff);
			// 		if (t_monster)
			// 		{
			// 			assert(t_monster->ai_type == 15);
			// 			t_monster->ai_data.add_boss_hp_ai.boss_uuid = monster->get_uuid();
			// 		}
			// 	}
			// }
			
			// return;
		}
	}

	normal_ai_tick(monster);
}


static void ai_beattack_10(monster_struct *monster, unit_struct *player)
{
	if (monster->ai_data.muxue_jiangshi_ai.state == 1)
		return;

	int percent = get_monster_hp_percent(monster);
	if (percent > 50)
		return;

	monster->ai_data.muxue_jiangshi_ai.state = 1;
	start_use_shiguan_skill(monster);	
}

struct ai_interface monster_ai_10_interface =
{
	ai_tick_10,
	ai_beattack_10,
	ai_dead_10,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};




