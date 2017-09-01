#include "team.h"
#include "../proto/msgid.h"
#include "../proto/team.pb-c.h"
#include "../proto/scene_transfer.pb-c.h"
#include "chat.pb-c.h"
#include "player_manager.h"
#include "uuid.h"
#include "raid_manager.h"
#include "time_helper.h"
#include "lua_config.h"
#include "raid_manager.h"
#include "raid.h"
#include "global_param.h"
#include "pvp_match_manager.h"
#include "global_shared_data.h"

#include <algorithm>

//Team::TEAM_MAP Team::team_manager_s_teamContain;
//struct comm_pool Team::team_manager_teamDataPool;


Team::Team()
{
	m_data = (Team_data *)comm_pool_alloc(&team_manager_teamDataPool);
	memset(m_data, 0, sizeof(Team_data));
	for (size_t i = 0; i < ARRAY_SIZE(m_team_player); ++i)
		m_team_player[i] = NULL;
	m_data->m_lvMax = 1000;
	m_data->m_lvType = 2;
	m_data->m_autoAccept = true;
	//leader_last_pos_x = leader_last_pos_z = 0;
	m_sumLevel = 0;
//	m_appearBoss = false;
}

Team::~Team()
{
	raid_struct *raid = NULL;
	if (m_data->m_raid_uuid != 0)
		raid = raid_manager::get_raid_by_uuid(m_data->m_raid_uuid);
	if (raid)
	{
		raid->team_destoryed(this);
	}

	comm_pool_free(&team_manager_teamDataPool, m_data);
}

void Team::set_raid_id_wait_ready(uint32_t raid_id)
{
	for (int i = 0; i < m_data->m_memSize; ++i)
	{
		player_struct *pMem = player_manager::get_player_by_id(m_data->m_mem[i].id);
		if (!pMem)
		{
			LOG_ERR("%s: player[%lu] not online for set raid[%u]", __FUNCTION__, m_data->m_mem[i].id, raid_id);
			continue;
		}
		pMem->set_team_raid_id_wait_ready(raid_id);
	}
}
void Team::unset_raid_id_wait_ready()
{
	for (int i = 0; i < m_data->m_memSize; ++i)
	{
		player_struct *pMem = player_manager::get_player_by_id(m_data->m_mem[i].id);
		if (!pMem)
		{
			LOG_ERR("%s: player[%lu] not online for unset raid", __FUNCTION__, m_data->m_mem[i].id);
			continue;
		}
		pMem->unset_team_raid_id_wait_ready();
	}
}

void Team::BroadcastToTeam(uint16_t msg_id, void *msg_data, pack_func func, uint64_t except)
{
	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(msg_id, msg_data, func);
	PROTO_HEAD_CONN_BROADCAST *head;
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
//	head->num_player_id = m_data->m_memSize;
	for (int i = 0; i < m_data->m_memSize; ++i)
	{
		if (m_data->m_mem[i].id == except)
			continue;
		if (get_entity_type(m_data->m_mem[i].id) != ENTITY_TYPE_PLAYER)
			continue;
		if (m_data->m_mem[i].timeremove != 0)
			continue;
		ppp[(head->num_player_id)++] = m_data->m_mem[i].id;
	}

	if (head->num_player_id > 0)
	{
		head->len += sizeof(uint64_t) * head->num_player_id;
		conn_node_gamesrv::broadcast_msg_send();
	}
}

void Team::BroadcastToTeamNotinSight(player_struct &player, uint16_t msg_id, void *msg_data, pack_func func, uint64_t except)
{
	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(msg_id, msg_data, func);
	PROTO_HEAD_CONN_BROADCAST *head;
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
//	head->num_player_id = 0;//m_data->m_memSize;
	for (int i = 0; i < m_data->m_memSize; ++i)
	{
		if (m_data->m_mem[i].id != except
			&& !player.is_player_in_sight(m_data->m_mem[i].id)
			&& (get_entity_type(m_data->m_mem[i].id) == ENTITY_TYPE_PLAYER))
		{
			ppp[(head->num_player_id)++] = m_data->m_mem[i].id;
		}
	}

	if (head->num_player_id > 0)
	{
		head->len += sizeof(uint64_t) * head->num_player_id;
		conn_node_gamesrv::broadcast_msg_send();
	}
}

Team *Team::CreateTeam(player_struct **player, int size)
{
	if (size > MAX_TEAM_MEM)
		return NULL;
	
	Team *pTeam = new Team();
//	pTeam->m_data->m_id = ++s_id;
	pTeam->m_data->m_id = ++global_shared_data->g_team_id;
	
	team_manager_s_teamContain.insert(std::make_pair(pTeam->m_data->m_id, pTeam));
	pTeam->m_data->m_memSize = size;
	
	assert(size > 0);
	for (int i = 0; i < size; ++i)
	{
		player[i]->m_team = pTeam;
//		pTeam->AddMember(player[i]);

		// MEM_INFO tmp;
		// tmp.id = player[i]->get_uuid();
		// tmp.level = player[i]->get_attr(PLAYER_ATTR_LEVEL);

	// if (m_data->m_memSize > 0)
	// {
	// 	TeamMemInfo notice;
	// 	PackMemberInfo(notice, player);
	// 	BroadcastToTeam(MSG_ID_TEAM_ADD_MEMBER_NOTIFY, &notice, (pack_func)team_mem_info__pack);
	// }
	
//		pTeam->m_data->m_mem[i] = tmp;
		pTeam->m_data->m_mem[i].id = player[i]->get_uuid();
		pTeam->m_data->m_mem[i].level = player[i]->get_attr(PLAYER_ATTR_LEVEL);
		pTeam->m_team_player[i] = player[i];

		player[i]->m_team = pTeam;
		player[i]->data->teamid = pTeam->m_data->m_id;

		pTeam->m_sumLevel += player[i]->get_attr(PLAYER_ATTR_LEVEL);
	
		player[i]->refresh_player_redis_info();

		pTeam->OnTeamidChange(*player[i]);

		uint64_t target = TeamMatch::DelRole(player[i]->get_uuid());

		if (get_entity_type(player[i]->get_uuid()) == ENTITY_TYPE_PLAYER)
		{
			MatchAnser resp;
			match_anser__init(&resp);
			resp.target = target;
			resp.ret = 0;
			EXTERN_DATA ext_data;
			ext_data.player_id = player[i]->get_uuid();
			fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_TEAM_CANCEL_MATCH_ANSWER, match_anser__pack, resp);

			player[i]->add_task_progress(TCT_JOIN_TEAM, 0, size);
		}
	}

	for (int i = 0; i < size; ++i)
	{
		pTeam->SendWholeTeamInfo(*player[i]);
	}
	
	return pTeam;
}

void Team::SendXunbaoPoint(player_struct &player)
{
	int i = 0;
	for (; i < m_data->m_memSize; ++i)
	{
		if (m_data->m_mem[i].id == player.get_uuid())
		{
			continue;
		}
		player_struct *pMem = player_manager::get_player_by_id(m_data->m_mem[i].id);
		if (pMem != NULL)
		{
			if (pMem->data->xunbao.door_map != 0 && pMem->data->xunbao.cd > time_helper::get_cached_time() / 1000
				&& pMem->data->xunbao.door_map == player.data->scene_id)
			{
				SightNpcInfo send;
				sight_npc_info__init(&send);
				send.npcid = pMem->data->xunbao.door_id;
				PosData pos;
				pos_data__init(&pos);
				pos.pos_x = pMem->data->xunbao.door_x;
				pos.pos_z = pMem->data->xunbao.door_z;
				send.y = pMem->data->xunbao.door_y;
				send.data = &pos;
				send.cd = pMem->data->xunbao.cd - time_helper::get_cached_time() / 1000;
				EXTERN_DATA ext;
				ext.player_id = player.get_uuid();
				fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_ADD_NPC_NOTIFY, sight_npc_info__pack, send);
				return;
			}
		}
	}

	if (player.data->xunbao.door_map != 0 && player.data->xunbao.cd > time_helper::get_cached_time() / 1000)
	{
		SightNpcInfo send;
		sight_npc_info__init(&send);
		send.npcid = player.data->xunbao.door_id;
		PosData pos;
		pos_data__init(&pos);
		pos.pos_x = player.data->xunbao.door_x;
		pos.pos_z = player.data->xunbao.door_z;
		send.y = player.data->xunbao.door_y;
		send.data = &pos;
		send.cd = player.data->xunbao.cd - time_helper::get_cached_time() / 1000;
		i = 0;
		for (; i < m_data->m_memSize; ++i)
		{
			if (m_data->m_mem[i].id == player.get_uuid())
			{
				continue;
			}
			player_struct *pMem = player_manager::get_player_by_id(m_data->m_mem[i].id);
			if (pMem != NULL)
			{
				if (pMem->data->xunbao.door_map == player.data->scene_id)
				{
					EXTERN_DATA ext;
					ext.player_id = pMem->get_uuid();
					fast_send_msg(&conn_node_gamesrv::connecter, &ext, MSG_ID_ADD_NPC_NOTIFY, sight_npc_info__pack, send);
				}
			}
		}
	}
}

bool Team::AddMember(player_struct &player)
{
	if (!CheckLevel(player.get_attr(PLAYER_ATTR_LEVEL)))
	{
		return false;
	}
	if (m_data->m_memSize == MAX_TEAM_MEM)
	{
		return false;
	}
	raid_struct *raid = raid_manager::get_raid_by_uuid(m_data->m_raid_uuid);
	if (raid != NULL)
	{
		if (!raid->check_can_add_team_mem(&player))
		{
			return false;
		}
	}
	int pos = FindMember(player.get_uuid());
	if (pos >= 0)
	{
		return false;
	}
	MEM_INFO tmp;
	tmp.id = player.get_uuid();
	tmp.level = player.get_attr(PLAYER_ATTR_LEVEL);

	if (m_data->m_memSize > 0)
	{
		TeamMemInfo notice;
		PackMemberInfo(notice, player);
		BroadcastToTeam(MSG_ID_TEAM_ADD_MEMBER_NOTIFY, &notice, (pack_func)team_mem_info__pack);
	}
	else
	{
		TeamPlayerid send;
		team_playerid__init(&send);
		send.id = player.get_uuid();
		player.broadcast_to_sight(MSG_ID_IS_TEAM_LEAD_NOTIFY, &send, (pack_func)team_playerid__pack, false);
	}
	
	m_data->m_mem[m_data->m_memSize] = tmp;
	m_team_player[m_data->m_memSize] = &player;
	++m_data->m_memSize;

	player.m_team = this;
	player.data->teamid = m_data->m_id;

	SendWholeTeamInfo(player);
	m_sumLevel += player.get_attr(PLAYER_ATTR_LEVEL);
	
	if (raid && raid->data->state == RAID_STATE_START)
	{
		assert(raid->res_config);
		raid->player_enter_raid(&player, raid->res_config->BirthPointX, raid->res_config->BirthPointZ);
	}

	player.refresh_player_redis_info();

	OnTeamidChange(player);

	RemoveApply(player.get_uuid());

	uint64_t target = TeamMatch::DelRole(player.get_uuid());
	MatchAnser resp;
	match_anser__init(&resp);
	resp.target = target;
	resp.ret = 0;
	EXTERN_DATA ext_data;
	ext_data.player_id = player.get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_TEAM_CANCEL_MATCH_ANSWER, match_anser__pack, resp);

	pvp_match_on_team_member_changed(&player);	

	SendXunbaoPoint(player);

	for (int i = 0; i < m_data->m_memSize; ++i)
	{
		if (m_team_player[i] == NULL)
		{
			continue;
		}
		m_team_player[i]->add_task_progress(TCT_JOIN_TEAM, 0, m_data->m_memSize);
	}

	return true;
}

void Team::SendWholeTeamInfo(player_struct &player)
{
	if (get_entity_type(player.get_uuid()) != ENTITY_TYPE_PLAYER)
		return;
	
	TeamMemInfo arr[MAX_TEAM_MEM];
	TeamMemInfo *arrMemPoint[MAX_TEAM_MEM] = { arr, arr + 1,arr + 2, arr + 3 };
	int i = 0;
	TeamInfo send;
	team_info__init(&send);
	int offline = 0;
	for (int pos = 0; pos < m_data->m_memSize; ++pos)
	{
		player_struct *pMem = player_manager::get_player_by_id(m_data->m_mem[pos].id);
		if (pMem != NULL)
		{
			PackMemberInfo(arr[i], *pMem);
		}
		else
		{
			team_mem_info__init(&arr[i]);
			arr[i].playerid = m_data->m_mem[pos].id;
			arr[i].name = (char *)g_tmp_name;
			++offline;
		}

		arrMemPoint[i] = &arr[i];
		++i;
	}
	TeamLimited limit;
	PackLimit(limit);

	send.limit = &limit;
	send.n_mem = i;
	send.mem = arrMemPoint;
	send.teamid = m_data->m_id;


	EXTERN_DATA ext_data;
	ext_data.player_id = player.get_uuid();
	if (offline > 0)
	{
		conn_node_gamesrv::connecter.send_to_friend(&ext_data, MSG_ID_TEAM_INFO_NOTIFY, &send, (pack_func)team_info__pack);
	}
	else
	{
		fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_TEAM_INFO_NOTIFY, team_info__pack, send);
	}

	if (player.get_uuid() == GetLeadId())
	{
		SendApplyList(player);
	}
}

void Team::SetFollow(player_struct &player, bool follow)
{
	for (int i = 0; i < m_data->m_memSize; ++i)
	{
		if (m_data->m_mem[i].id == player.get_uuid())
		{
			m_data->m_mem[i].follow = follow;
			Follow note;
			follow__init(&note);
			note.state = follow;
			note.playerid = player.get_uuid();
			BroadcastToTeam(MSG_ID_TEAM_SET_FOLLOW_NOTIFY, &note, (pack_func)follow__pack);
			return;
		}
	}
}

bool Team::IsFollow(player_struct &player)
{
	for (int i = 0; i < m_data->m_memSize; ++i)
	{
		if (m_data->m_mem[i].id == player.get_uuid())
		{
			if (m_data->m_mem[i].follow)
			{
				return true;
			} 
			else
			{
				return false;
			}
			
		}
	}
	return false;
}

int Team::AddApply(player_struct &player)
{
	if (m_data->m_applySize == MAX_TEAM_APPLY)
	{
		return 190500024;
	}
	if (m_data->m_memSize == MAX_TEAM_MEM)
	{
		return 190500024;//190500027;
	}
	int pos = FindApply(player.get_uuid());
	if (pos >= 0)
	{
		return -1;
	}
	m_data->m_applyList[m_data->m_applySize] = player.get_uuid();
	++m_data->m_applySize;
	return 0;
}
void Team::RemoveApply(uint64_t pid)
{
	int pos = FindApply(pid);
	if (pos < 0)
	{
		return;
	}
	memmove(m_data->m_applyList + pos, m_data->m_applyList + pos + 1, sizeof(uint64_t) * (m_data->m_applySize - pos - 1));
	--m_data->m_applySize;
}
int Team::FindApply(uint64_t id)
{
	int i = 0;
	for (; i < m_data->m_applySize; ++i)
	{
		if (m_data->m_applyList[i] == id)
		{
			return i;
		}
	}
	return -1;
}

void Team::SummonMem()
{
	TeamApplyerList send;
	team_applyer_list__init(&send);
	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_TEAM_SUMMON_MEM_NOTIFY, &send, (pack_func)team_applyer_list__pack);
	for (int i = 1; i < m_data->m_memSize; ++i)
	{
		if (m_data->m_mem[i].timeremove == 0 && !m_data->m_mem[i].follow)
		{
			conn_node_gamesrv::broadcast_msg_add_players(m_data->m_mem[i].id, ppp);
			player_struct *pMember = player_manager::get_player_by_id(m_data->m_mem[i].id);
			if (pMember != NULL && pMember->data->truck.truck_id != 0)
			{
				BeLeadAnswer send;
				be_lead_answer__init(&send);
				send.ret = 190500361;
				send.name = pMember->get_name();
				EXTERN_DATA extern_data;
				extern_data.player_id = m_data->m_mem[0].id;
				fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_TEAM_HANDLE_BE_LEAD_ANSWER, be_lead_answer__pack, send);
			}
		}			
	}
	conn_node_gamesrv::broadcast_msg_send();
}

void Team::FollowLeadTrans(uint32_t scene_id, double pos_x, double pos_y, double pos_z, double direct)
{

	for (int i = 1; i < m_data->m_memSize; ++i)
	{
		if (m_data->m_mem[i].timeremove == 0 && m_data->m_mem[i].follow)
		{
			player_struct *member = player_manager::get_player_by_id(m_data->m_mem[i].id);
			if (member == NULL)
			{
				continue;
			}
			if (member->check_can_transfer() != 0)
			{
				continue;
			}
			
			EXTERN_DATA extern_data; 
			extern_data.player_id = member->get_uuid();
			member->transfer_to_new_scene(scene_id, pos_x, pos_y, pos_z, direct, &extern_data);
		}
	}

}

int Team::CkeckApplyCd(uint64_t playerid)
{
	for (int i = 0; i < m_data->m_memSize; ++i)
	{
		if (m_data->m_mem[i].id == playerid)
		{
			if (m_data->m_mem[i].appLeadCd <= (time_t)time_helper::get_cached_time()/1000)
			{
				m_data->m_mem[i].appLeadCd = time_helper::get_cached_time() / 1000 + 120;
				return 0;
			}
			else
			{
				return m_data->m_mem[i].appLeadCd - time_helper::get_cached_time() / 1000  ;
			}
		}
	}
	return 120;
}

void Team::SendApplyList(player_struct &player)
{
	if (get_entity_type(player.get_uuid()) != ENTITY_TYPE_PLAYER)
		return;
	
	TeamApplyerList send;
	TeamMemInfo arr[MAX_TEAM_APPLY];
	TeamMemInfo *arrMemPoint[MAX_TEAM_APPLY] = { arr, arr + 1,arr + 2, arr + 3 };
	int i = 0;
	team_applyer_list__init(&send);

	int offline = 0;
	for (int pos = 0; pos < m_data->m_applySize; ++pos)
	{
		player_struct *pMem = player_manager::get_player_by_id(m_data->m_applyList[i]);
		if (pMem == NULL)
		{
			team_mem_info__init(&arr[i]);
			arr[i].playerid = m_data->m_applyList[i];
			arr[i].name = (char *)g_tmp_name;
			++offline;
		}
		else
		{
			PackMemberInfo(arr[i], *pMem);
		}
		arrMemPoint[i] = &arr[i];
		++i;
	}
	send.n_apply = i;
	send.apply = arrMemPoint;

	EXTERN_DATA ext_data;
	ext_data.player_id = player.get_uuid();
	if (offline > 0)
	{
		conn_node_gamesrv::connecter.send_to_friend(&ext_data, MSG_ID_APPLYERLIST_TEAM_NOTIFY, &send, (pack_func)team_applyer_list__pack);
	}
	else
	{
		fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_APPLYERLIST_TEAM_NOTIFY, team_applyer_list__pack, send);
	}
}

void Team::PackMemberInfo(TeamMemInfo &notice, player_struct &player)
{
	team_mem_info__init(&notice);
	notice.playerid = player.get_uuid();
	notice.icon = player.get_attr(PLAYER_ATTR_HEAD);
	notice.lv = player.get_attr(PLAYER_ATTR_LEVEL);
	notice.hp = player.get_attr(PLAYER_ATTR_HP);
	notice.maxhp = player.get_attr(PLAYER_ATTR_MAXHP);
	notice.job = player.get_attr(PLAYER_ATTR_JOB);
	notice.name = player.data->name;
	notice.online = true;
	notice.clothes = player.get_attr(PLAYER_ATTR_CLOTHES);
	notice.clothes_color_up = player.get_attr(PLAYER_ATTR_CLOTHES_COLOR_UP);
	notice.clothes_color_down = player.get_attr(PLAYER_ATTR_CLOTHES_COLOR_DOWN);
	notice.hat = player.get_attr(PLAYER_ATTR_HAT);
	notice.hat_color = player.get_attr(PLAYER_ATTR_HAT_COLOR);
	notice.weapon = player.get_attr(PLAYER_ATTR_WEAPON);
	notice.fight = player.get_attr(PLAYER_ATTR_FIGHTING_CAPACITY);
	notice.pos_x = player.get_pos()->pos_x;
	notice.pos_z = player.get_pos()->pos_z;
	notice.weapon_color = player.get_attr(PLAYER_ATTR_WEAPON_COLOR);
	notice.zhenying = player.get_attr(PLAYER_ATTR_ZHENYING);
	notice.head_icon = player.get_attr(PLAYER_ATTR_HEAD);
	if (player.scene != NULL)
	{
		notice.scene_id = player.scene->m_id;
	}
}

player_struct * Team::GetLead()
{
	if (m_data->m_memSize == 0)
	{
		return NULL;
	}
	return player_manager::get_player_by_id(m_data->m_mem[0].id);
}

uint64_t Team::GetLeadId()
{
	if (m_data->m_memSize == 0)
	{
		return 0;
	}
	return m_data->m_mem[0].id;
}

void Team::MemberOffLine(player_struct &player)
{
	for (int i = 0; i < m_data->m_memSize; ++i)
	{
		if (m_data->m_mem[i].id == player.get_uuid())
		{
			m_data->m_mem[i].timeremove = time_helper::get_cached_time() / 1000 + 300;
			m_data->m_mem[i].follow = false;
			m_team_player[i] = NULL;
			break;
		}
	}

	PlayerOnlineState note;
	player_online_state__init(&note);
	note.id = player.get_uuid();
	note.state = false;
	BroadcastToTeam(MSG_ID_PLAYER_ONLINE_STATE_NOTIFY, &note, (pack_func)player_online_state__pack, player.get_uuid());

	if (player.get_uuid() == GetLeadId())
	{
		player_struct *lead = AutoChangeLeader();
		if (lead != NULL)
		{
			TeamPlayerid noticeCl;
			team_playerid__init(&noticeCl);
			noticeCl.id = lead->get_uuid();
			BroadcastToTeam(MSG_ID_TEAM_CHANGE_LEAD_NOTIFY, &noticeCl, (pack_func)team_playerid__pack);
			lead->broadcast_to_sight(MSG_ID_IS_TEAM_LEAD_NOTIFY, &noticeCl, (pack_func)team_playerid__pack, false);
			SendApplyList(*lead);
		}
		OnLeaderChange(player.get_uuid(), GetLeadId());
	}
}
/*
int Team::try_return_raid(player_struct &player)
{
	if (m_raid_uuid == 0)
		return (-1);
	raid_struct *raid = raid_manager::get_raid_by_uuid(m_raid_uuid);
	if (!raid)
	{
		m_raid_uuid = 0;
		return (-1);
	}
	return raid->player_return_raid(&player);
}
*/
bool Team::MemberOnLine(player_struct &player)
{
	TeamMatch::DelRole(player.get_uuid());
	int i = 0;
	for (; i < m_data->m_memSize; ++i)
	{
		if (m_data->m_mem[i].id == player.get_uuid())
		{
			m_data->m_mem[i].timeremove = 0;
			m_team_player[i] = &player;
			break;
		}
	}
	if (i == m_data->m_memSize)
	{
		return false;
	}
	
	PlayerOnlineState note;
	player_online_state__init(&note);
	note.id = player.get_uuid();
	note.state = true;
	BroadcastToTeam(MSG_ID_PLAYER_ONLINE_STATE_NOTIFY, &note, (pack_func)player_online_state__pack, player.get_uuid());

	OnMemberHpChange(player);

	return true;
}

bool Team::ChangeLeader(player_struct &player)
{
	//tmp.id = player.get_uuid();
	if (m_data->m_memSize < 2)
	{
		return false;
	}
	for (int i = 0; i < m_data->m_memSize; ++i)
	{
		if (m_data->m_mem[i].id == player.get_uuid())
		{
			if (i == 0)
			{
				return false;
			}
			MEM_INFO tmp;
			tmp = m_data->m_mem[i];
			m_data->m_mem[i] = m_data->m_mem[0];
			m_data->m_mem[0] = tmp;
			m_data->m_mem[0].follow = false;

			pvp_match_on_team_member_changed(&player);
			OnLeaderChange(m_data->m_mem[i].id, m_data->m_mem[0].id);
			return true;
		}
	}
	return false;
}

player_struct * Team::AutoChangeLeader()
{
	for (int i = 0; i < m_data->m_memSize; ++i)
	{
		if (m_data->m_mem[i].timeremove == 0)
		{
			player_struct *pMem = player_manager::get_player_by_id(m_data->m_mem[i].id);
			if (pMem != NULL)
			{
				if (i > 0)
				{
					MEM_INFO tmp = m_data->m_mem[i];
					m_data->m_mem[i] = m_data->m_mem[0];
					m_data->m_mem[0] = tmp;
				}
				m_data->m_mem[0].follow = false;
				return pMem;
			}
		}
	}
	return NULL;
}

void Team::OnLeaderChange(uint64_t id_old, uint64_t id_new)
{
	player_struct *old_leader = player_manager::get_player_by_id(id_old);
	player_struct *new_leader = player_manager::get_player_by_id(id_new);
	if (!old_leader)
	{
		return;
	}

	old_leader->hand_out_team_leader(new_leader);
}

void Team::RemoveMember(player_struct &player, bool kick /* = false */)
{
	player.m_team = NULL;
	player.data->teamid = 0;
	player.leave_team(GetLead());
	RemoveMember(player.get_uuid(), kick);

	if (m_data->m_raid_uuid != 0)
	{
		raid_struct *raid = raid_manager::get_raid_by_uuid(m_data->m_raid_uuid);
		if (raid)
		{
			raid->player_leave_raid(&player);
		}
	}

	OnTeamidChange(player);
}
void Team::RemoveMember(uint64_t playerid, bool kick)
{
	player_struct *leader = GetLead();
	if (leader)
		pvp_match_on_team_member_changed(leader);
	
	bool changeLead = false;
	if (GetLeadId() == playerid)
	{
		changeLead = true;
	}

	DelTeamPlayer notice;
	del_team_player__init(&notice);
	notice.playerid = playerid;
	notice.kick = kick;
	BroadcastToTeam(MSG_ID_REMOVE_TEAM_MEM_NITIFY, &notice, (pack_func)del_team_player__pack);

	int pos = FindMember(playerid);
	if (pos < 0)
	{
		return ;
	}
	m_sumLevel -= m_data->m_mem[pos].level;
	memmove(m_data->m_mem + pos, m_data->m_mem + pos + 1, sizeof(MEM_INFO) * (m_data->m_memSize - pos - 1));
	memmove(m_team_player + pos, m_team_player + pos + 1, sizeof(void *) * (m_data->m_memSize - pos - 1));
	//for (int i = pos; i < m_data->m_memSize; ++i)
	//{
	//	m_data->m_mem[i] = m_data->m_mem[i + 1];
	//}
	--m_data->m_memSize;

	if (changeLead)
	{
		player_struct *leadNew = AutoChangeLeader();
		if (leadNew != NULL)
		{
			TeamPlayerid noticeCl;
			team_playerid__init(&noticeCl);
			noticeCl.id = leadNew->get_uuid();
			BroadcastToTeam(MSG_ID_TEAM_CHANGE_LEAD_NOTIFY, &noticeCl, (pack_func)team_playerid__pack);
			leadNew->broadcast_to_sight(MSG_ID_IS_TEAM_LEAD_NOTIFY, &noticeCl, (pack_func)team_playerid__pack, false);
			SendApplyList(*leadNew);
		}
		OnLeaderChange(playerid, GetLeadId());
	}
}

int Team::CreateTeam(player_struct &player, int type, int target)
{
	if (player.data->scene_id > SCENCE_DEPART)
	{
		return 190500002;
	}
	Team *pTeam = new Team();
//	pTeam->m_data->m_id = ++s_id;
	pTeam->m_data->m_id = ++global_shared_data->g_team_id;	
	team_manager_s_teamContain.insert(std::make_pair(pTeam->m_data->m_id, pTeam));
	
	pTeam->m_data->m_lvType = type;
	pTeam->m_data->m_targrt = target;

	LOG_INFO("%s: teamid[%lu]", __FUNCTION__, pTeam->m_data->m_id);

	player.m_team = pTeam;
	player.m_team->AddMember(player);

	return 0;
}

void Team::DestroyTeam(Team *pTeam)
{
	if (pTeam != NULL)
	{
		LOG_INFO("%s: teamid[%lu]", __FUNCTION__, pTeam->m_data->m_id);
		TeamMatch::DelTeam(pTeam->m_data->m_id);
		team_manager_s_teamContain.erase(pTeam->m_data->m_id);
		delete pTeam;
	}
}

void Team::Disband()
{
	CommAnswer notice;
	comm_answer__init(&notice);
	BroadcastToTeam(MSG_ID_DISBAND_TEAM_NOTIFY, &notice, (pack_func)comm_answer__pack);

	for (int pos = 0; pos < m_data->m_memSize; ++pos)
	{
		player_struct *pMem = player_manager::get_player_by_id(m_data->m_mem[pos].id);
		if (pMem != NULL)
		{
			pMem->data->teamid = 0;
			pMem->m_team = NULL;
		}
	}
}

void Team::DestroyTeamAndNotify(Team *pTeam)
{
	if (pTeam != NULL)
	{
		pTeam->Disband();
		DestroyTeam(pTeam);
	}
}

Team * Team::GetTeam(uint64_t teamid)
{
	TEAM_MAP::iterator it = team_manager_s_teamContain.find(teamid);
	if (it == team_manager_s_teamContain.end())
	{
		return NULL;
	}
	else
	{
		return it->second;
	}
}

void Team::NotityXiayi()
{
	player_struct *lead = GetLead();
	if (lead == NULL)
	{
		return;
	}
	DungeonTable *tFb = get_config_by_id(m_data->m_targrt, &all_raid_config);
	if (tFb == NULL)
	{
		return;
	}
	uint64_t chivalry_id = 0;
	for (std::map<uint64_t, EventCalendarTable*>::iterator iter = activity_config.begin(); iter != activity_config.end(); ++iter)
	{
		if (iter->second->RelationID == tFb->ActivityControl)
		{
			chivalry_id = iter->second->ChivalrousID;
			break;
		}
	}
	if (chivalry_id == 0)
		return;
	ChivalrousTable *chivalry_config = get_config_by_id(chivalry_id, &activity_chivalry_config);
	if (chivalry_config == NULL)
		return;
	uint32_t min_level = 0;
	for (int i = 0; i < m_data->m_memSize; ++i)
	{
		uint32_t player_level = m_data->m_mem[i].level;
		if (min_level == 0)
		{
			min_level = player_level;
		}
		else
		{
			min_level = std::min(min_level, player_level);
		}
	}
	int lv = min_level + chivalry_config->Condition1;
	if (lv < m_data->m_lvMin || lv > m_data->m_lvMax)
	{
		return;
	}

	ParameterTable *tPram = get_config_by_id(190500323, &parameter_config);
	if (tPram == NULL)
	{
		return;
	}
	ParameterTable *tPram1 = get_config_by_id(161000261, &parameter_config);
	if (tPram1 == NULL)
	{
		return;
	}
	SceneResTable *tScene = get_config_by_id(m_data->m_targrt, &scene_res_config);
	if (tScene == NULL)
	{
		return;
	}

	Chat send;
	chat__init(&send);

	char str[200] = "";
	sprintf(str, tPram->parameter2, lead->get_name(), tScene->SceneName);
	send.contain = str;
	send.channel = CHANNEL__private;
	send.sendname = lead->data->name;
	send.sendplayerid = lead->get_uuid();
	send.sendplayerlv = lead->get_attr(PLAYER_ATTR_LEVEL);
	send.sendplayerjob = lead->get_attr(PLAYER_ATTR_JOB);
	send.sendplayerpicture = lead->get_attr(PLAYER_ATTR_HEAD);
	send.has_sendplayerpicture = true;

	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_CHAT_NOTIFY, &send, (pack_func)chat__pack);
	PROTO_HEAD_CONN_BROADCAST *head;
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
	
	std::map<uint64_t, player_struct *>::iterator it = player_manager_all_players_id.begin();
	for (; it != player_manager_all_players_id.end(); ++it, ++head->num_player_id)
	{
		if (head->num_player_id == tPram1->parameter1[0])
		{
			continue;
		}
		if (get_entity_type(it->second->get_uuid()) == ENTITY_TYPE_AI_PLAYER)
			continue;
		if (it->second->get_attr(PLAYER_ATTR_LEVEL) < (double)lv || it->second->get_attr(PLAYER_ATTR_LEVEL) > (double)m_data->m_lvMax)
		{
			continue;
		}
		ppp[head->num_player_id] = it->second->get_uuid();
	}
	
	if (head->num_player_id > 0)
	{
		head->len += sizeof(uint64_t) * head->num_player_id;
		conn_node_gamesrv::broadcast_msg_send();
	}
}

void Team::SetLimited(TeamLimited &limit, player_struct &player)
{
	m_data->m_lvLimit = limit.lv;
	m_data->m_targrt = limit.target;
	m_data->m_lvMax = 0;

	char str[200] = "";

	std::map<uint64_t, struct SceneResTable *>::iterator itfb = scene_res_config.find(m_data->m_targrt);
	if (m_data->m_targrt == 0)
	{
		sprintf(str, "无目标, 不限等级 {申请入队_%lu}", m_data->m_id);
		m_data->m_lvMin = 0;
		m_data->m_lvMax = 1000;
	}

	if (limit.lv_max > 0)
	{
		m_data->m_lvMin = limit.lv_min;
		m_data->m_lvMax = limit.lv_max;
	}
	else
	{
		if (m_data->m_lvLimit > 0)
		{
			std::map<uint64_t, struct ParameterTable *>::iterator it = parameter_config.find(161000015);
			if (it != parameter_config.end() && it->second->n_parameter1 >= m_data->m_lvLimit)
			{
				m_data->m_lvMin = player.get_attr(PLAYER_ATTR_LEVEL) - it->second->parameter1[m_data->m_lvLimit - 1];
				if (m_data->m_lvMin < 1)
				{
					m_data->m_lvMin = 1;
				}
				m_data->m_lvMax = player.get_attr(PLAYER_ATTR_LEVEL) + it->second->parameter1[m_data->m_lvLimit - 1];

				if (itfb != scene_res_config.end())
				{
					sprintf(str, "%s 等级 : %d~%d{ 申请入队_%lu }", itfb->second->SceneName, m_data->m_lvMin, m_data->m_lvMax, m_data->m_id);
				}
			}
		}
		else
		{

			if (itfb != scene_res_config.end())
			{
				sprintf(str, "%s 等级:%lu以上{申请入队_%lu}", itfb->second->SceneName, itfb->second->Level, m_data->m_id);
				m_data->m_lvMin = itfb->second->Level;
				m_data->m_lvMax = 1000;
			}
		}
	}
	

	if (limit.speek)
	{
		if (m_data->speekCd[CHANNEL__world] <= (time_t)time_helper::get_cached_time() / 1000)
		{
			player.send_chat(CHANNEL__world, str);

			ParameterTable *table = get_config_by_id(161000005, &parameter_config);
			if (table != NULL)
			{
				m_data->speekCd[CHANNEL__world] = time_helper::get_cached_time() / 1000 + table->parameter1[CHANNEL__world];
			}

			NotityXiayi();
		} 
		else
		{
			TeamNotifyCd note;
			team_notify_cd__init(&note);
			note.cd = m_data->speekCd[CHANNEL__world] - (time_t)time_helper::get_cached_time() / 1000;
			EXTERN_DATA extern_data;
			extern_data.player_id = player.get_uuid();
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_TEAM_SPEEK_CD_NOTIFY, team_notify_cd__pack, note);
		}
	}
	m_data->m_autoAccept = limit.auto_accept;

	if (m_data->m_autoAccept)
	{
		TeamMatch::AddTeam(m_data->m_id);
	}
	else
	{
		TeamMatch::DelTeam(m_data->m_id);
	}
}

void Team::PackLimit(_TeamLimited &limit)
{
	team_limited__init(&limit);
	limit.target = m_data->m_targrt;
	limit.lv = m_data->m_lvLimit;
	limit.lv_max = m_data->m_lvMax;
	limit.lv_min = m_data->m_lvMin;
	limit.auto_accept = m_data->m_autoAccept;
	//limit.lv_type = m_data->m_lvType;
}

int Team::PackLvJob(int *lv, int *job)
{
	int num = 0;
	//for (TEAM_MEM_CONTAIN::iterator it = m_mem.begin(); it != m_mem.end(); ++it)
	//{
	//	player_struct *pMem = player_manager::get_player_by_id(it->id);
	//	if (pMem == NULL)
	//	{
	//		continue;
	//	}
	//	*(lv + num) = pMem->get_attr(PLAYER_ATTR_LEVEL);
	//	*(job + num) = pMem->get_attr(PLAYER_ATTR_JOB);
	//	++num;
	//}
	return num;
}

int Team::PackAllMemberInfo(_TeamMemInfo *notice, _TeamMemInfo **noticeArr)
{
	int num = 0;
	int offline = 0;
	for (int i = 0; i < m_data->m_memSize; ++i)
	{
		player_struct *pMem = player_manager::get_player_by_id(m_data->m_mem[i].id);
		if (pMem == NULL)
		{
			team_mem_info__init(notice + num);
			notice[num].playerid = m_data->m_mem[i].id;
			notice[num].name = (char *)g_tmp_name;
			++offline;
		}
		else
		{
			PackMemberInfo(*(notice + num), *pMem);
		}

		noticeArr[num] = notice;
		++num;
	}
	return offline;
}

void Team::GetTargetTeamList(uint32_t target, player_struct &player)
{
	TEAM_MAP teamContain;
	for (TEAM_MAP::iterator it = team_manager_s_teamContain.begin(); it != team_manager_s_teamContain.end(); ++it)
	{
		if (it->second->GetTarget() == target && it->second->GetLead() != NULL && it->second->m_data->m_raid_uuid == 0)
		{
			teamContain.insert(std::make_pair(it->first, it->second));
		}
	}
	SendTeamList(teamContain, player, TeamMatch::GetTargetRoleNum(target));
}

bool Team::CheckLevel(int lv)
{
	if (lv < m_data->m_lvMin || lv > m_data->m_lvMax)
	{
		return false;
	}
	return true;
}

// TEAM_MEM_CONTAIN Team::GetMemberOnline()
// {
// 	TEAM_MEM_CONTAIN ret;
// 	//for (TEAM_MEM_CONTAIN::iterator it = m_mem.begin(); it != m_mem.end(); ++it)
// 	//{
// 	//	if (it->timeremove != 0)
// 	//	{
// 	//		ret.push_back(*it);
// 	//	}
// 	//}
// 	return ret;
// }

void Team::GetNearTeamList(player_struct &player)
{
	uint64_t *pp = player.get_all_sight_player();
	int *pnum = player.get_cur_sight_player();
	TEAM_MAP teamContain;
	int np = 0;
	for (int i = 0; i < *pnum; ++i)
	{
		player_struct *other = player_manager::get_player_by_id(pp[i]);
		if (other == NULL)
		{
			continue;
		}
		if (other->m_team == NULL)
		{
			if (TeamMatch::IsInMacher(pp[i]))
			{
				++np;
			}
		}
		else
		{
			teamContain.insert(std::make_pair(other->m_team->GetId(), other->m_team));
		}
		
	}

	SendTeamList(teamContain, player, np);
}

void Team::SendTeamList(TEAM_MAP &teamContain, player_struct &player, int target)
{
	TeamList send;
	team_list__init(&send);
	TeamListInfo arr[MAX_TEAM_LIST];
	TeamListInfo *arrPoint[MAX_TEAM_LIST];
	TeamMemInfo arrMem[MAX_TEAM_LIST][MAX_TEAM_MEM];
	TeamMemInfo *arrMemPoint[MAX_TEAM_LIST][MAX_TEAM_MEM];
	TeamLimited arrLimit[MAX_TEAM_LIST];

	uint32_t num = 0;
	int offline = 0;
	for (TEAM_MAP::iterator it = teamContain.begin(); it != teamContain.end(); ++it)
	{
		team_list_info__init(&arr[num]);
		arr[num].teamid = it->second->GetId();
		arr[num].apply = it->second->FindApply(player.get_uuid()) < 0 ? false : true;
		arr[num].lead = &arrMemPoint[num][0];
		arr[num].n_lead = it->second->GetMemberSize();
		offline += it->second->PackAllMemberInfo(&arrMem[num][0], &arrMemPoint[num][0]);

		arr[num].limit = &arrLimit[num];
		it->second->PackLimit(arrLimit[num]);
		arrPoint[num] = &arr[num];

		++num;
	}
	send.n_team = num;
	send.team = arrPoint;
	//if (target != 0)
	{
		send.player = target; 
	}

	EXTERN_DATA ext_data;
	ext_data.player_id = player.get_uuid();
	if (offline > 0)
	{
		conn_node_gamesrv::connecter.send_to_friend(&ext_data, MSG_ID_TEAM_LIST_ANSWER, &send, (pack_func)team_list__pack);
	}
	else
	{
		fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_TEAM_LIST_ANSWER, team_list__pack, send);
	}
}

void Team::broadcast_leader_pos(struct position *pos, uint32_t scene_id, uint64_t playerid)
{
	
	SyncPlayerPosNotify notify;
	sync_player_pos_notify__init(&notify);
	notify.player_id = playerid;
	notify.pos_x = pos->pos_x;
	notify.pos_z = pos->pos_z;
	notify.scene_id = scene_id;
	
	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SYNC_PLAYER_POS_NOTIFY, &notify, (pack_func)sync_player_pos_notify__pack);
		//不包括离线玩家
	for (int i = 0; i < m_data->m_memSize; ++i)
	{
		if (m_data->m_mem[i].id == playerid)
		{
			m_data->m_mem[i].last_pos_x = pos->pos_x;
			m_data->m_mem[i].last_pos_z = pos->pos_z;
			continue;
		}
		player_struct *pPlayer = player_manager::get_player_by_id(m_data->m_mem[i].id);
			//在同一个场景才广播
		if (pPlayer == NULL || (/*pPlayer->get_uuid() != m_data->m_mem[0].id && */pPlayer->data->scene_id != scene_id))
			continue;

			//在视野里面的不广播
		if (pPlayer->is_player_in_sight(playerid))
			continue;
		
		conn_node_gamesrv::broadcast_msg_add_players(m_data->m_mem[i].id, ppp);
	}

//	LOG_DEBUG("%s %d: [%lu] to %d player", __FUNCTION__, __LINE__, playerid, ((PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf)->num_player_id);	
	conn_node_gamesrv::broadcast_msg_send();
}

void Team::OnTimer()
{
		//同步位置
	for (int i = 0; i < m_data->m_memSize; ++i)
	{
		player_struct *leader = player_manager::get_player_by_id(m_data->m_mem[i].id);
		if (leader && leader->scene)
		{
			struct position *pos = leader->get_pos();
			if ((int)pos->pos_x != m_data->m_mem[i].last_pos_x || (int)pos->pos_z != m_data->m_mem[i].last_pos_z)
			{
				broadcast_leader_pos(pos, leader->scene->m_id, m_data->m_mem[i].id);
			}
		}
	}

		//移除离线玩家
	for (int i = 0; i < m_data->m_memSize; ++i)
	{
		if (m_data->m_mem[i].timeremove != 0 && m_data->m_mem[i].timeremove < (time_t)time_helper::get_cached_time() / 1000)
		{
			RemoveMember(m_data->m_mem[i].id);
			break;
		}
	}
}

int Team::OnMemberHpChange(player_struct &player)
{
	TeamHp note;
	team_hp__init(&note);
	note.maxhp = player.get_attr(PLAYER_ATTR_MAXHP);
	if (player.get_attr(PLAYER_ATTR_HP) < 0)
	{
		note.hp = 0;
	}
	else
	{
		note.hp = player.get_attr(PLAYER_ATTR_HP); 
	}	
	note.lv = player.get_attr(PLAYER_ATTR_LEVEL);
	note.playerid = player.get_uuid();
	BroadcastToTeam(MSG_ID_TEAM_HP_NOTIFY, &note, (pack_func)team_hp__pack);

	return 0;
}

void Team::OnTeamidChange(player_struct &player)
{
	ChangeTeamid note;
	change_teamid__init(&note);
	note.playerid = player.get_uuid();
	note.teamid = player.data->teamid;
	player.broadcast_to_sight(MSG_ID_CHANGE_TEAMID_NOTIFY, &note, (pack_func)change_teamid__pack, false);
	//好友
	//帮派 
}

uint32_t Team::GetTeamNum()
{
	return team_manager_s_teamContain.size();
}

uint32_t Team::get_team_pool_max_num()
{
	return team_manager_teamDataPool.num;
}

void Team::Timer()
{
	for (TEAM_MAP::iterator it = team_manager_s_teamContain.begin(); it != team_manager_s_teamContain.end(); ++it)
	{
		it->second->OnTimer();
		if (it->second->GetMemberSize() == 0)
		{
			DestroyTeam(it->second);
			break;
		}
	}
}


int Team::FindMember(uint64_t id)
{
	int i = 0;
	for (; i < m_data->m_memSize; ++i)
	{
		if (m_data->m_mem[i].id == id)
		{
			return i;
		}
	}
	return -1;
}


int Team::InitTeamData(int num, unsigned long key)
{
	return init_comm_pool(0, sizeof(Team_data), num, key, &team_manager_teamDataPool);
}

//////////////////////TeamMatch

void TeamMatch::AddRole(player_struct &player, int target)
{
	ALL_ROLE_CONTAIN::iterator it = team_manager_m_allRole.find(player.get_uuid());
	if (it != team_manager_m_allRole.end())
	{
		return;
	}
	team_manager_m_allRole.insert(std::make_pair(player.get_uuid(), target));

	//todo 检查target
	std::map<uint64_t, struct SceneResTable *>::iterator itconf = scene_res_config.find(target);
	if (itconf != scene_res_config.end() || target == 0 || target == 1)
	{
		TARGET_ROLE_CONTAIN::iterator itTar = team_manager_m_targetRole.find(target);
		if (itTar == team_manager_m_targetRole.end())
		{
			ROLE_IN_TARGET roleInTarget;
			roleInTarget.insert(player.get_uuid());
			team_manager_m_targetRole.insert(std::make_pair(target, roleInTarget));
		}
		else
		{
			itTar->second.insert(player.get_uuid());
		}
	}
	LOG_DEBUG("%s %d: add match team player = %lu", __FUNCTION__, __LINE__, player.get_uuid());
}

uint64_t TeamMatch::DelRole(uint64_t playerid)
{
	ALL_ROLE_CONTAIN::iterator it = team_manager_m_allRole.find(playerid);
	if (it == team_manager_m_allRole.end())
	{
		return 0;
	}
	TARGET_ROLE_CONTAIN::iterator itTar = team_manager_m_targetRole.find(it->second);
	if (itTar != team_manager_m_targetRole.end())
	{
		itTar->second.erase(playerid);
	}
	uint64_t target = it->second;
	team_manager_m_allRole.erase(it);

	LOG_DEBUG("%s %d: del match team player = %lu", __FUNCTION__, __LINE__, playerid);

	return target;
}

int TeamMatch::GetTargetRoleNum(int target)
{
	TARGET_ROLE_CONTAIN::iterator itTar = team_manager_m_targetRole.find(target);
	if (itTar != team_manager_m_targetRole.end())
	{
		return itTar->second.size();
	}
	return 0;
}

void TeamMatch::AddTeam(uint64_t id)
{
	TEAM_CONTAIN::iterator it = std::find(team_manager_m_team.begin(), team_manager_m_team.end(), id);
	if (it != team_manager_m_team.end())
	{
		return;
	}
	team_manager_m_team.push_back(id);
}

void TeamMatch::DelTeam(uint64_t id)
{
	TEAM_CONTAIN::iterator it = std::find(team_manager_m_team.begin(), team_manager_m_team.end(), id);
	if (it != team_manager_m_team.end())
	{
		team_manager_m_team.erase(it);
	}
}

Team * TeamMatch::GetMatchTeam(uint32_t target, player_struct &player)
{
	for (TEAM_CONTAIN::iterator it = team_manager_m_team.begin(); it != team_manager_m_team.end(); ++it)
	{
		Team *pt = Team::GetTeam(*it);
		if (pt == NULL)
		{
			team_manager_m_team.erase(it);
			break;
		}
		if (pt->GetTarget() == target && pt->IsAutoAccept() && pt->GetMemberSize() < MAX_TEAM_MEM
			&& CheckFbNum(player, pt->GetTarget()) && pt->GetLead() != NULL)
		{
			return pt; 
		}
	}
	return NULL;
}

Team * TeamMatch::GetNearMatchTeam(player_struct &player)
{
	uint64_t *pp = player.get_all_sight_player();
	int *pnum = player.get_cur_sight_player();
	for (int i = 0; i < *pnum; ++i)
	{
		player_struct *other = player_manager::get_player_by_id(pp[i]);
		if (other == NULL || other->m_team == NULL)
		{
			continue;
		}
		if (other->m_team->IsAutoAccept() && other->m_team->GetMemberSize() < MAX_TEAM_MEM 
			&& (CheckFbNum(player ,other->m_team->GetTarget()) || other->m_team->GetTarget() == 0)
			&& other->m_team->GetLead() != NULL)
		{
			return other->m_team;
		}
	}
	return NULL;
}

Team *TeamMatch::GetRandomMatchTeam(player_struct &player)
{
	for (TEAM_CONTAIN::iterator it = team_manager_m_team.begin(); it != team_manager_m_team.end(); ++it)
	{
		Team *pt = Team::GetTeam(*it);
		if (pt != NULL && (CheckFbNum(player, pt->GetTarget()) || pt->GetTarget() == 0)
			&& pt->GetLead() != NULL)
		{
			return pt;
		}
	}
	return NULL;
}

bool TeamMatch::CheckFbNum(player_struct &player, int target)
{
	std::map<uint64_t, struct DungeonTable*>::iterator it = all_raid_config.find(target);
	if (it == all_raid_config.end())
	{
		return true;
	}

	struct ControlTable *control_config = get_config_by_id(it->second->ActivityControl, &all_control_config);
	assert(control_config);
	
	if (player.get_raid_reward_count(target) >= control_config->RewardTime)
	{
		return false;
	}
	return true;
}

void TeamMatch::Timer()
{
	ALL_ROLE_CONTAIN::iterator it = team_manager_m_allRole.begin();
	for (; it != team_manager_m_allRole.end(); )
	{
		Team *pt = NULL;
		player_struct *player = player_manager::get_player_by_id(it->first);
		
		if (player == NULL)
		{
			uint64_t playerid = it->first;
			++it;
			DelRole(playerid);
			continue;
		}
		if (it->second == 0)
		{
			pt = GetRandomMatchTeam(*player);
		}
		else if (it->second == 1)
		{
			pt = GetNearMatchTeam(*player);
		}
		else
		{
			pt = GetMatchTeam(it->second, *player);
		}
		
		++it;
		if (pt != NULL)
		{
			pt->AddMember(*player);
		}
	}
}

bool TeamMatch::IsInMacher(uint64_t id)
{
	return team_manager_m_allRole.find(id) == team_manager_m_allRole.end() ? false : true;
}

