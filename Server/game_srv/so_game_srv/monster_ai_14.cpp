//护送怪物AI
#include <math.h>
#include <stdlib.h>
#include "game_event.h"
#include "monster_ai.h"
#include "path_algorithm.h"
#include "player_manager.h"
#include "time_helper.h"
#include "unit.h"
#include "check_range.h"
#include "cached_hit_effect.h"
#include "count_skill_damage.h"
#include "msgid.h"
#include "monster_manager.h"

static bool monster_ai_14_check_goback(monster_struct *monster);
static void monster_ai_14_do_goback(monster_struct *monster);
static int get_escort_ai_wait_time(monster_struct * monster)
{
	assert(monster->create_config->n_TargetInfoList > 0);
	assert(monster->ai_data.escort_ai.cur_pos_index < monster->create_config->n_TargetInfoList);
	return monster->create_config->TargetInfoList[monster->ai_data.escort_ai.cur_pos_index]->RemainTime;
}

static bool get_escort_ai_next_pos(monster_struct * monster, float *pos_x, float *pos_z)
{
	assert(monster->create_config->n_TargetInfoList > 0);
	++monster->ai_data.escort_ai.cur_pos_index;
	if (monster->ai_data.escort_ai.cur_pos_index >= monster->create_config->n_TargetInfoList)
	{
		return false;
	}
	*pos_x = monster->create_config->TargetInfoList[monster->ai_data.escort_ai.cur_pos_index]->TargetPos->TargetPosX;
	*pos_z = monster->create_config->TargetInfoList[monster->ai_data.escort_ai.cur_pos_index]->TargetPos->TargetPosZ;
	return (true);
}

static bool get_escort_ai_cur_pos(monster_struct * monster, float *pos_x, float *pos_z)
{
	assert(monster->create_config->n_TargetInfoList > 0);
	if (monster->ai_data.escort_ai.cur_pos_index >= monster->create_config->n_TargetInfoList)
	{
		return false;
	}
	*pos_x = monster->create_config->TargetInfoList[monster->ai_data.escort_ai.cur_pos_index]->TargetPos->TargetPosX;
	*pos_z = monster->create_config->TargetInfoList[monster->ai_data.escort_ai.cur_pos_index]->TargetPos->TargetPosZ;
	return (true);
}

//检查护送周围是否有指定的怪物
static bool check_watch_monster_in_range(monster_struct *monster)
{
	assert(monster->ai_type == 14);
	do
	{
		EscortTask *config = get_config_by_id(monster->ai_data.escort_ai.escort_id, &escort_config);
		if (!config)
		{
			break;
		}

		position *my_pos = monster->get_pos();
		for (int i = 0; i < monster->data->cur_sight_monster; ++i)
		{
			monster_struct *target_monster = monster_manager::get_monster_by_id(monster->data->sight_monster[i]);
			if (!target_monster)
			{
				continue;
			}

			if (!check_circle_in_range(my_pos, target_monster->get_pos(), config->Range))
			{
				continue;
			}

			for (uint32_t j = 0; j < config->n_MonsterID; ++j)
			{
				if ((uint32_t)config->MonsterID[j] == target_monster->data->monster_id)
				{
					return true;
				}
			}
		}
	} while(0);

	return false;
}

static void do_attack(monster_struct *monster)
{
	
}

static void do_wait(monster_struct *monster)
{
	//position *cur_pos = monster->get_pos();
	//LOG_DEBUG("[%s:%d] monster[%lu][%u][%p], pos_x:%f, pos_z:%f", __FUNCTION__, __LINE__, monster->get_uuid(), monster->data->monster_id, monster, cur_pos->pos_x, cur_pos->pos_z);
	float pos_x, pos_z;
	if (!get_escort_ai_next_pos(monster, &pos_x, &pos_z))
	{
			//到达目的地了
		player_struct *player = player_manager::get_player_by_id(monster->ai_data.escort_ai.owner_player_id);
		if (player)
			player->stop_escort(monster->ai_data.escort_ai.escort_id, true);
		return;
	}
	monster->reset_pos();
	monster->data->move_path.pos[1].pos_x = pos_x;
	monster->data->move_path.pos[1].pos_z = pos_z;
	monster->send_patrol_move();
	monster->ai_state = AI_PATROL_STATE;
}

static bool check_into_pursue(monster_struct *monster)
{
	do
	{
		if (!monster->ai_data.escort_ai.config)
		{
			break;
		}

		//2是不停下，所以不需要检测怪物
		if (monster->ai_data.escort_ai.config->BlockedStop == 2)
		{
			break;
		}

		if (!check_watch_monster_in_range(monster))
		{
			break;
		}

		monster->ai_state = AI_PURSUE_STATE;
		monster->data->target_pos.pos_x = 0;
		monster->data->target_pos.pos_z = 0;	
		//停止移动
		monster->stop_move();
		struct position *my_pos = monster->get_pos();

		if (monster->ai_data.escort_ai.config->BlockedStop == 3) //停下并攻击
		{
			//需要一个攻击目标
			if (!monster->target)
			{
				monster->try_active_attack();
			}

			//如果可以追击，记录追击需要的数据
			if (monster->ai_config->ChaseRange > 0)
//			if (monster->ai_config->IsChase == 1)
			{
				//需要记录go_back的坐标
				monster->ai_data.escort_ai.ret_pos.pos_x = my_pos->pos_x;
				monster->ai_data.escort_ai.ret_pos.pos_z = my_pos->pos_z;			
				if (monster->target)
				{
					struct position *his_pos = monster->target->get_pos();
					monster->data->target_pos.pos_x = his_pos->pos_x;
					monster->data->target_pos.pos_z = his_pos->pos_z;
					monster->data->move_path.pos[1].pos_x = his_pos->pos_x;
					monster->data->move_path.pos[1].pos_z = his_pos->pos_z;
					monster->send_patrol_move();
				}
			}
		}
		//LOG_DEBUG("[%s:%d] monster[%lu][%u][%p], pos_x:%f, pos_z:%f, target_pos_x:%f, target_pos_z:%f, ActiveAttackRange:%u, IsChase:%u", __FUNCTION__, __LINE__, monster->get_uuid(), monster->data->monster_id, monster, my_pos->pos_x, my_pos->pos_z, monster->data->target_pos.pos_x, monster->data->target_pos.pos_z, monster->ai_config->ActiveAttackRange, monster->ai_config->IsChase);

		return true;
	} while(0);

	return false;
}

static bool check_into_stand(monster_struct *monster)
{
	do
	{
		player_struct *player = player_manager::get_player_by_id(monster->ai_data.escort_ai.owner_player_id);
		if (!player)
		{
			break;
		}

		position *player_pos = player->get_pos();
		position *monster_pos = monster->get_pos();
		if (check_circle_in_range(player_pos, monster_pos, monster->ai_data.escort_ai.config->Distance))
		{
			break;
		}

		monster->ai_state = AI_STAND_STATE;
		monster->stop_move();
		//LOG_DEBUG("[%s:%d] monster[%lu][%u][%p], pos_x:%f, pos_z:%f", __FUNCTION__, __LINE__, monster->get_uuid(), monster->data->monster_id, monster, monster_pos->pos_x, monster_pos->pos_z);
		return true;
	} while(0);

	return false;
}

static void do_patrol(monster_struct *monster)
{
	if (!monster->config)
		return;

	if (check_into_pursue(monster))
	{
		return;
	}

	if (check_into_stand(monster))
	{
		return;
	}

	//position *monster_pos = monster->get_pos();
	//LOG_DEBUG("[%s:%d] monster[%lu][%u][%p], pos_x:%f, pos_z:%f", __FUNCTION__, __LINE__, monster->get_uuid(), monster->data->monster_id, monster, monster_pos->pos_x, monster_pos->pos_z);
	if (monster->is_unit_in_move())
		return;

	monster->ai_state = AI_WAIT_STATE;
	monster->data->ontick_time = time_helper::get_cached_time() + get_escort_ai_wait_time(monster) * 1000;
	return;
}

static void do_stand(monster_struct *monster)
{
	//position *my_pos = monster->get_pos();
	//LOG_DEBUG("[%s:%d] monster[%lu][%u][%p], pos_x:%f, pos_z:%f", __FUNCTION__, __LINE__, monster->get_uuid(), monster->data->monster_id, monster, my_pos->pos_x, my_pos->pos_z);
	if (check_into_pursue(monster))
	{
		return;
	}

	player_struct *player = player_manager::get_player_by_id(monster->ai_data.escort_ai.owner_player_id);
	if (!player)
	{
		return;
	}

	position *player_pos = player->get_pos();
	position *monster_pos = monster->get_pos();
	if (!check_circle_in_range(player_pos, monster_pos, monster->ai_data.escort_ai.config->Distance))
	{
		return;
	}

	float pos_x, pos_z;
	if (!get_escort_ai_cur_pos(monster, &pos_x, &pos_z))
	{
		//到达目的地了
		player_struct *player = player_manager::get_player_by_id(monster->ai_data.escort_ai.owner_player_id);
		if (player)
			player->stop_escort(monster->ai_data.escort_ai.escort_id, true);
		return;
	}

	monster->reset_pos();
	monster->data->move_path.pos[1].pos_x = pos_x;
	monster->data->move_path.pos[1].pos_z = pos_z;
	monster->send_patrol_move();
	monster->ai_state = AI_PATROL_STATE;
}

//尝试回到巡逻状态
static bool try_turn_into_patrol(monster_struct *monster)
{
	if (!check_watch_monster_in_range(monster)) //直到范围内怪物不存在，才继续往前走
	{
		//position *my_pos = monster->get_pos();
		//LOG_DEBUG("[%s:%d] monster[%lu][%u] patrol, pos_x:%f, pos_z:%f", __FUNCTION__, __LINE__, monster->get_uuid(), monster->data->monster_id, my_pos->pos_x, my_pos->pos_z);
		monster->ai_state = AI_PATROL_STATE;

		float pos_x, pos_z;
		if (!get_escort_ai_cur_pos(monster, &pos_x, &pos_z))
		{
			//到达目的地了
			player_struct *player = player_manager::get_player_by_id(monster->ai_data.escort_ai.owner_player_id);
			if (player)
				player->stop_escort(monster->ai_data.escort_ai.escort_id, true);
			return true;
		}
		monster->reset_pos();
		monster->data->move_path.pos[1].pos_x = pos_x;
		monster->data->move_path.pos[1].pos_z = pos_z;
		monster->send_patrol_move();
		monster->ai_state = AI_PATROL_STATE;
		return true;
	}

	return false;
}

static void	do_pursue(monster_struct *monster)
{
	if (!monster->data)
		return;
	if (monster->ai_type != AI_TYPE_ESCORT)
		return;

	if (monster->ai_data.escort_ai.config->BlockedStop == 1) //停下不攻击
	{
		try_turn_into_patrol(monster);
	}
	else if (monster->ai_data.escort_ai.config->BlockedStop == 3) //停下并攻击
	{
		if (monster->ai_config->ChaseRange == 0)
//		if (monster->ai_config->IsChase == 0) //不追击，只能在原地攻击
		{
			if (!monster->target || !monster->target->is_avaliable() || !monster->target->is_alive())
			{
				//LOG_DEBUG("[%s:%d] monster[%lu][%u] not chase, choose another target, ActiveAttackRange:%u", __FUNCTION__, __LINE__, monster->get_uuid(), monster->data->monster_id, monster->ai_config->ActiveAttackRange);
				if (!try_turn_into_patrol(monster))
				{
					monster->try_active_attack(); //主动怪主动寻找下一个攻击目标
				}
				return;
			}

			struct position *my_pos = monster->get_pos();
			struct position *his_pos = monster->target->get_pos();
			//如果足够距离释放技能，就放技能
			if (check_distance_in_range(my_pos, his_pos, monster->ai_config->ActiveAttackRange))
			{
				//LOG_DEBUG("[%s:%d] monster[%lu][%u] not chase, cast skill, ActiveAttackRange:%u", __FUNCTION__, __LINE__, monster->get_uuid(), monster->data->monster_id, monster->ai_config->ActiveAttackRange);
				monster->reset_pos();
				// 释放技能攻击目标
				if (monster->config->n_Skill > 0)
				{
					monster_cast_immediate_skill_to_player(monster->config->Skill[0], monster, NULL, monster->target);
				}

			}

		}
		else //追击
		{
			monster->on_pursue();

			struct position *my_pos = monster->get_pos();
			if (!monster->target || !monster->target->is_avaliable() || !monster->target->is_alive())
			{
				//LOG_DEBUG("[%s:%d] monster[%lu][%u] chase, target miss, ActiveAttackRange:%u", __FUNCTION__, __LINE__, monster->get_uuid(), monster->data->monster_id, monster->ai_config->ActiveAttackRange);
				if (my_pos->pos_x == monster->ai_data.escort_ai.ret_pos.pos_x && my_pos->pos_z == monster->ai_data.escort_ai.ret_pos.pos_z) //如果已经在原点
				{
					if (!try_turn_into_patrol(monster))
					{
						if (monster->try_active_attack()) //主动怪主动寻找下一个目标，并追击
						{
							struct position *his_pos = monster->target->get_pos();
							monster->data->move_path.pos[1].pos_x = his_pos->pos_x;
							monster->data->move_path.pos[1].pos_z = his_pos->pos_z;
							monster->send_patrol_move();
						}
					}
				}
				else
				{
					monster_ai_14_do_goback(monster);
					//LOG_DEBUG("[%s:%d] monster[%lu][%u] chase, target miss so go back, ActiveAttackRange:%u", __FUNCTION__, __LINE__, monster->get_uuid(), monster->data->monster_id, monster->ai_config->ActiveAttackRange);
				}
				return;
			}

			struct position *his_pos = monster->target->get_pos();
			//如果目标的位置没有改变，就继续移动
			if (monster->is_unit_in_move())
			{
				if (monster->data->target_pos.pos_x == his_pos->pos_x
						&& monster->data->target_pos.pos_z == his_pos->pos_z)
				{
					monster->data->ontick_time += random() % 1000;		
					return;
				}
			}

			//如果超出追击范围，就走回原点
			if (monster_ai_14_check_goback(monster))
			{
				//LOG_DEBUG("[%s:%d] monster[%lu][%u] chase, out of range so go back, ActiveAttackRange:%u", __FUNCTION__, __LINE__, monster->get_uuid(), monster->data->monster_id, monster->ai_config->ActiveAttackRange);
				monster_ai_14_do_goback(monster);
				return;
			}

			//如果足够距离释放技能，就放技能
			if (check_distance_in_range(my_pos, his_pos, monster->ai_config->ActiveAttackRange))
			{
				//LOG_DEBUG("[%s:%d] monster[%lu][%u] chase, cast skill, ActiveAttackRange:%u", __FUNCTION__, __LINE__, monster->get_uuid(), monster->data->monster_id, monster->ai_config->ActiveAttackRange);
				monster->reset_pos();
				// 释放技能攻击目标
				if (monster->config->n_Skill > 0)
				{
					monster_cast_immediate_skill_to_player(monster->config->Skill[0], monster, NULL, monster->target);
				}

			}
			else //否则重新往目标追去
			{
				//LOG_DEBUG("[%s:%d] monster[%lu][%u] chase, target pos change then change path, ActiveAttackRange:%u", __FUNCTION__, __LINE__, monster->get_uuid(), monster->data->monster_id, monster->ai_config->ActiveAttackRange);
				monster->reset_pos();
				monster->data->target_pos.pos_x = his_pos->pos_x;
				monster->data->target_pos.pos_z = his_pos->pos_z;
				monster->data->move_path.pos[1].pos_x = his_pos->pos_x;
				monster->data->move_path.pos[1].pos_z = his_pos->pos_z;
				monster->send_patrol_move();
			}
		}
		return;
	}

}

static void	do_goback(monster_struct *monster)
{
	if (!monster->data)
		return;
	if (monster->ai_type != AI_TYPE_ESCORT)
		return;
	if (!monster->config)
		return;
	if (monster->is_unit_in_move())
		return;

	//没有回到起点，可能是其他原因导致停止移动
	struct position *my_pos = monster->get_pos();
	if (!(my_pos->pos_x == monster->ai_data.escort_ai.ret_pos.pos_x && my_pos->pos_z == monster->ai_data.escort_ai.ret_pos.pos_z))
	{
		monster_ai_14_do_goback(monster);
		return;
	}

	//LOG_DEBUG("[%s:%d] monster[%lu][%u] chase, back end, ActiveAttackRange:%u", __FUNCTION__, __LINE__, monster->get_uuid(), monster->data->monster_id, monster->ai_config->ActiveAttackRange);
	if (!try_turn_into_patrol(monster))
	{
		monster->ai_state = AI_PURSUE_STATE;
		if (monster->try_active_attack()) //主动怪主动寻找下一个目标，并追击
		{
			struct position *his_pos = monster->target->get_pos();
			monster->data->move_path.pos[1].pos_x = his_pos->pos_x;
			monster->data->move_path.pos[1].pos_z = his_pos->pos_z;
			monster->send_patrol_move();
		}
	}
}

static void monster_ai_14_tick(monster_struct *monster)
{
	if (monster->ai_type != AI_TYPE_ESCORT)
		return;
	
	if (monster->ai_state == AI_DEAD_STATE)
		return;

	if (monster->is_in_lock_time())
		return;
	
	assert(monster->scene);
	area_struct *area = monster->area;
	assert(area);
//	if (!area->is_all_neighbour_have_player())
//		return;
	switch (monster->ai_state)
	{
		case AI_WAIT_STATE:
			do_wait(monster);
			break;
		case AI_ATTACK_STATE:
			do_attack(monster);
			break;
		case AI_PURSUE_STATE:
			do_pursue(monster);
			break;
		case AI_PATROL_STATE:
			do_patrol(monster);
			break;
		case AI_STAND_STATE:
			do_stand(monster);
			break;
		case AI_GO_BACK_STATE:
			do_goback(monster);
			break;
		default:
			break;
	}
}

static void monster_ai_14_beattack(monster_struct *monster, unit_struct *player)
{
	//struct position *cur_pos = monster->get_pos();
	//LOG_DEBUG("[%s:%d] monster[%lu][%u][%p], pos_x:%f, pos_z:%f", __FUNCTION__, __LINE__, monster->get_uuid(), monster->data->monster_id, monster, cur_pos->pos_x, cur_pos->pos_z);
	if (monster->ai_data.escort_ai.config->BlockedStop != 3)
	{
		return;
	}

	bool record_ret_pos = (monster->ai_state == AI_PATROL_STATE);
	
	monster->ai_state = AI_PURSUE_STATE;
	monster->data->target_pos.pos_x = 0;
	monster->data->target_pos.pos_z = 0;	
		//停止移动
	monster->stop_move();
	
	if (!(monster->target && monster->target->is_avaliable()))
	{
		monster->target = player;
	}
	
	//如果可以追击，记录追击需要的数据
	if (monster->ai_config->ChaseRange > 0)
//	if (monster->ai_config->IsChase == 1)
	{
		//需要记录go_back的坐标
		if (record_ret_pos)
		{
			struct position *pos = monster->get_pos();
			monster->ai_data.escort_ai.ret_pos.pos_x = pos->pos_x;
			monster->ai_data.escort_ai.ret_pos.pos_z = pos->pos_z;			
		}
		if (monster->target)
		{
			struct position *his_pos = monster->target->get_pos();
			monster->data->target_pos.pos_x = his_pos->pos_x;
			monster->data->target_pos.pos_z = his_pos->pos_z;
			monster->data->move_path.pos[1].pos_x = his_pos->pos_x;
			monster->data->move_path.pos[1].pos_z = his_pos->pos_z;
			monster->send_patrol_move();
		}
	}
}

static void monster_ai_14_befly(monster_struct *monster, unit_struct *player)
{
}

static void monster_ai_14_dead(monster_struct *monster, scene_struct *scene)
{
	monster->ai_state = AI_DEAD_STATE;
	player_struct *player = player_manager::get_player_by_id(monster->ai_data.escort_ai.owner_player_id);
	if (player)
		player->stop_escort(monster->ai_data.escort_ai.escort_id, false);	
}

//检查是否需要返回
static bool monster_ai_14_check_goback(monster_struct *monster)
{
	struct position *my_pos = monster->get_pos();
//	monster->data->move_path.pos[1].pos_x = monster->ai_data.circle_ai.ret_pos.pos_x;
//	monster->data->move_path.pos[1].pos_z = monster->ai_data.circle_ai.ret_pos.pos_z;

	if (fabsf(my_pos->pos_x - monster->ai_data.escort_ai.ret_pos.pos_x) > (int)(monster->ai_config->ChaseRange)
		|| fabsf(my_pos->pos_z - monster->ai_data.escort_ai.ret_pos.pos_z) > (int)(monster->ai_config->ChaseRange))
	{
//		do_goback(monster);
		return true;
	}
	
	return false;
}

static void monster_ai_14_do_goback(monster_struct *monster)
{
	monster->on_go_back();	
	monster->ai_state = AI_GO_BACK_STATE;
	monster->reset_pos();
	monster->data->move_path.pos[1].pos_x = monster->ai_data.escort_ai.ret_pos.pos_x;
	monster->data->move_path.pos[1].pos_z = monster->ai_data.escort_ai.ret_pos.pos_z;
	monster->send_patrol_move();
}

struct ai_interface monster_ai_14_interface =
{
	monster_ai_14_tick,
	monster_ai_14_beattack,
	monster_ai_14_dead,
	monster_ai_14_befly,
	NULL,
	NULL
};




