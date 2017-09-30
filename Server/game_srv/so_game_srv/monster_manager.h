#ifndef MONSTER_MANAGER_H
#define MONSTER_MANAGER_H

#include "scene.h"
#include "mem_pool.h"
#include "monster.h"
#include "boss.h"
#include "sight_space.h"
#include "server_proto.h"
#include <map>
#include <set>
#include <list>

extern struct minheap monster_manager_m_minheap;
extern std::list<monster_struct *> monster_manager_delete_list;
extern std::list<monster_struct *> monster_manager_monster_free_list;
extern std::set<monster_struct *> monster_manager_monster_used_list;
//extern std::list<boss_struct *> monster_manager_boss_free_list;
//extern std::set<boss_struct *> monster_manager_boss_used_list;

extern struct minheap monster_manager_m_boss_minheap;
extern struct comm_pool monster_manager_monster_data_pool;
//extern struct comm_pool monster_manager_boss_data_pool;
extern std::map<uint64_t, monster_struct *> monster_manager_all_monsters_id;
extern std::map<uint64_t, monster_struct *> world_boss_all_monsters_id;
//extern std::map<uint64_t, boss_struct *> monster_manager_all_boss_id;
static const int WORD_BOSS_ACTIVE_ID = 330400039; //世界boss ControlTable表id

class monster_manager
{
public:
	monster_manager();
	virtual ~monster_manager();

	static int reset_all_monster_ai();

	static void on_tick_1();
	static void on_tick_5();	
	static void on_tick_10();
	static void on_tick_30();
	static void on_tick_50();
	static void on_tick_100();
	static void on_tick_500();
	static void on_tick_1000();

	static unsigned int get_monster_pool_max_num();
//	static unsigned int get_boss_pool_max_num();		

		//刷出一个怪物并加入相位
	static monster_struct *create_sight_space_monster(sight_space_struct *sight_space, scene_struct *scene,
		uint32_t monster_id, uint32_t level, double pos_x, double pos_z);	
		//刷出一个只有指定玩家能看到的怪物, 没有用
//	static monster_struct *create_sight_only_monster(player_struct *player, uint32_t monster_id, uint32_t level, double pos_x, double pos_z);
		//刷出一个技能召唤的怪物
	static monster_struct *create_call_monster(player_struct *player, SkillTable *skill_config);
		//刷出一个地图编辑器配置的怪物
	static monster_struct *create_monster_at_pos(scene_struct *scene, uint64_t id, uint32_t lv, int32_t pos_x, int32_t pos_z, uint32_t effectid, unit_struct *owner);
	static monster_struct *create_monster_by_config(scene_struct *scene, int index);
	static int create_monster_by_id(scene_struct *scene, uint32_t id, uint32_t num);	
	static monster_struct *add_monster(uint64_t monster_id, uint64_t lv, unit_struct *owner = NULL);
	static void delete_monster(monster_struct *p);
	static monster_struct * get_monster_by_id(uint64_t id);
//	static boss_struct * get_boss_by_id(uint64_t id);

	static int init_monster_struct(int num, unsigned long key);
//	static int init_boss_struct(int num, unsigned long key);
	static int reinit_monster_min_heap();
	static int reinit_boss_min_heap();	
	
	//创建世界boss
	static int add_world_boss_monster();
	static monster_struct * get_world_boss_by_id(uint64_t id);
//	static int resume_monster_struct(int num, unsigned long key);

		//定时器相关接口
	static void monster_ontick_settimer(monster_struct *p);
	static void monster_ontick_reset_timer(monster_struct *p);
	static monster_struct *get_ontick_monster(uint64_t now);
	static void monster_ontick_delete(monster_struct *p);

	static void boss_ontick_settimer(monster_struct *p);
//	static void boss_ontick_reset_timer(monster_struct *p);
	static monster_struct *get_ontick_boss(uint64_t now);
//	static void boss_ontick_delete(monster_struct *p);
private:
//	static int resume_monster_bag_data(monster_struct *monster);
	static int add_monster(monster_struct *p);
	static int remove_monster(monster_struct *p);
	static monster_struct *alloc_monster();

//	static int add_boss(boss_struct *p);
//	static int remove_boss(boss_struct *p);
//	static boss_struct *alloc_boss();

	static void delete_monster_impl(monster_struct *p);
//	static void delete_boss_impl(boss_struct *p);

private:
};

#endif /* MONSTER_MANAGER_H */
