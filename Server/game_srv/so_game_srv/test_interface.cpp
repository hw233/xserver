#include <stdint.h>
#include "test_timer.h"
#include "game_event.h"
#include "player_manager.h"
#include "monster_manager.h"
#include "scene_manager.h"
#include "msgid.h"
#include "../proto/move.pb-c.h"
#include "../proto/move_direct.pb-c.h"
#include "app_data_statis.h"
extern int handle_move_request_impl(player_struct *player, EXTERN_DATA *extern_data, MoveRequest *req);
extern int handle_move_start_request_impl(player_struct *player, EXTERN_DATA *extern_data, MoveStartRequest *req);

extern "C"
{

void test1()
{
	LOG_DEBUG("%s", __FUNCTION__);
}

void test2(int a)
{
	LOG_INFO("%s %d", __FUNCTION__, a);
}

void test3(char *a)
{
	LOG_ERR("%s: %s", __FUNCTION__, a);
}

// void log(char *a)
// {
// 	LOG_ERR("%s: %s", __FUNCTION__, a);
// }

	int register_timer10_func(timer_func func)
	{
		all_test_timer10_funcs[cur_test_timer10_funcs++] = func;
		func();
		return (0);
	}
	int register_timer100_func(timer_func func)
	{
		all_test_timer100_funcs[cur_test_timer100_funcs++] = func;
		func();
		return (0);
	}	
	int unregister_timer10_func(timer_func func)
	{
		for (int i = 0; i < cur_test_timer10_funcs; ++i)
		{
			if (all_test_timer10_funcs[i] == func)
			{
				all_test_timer10_funcs[i] = all_test_timer10_funcs[cur_test_timer10_funcs--];
				return (0);
			}
		}
		return (0);
	}
	int unregister_timer100_func(timer_func func)
	{
		for (int i = 0; i < cur_test_timer100_funcs; ++i)
		{
			if (all_test_timer100_funcs[i] == func)
			{
				all_test_timer100_funcs[i] = all_test_timer100_funcs[cur_test_timer100_funcs--];
				return (0);
			}
		}
		return (0);
	}
/*
	int test_create_monster(uint64_t monster_id, uint64_t scene_id, double pos_x, double pos_z)
	{
		LOG_DEBUG("%s %lu %lu %.1f %.1f", __FUNCTION__, monster_id, scene_id, pos_x, pos_z);
		scene_struct *scene = scene_manager::get_scene(scene_id);
		if (scene)
			monster_manager::create_monster_at_position(monster_id, scene, pos_x, pos_x);
		return (0);
	}
*/	
	int test_create_tmp_player(uint64_t player_id)
	{
		LOG_DEBUG("%s %lu", __FUNCTION__, player_id);
		player_struct *player;
		player = player_manager::get_player_by_id(player_id);
		if (!player) {
			player = player_manager::create_tmp_player(player_id);
		}
		return (0);
	}

	int get_player(uint64_t player_id, void **ret)
	{
		*ret = player_manager::get_player_by_id(player_id);
		LOG_DEBUG("%s %lu: %p, ret = %p", __FUNCTION__, player_id, *ret, ret);
		return (0);
	}

	int get_player_id_and_name(void *player, char **name, uint64_t *id)
	{
		player_struct *p = (player_struct *)player;
		LOG_DEBUG("%s %lu: %s", __FUNCTION__, p->data->player_id, p->data->name);
		*name = p->data->name;
		*id = p->data->player_id;
		return (0);
	}

	int test_move_request_v2(uint64_t player_id, uint8_t *data, int len)
	{
		LOG_DEBUG("%s %lu", __FUNCTION__, player_id);
		player_struct *player;
		player = player_manager::get_player_by_id(player_id);
		if (!player) {
			player = player_manager::create_tmp_player(player_id);
		}
		EXTERN_DATA ext;
		ext.player_id = player_id;
		MoveRequest *req;
		req = move_request__unpack(NULL, len, data);
		if (!req) {
			LOG_ERR("unpack failed");
			return (0);
		}
		for (size_t i = 0; i < req->n_data; ++i)
		{
			LOG_DEBUG("%.1f %.1f", req->data[i]->pos_x, req->data[i]->pos_z);
		}
		
		handle_move_request_impl(player, &ext, req);		
		move_request__free_unpacked(req, NULL);
		return (0);
	}
	

	int test_move_request(uint64_t player_id, uint32_t n, double *pos_x, double *pos_z)
	{
		LOG_DEBUG("%s %lu", __FUNCTION__, player_id);
		for (size_t i = 0; i < n; ++i)
			LOG_DEBUG("%.2f %.2f", pos_x[i], pos_z[i]);
		player_struct *player;
		player = player_manager::get_player_by_id(player_id);
		if (!player) {
			player = player_manager::create_tmp_player(player_id);
		}
		
		EXTERN_DATA ext;
		ext.player_id = player_id;
		MoveRequest req;
		PosData data[n];
		PosData *data_p[n];
		req.n_data = n;
		for (size_t i = 0; i < n; ++i)
		{
			data_p[i] = &data[i];
			pos_data__init(data_p[i]);
			data[i].pos_x = pos_x[i];
			data[i].pos_z = pos_z[i];			
		}
		req.data = data_p;
		
//		conn_node_gamesrv::connecter.m_handleMap[MSG_ID_MOVE_REQUEST](player, &ext);
		handle_move_request_impl(player, &ext, &req);
		return (0);
	}

	int kick_player(uint64_t player_id)
	{
		LOG_DEBUG("%s %lu", __FUNCTION__, player_id);
		player_struct *player;
		player = player_manager::get_player_by_id(player_id);
		if (!player)
			return (0);
		return (0);
	}

	int test_move_start_request(uint64_t player_id, uint8_t *data, int len)
	{
		LOG_DEBUG("%s %lu", __FUNCTION__, player_id);
		player_struct *player;
		player = player_manager::get_player_by_id(player_id);
		if (!player) {
			player = player_manager::create_tmp_player(player_id);
		}
		EXTERN_DATA ext;
		ext.player_id = player_id;
		MoveStartRequest *req;
		req = move_start_request__unpack(NULL, len, data);
		if (!req) {
			LOG_ERR("unpack failed");
			return (0);
		}
		
		handle_move_start_request_impl(player, &ext, req);		
		move_start_request__free_unpacked(req, NULL);
		return (0);
	}

	int set_attr(uint64_t player_id, uint32_t id, uint32_t value)
	{
		LOG_DEBUG("[%s:%d] player[%lu], id:%u, value:%u", __FUNCTION__, __LINE__, player_id, id, value);
		player_struct *player = player_manager::get_player_by_id(player_id);
		if (!player || !player->data)
		{
			return -1;
		}
		player->data->attrData[id] = value;
		return (0);
	}

	int add_item(uint64_t player_id, uint32_t id, uint32_t num)
	{
		LOG_DEBUG("[%s:%d] player[%lu], id:%u, num:%u", __FUNCTION__, __LINE__, player_id, id, num);
		player_struct *player = player_manager::get_player_by_id(player_id);
		if (!player)
		{
			return -1;
		}

		player->add_item(id, num, MAGIC_TYPE_GM);
		return 0;
	}

	int del_item(uint64_t player_id, uint32_t id, uint32_t num)
	{
		LOG_DEBUG("[%s:%d] player[%lu], id:%u, num:%u", __FUNCTION__, __LINE__, player_id, id, num);
		player_struct *player = player_manager::get_player_by_id(player_id);
		if (!player)
		{
			return -1;
		}

		player->del_item_by_id(id, num, MAGIC_TYPE_GM);
		return 0;
	}
	
	
};
