#include <math.h>
#include <stdlib.h>
#include "game_event.h"
#include "raid_ai.h"
#include "raid_manager.h"
#include "time_helper.h"
#include "app_data_statis.h"
#include "unit.h"
#include "msgid.h"
#include "raid.pb-c.h"
#include "monster_manager.h"
#include "../../proto/hotel.pb-c.h"

static void guoyu_raid_ai_init(raid_struct *raid, player_struct *player)
{
	if (player->m_team == NULL)
	{
		return;
	}
	std::map<uint64_t, struct RandomMonsterTable*>::iterator it = random_monster.find(player->data->guoyu.cur_task);
	if (it == random_monster.end())
	{
		return;
	}
	std::map<uint64_t, struct RandomDungeonTable*>::iterator itFb = random_guoyu_dungenon_config.find(player->data->guoyu.random_map);
	if (itFb == random_guoyu_dungenon_config.end())
	{
		return;
	}
	int r = rand() % 10000;
	int all = 0;
	int i = 0;
	for (; (uint32_t)i < itFb->second->n_GroupProbability - 1; ++i)
	{
		all += itFb->second->GroupProbability[i];
		if (r <= all)
		{
			break;
		}
	}
	uint32_t avLv = player->m_team->GetAverageLevel();
	if (avLv > it->second->MaxLevel)
	{
		avLv = it->second->MaxLevel;
	}

	monster_manager::create_monster_at_pos(raid, it->second->MonsterID, avLv, itFb->second->PointX[i], itFb->second->PointZ[i], 0, NULL);
}

static void guoyu_raid_ai_finished(raid_struct *raid)
{
	raid->clear_monster();

	GuoyuFbSucc notify;
	guoyu_fb_succ__init(&notify);
	notify.succ = 0;

	raid->data->state = RAID_STATE_PASS;

	EXTERN_DATA extern_data;
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (!raid->m_player[i])
			continue;
		extern_data.player_id = raid->m_player[i]->get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_GUOYU_FB_SUCC_NOTIFY, guoyu_fb_succ__pack, notify);
	}
}

static void guoyu_raid_ai_monster_dead(raid_struct *raid, monster_struct *monster, unit_struct *killer)
{
	EXTERN_DATA extern_data;
	GuoyuSucc notify;
	guoyu_succ__init(&notify);
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (!raid->m_player[i])
			continue;
		extern_data.player_id = raid->m_player[i]->get_uuid();

		std::map<uint64_t, struct RandomMonsterTable*>::iterator it = random_monster.find(raid->m_player[i]->data->guoyu.cur_task);
		if (it == random_monster.end())
		{
			continue;
		}
		uint64_t dropid = it->second->BasicsReward;
		if (monster->data->monster_id == it->second->MonsterID)
		{
			notify.succ = true;
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_GUOYU_TASK_SUCC_NOTIFY, guoyu_succ__pack, notify);
			raid->m_player[i]->data->guoyu.cur_task = 0;

			if (raid->m_player[i]->data->guoyu.award)
			{
				dropid = it->second->ActivityReward;
				raid->m_player[i]->data->guoyu.award = false;
			}
		}

		std::map<uint32_t, uint32_t> item_list;
		int ret = get_drop_item(dropid, item_list);
		if (ret != 0)
		{
			continue;
		}
		SpecialtySkillTable *tableSkill = get_yaoshi_skill_config(GUOYU_TWELVE, raid->m_player[i]->data->guoyu.guoyu_level);
		for (std::map<uint32_t, uint32_t>::iterator it = item_list.begin(); it != item_list.end(); ++it)
		{
			uint32_t add = 0;
			if (tableSkill != NULL)
			{
				add = tableSkill->EffectValue[0];
			}
			if (get_item_type(it->first) == ITEM_TYPE_GUOYU_EXP)
			{
				if (raid->m_player[i]->data->cur_yaoshi == MAJOR__TYPE__SHUANGJIN)  //专精加成
				{
					SpecialTitleTable *title = get_yaoshi_title_table(raid->m_player[i]->data->cur_yaoshi, raid->m_player[i]->data->guoyu.guoyu_level);
					if (title != NULL)
					{
						add += title->TitleEffect2;
					}
				}
			}
			it->second = it->second *(10000 + add) / 10000;
		}
		raid->m_player[i]->add_item_list(item_list, MAGIC_TYPE_YAOSHI, ADD_ITEM_SEND_MAIL_WHEN_BAG_FULL);
		raid->m_player[i]->check_activity_progress(AM_YAOSHI, 3);
	}

	if (raid->get_monster_num() == 0)
	{
		guoyu_raid_ai_finished(raid);
	}
}

static void guoyu_raid_ai_tick(raid_struct *raid)
{
	if (time_helper::get_cached_time() > raid->data->start_time + raid->m_config->ScoreValue[0] * 1000 && raid->data->state == RAID_STATE_START)
	{
		raid->clear_monster();
		raid->data->state = RAID_STATE_FAIL;

		GuoyuFbSucc notify;
		guoyu_fb_succ__init(&notify);
		notify.succ = 1;
		EXTERN_DATA extern_data;
		for (int i = 0; i < MAX_TEAM_MEM; ++i)
		{
			if (!raid->m_player[i])
				continue;
			extern_data.player_id = raid->m_player[i]->get_uuid();
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_GUOYU_FB_SUCC_NOTIFY, guoyu_fb_succ__pack, notify);
			raid->m_player[i]->check_activity_progress(AM_YAOSHI, 3);
		}
	}
}

static void guoyu_raid_ai_player_enter(raid_struct *raid, player_struct *player)
{
	if (raid == NULL || player == NULL)
	{
		return;
	}
	GuoyuFb notify;
	guoyu_fb__init(&notify);
	notify.cd = 0;
	if (time_helper::get_cached_time() < raid->data->start_time + raid->m_config->ScoreValue[0] * 1000)
	{
		notify.cd = raid->data->start_time + raid->m_config->ScoreValue[0] * 1000 - time_helper::get_cached_time();
		notify.cd /= 1000;
	}
	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_GUOYU_FB_CD_NOTIFY, guoyu_fb__pack, notify);
}

struct raid_ai_interface raid_ai_guoyu_interface =
{
	guoyu_raid_ai_init,
	guoyu_raid_ai_tick,
	guoyu_raid_ai_player_enter,
	NULL,
	NULL,
	NULL, //normal_raid_ai_player_relive,
	guoyu_raid_ai_monster_dead,
	NULL,
	NULL,
	guoyu_raid_ai_finished,
	NULL,
};
