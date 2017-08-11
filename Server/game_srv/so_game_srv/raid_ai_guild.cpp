#include <math.h>
#include <stdlib.h>
#include "game_event.h"
#include "raid_ai.h"
#include "raid_manager.h"
#include "map_config.h"
#include "time_helper.h"
#include "player_manager.h"
#include "check_range.h"
#include "unit.h"
#include "uuid.h"
#include "msgid.h"
#include "raid.pb-c.h"
#include "pvp_raid.pb-c.h"
#include "scene_transfer.pb-c.h"
#include "app_data_statis.h"
#include "buff_manager.h"
#include "monster_manager.h"
#include "guild_battle_manager.h"
#include "relive.pb-c.h"
#include "guild_battle.pb-c.h"

extern void fill_player_base_data(raid_player_info &info, PlayerBaseData &data);
static void get_team_kill_num(raid_struct *raid, int *kill1, int *kill2);
static void	finished_raid(raid_struct *raid, int win_team)
{
	raid->data->state = RAID_STATE_PASS;	
	// 结算
	std::map<uint32_t, GuildBattleFightGuildRewardInfo> reward_map;
	get_guild_raid_reward(raid, win_team, reward_map);
	send_guild_battle_fight_reward_to_guildsrv(reward_map);
}

static void guild_raid_ai_init(raid_struct *raid, player_struct *player)
{
}

static void guild_raid_ai_player_region_changed(raid_struct *raid, player_struct *player, uint32_t region_id)
{
}

static void guild_raid_ai_tick(raid_struct *raid)
{
	if (raid->data->state == RAID_STATE_PASS)
		return;
	
/*	uint32_t now = time_helper::get_cached_time() / 1000;
	int delta_time = now - raid->data->start_time / 1000;
	if (delta_time > 120)
	{
			// 时间到了，副本结束
		int team_kill_1, team_kill_2;
		get_team_kill_num(raid, &team_kill_1, &team_kill_2);
		if (team_kill_1 > team_kill_2)
			finished_raid(raid, 1);
		else if (team_kill_1 < team_kill_2)
			finished_raid(raid, 2);
		else
			finished_raid(raid, 0);
		return;
	}*/
}

static void guild_raid_player_kill(raid_struct *raid, player_struct *player, player_struct *target)
{
	int t;
	struct raid_player_info *tmp = raid->get_raid_player_info(player->get_uuid(), &t);
	assert(tmp);

	raid->GUILD_DATA.kill_record[t]++;

	//广播给所有玩家
	do
	{
		GuildBattleKillNotify nty;
		guild_battle_kill_notify__init(&nty);
		nty.killerid = player->get_uuid();
		nty.deadid = target->get_uuid();
		raid->broadcast_to_raid(MSG_ID_GUILD_BATTLE_KILL_NOTIFY, &nty, (pack_func)guild_battle_kill_notify__pack);
	} while(0);

		// 检查杀人数目是否足够结束游戏
	int team_kill_1, team_kill_2;
	get_team_kill_num(raid, &team_kill_1, &team_kill_2);

	if (team_kill_1 >= (int)raid->m_config->PassValue[0])
	{
		finished_raid(raid, 1);
		return;
	}
	else if (team_kill_2 >= (int)raid->m_config->PassValue[0])
	{
		finished_raid(raid, 2);
		return;
	}
//	pvp_raid_check_ace(raid, target_index);
}

static void guild_raid_ai_player_dead(raid_struct *raid, player_struct *player, unit_struct *killer)
{
	if (killer->get_unit_type() != UNIT_TYPE_PLAYER)
		return;
	if (killer == player)
		return;
	guild_raid_player_kill(raid, (player_struct *)killer, player);
}

static void guild_raid_ai_monster_dead(raid_struct *raid, monster_struct *monster, unit_struct *killer)
{
}

static void guild_raid_ai_player_relive(raid_struct *raid, player_struct *player, uint32_t type)
{
	if (type == 1)	//原地复活
		return;
	player->data->attrData[PLAYER_ATTR_HP] = player->data->attrData[PLAYER_ATTR_MAXHP];
	player->on_hp_changed(0);

	ReliveNotify nty;
	relive_notify__init(&nty);
	nty.playerid = player->get_uuid();
	nty.type = type;
		// 出生点
	int pos;
	float pos_x, pos_z;
	raid->get_raid_player_info(player->get_uuid(), &pos);
	if (pos < MAX_TEAM_MEM)
	{
		get_rand_born_pos1(&pos_x, &pos_z);
		nty.direct = sg_guild_raid_param1[4];
	}
	else
	{
		get_rand_born_pos2(&pos_x, &pos_z);
		nty.direct = sg_guild_raid_param2[4];
	}
	nty.pos_x = pos_x;
	nty.pos_z = pos_z;
	
//	pvp_raid_get_relive_pos(raid, &nty.pos_x, &nty.pos_z, &nty.direct);
	LOG_DEBUG("%s: player[%lu] relive to pos[%d][%d][%d]", __FUNCTION__, player->get_uuid(), nty.pos_x, nty.pos_z, nty.direct);

	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_RELIVE_NOTIFY, relive_notify__pack, nty);

	player->send_clear_sight();
	scene_struct *t_scene = raid;
	raid->delete_player_from_scene(player);
	player->set_pos(nty.pos_x, nty.pos_z);
	int camp_id = player->get_camp_id();
	t_scene->add_player_to_scene(player);
	player->set_camp_id(camp_id);

		//复活的时候加上一个无敌buff
	buff_manager::create_default_buff(114400001, player, player, false);

	player->m_team == NULL ? true : player->m_team->OnMemberHpChange(*player);
}

static void guild_raid_ai_player_attack(raid_struct *raid, player_struct *player, unit_struct *target, int damage)
{
}

static void guild_raid_ai_player_leave(raid_struct *raid, player_struct *player)
{
	int pos;
	raid->get_raid_player_info(player->get_uuid(), &pos);
	player_struct **team_players = NULL;
	if (pos >= MAX_TEAM_MEM)
	{
		team_players = raid->m_player2;
	}
	else
	{
		team_players = raid->m_player;
	}

	bool finished = true;
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (team_players[i] != NULL)
		{
			finished = false;
			break;
		}
	}

	if (finished)
	{
		int win_team = (pos >= MAX_TEAM_MEM ? 2 : 1);
		finished_raid(raid, win_team);
	}
}

static void get_team_kill_num(raid_struct *raid, int *kill1, int *kill2)
{
	int a = 0, b = 0;
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		a += raid->GUILD_DATA.kill_record[i];
		b += raid->GUILD_DATA.kill_record[i + MAX_TEAM_MEM];
	}
	*kill1 = a;
	*kill2 = b;
}

static void guild_raid_ai_player_ready(raid_struct *raid, player_struct *player)
{
	if (!is_guild_battle_opening())
	{
		return;
	}

	int pos;
	raid->get_raid_player_info(player->get_uuid(), &pos);
	if (pos >= MAX_TEAM_MEM)
	{
		player->set_camp_id(2);
	}
	else
	{
		player->set_camp_id(1);		
	}
	LOG_DEBUG("%s: player[%s] camp[%d] pos[%d]", __FUNCTION__, player->get_name(), player->get_camp_id(), pos);
	
	//前端要求进入的时候发
	//获取战斗区信息
	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();

	//双方玩家信息
	do
	{
		GuildBattleMatchNotify nty;
		guild_battle_match_notify__init(&nty);

		GuildBattleMatchData team_data[MAX_GUILD_RAID_TEAM_NUM];
		GuildBattleMatchData* team_point[MAX_GUILD_RAID_TEAM_NUM];
		PlayerBaseData player_data[MAX_GUILD_RAID_TEAM_NUM][MAX_TEAM_MEM];
		PlayerBaseData* player_point[MAX_GUILD_RAID_TEAM_NUM][MAX_TEAM_MEM];
		AttrData attr_data[MAX_GUILD_RAID_TEAM_NUM][MAX_TEAM_MEM][MAX_PLAYER_BASE_ATTR_NUM];
		AttrData* attr_point[MAX_GUILD_RAID_TEAM_NUM][MAX_TEAM_MEM][MAX_PLAYER_BASE_ATTR_NUM];

		nty.n_teams = 0;
		nty.teams = team_point;
		for (int k = 0; k < MAX_GUILD_RAID_TEAM_NUM; ++k)
		{
			if (raid->GUILD_DATA.guild_id[k] == 0)
			{
				continue;
			}

			raid_player_info *players = NULL;
			if (k == 0)
			{
				players = raid->data->player_info;
			}
			else if (k == 1)
			{
				players = raid->data->player_info2;
			}
			else
			{
				break;
			}

			uint32_t guild_id = raid->GUILD_DATA.guild_id[k];
			
			team_point[nty.n_teams] = &team_data[nty.n_teams];
			guild_battle_match_data__init(&team_data[nty.n_teams]);
			team_data[nty.n_teams].guildid = guild_id;
			team_data[nty.n_teams].guildname = get_guild_name(guild_id);

			uint8_t player_num = 0;
			for (int i = 0; i < MAX_TEAM_MEM; ++i)
			{
				raid_player_info &team_player = players[i];
				if (team_player.player_id > 0)
				{
					player_point[nty.n_teams][player_num] = &player_data[nty.n_teams][player_num];
					player_base_data__init(&player_data[nty.n_teams][player_num]);
					for (int j = 0; j < MAX_PLAYER_BASE_ATTR_NUM; ++j)
					{
						attr_point[nty.n_teams][player_num][j] = &attr_data[nty.n_teams][player_num][j];
						attr_data__init(&attr_data[nty.n_teams][player_num][j]);
					}
					player_data[nty.n_teams][player_num].attrs = attr_point[nty.n_teams][player_num];
					fill_player_base_data(team_player, player_data[nty.n_teams][player_num]);
					player_num++;
				}
			}
			team_data[nty.n_teams].members = player_point[nty.n_teams];
			team_data[nty.n_teams].n_members = player_num;

			nty.n_teams++;
		}


		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_GUILD_BATTLE_MATCH_NOTIFY, guild_battle_match_notify__pack, nty);
	} while(0);

	//击杀记录
	do
	{
		GuildBattleRecordNotify nty;
		guild_battle_record_notify__init(&nty);

		GuildBattleRecordData record_data[MAX_TEAM_MEM * 2];
		GuildBattleRecordData* record_point[MAX_TEAM_MEM * 2];

		nty.n_records = 0;
		nty.records = record_point;
		for (int i = 0; i < MAX_TEAM_MEM; ++i)
		{
			raid_player_info &team1_player = raid->data->player_info[i];
			if (team1_player.player_id > 0)
			{
				record_point[nty.n_records] = &record_data[nty.n_records];
				guild_battle_record_data__init(&record_data[nty.n_records]);
				record_data[nty.n_records].playerid = team1_player.player_id;
				record_data[nty.n_records].dead = team1_player.dead_count;
				record_data[nty.n_records].kill = raid->GUILD_DATA.kill_record[i];
				nty.n_records++;
			}

			raid_player_info &team2_player = raid->data->player_info2[i];
			if (team2_player.player_id > 0)
			{
				record_point[nty.n_records] = &record_data[nty.n_records];
				guild_battle_record_data__init(&record_data[nty.n_records]);
				record_data[nty.n_records].playerid = team2_player.player_id;
				record_data[nty.n_records].dead = team2_player.dead_count;
				record_data[nty.n_records].kill = raid->GUILD_DATA.kill_record[i + MAX_TEAM_MEM];
				nty.n_records++;
			}
		}

		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_GUILD_BATTLE_RECORD_NOTIFY, guild_battle_record_notify__pack, nty);
	} while(0);

	//每轮信息
	do
	{
		GuildBattleRoundInfoNotify nty;
		guild_battle_round_info_notify__init(&nty);

		nty.endtime = get_cur_round_end_time();

		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_GUILD_BATTLE_ROUND_INFO_NOTIFY, guild_battle_round_info_notify__pack, nty);
	} while(0);
}

static void guild_raid_ai_player_enter(raid_struct *raid, player_struct *player)
{
}

struct raid_ai_interface raid_ai_guild_interface =
{
	guild_raid_ai_init,
	guild_raid_ai_tick,
	guild_raid_ai_player_enter,
	guild_raid_ai_player_leave,
	guild_raid_ai_player_dead,
	guild_raid_ai_player_relive,
	guild_raid_ai_monster_dead,
	NULL,// guild_raid_ai_collect,
	guild_raid_ai_player_ready,
	NULL,// guild_raid_ai_finished,
	guild_raid_ai_player_attack,
	guild_raid_ai_player_region_changed,

};
