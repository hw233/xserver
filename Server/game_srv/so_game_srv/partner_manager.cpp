#include "partner_manager.h"
#include "uuid.h"

partner_manager::partner_manager()
{
}

partner_manager::~partner_manager()
{
}

static bool minheap_cmp_partner_timeout(void *a, void *b)
{
	partner_struct *aa = (partner_struct *)a;
	partner_struct *bb = (partner_struct *)b;
	if (aa->data->ontick_time < bb->data->ontick_time)
		return true;
	return false;
}

static int minheap_get_partner_timeout_index(void *a)
{
	partner_struct *aa = (partner_struct *)a;
	return aa->data->heap_index;
}

static void minheap_set_partner_timeout_index(int index, void *a)
{
	partner_struct *aa = (partner_struct *)a;
	aa->data->heap_index = index;
}

void partner_manager::partner_ontick_settimer(partner_struct *p)
{
	push_heap(&partner_manager_minheap, p);
}

void partner_manager::partner_ontick_reset_timer(partner_struct *p)
{
	adjust_heap_node(&partner_manager_minheap, p);
}

partner_struct *partner_manager::get_ontick_partner(uint64_t now)
{
	if (partner_manager_minheap.cur_size == 0)
		return NULL;

	if (((partner_struct *)get_heap_first(&partner_manager_minheap))->data->ontick_time > now)
		return NULL;
	return (partner_struct *)pop_heap(&partner_manager_minheap);
}

void partner_manager::partner_ontick_delete(partner_struct *p)
{
	erase_heap_node(&partner_manager_minheap, p);
}

int partner_manager::init_partner_struct(int num, unsigned long key)
{
	init_heap(&partner_manager_minheap, num, minheap_cmp_partner_timeout, minheap_get_partner_timeout_index, minheap_set_partner_timeout_index);
	
	partner_struct *partner;
	for (int i = 0; i < num; ++i) {
		partner = new partner_struct();
		partner_manager_partner_free_list.push_back(partner);
	}
	LOG_DEBUG("%s: init mem[%d][%d]", __FUNCTION__, sizeof(partner_struct) * num, sizeof(partner_data) * num);				
	return init_mass_pool(0, sizeof(partner_data), num, key, &partner_manager_partner_data_pool);
}

int partner_manager::reinit_partner_min_heap()
{
	partner_manager_minheap.cmp = minheap_cmp_partner_timeout;
	partner_manager_minheap.get = minheap_get_partner_timeout_index;
	partner_manager_minheap.set = minheap_set_partner_timeout_index;
	return (0);
}

void partner_manager::on_tick_5()
{
	uint64_t now = time_helper::get_cached_time();
	partner_struct *partner = get_ontick_partner(now);
	while (partner != NULL)
	{
			// TODO: ai间隔
//		partner->data->ontick_time = now + partner->ai_config->Response;
		partner->data->ontick_time = now + 1000;
		if (!partner->data->stop_ai)
			partner->on_tick();
		if (partner->data)
			partner_ontick_settimer(partner);
		partner = get_ontick_partner(now);
	}
}

void partner_manager::delete_partner(partner_struct *p)
{
	LOG_DEBUG("[%s:%d] uuid[%lu], partner:%p, data:%p", __FUNCTION__, __LINE__, p->data->uuid, p, p->data);
	
	if (p->scene)
		p->scene->delete_partner_from_scene(p, true);
	
	partner_manager_partner_used_list.erase(p);
	partner_manager_partner_free_list.push_back(p);

	if (is_node_in_heap(&partner_manager_minheap, p))
		partner_ontick_delete(p);

	p->clear_all_buffs();
	
	if (p->data) {
		remove_partner(p);
		mass_pool_free(&partner_manager_partner_data_pool, p->data);
		p->data = NULL;
	}
}

partner_struct *partner_manager::create_partner(uint32_t partner_id, player_struct *owner, uint64_t uuid)
{
	partner_struct *ret = alloc_partner();
	if (!ret)
		return NULL;
	ret->data->uuid = (uuid > 0 ? uuid : alloc_partner_uuid());
	ret->init_partner(partner_id, owner);

	add_partner(ret);

	LOG_DEBUG("[%s:%d] uuid[%lu], partner:%p, data:%p", __FUNCTION__, __LINE__, ret->data->uuid, ret, ret->data);
	
	return ret;
}

partner_struct *partner_manager::get_partner_by_uuid(uint64_t uuid)
{
	std::map<uint64_t, partner_struct *>::iterator iter = partner_manager_all_partner_id.find(uuid);
	if (iter != partner_manager_all_partner_id.end())
	{
		return iter->second;
	}
	return NULL;
}

int partner_manager::add_partner(partner_struct *p)
{
	partner_manager_all_partner_id[p->data->uuid] = p;
	return 0;
}

int partner_manager::remove_partner(partner_struct *p)
{
	partner_manager_all_partner_id.erase(p->data->uuid);
	return 0;
}

partner_struct *partner_manager::alloc_partner()
{
	partner_struct *ret = NULL;
	partner_data *data = NULL;
	if (partner_manager_partner_free_list.empty())
		return NULL;
	ret = partner_manager_partner_free_list.back();
	partner_manager_partner_free_list.pop_back();
	data = (partner_data *)mass_pool_alloc(&partner_manager_partner_data_pool);
	if (!data)
		goto fail;
	memset(data, 0, sizeof(partner_data));
	ret->data = data;
	partner_manager_partner_used_list.insert(ret);

	return ret;
fail:
	if (ret) {
		partner_manager_partner_used_list.erase(ret);
		partner_manager_partner_free_list.push_back(ret);
	}
	if (data) {
		mass_pool_free(&partner_manager_partner_data_pool, data);
	}
	return NULL;
}

int partner_manager::reset_all_partner_ai()
{
	{
		std::map<uint64_t, partner_struct *>::iterator it = partner_manager_all_partner_id.begin();
		for (; it != partner_manager_all_partner_id.end(); ++it)
		{
			it->second->set_ai_interface(it->second->ai_type);
		}
	}
	return (0);
}
