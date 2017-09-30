//墓穴副本最终boss
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
#include "camp_judge.h"
#include "count_skill_damage.h"
#include "msgid.h"
#include "buff.h"
#include "buff_manager.h"
#include "raid.pb-c.h"

//雷鸣鼓倒计时
#define LEIMINGGU_TIMEOUT 10
//雷鸣鼓技能ID
#define LEIMINGGU_SKILL_ID 112130603
//每隔25秒一次魅惑技能
#define LEIXINYE_MEIHUO_SKILL_ID 112130606
#define LEIXINYE_MEIHUO_TIME (25)

//雷薪野免疫buff id
#define LEIXINYE_MIANYI_BUFF_ID 114400011

static void use_meihuo_skill(monster_struct *monster, uint64_t now);
static void use_leiminggu_skill(monster_struct *monster);

static int flash_to_leiminggu(monster_struct *monster)
{
	scene_struct *scene = monster->scene;	
	if (!scene)
		return (0);

	scene->delete_monster_from_scene(monster, true);
	monster->set_pos(sg_leiminggu_boss_pos[0], sg_leiminggu_boss_pos[2]);
	scene->add_monster_to_scene(monster, 0);
	return (0);
}

extern void normal_ai_tick(monster_struct *monster);
static void ai_tick_13(monster_struct *monster)
{
	uint64_t now = time_helper::get_cached_time();
	if (monster->ai_data.leixinye_ai.leminggu_time != 0)
	{
		if (now > monster->ai_data.leixinye_ai.leminggu_time)
		{
			use_leiminggu_skill(monster);
			return;
		}
	}

		// 定时释放魅惑技能
	if (monster->ai_data.leixinye_ai.meihuo_time != 0)
	{
		if (now > monster->ai_data.leixinye_ai.meihuo_time)
		{
			use_meihuo_skill(monster, now);
		}
	}

	normal_ai_tick(monster);
}

const static int next_leiminggu_percent[] = {100, 90, 82, 74, 66, 58, 50, 42, 34, 26, 18, 10, 2};
static int count_next_leiminggu_hp_percent(int cur_percent)
{
	for (size_t i = 0; i < ARRAY_SIZE(next_leiminggu_percent); ++i)
	{
		if (cur_percent > next_leiminggu_percent[i])
			return next_leiminggu_percent[i];
	}
	return (0);
}

static void use_meihuo_skill(monster_struct *monster, uint64_t now)
{
	// struct position *my_pos = monster->get_pos();	
	// for (int i = 0; i < monster->data->cur_sight_player; ++i)
	// {
	// 	player_struct *player = player_manager::get_player_by_id(monster->data->sight_player[i]);
	// 	if (!player)
	// 		continue;
	// 	if (get_unit_fight_type(monster, player) != UNIT_FIGHT_TYPE_ENEMY)							
	// 		continue;
		
	// 	struct position *pos = player->get_pos();
	// 	double x = pos->pos_x - my_pos->pos_x;
	// 	double z = pos->pos_z - my_pos->pos_z;
	// 	if (x * x + z * z > range)
	// 		continue;
	// 	monster->target = player;
	// 	return;
	// }

	if (monster->data->cur_sight_player == 0)
		return;
	int rand = random() % monster->data->cur_sight_player;
	player_struct *player = player_manager::get_player_by_id(monster->data->sight_player[rand]);
	if (!player)
		return;
	monster_cast_immediate_skill_to_player(LEIXINYE_MEIHUO_SKILL_ID, monster, NULL, player);
	monster->ai_data.leixinye_ai.meihuo_time = now + LEIXINYE_MEIHUO_TIME * 1000;
}

static void use_leiminggu_skill(monster_struct *monster)
{
	monster->ai_data.leixinye_ai.leminggu_time = 0;
	flash_to_leiminggu(monster);
		// 释放技能
	monster_cast_skill_to_pos(LEIMINGGU_SKILL_ID, monster, 0, 0);
		// 删除采集物

	Collect *c = Collect::GetById(monster->ai_data.leixinye_ai.leiminggu_collect_id);
	if (c)
	{
		monster->scene->delete_collect_to_scene(c);	
		monster->scene->m_collect.erase(monster->ai_data.leixinye_ai.leiminggu_collect_id);
		Collect::DestroyCollect(monster->ai_data.leixinye_ai.leiminggu_collect_id);
	}
}

static void start_use_leiminggu_skill(monster_struct *monster, int cur_percent)
{
		// 下次使用雷鸣鼓的百分比
	monster->ai_data.leixinye_ai.next_leiminggu_hp_percent = count_next_leiminggu_hp_percent(cur_percent);
	if (monster->ai_data.leixinye_ai.can_use_leiminggu)
	{
			// 如果玩家可以使用，那么视野增加一个采集物 
		Collect *c = Collect::CreateCollectByPos(monster->scene, sg_leiminggu_collect_id,
			sg_leiminggu_pos[0], sg_leiminggu_pos[1], sg_leiminggu_pos[2], sg_leiminggu_pos[3]);
		if (c)
		{
			monster->ai_data.leixinye_ai.leiminggu_collect_id = c->m_uuid;
		}
			// TODO: 发送雷鸣鼓开始使用的广播		
		monster->ai_data.leixinye_ai.leminggu_time = time_helper::get_cached_time() + LEIMINGGU_TIMEOUT * 1000;
		RaidShowCountdownNotify nty;
		raid_show_countdown_notify__init(&nty);
		nty.time = 10;
		monster->broadcast_to_sight(MSG_ID_RAID_SHOW_COUNTDOWN_NOTIFY, &nty, (pack_func)raid_show_countdown_notify__pack, false);
	}
	else
	{
		use_leiminggu_skill(monster);
	}
}

static void ai_beattack_13(monster_struct *monster, unit_struct *player)
{
	int percent = get_monster_hp_percent(monster);
	switch (monster->ai_data.leixinye_ai.state)
	{
		case 0:
				//低于50%血量，开始使用雷鸣鼓
			if (percent <= 50)
			{
				monster->ai_data.leixinye_ai.state = 1;
					// 使用雷鸣鼓技能
				start_use_leiminggu_skill(monster, percent);
					// 添加免疫buff
				buff_manager::create_default_buff(LEIXINYE_MIANYI_BUFF_ID, monster, monster, true);
			}
			if (percent <= 40)
			{
				monster->ai_data.leixinye_ai.state = 2;
					// 召唤 monster->ai_data.leixinye_ai.call_skill_id
				monster_cast_call_monster_skill(monster, monster->ai_data.leixinye_ai.call_skill_id);
			}
			break;
		case 1:
				//低于40%，召唤
			if (percent <= 40)
			{
				monster->ai_data.leixinye_ai.state = 2;
					// 召唤 monster->ai_data.leixinye_ai.call_skill_id
				monster_cast_call_monster_skill(monster, monster->ai_data.leixinye_ai.call_skill_id);				
			}

			if (percent <= monster->ai_data.leixinye_ai.next_leiminggu_hp_percent)
			{
					// 使用雷鸣鼓技能
				start_use_leiminggu_skill(monster, percent);				
			}
			break;
		case 2:
			if (percent <= monster->ai_data.leixinye_ai.next_leiminggu_hp_percent)
			{
					// 使用雷鸣鼓技能
				start_use_leiminggu_skill(monster, percent);				
			}
			break;
		default:
			break;
	}
}

struct ai_interface monster_ai_13_interface =
{
	ai_tick_13,
	ai_beattack_13,
	normal_ai_dead,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

//初始化雷薪野，1,2,3对应三种情况
//1 正常打,    不做任务
//2 正常打, 换一个召唤怪   做护送任务成功
//3 进入第二阶段，无法使用雷鸣鼓   做护送任务失败
void set_leixinye_type(monster_struct *monster, uint32_t type)
{
	LOG_INFO("%s: monster[%p][%lu] type[%u]", __FUNCTION__, monster, monster->get_uuid(), type);
	assert(monster->ai_type == 13);
	switch (type)
	{
		case 1:
			monster->ai_data.leixinye_ai.call_skill_id = 112130605;
			monster->ai_data.leixinye_ai.state = 0;
			monster->ai_data.leixinye_ai.can_use_leiminggu = true;						
			break;
		case 2:
			monster->ai_data.leixinye_ai.call_skill_id = 112130604;			
			monster->ai_data.leixinye_ai.state = 0;
			monster->ai_data.leixinye_ai.can_use_leiminggu = true;			
			break;
		case 3:
			monster->ai_data.leixinye_ai.call_skill_id = 112130605;			
			monster->ai_data.leixinye_ai.state = 2;
			monster->ai_data.leixinye_ai.can_use_leiminggu = false;
			start_use_leiminggu_skill(monster, 100);							
				// 添加免疫buff
			buff_manager::create_default_buff(LEIXINYE_MIANYI_BUFF_ID, monster, monster, true);
			break;
		default:
			assert(0);
	}
	monster->ai_data.leixinye_ai.type = type;
}

void stop_leiminggu_skill(monster_struct *monster)
{
	monster->ai_data.leixinye_ai.leminggu_time = 0;
	RaidShowCountdownNotify nty;
	raid_show_countdown_notify__init(&nty);
	nty.time = 0;
	monster->broadcast_to_sight(MSG_ID_RAID_SHOW_COUNTDOWN_NOTIFY, &nty, (pack_func)raid_show_countdown_notify__pack, false);
}





