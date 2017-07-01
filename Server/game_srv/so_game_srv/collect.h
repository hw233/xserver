#ifndef _COLLECT_H__
#define _COLLECT_H__

#include "conn_node_gamesrv.h"
#include "unit_path.h"
#include <map>

class player_struct;
class scene_struct;
struct area_struct;

enum COLLECT_STATE
{
	COLLECT_NORMOR,
	COLLECT_GATHING,
	COLLECT_RELIVE,
	COLLECT_DESTROY
};

class Collect;
typedef std::map<uint32_t, Collect *> COLLECT_MAP;
extern uint32_t collect_manager_s_id;
extern COLLECT_MAP collect_manager_s_collectContain;

class Collect 
{
public:
	Collect();
	~Collect();

	void AddAreaPlayerToSight(area_struct *area, uint16_t *add_player_id_index, uint64_t *add_player);

	void BroadcastCollectCreate();
	void BroadcastCollectDelete();
	void BroadcastToSight(uint16_t msg_id, void *msg_data, pack_func func);

	//采集操作相关
	void CashTruckDrop(player_struct &player);

	//开始采集
	int BegingGather(player_struct *player);
	bool InterruptGather();
	int GatherComplete(player_struct *player);
	int GatherInterupt(player_struct *player);

	//定时器
	bool OnTick();

	scene_struct *scene;
	area_struct *area;
	position m_pos;
	uint32_t m_uuid; //唯一id
	uint32_t m_collectId;  //采集点ID
	uint64_t m_gatherPlayer;
	uint64_t m_commpleteTime;
	uint64_t m_reliveTime;
	uint64_t m_liveTime;
	int32_t m_state;
	int32_t m_minType;
	uint32_t m_ownerLv;
	uint64_t m_active; //镖车活动表id 
	float m_y;
	float m_yaw;  //朝向
	uint32_t m_scenceId;
	uint32_t m_guild_id;
	uint64_t m_raid_uuid; //副本唯一ID

	static Collect * CreateCollect(scene_struct *scene, int index);
	static Collect *CreateCollectByConfig(scene_struct *scene, int index);
	static Collect *CreateCollectByPos(scene_struct *scene, uint32_t id, uint32_t x, uint32_t y, uint32_t z, float yaw);
	static int CreateCollectByID(scene_struct *scene, uint32_t id, uint32_t num);
	static void DestroyCollect(uint64_t id);
	static Collect * GetById(const uint32_t id);
	static void Tick();
	
	static uint32_t get_total_collect_num();
};

#endif
