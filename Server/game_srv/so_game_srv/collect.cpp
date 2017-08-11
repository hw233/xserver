#include "collect.h"

#include "player_manager.h"
#include "scene_manager.h"
#include "raid_manager.h"
#include "../proto/move.pb-c.h"
#include "../proto/collect.pb-c.h"
#include "app_data_statis.h"
#include "game_event.h"
#include "msgid.h"
#include "time_helper.h"
#include "lua_config.h"
#include "zhenying_raid_manager.h"

//uint32_t Collect::collect_manager_s_id = 1;
//Collect::COLLECT_MAP Collect::collect_manager_s_collectContain;

static int g_collect_num;

Collect::Collect()
{
	m_uuid = 0;
	m_collectId = 0;
	m_commpleteTime = 0;
	m_gatherPlayer = 0;
	area = NULL;
	scene = NULL;
	m_state = COLLECT_NORMOR;
	m_y = 34;
	m_scenceId = 0;
	m_liveTime = 0;
	m_minType = 0;
	m_ownerLv = 0;
	m_active = 0;
	m_raid_uuid = 0;
	m_dropId = 0;
	++g_collect_num;
}


Collect::~Collect()
{
	--g_collect_num;
}

void Collect::BroadcastToSight(uint16_t msg_id, void *msg_data, pack_func func)
{
	if (!area)
		return;

	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(msg_id, msg_data, func);
	PROTO_HEAD_CONN_BROADCAST *head;
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;

	AddAreaPlayerToSight(area, &head->num_player_id, ppp);
	for (int i = 0; i < MAX_NEIGHBOUR_AREA; ++i)
	{
		AddAreaPlayerToSight(area->neighbour[i], &head->num_player_id, ppp);
	}
	if (head->num_player_id > 0)
	{
		head->len += sizeof(uint64_t) * head->num_player_id;
		conn_node_gamesrv::broadcast_msg_send();
	}
}


void Collect::NotifyCollectCreate(player_struct *player)
{
	SightChangedNotify notify;
	sight_changed_notify__init(&notify);
	SightCollectInfo collect_info[1];
	SightCollectInfo *collect_info_point[1];
	collect_info_point[0] = &collect_info[0];
	notify.n_add_collect = 1;
	notify.add_collect = collect_info_point;

	sight_collect_info__init(collect_info_point[0]);
	collect_info_point[0]->uuid = m_uuid;
	collect_info_point[0]->collectid = m_collectId;
	collect_info_point[0]->y = m_y;
	collect_info_point[0]->yaw = m_yaw;
	PosData pos;
	pos_data__init(&pos);
	pos.pos_x = m_pos.pos_x;
	pos.pos_z = m_pos.pos_z;
	collect_info_point[0]->data = &pos;

	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_SIGHT_CHANGED_NOTIFY, sight_changed_notify__pack, notify);
}

void Collect::BroadcastCollectCreate()
{
	if (!area)
		return ;

	SightChangedNotify notify;
	sight_changed_notify__init(&notify);
	SightCollectInfo collect_info[1];
	SightCollectInfo *collect_info_point[1];
	collect_info_point[0] = &collect_info[0];
	notify.n_add_collect = 1;
	notify.add_collect = collect_info_point;

	sight_collect_info__init(collect_info_point[0]);
	collect_info_point[0]->uuid = m_uuid;
	collect_info_point[0]->collectid = m_collectId;
	collect_info_point[0]->y = m_y;
	collect_info_point[0]->yaw = m_yaw;	
	PosData pos;
	pos_data__init(&pos);
	pos.pos_x = m_pos.pos_x;
	pos.pos_z = m_pos.pos_z;
	collect_info_point[0]->data = &pos;

	BroadcastToSight(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
	//uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
	//PROTO_HEAD_CONN_BROADCAST *head;
	//head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;

	//add_area_player_to_sight(area, &head->num_player_id, ppp);
	//for (int i = 0; i < MAX_NEIGHBOUR_AREA; ++i)
	//{
	//	add_area_player_to_sight(area->neighbour[i], &head->num_player_id, ppp);
	//}
	//if (head->num_player_id > 0)
	//{
	//	head->len += sizeof(uint64_t) * head->num_player_id;
	//	conn_node_gamesrv::broadcast_msg_send();
	//}
}

void Collect::BroadcastCollectDelete()
{
	//SightChangedNotify notify;
	//sight_changed_notify__init(&notify);
	//notify.n_delete_collect = 1;
	//uint32_t ar[1];
	//ar[0] = m_uuid;
	//notify.delete_collect = ar;

	//BroadcastToSight(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
}

Collect * Collect::CreateCollect(scene_struct *scene, int index)
{
	Collect * pCollect = new Collect();
	pCollect->m_collectId = index;
	pCollect->m_uuid = ++collect_manager_s_id;
	pCollect->m_pos.pos_x = 202;
	pCollect->m_pos.pos_z = 46;

	scene->add_collect_to_scene(pCollect);
	//pCollect->BroadcastCollectCreate();

	collect_manager_s_collectContain.insert(std::make_pair(pCollect->m_uuid, pCollect));
	return pCollect;
}

int Collect::CreateCollectByID(scene_struct *scene, uint32_t id, uint32_t num)
{
	if (!scene->create_monster_config)
	{
		return (-1);
	}
	int len = scene->create_monster_config->size();
	for (int i = 0; i < len && num > 0; ++i)
	{
		struct SceneCreateMonsterTable *create_config = (*scene->create_monster_config)[i];
		if (!create_config)
			continue;
		if (create_config->ID != id)
			continue;
		CreateCollectByConfig(scene, i);
		--num;
	}	
	return (0);
}

Collect * Collect::CreateCollectByConfig(scene_struct *scene, int index)
{
	struct SceneCreateMonsterTable *create_config = (*scene->create_monster_config)[index];
	if (!create_config)
		return NULL;

	//std::map<uint64_t, struct CollectTable *>::iterator it = collect_config.find(create_config->ID);
	//if (it == collect_config.end())
	//{
	//	return NULL;
	//}

	return CreateCollectByPos(scene, create_config->ID, create_config->PointPosX, create_config->PointPosY, create_config->PointPosZ, create_config->Yaw);
}

Collect *Collect::CreateCollectByPos(scene_struct *scene, uint32_t id, double x, double y, double z, float yaw, player_struct *player) 
{
	if (player == NULL)
	{
		return NULL;
	}

	CollectTable *table = get_config_by_id(id, &collect_config);
	if (table == NULL)
	{
		return NULL;
	}
	//if (table->LifeTime == 0)
	//{
	//	return NULL;
	//}
	Collect * pCollect = new Collect();
	pCollect->m_collectId = id;
	pCollect->m_uuid = ++collect_manager_s_id;
	pCollect->m_pos.pos_x = x;
	pCollect->m_pos.pos_z = z;
	pCollect->m_y = y;
	pCollect->m_yaw = yaw;
	//pCollect->m_liveTime = time_helper::get_cached_time() / 1000 + table->LifeTime;

	pCollect->NotifyCollectCreate(player);

	collect_manager_s_collectContain.insert(std::make_pair(pCollect->m_uuid, pCollect));

	return pCollect;
}

Collect *Collect::CreateCollectByPos(scene_struct *scene, uint32_t id, double x, double y, double z, float yaw)
{
	CollectTable *table = get_config_by_id(id, &collect_config);
	if (table == NULL)
	{
		return NULL;
	}
	Collect * pCollect = new Collect();
	pCollect->m_collectId = id;
	pCollect->m_uuid = ++collect_manager_s_id;
	pCollect->m_pos.pos_x = x;
	pCollect->m_pos.pos_z = z;
	pCollect->m_y = y;
	pCollect->m_yaw = yaw;
	if (table->LifeTime != 0)
	{
		pCollect->m_liveTime = time_helper::get_cached_time() / 1000 + table->LifeTime;
	}

	scene->add_collect_to_scene(pCollect);
	//pCollect->BroadcastCollectCreate();

	collect_manager_s_collectContain.insert(std::make_pair(pCollect->m_uuid, pCollect));

	return pCollect;
}

void Collect::DestroyCollect(uint64_t id)
{
	COLLECT_MAP::iterator it = collect_manager_s_collectContain.find(id);
	if (it != collect_manager_s_collectContain.end())
	{
		delete it->second;
		collect_manager_s_collectContain.erase(it);
	}
}

void Collect::AddAreaPlayerToSight(area_struct *area, uint16_t *add_player_id_index, uint64_t *add_player)
{
	if (!area)
		return;
	for (int i = 0; i < area->cur_player_num; ++i)
	{
		player_struct *player = player_manager::get_player_by_id(area->m_player_ids[i]);
		if (!player)
		{
			LOG_ERR("%s %d: can not find sight player %lu area[%p]", __FUNCTION__, __LINE__, area->m_player_ids[i], area);
			continue;
		}
		
		add_player[*add_player_id_index] = player->data->player_id;
		(*add_player_id_index)++;
	}
}

uint32_t Collect::get_total_collect_num()
{
	uint32_t ret = collect_manager_s_collectContain.size();
	if ((int)ret != g_collect_num)
	{
		LOG_ERR("%s: container_size[%u], collect_num[%d], maybe bug", __FUNCTION__, ret, g_collect_num);
	}
	return ret;
}

Collect * Collect::GetById(const uint32_t id)
{
	COLLECT_MAP::iterator it = collect_manager_s_collectContain.find(id);
	if (it == collect_manager_s_collectContain.end())
	{
		return NULL;
	}
	return it->second;
}

int Collect::BegingGather(player_struct *player, uint32_t step)
{
	if (player->scene != this->scene && this->scene != NULL)
	{
		return 5;
	}
	std::map<uint64_t, struct CollectTable *>::iterator it = collect_config.find(m_collectId);
	if (it == collect_config.end())
	{
		return 3;
	}
	if (it->second->Level > player->get_attr(PLAYER_ATTR_LEVEL))
	{
		return 2;
	}
	//todo 验证距离
	float lx = player->get_pos()->pos_x - this->m_pos.pos_x;
	float lz = player->get_pos()->pos_z - this->m_pos.pos_z;
	if (lx * lx + lz * lz > 32)
	{
		return 7;
	}

	if (it->second->CollectionTeyp == 1)
	{
		m_dropId = it->second->DropID[0];
	}
	else
	{
		if (step >= it->second->n_DropID)
		{
			return 6;
		}
		m_dropId = it->second->DropID[step];
	}
	std::map<uint32_t, uint32_t> item_list;
	get_drop_item(m_dropId, item_list);
	if (!player->check_can_add_item_list(item_list))
	{
		return 190500097;
	}

		//阵营战的宝箱限制采集次数
	if (player->data->zhenying.mine < 1 && player->scene->is_in_zhenying_raid())
	{
		std::vector<struct FactionBattleTable*>::iterator it = zhenying_battle_config.begin();
		FactionBattleTable *table = *it;
		if (m_collectId == table->BoxID)
		{
			return 4;
		}
	}
	
	if (it->second->ConsumeTeyp == 2) //道具
	{
		if (player->del_item(it->second->Parameter1[step], it->second->Parameter2[step], MAGIC_TYPE_GATHER) < 0)
		{
			return 190600003;
		}
	}
	else if (it->second->ConsumeTeyp == 3) //金币
	{
		if (player->sub_coin(it->second->Parameter1[step], MAGIC_TYPE_GATHER) < 0)
		{
			return 190500063;
		}
	}
	else if (it->second->ConsumeTeyp == 4) //元宝
	{
		if (player->sub_comm_gold(it->second->Parameter1[step], MAGIC_TYPE_GATHER) < 0)
		{
			return 190400005;
		}
	}
	if (m_state == COLLECT_NORMOR || (m_state == COLLECT_GATHING && m_commpleteTime + 2 < time_helper::get_cached_time() / 1000))
	{
		m_state = COLLECT_GATHING;
		m_gatherPlayer = player->get_uuid();
		m_commpleteTime = time_helper::get_cached_time() / 1000 + it->second->Time;
	}
	player->data->m_collect_uuid = m_uuid;

	return 0;
}

int Collect::GatherComplete(player_struct *player)
{
	std::map<uint64_t, struct CollectTable *>::iterator it = collect_config.find(m_collectId);
	if (it == collect_config.end())
	{
		return 3;
	}

	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	player->data->m_collect_uuid = 0;
	CollectComplete send;
	collect_complete__init(&send);
	send.playerid = player->get_uuid();
	send.collectid = m_uuid;
	send.del = true;

	int ret = 0;
	if (m_commpleteTime <= time_helper::get_cached_time() / 1000 && m_state == COLLECT_GATHING)
	{
		m_gatherPlayer = 0; 
		SightChangedNotify notify;
		sight_changed_notify__init(&notify);
		notify.n_delete_collect = 1;
		uint32_t ar[1];
		ar[0] = m_uuid;
		notify.delete_collect = ar;
		if (it->second->Regeneration > 0)
		{
			m_state = COLLECT_RELIVE;
			m_reliveTime = time_helper::get_cached_time() / 1000 + it->second->Regeneration;
			if (scene != NULL)
			{
				scene->delete_collect_to_scene(this);
			}
			//else
			//{
			//	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_SIGHT_CHANGED_NOTIFY, sight_changed_notify__pack, notify);
			//}
		}
		else if (it->second->Regeneration == 0)
		{
			m_state = COLLECT_DESTROY;
			if (scene != NULL)
			{
				scene->delete_collect_to_scene(this);
			}
			//else
			//{
			//	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_SIGHT_CHANGED_NOTIFY, sight_changed_notify__pack, notify);
			//}
		}
		else
		{
			m_state = COLLECT_NORMOR;
			send.del = false;
		}
		
		if (area == NULL)
		{
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_COLLECT_COMMPLETE_NOTIFY, collect_complete__pack, send);
		}
		else {
			BroadcastToSight(MSG_ID_COLLECT_COMMPLETE_NOTIFY, &send, (pack_func)notify_collect__pack);
		}

		if (m_ownerLv != 0)
		{
			CashTruckDrop(*player);
		}
		else
		{
			player->give_drop_item(m_dropId, MAGIC_TYPE_GATHER, ADD_ITEM_AS_MUCH_AS_POSSIBLE);
		}
		if (m_minType == 1)
		{
			fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_XUNBAO_USE_NEXT_NOTIFY, 0, 0);
		}
	}
	else
	{
		ret = 190500093;
	}

	return ret;
}

int Collect::GatherInterupt(player_struct *player)
{
	std::map<uint64_t, struct CollectTable *>::iterator it = collect_config.find(m_collectId);
	if (it == collect_config.end())
	{
		return 3;
	}

	//if (!((m_gatherPlayer == player->get_uuid() && m_state == COLLECT_GATHING) || it->second->n_TaskId > 0))
	//{
	//	return 1;
	//}

	NotifyCollect send;
	notify_collect__init(&send);
	send.playerid = player->get_uuid();
	send.collectid = m_uuid;
	if (area == NULL)
	{
		EXTERN_DATA extern_data;
		extern_data.player_id = player->get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_COLLECT_INTERRUPT_NOTIFY, notify_collect__pack, send);
	}
	else {
		BroadcastToSight(MSG_ID_COLLECT_INTERRUPT_NOTIFY, &send, (pack_func)notify_collect__pack);
	}
	
	player->data->m_collect_uuid = 0;
	if (m_gatherPlayer == player->get_uuid())
	{
		m_state = COLLECT_NORMOR; 
		m_gatherPlayer = 0;	
	}
	
	return 0;
}

bool Collect::OnTick()
{
	if (m_state == COLLECT_DESTROY)
	{
		return false;
	}
	if (m_liveTime != 0 && m_liveTime < time_helper::get_cached_time() /1000 )
	{
		BroadcastCollectDelete();
		if (scene != NULL)
		{
			scene->delete_collect_to_scene(this);
		}
		return false;
	}
	if (m_state == COLLECT_RELIVE)
	{
		if (m_reliveTime <= time_helper::get_cached_time() / 1000)
		{
			m_state = COLLECT_NORMOR;
			
			scene_struct *pScence;

			if (m_raid_uuid)
			{
				if (m_scenceId > 30000)
				{
					pScence = zhenying_raid_manager::get_zhenying_raid_by_uuid(m_raid_uuid);
				}
				else
				{
					pScence = raid_manager::get_raid_by_uuid(m_raid_uuid);
				}
			}
			else
			{
				if (is_guild_scene_id(m_scenceId))
				{
					pScence = scene_manager::get_guild_scene(m_guild_id);
				}
				else
				{
					pScence = scene_manager::get_scene(m_scenceId);
				}
			}
			
			if (pScence != NULL)
			{
				if (pScence->add_collect_to_scene(this) != 0)
					return false;
				else
					return true;
			}
			{
				return false;
			}
		}
	}
	return true;
}


void Collect::Tick()
{
	COLLECT_MAP::iterator it = collect_manager_s_collectContain.begin();
	while (it != collect_manager_s_collectContain.end())
	{
		if (!it->second->OnTick())
			DestroyCollect((it++)->first);
		else
			++it;
	}
}

void Collect::CashTruckDrop(player_struct &player)
{
	BiaocheRewardTable *reward_config = get_config_by_id(m_active, &cash_truck_reward_config);
	if (reward_config == NULL)
	{
		return;
	}

	player.add_coin(reward_config->RewardMoney2 * m_ownerLv, MAGIC_TYPE_CASH_TRUCK);
	player.add_exp(reward_config->RewardExp1 * m_ownerLv, MAGIC_TYPE_CASH_TRUCK);
	player.add_bind_gold(reward_config->RewardLv1 * m_ownerLv, MAGIC_TYPE_CASH_TRUCK);
	for (uint32_t i = 0; i < reward_config->n_RewardItem2; ++i)
	{
		player.add_item(reward_config->RewardItem2[i], reward_config->RewardNum2[i], MAGIC_TYPE_CASH_TRUCK);
	}
}
