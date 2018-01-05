#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include "game_event.h"
#include "raid_ai.h"
#include "raid_manager.h"
#include "time_helper.h"
#include "app_data_statis.h"
#include "unit.h"
#include "msgid.h"
#include "raid.pb-c.h" 
#include "../proto/relive.pb-c.h"
#include "../proto/player_redis_info.pb-c.h"
#include "../proto/tower.pb-c.h"
#include "server_proto.h"
#include "buff_manager.h"
#include "player_manager.h"
#include "monster_manager.h"

//static const int MAX_TOWER_LEVEL = 100;
enum TOWER_STATE
{
	TOWER_STATE_INIT = 0,
	TOWER_STATE_RUN = 1,
	TOWER_STATE_END = 2,
};

int refresh_next_tower(player_struct *player)
{
	if (player == NULL)
	{
		return 1;
	}
	if (!player->is_in_raid())
	{
		return 2;
	}
	raid_struct *raid = (raid_struct *)player->scene;
	if (raid == NULL)
	{
		return 3;
	}
	if (raid->m_config->DengeonRank != DUNGEON_TYPE_TOWER)
	{
		return 4;
	}
	if (player->data->tower.cur_num == 0)
	{
		return 5;
	}
	ParameterTable *table = get_config_by_id(161001004, &parameter_config);
	if (table == NULL)
	{
		return 6;
	}
	if (player->get_attr(PLAYER_ATTR_HP) <= 0)
	{
		ReliveNotify nty;
		relive_notify__init(&nty);
		nty.playerid = player->get_uuid();
		nty.type = 1;
		player->broadcast_to_sight(MSG_ID_RELIVE_NOTIFY, &nty, (pack_func)relive_notify__pack, true);
		player->data->attrData[PLAYER_ATTR_HP] = player->data->attrData[PLAYER_ATTR_MAXHP];
		player->on_hp_changed(0);
	}

	--player->data->tower.cur_num;
	raid->data->ai_data.tower_data.end = time_helper::get_cached_time() / 1000 + table->parameter1[2];
	raid->data->ai_data.tower_data.state = TOWER_STATE_RUN;

	StartTower send;
	start_tower__init(&send);
	send.cd = table->parameter1[2];
	send.lv = player->data->tower.cur_lv;
	send.cur_num = player->data->tower.cur_num;
	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_START_CLIMB_TOWER_NOTIFY, start_tower__pack, send);

	P20076Table *monTable = get_config_by_id(351601000 + player->data->tower.cur_lv, &tower_level_config);
	if (monTable == NULL)
	{
		return 7;
	}
	for (uint32_t i = 0; i < monTable->n_MonsterID; ++i)
	{
		for (uint32_t num = 0; num < monTable->MonsterNum[i]; ++num)
		{
			uint64_t x = monTable->BirthPointX + monTable->BirthRange - rand() % (2 * monTable->BirthRange + 1);
			uint64_t z = monTable->BirthPointZ + monTable->BirthRange - rand() % (2 * monTable->BirthRange + 1);
			monster_struct *mon = monster_manager::create_monster_at_pos(player->scene, monTable->MonsterID[i], monTable->MonsterLevel[i], x, z, 0, NULL, 0);
			if (mon == NULL)
			{
				LOG_INFO("%s: player[%lu] add to %lu", __FUNCTION__, player->get_uuid(), raid->data->uuid);
			}
		}
	}
	return 0;
}

static void tower_faile(raid_struct *raid, player_struct *player)
{
	if (player == NULL)
	{
		return;
	}

	raid->data->ai_data.tower_data.state = TOWER_STATE_END;

	TowerResult send;
	tower_result__init(&send);
	send.ret = 1;
	send.cur_num = raid->m_player[0]->data->tower.cur_num;

	EXTERN_DATA extern_data;
	extern_data.player_id = raid->m_player[0]->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_CLIMB_TOWER_RESULT_NOTIFY, tower_result__pack, send);

	raid->clear_monster();
}

void tower_raid_ai_tick(raid_struct *raid)
{
	if (raid->data->ai_data.tower_data.state == TOWER_STATE_RUN)
	{
		if (raid->data->ai_data.tower_data.end < time_helper::get_cached_time() / 1000)
		{
			tower_faile(raid, raid->m_player[0]);
		}
	}
}

static void tower_raid_ai_init(raid_struct *raid, player_struct *player)
{

}

void tower_raid_ai_finished(raid_struct *raid)
{
	
}

static void tower_raid_ai_failed(raid_struct *raid)
{
	raid->clear_monster();
}

static void tower_raid_ai_player_enter(raid_struct *raid, player_struct *player)
{
	LOG_INFO("%s: player[%lu] add to %lu", __FUNCTION__, player->get_uuid(), raid->data->uuid);
//	raid->ZHENYING_DATA.cur_player_num++;

}
static void tower_raid_ai_player_leave(raid_struct *raid, player_struct *player)
{
	LOG_INFO("%s: player[%lu] del from %lu", __FUNCTION__, player->get_uuid(), raid->data->uuid);	
}

static void tower_raid_ai_player_dead(raid_struct *raid, player_struct *player, unit_struct *killer)
{
	tower_faile(raid, player);
}
static void tower_raid_ai_monster_dead(raid_struct *raid, monster_struct *monster, unit_struct *killer)
{
	if (raid->m_monster.size() != 0)
	{
		return;
	}
	player_struct *player = NULL;
	if (killer->get_unit_type() == UNIT_TYPE_PLAYER)
	{
		player = (player_struct *)killer;
	}
	if (player == NULL)
	{
		return;
	}
	ParameterTable *table = get_config_by_id(161001004, &parameter_config);
	if (table == NULL)
	{
		return;
	}

	uint32_t next = 0;
	//if (player->data->tower.cur_lv < MAX_TOWER_LEVEL && player->data->tower.top_lv < MAX_TOWER_LEVEL)
	{
		if (player->data->tower.cur_lv > player->data->tower.top_lv)
		{
			player->data->tower.top_lv = player->data->tower.cur_lv;
		}
		if (player->data->tower.cur_lv + 5 > player->data->tower.top_lv)
		{
			if (player->data->tower.top_lv < MAX_TOWER_LEVEL)
			{
				next = player->data->tower.top_lv + 1;
			}
			else if (player->data->tower.cur_lv < MAX_TOWER_LEVEL)
			{
				next = player->data->tower.top_lv;
			}
		}
		else
		{
			next = player->data->tower.cur_lv + 5;
		}
		//if (player->data->tower.top_lv == player->data->tower.cur_lv)
		//{
		//	++player->data->tower.top_lv;
		//	next = player->data->tower.top_lv;
		//}
		
	}
	raid->data->ai_data.tower_data.state = TOWER_STATE_END;

	static const int MAX_ITEM_TOWER = 5;
	uint32_t item[MAX_ITEM_TOWER];
	uint32_t num[MAX_ITEM_TOWER];
	uint32_t itemAdd[MAX_ITEM_TOWER];
	uint32_t numAdd[MAX_ITEM_TOWER];
	uint32_t addExp = 0;
	uint32_t addMoney = 0;
	std::map<uint32_t, uint32_t> item_list; 
	TowerResult send;
	tower_result__init(&send);
	for (uint32_t l = player->data->tower.award_lv; l <= player->data->tower.cur_lv; ++l)
	{
		P20076Table *monTable = get_config_by_id(351601000 + l, &tower_level_config);
		if (monTable == NULL)
		{
			continue;
		}
		for (uint32_t i = 0; i < monTable->n_RewardNum1; ++i)
		{
			item_list.insert(std::make_pair(monTable->Reward1[i], monTable->RewardNum1[i]));
			if (l == player->data->tower.cur_lv)
			{
				item[i] = monTable->Reward1[i];
				num[i] = monTable->RewardNum1[i];
			}
		}
		for (uint32_t i = 0; i < monTable->n_RewardNum2; ++i)
		{
			item_list.insert(std::make_pair(monTable->Reward2[i], monTable->RewardNum2[i]));
			if (l == player->data->tower.cur_lv)
			{
				item[i] = monTable->Reward2[i];
				num[i] = monTable->RewardNum2[i];
			}
		}
		if (l == player->data->tower.cur_lv)
		{
			send.n_item_id = send.n_item_num = monTable->n_RewardNum1;
			send.n_item_add = send.n_item_num_add = monTable->n_RewardNum2; 
			send.exp = monTable->ExpReward;
			send.money = monTable->MoneyReward;
		}
		addExp += monTable->ExpReward;
		addMoney += monTable->MoneyReward;
	}
	send.top_lv = player->data->tower.top_lv;
	send.next = next;
	send.item_id = item;
	send.item_num = num;
	send.item_add = itemAdd;
	send.item_num_add = numAdd;
	send.cd = time_helper::get_cached_time() / 1000 - (raid->data->ai_data.tower_data.end - table->parameter1[2]);
	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	
	UpdateTower toFr;
	update_tower__init(&toFr);
	toFr.lv = player->data->tower.cur_lv;
	toFr.cd = send.cd;
	conn_node_gamesrv::send_to_friend(&extern_data, SERVER_PROTO_UPDATE_TOWER_REQUEST, &toFr, (pack_func)update_tower__pack);

	if (next == 0)
	{
		player->data->tower.cur_num = 0;
	}
	else
	{
		player->data->tower.award_lv = player->data->tower.cur_lv + 1;
		player->data->tower.cur_lv = next;
		player->data->tower.cur_num = table->parameter1[0];
		send.cur_num = table->parameter1[0];
	}
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_CLIMB_TOWER_RESULT_NOTIFY, tower_result__pack, send);

	player->add_item_list_as_much_as_possible(item_list, MSGIC_TYPE_TOWER);
	player->add_exp(addExp, MSGIC_TYPE_TOWER);
	player->add_coin(addMoney, MSGIC_TYPE_TOWER);

	
	
}
static void tower_raid_ai_collect(raid_struct *raid, player_struct *player, Collect *collect)
{

}

static void tower_raid_ai_player_ready(raid_struct *raid, player_struct *player)
{
	if (raid->data->state != RAID_STATE_START)
	{
		raid->player_leave_raid(player);
	 	return;
	}
	int ret = refresh_next_tower(player);
	if (ret != 0)
	{
		LOG_ERR("[%s:%d] ret = %d", __FUNCTION__, __LINE__, ret);
	}
}

static void tower_raid_ai_player_relive(raid_struct *raid, player_struct *player, uint32_t type)
{
	ReliveNotify nty;
	relive_notify__init(&nty);
	nty.playerid = player->get_uuid();
	nty.type = type;
	//if (type == 1)	//原地复活
	{
		if (raid->m_config->InstantRelive == 0)
		{
			LOG_ERR("%s player[%lu] can not relive type 1 in raid %u", __FUNCTION__, player->get_uuid(), raid->m_id);
			return;
		}

		int relive_times = player->get_attr(PLAYER_ATTR_RELIVE_TYPE1);
		if (relive_times >= sg_relive_free_times)
		{
			int fin_cost = (relive_times - sg_relive_free_times) * sg_relive_grow_cost + sg_relive_first_cost;
			if (fin_cost > sg_relive_max_cost)
				fin_cost = sg_relive_max_cost;
			if (player->get_comm_gold() < fin_cost)
			{
				LOG_ERR("%s: player[%lu] do not have enough gold", __FUNCTION__, player->get_uuid());
				return;
			}
			player->sub_comm_gold(fin_cost, MAGIC_TYPE_RELIVE);
		}

		++player->data->attrData[PLAYER_ATTR_RELIVE_TYPE1];
		++player->data->attrData[PLAYER_ATTR_RELIVE_TYPE2];
		player->broadcast_to_sight(MSG_ID_RELIVE_NOTIFY, &nty, (pack_func)relive_notify__pack, true);

	}
	
	player->data->attrData[PLAYER_ATTR_HP] = player->data->attrData[PLAYER_ATTR_MAXHP];
	player->on_hp_changed(0);

	//复活的时候加上一个无敌buff
	buff_manager::create_default_buff(114400001, player, player, false);
	player->m_team == NULL ? true : player->m_team->OnMemberHpChange(*player);
}

static void tower_raid_ai_attack(raid_struct *raid, player_struct *player, unit_struct *target, int damage)
{

}


//副本阵营战(阵营攻防)  新手阵营战
struct raid_ai_interface raid_ai_tower_interface =
{
	tower_raid_ai_init,
	tower_raid_ai_tick,
	tower_raid_ai_player_enter,
	tower_raid_ai_player_leave,
	tower_raid_ai_player_dead,
	tower_raid_ai_player_relive,
	tower_raid_ai_monster_dead,
	tower_raid_ai_collect,
	tower_raid_ai_player_ready,
	tower_raid_ai_finished,
	tower_raid_ai_attack,
	NULL,
	NULL, //护送结果
	NULL, //和npc对话
	NULL, //获取配置，主要是万妖谷的配置
	tower_raid_ai_failed, //失败
	NULL	
};
