#ifndef _COLLECT_H__
#define _COLLECT_H__

#include "conn_node_gamesrv.h"
#include "unit_path.h"
#include "move.pb-c.h"
#include <map>

class player_struct;
class scene_struct;
class sight_space_struct;
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
	void pack_sight_collect_info(SightCollectInfo *point, PosData *pos);
	void BroadcastCollectCreate();
	void BroadcastCollectDelete();
	void BroadcastToSight(uint16_t msg_id, void *msg_data, pack_func func);
	void NotifyCollectCreate(player_struct *player);

	//采集操作相关
	void CashTruckDrop(player_struct &player);

	//开始采集
	int BegingGather(player_struct *player, uint32_t step);
	bool InterruptGather();
	int GatherComplete(player_struct *player);
	int GatherInterupt(player_struct *player);

	//定时器
	bool OnTick();

	scene_struct *scene;
	area_struct *area;
	sight_space_struct *sight_space;
	position m_pos;
	uint32_t m_uuid; //唯一id
	uint32_t m_collectId;  //采集点ID
	uint64_t m_gatherPlayer;
	uint64_t m_commpleteTime;
	uint64_t m_reliveTime;
	uint64_t m_liveTime;
	int32_t m_state;
	uint32_t m_dropId; //
	int32_t m_minType; //根据类型掉落 0普通 1寻宝 2日常阵营镖车到达 3日常阵营镖车死亡
	uint32_t m_ownerLv; //根据所有者的等级掉落
	uint64_t m_active; //镖车活动表id 
	float m_y;
	float m_yaw;  //朝向
	uint32_t m_scenceId;
	uint32_t m_guild_id;
	uint64_t m_raid_uuid; //副本唯一ID

	static Collect *create_sight_space_collect(sight_space_struct *sight_space, uint32_t id, double x, double y, double z, float yaw);	
	static Collect *CreateCollectByConfig(scene_struct *scene, int index);
	static Collect *CreateCollectByPos(scene_struct *scene, uint32_t id, double x, double y, double z, float yaw);
	static Collect *CreateCollectByPos(scene_struct *scene, uint32_t id, double x, double y, double z, float yaw, player_struct *player); //只自己看得见的采集点
	static int CreateCollectByID(scene_struct *scene, uint32_t id, uint32_t num); //创建相同ID 最多num个不同位置的采集点
	static void RemoveFromSceneAndDestroyCollect(Collect *pCollect, bool send_msg = false);
	static void DestroyCollect(uint64_t id);
	static Collect * GetById(const uint32_t id);
	static void Tick();
	
	static uint32_t get_total_collect_num();

	static int CreateRandCollect(scene_struct *scene);

private:
	static Collect *CreateCollectImp(uint32_t id, double x, double y, double z, float yaw);
	int DoGatherDrop(player_struct *player);
};

#endif
