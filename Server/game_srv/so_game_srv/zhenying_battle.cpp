#include "zhenying_battle.h"
#include <unistd.h>
#include "comm.h"
#include "time_helper.h"
#include "player.h"
#include "msgid.h"
#include "../proto/zhenying.pb-c.h"
#include "../proto/player_redis_info.pb-c.h"
#include "game_config.h"
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
	if (itRoom == m_room.end()) //创建副本
	{
		return 5;
	}
	BattlefieldTable *table = get_config_by_id(itRoom->second.step + 360500001, &zhenying_fight_config);
	if (table == NULL)
	{
		return 6;
	}
	if (itRoom->second.uid == 0)
	{	
		raid = raid_manager::create_raid(table->Map, &player);
		if (raid == NULL)
		{
			LOG_ERR("%s: create raid failed", __FUNCTION__);
			return 3;
		}
		
		itRoom->second.uid = raid->data->uuid;
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
	raid->player_enter_raid_impl(&player, 0, x, z);
	return 0;
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
	std::vector<uint64_t>::iterator itFig = itRoom->second.fighter[zhenying].begin();
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
	send.rank = rank;
	send.point = it->second.point;
	EXTERN_DATA extern_data;
	extern_data.player_id = player.get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_FIGHT_MY_SCORE_NOTIFY, my_score__pack, send);
}

void ZhenyingBattle::Tick()
{
	uint64_t now = time_helper::get_cached_time() / 1000;
	if (now > m_nextTick)
	{
		ZhenyingExp send;
		zhenying_exp__init(&send);
		switch (m_state)
		{
		case JOIN_STATE:
			m_nextTick = now + 600;
			m_state = RUN_STATE;
			conn_node_gamesrv::send_to_all_player(MSG_ID_ZHENYING_FIGHT_START_NOTIFY, &send, (pack_func)zhenying_exp__pack);
			CreateRoom();
			break;
		case RUN_STATE:
			m_nextTick = now + 60;
			m_state = REST_STATE;
			break;
		case REST_STATE:
			m_nextTick = now + 60;
			m_state = JOIN_STATE;
			m_allJoin.clear();
			m_room.clear();
			conn_node_gamesrv::send_to_all_player(MSG_ID_JOIN_ZHENYING_FIGHT_NOTIFY, &send, (pack_func)zhenying_exp__pack);
			break;
		}
	}
	;
}
