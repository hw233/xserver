#ifndef __PARTNER_MANAGER_H__
#define __PARTNER_MANAGER_H__

#include "mem_pool.h"
#include "partner.h"

extern std::map<uint64_t, partner_struct *> partner_manager_all_partner_id;
extern std::list<partner_struct *> partner_manager_partner_free_list;
extern std::set<partner_struct *> partner_manager_partner_used_list;
extern struct mass_pool partner_manager_partner_data_pool;
extern struct minheap partner_manager_minheap;

class partner_manager
{
public:
	partner_manager();
	virtual ~partner_manager();
	static void on_tick_5();
	static void partner_ontick_settimer(partner_struct *p);
	static void partner_ontick_reset_timer(partner_struct *p);
	static partner_struct *get_ontick_partner(uint64_t now);
	static void partner_ontick_delete(partner_struct *p);
	static int reset_all_partner_ai();
	
	static void delete_partner(partner_struct *p);
	static partner_struct *create_partner(uint32_t partner_id, player_struct *owner, uint64_t uuid, bool init_name);
	static partner_struct * get_partner_by_uuid(uint64_t uuid);

	static int reinit_partner_min_heap();
	static int init_partner_struct(int num, unsigned long key);
	static int resume_partner_struct(int num, unsigned long key);	
private:
	static int add_partner(partner_struct *p);
	static int remove_partner(partner_struct *p);
	static partner_struct *alloc_partner();
};

#endif /* __PARTNER_MANAGER_H__ */
