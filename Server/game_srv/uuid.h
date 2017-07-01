#ifndef UUID_H
#define UUID_H
#include <stdint.h>
#include "time_helper.h"

uint64_t alloc_raid_uuid();   //分配副本的唯一iD
uint64_t alloc_monster_uuid(); //分配怪物的唯一iD
uint64_t alloc_trap_uuid(); //分配陷阱的唯一iD
uint64_t alloc_ai_player_uuid(); //分配机器人唯一ID
uint64_t alloc_partner_uuid(); //分配伙伴唯一ID
uint64_t alloc_truck_uuid();   //镖车唯一ID
	
//判断对象是玩家还是怪物
enum entity_type
{
	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_MONSTER = 1,
	ENTITY_TYPE_TRAP,
	ENTITY_TYPE_AI_PLAYER, //机器人，当成玩家处理	
	ENTITY_TYPE_PARTNER,  //伙伴
	ENTITY_TYPE_TRUCK,   //镖车
};
entity_type get_entity_type(uint64_t uuid);  
#endif /* UUID_H */
