#include "rank_config.h"
#include "rank_world_boss.h"

//#define MAX_BOSS_NUM 100

//uint64_t boss_id[MAX_BOSS_NUM] = {0};

std::set<uint64_t> world_boss_id;
int init_rank_world_boss_id()
{
	if(rank_world_boss_config.size() ==0)
		return -1;	

	for(std::map<uint64_t, struct WorldBossTable*>::iterator ite = rank_world_boss_config.begin(); ite != rank_world_boss_config.end() ; ite++)
	{
		world_boss_id.insert(ite->first);		
	}

	return 0;
}
