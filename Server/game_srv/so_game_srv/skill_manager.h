#ifndef SKILL_MANAGER_H
#define SKILL_MANAGER_H

#include <stdint.h>
#include <set>
#include <list>
#include "mem_pool.h"
#include "skill.h"

extern std::list<skill_struct *> skill_manager_skill_free_list;
extern std::set<skill_struct *> skill_manager_skill_used_list;
extern struct comm_pool skill_manager_skill_data_pool;	

class skill_manager
{
public:
	skill_manager();
	virtual ~skill_manager();
	static void on_tick_1();
	static void on_tick_10();
	static int init_skill_struct(int num, unsigned long key);
	static skill_struct *create_skill(uint64_t id, uint64_t owner, uint64_t target);
	static void delete_skill(skill_struct *p);

	static uint32_t get_skill_count();
	static uint32_t get_skill_pool_max_num();
	static skill_struct *copy_skill(skill_struct *p);
	static skill_struct *copy_skill(struct skill_data *skill);		
	
private:
	static skill_struct *alloc_skill();	
};


#endif /* SKILL_MANAGER_H */
