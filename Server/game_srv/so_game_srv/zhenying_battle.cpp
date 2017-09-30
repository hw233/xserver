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

extern ZhenyingBattle *g_battle_ins;
extern PRIVATE_BATTLE_T g_battle_private;
int ZhenyingBattle::battle_num;
const uint32_t ZhenyingBattle::s_border[MAX_STEP] = { 49,69,89,0 };

ZhenyingBattle::ZhenyingBattle()
{
	m_nextTick = time_helper::get_cached_time() / 1000 + 60;
	m_state = REST_STATE;
	++battle_num;
}

ZhenyingBattle::~ZhenyingBattle()
{
	LOG_INFO("%s: %p", __FUNCTION__, this);
	--battle_num;
}

ZhenyingBattle *ZhenyingBattle::GetInstance()
{
	if (g_battle_ins == NULL)
	{
		g_battle_ins = new ZhenyingBattle;
	}
	return g_battle_ins;
}

int ZhenyingBattle::Join(player_struct &player)
{
	BattlefieldTable *table = zhenying_fight_config.begin()->second;
	if (table == NULL)
	{
		return 1;
	}
	if (table->LowerLimitLv > player.get_attr(PLAYER_ATTR_LEVEL))
	{
		return 190500404;
	}
	if (player.get_attr(PLAYER_ATTR_ZHENYING) == 0)
	{
		return 190500412;
	}
	if (m_state != JOIN_STATE)
	{
		return 190500390;
	}
	if (m_allJoin.find(player.get_uuid()) != m_allJoin.end())
	{
		return 4;
	}
	BATTLE_JOINER tmp;
	tmp.room = 0;
	tmp.kill = 0;
	tmp.dead = 0;
	tmp.point = 0;
	tmp.lv = player.get_attr(PLAYER_ATTR_LEVEL);
	tmp.zhenying = player.get_attr(PLAYER_ATTR_ZHENYING);
	m_allJoin.insert(std::make_pair(player.get_uuid(), tmp));

	return 0;
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

				LOG_INFO("%s: battle[%p] insert room [%u]", __FUNCTION__, this, roomId);	
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

void ZhenyingBattle::GetRelivePos(BattlefieldTable *table, int zhenying, int *x, int *z, double *direct)
{
	if (zhenying == ZHENYING__TYPE__FULONGGUO)
	{
		*x = table->BirthPoint1[0];
		*z = table->BirthPoint1[2];
		*direct = (int64_t)(table->BirthPoint1[3]);
	}
	else
	{
		*x = table->BirthPoint2[0];
		*z = table->BirthPoint2[2];
		*direct = (int64_t)(table->BirthPoint2[3]);
	}
	*x += 2 - random() % 5;
	*z += 2 - random() % 5;
}

int ZhenyingBattle::IntoBattle(player_struct &player)
{
	int zhenying = player.get_attr(PLAYER_ATTR_ZHENYING);
	if (zhenying == 0)
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
	if (m_state != AI_PLAYER_WAIT_STATE)
	{
		return 8;
	}
	int ret = raid_manager::check_player_enter_raid(&player, table->Map);
	if (ret != 0)
	{
		return ret;
	}
	if (player.m_team != NULL)
	{
		player.m_team->RemoveMember(player);
	}

	int x, z;
	double direct = 0;
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
		raid->data->start_time = (m_nextTick - table->ReadyTime) * 1000;
		itRoom->second.uid = raid->data->uuid;

		int name_index = random();
		for (int iZhenying = 1; iZhenying <= 2; ++iZhenying)
		{
			for (size_t i = itRoom->second.fighter[iZhenying - 1].size(); i < MAX_ROOM_SIDE; ++i)
			{
				player_struct *rob = player_manager::create_ai_player(&player, NULL, name_index++, 2);

				rob->ai_data->ai_patrol_config = robot_zhenyingzhan_config[i + (iZhenying - 1) * MAX_ROOM_SIDE];
				rob->ai_data->player_ai_index = 3;
				rob->set_camp_id(iZhenying);
				
				rob->set_attr(PLAYER_ATTR_ZHENYING, iZhenying);
				rob->set_attr(PLAYER_ATTR_PK_TYPE, 1);
				GetRelivePos(table, rob->get_attr(PLAYER_ATTR_ZHENYING), &x, &z, &direct);
				raid->player_enter_raid_impl(rob, -1, x, z, direct);
				BATTLE_JOINER tmp;
				tmp.room = itRoom->first;
				tmp.kill = 0;
				tmp.dead = 0;
				tmp.point = 0;
				tmp.lv = rob->get_attr(PLAYER_ATTR_LEVEL);
				tmp.zhenying = rob->get_attr(PLAYER_ATTR_ZHENYING);
				m_allJoin.insert(std::make_pair(rob->get_uuid(), tmp));
				itRoom->second.fighter[iZhenying - 1].push_back(rob->get_uuid());
			}
		}
	}
	else
	{
		raid = raid_manager::get_raid_by_uuid(itRoom->second.uid);
		if (raid == NULL)
		{
			return 4;
		}
	}

	it->second.in = true;
	player.set_camp_id(zhenying);	
//	if (player.get_attr(PLAYER_ATTR_PK_TYPE) != PK_TYPE_CAMP)
//	{
//		player.set_attr(PLAYER_ATTR_PK_TYPE, PK_TYPE_CAMP);
//		player.broadcast_one_attr_changed(PLAYER_ATTR_PK_TYPE, PK_TYPE_CAMP, true, true);
//	}
	GetRelivePos(table, player.get_attr(PLAYER_ATTR_ZHENYING), &x, &z, &direct);
	raid->player_enter_raid_impl(&player, -1, x, z, direct);
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
			itRoom->second.flag[i].playerarr[zhenying].insert(player->get_uuid());
			if (itRoom->second.flag[i].state == BATTLE_FLAG_NORMOR 
				|| (itRoom->second.flag[i].state == BATTLE_FLAG_COMMPLETE && itRoom->second.flag[i].own != player->get_attr(PLAYER_ATTR_ZHENYING) && itRoom->second.flag[i].playerarr[(zhenying + 1)%2].size() ==  0))
			{
				msgId = MSG_ID_ZHENYING_FIGHT_START_FLAG_NOTIFY;
				itRoom->second.flag[i].startTime = time_helper::get_cached_time() / 1000;
				itRoom->second.flag[i].state = BATTLE_FLAG_GATHERING;
				itRoom->second.flag[i].own = player->get_attr(PLAYER_ATTR_ZHENYING);
				//itRoom->second.flag[i].playerarr[zhenying].insert(player->get_uuid());
				send.cd = table->ForestSet[4];
				
				send.npc = itRoom->second.flag[i].npc;
				send.region = itRoom->second.flag[i].id;
				BroadMessageRoom(itRoom->first, msgId, &send, (pack_func)start_flag__pack);
			}
		}
	}

	LeaveRegion(itRoom->first, player, old_region);
}

bool ZhenyingBattle::PackOneScore(_OneScore *side, uint32_t rank, uint64_t playerid)
{
	one_score__init(side);
	side->playerid = playerid;
	player_struct *player = player_manager::get_player_by_id(playerid);
	if (player != NULL)
	{
		side->name = player->get_name();
		side->online = true;
	}
	else
	{
		side->name = (char *)g_tmp_name;
	}
	JOIN_T::iterator itJ = m_allJoin.find(playerid);
	if (itJ != m_allJoin.end())
	{
		side->kill = itJ->second.kill;
		side->death = itJ->second.dead;
		side->score = itJ->second.point;
	}
	side->rank = rank;

	return side->online;
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
	BattlefieldTable *table = get_config_by_id(itRoom->second.step + 360500001, &zhenying_fight_config);
	if (table == NULL)
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
	bool toF = false;
	for (int i = 0; i < 2; ++i)
	{
		for (size_t c = 0; c < itRoom->second.fighter[i].size(); ++c)
		{
			if (!PackOneScore(&side[i][c], c + 1, itRoom->second.fighter[i][c]))
			{
				toF = true;
			}
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
	//raid_struct *raid = (raid_struct *)scence;
	ItemData item_data[4];
	ItemData *item_data_point[4];

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

				player->add_task_progress(TCT_FINISH_RAID, raid->data->ID, 1);

				if (!player->data->zhenying.one_award && player->get_attr(PLAYER_ATTR_ZHENYING) == send.winer)
				{
					player->data->zhenying.one_award = true; //todo 真实奖励
					std::map<uint32_t, uint32_t> item_list;
					for (uint32_t i = 0; i < table->n_FirstReward; ++i)
					{
						item_data_point[i] = &(item_data[i]);
						item_data__init(item_data_point[i]);
						item_data[i].id = table->FirstReward[i];
						item_data[i].num = table->Num[i];
						item_list.insert(std::make_pair(item_data[i].id, item_data[i].num));
					}
					send.n_item = table->n_FirstReward;
					send.item = item_data_point;
					player->add_item_list_as_much_as_possible(item_list, 0);
				}
				else
				{
					send.n_item = 0;
					send.item = NULL;
				}

				EXTERN_DATA extern_data;
				extern_data.player_id = player->get_uuid();
				if (toF)
				{
					conn_node_gamesrv::send_to_friend(&extern_data, MSG_ID_ZHENYING_FIGHT_SETTLE_NOTIFY, &send, (pack_func)zhen_ying_result__pack);
				} 
				else
				{
					fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_FIGHT_SETTLE_NOTIFY, zhen_ying_result__pack, send);
				}
			}
		}
	}
}

void ZhenyingBattle::ClearRob(uint32_t room)
{
	LOG_INFO("%s: this = %p, room [%u]", __FUNCTION__, this, room);	
	ROOM_T::iterator itRoom = m_room.find(room);
	if (itRoom == m_room.end())
	{
		LOG_ERR("%s: battle find root failed [%u]", __FUNCTION__, room);
		return;
	}
	for (int i = 0; i < 2; ++i)
	{
		for (size_t c = 0; c < itRoom->second.fighter[i].size(); ++c)
		{
			if (get_entity_type(itRoom->second.fighter[i][c]) != ENTITY_TYPE_AI_PLAYER)
			{
				continue;
			}
			player_manager::delete_player_by_id(itRoom->second.fighter[i][c]);
		}
	}
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
	int rank = std::distance(itRoom->second.fighter[zhenying].begin(), itFig);
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
	bool toF = false;
	for (size_t c = 0; c < itRoom->second.fighter[zhenying].size(); ++c)
	{
		if (!PackOneScore(side + c, c + 1, itRoom->second.fighter[zhenying][c]))
		{
			toF = true;
		}
		sidePoint[c] = &side[c];
	}
	send.total = itRoom->second.totalPoint[zhenying];
	send.side = sidePoint;
	send.n_side = itRoom->second.fighter[zhenying].size();

	EXTERN_DATA extern_data;
	extern_data.player_id = player.get_uuid();
	if (toF)
	{
		conn_node_gamesrv::send_to_friend(&extern_data, MSG_ID_ZHENYING_FIGHT_MYSIDE_SCORE_NOTIFY, &send, (pack_func)side_score__pack);
	}
	else
	{
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_FIGHT_MYSIDE_SCORE_NOTIFY, side_score__pack, send);
	}
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
void ZhenyingBattle::KillEnemy(unit_struct *killer, player_struct &dead)
{
	JOIN_T::iterator itDead = m_allJoin.find(dead.get_uuid());
	if (itDead != m_allJoin.end())
	{
		++itDead->second.dead;
		LeaveRegion(itDead->second.room, &dead, dead.get_attr(PLAYER_ATTR_REGION_ID));
	}

	if (killer == NULL)
	{
		return;
	}
	if (killer->get_unit_type() != UNIT_TYPE_PLAYER)
	{
		return;
	}
	player_struct &player = *(player_struct *)killer;
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

}

void ZhenyingBattle::LeaveRegion(uint32_t room, player_struct *player, uint32_t old_region)
{
	if (old_region == 0)
	{
		return;
	}
	ROOM_T::iterator itRoom = m_room.find(room);
	if (itRoom == m_room.end())
	{
		return;
	}
	int zhenying = player->get_attr(PLAYER_ATTR_ZHENYING) - 1;
	if (zhenying >= 2 || zhenying < 0)
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
	send.zhenying = player->get_attr(PLAYER_ATTR_ZHENYING);
	for (uint32_t i = 0; i < MAX_ROOM_FLAG; ++i)
	{
		if (itRoom->second.flag[i].id == old_region)
		{
			itRoom->second.flag[i].playerarr[zhenying].erase(player->get_uuid());
			if (itRoom->second.flag[i].own != player->get_attr(PLAYER_ATTR_ZHENYING))
			{
				break;
			}
			if (itRoom->second.flag[i].state == BATTLE_FLAG_GATHERING)
			{
				if (itRoom->second.flag[i].playerarr[zhenying].empty())
				{
					itRoom->second.flag[i].state = BATTLE_FLAG_NORMOR;
					send.npc = itRoom->second.flag[i].npc;
					send.region = itRoom->second.flag[i].id;
					BroadMessageRoom(itRoom->first, MSG_ID_ZHENYING_FIGHT_INTERUPT_FLAG_NOTIFY, &send, (pack_func)start_flag__pack);
				}
			}

			if (!itRoom->second.flag[i].playerarr[(zhenying + 1)%2].empty() && itRoom->second.flag[i].playerarr[zhenying].empty())
			{
				itRoom->second.flag[i].own = (zhenying + 1) % 2 + 1;
				send.zhenying = itRoom->second.flag[i].own;
				itRoom->second.flag[i].startTime = time_helper::get_cached_time() / 1000;
				itRoom->second.flag[i].state = BATTLE_FLAG_GATHERING;
				send.cd = table->ForestSet[4];
				send.zhenying = itRoom->second.flag[i].own;
				send.npc = itRoom->second.flag[i].npc;
				send.region = itRoom->second.flag[i].id;
				BroadMessageRoom(itRoom->first, MSG_ID_ZHENYING_FIGHT_START_FLAG_NOTIFY, &send, (pack_func)start_flag__pack);
			}

			break;
		}
	}
}

int ZhenyingBattle::get_room_num()
{
	return m_room.size();
}

int ZhenyingBattle::get_private_battle_num()
{
	return g_battle_private.size();
}

void ZhenyingBattle::GmStartBattle()
{
	if (m_state == RUN_STATE || m_state == AI_PLAYER_WAIT_STATE)
	{
		return;
	}
	m_nextTick = 0;
	m_state = REST_STATE;
	LOG_INFO("%s: battle[%p] clear room", __FUNCTION__, this);	
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
					itRoom->second.flag[i].startTime = now + table->ForestSet[0];
					for (std::set<uint64_t>::iterator it = itRoom->second.flag[i].playerarr[zhenying].begin(); it != itRoom->second.flag[i].playerarr[zhenying].end(); ++it)
					{
						BATTLE_JOINER *join = GetJoins(*it);
						if (join == NULL)
						{
							continue;
						}
						join->point += table->ForestSet[1];
						itRoom->second.totalPoint[join->zhenying - 1] += table->ForestSet[1];
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
					TotalScore notify;
					total_score__init(&notify);
					notify.fulongguo = itRoom->second.totalPoint[ZHENYING__TYPE__FULONGGUO - 1];
					notify.dianfenggu = itRoom->second.totalPoint[ZHENYING__TYPE__WANYAOGU - 1];
					BroadMessageRoom(itRoom->first, MSG_ID_ZHENYING_FIGHT_SCORE_NOTIFY, &notify, (pack_func)total_score__pack);

					std::sort(itRoom->second.fighter[zhenying].begin(), itRoom->second.fighter[zhenying].end(), SortFighterPoint);
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

		LOG_INFO("%s: battle[%p] state[%d]", __FUNCTION__, this, m_state);
		
		switch (m_state)
		{
		case JOIN_STATE:
		{
			m_nextTick = now + 10;			

			m_state = AI_PLAYER_WAIT_STATE;
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
		case AI_PLAYER_WAIT_STATE:
		{
			DungeonTable* config = get_config_by_id(table->Map, &all_raid_config);
			if (config != NULL)
			{
				m_nextTick = now + config->FailValue[0] - table->ReadyTime;
			}
			m_state = RUN_STATE;
			StartRob();
		}
		break;
		case RUN_STATE:
			m_nextTick = now + 3600;
			m_state = REST_STATE; 
//			ClearRob();
			LOG_INFO("%s: battle[%p] clear room", __FUNCTION__, this);	
			m_room.clear();
			break;
		case REST_STATE:
		{
			m_nextTick = now + table->ReadyTime;
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
	player.set_camp_id(player.get_attr(PLAYER_ATTR_ZHENYING));
	battle->m_state = JOIN_STATE;
	int ret = battle->Join(player);
	if (ret != 0)
	{
		LOG_ERR("%s: player[%lu] can not find in battle ret = %d", __FUNCTION__, player.get_uuid(), ret);
	}
	battle->CreateRoom();
	battle->m_state = AI_PLAYER_WAIT_STATE;

	LOG_INFO("%s: player[%lu] raid[%p][%lu] battle[%p]", __FUNCTION__, player.get_uuid(), raid, raid->data->uuid, battle);
	
	BATTLE_JOINER *join = battle->GetJoins(player.get_uuid());
	if (join == NULL)
	{
		LOG_ERR("%s: player[%lu] can not find in battle", __FUNCTION__, player.get_uuid());		
		delete battle;
		return NULL;
	}
	ROOM_T::iterator itRoom = battle->m_room.find(join->room);
	if (itRoom == battle->m_room.end())
	{
		LOG_ERR("%s: player[%lu] can not find room %u", __FUNCTION__, player.get_uuid(), join->room);
		delete battle;
		return NULL;
	}

	raid->data->ai_data.battle_data.room = itRoom->first;
	raid->data->ai_data.battle_data.step = itRoom->second.step;

	BattlefieldTable *table = get_config_by_id(itRoom->second.step + 360500001, &zhenying_fight_config);
	if (table != NULL)
	{
		battle->m_nextTick = time_helper::get_cached_time() / 1000 + table->ReadyTime;
		int x, z;
		double direct;
		int zhenying;
		if (join->zhenying == ZHENYING__TYPE__FULONGGUO)
		{
			zhenying = ZHENYING__TYPE__WANYAOGU;
		}
		else
		{
			zhenying = ZHENYING__TYPE__FULONGGUO;
		}
		battle->GetRelivePos(table, zhenying, &x, &z, &direct);
/*	
		player_struct *rob = player_manager::create_ai_player(&player, NULL, 1, 2);
//		rob->set_camp_id(zhenying);		
		rob->set_attr(PLAYER_ATTR_PK_TYPE, 1);
		rob->set_attr(PLAYER_ATTR_ZHENYING, zhenying);
		rob->ai_data->ai_patrol_config = robot_zhenyingzhan_config[(zhenying - 1) * MAX_ROOM_SIDE + 6];
		rob->ai_data->player_ai_index = 3;
		rob->set_camp_id(zhenying);
		itRoom->second.fighter[zhenying - 1].push_back(rob->get_uuid());
		BATTLE_JOINER tmp;
		tmp.room = itRoom->first;
		tmp.kill = 0;
		tmp.dead = 0;
		tmp.point = 0;
		tmp.lv = rob->get_attr(PLAYER_ATTR_LEVEL);
		tmp.zhenying = rob->get_attr(PLAYER_ATTR_ZHENYING);
		battle->m_allJoin.insert(std::make_pair(rob->get_uuid(), tmp));
		raid->player_enter_raid_impl(rob, -1, x, z, direct);
*/
		int name_index = random();
		for (int iZhenying = 1; iZhenying <= 2; ++iZhenying)
		{
			for (size_t i = itRoom->second.fighter[iZhenying - 1].size(); i < 5; ++i)
			{
				player_struct *rob = player_manager::create_ai_player(&player, NULL, name_index++, 2);
				if (rob == NULL)
				{
					continue;
				}
				rob->ai_data->ai_patrol_config = robot_zhenyingzhan_config[i + 5 + (iZhenying - 1) * MAX_ROOM_SIDE];
				rob->ai_data->player_ai_index = 3;
				rob->set_camp_id(iZhenying);

				rob->set_attr(PLAYER_ATTR_ZHENYING, iZhenying);
				rob->set_attr(PLAYER_ATTR_PK_TYPE, 1);
				battle->GetRelivePos(table, rob->get_attr(PLAYER_ATTR_ZHENYING), &x, &z, &direct);
				raid->player_enter_raid_impl(rob, -1, x, z, direct);
				BATTLE_JOINER tmp;
				tmp.room = itRoom->first;
				tmp.kill = 0;
				tmp.dead = 0;
				tmp.point = 0;
				tmp.lv = rob->get_attr(PLAYER_ATTR_LEVEL);
				tmp.zhenying = rob->get_attr(PLAYER_ATTR_ZHENYING);
				battle->m_allJoin.insert(std::make_pair(rob->get_uuid(), tmp));
				itRoom->second.fighter[iZhenying - 1].push_back(rob->get_uuid());
			}
		}
	}
	else
	{
		LOG_ERR("%s: player[%lu] get table %u", __FUNCTION__, player.get_uuid(), itRoom->second.step + 360500001);
	}

	g_battle_private.insert(std::make_pair(raid->data->uuid, battle));
	
	return battle;
}

void ZhenyingBattle::DestroyPrivateBattle(uint64_t raid)
{
	LOG_INFO("%s: raid uuid = %lu", __FUNCTION__, raid);
	PRIVATE_BATTLE_T::iterator it = g_battle_private.find(raid);
	if (it == g_battle_private.end())
	{
		LOG_ERR("%s: can not find raid %lu", __FUNCTION__, raid);
		return;
	}
	delete it->second;
	g_battle_private.erase(it);
}

ZhenyingBattle * ZhenyingBattle::GetPrivateBattle(uint64_t raid)
{
	PRIVATE_BATTLE_T::iterator it = g_battle_private.find(raid);
	if (it == g_battle_private.end())
	{
		LOG_ERR("%s: can not find raid %lu", __FUNCTION__, raid);
		return NULL;
	}
	return it->second;
}

void ZhenyingBattle::StartRob()
{
	for (JOIN_T::iterator it = m_allJoin.begin(); it != m_allJoin.end(); ++it)
	{
		player_struct *p = player_manager::get_ai_player_by_id(it->first);
		if (p != NULL && p->ai_data != NULL)
		{
			p->ai_data->stop_ai = false;
		}
	}
}

void ZhenyingBattle::ClearRob()
{
	LOG_INFO("%s: this = %p", __FUNCTION__, this);
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
