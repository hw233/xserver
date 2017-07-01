#include <assert.h>
#include "uuid.h"
#include "game_event.h"

extern uint64_t  sg_server_id;

// uint64_t alloc_equip_uuid()
// {
// 	/*
// 		高   -》 低
// 		10             22             32
// 		serverid       递增的ID       时间戳
// 	*/
// 	static uint64_t last_uuid = 0;
// 	uint64_t t = time_helper::get_cached_time() & 0xffffffff;
// 	if ((last_uuid & 0xffffffff) == t) {
// 		uint64_t high = last_uuid >> 32;
// 		++high;
// 		last_uuid = (high << 32) | t;
// 		goto done;
// 	}
// 	last_uuid = t | (sg_server_id << 54);
// done:
// 	LOG_DEBUG("%s %d: uuid[%lx]", __FUNCTION__, __LINE__, last_uuid);
// 	return (last_uuid);
// }
uint64_t alloc_raid_uuid()
{
	static uint64_t last_uuid;
	uint32_t t = time_helper::get_cached_time() / 1000 & 0xffffffff;
	if ((last_uuid & 0xffffffff) == t) {
		uint64_t high = last_uuid >> 32;
		++high;
		last_uuid = high << 32 | t;
		goto done;
	}
	last_uuid = t;
done:
	LOG_DEBUG("%s %d: uuid[%lu]", __FUNCTION__, __LINE__, last_uuid);	
	return (last_uuid);
}

union uuid_data
{
	uint64_t data1;
	struct
	{
		uint32_t time : 32;		
		uint32_t auto_inc : 18;
		uint32_t type : 4;
	} data2;
};

// TODO: 伙伴的UUID没有服务器信息，合服的时候会冲突
//double可以保存53位  1FFFFFFFFFFFFF
//50-53, 4个bit位用来表示类型，共支持16种
//32-49，18个bit位表示服务器ID(player)或者自增, 共262144个
//0-31,  32个bit位自定义，一般用来自增(player)或者表示时间
// #define MONSTER_TYPE_BIT       0x4000000000000
// #define TRAP_TYPE_BIT          0x8000000000000
// #define AI_PLAYER_TYPE_BIT    0x10000000000000
#define MONSTER_TYPE_UUID 1
#define TRAP_TYPE_UUID 2
#define AI_PLAYER_TYPE_UUID 3
#define PARTNER_TYPE_UUID 4
#define TRUCK_TYPE_UUID 5 //镖车

uint64_t alloc_ai_player_uuid()
{
//	static uint64_t last_uuid;
	uint64_t t = time_helper::get_cached_time() / 1000 & 0xffffffff;

//	{
		static union uuid_data last_ret_data;
		union uuid_data ret_data;

		if (last_ret_data.data2.time == t)
		{
			last_ret_data.data2.type = AI_PLAYER_TYPE_UUID;			
			last_ret_data.data2.auto_inc++;
			ret_data.data1 = last_ret_data.data1;
		}
		else
		{
			ret_data.data1 = 0;
			ret_data.data2.time = t;
			ret_data.data2.type = AI_PLAYER_TYPE_UUID;			
			last_ret_data.data1 = ret_data.data1;
		}
//	}
	
	// if ((last_uuid & 0xffffffff) == t) {
	// 	uint64_t high = last_uuid >> 32;
	// 	++high;
	// 	last_uuid = high << 32 | t;
	// 	last_uuid |= AI_PLAYER_TYPE_BIT;
	// 	goto done;
	// }
	// last_uuid = t | AI_PLAYER_TYPE_BIT;
//done:
	LOG_DEBUG("%s %d: uuid[%lu]", __FUNCTION__, __LINE__, ret_data.data1);
//	assert(last_uuid == ret_data.data1);
//	return (last_uuid);
	return ret_data.data1;
}

uint64_t alloc_monster_uuid()
{
//	static uint64_t last_uuid;
	uint64_t t = time_helper::get_cached_time() / 1000 & 0xffffffff;

//	{
		static union uuid_data last_ret_data;
		union uuid_data ret_data;

		if (last_ret_data.data2.time == t)
		{
			last_ret_data.data2.type = MONSTER_TYPE_UUID;
			last_ret_data.data2.auto_inc++;
			ret_data.data1 = last_ret_data.data1;
		}
		else
		{
			ret_data.data1 = 0;
			ret_data.data2.time = t;
			ret_data.data2.type = MONSTER_TYPE_UUID;
			last_ret_data.data1 = ret_data.data1;
		}
//	}
	
// 	if ((last_uuid & 0xffffffff) == t) {
// 		uint64_t high = last_uuid >> 32;
// 		++high;
// 		last_uuid = high << 32 | t;
// 		last_uuid |= MONSTER_TYPE_BIT;
// 		goto done;
// 	}
// 	last_uuid = t | MONSTER_TYPE_BIT;
// done:
	LOG_DEBUG("%s %d: uuid[%lu]", __FUNCTION__, __LINE__, ret_data.data1);
//	assert(last_uuid == ret_data.data1);	
//	return (last_uuid);
	return ret_data.data1;
}

uint64_t alloc_trap_uuid()
{
//	static uint64_t last_uuid;
	uint64_t t = time_helper::get_cached_time() / 1000 & 0xffffffff;

//	{
		static union uuid_data last_ret_data;
		union uuid_data ret_data;

		if (last_ret_data.data2.time == t)
		{
			last_ret_data.data2.type = TRAP_TYPE_UUID;
			last_ret_data.data2.auto_inc++;
			ret_data.data1 = last_ret_data.data1;
		}
		else
		{
			ret_data.data1 = 0;
			ret_data.data2.time = t;
			ret_data.data2.type = TRAP_TYPE_UUID;
			last_ret_data.data1 = ret_data.data1;
		}
//	}
	
// 	if ((last_uuid & 0xffffffff) == t) {
// 		uint64_t high = last_uuid >> 32;
// 		++high;
// 		last_uuid = high << 32 | t;
// 		last_uuid |= TRAP_TYPE_BIT;		
// 		goto done;
// 	}
// 	last_uuid = t | TRAP_TYPE_BIT;
// done:
	LOG_DEBUG("%s %d: uuid[%lu]", __FUNCTION__, __LINE__, ret_data.data1);
//	assert(last_uuid == ret_data.data1);	
//	return (last_uuid);
	return ret_data.data1;
}

uint64_t alloc_partner_uuid()
{
	uint64_t t = time_helper::get_cached_time() / 1000 & 0xffffffff;

	static union uuid_data last_ret_data;
	union uuid_data ret_data;

	if (last_ret_data.data2.time == t)
	{
		last_ret_data.data2.type = PARTNER_TYPE_UUID;
		last_ret_data.data2.auto_inc++;
		ret_data.data1 = last_ret_data.data1;
	}
	else
	{
		ret_data.data1 = 0;
		ret_data.data2.time = t;
		ret_data.data2.type = PARTNER_TYPE_UUID;
		last_ret_data.data1 = ret_data.data1;
	}
	LOG_DEBUG("%s %d: uuid[%lu]", __FUNCTION__, __LINE__, ret_data.data1);
	return ret_data.data1;
}

uint64_t alloc_truck_uuid()
{
	uint64_t t = time_helper::get_cached_time() / 1000 & 0xffffffff;

	static union uuid_data last_ret_data;
	union uuid_data ret_data;

	if (last_ret_data.data2.time == t)
	{
		last_ret_data.data2.type = TRUCK_TYPE_UUID;
		last_ret_data.data2.auto_inc++;
		ret_data.data1 = last_ret_data.data1;
	}
	else
	{
		ret_data.data1 = 0;
		ret_data.data2.time = t;
		ret_data.data2.type = TRUCK_TYPE_UUID;
		last_ret_data.data1 = ret_data.data1;
	}
	LOG_DEBUG("%s %d: uuid[%lu]", __FUNCTION__, __LINE__, ret_data.data1);
	return ret_data.data1;
}

// uint64_t alloc_skill_uuid()
// {
// 	static uint64_t last_uuid;
// 	uint32_t t = time_helper::get_cached_time() & 0xffffffff;
// 	if ((last_uuid & 0xffffffff) == t) {
// 		uint64_t high = last_uuid >> 32;
// 		++high;
// 		last_uuid = high << 32 | t;
// 		goto done;
// 	}
// 	last_uuid = t;
// done:
// 	LOG_DEBUG("%s %d: uuid[%lx]", __FUNCTION__, __LINE__, last_uuid);	
// 	return (last_uuid);
// }

entity_type get_entity_type(uint64_t uuid)
{
	switch (((union uuid_data *)&uuid)->data2.type)
	{
		case AI_PLAYER_TYPE_UUID:
			return ENTITY_TYPE_AI_PLAYER;
		case MONSTER_TYPE_UUID:
			return ENTITY_TYPE_MONSTER;
		case TRAP_TYPE_UUID:
			return ENTITY_TYPE_TRAP;
		case PARTNER_TYPE_UUID:
			return ENTITY_TYPE_PARTNER;
		case TRUCK_TYPE_UUID:
			return ENTITY_TYPE_TRUCK;
		default:
			return ENTITY_TYPE_PLAYER;
	}
		
	// if (uuid & MONSTER_TYPE_BIT)
	// 	return ENTITY_TYPE_MONSTER;
	// else if (uuid & TRAP_TYPE_BIT)
	// 	return ENTITY_TYPE_TRAP;
	// else if (uuid & AI_PLAYER_TYPE_BIT)
	// 	return ENTITY_TYPE_AI_PLAYER;
	// else
	// 	return ENTITY_TYPE_PLAYER;
}
