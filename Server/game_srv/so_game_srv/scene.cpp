#include "scene.h"
//#include "team.h"
#include "game_event.h"
#include "game_config.h"
#include "monster_manager.h"
#include "player.h"
#include "partner.h"
#include "collect.h"
#include "cash_truck.h"
#include "uuid.h"
#include "unit.h"
#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include "msgid.h"

struct minheap g_minheap;
struct map_block **closed_map_block;
//#define AREA_WIDTH (15)

extern std::map<uint64_t, uint64_t> g_special_mon_map;

__attribute__((unused)) static void	dump_map_config(struct map_config *map_config)
{
	printf("offset %d %d\n", map_config->offset_x, map_config->offset_z);
	for (int z = 0; z < map_config->size_z; ++z)
	{
		for (int x = 0; x < map_config->size_x; ++x)
		{
			int index = z * map_config->size_x + x;
			printf("[%05d, %d, %05d] ", index,
				map_config->block_data[index].can_walk,
				map_config->block_data[index].first_block);
		}
		printf("\n");
	}
}

scene_struct::~scene_struct()
{
	clear();
}

scene_struct::scene_struct()
{
	m_area_size = 0;
	m_area = NULL;
	m_guild_id = 0;
}

void scene_struct::clear()
{
	for (int i = 0; i < m_area_size; ++i)
		m_area[i].clean_area_struct();
	
	free(m_area);
	m_area = NULL;
	m_area_size = 0;
	clear_all_collet();
	m_guild_id = 0;
}

uint32_t scene_struct::get_area_width()
{
	return 15;
}

int scene_struct::init_scene_struct(uint64_t sceneid, bool create_monster, int lv)
{
		// 从配置读取地图资源路径
	std::map<uint64_t, struct SceneResTable *>::iterator iter = scene_res_config.find(sceneid);
	if (iter == scene_res_config.end())
	{
		LOG_ERR("%s: load scene %lu config failed", __FUNCTION__, sceneid);
		return (-1);
	}

	res_config = iter->second;
	m_born_x = (int64_t)res_config->BirthPointX;
	m_born_y = (int64_t)res_config->BirthPointY;
	m_born_z = (int64_t)res_config->BirthPointZ;
	m_born_direct = (int64_t)res_config->FaceY;

	map_config = get_config_by_id(sceneid, &scene_map_config);
	if (!map_config)
	{
		LOG_ERR("%s %d: load map %lu failed", __FUNCTION__, __LINE__, sceneid);
		return -10;
	}
	region_config = get_config_by_id(sceneid, &scene_region_config);

	m_id = sceneid;
	m_area_width = get_area_width();

//	dump_map_config(map_config);
	
	m_area_x_size = map_config->size_x / m_area_width;
	if (map_config->size_x % m_area_width) ++m_area_x_size;
	m_area_z_size = map_config->size_z / m_area_width;
	if (map_config->size_z % m_area_width) ++m_area_z_size;
	m_area_size = m_area_x_size * m_area_z_size;
	m_area = (area_struct *)malloc(sizeof(area_struct) * m_area_size);
	if (!m_area)
	{
		LOG_ERR("%s %d: scene[%lu] malloc area %d failed", __FUNCTION__, __LINE__, sceneid, m_area_size);
		return -20;
	}
	
	for (int i = 0; i < m_area_size; ++i)
	{
		m_area[i].init_area_struct(m_area, i, m_area_x_size, m_area_z_size);
	}

	unsigned max_size = map_config->size_x * map_config->size_z;
	if (max_size > g_minheap.max_size)
	{
		free(g_minheap.nodes);
		init_heap(&g_minheap, max_size, g_minheap.cmp, g_minheap.get, g_minheap.set);
		free(closed_map_block);
		closed_map_block = (struct map_block **)malloc(sizeof(void *) * max_size);
	}

	create_monster_config = all_scene_create_monster_config[sceneid];

	if (create_monster)
		create_all_monster(lv);

	int ret = Collect::CreateRandCollect(this);
	if (ret != 0)
	{
		LOG_ERR("%s %d: scene[%lu] CreateRandCollect failed ret =  %d", __FUNCTION__, __LINE__, sceneid, ret);
	}

	return (0);
}

area_struct *scene_struct::get_area_by_pos(float pos_x, float pos_z)
{
//	int id = calc_area_id(pos_x - map_config->offset_x, pos_z - map_config->offset_z);
//	int id = calc_area_id(pos_x + map_config->size_x / 2.0 - map_config->offset_x,
//		pos_z + map_config->size_z / 2.0 - map_config->offset_x);

	if (!check_pos_valid(map_config, pos_x, pos_z))
		return NULL;

	int id = calc_area_id(
		pos_to_block_x(map_config, pos_x),
		pos_to_block_z(map_config, pos_z)
						  );
	
	if (id < 0 || id >= m_area_size)
		return NULL;
	return get_area(id);
}

int scene_struct::calc_area_id(int pos_x, int pos_z)
{
	uint16_t x = (pos_x + 1) / m_area_width;
	uint16_t z = (pos_z + 1) / m_area_width;
	if (x >= m_area_x_size || z >= m_area_z_size)
		return -1;
	return z * m_area_x_size + x;
}

area_struct *scene_struct::get_area(int id)
{
	assert(m_area);
	assert(id < m_area_size);
	return &m_area[id];
}

//add scene->map_config->offset
struct position scene_struct::get_area_pos(area_struct *area)
{
	struct position ret;
	int id = area - m_area;
	int id_x = id % m_area_x_size;
	int id_z = id / m_area_z_size;
	ret.pos_x = id_x * m_area_width;
	ret.pos_z = id_z * m_area_width;	
	return ret;
}

int scene_struct::broadcast_partner_create(partner_struct *partner)
{
	partner->broadcast_partner_create();
	return (0);
}
int scene_struct::broadcast_monster_create(monster_struct *monster)
{
	monster->broadcast_monster_create();
	return (0);
}
int scene_struct::broadcast_player_create(player_struct *player)
{
	player->broadcast_player_create(this);
	return (0);
}
int scene_struct::broadcast_player_delete(player_struct *player)
{
	player->broadcast_player_delete();
	return (0);
}

int scene_struct::create_all_monster(int lv)
{
//	return (0);
	if (!create_monster_config)
	{
		return (-1);
	}
	int len = create_monster_config->size();
	for (int i = 0; i < len; ++i)
	{
		if (monster_manager::create_monster_by_config(this, i, lv) == NULL)
			Collect::CreateCollectByConfig(this, i) ;
	}
//	std::vector<struct SceneCreateMonsterTable *>::iterator ite;
//	for (ite = create_monster_config->begin(); ite != create_monster_config->end(); ++ite)
//	{
//		monster_manager::create_monster_at_position((*ite)->ID, this, (*ite)->PointPosX, (*ite)->PointPosZ);
//	}
	return (0);
}

int scene_struct::add_partner_to_scene(partner_struct *partner)
{
	assert(partner->area == NULL);
	struct unit_path *move_path = &partner->data->move_path;	
	area_struct *area = get_area_by_pos(move_path->pos[move_path->cur_pos].pos_x, move_path->pos[move_path->cur_pos].pos_z);
	if (!area)
	{
		LOG_ERR("%s: add partner %u %lu to scene[%u] fail, pos[%.1f][%.1f]", __FUNCTION__, partner->data->partner_id,
			partner->data->uuid, m_id, move_path->pos[move_path->cur_pos].pos_x, move_path->pos[move_path->cur_pos].pos_z);
		return -1;
	}
	area->add_partner_to_area(partner->data->uuid);
//	partner->data->scene_id = m_id;
	partner->scene = this;
	partner->area = area;
//	if (partner->ai && partner->ai->on_alive)
//		partner->ai->on_alive(partner);
	broadcast_partner_create(partner);
	return (0);
}

int scene_struct::add_monster_to_scene(monster_struct *monster, uint32_t effectid)
{
	assert(monster->area == NULL);
	struct unit_path *move_path = &monster->data->move_path;	
	area_struct *area = get_area_by_pos(move_path->pos[move_path->cur_pos].pos_x, move_path->pos[move_path->cur_pos].pos_z);
	if (!area)
	{
		LOG_ERR("%s: add monster %u %lu to scene[%u] fail, pos[%.1f][%.1f]", __FUNCTION__, monster->data->monster_id,
			monster->data->player_id, m_id, move_path->pos[move_path->cur_pos].pos_x, move_path->pos[move_path->cur_pos].pos_z);
		return -1;
	}
	area->add_monster_to_area(monster->data->player_id);
	monster->data->scene_id = m_id;
	monster->data->guild_id = m_guild_id;
	monster->scene = this;
	monster->area = area;
	if (monster->ai && monster->ai->on_alive)
		monster->ai->on_alive(monster);
	broadcast_monster_create(monster);
	if (monster->config->SpecialDisplayIs > 0)
	{
		g_special_mon_map.insert(std::make_pair(monster->get_uuid(), m_id));
	}
	return (0);
}

int scene_struct::add_cash_truck_to_scene(cash_truck_struct *pTruck)
{
	assert(pTruck->area == NULL);
	struct unit_path *move_path = &pTruck->data->move_path;
	area_struct *area = get_area_by_pos(move_path->pos[move_path->cur_pos].pos_x, move_path->pos[move_path->cur_pos].pos_z);
	if (!area)
	{
		LOG_ERR("%s: add truck %u %lu to scene[%u] fail, pos[%.1f][%.1f]", __FUNCTION__, pTruck->data->monster_id,
			pTruck->data->player_id, m_id, move_path->pos[move_path->cur_pos].pos_x, move_path->pos[move_path->cur_pos].pos_z);
		return -1;
	}
	area->add_truck_to_area(pTruck->data->player_id);
	pTruck->data->scene_id = m_id;
//	pTruck->data->guild_id = m_guild_id;
	pTruck->scene = this;
	pTruck->area = area;

	pTruck->broadcast_cash_truck_create();
	return (0);
}

int scene_struct::add_player_to_scene(player_struct *player)
{
	assert(player->area == NULL);
	assert(player->sight_space == NULL);
	struct position *pos = player->get_pos();
	area_struct *area = get_area_by_pos(pos->pos_x, pos->pos_z);
	if (!area)
	{
		LOG_ERR("%s %d: player[%lu] add area[%.1f][%.1f] failed", __FUNCTION__, __LINE__, player->data->player_id, pos->pos_x, pos->pos_z);
		return -1;
	}
	LOG_DEBUG("%s %d: scene[%d] player[%lu] area[%p] pos[%.1f][%.1f]", __FUNCTION__, __LINE__, m_id, player->data->player_id, area, pos->pos_x, pos->pos_z);	
	area->add_player_to_area(player->data->player_id);
	player->area = area;
	player->data->scene_id = m_id;
	if (get_scene_type() != SCENE_TYPE_RAID)
		player->set_camp_id(0);
	player->scene = this;
//	player->data->player_is_in_loading = false;	
	broadcast_player_create(player);
	
	player->update_region_id();
	player->on_enter_scene(this);
	return (0);
}

int scene_struct::add_collect_to_scene(Collect *pCollect)
{
	area_struct *area = get_area_by_pos(pCollect->m_pos.pos_x, pCollect->m_pos.pos_z);
	if (!area)
	{
		LOG_ERR("%s %d: scene[%u] collect[%u] add area[%.1f][%.1f] failed", __FUNCTION__, __LINE__, m_id, pCollect->m_collectId, pCollect->m_pos.pos_x, pCollect->m_pos.pos_z);
		return -1;
	}
	area->add_collect_to_area(pCollect->m_uuid);
	pCollect->area = area;
	pCollect->scene = this;
	pCollect->m_scenceId = m_id;
	pCollect->m_guild_id = m_guild_id;

	pCollect->BroadcastCollectCreate();
	m_collect.insert(pCollect->m_uuid);
	return 0;
}

int scene_struct::delete_player_from_scene(player_struct *player)
{
//	area_struct *area = get_area_by_pos(player->get_player_pos()->pos_x, player->get_player_pos()->pos_z);
	area_struct *area = player->area;
	if (!area)
		return -1;
	LOG_DEBUG("%s %d: scene[%d] player[%lu] area[%p]", __FUNCTION__, __LINE__, m_id, player->data->player_id, area);
	broadcast_player_delete(player);
	if (area->del_player_from_area(player->data->player_id) != 0)
	{
		LOG_ERR("%s %d: can not del player[%lu] from area[%ld]", __FUNCTION__, __LINE__, player->data->player_id, area - m_area);
	}
		//如果要重新加入场景，必须要保留这个ID
//	player->data->scene_id = 0;
	player->scene = NULL;
	player->area = NULL;
//	player->data->player_is_in_loading = true;
	player->on_leave_scene(this);
	return (0);
}

int scene_struct::delete_partner_from_scene(partner_struct *partner, bool send_msg)
{
	area_struct *area = partner->area;
	if (!area)
		return -1;
	
	LOG_DEBUG("%s %d: scene[%d] player[%lu] area[%p], sight_player[%u] sight_partner[%u]",
		__FUNCTION__, __LINE__, m_id, partner->data->uuid, area,
		partner->data->cur_sight_player, partner->data->cur_sight_partner);
	
	partner->broadcast_partner_delete(send_msg);
	if (area->del_partner_from_area(partner->data->uuid) != 0)
	{
		LOG_ERR("%s %d: can not del partner[%lu] from area[%ld]", __FUNCTION__, __LINE__, partner->data->uuid, area - m_area);
	}
		//如果要重新加入场景，必须要保留这个ID
//	player->data->scene_id = 0;
	partner->scene = NULL;
	partner->area = NULL;
	return (0);
}
int scene_struct::delete_monster_from_scene(monster_struct *monster, bool send_msg)
{
	area_struct *area = monster->area;
	if (!area)
		return -1;
	
	LOG_DEBUG("%s %d: scene[%d] player[%lu] area[%p], sight_player[%u] sight_monster[%u]",
		__FUNCTION__, __LINE__, m_id, monster->data->player_id, area,
		monster->data->cur_sight_player, monster->data->cur_sight_monster);
	
	monster->broadcast_monster_delete(send_msg);
	if (area->del_monster_from_area(monster->data->player_id) != 0)
	{
		LOG_ERR("%s %d: can not del monster[%lu] from area[%ld]", __FUNCTION__, __LINE__, monster->data->player_id, area - m_area);
	}
		//如果要重新加入场景，必须要保留这个ID
//	player->data->scene_id = 0;
	monster->scene = NULL;
	monster->area = NULL;
	if (monster->config->SpecialDisplayIs > 0)
	{
		g_special_mon_map.erase(monster->get_uuid());
	}
	return (0);
}

int scene_struct::delete_cash_truck_from_scene(cash_truck_struct *pTruck, bool send_msg)
{
	area_struct *area = pTruck->area;
	if (!area)
		return -1;

	LOG_DEBUG("%s %d: scene[%d] player[%lu] area[%p], sight_player[%u] sight_monster[%u]",
		__FUNCTION__, __LINE__, m_id, pTruck->data->player_id, area,
		pTruck->data->cur_sight_player, pTruck->data->cur_sight_monster);

	pTruck->broadcast_cash_truck_delete(send_msg);
	if (area->del_truck_from_area(pTruck->data->player_id) != 0)
	{
		LOG_ERR("%s %d: can not del monster[%lu] from area[%ld]", __FUNCTION__, __LINE__, pTruck->data->player_id, area - m_area);
	}
	//如果要重新加入场景，必须要保留这个ID
	//	player->data->scene_id = 0;
	pTruck->scene = NULL;
	pTruck->area = NULL;
	return 0;
}

int scene_struct::delete_collect_from_scene(Collect *pCollect, bool send_msg)
{
	area_struct *area = pCollect->area;
	if (!area)
		return -1;
	LOG_DEBUG("%s %d: scene[%d] collect[%u] area[%p]", __FUNCTION__, __LINE__, m_id, pCollect->m_uuid, area);
	if (send_msg)
	{
		pCollect->BroadcastCollectDelete();
	}
	if (area->del_collect_from_area(pCollect->m_uuid) != 0)
	{
		LOG_ERR("%s %d: can not del collect[%u] from area[%ld]", __FUNCTION__, __LINE__, pCollect->m_uuid, area - m_area);
	}

	m_collect.erase(pCollect->m_uuid);
	pCollect->scene = NULL;
	pCollect->area = NULL;
	return (0);
}

SCENE_TYPE_DEFINE scene_struct::get_scene_type()
{
	return SCENE_TYPE_WILD;
}

void scene_struct::on_monster_dead(monster_struct *monster, unit_struct *killer)
{
}

void scene_struct::on_player_dead(player_struct *player, unit_struct *killer)
{
}

void scene_struct::on_collect(player_struct *player, Collect *collect)
{
}

bool scene_struct::is_in_zhenying_raid()
{
	return false;
}

bool scene_struct::can_use_horse()
{
	if (res_config && res_config->UseMounts == 0)
		return false;
	return true;
}
bool scene_struct::can_transfer(uint32_t type)
{
	if (!res_config)
		return true;
	for (size_t i = 0; i < res_config->n_UseDelivery; ++i)
	{
		if (type == res_config->UseDelivery[i])
			return false;
	}
	return true;
}

void scene_struct::get_all_player(std::set<uint64_t> &playerIds)
{
	for (uint16_t i = 0; i < m_area_size; ++i)
	{
		area_struct *area = &m_area[i];
		for (int j = 0; j < area->cur_player_num; ++j)
		{
			playerIds.insert(area->m_player_ids[j]);
		}
	}
}

void scene_struct::get_relive_pos(float pos_x, float pos_z, int32_t *ret_pos_x, int32_t *ret_pos_z, int32_t *ret_direct)
{
	double distance = 0xffffffff;
	int index = -1;
	for (size_t i = 0; i < res_config->n_RelivePointX; ++i)
	{
		double t = (pos_x - res_config->RelivePointX[i]) * (pos_x - res_config->RelivePointX[i]) + (pos_z - res_config->RelivePointZ[i]) * (pos_z - res_config->RelivePointZ[i]);
		if (t < distance)
		{
			distance = t;
			index = i;
		}
	}
	assert(index >= 0);
	int rand_x = random();
	rand_x = rand_x % (2 * res_config->ReliveRange[index]);
	rand_x = rand_x - res_config->ReliveRange[index];
	int rand_z = random();
	rand_z = rand_z % (2 * res_config->ReliveRange[index]);
	rand_z = rand_z - res_config->ReliveRange[index];
	*ret_pos_x = res_config->RelivePointX[index] + rand_x;
	*ret_pos_z = res_config->RelivePointZ[index] + rand_z;
	*ret_direct = res_config->ReliveFaceY[index];
}

void scene_struct::clear_all_collet()
{
	for (std::set<uint64_t>::iterator it = m_collect.begin(); it != m_collect.end(); ++it)
	{
		Collect::DestroyCollect(*it);
	}
	m_collect.clear();
}

// int scene_struct::enter_other_scene(player_struct *player, scene_struct *new_scene, double pos_x, double pos_y, double pos_z, double direct)
// {
// //  是否需要检查能否传送 ?
// //	int ret = player->check_can_transfer();
// //  player->scene->can_transfer(type))

// 	if (player->sight_space)
// 	{
// 		LOG_ERR("%s: player[%lu] in sightspace, can not transfer", __FUNCTION__, player->data->player_id);
// 		return (-1);
// 	}
// 	if (!player->scene)
// 	{
// 		LOG_ERR("%s: player[%lu] not in scene, can not transfer", __FUNCTION__, player->data->player_id);
// 		return (-10);
// 	}
// 	assert(player->scene->get_scene_type() == SCENE_TYPE_WILD);

// 	if (new_scene->m_id == player->data->scene_id)
// 	{
// 		assert(player->scene == new_scene);
// 		player->send_clear_sight();
// 		player->scene->delete_player_from_scene(player);
// 		player->set_pos(pos_x, pos_z);
// 		new_scene->add_player_to_scene(player);
// 		player->take_partner_into_scene();
// 	}
// 	else
// 	{
// 		player->data->last_scene_id = player->data->scene_id;
// 		player->scene->delete_player_from_scene(player);
// 		player->data->scene_id = new_scene->m_id;
// 		player->set_pos(pos_x, pos_z);
// 	}
// 	player->data->pos_y = pos_y;

// 	player->data->m_angle = unity_angle_to_c_angle(direct);
// 	player->send_scene_transfer(direct, pos_x, pos_y, pos_z, new_scene->m_id, 0);

// 	player->interrupt();

// 	if (player->m_team !=  NULL)
// 	{
// 		if (player->m_team->GetLeadId() == player->get_uuid())
// 		{
// 			player->m_team->FollowLeadTrans(new_scene->m_id, pos_x, pos_y, pos_z, direct);
// 		}
// 		player->m_team->broadcast_leader_pos(player->get_pos(), player->data->scene_id, player->get_uuid());
// 	}
// 	return (0);
// }
// int scene_struct::enter_other_raid(player_struct *player, raid_struct *new_raid)
// {
// 	return (0);
// }
	
int scene_struct::player_leave_scene(player_struct *player)
{
	assert(player->scene == this);
//	if (send_clear_sight)
//		player->send_clear_sight();
		
	delete_player_from_scene(player);	
	return (0);
}
int scene_struct::player_enter_scene(player_struct *player, double pos_x, double pos_y, double pos_z, double direct)
{
	assert(!player->sight_space);
	assert(!player->scene);

	player->data->last_scene_id = player->data->scene_id;
	player->data->scene_id = m_id;
	player->set_pos(pos_x, pos_z);
	player->data->pos_y = pos_y;

	player->data->m_angle = unity_angle_to_c_angle(direct);
	player->send_scene_transfer(direct, pos_x, pos_y, pos_z, player->data->last_scene_id, m_id, 0);

	player->interrupt();

	player->on_player_enter_scene(direct);
	// if (player->m_team !=  NULL)
	// {
	// 	if (player->m_team->GetLeadId() == player->get_uuid())
	// 	{
	// 		player->m_team->FollowLeadTrans(m_id, pos_x, pos_y, pos_z, direct);
	// 	}
	// 	player->m_team->broadcast_leader_pos(player->get_pos(), player->data->scene_id, player->get_uuid());
	// }
	return (0);
}

void scene_struct::broadcast_to_scene(uint16_t msg_id, void *msg_data, pack_func func)
{
	PROTO_HEAD_CONN_BROADCAST *head;
	PROTO_HEAD *real_head;

	/** ================广播数据============ **/
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
	head->msg_id = ENDION_FUNC_2(SERVER_PROTO_BROADCAST);
	real_head = &head->proto_head;

	real_head->msg_id = ENDION_FUNC_2(msg_id);
	real_head->seq = 0;
	//	memcpy(real_head->data, msg_data, len);
	size_t len = func(msg_data, (uint8_t *)real_head->data);
	real_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD)+len);

	uint64_t *ppp = (uint64_t*)((char *)&head->player_id + len);

	head->num_player_id = 0;
	std::set<uint64_t> playerIds;
	get_all_player(playerIds);
	for (std::set<uint64_t>::iterator it = playerIds.begin(); it != playerIds.end(); ++it)
	{
		if (get_entity_type(*it) != ENTITY_TYPE_PLAYER)
			continue;
		ppp[head->num_player_id++] = *it;
	}

	if (head->num_player_id == 0)
		return;

	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD_CONN_BROADCAST)+len + sizeof(uint64_t)* head->num_player_id);

	LOG_DEBUG("%s %d: broad [%u] to %d player", __FUNCTION__, __LINE__, ENDION_FUNC_2(head->proto_head.msg_id), head->num_player_id);

	if (conn_node_gamesrv::connecter.send_one_msg((PROTO_HEAD *)head, 1) != (int)(ENDION_FUNC_4(head->len))) {
		LOG_ERR("%s %d: send to all failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
}
