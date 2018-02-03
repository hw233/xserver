#include "zhenying_battle.h"
#include <unistd.h>
#include <sstream>
#include <algorithm>
#include "comm.h"
#include "time_helper.h"
#include "player.h"
#include "player_manager.h"
#include "msgid.h"
#include "raid.pb-c.h" 
#include "../proto/zhenying.pb-c.h"
#include "../proto/player_redis_info.pb-c.h"
#include "../proto/tower.pb-c.h"
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
	send.level = data->zhenying.step_lv;
	send.exp = data->zhenying.exp;
	send.task = data->zhenying.task;
	send.task_type = data->zhenying.task_type;
	send.task_num = data->zhenying.task_num;
	send.step = data->zhenying.step;
	send.free_change = data->zhenying.free;
	send.daily_award = data->zhenying.daily_award;
	send.exploit = data->zhenying.exploit;
	send.history_exploit = data->zhenying.history_exploit;
	send.task_award_state = data->zhenying.task_award_state;
	send.daily_step = data->zhenying.daily_step;
	EXTERN_DATA extern_data;
	extern_data.player_id = get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_INFO_NOTIFY, zhenying_info__pack, send);
}

void player_struct::calc_zhenying_attr(double *attr)
{
	memset(attr, 0, sizeof(double)* PLAYER_ATTR_MAX);
	GradeTable *table = get_zhenying_grade_table(get_attr(PLAYER_ATTR_ZHENYING), data->zhenying.level);
	if (table == NULL)
	{
		return;
	}
	for (uint32_t i = 0; i < table->n_AttributeType; ++i)
	{
		attr[table->AttributeType[i]] += table->AttributeTypeValue[i];
	}
}

void player_struct::clear_zhenying_task()
{
	int tmp = this->get_attr(PLAYER_ATTR_ZHENYING);
	CampDefenseTable *tableDaily = get_config_by_id(360600000 + (tmp + 1) % 2, &zhenying_daily_config);
	if (tableDaily == NULL)
	{
		return;
	}
	for (uint32_t i = 0; i < tableDaily->n_TaskID; ++i)
	{
		remove_task(tableDaily->TaskID[i]);
	}
	data->zhenying.score = 0;
}

void player_struct::add_zhenying_exp(uint32_t num)
{
	if (get_attr(PLAYER_ATTR_ZHENYING) == 0)
	{
		return;
	}
	//CampTable *config = get_config_by_id(360101001, &zhenying_base_config);
	//if (config != NULL)
	//{
	//	if (data->zhenying.exp_day >= config->MaxExp)
	//	{
	//		return;
	//	}
	//}

	GradeTable *table = get_zhenying_grade_table(get_attr(PLAYER_ATTR_ZHENYING), data->zhenying.level + 1);
	if (table == NULL)
	{
		return;
	}
	data->zhenying.exp += num;
	data->zhenying.exp_day += num;
	bool up = false;
	while (table != NULL && data->zhenying.exp >= table->LevelExp)
	{
		up = true;
		data->zhenying.exp -= table->LevelExp;
		++data->zhenying.level;
		data->zhenying.step = table->Level;
		data->zhenying.step_lv = table->SmallLevel;
		if (table->n_BreachReward > 0)
		{
			std::map<uint32_t, uint32_t> item_list;
			for (uint32_t i = 0; i < table->n_BreachReward; i += 2)
			{
				item_list.insert(std::make_pair(table->BreachReward[i], table->BreachRewardNum[i]));
			}
			send_mail(270300029, NULL, NULL, NULL, NULL, &item_list, MAGIC_TYPE_ZHENYING);
			//add_item_list_as_much_as_possible(item_list, MAGIC_TYPE_ZHENYING);
		}
		table = get_zhenying_grade_table(get_attr(PLAYER_ATTR_ZHENYING), data->zhenying.level + 1);
		add_achievement_progress(ACType_ZHENYING_GRADE, data->zhenying.level, 0, 0, 1);
	}
	if (table == NULL)
	{
		table = get_zhenying_grade_table(get_attr(PLAYER_ATTR_ZHENYING), data->zhenying.level);
		data->zhenying.exp = table->LevelExp;
	}

	ZhenyingExp send;
	zhenying_exp__init(&send);
	send.exp = data->zhenying.exp;
	send.exp_day = data->zhenying.exp_day;
	send.level = data->zhenying.step_lv;
	send.step = data->zhenying.step;
	EXTERN_DATA extern_data;
	extern_data.player_id = get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_EXP_NOTIFY, zhenying_exp__pack, send);

	if (up)
	{
		calculate_attribute(true);
		refresh_player_redis_info();
	}
}

int on_login_send_tower_info(player_struct *player, EXTERN_DATA *extern_data)
{
	if (player->data->tower.top_lv == 0)
	{
		ParameterTable *table = get_config_by_id(161001004, &parameter_config);
		if (table != NULL)
		{
			player->data->tower.top_lv = 1;
			player->data->tower.cur_lv = 1;
			player->data->tower.award_lv = 1;
			player->data->tower.cur_num = table->parameter1[0];
			player->data->tower.reset_num = table->parameter1[1];
		}
	}

	TowerInfo send;
	tower_info__init(&send);
	send.cur_lv = player->data->tower.cur_lv;
	send.top_lv = player->data->tower.top_lv;
	send.cur_num = player->data->tower.cur_num;
	send.reset_num = player->data->tower.reset_num;
	fast_send_msg(&conn_node_gamesrv::connecter, extern_data, MSG_ID_TOWER_INFO_NOTIFY, tower_info__pack, send);

	return 0;
}

void player_struct::refresh_zhenying_task_oneday()
{
	clear_zhenying_task();
	ParameterTable *table = get_config_by_id(161001004, &parameter_config);
	if (table != NULL)
	{
		data->tower.cur_lv = 1;
		data->tower.award_lv = 1;
		data->tower.cur_num = table->parameter1[0];
		data->tower.reset_num = table->parameter1[1];

		EXTERN_DATA extern_data;
		extern_data.player_id = get_uuid();
		on_login_send_tower_info(this, &extern_data);
	}

	data->zhenying.exp_day = 0;
	data->zhenying.daily_award = true;
	data->zhenying.daily_step = true;
	add_zhenying_exp(0);
	ParameterTable *tablePa = get_config_by_id(161000355, &parameter_config);
	if (tablePa != NULL)
	{
		data->zhenying.award_num = tablePa->parameter1[3];
	}
	send_zhenying_info();
}












//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern ZhenyingBattle *g_battle_ins;
extern int ZhenyingBattle_battle_num;
const uint32_t ZhenyingBattle::s_border[MAX_STEP] = { 49, 69, 89, 0 };

ZhenyingBattle::ZhenyingBattle()
{
	++ZhenyingBattle_battle_num;
	memset(m_tmpRoom, 0, sizeof(m_tmpRoom));
}

ZhenyingBattle::~ZhenyingBattle()
{
	LOG_INFO("%s: %p", __FUNCTION__, this);
	--ZhenyingBattle_battle_num;
}

ZhenyingBattle *ZhenyingBattle::GetInstance()
{
	if (g_battle_ins == NULL)
	{
		g_battle_ins = new ZhenyingBattle;
	}
	return g_battle_ins;
}

bool ZhenyingBattle::CheckCreateNewRoom(player_struct &player, int s, bool isNew, uint32_t &room)
{
	bool newRoom = false;
	ROOM_T::iterator itRoom;
	int zhenying = player.get_attr(PLAYER_ATTR_ZHENYING);
	if (isNew)
	{
		newRoom = true;
	}
	else
	{
		if (m_tmpRoom[s] == 0)
		{
			newRoom = true;
		}
		else
		{
			itRoom = m_room.find(m_tmpRoom[s]);
			if (itRoom == m_room.end())
			{
				newRoom = true;
			}
			else if ((itRoom->second.m_state == JOIN_STATE && itRoom->second.m_nextTick < time_helper::get_cached_time() / 1000)
				|| itRoom->second.m_state != JOIN_STATE)
			{
				newRoom = true;
			}
			else
			{
				itRoom->second.fighter[zhenying - 1].push_back(player.get_uuid());
				room = itRoom->first;
				FbCD send;
				fb_cd__init(&send);
				send.cd = itRoom->second.m_nextTick - time_helper::get_cached_time() / 1000;
				// 检查人数已满
				if (itRoom->second.fighter[zhenying - 1].size() == MAX_ROOM_SIDE)
				{
					m_tmpRoom[s] = 0;
					send.cd = 0;
				}
				EXTERN_DATA extern_data;
				extern_data.player_id = player.get_uuid();
				fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_JOIN_ZHENYING_FIGHT_STATE_NOTIFY, fb_cd__pack, send);
			}
		}
	}
	return newRoom;
}
int ZhenyingBattle::CheckCanJoin(player_struct &player, bool isNew)
{
	uint64_t now = time_helper::get_cached_time() / 1000;
	if (player.data->zhenying.fb_cd > now)
	{
		//系统提示
		std::vector<char *> args;
		std::string sz_num;
		std::stringstream ss;
		ss << player.data->zhenying.fb_cd - now;
		ss >> sz_num;
		args.push_back(const_cast<char*>(sz_num.c_str()));
		player.send_system_notice(190500475, &args);
		return 5;
	}
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
	//if (!isNew) //todo  正常流程要检查时间
	//{
	//	uint32_t cd = 0;
	//	if (!check_active_open(330400041, cd))
	//		return 190500390;
	//}
	if (m_allJoin.find(player.get_uuid()) != m_allJoin.end())
	{
		return 4;
	}
	return 0;
}
void ZhenyingBattle::AddJoiner(player_struct &player, BATTLE_JOINER &tmp, uint32_t room)
{
	tmp.room = room;
	tmp.kill = 0;
	tmp.dead = 0;
	tmp.point = 0;
	tmp.continHelp = 0;
	tmp.continKill = 0;
	tmp.help = 0;
	tmp.lv = player.get_attr(PLAYER_ATTR_LEVEL);
	tmp.zhenying = player.get_attr(PLAYER_ATTR_ZHENYING);

	m_allJoin.insert(std::make_pair(player.get_uuid(), tmp));
}
int ZhenyingBattle::Join(player_struct &player, bool isNew)
{
	int ret = CheckCanJoin(player, isNew);
	if (ret != 0)
	{
		return ret;
	}

	BATTLE_JOINER tmp;
	
	//tmp.room = 0;
	//tmp.kill = 0;
	//tmp.dead = 0;
	//tmp.point = 0;
	//tmp.continHelp = 0;
	//tmp.continKill = 0;
	//tmp.help = 0;
	//tmp.lv = player.get_attr(PLAYER_ATTR_LEVEL);
	//tmp.zhenying = player.get_attr(PLAYER_ATTR_ZHENYING);

	int s = CalcStep(player);
	uint32_t room = 0;
	bool newRoom = CheckCreateNewRoom(player, s, isNew, room);
	if (newRoom)
	{
		room = CreateNewRoom(player, s, isNew);
	}
	AddJoiner(player, tmp, room);

	//m_allJoin.insert(std::make_pair(player.get_uuid(), tmp));

	return 0;
}
uint32_t ZhenyingBattle::CreateNewRoom(player_struct &player, int s, bool isNew)
{
	static uint32_t roomId = 1;
	ROOM_INFO info;
	info.step = s;
	info.fighter[0].clear();
	info.fighter[1].clear();
	info.uid = 0;
	info.totalPoint[0] = 0;
	info.totalPoint[1] = 0;
	info.addTick[0] = 0;
	info.addTick[1] = 0;
	BattlefieldTable *table = get_config_by_id(s + 360500001, &zhenying_fight_config);
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

	info.m_state = JOIN_STATE;
	info.readyNum[0] = 0;
	info.readyNum[1] = 0;
	info.m_nextTick = 0;
	ParameterTable *tablePa = get_config_by_id(161000355, &parameter_config);
	if (tablePa != NULL)
	{
		info.m_nextTick = time_helper::get_cached_time() / 1000 + tablePa->parameter1[2];
	}
	if (!isNew)
	{
		m_tmpRoom[s] = roomId;
	}
	else
	{
		info.m_state = AI_PLAYER_WAIT_STATE;
	}
	std::pair<ROOM_T::iterator, bool> ret = m_room.insert(std::make_pair(roomId++, info));
	int zhenying = player.get_attr(PLAYER_ATTR_ZHENYING) - 1;
	ret.first->second.fighter[zhenying].push_back(player.get_uuid());

	if (!isNew)
	{
		FbCD send;
		fb_cd__init(&send);
		send.cd = ret.first->second.m_nextTick - time_helper::get_cached_time() / 1000;
		EXTERN_DATA extern_data;
		extern_data.player_id = player.get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_JOIN_ZHENYING_FIGHT_STATE_NOTIFY, fb_cd__pack, send);
	}

	return ret.first->first;
}

int ZhenyingBattle::CancelJoin(player_struct &player)
{
	JOIN_T::iterator it = m_allJoin.find(player.get_uuid());
	if (it == m_allJoin.end())
	{
		return 1;
	}
	ROOM_T::iterator itRoom = m_room.find(it->second.room);
	if (itRoom == m_room.end()) //
	{
		LOG_ERR("%s: player[%lu] can not find room %u", __FUNCTION__, player.get_uuid(), it->second.room);
		return 2;
	}
	if (itRoom->second.uid != 0)// itRoom->second.m_state == AI_PLAYER_WAIT_STATE || itRoom->second.m_state == RUN_STATE)
	{
		return 3;
	}
	ROOM_MAN_T::iterator itRMan = std::find(itRoom->second.fighter[it->second.zhenying - 1].begin(), itRoom->second.fighter[it->second.zhenying - 1].end(), player.get_uuid());
	if (itRMan != itRoom->second.fighter[it->second.zhenying - 1].end())
	{
		itRoom->second.fighter[it->second.zhenying - 1].erase(itRMan);
	}
	if (itRoom->second.fighter[0].empty() && itRoom->second.fighter[1].empty())
	{
		LOG_INFO("%s: this = %p, delete room [%u]", __FUNCTION__, this, itRoom->first);
		m_room.erase(itRoom);
	}
	m_allJoin.erase(it);
	
	AddFbCd(player);
	return 0;
}

void ZhenyingBattle::AddFbCd(player_struct &player)
{
	player.data->zhenying.fb_cd = time_helper::get_cached_time() / 1000 + 180;
}

int ZhenyingBattle::SetReady(player_struct &player, bool ready)
{
	JOIN_T::iterator it = m_allJoin.find(player.get_uuid());
	if (it == m_allJoin.end())
	{
		return 1;
	}
	if (ready && it->second.ready == ready)
	{
		return 4;
	}
	if (it->second.room == 0)
	{
		return 2;
	}
	ROOM_T::iterator itRoom = m_room.find(it->second.room);
	if (itRoom == m_room.end()) //
	{
		return 3;
	}
	if (itRoom->second.m_state == JOIN_STATE)
	{
		if (itRoom->second.m_nextTick < time_helper::get_cached_time() / 1000 + 3
			|| (itRoom->second.fighter[0].size() == MAX_ROOM_SIDE && itRoom->second.fighter[1].size() == MAX_ROOM_SIDE))
		{
			itRoom->second.m_state = BATTLE_READY_STATE;
		}
	}
	if (itRoom->second.m_state != BATTLE_READY_STATE)
	{
		return 5;
	}
	if (ready)
	{
		itRoom->second.readyNum[it->second.zhenying - 1] += 1;
	}
	else
	{
		itRoom->second.readyNum[it->second.zhenying - 1] -= 1;
		m_allJoin.erase(player.get_uuid());
		ROOM_MAN_T::iterator itRMan = std::find(itRoom->second.fighter[it->second.zhenying - 1].begin(), itRoom->second.fighter[it->second.zhenying - 1].end(), player.get_uuid());
		if (itRMan != itRoom->second.fighter[it->second.zhenying - 1].end())
		{
			itRoom->second.fighter[it->second.zhenying - 1].erase(itRMan);
		}
		AddFbCd(player);
	}
	it->second.ready = ready;
	ZhenyingReadyState send;
	zhenying_ready_state__init(&send);
	send.fulongguo = itRoom->second.readyNum[0] + MAX_ROOM_SIDE - itRoom->second.fighter[0].size();
	send.dianfenggu = itRoom->second.readyNum[1] + MAX_ROOM_SIDE - itRoom->second.fighter[1].size();
	if (itRoom->second.readyNum[0] + itRoom->second.readyNum[1] == itRoom->second.fighter[0].size() + itRoom->second.fighter[1].size())
	{
		send.start = true;
	}
	send.ready = ready;
	send.playerid = player.get_uuid();
	BroadMessageRoom(itRoom->first, MSG_ID_ZHENYING_FIGHT_READY_STATE_NOTIFY, &send, (pack_func)zhenying_ready_state__pack);

	if (itRoom->second.fighter[0].empty() && itRoom->second.fighter[1].empty())
	{
		LOG_INFO("%s: this = %p, delete room [%u]", __FUNCTION__, this, itRoom->first);
		m_room.erase(itRoom);
	}

	return 0;
}
int ZhenyingBattle::GetReadyState(player_struct &player)
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
	ROOM_T::iterator itRoom = m_room.find(it->second.room);
	if (itRoom == m_room.end()) //
	{
		return 3;
	}
	ZhenyingReadyState send;
	zhenying_ready_state__init(&send);
	send.fulongguo = itRoom->second.readyNum[0] + MAX_ROOM_SIDE - itRoom->second.fighter[0].size();
	send.dianfenggu = itRoom->second.readyNum[1] + MAX_ROOM_SIDE - itRoom->second.fighter[1].size();
	if (itRoom->second.readyNum[0] + itRoom->second.readyNum[1] == itRoom->second.fighter[0].size() + itRoom->second.fighter[1].size())
	{
		send.start = true;
	}
	EXTERN_DATA extern_data;
	extern_data.player_id = player.get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_FIGHT_READY_STATE_NOTIFY, zhenying_ready_state__pack, send);

	if (itRoom->second.fighter[0].empty() && itRoom->second.fighter[1].empty())
	{
		LOG_INFO("%s: this = %p, delete room [%u]", __FUNCTION__, this, itRoom->first);
		m_room.erase(itRoom);
	}

	return 0;
}

uint32_t ZhenyingBattle::CalcStep(player_struct &player)
{
	int s = 0;
	for (; s < MAX_STEP - 1; ++s)
	{
		if (player.get_attr(PLAYER_ATTR_LEVEL) <= s_border[s])
		{
			break;
		}
	}
	return s;
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
	//if (m_state != AI_PLAYER_WAIT_STATE)
	//{
	//	return 8;
	//}
	//int ret = raid_manager::check_player_enter_raid(&player, table->Map);
	//if (ret != 0)
	//{
	//	return ret;
	//}
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
		//raid->data->start_time = (m_nextTick - table->ReadyTime) * 1000;
		itRoom->second.uid = raid->data->uuid;
		itRoom->second.m_state = AI_PLAYER_WAIT_STATE;
		
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
				AddJoiner(*rob, tmp, itRoom->first);
				//tmp.room = itRoom->first;
				//tmp.kill = 0;
				//tmp.dead = 0;
				//tmp.point = 0;
				//tmp.continHelp = 0;
				//tmp.continKill = 0;
				//tmp.lv = rob->get_attr(PLAYER_ATTR_LEVEL);
				//tmp.zhenying = rob->get_attr(PLAYER_ATTR_ZHENYING);
				//m_allJoin.insert(std::make_pair(rob->get_uuid(), tmp));
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
			//if (player->scene->get_scene_type() != SCENE_TYPE_RAID)
			//{
			//	continue;
			//}
			//raid_struct *raid = (raid_struct *)player->scene;
			//if (raid->m_config->DengeonRank != DUNGEON_TYPE_BATTLE && raid->m_config->DengeonRank != DUNGEON_TYPE_BATTLE_NEW)
			//{
			//	continue;
			//}
			ppp[(head->num_player_id)++] = player->get_uuid();
		}
	}
	if (head->num_player_id > 0)
	{
		head->len += sizeof(uint64_t)* head->num_player_id;
		conn_node_gamesrv::broadcast_msg_send();
	}
}

void ZhenyingBattle::OnRegionChanged(raid_struct *raid, player_struct *player, uint32_t old_region, uint32_t new_region)
{
	if (raid == NULL)
	{
		return;
	}
	IntoRegion(raid->data->ai_data.battle_data.room, player, new_region);
	LeaveRegion(raid->data->ai_data.battle_data.room, player, old_region);
}

void ZhenyingBattle::IntoRegion(uint32_t room, player_struct *player, uint32_t new_region)
{
	if (player == NULL)
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
			FlagNpc notify;
			flag_npc__init(&notify);
			notify.region = itRoom->second.flag[i].id;
			EXTERN_DATA extern_data;
			extern_data.player_id = player->get_uuid();
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_FIGHT_FLAG_NPC_NOTIFY, flag_npc__pack, notify);

			if ((itRoom->second.flag[i].own != player->get_attr(PLAYER_ATTR_ZHENYING) || itRoom->second.flag[i].state == BATTLE_FLAG_NORMOR)
				&& itRoom->second.flag[i].playerarr[zhenying].size() > itRoom->second.flag[i].playerarr[(zhenying + 1) % 2].size())
			{
				msgId = MSG_ID_ZHENYING_FIGHT_START_FLAG_NOTIFY;
				send.cd = CalcFlagTime(itRoom->second.flag[i].state, itRoom->second.flag[i].endTime);
				//itRoom->second.flag[i].endTime = time_helper::get_cached_time() / 1000 + send.cd;OT
				itRoom->second.flag[i].state = BATTLE_FLAG_GATHERING;
				itRoom->second.flag[i].own = player->get_attr(PLAYER_ATTR_ZHENYING);

				send.npc = itRoom->second.flag[i].npc;
				send.region = itRoom->second.flag[i].id;
				BroadMessageRoom(itRoom->first, msgId, &send, (pack_func)start_flag__pack);
			}
		}
	}
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
		side->help = itJ->second.help;
		side->zhenying = itJ->second.zhenying;
	}
	side->rank = rank;

	return side->online;
}

void ZhenyingBattle::Settle(uint32_t room)
{
	ROOM_T::iterator itRoom = m_room.find(room);
	if (itRoom == m_room.end())
	{
		return;
	}
	if (itRoom->second.m_state != RUN_STATE)
	{
		return;
	}
	itRoom->second.m_state = REST_STATE;
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
	if (total.fulongguo != 0 && total.dianfenggu != 0)
	{
		if (total.fulongguo > total.dianfenggu)
		{
			send.winer = ZHENYING__TYPE__FULONGGUO;
		}
		else
		{
			send.winer = ZHENYING__TYPE__WANYAOGU;
		}
	}

	//raid_struct *raid = (raid_struct *)scence;
	DoRealSettle(itRoom, table, &send, toF);

	ClearRob(room);
}
void ZhenyingBattle::DoRealSettle(ROOM_T::iterator &itRoom, BattlefieldTable *table, _ZhenYingResult *send, bool toF)
{
	static const uint32_t MAX_ZHENYING_ITEM_REWARD = 5;
	ItemData item_data1[MAX_ZHENYING_ITEM_REWARD];
	ItemData *item_data_point1[MAX_ZHENYING_ITEM_REWARD];

	std::map<uint32_t, uint32_t> item_list;
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
				JOIN_T::iterator itJ = m_allJoin.find(player->get_uuid());
				if (itJ == m_allJoin.end())
				{
					continue;
				}

				bool giveAward = false;
				if (raid->m_config->DengeonRank == DUNGEON_TYPE_BATTLE)
				{
					if (player->data->zhenying.award_num > 0)
					{
						--player->data->zhenying.award_num;
						giveAward = true;
					}
				}
				else
				{
					if (!player->data->zhenying.one_award)// && player->get_attr(PLAYER_ATTR_ZHENYING) == send->winer)
					{
						player->data->zhenying.one_award = true; //新手奖励
						giveAward = true;
					}
				}
				player->add_task_progress(TCT_FINISH_RAID, raid->data->ID, 1);
				if (giveAward)
				{
					item_list.clear();
					if (itJ->second.point < table->BottomKillMark)  //最低积分奖励
					{
						for (uint32_t i = 0; i < table->n_BottomReward && i < MAX_ZHENYING_ITEM_REWARD; ++i)
						{
							item_data_point1[i] = &(item_data1[i]);
							item_data__init(item_data_point1[i]);
							item_data1[i].id = table->BottomReward[i];
							item_data1[i].num = table->BottomRewardNum[i];
							item_list.insert(std::make_pair(item_data1[i].id, item_data1[i].num));
						}
					}
					else //排名奖励
					{
						std::map<uint32_t, std::vector<BattleFieldStepRank *> >::iterator itAward = sg_battle_award.find(itRoom->second.step);
						if (itAward != sg_battle_award.end())
						{
							for (std::vector<BattleFieldStepRank *>::iterator itV = itAward->second.begin(); itV != itAward->second.end(); ++itV)
							{
								BattleFieldStepRank *tableAward = *itV;
								if (c + 1 <= tableAward->LowerLimitRank && c + 1 >= tableAward->UpperLimitRank)
								{
									for (uint32_t i = 0; i < tableAward->n_Reward && i < MAX_ZHENYING_ITEM_REWARD; ++i)
									{
										item_data_point1[i] = &(item_data1[i]);
										item_data__init(item_data_point1[i]);
										item_data1[i].id = tableAward->Reward[i];
										item_data1[i].num = tableAward->Num[i];
										item_list.insert(std::make_pair(item_data1[i].id, item_data1[i].num));
									}
									break;
								}
							}
						}
					}

					send->exp = itJ->second.point * table->Ratio;
					player->add_zhenying_exp(send->exp);
					player->add_item_list_as_much_as_possible(item_list, 0);
				}
				
				send->person = item_data_point1;
				send->n_person = item_list.size();
				send->point = itJ->second.point;
				send->kill = itJ->second.kill;
				send->help = itJ->second.help;

				//todo
				send->n_item = item_list.size();
				send->item = item_data_point1;

				//ItemData item_data[MAX_ZHENYING_ITEM_REWARD];
				//ItemData *item_data_point[MAX_ZHENYING_ITEM_REWARD];
				//if (!player->data->zhenying.one_award && player->get_attr(PLAYER_ATTR_ZHENYING) == send->winer)
				//{
				//	item_list.clear();
				//	player->data->zhenying.one_award = true; //新手奖励
				//	for (uint32_t i = 0; i < table->n_FirstReward; ++i)
				//	{
				//		item_data_point[i] = &(item_data[i]);
				//		item_data__init(item_data_point[i]);
				//		item_data[i].id = table->FirstReward[i];
				//		item_data[i].num = table->Num[i];
				//		item_list.insert(std::make_pair(item_data[i].id, item_data[i].num));
				//	}
				//	send->n_item = table->n_FirstReward;
				//	send->item = item_data_point;
				//	player->add_item_list_as_much_as_possible(item_list, 0);
				//}
				//else
				//{
				//	send->n_item = 0;
				//	send->item = NULL;
				//}

				EXTERN_DATA extern_data;
				extern_data.player_id = player->get_uuid();
				if (toF)
				{
					conn_node_gamesrv::send_to_friend(&extern_data, MSG_ID_ZHENYING_FIGHT_SETTLE_NOTIFY, send, (pack_func)zhen_ying_result__pack);
				}
				else
				{
					fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_FIGHT_SETTLE_NOTIFY, zhen_ying_result__pack, *send);
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
	itRoom->second.m_state = REST_STATE;
}

void ZhenyingBattle::DestroyRoom(uint32_t room)
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
			m_allJoin.erase(itRoom->second.fighter[i][c]);
		}
	}
	m_room.erase(itRoom);
}

ROOM_INFO * ZhenyingBattle::GetRoom(uint32_t room)
{
	ROOM_T::iterator itRoom = m_room.find(room);
	if (itRoom == m_room.end())
	{
		LOG_ERR("%s: battle find root failed [%u]", __FUNCTION__, room);
		return NULL;
	}
	return &(itRoom->second);
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

	OneScore side[MAX_ROOM_SIDE * 2 + 1];
	OneScore *sidePoint[MAX_ROOM_SIDE * 2 + 1];
	SideScore send;
	side_score__init(&send);
	bool toF = false;
	ROOM_MAN_T::iterator itf = itRoom->second.fighter[0].begin();
	ROOM_MAN_T::iterator itd = itRoom->second.fighter[1].begin();
	size_t c = 0;
	for (; itf != itRoom->second.fighter[0].end();)
	{
		bool in = false;
		JOIN_T::iterator itJf = m_allJoin.find(*itf);
		if (itJf != m_allJoin.end())
		{
			if (itd != itRoom->second.fighter[1].end())
			{
				JOIN_T::iterator itJd = m_allJoin.find(*itd);
				if (itJd != m_allJoin.end())
				{
					if (itJf->second.point > itJd->second.point)
					{
						in = true;
					}
					else
					{
						if (!PackOneScore(side + c, c + 1, *itd))
						{
							toF = true;
						}
						sidePoint[c] = &side[c];
						++c;
						++itd;
					}
				}
				else
				{
					in = true;
				}
			}
			else
			{
				in = true;
			}
		}
		else
		{
			in = true;
		}
		if (in)
		{
			if (!PackOneScore(side + c, c + 1, *itf))
			{
				toF = true;
			}
			sidePoint[c] = &side[c];
			++c;
			++itf;
		}
	}
	for (; itd != itRoom->second.fighter[1].end(); ++itd, ++c)
	{
		if (!PackOneScore(side + c, c + 1, *itd))
		{
			toF = true;
		}
		sidePoint[c] = &side[c];
	}
	send.total = itRoom->second.totalPoint[zhenying];
	send.side = sidePoint;
	send.n_side = c;

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
	ZhenyingNum send;
	zhenying_num__init(&send);
	EXTERN_DATA extern_data;
	JOIN_T::iterator itDead = m_allJoin.find(dead.get_uuid());
	if (itDead->second.room == 0)
	{
		return;
	}
	ROOM_T::iterator itRoom = m_room.find(itDead->second.room);
	if (itRoom == m_room.end())
	{
		return;
	}
	BattlefieldTable *table = get_config_by_id(itRoom->second.step + 360500001, &zhenying_fight_config);
	if (table == NULL)
	{
		return;
	}

	if (itDead != m_allJoin.end())
	{
		++itDead->second.dead;
		itDead->second.continHelp = 0;
		itDead->second.continKill = 0;

		if (killer->get_unit_type() == UNIT_TYPE_PLAYER)
		{
			//player_struct *pKill = (player_struct *)killer;
			for (std::deque<uint64_t>::iterator it = dead.m_hitMe.begin(); it != dead.m_hitMe.end(); ++it)
			{
				if (*it == killer->get_uuid())
				{
					continue;
				}
				JOIN_T::iterator itHelp = m_allJoin.find(*it);
				if (itHelp != m_allJoin.end())
				{
					++itHelp->second.help;
					++itHelp->second.continHelp;
					itHelp->second.point += table->Assists; //todo 换成助攻积分 

					player_struct *hit = player_manager::get_player_by_id(*it);
					if (hit != NULL)
					{
						hit->m_meHit.erase(*it);
						send.num = itHelp->second.continHelp;
						extern_data.player_id = *it;
						fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_FIGHT_HELP_NOTIFY, zhenying_num__pack, send);
					}
				}
			}
			dead.m_hitMe.clear();
		}
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
	int zhenying = player.get_attr(PLAYER_ATTR_ZHENYING) - 1;
	if (zhenying >= 2 || zhenying < 0)
	{
		return;
	}

	++it->second.kill;
	++it->second.continKill;
	it->second.point += table->Kill;

	std::sort(itRoom->second.fighter[zhenying].begin(), itRoom->second.fighter[zhenying].end(), SortFighterPoint);
	SendMyScore(player);

	send.num = it->second.continKill;
	extern_data.player_id = it->first;
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_FIGHT_KILL_NOTIFY, zhenying_num__pack, send);
	
	ZhenyingKillTips notify;
	zhenying_kill_tips__init(&notify);
	char tmpName[MAX_PLAYER_NAME_LEN];
	if (send.num == 5)
	{
		notify.ret = 190500473;
		notify.bekill = dead.get_name();
	}
	else if (send.num % 10 == 0)
	{
		notify.ret = 190500474;
		sprintf(tmpName,"%d", send.num);
		notify.bekill = tmpName;
	}
	if (notify.ret != 0)
	{
		notify.kill = player.get_name();
		
		BroadMessageRoom(itDead->second.room, MSG_ID_ZHENYING_FIGHT_KILL_TIPS_NOTIFY, &notify, (pack_func)zhenying_kill_tips__pack);
	}
}

uint32_t ZhenyingBattle::CalcFlagTime(int state, uint64_t &end)
{
	uint64_t now = time_helper::get_cached_time() / 1000;
	ParameterTable *table = get_config_by_id(161000355, &parameter_config);
	if (table == NULL)
	{
		end = now + 10;
		//return 10;
	}
	if (state == BATTLE_FLAG_NORMOR)
	{
		end = now + table->parameter1[0] / 2;
		//return table->parameter1[0] / 2;
	}
	else if (state == BATTLE_FLAG_COMMPLETE)
	{
		end = now + table->parameter1[0];
		//return table->parameter1[0];
	}
	else
	{
		end = now + table->parameter1[0] - (end - now);
		//return end - now;
	}
	return end;
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
	for (uint32_t i = 0; i < MAX_ROOM_FLAG; ++i)
	{
		if (itRoom->second.flag[i].id == old_region)
		{
			itRoom->second.flag[i].playerarr[zhenying].erase(player->get_uuid());
			FlagNpc notify;
			flag_npc__init(&notify);
			EXTERN_DATA extern_data;
			extern_data.player_id = player->get_uuid();
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ZHENYING_FIGHT_FLAG_NPC_NOTIFY, flag_npc__pack, notify);

			if (itRoom->second.flag[i].own != player->get_attr(PLAYER_ATTR_ZHENYING))
			{
				break;
			}
			if (itRoom->second.flag[i].playerarr[zhenying].size() < itRoom->second.flag[i].playerarr[(zhenying + 1) % 2].size())
			{
				send.cd = CalcFlagTime(itRoom->second.flag[i].state, itRoom->second.flag[i].endTime);
				//itRoom->second.flag[i].endTime = time_helper::get_cached_time() / 1000 + send.cd;
				itRoom->second.flag[i].state = BATTLE_FLAG_GATHERING;
				itRoom->second.flag[i].own = player->get_attr(PLAYER_ATTR_ZHENYING);
				itRoom->second.flag[i].own = itRoom->second.flag[i].own % 2 + 1;

				send.zhenying = itRoom->second.flag[i].own;
				send.npc = itRoom->second.flag[i].npc;
				send.region = itRoom->second.flag[i].id;
				BroadMessageRoom(itRoom->first, MSG_ID_ZHENYING_FIGHT_START_FLAG_NOTIFY, &send, (pack_func)start_flag__pack);
			}
			else if (itRoom->second.flag[i].playerarr[zhenying].empty() && itRoom->second.flag[i].state == BATTLE_FLAG_GATHERING)
			{
				if (itRoom->second.flag[i].playerarr[zhenying].empty())
				{
					itRoom->second.flag[i].state = BATTLE_FLAG_NORMOR;
					send.npc = itRoom->second.flag[i].npc;
					send.region = itRoom->second.flag[i].id;
					BroadMessageRoom(itRoom->first, MSG_ID_ZHENYING_FIGHT_INTERUPT_FLAG_NOTIFY, &send, (pack_func)start_flag__pack);
				}
			}

			break;
		}
	}
}

int ZhenyingBattle::get_room_num()
{
	return m_room.size();
}

void ZhenyingBattle::GmStartBattle()
{
	BattlefieldTable *table = zhenying_fight_config.begin()->second;
	if (table == NULL)
	{
		return;
	}
	ZhenyingExp send;
	zhenying_exp__init(&send);
	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_JOIN_ZHENYING_FIGHT_NOTIFY, &send, (pack_func)zhenying_exp__pack);
	PROTO_HEAD_CONN_BROADCAST *head;
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
	std::map<uint64_t, player_struct *>::iterator it = player_manager_all_players_id.begin();
	for (; it != player_manager_all_players_id.end(); ++it)
	{
		if (get_entity_type(it->first) == ENTITY_TYPE_AI_PLAYER)
			continue;

		if (it->second->get_attr(PLAYER_ATTR_ZHENYING) == 0 || it->second->get_attr(PLAYER_ATTR_LEVEL) < table->LowerLimitLv)
		{
			continue;
		}

		ppp[(head->num_player_id)++] = it->second->get_uuid();
	}
	if (head->num_player_id > 0)
	{
		head->len += sizeof(uint64_t)* head->num_player_id;
		conn_node_gamesrv::broadcast_msg_send();
	}

	LOG_INFO("%s: battle[%p] clear room", __FUNCTION__, this);
}

void ZhenyingBattle::AddFlagScore(ROOM_T::iterator &itRoom, BattlefieldTable *table, uint32_t i, uint64_t now, bool isNew)
{
	if (itRoom->second.m_state != RUN_STATE)
	{
		return;
	}
	int zhenying = itRoom->second.flag[i].own - 1;

	if (itRoom->second.flag[i].playerarr[zhenying].size() == 0)
	{
		return;
	}
	if (itRoom->second.addTick[zhenying] > now)
	{
		return;
	}
	
	itRoom->second.addTick[zhenying] = now + table->ForestSet[0];
	itRoom->second.totalPoint[zhenying] += table->ForestSet[1] * itRoom->second.flag[i].playerarr[zhenying].size();

	TotalScore notify;
	total_score__init(&notify);
	notify.fulongguo = itRoom->second.totalPoint[ZHENYING__TYPE__FULONGGUO - 1];
	notify.dianfenggu = itRoom->second.totalPoint[ZHENYING__TYPE__WANYAOGU - 1];
	BroadMessageRoom(itRoom->first, MSG_ID_ZHENYING_FIGHT_SCORE_NOTIFY, &notify, (pack_func)total_score__pack);

	uint32_t VictoryIntegral = isNew ? sg_new_battle_point : table->VictoryIntegral;
	if (itRoom->second.totalPoint[zhenying] >= VictoryIntegral)
	{
		Settle(itRoom->first);
	}
}
void ZhenyingBattle::FlagOnTick(ROOM_T::iterator &itRoom, BattlefieldTable *table, uint64_t now, bool isNew)
{
	for (uint32_t i = 0; i < MAX_ROOM_FLAG; ++i)
	{
		if (itRoom->second.flag[i].state == BATTLE_FLAG_GATHERING)
		{
			if (itRoom->second.flag[i].endTime <= now)
			{
				itRoom->second.flag[i].state = BATTLE_FLAG_COMMPLETE;
				StartFlag send;
				start_flag__init(&send);
				send.zhenying = itRoom->second.flag[i].own;
				send.npc = itRoom->second.flag[i].npc;
				send.region = itRoom->second.flag[i].id;
				BroadMessageRoom(itRoom->first, MSG_ID_ZHENYING_FIGHT_FINISH_FLAG_NOTIFY, &send, (pack_func)start_flag__pack);
				itRoom->second.flag[i].endTime = now + table->ForestSet[0];

				AddFlagScore(itRoom, table, i, now, isNew);
			}
		}
		else if (itRoom->second.flag[i].state == BATTLE_FLAG_COMMPLETE)
		{
			if (itRoom->second.flag[i].endTime < now)
			{
				itRoom->second.flag[i].endTime = now + table->ForestSet[0];
				AddFlagScore(itRoom, table, i, now, isNew);
			}
		}
	}

}
void ZhenyingBattle::Tick(uint32_t room, raid_struct *raid)
{
	if (raid == NULL)
	{
		return;
	}

	uint64_t now = time_helper::get_cached_time() / 1000;

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
	if (itRoom->second.m_state == RUN_STATE)
	{
		FlagOnTick(itRoom, table, now, raid->m_config->DengeonRank == DUNGEON_TYPE_BATTLE_NEW ? true : false);
	}
	else if (itRoom->second.m_state == AI_PLAYER_WAIT_STATE)
	{
		if (now < raid->data->start_time / 1000 + table->ReadyTime)
		{
			return;
		}
		itRoom->second.m_state = RUN_STATE;
		for (int i = 0; i < 2; ++i)
		{
			for (size_t c = 0; c < itRoom->second.fighter[i].size(); ++c)
			{
				if (get_entity_type(itRoom->second.fighter[i][c]) != ENTITY_TYPE_AI_PLAYER)
				{
					continue;
				}
				player_struct *player = player_manager::get_player_by_id(itRoom->second.fighter[i][c]);
				if (player == NULL || player->ai_data == NULL)
				{
					continue;
				}

				player->ai_data->stop_ai = false;

			}
		}

	}
}

int ZhenyingBattle::CreatePrivateBattle(player_struct &player, raid_struct *raid)
{
	player.set_camp_id(player.get_attr(PLAYER_ATTR_ZHENYING));
	int ret = Join(player, true);
	if (ret != 0)
	{
		LOG_ERR("%s: player[%lu] can not find in battle ret = %d", __FUNCTION__, player.get_uuid(), ret);
	}

	LOG_INFO("%s: player[%lu] raid[%p][%lu] ", __FUNCTION__, player.get_uuid(), raid, raid->data->uuid);

	BATTLE_JOINER *join = GetJoins(player.get_uuid());
	if (join == NULL)
	{
		LOG_ERR("%s: player[%lu] can not find in battle", __FUNCTION__, player.get_uuid());
		return 1;
	}
	ROOM_T::iterator itRoom = m_room.find(join->room);
	if (itRoom == m_room.end())
	{
		LOG_ERR("%s: player[%lu] can not find room %u", __FUNCTION__, player.get_uuid(), join->room);
		return 2;
	}

	raid->data->ai_data.battle_data.room = itRoom->first;
	raid->data->ai_data.battle_data.step = itRoom->second.step;

	BattlefieldTable *table = get_config_by_id(itRoom->second.step + 360500001, &zhenying_fight_config);
	if (table != NULL)
	{
		//m_nextTick = time_helper::get_cached_time() / 1000 + table->ReadyTime;
		int x, z;
		double direct;

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
				GetRelivePos(table, rob->get_attr(PLAYER_ATTR_ZHENYING), &x, &z, &direct);
				raid->player_enter_raid_impl(rob, -1, x, z, direct);
				BATTLE_JOINER tmp;
				AddJoiner(*rob, tmp, itRoom->first);
				//tmp.room = itRoom->first;
				//tmp.kill = 0;
				//tmp.dead = 0;
				//tmp.point = 0;
				//tmp.continHelp = 0;
				//tmp.continKill = 0;
				//tmp.lv = rob->get_attr(PLAYER_ATTR_LEVEL);
				//tmp.zhenying = rob->get_attr(PLAYER_ATTR_ZHENYING);
				//m_allJoin.insert(std::make_pair(rob->get_uuid(), tmp));
				itRoom->second.fighter[iZhenying - 1].push_back(rob->get_uuid());
			}
		}
	}
	else
	{
		LOG_ERR("%s: player[%lu] get table %u", __FUNCTION__, player.get_uuid(), itRoom->second.step + 360500001);
	}

	return 0;
}
