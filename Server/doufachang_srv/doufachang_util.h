#ifndef DOUFACHANG_UTIL_H
#define DOUFACHANG_UTIL_H
#include "player_redis_info.pb-c.h"
#include "doufachang.pb-c.h"
#include "redis_client.h"

//默认挑战次数是10次
#define DEFAULT_CHALLENGE_COUNT 10
//4个小时增加一次挑战次数
#define ADD_COUNT_SECONDS (4 * 3600)
//斗法场最大排名
#define DOUFACHANG_MAX_RANK 7000

class AutoReleaseDoufachangInfo
{
public:
	AutoReleaseDoufachangInfo();
	~AutoReleaseDoufachangInfo();
	void set(PlayerDoufachangInfo* r);
private:
	PlayerDoufachangInfo *pointer;
};

class AutoReleaseDoufachangRecord
{
public:
	AutoReleaseDoufachangRecord();
	~AutoReleaseDoufachangRecord();
	void set(DoufachangRecordAnswer* r);
private:
	DoufachangRecordAnswer *pointer;
};

extern uint64_t sg_next_copy_rank_time;

void reinit_default_doufachang_info(PlayerDoufachangInfo *info);

DoufachangRecordAnswer *get_player_doufachang_record(uint64_t player_id, char *player_key, CRedisClient &rc, AutoReleaseDoufachangRecord &_pool);
int save_player_doufachang_record(DoufachangRecordAnswer *info, uint64_t player_id, char *player_key, CRedisClient &rc);

PlayerDoufachangInfo *get_player_doufachang_info(uint64_t player_id, char *player_key, CRedisClient &rc, AutoReleaseDoufachangInfo &_pool);
int save_player_doufachang_info(PlayerDoufachangInfo *info, uint64_t player_id, char *player_key, CRedisClient &rc);

//void init_doufachang_player_info(uint64_t player_id, PlayerDoufachangInfo *info, uint32_t now);
bool update_doufachang_player_info(uint64_t player_id, PlayerDoufachangInfo *info, uint32_t now);
void copy_doufachang_rank(char *from, char *to, CRedisClient &rc);

#endif /* DOUFACHANG_UTIL_H */
