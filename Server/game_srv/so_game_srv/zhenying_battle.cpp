#include "zhenying_battle.h"
#include <unistd.h>
#include <algorithm>
#include "comm.h"
#include "time_helper.h"
#include "player.h"
#include "player_manager.h"
#include "msgid.h"
#include "../proto/zhenying.pb-c.h"
#include "../proto/player_redis_info.pb-c.h"
#include "game_config.h"
#include "uuid.h"
#include "raid_manager.h"
#include "camp_judge.h"
#include "app_data_statis.h"
#include "zhenying_raid_manager.h"


void player_struct::send_zhenying_info()
{
	ZhenyingInfo send;
	zhenying_info__init(&send);
	send.zhenying = get_attr(PLAYER_ATTR_ZHENYING);
	send.level = data->zhenying.level;
	send.exp = data->zhenying.exp;
	send.task = data->zhenying.task;
	send.task_type = data->zhenying.task_type;
	send.task_num = data->zhenying.task_num;
	send.step =data->zhenying.step;
	send.free_change = data->zhenying.free;
	EXTERN_DATA extern_data;
	extern_data.player_id = get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_INFO_NOTIFY, zhenying_info__pack, send);
}

void player_struct::add_zhenying_exp(uint32_t num)
{
	if (get_attr(PLAYER_ATTR_ZHENYING) == 0)
	{
		return;
	}
	CampTable *config = get_config_by_id(360101001, &zhenying_base_config);
	if (config != NULL)
	{
		if (data->zhenying.exp_day >= config->MaxExp)
		{
			return;
		}
	}

	GradeTable *table = get_config_by_id((36020 + get_attr(PLAYER_ATTR_ZHENYING)) * 10000 + data->zhenying.level, &zhenying_level_config); 
	if (table != NULL && table->LevelExp == 0)
	{
		return;
	}
	data->zhenying.exp += num;
	data->zhenying.exp_day += num;
	while (table != NULL && data->zhenying.exp >= table->LevelExp && table->LevelExp > 0)
	{
		data->zhenying.exp -= table->LevelExp;
		++data->zhenying.level;
		table = get_config_by_id((36020 + get_attr(PLAYER_ATTR_ZHENYING)) * 10000 + data->zhenying.level, &zhenying_level_config);
		add_achievement_progress(ACType_ZHENYING_GRADE, table->Level, 0, 1);
	}

	ZhenyingExp send;
	zhenying_exp__init(&send);
	send.exp = data->zhenying.exp;
	send.exp_day = data->zhenying.exp_day;
	send.level = data->zhenying.level;
	EXTERN_DATA extern_data;
	extern_data.player_id = get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_EXP_NOTIFY, zhenying_exp__pack, send);
}

void player_struct::refresh_zhenying_task_oneday()
{
	data->zhenying.task_num = 0;
	data->zhenying.kill = 0;

	NewZhenyingTask send;
	new_zhenying_task__init(&send);
	send.task = data->zhenying.task;
	send.task_type = data->zhenying.task_type;
	send.task_num = data->zhenying.task_num;
	EXTERN_DATA extern_data;
	extern_data.player_id = get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_NEW_ZHENYING_TASK_NOTIFY, new_zhenying_task__pack, send);

}












//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ZhenyingBattle *ZhenyingBattle::ins;
PRIVATE_BATTLE_T ZhenyingBattle::s_private;
const uint32_t ZhenyingBattle::s_border[MAX_STEP] = { 49,69,89,0 };

ZhenyingBattle::ZhenyingBattle()
{
	m_nextTick = time_helper::get_cached_time() / 1000 + 60;
	m_state = REST_STATE;
}
ZhenyingBattle *ZhenyingBattle::GetInstance()
{
	if (ins == NULL)
	{
		ins = new ZhenyingBattle;
	}
	return ins;
}

void ZhenyingBattle::Join(player_struct &player)
{
	//int i = 0;
	//for (; i < MAX_STEP - 1; ++i)
	//{
	//	if (player.get_attr(PLAYER_ATTR_LEVEL) <= s_border[i])
	//	{
	//		break;
	//	}
	//}
	//if (m_join[i].find(player.get_uuid()) != m_join[i].end())
	//{
	//	return;
	//}

	//todo 判断状态
	if (m_allJoin.find(player.get_uuid()) != m_allJoin.end())
	{
		return;
	}
	BATTLE_JOINER tmp;
	tmp.room = 0;
	tmp.kill = 0;
	tmp.dead = 0;
	tmp.point = 0;
	tmp.lv = player.get_attr(PLAYER_ATTR_LEVEL);
	tmp.zhenying = player.get_attr(PLAYER_ATTR_ZHENYING);
	//m_join[i].insert(std::make_pair(player.get_uuid(), tmp));
	m_allJoin.insert(std::make_pair(player.get_uuid(), tmp));
}

void ZhenyingBattle::CreateRoom()
{
	uint32_t roomId = 1;
	STEP_JOIN_T stepJoin[MAX_STEP];
	JOIN_T::iterator it = m_allJoin.begin();
	for (; it != m_allJoin.end(); ++it) //按等级分组
	{
		int s = 0;
		for (; s < MAX_STEP - 1; ++s)
		{
			if (it->second.lv <= s_border[s])
			{
				break;
			}
		}
		stepJoin[s].push_back(it->first);
	}
	for (int i = 0; i < MAX_STEP; ++i)
	{	
		std::vector<uint64_t> fulongguoSide;
		std::vector<uint64_t> dianfengguSide;
		for (STEP_JOIN_T::iterator itStep = stepJoin[i].begin(); itStep != stepJoin[i].end(); ++itStep) //相同阵营的放到一起
		{
			it = m_allJoin.find(*itStep);
			if (it == m_allJoin.end())
			{
				continue;
			}
			if (it->second.zhenying == ZHENYING__TYPE__FULONGGUO)
			{
				fulongguoSide.push_back(it->first);
			}
			else
			{
				dianfengguSide.push_back(it->first);
			}
		}
		std::vector<uint64_t>::iterator itSflg = fulongguoSide.begin();
		std::vector<uint64_t>::iterator itSdfg = dianfengguSide.begin();
		for (; itSdfg !=  dianfengguSide.end() || itSflg != fulongguoSide.end();) //分房间
		{
			ROOM_T::iterator itRoom = m_room.find(roomId);
			if (itRoom == m_room.end())
			{
				ROOM_INFO info;
				info.step = i;
				info.fighter[0].clear();
				info.fighter[1].clear();
				info.uid = 0;
				info.totalPoint[0] = 0;
				info.totalPoint[1] = 0;
				BattlefieldTable *table = get_config_by_id(i + 360500001, &zhenying_fight_config);
				if (table != NULL)
				{
					info.flag[0].id = table->MineSet[2];
					info.flag[0].npc = table->MineSet[3];
					info.flag[0].state = BATTLE_FLAG_NORMOR;
					
					info.flag[1].id = table->ForestSet[2];
					info.flag[1].npc = table->ForestSet[3];
					info.flag[1].state = BATTLE_FLAG_NORMOR;

					info.flag[2].id = table->WarSet[2];
					info.flag[2].npc = table->WarSet[3];
					info.flag[2].state = BATTLE_FLAG_NORMOR;
				}
	
				std::pair<ROOM_T::iterator, bool> ret = m_room.insert(std::make_pair(roomId, info));
				itRoom = ret.first;
			}
			if (itSdfg != dianfengguSide.end() && itRoom->second.fighter[ZHENYING__TYPE__WANYAOGU - 1].size() < MAX_ROOM_SIDE)
			{
				it = m_allJoin.find(*itSdfg);
				if (it != m_allJoin.end())
				{
					it->second.room = roomId;
				}
				++itSdfg;
				itRoom->second.fighter[ZHENYING__TYPE__WANYAOGU - 1].push_back(it->first);
			}
			if (itSflg != fulongguoSide.end() && itRoom->second.fighter[ZHENYING__TYPE__FULONGGUO - 1].size() < MAX_ROOM_SIDE)
			{
				it = m_allJoin.find(*itSflg);
				if (it != m_allJoin.end())
				{
					it->second.room = roomId;
				}
				++itSflg;
				itRoom->second.fighter[ZHENYING__TYPE__FULONGGUO - 1].push_back(it->first);
			}
			if (itRoom->second.fighter[0].size() >= MAX_ROOM_SIDE || itRoom->second.fighter[1].size() >= MAX_ROOM_SIDE) //新房间 
			{
				++roomId;
			}
		}
	}
}

int ZhenyingBattle::IntoBattle(player_struct &player)
{
	if (player.get_attr(PLAYER_ATTR_ZHENYING) == 0)
	{
		return 7;
	}
	
	JOIN_T::iterator it = m_allJoin.find(player.get_uuid());
	if (it == m_allJoin.end())
	{
		return 1;
	}
	if (it->second.room == 0)
	{
		return 2;
	}
	raid_struct *raid = NULL;
	ROOM_T::iterator itRoom = m_room.find(it->second.room);
	if (itRoom == m_room.end()) //
	{
		return 5;
	}
	BattlefieldTable *table = get_config_by_id(itRoom->second.step + 360500001, &zhenying_fight_config);
	if (table == NULL)
	{
		return 6;
	}
	//if (m_state != RUN_STATE || m_nextTick > table->ReadyTime + time_helper::get_cached_time() / 1000)
	//{
	//	return 8;
	//}
	int ret = raid_manager::check_player_enter_raid(&player, table->Map);
	if (ret != 0)
	{
		return ret;
	}
	if (player.m_team != NULL)
	{
		player.m_team->RemoveMember(player);
	}
	if (itRoom->second.uid == 0)
	{	
		raid = raid_manager::create_raid(table->Map, &player);
		if (raid == NULL)
		{
			LOG_ERR("%s: create raid failed", __FUNCTION__);
			return 3;
		}
		raid->data->ai_data.battle_data.room = itRoom->first;
		raid->data->ai_data.battle_data.step = itRoom->second.step;
		itRoom->second.uid = raid->data->uuid;

		int name_index = random();
		for (size_t i = itRoom->second.fighter[ZHENYING__TYPE__FULONGGUO - 1].size(); i < MAX_ROOM_SIDE; ++i)
		{
			player_struct *rob = player_manager::create_ai_player(&player, NULL, name_index++);
			rob->set_attr(PLAYER_ATTR_ZHENYING, ZHENYING__TYPE__FULONGGUO);
			rob->set_attr(PLAYER_ATTR_PK_TYPE, 1);
			raid->player_enter_raid_impl(rob, 0, (int)table->BirthPoint1[0] + 2 - random() % 5, table->BirthPoint1[2] + 2 - random() % 5);
			BATTLE_JOINER tmp;
			tmp.room = itRoom->first;
			tmp.kill = 0;
			tmp.dead = 0;
			tmp.point = 0;
			tmp.lv = rob->get_attr(PLAYER_ATTR_LEVEL);
			tmp.zhenying = rob->get_attr(PLAYER_ATTR_ZHENYING);
			m_allJoin.insert(std::make_pair(rob->get_uuid(), tmp));
			itRoom->second.fighter[ZHENYING__TYPE__FULONGGUO - 1].push_back(rob->get_uuid());
		}
		for (size_t i = itRoom->second.fighter[ZHENYING__TYPE__WANYAOGU - 1].size(); i < MAX_ROOM_SIDE; ++i)
		{
			player_struct *rob = player_manager::create_ai_player(&player, NULL, name_index++);
			rob->set_attr(PLAYER_ATTR_ZHENYING, ZHENYING__TYPE__WANYAOGU);
			rob->set_attr(PLAYER_ATTR_PK_TYPE, 1);
			raid->player_enter_raid_impl(rob, 0, (int)table->BirthPoint2[0] + 2 - random() % 5, table->BirthPoint2[2] + 2 - random() % 5);
			BATTLE_JOINER tmp;
			tmp.room = itRoom->first;
			tmp.kill = 0;
			tmp.dead = 0;
			tmp.point = 0;
			tmp.lv = rob->get_attr(PLAYER_ATTR_LEVEL);
			tmp.zhenying = rob->get_attr(PLAYER_ATTR_ZHENYING);
			m_allJoin.insert(std::make_pair(rob->get_uuid(), tmp));
			itRoom->second.fighter[ZHENYING__TYPE__WANYAOGU - 1].push_back(rob->get_uuid());
		}
		

		//rob = player_manager::create_ai_player(&player, NULL, 2);
		//rob->set_attr(PLAYER_ATTR_ZHENYING, ZHENYING__TYPE__WANYAOGU);
		//raid->player_enter_raid_impl(rob, 1, (int)table->BirthPoint2[0], table->BirthPoint2[2]);
	}
	else
	{
		raid = raid_manager::get_raid_by_uuid(itRoom->second.uid);
		if (raid == NULL)
		{
			return 4;
		}
	}
	int x, z;
	if (it->second.zhenying == ZHENYING__TYPE__FULONGGUO)
	{
		x = table->BirthPoint1[0];
		z = table->BirthPoint1[2];
	}
	else
	{
		x = table->BirthPoint2[0];
		z = table->BirthPoint2[2];
	}
	it->second.in = true;
	if (player.get_attr(PLAYER_ATTR_PK_TYPE) != PK_TYPE_CAMP)
	{
		player.set_attr(PLAYER_ATTR_PK_TYPE, PK_TYPE_CAMP);
		player.broadcast_one_attr_changed(PLAYER_ATTR_PK_TYPE, PK_TYPE_CAMP, true, true);
	}
	raid->player_enter_raid_impl(&player, 0, x, z);
	return 0;
}

void ZhenyingBattle::BroadMessageRoom(uint32_t room, uint16_t msg_id, void *msg_data, pack_func func, uint64_t except)
{
	ROOM_T::iterator itRoom = m_room.find(room);
	if (itRoom == m_room.end())
	{
		return;
	}
	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(msg_id, msg_data, func);
	PROTO_HEAD_CONN_BROADCAST *head;
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
	for (int i = 0; i < 2; ++i)
	{
		for (ROOM_MAN_T::iterator it = itRoom->second.fighter[i].begin(); it != itRoom->second.fighter[i].end(); ++it)
		{
			if (get_entity_type(*it) == ENTITY_TYPE_AI_PLAYER)
			{
				continue;
			}
			player_struct *player = player_manager::get_player_by_id(*it);
			if (player == NULL || player->scene == NULL)
			{
				continue;
			}
			if (player->scene->get_scene_type() != SCENE_TYPE_RAID)
			{
				continue;
			}
			raid_struct *raid = (raid_struct *)player->scene;
			if (raid->m_config->DengeonRank != DUNGEON_TYPE_BATTLE && raid->m_config->DengeonRank != DUNGEON_TYPE_BATTLE_NEW)
			{
				continue;
			}
			ppp[(head->num_player_id)++] = player->get_uuid();
		}
	}
	if (head->num_player_id > 0)
	{
		head->len += sizeof(uint64_t) * head->num_player_id;
		conn_node_gamesrv::broadcast_msg_send();
	}
}

void ZhenyingBattle::OnRegionChanged(raid_struct *raid, player_struct *player, uint32_t old_region, uint32_t new_region)
{
	if (raid == NULL)
	{
		return;
	}
	if (player == NULL)
	{
		return;
	}
	ROOM_T::iterator itRoom = m_room.find(raid->data->ai_data.battle_data.room);
	if (itRoom == m_room.end())
	{
		return;
	}
	BattlefieldTable *table = get_config_by_id(itRoom->second.step + 360500001, &zhenying_fight_config);
	if (table == NULL)
	{
		return;
	}
	StartFlag send;
	start_flag__init(&send);
	uint16_t msgId = 0;
	int zhenying = player->get_attr(PLAYER_ATTR_ZHENYING) - 1;
	send.zhenying = player->get_attr(PLAYER_ATTR_ZHENYING);
	for (uint32_t i = 0; i < MAX_ROOM_FLAG; ++i)
	{
		if (itRoom->second.flag[i].id == new_region)
		{
			if (itRoom->second.flag[i].state == BATTLE_FLAG_NORMOR)
			{
				msgId = MSG_ID_ZHENYING_FIGHT_START_FLAG_NOTIFY;
				itRoom->second.flag[i].startTime = time_helper::get_cached_time() / 1000;
				itRoom->second.flag[i].state = BATTLE_FLAG_GATHERING;
				itRoom->second.flag[i].own = player->get_attr(PLAYER_ATTR_ZHENYING);
				itRoom->second.flag[i].playerarr[zhenying].insert(player->get_uuid());
				send.cd = table->ForestSet[4];
				
				send.npc = itRoom->second.flag[i].npc;
				send.region = itRoom->second.flag[i].id;
				BroadMessageRoom(itRoom->first, msgId, &send, (pack_func)start_flag__pack);
			}
			else if (itRoom->second.flag[i].state == BATTLE_FLAG_GATHERING)
			{
				itRoom->second.flag[i].playerarr[zhenying].insert(player->get_uuid());
			}
		}
		else if (itRoom->second.flag[i].id == old_region)
		{
			if (itRoom->second.flag[i].state != BATTLE_FLAG_GATHERING)
			{
				continue;
			}
			itRoom->second.flag[i].playerarr[zhenying].erase(player->get_uuid());
			if (itRoom->second.flag[i].playerarr[zhenying].empty() && itRoom->second.flag[i].own == player->get_attr(PLAYER_ATTR_ZHENYING))
			{
				msgId = MSG_ID_ZHENYING_FIGHT_INTERUPT_FLAG_NOTIFY;
				itRoom->second.flag[i].state = BATTLE_FLAG_NORMOR;
				send.npc = itRoom->second.flag[i].npc;
				send.region = itRoom->second.flag[i].id;
				BroadMessageRoom(itRoom->first, msgId, &send, (pack_func)start_flag__pack);
				if (itRoom->second.flag[i].own == ZHENYING__TYPE__FULONGGUO)
				{
					itRoom->second.flag[i].own = ZHENYING__TYPE__WANYAOGU;
				}
				else
				{
					itRoom->second.flag[i].own = ZHENYING__TYPE__FULONGGUO;
				}
				if (!itRoom->second.flag[i].playerarr[itRoom->second.flag[i].own - 1].empty())
				{
					msgId = MSG_ID_ZHENYING_FIGHT_START_FLAG_NOTIFY;
					itRoom->second.flag[i].startTime = time_helper::get_cached_time() / 1000;
					itRoom->second.flag[i].state = BATTLE_FLAG_GATHERING;
					send.cd = table->ForestSet[4];
					send.zhenying = itRoom->second.flag[i].own;
					
					send.npc = itRoom->second.flag[i].npc;
					send.region = itRoom->second.flag[i].id;
					BroadMessageRoom(itRoom->first, msgId, &send, (pack_func)start_flag__pack);
				}
			}
		}
		//if (change)
		//{
		//	send.npc = itRoom->second.flag[i].npc;
		//	send.region = itRoom->second.flag[i].id;
		//	BroadMessageRoom(itRoom->first, msgId, &send, (pack_func)start_flag__pack);
		//}
	}
}

void ZhenyingBattle::Settle(scene_struct *scence, uint32_t room)
{
	if (scence == NULL)
	{
		return;
	}
	ROOM_T::iterator itRoom = m_room.find(room);
	if (itRoom == m_room.end())
	{
		return;
	}
	OneScore side[2][MAX_ROOM_SIDE];
	OneScore *sidePoint[2][MAX_ROOM_SIDE];
	ZhenYingResult send;
	zhen_ying_result__init(&send);
	TotalScore total;
	total_score__init(&total);
	total.fulongguo = itRoom->second.totalPoint[ZHENYING__TYPE__FULONGGUO - 1];
	total.dianfenggu = itRoom->second.totalPoint[ZHENYING__TYPE__WANYAOGU - 1];
	send.score = &total;
	for (int i = 0; i < 2; ++i)
	{
		for (size_t c = 0; c < itRoom->second.fighter[i].size(); ++c)
		{
			one_score__init(&side[i][c]);
			side[i][c].playerid = itRoom->second.fighter[i][c];
			player_struct *player = player_manager::get_player_by_id(side[i][c].playerid);
			if (player != NULL)
			{
				side[i][c].name = player->get_name();
				side[i][c].online = true;
			}
			else
			{
				side[i][c].name = (char *)g_tmp_name;
			}
			JOIN_T::iterator itJ = m_allJoin.find(side[i][c].playerid);
			if (itJ != m_allJoin.end())
			{
				side[i][c].kill = itJ->second.kill;
				side[i][c].death = itJ->second.dead;
				side[i][c].score = itJ->second.point;
			}
			side[i][c].rank = c + 1;
			sidePoint[i][c] = &side[i][c];
		}
	}
	send.fulongguo = sidePoint[ZHENYING__TYPE__FULONGGUO - 1];
	send.dianfenggu = sidePoint[ZHENYING__TYPE__WANYAOGU - 1];
	send.n_fulongguo = itRoom->second.fighter[ZHENYING__TYPE__FULONGGUO - 1].size();
	send.n_dianfenggu = itRoom->second.fighter[ZHENYING__TYPE__WANYAOGU - 1].size();
	if (total.fulongguo > total.dianfenggu)
	{
		send.winer = ZHENYING__TYPE__FULONGGUO;
	}
	else
	{
		send.winer = ZHENYING__TYPE__WANYAOGU;
	}
	raid_struct *raid = (raid_struct *)scence;
	ItemData item_data[4];
	ItemData *item_data_point[4];
	if (raid->m_config->DengeonRank == DUNGEON_TYPE_BATTLE)
	{
		for (int i = 0; i < 2; ++i)
		{
			for (size_t c = 0; c < itRoom->second.fighter[i].size(); ++c)
			{
				if (get_entity_type(itRoom->second.fighter[i][c]) == ENTITY_TYPE_AI_PLAYER)
				{
					continue;
				}
				player_struct *player = player_manager::get_player_by_id(itRoom->second.fighter[i][c]);
				if (player != NULL)
				{
					if (player->scene == NULL)
					{
						continue;
					}
					if (player->scene->get_scene_type() != SCENE_TYPE_RAID)
					{
						continue;
					}
					raid_struct *raid = (raid_struct *)player->scene;
					if (raid->m_config->DengeonRank != DUNGEON_TYPE_BATTLE && raid->m_config->DengeonRank != DUNGEON_TYPE_BATTLE_NEW)
					{
						continue;
					}

					if (!player->data->zhenying.one_award && player->get_attr(PLAYER_ATTR_ZHENYING) == send.winer)
					{
						player->data->zhenying.one_award = true; //todo 真实奖励
						for (int i = 0; i < 4; ++i)
						{
							item_data_point[i] = &(item_data[i]);
							item_data__init(item_data_point[i]);
							item_data[i].id = 201070074 + i;
							item_data[i].num = 1;
						}
						send.n_item = 4;
						send.item = item_data_point;
					}
					else
					{
						send.n_item = 0;
						send.item = NULL;
					}

					
					EXTERN_DATA extern_data;
					extern_data.player_id = player->get_uuid();
					fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_FIGHT_SETTLE_NOTIFY, zhen_ying_result__pack, send);
				}
			}
		}
	}

	//BroadMessageRoom(room, MSG_ID_ZHENYING_FIGHT_SETTLE_NOTIFY, &send, (pack_func)zhen_ying_result__pack);
}

uint32_t ZhenyingBattle::GetStep(player_struct &player)
{
	JOIN_T::iterator it = m_allJoin.find(player.get_uuid());
	if (it == m_allJoin.end())
	{
		return 0;
	}
	if (it->second.room == 0)
	{
		return 0;
	}
	ROOM_T::iterator itRoom = m_room.find(it->second.room);
	if (itRoom == m_room.end()) 
	{
		return 0;
	}
	return itRoom->second.step;
}

void ZhenyingBattle::SendMyScore(player_struct &player)
{
	JOIN_T::iterator it = m_allJoin.find(player.get_uuid());
	if (it == m_allJoin.end())
	{
		return ;
	}
	if (it->second.room == 0)
	{
		return ;
	}
	ROOM_T::iterator itRoom = m_room.find(it->second.room);
	if (itRoom == m_room.end()) 
	{
		return ;
	}
	int zhenying = player.get_attr(PLAYER_ATTR_ZHENYING) - 1;
	if (zhenying >= 2 || zhenying < 0)
	{
		return;
	}
	ROOM_MAN_T::iterator itFig = itRoom->second.fighter[zhenying].begin();
	for (; itFig != itRoom->second.fighter[zhenying].end(); ++itFig)
	{
		if (*itFig == player.get_uuid())
		{
			break;
		}
	}
	if (itFig == itRoom->second.fighter[zhenying].end())
	{
		return;
	}
	int rank = std::distance(itFig, itRoom->second.fighter[zhenying].begin());
	MyScore send;
	my_score__init(&send);
	send.rank = rank + 1;
	send.point = it->second.point;
	EXTERN_DATA extern_data;
	extern_data.player_id = player.get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_FIGHT_MY_SCORE_NOTIFY, my_score__pack, send);
}

void ZhenyingBattle::GetMySideScore(player_struct &player)
{
	JOIN_T::iterator it = m_allJoin.find(player.get_uuid());
	if (it == m_allJoin.end())
	{
		return;
	}
	if (it->second.room == 0)
	{
		return;
	}
	ROOM_T::iterator itRoom = m_room.find(it->second.room);
	if (itRoom == m_room.end())
	{
		return;
	}
	int zhenying = player.get_attr(PLAYER_ATTR_ZHENYING) - 1;
	if (zhenying >= 2 || zhenying < 0)
	{
		return;
	}

	OneScore side[MAX_ROOM_SIDE + 1];
	OneScore *sidePoint[MAX_ROOM_SIDE + 1];
	SideScore send;
	side_score__init(&send);

	for (size_t c = 0; c < itRoom->second.fighter[zhenying].size(); ++c)
	{
		one_score__init(&side[c]);
		side[c].playerid = itRoom->second.fighter[zhenying][c];
		player_struct *player = player_manager::get_player_by_id(side[c].playerid);
		if (player != NULL)
		{
			side[c].name = player->get_name();
			side[c].online = true;
		}
		else
		{
			side[c].name = (char *)g_tmp_name;
		}
		JOIN_T::iterator itJ = m_allJoin.find(side[c].playerid);
		if (itJ != m_allJoin.end())
		{
			side[c].kill = itJ->second.kill;
			side[c].death = itJ->second.dead;
			side[c].score = itJ->second.point;
		}
		side[c].rank = c + 1;
		sidePoint[c] = &side[c];
	}
	send.total = itRoom->second.totalPoint[zhenying];
	send.side = sidePoint;
	send.n_side = itRoom->second.fighter[zhenying].size();

	EXTERN_DATA extern_data;
	extern_data.player_id = player.get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_FIGHT_MYSIDE_SCORE_NOTIFY, side_score__pack, send);
}

BATTLE_JOINER *ZhenyingBattle::GetJoins(uint64_t pid)
{
	JOIN_T::iterator it = m_allJoin.find(pid);
	if (it == m_allJoin.end())
	{
		return NULL;
	}
	return &it->second;
}
bool SortFighterPoint(uint64_t left, uint64_t right)
{
	const BATTLE_JOINER *joinL = ZhenyingBattle::GetInstance()->GetJoins(left);
	if (joinL == NULL)
	{
		return false;
	}
	const BATTLE_JOINER *joinR = ZhenyingBattle::GetInstance()->GetJoins(right);
	if (joinR == NULL)
	{
		return false;
	}
	if (joinL->point > joinR->point)
	{
		return true;
	} 
	else
	{
		return false;
	}
}
void ZhenyingBattle::KillEnemy(player_struct &player, player_struct &dead)
{
	JOIN_T::iterator it = m_allJoin.find(player.get_uuid());
	if (it == m_allJoin.end())
	{
		return;
	}
	if (it->second.room == 0)
	{
		return;
	}
	ROOM_T::iterator itRoom = m_room.find(it->second.room);
	if (itRoom == m_room.end())
	{
		return;
	}
	int zhenying = player.get_attr(PLAYER_ATTR_ZHENYING) - 1;
	if (zhenying >= 2 || zhenying < 0)
	{
		return;
	}
	BattlefieldTable *table = get_config_by_id(itRoom->second.step + 360500001, &zhenying_fight_config);
	if (table == NULL)
	{
		return;
	}

	++it->second.kill;
	it->second.point += table->Kill;
	itRoom->second.totalPoint[zhenying] += table->Kill;
	TotalScore notify;
	total_score__init(&notify);
	notify.fulongguo = itRoom->second.totalPoint[ZHENYING__TYPE__FULONGGUO - 1];
	notify.dianfenggu = itRoom->second.totalPoint[ZHENYING__TYPE__WANYAOGU - 1];
	std::sort(itRoom->second.fighter[zhenying].begin(), itRoom->second.fighter[zhenying].end(), SortFighterPoint);
	SendMyScore(player);
	BroadMessageRoom(itRoom->first, MSG_ID_ZHENYING_FIGHT_SCORE_NOTIFY, &notify, (pack_func)total_score__pack);

	StartFlag send;
	start_flag__init(&send);
	uint16_t msgId = 0;
	send.zhenying = player.get_attr(PLAYER_ATTR_ZHENYING);
	zhenying = dead.get_attr(PLAYER_ATTR_ZHENYING) - 1;
	for (uint32_t i = 0; i < MAX_ROOM_FLAG; ++i)
	{
		if (itRoom->second.flag[i].state != BATTLE_FLAG_GATHERING)
		{
			continue;
		}
		itRoom->second.flag[i].playerarr[zhenying].erase(dead.get_uuid());
		if (itRoom->second.flag[i].playerarr[zhenying].empty() && itRoom->second.flag[i].own == dead.get_attr(PLAYER_ATTR_ZHENYING))
		{
			msgId = MSG_ID_ZHENYING_FIGHT_INTERUPT_FLAG_NOTIFY;
			itRoom->second.flag[i].state = BATTLE_FLAG_NORMOR;
			send.npc = itRoom->second.flag[i].npc;
			send.region = itRoom->second.flag[i].id;
			BroadMessageRoom(itRoom->first, msgId, &send, (pack_func)start_flag__pack);
			if (itRoom->second.flag[i].own == ZHENYING__TYPE__FULONGGUO)
			{
				itRoom->second.flag[i].own = ZHENYING__TYPE__WANYAOGU;
			}
			else
			{
				itRoom->second.flag[i].own = ZHENYING__TYPE__FULONGGUO;
			}
			if (!itRoom->second.flag[i].playerarr[itRoom->second.flag[i].own - 1].empty())
			{
				msgId = MSG_ID_ZHENYING_FIGHT_START_FLAG_NOTIFY;
				itRoom->second.flag[i].startTime = time_helper::get_cached_time() / 1000;
				itRoom->second.flag[i].state = BATTLE_FLAG_GATHERING;
				send.cd = table->ForestSet[4];
				send.zhenying = itRoom->second.flag[i].own;
				send.npc = itRoom->second.flag[i].npc;
				send.region = itRoom->second.flag[i].id;
				BroadMessageRoom(itRoom->first, msgId, &send, (pack_func)start_flag__pack);
			}
		}
	}
}

void ZhenyingBattle::GmStartBattle()
{
	if (m_state == RUN_STATE)
	{
		return;
	}
	m_nextTick = 0;
	m_state = REST_STATE;
	ClearRob();
	m_room.clear();
}

void ZhenyingBattle::FlagOnTick()
{
	uint64_t now = time_helper::get_cached_time() / 1000;
	if (m_state != RUN_STATE)
	{
		return;
	}
	ROOM_T::iterator itRoom = m_room.begin();
	for (; itRoom != m_room.end(); ++itRoom)
	{
		BattlefieldTable *table = get_config_by_id(itRoom->second.step + 360500001, &zhenying_fight_config);
		if (table == NULL)
		{
			continue;
		}
		for (uint32_t i = 0; i < MAX_ROOM_FLAG; ++i)
		{
			if (itRoom->second.flag[i].state == BATTLE_FLAG_GATHERING)
			{
				if (itRoom->second.flag[i].startTime + table->ForestSet[4] < now)
				{
					itRoom->second.flag[i].state = BATTLE_FLAG_COMMPLETE;
					StartFlag send;
					start_flag__init(&send);
					send.zhenying = itRoom->second.flag[i].own;
					send.npc = itRoom->second.flag[i].npc;
					send.region = itRoom->second.flag[i].id;
					BroadMessageRoom(itRoom->first, MSG_ID_ZHENYING_FIGHT_FINISH_FLAG_NOTIFY, &send, (pack_func)start_flag__pack);
					itRoom->second.flag[i].startTime = now + table->ForestSet[0];
				}
			}
			else if (itRoom->second.flag[i].state == BATTLE_FLAG_COMMPLETE)
			{
				int zhenying = itRoom->second.flag[i].own - 1;
				if (itRoom->second.flag[i].startTime < now)
				{
					itRoom->second.totalPoint[zhenying] += table->ForestSet[1] * MAX_ROOM_SIDE;
					itRoom->second.flag[i].startTime = now + table->ForestSet[0];
					TotalScore notify;
					total_score__init(&notify);
					notify.fulongguo = itRoom->second.totalPoint[ZHENYING__TYPE__FULONGGUO - 1];
					notify.dianfenggu = itRoom->second.totalPoint[ZHENYING__TYPE__WANYAOGU - 1];
					BroadMessageRoom(itRoom->first, MSG_ID_ZHENYING_FIGHT_SCORE_NOTIFY, &notify, (pack_func)total_score__pack);

					for (ROOM_MAN_T::iterator it = itRoom->second.fighter[zhenying].begin(); it != itRoom->second.fighter[zhenying].end(); ++it)
					{
						BATTLE_JOINER *join = GetJoins(*it);
						if (join == NULL)
						{
							continue;
						}
						join->point += table->ForestSet[1];
						if (get_entity_type(*it) == ENTITY_TYPE_AI_PLAYER)
						{
							continue;
						}
						player_struct *player = player_manager::get_player_by_id(*it);
						if (player == NULL || player->scene == NULL)
						{
							continue;
						}
						if (player->scene->get_scene_type() != SCENE_TYPE_RAID)
						{
							continue;
						}
						raid_struct *raid = (raid_struct *)player->scene;
						if (raid->m_config->DengeonRank != DUNGEON_TYPE_BATTLE && raid->m_config->DengeonRank != DUNGEON_TYPE_BATTLE_NEW)
						{
							continue;
						}
						SendMyScore(*player);
					}
				}
			}
		}
	}
}
void ZhenyingBattle::Tick()
{
	FlagOnTick();

	uint64_t now = time_helper::get_cached_time() / 1000;
	if (now > m_nextTick)
	{
		BattlefieldTable *table = zhenying_fight_config.begin()->second;
		if (table == NULL)
		{
			return;
		}
		ZhenyingExp send;
		zhenying_exp__init(&send);
		switch (m_state)
		{
		case JOIN_STATE:
		{

			DungeonTable* config = get_config_by_id(table->Map, &all_raid_config);
			if (config != NULL)
			{
				m_nextTick = now + config->FailValue[0];
			}

			m_state = RUN_STATE;
			uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_ZHENYING_FIGHT_START_NOTIFY, &send, (pack_func)zhenying_exp__pack);
			PROTO_HEAD_CONN_BROADCAST *head;
			head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
			for (JOIN_T::iterator it = m_allJoin.begin(); it != m_allJoin.end(); ++it)
			{
				player_struct *p = player_manager::get_player_by_id(it->first);
				if (p == NULL)
				{
					continue;
				}
				ppp[(head->num_player_id)++] = it->first;
			}
			if (head->num_player_id > 0)
			{
				head->len += sizeof(uint64_t) * head->num_player_id;
				conn_node_gamesrv::broadcast_msg_send();
			}
			//conn_node_gamesrv::send_to_all_player(MSG_ID_ZHENYING_FIGHT_START_NOTIFY, &send, (pack_func)zhenying_exp__pack);
			CreateRoom();
		}
			break;
		case RUN_STATE:
			m_nextTick = now + 3600;
			m_state = REST_STATE; 
			ClearRob();
			m_room.clear();
			break;
		case REST_STATE:
		{
			m_nextTick = now + 10;
			m_state = JOIN_STATE;
			
			uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_JOIN_ZHENYING_FIGHT_NOTIFY, &send, (pack_func)zhenying_exp__pack);
			PROTO_HEAD_CONN_BROADCAST *head;
			head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
			std::map<uint64_t, player_struct *>::iterator it = player_manager_all_players_id.begin();
			for (; it != player_manager_all_players_id.end(); ++it)
			{
				if (it->second->get_attr(PLAYER_ATTR_ZHENYING) == 0 || it->second->get_attr(PLAYER_ATTR_LEVEL) < table->LowerLimitLv)
				{
					continue;
				}

				ppp[(head->num_player_id)++] = it->second->get_uuid();
			}
			if (head->num_player_id > 0)
			{
				head->len += sizeof(uint64_t) * head->num_player_id;
				conn_node_gamesrv::broadcast_msg_send();
			}
			//conn_node_gamesrv::send_to_all_player(MSG_ID_JOIN_ZHENYING_FIGHT_NOTIFY, &send, (pack_func)zhenying_exp__pack);
		}
			break;
		}
	}
}

ZhenyingBattle *ZhenyingBattle::CreatePrivateBattle(player_struct &player, raid_struct *raid)
{
	ZhenyingBattle *battle = new ZhenyingBattle;
	s_private.insert(std::make_pair(raid->data->uuid, battle));
	battle->Join(player);
	battle->CreateRoom();
	battle->m_state = RUN_STATE;
	BATTLE_JOINER *join = battle->GetJoins(player.get_uuid());
	if (join == NULL)
	{
		delete battle;
		return NULL;
	}
	ROOM_T::iterator itRoom = battle->m_room.find(join->room);
	if (itRoom == battle->m_room.end())
	{
		delete battle;
		return NULL;
	}
	
	raid->data->ai_data.battle_data.room = itRoom->first;
	raid->data->ai_data.battle_data.step = itRoom->second.step;

	BattlefieldTable *table = get_config_by_id(itRoom->second.step + 360500001, &zhenying_fight_config);
	if (table != NULL)
	{
		DungeonTable* config = get_config_by_id(table->Map, &all_raid_config);
		if (config != NULL)
		{
			battle->m_nextTick = time_helper::get_cached_time() / 1000 + config->FailValue[0];
		}

		int x, z;
		if (join->zhenying == ZHENYING__TYPE__FULONGGUO)
		{
			x = table->BirthPoint1[0];
			z = table->BirthPoint1[2];
		}
		else
		{
			x = table->BirthPoint2[0];
			z = table->BirthPoint2[2];
		}
		player_struct *rob = player_manager::create_ai_player(&player, NULL, 1);
		rob->set_attr(PLAYER_ATTR_ZHENYING, ZHENYING__TYPE__FULONGGUO);
		raid->player_enter_raid_impl(rob, 0, x, z);
	}
	
	return battle;
}

void ZhenyingBattle::DestroyPrivateBattle(uint64_t raid)
{
	PRIVATE_BATTLE_T::iterator it = s_private.find(raid);
	if (it == s_private.end())
	{
		return;
	}
	delete it->second;
	s_private.erase(it);
}

ZhenyingBattle * ZhenyingBattle::GetPrivateBattle(uint64_t raid)
{
	PRIVATE_BATTLE_T::iterator it = s_private.find(raid);
	if (it == s_private.end())
	{
		return NULL;
	}
	return it->second;
}

void ZhenyingBattle::ClearRob()
{
	for (JOIN_T::iterator it = m_allJoin.begin(); it != m_allJoin.end(); ++it)
	{
		player_struct *p = player_manager::get_ai_player_by_id(it->first);
		if (p != NULL && p->data != NULL)
		{
			player_manager::delete_player(p);
		}
	}
	m_allJoin.clear();
}
