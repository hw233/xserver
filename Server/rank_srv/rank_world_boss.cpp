#include<time.h>
#include<sys/time.h>
#include "rank_config.h"
#include "rank_world_boss.h"

//#define MAX_BOSS_NUM 100

//uint64_t boss_id[MAX_BOSS_NUM] = {0};

char cur_world_boss_key[64]; //当前世界boss数据
char befor_world_boss_key[64]; //上轮世界boss数据
char tou_mu_world_boss_reward_num[64]; //头目类型世界boss玩家领奖信息
char shou_ling_world_boss_reward_num[64]; //首领类型世界boss玩家领奖信息
std::map<uint64_t, std::string> world_boss_rank_keys;
std::set<uint64_t> world_boss_id;
extern CRedisClient sg_redis_client;
uint32_t tou_mu_parame_id = 161000327; //头领限制领奖次数在参数表里面的id
uint32_t shou_ling_parame_id = 161000328; //首领限制领奖次数在参数表里面的id

//获取当前时间，单位、秒
uint64_t rank_srv_get_now_time()
{
	timeval t;
	gettimeofday(&t,NULL);
	return ((t.tv_sec * 1000000 + t.tv_usec) /1000000);
}


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

void init_world_boss_rank_key_map()
{
	
	for(std::set<uint64_t>::iterator itr = world_boss_id.begin(); itr != world_boss_id.end(); itr++)
	{
		world_boss_rank_keys[*itr] = "s%u_rank_world_boss_%lu";
		world_boss_rank_keys[*itr*10] = "s%u_rank_world_boss_%lu";
	}
}
char *get_world_boss_rank_key(uint64_t rank_type)
{
	std::map<uint64_t, std::string>::iterator iter = world_boss_rank_keys.find(rank_type);
	if (iter != world_boss_rank_keys.end())
	{
		return const_cast<char*>(iter->second.c_str());
	}
	return NULL;
}

//服务器重启是清除当前世界boss的redis信息
int init_cur_world_boss_info()
{

	int ret = 0;
	char field[128];
	uint64_t boss_id = 0;
	uint64_t monster_id =0;
	CRedisClient &rc = sg_redis_client;
	for(std::map<uint64_t, struct WorldBossTable*>::iterator itr = rank_world_boss_config.begin(); itr != rank_world_boss_config.end(); itr++)
	{
		boss_id = itr->second->ID;
		monster_id = itr->second->MonsterID;
		std::map<uint64_t, struct MonsterTable *>::iterator monster = monster_config.find(monster_id);
		if(monster == monster_config.end())
			continue;	
		uint64_t attr_table_id = monster->second->BaseAttribute*1000+itr->second->Level;
		std::map<uint64_t, struct ActorAttributeTable *>::iterator attr = actor_attribute_config.find(attr_table_id);
		if(attr == actor_attribute_config.end())
			continue;


		//重置当前轮信息，主要是设置当前血量信息
		CurWorldBossRedisinfo cur_world_boss_info;
		cur_world_boss_redisinfo__init(&cur_world_boss_info);
		cur_world_boss_info.boss_id = boss_id;
		cur_world_boss_info.max_hp = attr->second->Health;
		cur_world_boss_info.cur_hp = attr->second->Health;
		static uint8_t data_buffer[128 * 1024];
		sprintf(field, "%lu", boss_id);
		do
		{
			size_t data_len = cur_world_boss_redisinfo__pack(&cur_world_boss_info, data_buffer);
			if (data_len == (size_t)-1)
			{
				LOG_ERR("[%s:%d] pack redis world_boss_info failed, bossid[%lu]", __FUNCTION__, __LINE__, boss_id);
				break;
			}

			ret = rc.hset_bin(cur_world_boss_key, field, (const char *)data_buffer, (int)data_len);
			if (ret < 0)
			{
				LOG_ERR("[%s:%d] set cur world boss failed, bossid[%lu] ret = %d", __FUNCTION__, __LINE__, boss_id, ret);
				break;
			}
		} while(0);


		//删除排行榜信息
		char *rank_key = NULL;
		rank_key = get_world_boss_rank_key(boss_id);
		if(rank_key == NULL)
		{
			LOG_ERR("[%s:%d]务器重启是清除当前世界boss的redis信息出错,bossid[%lu]", __FUNCTION__, __LINE__,boss_id);
			continue;
		}
		ret = rc.zdel_rank(rank_key, 0, -1);
		if(ret != 0)
		{
			LOG_ERR("[%s:%d]务器重启是，清除本轮榜单信息失败[%lu]", __FUNCTION__, __LINE__, boss_id)	
		}

	}
	return 0;
}

//判断前后两个时间是否跨过某个时间点
//param1 前一个时间点 1970 年 1 月 1 日（00:00:00 GMT）以来的秒数
//param2 后一个时间点 1970 年 1 月 1 日（00:00:00 GMT）以来的秒数
//param3 - param4中间的是否跨过的时间点 年/月/日/时/分/秒 小于0表示用当前时间
//return 1:跨越了 0:没跨越 <0:出错
int judge_the_time_span(uint64_t befor_time, uint64_t behind_time, int year ,int month, int day, int hour, int min, int second)
{
//sdsd
	
	if(befor_time > behind_time)
	{
		LOG_ERR("[%s:%d] 判断时间跨度有误，前一时间戳大于后面的时间戳,befor_time:%lu, behind_time%lu", __FUNCTION__, __LINE__, befor_time, behind_time);
		return -1;
	}
	if(month > 12 || month == 0)
	{
		LOG_ERR("[%s:%d] 判断时间跨度有误，月份有误,month:%d", __FUNCTION__, __LINE__, month);
		return -2;
	}
	if(day > 31 || day == 0)
	{
		LOG_ERR("[%s:%d] 判断时间跨度有误，日期有误,day:%d", __FUNCTION__, __LINE__, day);
		return -3;
	}
	if( month == 4 || month == 6 || month == 9 || month == 11)
	{
		if( day > 30)
		{			
			LOG_ERR("[%s:%d] 判断时间跨度有误，日期有误,day:%d", __FUNCTION__, __LINE__, day);
			return -4;
		}
	}
	if(month == 2)
	{
		if(day > 28)
		{
			LOG_ERR("[%s:%d] 判断时间跨度有误，日期有误,day:%d", __FUNCTION__, __LINE__, day);
			return -5;
		}
	}
	if(hour > 23)
	{
		LOG_ERR("[%s:%d] 判断时间跨度有误，小时有误,hour:%d", __FUNCTION__, __LINE__, hour);
		return -6;
	}
	if(min > 59)
	{
		LOG_ERR("[%s:%d] 判断时间跨度有误，分钟有误,min:%d", __FUNCTION__, __LINE__, min);
		return -7;
	}
	if(second > 59)
	{
		LOG_ERR("[%s:%d] 判断时间跨度有误，日期有误,second:%d", __FUNCTION__, __LINE__, second);
		return -8;
	}
	struct tm tm;
	time_t now_time = rank_srv_get_now_time();
	localtime_r(&now_time, &tm);
	if(year > 0)
	{
		tm.tm_year = year - 1900;
		if(tm.tm_year < 0)
		{
			LOG_ERR("[%s:%d]判断时间跨度有误，年份有误 year:%d", __FUNCTION__, __LINE__, year);
			return -3;
		}
	}

	if(month >0)
	{
		tm.tm_mon = month -1 ;
	}

	if(day > 0)
	{
		tm.tm_mday = day;
	}
	if(hour >= 0)
	{
		tm.tm_hour = hour;
	}
	if(min >= 0)
	{
		tm.tm_min = min;
	}
	if(second >= 0)
	{
		tm.tm_sec = second;
	}
	uint64_t my_time = mktime(&tm);

	if(my_time > befor_time && my_time < behind_time)
	{
		return 1;
	}

	return 0;

}


