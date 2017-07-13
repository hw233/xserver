#ifndef BUFF_MANAGER_H
#define BUFF_MANAGER_H

#include <list>
#include <set>
#include "player_db.pb-c.h"
#include "mem_pool.h"
#include "minheap.h"
#include "buff.h"
#include "scene.h"
#include "unit.h"

extern struct minheap buff_manager_m_minheap;	
extern std::list<buff_struct *> buff_manager_buff_free_list;
extern std::set<buff_struct *> buff_manager_buff_used_list;
extern struct mass_pool buff_manager_buff_data_pool;

class buff_manager
{
public:
	buff_manager();
	~buff_manager();

	static uint64_t get_buff_first_effect_type(uint64_t id);	
	static bool is_move_buff_effect(uint64_t effect_id);  //是否是击飞击倒击退buff
	static bool is_clear_debuff_buff_effect(uint64_t effect_id);	 //是否是清除debuff的效果
	static void on_tick_30();
	static int init_buff_struct(int num, unsigned long key);
	static int reinit_min_heap();

//	static buff_struct *add_buff(uint64_t player_id);
//	static int add_buff(buff_struct *p);
	static void delete_buff(buff_struct *p);   //不要直接调用这个接口，调用buff_struct::del()来删除buff
	static buff_struct *create_default_buff(uint64_t id, unit_struct *attack, unit_struct *owner, bool notify = false);	
//	static int add_skill_buff(unit_struct *attack, unit_struct *owner, int add_num, uint32_t *buff_id, uint32_t *buff_end_time);

	static int load_item_buff(player_struct *player, ItemBuff *db_item_buff);  //从db里面读取的道具buff

		//定时器相关接口
	static void buff_ontick_settimer(buff_struct *p);
	static buff_struct *get_ontick_buff(uint64_t now);
	static void buff_ontick_delete(buff_struct *p);

	static uint32_t get_buff_count();
	static uint32_t get_buff_pool_max_num();	
	
private:
	static buff_struct *alloc_buff();
	static int do_move_buff_effect(struct BuffTable *config, unit_struct *attack, unit_struct *owner);
	static buff_struct *create_buff(uint64_t id, uint64_t end_time, unit_struct *attack, unit_struct *owner, bool notify = false);
};


#endif /* BUFF_MANAGER_H */
