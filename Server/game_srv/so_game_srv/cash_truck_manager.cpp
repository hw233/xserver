#include "cash_truck_manager.h"
#include "uuid.h"
#include "app_data_statis.h"

int cash_truck_manager::init_cash_truck_struct(int num, unsigned long key)
{
	cash_truck_struct *truck;
	for (int i = 0; i < num; ++i) {
		truck = new cash_truck_struct();
		cash_truck_manager_free_list.push_back(truck);
	}
	return init_comm_pool(0, sizeof(cash_truck_data), num, key, &cash_truck_manager_data_pool);
} 

cash_truck_struct *cash_truck_manager::create_cash_truck_at_pos(scene_struct *scene, uint64_t id, player_struct &player)
{
	return create_cash_truck_at_pos(scene, id, player, player.get_pos()->pos_x, player.get_pos()->pos_z, 0);
}

cash_truck_struct *cash_truck_manager::create_cash_truck_at_pos(scene_struct *scene, uint64_t id, player_struct &player, float x, float z, uint32_t hp)
{
	
	BiaocheTable *table = get_config_by_id(id, &cash_truck_config);
	if (table == NULL)
	{
		return NULL;
	}
	std::map<uint64_t, struct MonsterTable *>::iterator ite;
	ite = monster_config.find(table->MonsterId);
	if (ite == monster_config.end())
		return NULL;
	cash_truck_struct *ret = alloc_cash_truck();
	if (!ret)
		return NULL;
	ret->data->player_id = alloc_truck_uuid();
	cash_truck_manager_all_id[ret->data->player_id] = ret;
	ret->set_pos(x, z);
	ret->data->monster_id = table->MonsterId;
	ret->data->pos_y = player.data->pos_y;
	ret->data->owner = player.get_uuid();
	ret->truck_config = table;
	ret->config = ite->second;
	ret->data->fb_time = time_helper::get_cached_time() + ret->truck_config->Interval * 1000;


	//下面可以初始化镖车属性
	ret->set_attr(PLAYER_ATTR_LEVEL, player.get_attr(PLAYER_ATTR_LEVEL));
	

	ret->calculate_attribute();
	if (ret->get_truck_type() == 2) //财宝镖车
	{
		ret->set_attr(PLAYER_ATTR_ZHENYING, player.get_attr(PLAYER_ATTR_ZHENYING));
		ret->set_attr(PLAYER_ATTR_PK_TYPE, 1);
	}
	else
	{
		// 阵营模式, PK模式
		ret->set_attr(PLAYER_ATTR_PK_TYPE, ret->config->PkType);
		ret->set_attr(PLAYER_ATTR_ZHENYING, ret->config->Camp);
	}
	if (hp != 0)
	{
		ret->set_attr(PLAYER_ATTR_HP, hp);
	}

	// 进场景
	scene->add_cash_truck_to_scene(ret);
	return ret;
}

cash_truck_struct *cash_truck_manager::alloc_cash_truck()
{
	cash_truck_struct *ret = NULL;
	cash_truck_data *data = NULL;
	if (cash_truck_manager_free_list.empty())
		return NULL;
	ret = cash_truck_manager_free_list.back();
	cash_truck_manager_free_list.pop_back();
	data = (cash_truck_data *)comm_pool_alloc(&cash_truck_manager_data_pool);
	if (!data)
		goto fail;
	memset(data, 0, sizeof(cash_truck_data));
	ret->data = data;
	cash_truck_manager_used_list.insert(ret);
	ret->init_cash_truck();
	return ret;
fail:
	if (ret) {
		cash_truck_manager_used_list.erase(ret);
		cash_truck_manager_free_list.push_back(ret);
	}
	if (data) {
		comm_pool_free(&cash_truck_manager_data_pool, data);
	}
	return NULL;
}

void cash_truck_manager::delete_cash_truck(cash_truck_struct *p)
{
	LOG_DEBUG("%s %d: monster[%p] data[%p]", __FUNCTION__, __LINE__, p, p->data);

	//if (p->data && p->data->owner)
	//{
	//	player_struct *owner = player_manager::get_player_by_id(p->data->owner);
	//	if (owner)
	//	{
	//		owner->del_pet(p);
	//	}
	//}

	cash_truck_manager_used_list.erase(p);
	cash_truck_manager_free_list.push_back(p);

	p->clear_all_buffs();

	if (p->data) {
		LOG_INFO("[%s:%d] monster_id[%u], uuid[%lu]", __FUNCTION__, __LINE__, p->data->monster_id, p->data->player_id);
		cash_truck_manager_all_id.erase(p->data->player_id);
		comm_pool_free(&cash_truck_manager_data_pool, p->data);
		p->data = NULL;
	}
	else {
		LOG_ERR("[%s:%d] monster[%p]", __FUNCTION__, __LINE__, p);
	}
}

cash_truck_struct * cash_truck_manager::get_cash_truck_by_id(uint64_t id)
{
	std::map<uint64_t, cash_truck_struct *>::iterator it = cash_truck_manager_all_id.find(id);
	if (it != cash_truck_manager_all_id.end())
		return it->second;

	return NULL;
}

void cash_truck_manager::cash_truck_drop(player_struct &player)
{
	BiaocheTable *table = get_config_by_id(player.data->truck.active_id, &cash_truck_config);
	if (table == NULL)
	{
		return;
	}
	BiaocheRewardTable *reward_config = get_config_by_id(table->Reward, &cash_truck_reward_config);
	if (reward_config == NULL)
	{
		return;
	}

	player.add_coin(reward_config->RewardMoney1 * player.get_attr(PLAYER_ATTR_LEVEL) * (reward_config->Deposit + 10000) / 10000.0, MAGIC_TYPE_CASH_TRUCK);
	player.add_exp(reward_config->RewardExp1 * player.get_attr(PLAYER_ATTR_LEVEL), MAGIC_TYPE_CASH_TRUCK);
	player.add_bind_gold(reward_config->RewardLv1 * player.get_attr(PLAYER_ATTR_LEVEL), MAGIC_TYPE_CASH_TRUCK);
	for (uint32_t i = 0; i < reward_config->n_RewardItem1; ++i)
	{
		player.add_item(reward_config->RewardItem1[i], reward_config->RewardNum1[i], MAGIC_TYPE_CASH_TRUCK);
	}
	player.data->truck.active_id = 0;
	player.data->truck.truck_id = 0;
}
