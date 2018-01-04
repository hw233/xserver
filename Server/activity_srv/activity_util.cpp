#include "activity_util.h"
#include "mysql_module.h"
#include "activity_db.pb-c.h"
#include <map>
#include <stdio.h>
#include "game_event.h"
#include "activity_config.h"
#include "time_helper.h"
#include "redis_util.h"
#include "activity.pb-c.h"
#include "conn_node_activitysrv.h"
#include "msgid.h"

static std::map<uint32_t, ActivityInfo *> activity_map;
static std::map<uint64_t, ActivityPlayer *> activity_player_map;

CRedisClient sg_redis_client;
uint32_t sg_server_id;
struct event sg_second_timer_event;
struct timeval sg_second_timer_val;	

char sg_player_key[64]; //玩家数据
char sg_rank_guild_fc_total_key[64];

static void give_shidamenzong_reward(ActivityInfo *info, uint32_t now);

void init_redis_keys(uint32_t server_id)
{
	sprintf(sg_player_key, "server_%u", server_id);
	sprintf(sg_rank_guild_fc_total_key, "s%u_rank_guild_fc_total", server_id);
}

void cb_second_timeout(evutil_socket_t, short, void* /*arg*/)
{
	uint32_t now = time_helper::get_micro_time() / 1000000;
	for (std::map<uint32_t, ActivityInfo *>::iterator iter = activity_map.begin(); iter != activity_map.end(); ++iter)
	{
		ActivityInfo *info = iter->second;
		uint32_t activity_type = get_time_limit_activity_type(info->activity_id);
		switch (info->state)
		{
			case TLAS_INIT: //检查是否到时间开启活动
				{
					if (info->begin_time <= now)
					{
						if (now >= info->end_time)
						{
							//活动结束
							info->state = TLAS_END;
						}
						else
						{
							begin_activity(info);
						}
					}
				}
				break;
			case TLAS_BEGIN: //
				{
					if (info->end_time <= now)
					{
						end_activity(info, now);
					}
				}
				break;
			case TLAS_END:
				{
					if (activity_type == ACTIVITY_SHIDAMENZONG)
					{
						if (info->data.shidamenzong.reward_time > 0 && now - info->data.shidamenzong.reward_time >= 300)
						{
							give_shidamenzong_reward(info, now);
						}
					}
				}
				break;
		}
	}

	add_timer(sg_second_timer_val, &sg_second_timer_event, NULL);
}

static int load_all_activity_info(void)
{
	char sql[1024];
	unsigned long *lengths;	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;

	sprintf(sql, "select `activity_id`, `state`, `comm_data` from activity;");

	res = query(sql, 1, NULL);
	if (!res)
	{
		LOG_ERR("[%s:%d] query failed, sql: %s", __FUNCTION__, __LINE__, sql);
		return -1;
	}

	while (true)
	{
		row = fetch_row(res);
		if (!row)
		{
			break;
		}

		lengths = mysql_fetch_lengths(res);
		uint32_t activity_id = strtoul(row[0], NULL, 10);
		uint32_t state = strtoul(row[1], NULL, 10);
		unsigned long &pack_size = lengths[2];
		uint8_t *pack_buffer = (uint8_t *)row[2];
		LimitActivityControlTable *config = get_config_by_id(activity_id, &time_limit_control_config);
		if (!config)
		{
			LOG_ERR("[%s:%d] get config failed, activity_id:%u", __FUNCTION__, __LINE__, activity_id);
			continue;
		}

		ActivityInfo *info = (ActivityInfo *)malloc(sizeof(ActivityInfo));
		if (!info)
		{
			LOG_ERR("[%s:%d] malloc activity failed", __FUNCTION__, __LINE__);
			continue;
		}

		memset(info, 0, sizeof(ActivityInfo));
		info->activity_id = activity_id;
		info->state = state;
		get_time_limit_activity_control(config->OpenType, config->ContinuedOpenTime, config->ContinuedTime, &info->begin_time, &info->end_time);

		switch (config->Activity)
		{
			case ACTIVITY_ZHANLIDAREN:
				{
					DBActivityZhanlidaren *dbinfo = dbactivity_zhanlidaren__unpack(NULL, pack_size, pack_buffer);
					if (!dbinfo)
					{
						LOG_ERR("[%s:%d] unpack zhanlidaren failed", __FUNCTION__, __LINE__);
						break;
					}
					AutoReleaseProtobufStruct t1(dbinfo, NULL, (AutoReleaseProtobufStruct::free_func)dbactivity_zhanlidaren__free_unpacked);

					for (size_t i = 0; i < dbinfo->n_gifts && i < MAX_ZHANLIDAREN_REWARD_NUM; ++i)
					{
						info->data.zhanlidaren.gift_get[i].id = dbinfo->gifts[i]->id;
						info->data.zhanlidaren.gift_get[i].num = dbinfo->gifts[i]->num;
					}
				}
				break;
			case ACTIVITY_SHIDAMENZONG:
				{
				}
				break;
		}

		activity_map.insert(std::make_pair(info->activity_id, info));
	}

	free_query(res);
	return 0;
}

static int load_all_activity_player(void)
{
	char sql[1024];
	unsigned long *lengths;	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;

	sprintf(sql, "select `player_id`, `comm_data` from activity_player;");

	res = query(sql, 1, NULL);
	if (!res)
	{
		LOG_ERR("[%s:%d] query failed, sql: %s", __FUNCTION__, __LINE__, sql);
		return -1;
	}

	while (true)
	{
		row = fetch_row(res);
		if (!row)
		{
			break;
		}

		lengths = mysql_fetch_lengths(res);
		uint64_t player_id = strtoull(row[0], NULL, 10);
		unsigned long &pack_size = lengths[1];
		uint8_t *pack_buffer = (uint8_t *)row[1];
		DBActivityPlayer *dbinfo = dbactivity_player__unpack(NULL, pack_size, pack_buffer);
		if (!dbinfo)
		{
			LOG_ERR("[%s:%d] unpack player failed, player_id:%lu", __FUNCTION__, __LINE__, player_id);
			continue;
		}
		AutoReleaseProtobufStruct t1(dbinfo, NULL, (AutoReleaseProtobufStruct::free_func)dbactivity_player__free_unpacked);

		ActivityPlayer *player = (ActivityPlayer*)malloc(sizeof(ActivityPlayer));
		if (!player)
		{
			LOG_ERR("[%s:%d] malloc player failed, player_id:%lu", __FUNCTION__, __LINE__, player_id);
			continue;
		}

		memset(player, 0, sizeof(ActivityPlayer));
		player->player_id = player_id;
		for (size_t i = 0; i < dbinfo->n_details && i < MAX_TIME_LIMIT_ACTIVITY_NUM; ++i)
		{
			player->details[i].activity_id = dbinfo->details[i]->activity_id;
			if (dbinfo->details[i]->zhanlidaren)
			{
				for (size_t j = 0; j < dbinfo->details[i]->zhanlidaren->n_reward && j < MAX_ZHANLIDAREN_REWARD_NUM; ++j)
				{
					player->details[i].matter.zhanlidaren.reward[j] = dbinfo->details[i]->zhanlidaren->reward[j];
				}
			}
		}

		activity_player_map.insert(std::make_pair(player_id, player));
	}

	free_query(res);
	return 0;
}

static void init_all_activity(void)
{
	for (std::map<uint64_t, LimitActivityControlTable *>::iterator iter = time_limit_control_config.begin(); iter != time_limit_control_config.end(); ++iter)
	{
		LimitActivityControlTable *config = iter->second;
		if (activity_map.find(config->ID) != activity_map.end())
		{
			continue;
		}

		ActivityInfo *activity = (ActivityInfo *)malloc(sizeof(ActivityInfo));
		if (!activity)
		{
			LOG_ERR("[%s:%d] malloc activity failed, activity_id:%lu", __FUNCTION__, __LINE__, config->ID);
			continue;
		}

		memset(activity, 0, sizeof(ActivityInfo));
		activity->activity_id = config->ID;
		activity->state = TLAS_INIT;
		get_time_limit_activity_control(config->OpenType, config->ContinuedOpenTime, config->ContinuedTime, &activity->begin_time, &activity->end_time);
		
		activity_map.insert(std::make_pair(activity->activity_id, activity));
	}
}

void load_activity_db(void)
{
	load_all_activity_info();
	load_all_activity_player();
	init_all_activity();
}

ActivityInfo *get_activity_info(uint32_t activity_id)
{
	std::map<uint32_t, ActivityInfo *>::iterator iter = activity_map.find(activity_id);
	if (iter != activity_map.end())
	{
		return iter->second;
	}
	return NULL;
}

ActivityPlayer *get_activity_player(uint64_t player_id)
{
	std::map<uint64_t, ActivityPlayer *>::iterator iter = activity_player_map.find(player_id);
	if (iter != activity_player_map.end())
	{
		return iter->second;
	}
	else
	{
		ActivityPlayer *player = (ActivityPlayer*)malloc(sizeof(ActivityPlayer));
		if (!player)
		{
			LOG_ERR("[%s:%d] malloc player failed, player_id:%lu", __FUNCTION__, __LINE__, player_id);
			return NULL;
		}

		memset(player, 0, sizeof(ActivityPlayer));
		player->player_id = player_id;
		activity_player_map[player_id] = player;
		return player;
	}

	return NULL;
}

int get_time_limit_activity_control(uint32_t type, uint32_t param1, uint32_t param2, uint32_t *begin_ts, uint32_t *end_ts)
{
	switch (type)
	{
		case 1: //相对时间
			{
				ServerResTable *config = get_config_by_id(sg_server_id, &server_res_config);
				if (!config)
				{
					break;
				}

				uint32_t begin_time = time_helper::get_day_timestamp(5, 0, 0, config->OpenTime) + (param1 - 1) * 24 * 3600;
				if (begin_ts)
				{
					*begin_ts = begin_time;
				}
				if (end_ts)
				{
					*end_ts = begin_time + param2 * 24 * 3600;
				}
			}
			break;
		case 2: //绝对时间
			{
				if (begin_ts)
				{
					*begin_ts = param1;
				}
				if (end_ts)
				{
					*end_ts = param2;
				}
			}
			break;
	}

	return 0;
}

int get_time_limit_activity_type(uint32_t activity_id)
{
	LimitActivityControlTable *config = get_config_by_id(activity_id, &time_limit_control_config);
	if (config)
	{
		return config->Activity;
	}
	return 0;
}

int pack_activity_info(ActivityInfo *info, uint8_t *out_data)
{
	switch (get_time_limit_activity_type(info->activity_id))
	{
		case ACTIVITY_SHIDAMENZONG:
			{
				DBActivityShidamenzong shidamenzong;
				dbactivity_shidamenzong__init(&shidamenzong);

				shidamenzong.reward_time = info->data.shidamenzong.reward_time;

				return dbactivity_shidamenzong__pack(&shidamenzong, out_data);
			}
			break;
		case ACTIVITY_ZHANLIDAREN:
			{
				DBActivityZhanlidaren zhanlidaren;
				dbactivity_zhanlidaren__init(&zhanlidaren);

				DBActivityZhanlidarenReward  reward_data[MAX_ZHANLIDAREN_REWARD_NUM];
				DBActivityZhanlidarenReward* reward_point[MAX_ZHANLIDAREN_REWARD_NUM];

				zhanlidaren.gifts = reward_point;
				zhanlidaren.n_gifts = 0;
				for (int i = 0; i < MAX_ZHANLIDAREN_REWARD_NUM; ++i)
				{
					if (info->data.zhanlidaren.gift_get[i].id == 0)
					{
						break;
					}

					reward_point[zhanlidaren.n_gifts] = &reward_data[zhanlidaren.n_gifts];
					dbactivity_zhanlidaren_reward__init(&reward_data[zhanlidaren.n_gifts]);
					reward_data[zhanlidaren.n_gifts].id = info->data.zhanlidaren.gift_get[i].id;
					reward_data[zhanlidaren.n_gifts].num = info->data.zhanlidaren.gift_get[i].num;
					zhanlidaren.n_gifts++;
				}

				return dbactivity_zhanlidaren__pack(&zhanlidaren, out_data);
			}
			break;
	}

	return 0;
}

int pack_activity_player(ActivityPlayer *player, uint8_t *out_data)
{
	DBActivityPlayer db_info;
	dbactivity_player__init(&db_info);

	DBActivityDetail  detail_data[MAX_TIME_LIMIT_ACTIVITY_NUM];
	DBActivityDetail* detail_point[MAX_TIME_LIMIT_ACTIVITY_NUM];
	DBPlayerZhanlidaren  zhanlidaren_data[MAX_TIME_LIMIT_ACTIVITY_NUM];

	db_info.details = detail_point;
	db_info.n_details = 0;
	for (int i = 0; i < MAX_TIME_LIMIT_ACTIVITY_NUM; ++i)
	{
		if (player->details[i].activity_id == 0)
		{
			break;
		}

		detail_point[db_info.n_details] = &detail_data[db_info.n_details];
		dbactivity_detail__init(&detail_data[db_info.n_details]);
		detail_data[db_info.n_details].activity_id = player->details[i].activity_id;
		switch (get_time_limit_activity_type(player->details[i].activity_id))
		{
			case ACTIVITY_ZHANLIDAREN:
				{
					detail_data[db_info.n_details].zhanlidaren = &zhanlidaren_data[db_info.n_details];
					dbplayer_zhanlidaren__init(&zhanlidaren_data[db_info.n_details]);
					zhanlidaren_data[db_info.n_details].reward = player->details[i].matter.zhanlidaren.reward;
					zhanlidaren_data[db_info.n_details].n_reward = 0;
					for (int j = 0; j < MAX_ZHANLIDAREN_REWARD_NUM; ++j)
					{
						if (player->details[i].matter.zhanlidaren.reward[j] == 0)
						{
							break;
						}
						zhanlidaren_data[db_info.n_details].n_reward++;
					}
				}
				break;
		}
		db_info.n_details++;
	}

	return dbactivity_player__pack(&db_info, out_data);
}

int insert_new_activity_to_db(ActivityInfo *info)
{
	static char save_sql[64 * 1024 + 300];
	static uint8_t save_data[64 * 1024 + 1];
	uint64_t effect = 0;
	char *p;

	size_t data_size = pack_activity_info(info, save_data);
	p = save_sql;
	p += sprintf(save_sql, "insert into activity set `activity_id` = %u, `begin_time` = FROM_UNIXTIME(%u), `end_time` = FROM_UNIXTIME(%u), `state` = %u, `comm_data` = \'", 
			info->activity_id, info->begin_time, info->end_time, info->state);
	p += escape_string(p, (const char *)save_data, data_size);
	p += sprintf(p, "\'");

	query(const_cast<char*>("set names utf8"), 1, NULL);
	query(save_sql, 1, &effect);	
	if (effect != 1) 
	{
		LOG_ERR("[%s:%d] insert activity failed, activity_id:%u, error:%s", __FUNCTION__, __LINE__, info->activity_id, mysql_error());
		return -1;
	}

	return 0;
}

int save_activity_info(ActivityInfo *info)
{
	static char save_sql[64 * 1024 + 300];
	static uint8_t save_data[64 * 1024 + 1];
	uint64_t effect = 0;
	char *p;

	size_t data_size = pack_activity_info(info, save_data);
	p = save_sql;
	p += sprintf(save_sql, "update activity set `state` = %u, `comm_data` = \'", info->state);
	p += escape_string(p, (const char *)save_data, data_size);
	p += sprintf(p, "\' where `activity_id` = %u", info->activity_id);

	query(const_cast<char*>("set names utf8"), 1, NULL);
	query(save_sql, 1, &effect);	
	if (effect != 1) 
	{
		LOG_ERR("[%s:%d] save activity failed, activity_id:%u, error:%s", __FUNCTION__, __LINE__, info->activity_id, mysql_error());
		return -1;
	}

	return 0;
}

int save_activity_player(ActivityPlayer *player)
{
	static char save_sql[64 * 1024 + 300];
	static uint8_t save_data[64 * 1024 + 1];
	uint64_t effect = 0;
	char *p;

	size_t data_size = pack_activity_player(player, save_data);
	p = save_sql;
	p += sprintf(save_sql, "replace activity_player set `player_id` = %lu, `comm_data` = \'", player->player_id);
	p += escape_string(p, (const char *)save_data, data_size);
	p += sprintf(p, "\'");

	query(const_cast<char*>("set names utf8"), 1, NULL);
	query(save_sql, 1, &effect);	
	if (effect != 1) 
	{
		LOG_ERR("[%s:%d] save activity_player failed, player_id:%lu, error:%s", __FUNCTION__, __LINE__, player->player_id, mysql_error());
		return -1;
	}

	return 0;
}

int begin_activity(ActivityInfo *info)
{
	info->state = TLAS_BEGIN;
	insert_new_activity_to_db(info);

	TimeLimitActivityData nty;
	time_limit_activity_data__init(&nty);

	ZhanlidarenActivityData  zhanlidaren_data;
	ZhanlidarenGiftData  zhanlidaren_gift_data[MAX_ZHANLIDAREN_REWARD_NUM];
	ZhanlidarenGiftData* zhanlidaren_gift_point[MAX_ZHANLIDAREN_REWARD_NUM];
	
	nty.activityid = info->activity_id;
	nty.begintime = info->begin_time;
	nty.endtime = info->end_time;

	LimitActivityControlTable *control_config = get_config_by_id(info->activity_id, &time_limit_control_config);
	if (control_config)
	{
		switch (control_config->Activity)
		{
			case ACTIVITY_ZHANLIDAREN:
				{
					nty.zhanlidaren = &zhanlidaren_data;
					zhanlidaren_activity_data__init(&zhanlidaren_data);

					zhanlidaren_data.gifts = zhanlidaren_gift_point;
					zhanlidaren_data.n_gifts = 0;
					size_t &n_gift = zhanlidaren_data.n_gifts;
					std::map<uint64_t, std::vector<struct PowerMasterTable *> >::iterator iter_batch = zhanlidaren_batch_config.find(control_config->Batch);
					if (iter_batch != zhanlidaren_batch_config.end())
					{
						for (std::vector<struct PowerMasterTable *>::iterator iter_vec = iter_batch->second.begin(); iter_vec != iter_batch->second.end() && n_gift < MAX_ZHANLIDAREN_REWARD_NUM; ++iter_vec)
						{
							PowerMasterTable *zhanlidaren_config = *iter_vec;
							zhanlidaren_gift_point[n_gift] = &zhanlidaren_gift_data[n_gift];
							zhanlidaren_gift_data__init(&zhanlidaren_gift_data[n_gift]);
							zhanlidaren_gift_data[n_gift].id = zhanlidaren_config->ID;
							zhanlidaren_gift_data[n_gift].getnum = 0;
							zhanlidaren_gift_data[n_gift].isget = false;
							n_gift++;
						}
					}
				}
				break;
		}
	}

	conn_node_activitysrv::broadcast_message_to_all(MSG_ID_TIME_LIMIT_ACTIVITY_BEGIN_NOTIFY, &nty, (pack_func)time_limit_activity_data__pack);

	return 0;
}

int end_activity(ActivityInfo *info, uint32_t now)
{
	info->state = TLAS_END;
	uint32_t activity_type = get_time_limit_activity_type(info->activity_id);
	if (activity_type == ACTIVITY_SHIDAMENZONG)
	{
		give_shidamenzong_reward(info, now);
	}
	else
	{
		save_activity_info(info);
	}

	TimeLimitActivityEndNotify nty;
	time_limit_activity_end_notify__init(&nty);
	
	nty.activityid = info->activity_id;

	conn_node_activitysrv::broadcast_message_to_all(MSG_ID_TIME_LIMIT_ACTIVITY_END_NOTIFY, &nty, (pack_func)time_limit_activity_end_notify__pack);

	return 0;
}

void notify_activity_info_to_client(ActivityPlayer *player, EXTERN_DATA *extern_data)
{
	TimeLimitActivityInfoNotify nty;
	time_limit_activity_info_notify__init(&nty);

	TimeLimitActivityData  act_data[MAX_TIME_LIMIT_ACTIVITY_NUM];
	TimeLimitActivityData* act_point[MAX_TIME_LIMIT_ACTIVITY_NUM];
	ZhanlidarenActivityData  zhanlidaren_data[MAX_TIME_LIMIT_ACTIVITY_NUM];
	ZhanlidarenGiftData  zhanlidaren_gift_data[MAX_TIME_LIMIT_ACTIVITY_NUM][MAX_ZHANLIDAREN_REWARD_NUM];
	ZhanlidarenGiftData* zhanlidaren_gift_point[MAX_TIME_LIMIT_ACTIVITY_NUM][MAX_ZHANLIDAREN_REWARD_NUM];

	nty.n_activitys = 0;
	nty.activitys = act_point;
	size_t &n_act = nty.n_activitys;
	for (std::map<uint32_t, ActivityInfo *>::iterator iter_act = activity_map.begin(); iter_act != activity_map.end(); ++iter_act)
	{
		ActivityInfo *info = iter_act->second;
		if (info->state < TLAS_BEGIN || info->state >= TLAS_END)
		{
			continue;
		}

		LimitActivityControlTable *control_config = get_config_by_id(info->activity_id, &time_limit_control_config);
		if (!control_config)
		{
			continue;
		}

		act_point[n_act] = &act_data[n_act];
		time_limit_activity_data__init(&act_data[n_act]);
		act_data[n_act].activityid = info->activity_id;
		act_data[n_act].begintime = info->begin_time;
		act_data[n_act].endtime = info->end_time;

		switch (control_config->Activity)
		{
			case ACTIVITY_ZHANLIDAREN:
				{
					act_data[n_act].zhanlidaren = &zhanlidaren_data[n_act];
					zhanlidaren_activity_data__init(&zhanlidaren_data[n_act]);

					zhanlidaren_data[n_act].gifts = zhanlidaren_gift_point[n_act];
					zhanlidaren_data[n_act].n_gifts = 0;
					size_t &n_gift = zhanlidaren_data[n_act].n_gifts;
					std::map<uint64_t, std::vector<struct PowerMasterTable *> >::iterator iter_batch = zhanlidaren_batch_config.find(control_config->Batch);
					if (iter_batch != zhanlidaren_batch_config.end())
					{
						for (std::vector<struct PowerMasterTable *>::iterator iter_vec = iter_batch->second.begin(); iter_vec != iter_batch->second.end() && n_gift < MAX_ZHANLIDAREN_REWARD_NUM; ++iter_vec)
						{
							PowerMasterTable *zhanlidaren_config = *iter_vec;
							zhanlidaren_gift_point[n_act][n_gift] = &zhanlidaren_gift_data[n_act][n_gift];
							zhanlidaren_gift_data__init(&zhanlidaren_gift_data[n_act][n_gift]);
							zhanlidaren_gift_data[n_act][n_gift].id = zhanlidaren_config->ID;
							zhanlidaren_gift_data[n_act][n_gift].getnum = get_zhanlidaren_gift_reward_num(info, zhanlidaren_config->ID);
							zhanlidaren_gift_data[n_act][n_gift].isget = get_player_zhanlidaren_gift_is_get(player, info->activity_id, zhanlidaren_config->ID);
							n_gift++;
						}
					}
				}
				break;
		}
		n_act++;
	}

	fast_send_msg(&conn_node_activitysrv::connecter, extern_data, MSG_ID_TIME_LIMIT_ACTIVITY_INFO_NOTIFY, time_limit_activity_info_notify__pack, nty);
}


int get_zhanlidaren_gift_reward_num(ActivityInfo *info, uint32_t gift_id)
{
	for (int i = 0; i < MAX_ZHANLIDAREN_REWARD_NUM; ++i)
	{
		if (info->data.zhanlidaren.gift_get[i].id == 0)
		{
			break;
		}
		else if (info->data.zhanlidaren.gift_get[i].id == gift_id)
		{
			return info->data.zhanlidaren.gift_get[i].num;
		}
	}
	return 0;
}

int add_zhanlidaren_gift_reward_num(ActivityInfo *info, uint32_t gift_id)
{
	bool bGood = false;
	for (int i = 0; i < MAX_ZHANLIDAREN_REWARD_NUM; ++i)
	{
		if (info->data.zhanlidaren.gift_get[i].id == 0)
		{
			info->data.zhanlidaren.gift_get[i].id = gift_id;
			info->data.zhanlidaren.gift_get[i].num = 1;
			bGood = true;
			break;
		}
		else if (info->data.zhanlidaren.gift_get[i].id == gift_id)
		{
			info->data.zhanlidaren.gift_get[i].num++;
			bGood = true;
			break;
		}
	}

	if (bGood)
	{
		save_activity_info(info);
		return 0;
	}

	return -1;
}

int sub_zhanlidaren_gift_reward_num(ActivityInfo *info, uint32_t gift_id)
{
	bool bGood = false;
	for (int i = 0; i < MAX_ZHANLIDAREN_REWARD_NUM; ++i)
	{
		if (info->data.zhanlidaren.gift_get[i].id == 0)
		{
			break;
		}
		else if (info->data.zhanlidaren.gift_get[i].id == gift_id)
		{
			if (info->data.zhanlidaren.gift_get[i].num > 0)
			{
				info->data.zhanlidaren.gift_get[i].num--;
				save_activity_info(info);
			}
			bGood = true;
			break;
		}
	}

	if (bGood)
	{
		return 0;
	}

	return -1;
}

void broadcast_zhanlidaren_gift_num_change(ActivityInfo *info, uint32_t gift_id)
{
	ZhanlidarenGiftNumNotify nty;
	zhanlidaren_gift_num_notify__init(&nty);

	nty.activityid = info->activity_id;
	nty.id = gift_id;
	nty.getnum = get_zhanlidaren_gift_reward_num(info, gift_id);

	conn_node_activitysrv::broadcast_message_to_all(MSG_ID_ZHANLIDAREN_GIFT_NUM_NOTIFY, &nty, (pack_func)zhanlidaren_gift_num_notify__pack);
}

bool get_player_zhanlidaren_gift_is_get(ActivityPlayer *player, uint32_t activity_id, uint32_t gift_id)
{
	for (int i = 0; i < MAX_TIME_LIMIT_ACTIVITY_NUM; ++i)
	{
		if (player->details[i].activity_id == 0)
		{
			break;
		}
		else if (player->details[i].activity_id == activity_id)
		{
			for (int j = 0; j < MAX_ZHANLIDAREN_REWARD_NUM; ++j)
			{
				if (player->details[i].matter.zhanlidaren.reward[j] == 0)
				{
					break;
				}
				else if (player->details[i].matter.zhanlidaren.reward[j] == gift_id)
				{
					return true;
				}
			}
			break;
		}
	}
	return false;
}

int mark_player_zhanlidaren_gift_get(ActivityPlayer *player, uint32_t activity_id, uint32_t gift_id)
{
	for (int i = 0; i < MAX_TIME_LIMIT_ACTIVITY_NUM; ++i)
	{
		if (player->details[i].activity_id == 0)
		{
			player->details[i].activity_id = activity_id;
			player->details[i].matter.zhanlidaren.reward[0] = gift_id;
			save_activity_player(player);
			return 0;
		}
		else if (player->details[i].activity_id == activity_id)
		{
			for (int j = 0; j < MAX_ZHANLIDAREN_REWARD_NUM; ++j)
			{
				if (player->details[i].matter.zhanlidaren.reward[j] == 0)
				{
					player->details[i].matter.zhanlidaren.reward[j] = gift_id;
					save_activity_player(player);
					return 0;
				}
				else if (player->details[i].matter.zhanlidaren.reward[j] == gift_id)
				{
					return 0;
				}
			}
			break;
		}
	}
	return -1;
}

int unmark_player_zhanlidaren_gift_get(ActivityPlayer *player, uint32_t activity_id, uint32_t gift_id)
{
	for (int i = 0; i < MAX_TIME_LIMIT_ACTIVITY_NUM; ++i)
	{
		if (player->details[i].activity_id == 0)
		{
			break;
		}
		else if (player->details[i].activity_id == activity_id)
		{
			for (int j = 0; j < MAX_ZHANLIDAREN_REWARD_NUM; ++j)
			{
				if (player->details[i].matter.zhanlidaren.reward[j] == 0)
				{
					break;
				}
				else if (player->details[i].matter.zhanlidaren.reward[j] == gift_id)
				{
					int last = MAX_ZHANLIDAREN_REWARD_NUM - 1;
					if (j < last)
					{
						memmove(&player->details[i].matter.zhanlidaren.reward[j], &player->details[i].matter.zhanlidaren.reward[j + 1], sizeof(uint32_t) * (last - j));
					}
					memset(&player->details[i].matter.zhanlidaren.reward[last], 0, sizeof(uint32_t));
					save_activity_player(player);
					return 0;
				}
			}
			break;
		}
	}
	return -1;
}

static void give_shidamenzong_reward(ActivityInfo *info, uint32_t now)
{
	info->data.shidamenzong.reward_time = now;
	save_activity_info(info);


	LimitActivityControlTable *control_config = get_config_by_id(info->activity_id, &time_limit_control_config);
	if (!control_config)
	{
		LOG_ERR("[%s:%d] reward fail, config miss, activity_id:%u", __FUNCTION__, __LINE__, info->activity_id);
		return;
	}

	std::map<uint64_t, std::vector<struct Top10GangsTable *> >::iterator iter_batch = shidamenzong_batch_config.find(control_config->Batch);
	if (iter_batch == shidamenzong_batch_config.end())
	{
		LOG_ERR("[%s:%d] reward fail, config miss, activity_id:%u", __FUNCTION__, __LINE__, info->activity_id);
		return;
	}

	GiveShidamenzongReward req;
	give_shidamenzong_reward__init(&req);

	std::vector<uint32_t> guild_id;
	ShidamenzongRewardData  reward_data[MAX_SHIDAMENZONG_REWARD_NUM];
	ShidamenzongRewardData* reward_point[MAX_SHIDAMENZONG_REWARD_NUM];
	std::vector<uint32_t> master_reward_id[MAX_SHIDAMENZONG_REWARD_NUM];
	std::vector<uint32_t> master_reward_num[MAX_SHIDAMENZONG_REWARD_NUM];
	std::vector<uint32_t> mass_reward_id[MAX_SHIDAMENZONG_REWARD_NUM];
	std::vector<uint32_t> mass_reward_num[MAX_SHIDAMENZONG_REWARD_NUM];

	req.rewards = reward_point;
	req.n_rewards = 0;
	uint32_t reward_rank = 0;
	for (std::vector<Top10GangsTable*>::iterator iter = iter_batch->second.begin(); iter != iter_batch->second.end() && req.n_rewards < MAX_SHIDAMENZONG_REWARD_NUM; ++iter)
	{
		Top10GangsTable *reward_config = *iter;
		reward_rank = std::max(reward_rank, (uint32_t)reward_config->RewardEndID);
		reward_point[req.n_rewards] = &reward_data[req.n_rewards];
		shidamenzong_reward_data__init(&reward_data[req.n_rewards]);
		reward_data[req.n_rewards].start_rank = reward_config->RewardStartID;
		reward_data[req.n_rewards].stop_rank = reward_config->RewardEndID;
		for (uint32_t i = 0; i < reward_config->n_Reward1 && i < reward_config->n_RewardNum1; ++i)
		{
			master_reward_id[req.n_rewards].push_back(reward_config->Reward1[i]);
			master_reward_num[req.n_rewards].push_back(reward_config->RewardNum1[i]);
		}
		reward_data[req.n_rewards].master_reward_id = &master_reward_id[req.n_rewards][0];
		reward_data[req.n_rewards].n_master_reward_id = master_reward_id[req.n_rewards].size();
		reward_data[req.n_rewards].master_reward_num = &master_reward_num[req.n_rewards][0];
		reward_data[req.n_rewards].n_master_reward_num = master_reward_num[req.n_rewards].size();
		for (uint32_t i = 0; i < reward_config->n_Reward2 && i < reward_config->n_RewardNum2; ++i)
		{
			mass_reward_id[req.n_rewards].push_back(reward_config->Reward2[i]);
			mass_reward_num[req.n_rewards].push_back(reward_config->RewardNum2[i]);
		}
		reward_data[req.n_rewards].mass_reward_id = &mass_reward_id[req.n_rewards][0];
		reward_data[req.n_rewards].n_mass_reward_id = mass_reward_id[req.n_rewards].size();
		reward_data[req.n_rewards].mass_reward_num = &mass_reward_num[req.n_rewards][0];
		reward_data[req.n_rewards].n_mass_reward_num = mass_reward_num[req.n_rewards].size();

		req.n_rewards++;
	}

	//获取排行榜名单
	std::vector<std::pair<uint64_t, uint32_t> > rank_info;
	int ret2 = sg_redis_client.zget(sg_rank_guild_fc_total_key, 0, reward_rank - 1, rank_info);
	if (ret2 == 0)
	{
		for (size_t i = 0; i < rank_info.size(); ++i)
		{
			guild_id.push_back(rank_info[i].first);
		}
	}

	req.activity_id = info->activity_id;
	req.guild_ids = &guild_id[0];
	req.n_guild_ids = guild_id.size();

	EXTERN_DATA ext_data;
	fast_send_msg(&conn_node_activitysrv::connecter, &ext_data, SERVER_PROTO_ACTIVITY_SHIDAMENZONG_GIVE_REWARD_REQUEST, give_shidamenzong_reward__pack, req);
}

