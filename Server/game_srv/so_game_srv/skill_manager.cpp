#include "skill_manager.h"
#include "uuid.h"
#include "game_event.h"
#include <string.h>

skill_manager::skill_manager()
{
}

skill_manager::~skill_manager()
{
}

/////////////////////  以下是静态成员
//std::map<uint64_t, skill_struct *> skill_manager::all_skills_id;

// std::list<skill_struct *> skill_manager::skill_manager_skill_free_list;
// std::set<skill_struct *> skill_manager::skill_manager_skill_used_list;
// struct comm_pool skill_manager::skill_manager_skill_data_pool;

int skill_manager::init_skill_struct(int num, unsigned long key)
{
	skill_struct *skill;
	for (int i = 0; i < num; ++i) {
		skill = new skill_struct();
		skill_manager_skill_free_list.push_back(skill);
	}
	LOG_DEBUG("%s: init mem[%d][%d]", __FUNCTION__, sizeof(skill_struct) * num, sizeof(skill_data) * num);	
	return init_comm_pool(0, sizeof(skill_data), num, key, &skill_manager_skill_data_pool);
}

skill_struct *skill_manager::copy_skill(skill_struct *p)
{
	skill_struct *ret = alloc_skill();
	if (!ret)
		return NULL;
	ret->copy(p);
	return ret;
}

skill_struct *skill_manager::create_skill(uint64_t id, uint64_t owner, uint64_t target)
{
	skill_struct *ret = alloc_skill();
	if (!ret)
		return NULL;
	if (ret->init_skill(id, owner, target) != 0)
	{
		delete_skill(ret);
		return NULL;
	}
	return ret;
}


skill_struct *skill_manager::alloc_skill()
{
	skill_struct *ret = NULL;
	skill_data *data = NULL;
	if (skill_manager_skill_free_list.empty())
		return NULL;
	ret = skill_manager_skill_free_list.back();
	skill_manager_skill_free_list.pop_back();
	data = (skill_data *)comm_pool_alloc(&skill_manager_skill_data_pool);
	if (!data)
		goto fail;
	memset(data, 0, sizeof(skill_data));
	ret->data = data;
	skill_manager_skill_used_list.insert(ret);
	LOG_INFO("[%s:%d] skill:%p, data:%p", __FUNCTION__, __LINE__, ret, ret->data);	
	return ret;
fail:
	if (ret) {
		skill_manager_skill_used_list.erase(ret);
		skill_manager_skill_free_list.push_back(ret);
	}
	if (data) {
		comm_pool_free(&skill_manager_skill_data_pool, data);
	}
	return NULL;
}

uint32_t skill_manager::get_skill_count()
{
	return skill_manager_skill_used_list.size();
}

uint32_t skill_manager::get_skill_pool_max_num()
{
	return skill_manager_skill_data_pool.num;
}

void skill_manager::delete_skill(skill_struct *p)
{
	LOG_INFO("[%s:%d] skill:%p, data:%p", __FUNCTION__, __LINE__, p, p->data);
	
	skill_manager_skill_used_list.erase(p);
	skill_manager_skill_free_list.push_back(p);

	if (p->data) {
		comm_pool_free(&skill_manager_skill_data_pool, p->data);		
		p->data = NULL;
	}
}

void skill_manager::on_tick_1()
{
}

void skill_manager::on_tick_10()
{
	// for (std::set<skill_struct *>::iterator iter = skill_manager_skill_used_list.begin(); iter != skill_manager_skill_used_list.end(); ++iter)
	// {
	// 	(*iter)->on_tick();
	// }
}

