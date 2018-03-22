#include "collect.h"
#include "so_game_srv/sight_space.h"

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
#include "monster_manager.h"
#include "buff_manager.h"

//uint32_t Collect::collect_manager_s_id = 1;
//Collect::COLLECT_MAP Collect::collect_manager_s_collectContain;
extern int collect_g_collect_num;

Collect::Collect()
{
	m_uuid = 0;
	m_collectId = 0;
	m_commpleteTime = 0;
	m_gatherPlayer = 0;
	area = NULL;
	scene = NULL;
	sight_space = NULL;
	m_state = COLLECT_NORMOR;
	m_y = 34;
	m_scenceId = 0;
	m_liveTime = 0;
	m_minType = 0;
	m_ownerLv = 0;
	m_active = 0;
	m_raid_uuid = 0;
	m_dropId = 0;
	m_rand_id = 0;
	++collect_g_collect_num;
}


Collect::~Collect()
{
	if (sight_space)
	{
		for (int i = 0; i < MAX_COLLECT_IN_SIGHT_SPACE; ++i)
		{
			if (sight_space->collects[i] == this)
			{
				sight_space->collects[i] = NULL;
				break;
			}
		}
	}
	
	--collect_g_collect_num;
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
	PosData pos;
	pack_sight_collect_info(collect_info_point[0], &pos);

	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_SIGHT_CHANGED_NOTIFY, sight_changed_notify__pack, notify);
}

void Collect::pack_sight_collect_info(SightCollectInfo *point, PosData *pos)
{
	sight_collect_info__init(point);
	point->uuid = m_uuid;
	point->collectid = m_collectId;
	point->y = m_y;
	point->yaw = m_yaw;	
	pos_data__init(pos);
	pos->pos_x = m_pos.pos_x;
	pos->pos_z = m_pos.pos_z;
	point->data = pos;
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
	PosData pos;
	pack_sight_collect_info(collect_info_point[0], &pos);

	BroadcastToSight(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
}

void Collect::BroadcastCollectDelete()
{
	SightChangedNotify notify;
	sight_changed_notify__init(&notify);
	notify.n_delete_collect = 1;
	uint32_t ar[1];
	ar[0] = m_uuid;
	notify.delete_collect = ar;

	BroadcastToSight(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
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
	return CreateCollectByPos(scene, create_config->ID, create_config->PointPosX, create_config->PointPosY, create_config->PointPosZ, create_config->Yaw);
}

Collect *Collect::CreateCollectByPos(scene_struct *scene, uint32_t id, double x, double y, double z, float yaw, player_struct *player) 
{
	if (player == NULL)
	{
		return NULL;
	}

	Collect * pCollect = CreateCollectImp(id, x, y, z, yaw);
	if (pCollect == NULL)
	{
		return NULL;
	}
	pCollect->NotifyCollectCreate(player);

	return pCollect;
}

Collect *Collect::create_sight_space_collect(sight_space_struct *sight_space, uint32_t id, double x, double y, double z, float yaw)
{
	for (int i = 0; i < MAX_COLLECT_IN_SIGHT_SPACE; ++i)
	{
		if (sight_space->collects[i] != NULL)
			continue;
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
		pCollect->sight_space = sight_space;

		sight_space->collects[i] = pCollect;
		sight_space->broadcast_collect_create(pCollect);

		collect_manager_s_collectContain.insert(std::make_pair(pCollect->m_uuid, pCollect));

		return pCollect;
	}
	return NULL;
}

Collect *Collect::CreateCollectByPos(scene_struct *scene, uint32_t id, double x, double y, double z, float yaw)
{
	Collect * pCollect = CreateCollectImp(id, x, y, z, yaw);
	if (pCollect == NULL)
	{
		return NULL;
	}
	scene->add_collect_to_scene(pCollect);

	return pCollect;
}

Collect *Collect::CreateCollectImp(uint32_t id, double x, double y, double z, float yaw)
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

void Collect::RemoveFromSceneAndDestroyCollect(Collect *pCollect, bool send_msg)
{
	if (pCollect == NULL)
	{
		return;
	}
	if (pCollect->scene != NULL)
	{
		pCollect->scene->delete_collect_from_scene(pCollect, send_msg);
	}
	else
	{
		LOG_ERR("[%s : %d]: collect = %u no scene", __FUNCTION__, __LINE__, pCollect->m_uuid);
	}
	Collect::DestroyCollect(pCollect->m_uuid);
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
	if ((int)ret != collect_g_collect_num)
	{
		LOG_ERR("%s: container_size[%u], collect_num[%d], maybe bug", __FUNCTION__, ret, collect_g_collect_num);
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

	float lx = player->get_pos()->pos_x - this->m_pos.pos_x;
	float lz = player->get_pos()->pos_z - this->m_pos.pos_z;
	if (lx * lx + lz * lz > 2.0 * (3.0 + it->second->CollectionSize)*(3.0 + it->second->CollectionSize))
	{
		LOG_ERR("%s: %lu x=%f,z=%f,ox=%f,oz=%f", __FUNCTION__, player->get_uuid(), player->get_pos()->pos_x, player->get_pos()->pos_z, this->m_pos.pos_x, this->m_pos.pos_z);
		return 7;
	}

	if (it->second->DropType == 1)
	{
		if (step >= it->second->n_Drop1)
		{
			return 6;
		}
		m_dropId = it->second->Drop1[step];
	}
	std::map<uint32_t, uint32_t> item_list;
	get_drop_item(m_dropId, item_list);
	if (!player->check_can_add_item_list(item_list))
	{
		return 190500097;
	}

		//阵营战的宝箱限制采集次数
	if (player->scene->is_in_zhenying_raid())
	{
		if (player->data->zhenying.gather > 0)
		{
			return 190500564;
		}
		raid_struct *raid = (raid_struct *)player->scene;
		if (m_minType == 3)
		{
			if (player->get_attr(PLAYER_ATTR_ZHENYING) != raid->data->ai_data.zhenying_data.camp % 10)
			{
				return 8;
			}
		} 
		else
		{
			if (player->get_attr(PLAYER_ATTR_ZHENYING) == raid->data->ai_data.zhenying_data.camp % 10)
			{
				return 4;
			}
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


	if (m_commpleteTime > time_helper::get_cached_time() / 1000 && m_state == COLLECT_GATHING)
	{
		return 190500093;
	}
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
			scene->delete_collect_from_scene(this);
		}
	}
	else if (it->second->Regeneration == 0)
	{
		m_state = COLLECT_DESTROY;
		if (scene != NULL)
		{
			scene->delete_collect_from_scene(this);
		}
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

	return DoGatherDrop(player);
}
int Collect::DoGatherDrop(player_struct *player)
{
	if (player == NULL)
	{
		return 0;
	}
	std::map<uint64_t, struct CollectTable *>::iterator it = collect_config.find(m_collectId);
	if (it == collect_config.end())
	{
		return 3;
	}
	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	if (m_minType != 2 && m_minType != 3) // 2 3是日常阵营镖车无掉落
	{
		if (m_ownerLv != 0)
		{
			CashTruckDrop(*player);
		}
		else
		{
			if (it->second->DropType == 1)
			{
				std::map<uint32_t, uint32_t> item_list;
				get_drop_item(m_dropId, item_list);
				if (item_list.empty())
				{
					CommAnswer resp;
					comm_answer__init(&resp);
					if (it->second->ConsumeTeyp != 1)
					{
						resp.result = 190500406;
					}
					else
					{
						resp.result = 190500442;
					}
					fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_COLLECT_BEGIN_ANSWER, comm_answer__pack, resp);
				}
				player->give_drop_item(m_dropId, MAGIC_TYPE_GATHER, ADD_ITEM_AS_MUCH_AS_POSSIBLE);
			}
			else if (it->second->DropType == 2)
			{
				for (uint32_t i = 0; i < it->second->n_Drop1; ++i)
				{
					for (uint32_t n = 0; n < it->second->Drop2[i]; ++n)
					{
						monster_manager::create_monster_at_pos(this->scene, it->second->Drop1[i], player->get_attr(PLAYER_ATTR_LEVEL), m_pos.pos_x + 3 - rand() % 7, m_pos.pos_z + 3 - rand() % 7, 0, NULL, 0);
					}
				}
			}
			else if (it->second->DropType == 3)
			{
				buff_manager::create_default_buff(it->second->Drop1[0], player, player, true);
			}
		}
	}

	if (m_minType == 1)
	{
		fast_send_msg_base(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_XUNBAO_USE_NEXT_NOTIFY, 0, 0);
	}
	return 0;
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
	if (m_liveTime != 0 && m_liveTime < time_helper::get_cached_time() / 1000)
	{
		m_liveTime = 0;
		if (scene != NULL)
		{
			scene->delete_collect_from_scene(this, true);
		}
		std::map<uint64_t, struct CollectTable *>::iterator it = collect_config.find(m_collectId);
		if (it == collect_config.end())
		{
			return false;
		}
		if (it->second->Regeneration > 0)
		{
			m_state = COLLECT_RELIVE;
			m_reliveTime = time_helper::get_cached_time() / 1000 + it->second->Regeneration;
			return true;
		}
		return false;
	}
	if (m_state == COLLECT_RELIVE && m_reliveTime <= time_helper::get_cached_time() / 1000)
	{
		return Relive();
	}
	return true;
}

bool Collect::Relive()
{
	std::map<uint64_t, struct CollectTable *>::iterator it = collect_config.find(m_collectId);
	if (it == collect_config.end())
	{
		return false;
	}
	if (it->second->LifeTime != 0)
	{
		m_liveTime = time_helper::get_cached_time() / 1000 + it->second->LifeTime;
	}

	m_state = COLLECT_NORMOR;

	scene_struct *pScence;
	if (m_raid_uuid)
	{
		DungeonTable* config = get_config_by_id(m_scenceId, &all_raid_config);
		if (config != NULL && config->DengeonRank == DUNGEON_TYPE_ZHENYING)
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
		pScence = scene_manager::get_scene(m_scenceId);
	}
	if (pScence == NULL)
	{
		return false;
	}

	if (m_rand_id != 0)
	{
		RandomCollectionTable *table = get_config_by_id(m_rand_id, &random_collect_config);
		if (table == NULL)
		{
			return false;
		}
		uint32_t pos = rand() % table->n_PointX;
		m_pos.pos_x = table->PointX[pos];
		m_pos.pos_z = table->PointZ[pos];
	}

	if (pScence->add_collect_to_scene(this) != 0)
		return false;
	else
		return true;
}

void Collect::Tick()
{
	COLLECT_MAP::iterator it = collect_manager_s_collectContain.begin();
	while (it != collect_manager_s_collectContain.end())
	{
		if (!it->second->OnTick())
		{
			COLLECT_MAP::iterator t = it;
			it++;
			DestroyCollect((t)->first);
		}
		else
		{
			++it;
		}
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

int Collect::CreateRandCollect(scene_struct *scene)
{
	std::map<uint64_t, std::vector<uint64_t> >::iterator itRand = sg_rand_collect.find(scene->m_id);
	if (itRand == sg_rand_collect.end())
	{
		return 1;
	}
	for (std::vector<uint64_t>::iterator itV = itRand->second.begin(); itV != itRand->second.end(); ++itV)
	{
		RandomCollectionTable *table = get_config_by_id(*itV, &random_collect_config);
		if (table == NULL)
		{
			return 2;
		}
		//for (uint32_t i = 0; i < table->Num; ++i)
		{
			uint32_t pos = rand() % table->n_PointX;
			Collect *ret = CreateCollectByPos(scene, table->CollectionID, table->PointX[pos], 10000, table->PointZ[pos], 0);
			if (ret == NULL)
				return 3;
			ret->m_rand_id = *itV;
			LOG_ERR("%s %d: scene[%lu] CreateRandCollect failed x=%f,y=%f", __FUNCTION__, __LINE__, scene->m_id, table->PointX[pos], table->PointZ[pos]);
		}
	}
	return 0;
}
