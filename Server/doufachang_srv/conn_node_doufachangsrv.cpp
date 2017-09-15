#include "conn_node_doufachangsrv.h"
#include "comm_message.pb-c.h"
#include "app_data_statis.h"
#include <set>
#include "redis_util.h"
#include "game_event.h"
#include "time_helper.h"
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <sstream>
#include <string.h>
#include <map>
#include <unistd.h>
#include "doufachang_util.h"
#include "doufachang.pb-c.h"
#include "server_proto.h"
#include "msgid.h"
#include "error_code.h"
#include "attr_id.h"

#define UNUSED(x) (void)(x)
//一局比赛最大时间
#define MAX_CHALLENGE_TIME 360 * 1000 * 1000
//每次随机的对手数量
#define MAX_CHALLENGE_PLAYER 4
//最大记录数
#define MAX_CHALLENGE_RECORD 20
//每天最大购买次数
#define MAX_CHALLENGE_BUY_COUNT 10

static bool doufachang_rank_full;
conn_node_doufachangsrv *conn_node_doufachangsrv::connecter = NULL;
PlayerDoufachangInfo conn_node_doufachangsrv::default_info;
char conn_node_doufachangsrv::server_key[64];
char conn_node_doufachangsrv::doufachang_rank_key[64];
char conn_node_doufachangsrv::doufachang_rank2_key[64];
char conn_node_doufachangsrv::doufachang_rank_reward_key[64];
char conn_node_doufachangsrv::doufachang_lock_key[64];
char conn_node_doufachangsrv::doufachang_record_key[64];
char conn_node_doufachangsrv::doufachang_key[64];
CRedisClient sg_redis_client;
uint32_t sg_server_id = 0;

conn_node_doufachangsrv::conn_node_doufachangsrv()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);
}

conn_node_doufachangsrv::~conn_node_doufachangsrv()
{
}

int conn_node_doufachangsrv::set_player_locked(uint64_t player_id, uint64_t target_id, uint64_t now)
{
	char buf[128];
	char player_key[32];
	sprintf(buf, "%lu_%lu", target_id, now);
	sprintf(player_key, "%lu", player_id);
	int ret = sg_redis_client.set(doufachang_lock_key, player_key, buf);
	if (ret != 0)
	{
		LOG_ERR("%s: player[%lu] set locked failed\n", __FUNCTION__, player_id);
	}

	if (player_id != target_id)
	{
		sprintf(buf, "%lu_%lu", player_id, now);
		sprintf(player_key, "%lu", target_id);
		ret = sg_redis_client.set(doufachang_lock_key, player_key, buf);
		if (ret != 0)
		{
			LOG_ERR("%s: player[%lu] set locked failed\n", __FUNCTION__, target_id);
		}
	}
	LOG_INFO("%s: player[%lu][%lu] locked", __FUNCTION__, player_id, target_id);
	return (0);
}
int conn_node_doufachangsrv::set_player_unlocked(uint64_t player_id, uint64_t target_id)
{
	char player_key[32];

	sprintf(player_key, "%lu", player_id);
	int ret = sg_redis_client.hdel(doufachang_lock_key, player_key);
	if (ret != 0)
	{
		LOG_ERR("%s: player[%lu] set unlocked failed\n", __FUNCTION__, player_id);
	}

	if (player_id != target_id)
	{
		sprintf(player_key, "%lu", target_id);
		ret = sg_redis_client.hdel(doufachang_lock_key, player_key);
		if (ret != 0)
		{
			LOG_ERR("%s: player[%lu] set unlocked failed\n", __FUNCTION__, target_id);
		}
	}
	LOG_INFO("%s: player[%lu][%lu] unlocked", __FUNCTION__, player_id, target_id);
	return (0);
}

int conn_node_doufachangsrv::is_player_locked(uint64_t player_id, uint64_t now)
{
	char buf[128];
	int len = sizeof(buf) - 1;
	char player_key[32];
	sprintf(player_key, "%lu", player_id);
	int ret = sg_redis_client.hget_bin(doufachang_lock_key, player_key, buf, &len);
	if (ret != 0)
		return (0);

	assert(len < (int)sizeof(buf));
	buf[len] = '\0';
	uint64_t t1, t2;
	if (sscanf(buf, "%lu_%lu", &t1, &t2) != 2)
		return (0);

	if (now < t2 || now - t2 >= MAX_CHALLENGE_TIME)
		return (0);
	return (1);
}

uint32_t conn_node_doufachangsrv::add_challenge_rank(DOUFACHANG_CHALLENGE_ANSWER *ans)
{
	if (ans->result != 0)
		return (0);

	uint64_t playerid[2] = {ans->attack, ans->defence};
	uint64_t rank[2];
	sg_redis_client.mget_uint64(conn_node_doufachangsrv::doufachang_rank2_key, 2, playerid, rank);
	if (rank[1] == 0)
	{
			//被挑战者不在榜上，不用交换
		return 0;
	}
	if (rank[0] == 0)
	{
			//挑战者不在榜上
			//rank2 删除defence，添加attack
			//rank  设置成attack
		LOG_INFO("%s: %lu[%lu] => %lu[%lu]", __FUNCTION__, playerid[0], rank[0], playerid[1], rank[1]);
		sg_redis_client.hdel(conn_node_doufachangsrv::doufachang_rank2_key, ans->defence);
		sg_redis_client.mset_uint64(conn_node_doufachangsrv::doufachang_rank2_key, 1, &ans->attack, &rank[1]);
		sg_redis_client.mset_uint64(conn_node_doufachangsrv::doufachang_rank_key, 1, &rank[1], &ans->attack);
		return (DOUFACHANG_MAX_RANK - rank[1]);
	}

	if (rank[0] <= rank[1])
	{
			//挑战者排名更靠前
		return (0);
	}

	LOG_INFO("%s: %lu[%lu] => %lu[%lu]", __FUNCTION__, playerid[0], rank[0], playerid[1], rank[1]);

	uint64_t t = rank[0];
	rank[0] = rank[1];
	rank[1] = t;

	sg_redis_client.mset_uint64(conn_node_doufachangsrv::doufachang_rank2_key, 2, playerid, rank);
	sg_redis_client.mset_uint64(conn_node_doufachangsrv::doufachang_rank_key, 2, rank, playerid);
	return (rank[1] - rank[0]);
}

int conn_node_doufachangsrv::add_challenge_record(DOUFACHANG_CHALLENGE_ANSWER *ans, uint32_t rank_add)
{
	if (ans->result != 0)
		return (0);
	AutoReleaseRedisPlayer t2;
	PlayerRedisInfo *player_info = get_redis_player(ans->defence, server_key, sg_redis_client, t2);
	if (!player_info)
	{
		return (0);
	}
	DoufachangRecordEntry entry;
	doufachang_record_entry__init(&entry);
	entry.time = time_helper::get_micro_time() / 1000000;
	entry.level = player_info->lv;
	entry.fight = player_info->fighting_capacity;
	entry.job = player_info->job;
	entry.name = player_info->name;
	entry.rank_add = rank_add;

	AutoReleaseDoufachangRecord t1;
	DoufachangRecordAnswer *info;
	info = get_player_doufachang_record(ans->attack, doufachang_record_key, sg_redis_client, t1);
	if (!info)
	{
		DoufachangRecordEntry *point[1];
		point[0] = &entry;
		DoufachangRecordAnswer info;
		doufachang_record_answer__init(&info);
		info.n_record = 1;
		info.record = point;
		save_player_doufachang_record(&info, ans->attack, doufachang_record_key, sg_redis_client);
		return (0);
	}

	DoufachangRecordEntry *point[MAX_CHALLENGE_RECORD];
	DoufachangRecordEntry **bakpoint = info->record;
	int bak_n_record = info->n_record;
	info->record = point;

	if (info->n_record >= MAX_CHALLENGE_RECORD)
	{
		for (int i = 0; i < MAX_CHALLENGE_RECORD - 1; ++i)
		{
			point[i] = info->record[i+1];
		}
		point[MAX_CHALLENGE_RECORD - 1] = &entry;
	}
	else
	{
		for (size_t i = 0; i < info->n_record; ++i)
		{
			point[i] = bakpoint[i];
		}
		point[info->n_record] = &entry;
		++info->n_record;
	}
	save_player_doufachang_record(info, ans->attack, doufachang_record_key, sg_redis_client);
	info->record = bakpoint;
	info->n_record = bak_n_record;

	return (0);
}

int conn_node_doufachangsrv::handle_challenge_answer(EXTERN_DATA *extern_data)
{
	DOUFACHANG_CHALLENGE_ANSWER *ans = (DOUFACHANG_CHALLENGE_ANSWER *)get_data();

	set_player_unlocked(ans->attack, ans->defence);
		// 计算排名
	uint32_t rank_add = add_challenge_rank(ans);
		// 记录record
	add_challenge_record(ans, rank_add);
		// 发送MSG_ID_DOUFACHANG_RAID_FINISHED_NOTIFY
	if (ans->notify)
	{
		DoufachangRaidFinishedNotify nty;
		doufachang_raid_finished_notify__init(&nty);
		nty.result = ans->result;
		nty.rank_add = rank_add;
		nty.courage_gold_add = ans->add_gold;
		fast_send_msg(connecter, extern_data, MSG_ID_DOUFACHANG_RAID_FINISHED_NOTIFY, doufachang_raid_finished_notify__pack, nty);
	}
	return (0);
}

int conn_node_doufachangsrv::handle_challenge_request(EXTERN_DATA *extern_data)
{
	DOUFACHANG_CHALLENGE_REQUEST *t = (DOUFACHANG_CHALLENGE_REQUEST *)connecter->get_send_data();
	DoufachangChallengeRequest *req = doufachang_challenge_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("%s: [%lu]req unpack failed", __FUNCTION__, extern_data->player_id);
		return -1;
	}
	uint64_t target_id = req->player_id;
	doufachang_challenge_request__free_unpacked(req, NULL);

	LOG_INFO("%s: player[%lu] challenge %lu", __FUNCTION__, extern_data->player_id, target_id);

	AutoReleaseDoufachangInfo t1;
	PlayerDoufachangInfo *info;

	DoufachangChallengeAnswer ans;
	doufachang_challenge_answer__init(&ans);
	ans.player_id = target_id;

	uint64_t now = time_helper::get_micro_time();
	if (is_player_locked(extern_data->player_id, now) != 0)
	{
		LOG_INFO("%s: player[%lu] already in challenge", __FUNCTION__, extern_data->player_id);
		ans.result = 1;
		goto done;
	}

	if (extern_data->player_id != target_id && is_player_locked(target_id, now) != 0)
	{
		LOG_INFO("%s: player[%lu] target[%lu] already in challenge", __FUNCTION__, extern_data->player_id, target_id);
		ans.result = 2;
		goto done;
	}

	info = get_player_doufachang_info(extern_data->player_id, doufachang_key, sg_redis_client, t1);
	if (!info)
	{
		info = &conn_node_doufachangsrv::default_info;
		--info->challenge_count;
		info->next_add_count = now / 1000000 + ADD_COUNT_SECONDS;
		save_player_doufachang_info(info, extern_data->player_id, doufachang_key, sg_redis_client);
		reinit_default_doufachang_info(info);
	}
	else
	{
		update_doufachang_player_info(extern_data->player_id, info, now / 1000000);
		if (info->challenge_count <= 0)
		{
			ans.result = 3;
			goto done;
		}
		--info->challenge_count;
		save_player_doufachang_info(info, extern_data->player_id, doufachang_key, sg_redis_client);
	}

	set_player_locked(extern_data->player_id, target_id, now);

	t->attack = extern_data->player_id;
	t->defence = target_id;
	fast_send_msg_base(connecter, extern_data, SERVER_PROTO_DOUFACHANG_CHALLENGE_REQUEST, sizeof(*t), 0);

done:
	fast_send_msg(connecter, extern_data, MSG_ID_DOUFACHANG_CHALLENGE_ANSWER, doufachang_challenge_answer__pack, ans);
	return (0);
}

//玩家挑战成功，交换排行
// __attribute_used__ static void inc_player_rank(uint64_t attack, uint64_t defence)
// {
// 	uint64_t playerid[2] = {attack, defence};
// 	uint64_t rank[2];
// 	sg_redis_client.mget_uint64(conn_node_doufachangsrv::doufachang_rank2_key, 2, playerid, rank);
// 	if (rank[1] == 0)
// 	{
// 			//被挑战者不在榜上，不用交换
// 		return;
// 	}
// 	if (rank[0] == 0)
// 	{
// 			//挑战者不在榜上
// 			//rank2 删除defence，添加attack
// 			//rank  设置成attack
// 		LOG_INFO("%s: %lu[%lu] => %lu[%lu]", __FUNCTION__, playerid[0], rank[0], playerid[1], rank[1]);
// 		sg_redis_client.hdel(conn_node_doufachangsrv::doufachang_rank2_key, defence);
// 		sg_redis_client.mset_uint64(conn_node_doufachangsrv::doufachang_rank2_key, 1, &attack, &rank[1]);
// 		sg_redis_client.mset_uint64(conn_node_doufachangsrv::doufachang_rank_key, 1, &rank[1], &attack);
// 		return;
// 	}

// 	if (rank[0] <= rank[1])
// 	{
// 			//挑战者排名更靠前
// 		return;
// 	}

// 	LOG_INFO("%s: %lu[%lu] => %lu[%lu]", __FUNCTION__, playerid[0], rank[0], playerid[1], rank[1]);

// 	uint64_t t = rank[0];
// 	rank[0] = rank[1];
// 	rank[1] = t;

// 	sg_redis_client.mset_uint64(conn_node_doufachangsrv::doufachang_rank2_key, 2, playerid, rank);
// 	sg_redis_client.mset_uint64(conn_node_doufachangsrv::doufachang_rank_key, 2, rank, playerid);
// }

//把玩家加入排行榜, 返回当前排行
static int add_player_into_rank(uint64_t player_id)
{
	uint64_t rank;
	if (sg_redis_client.mget_uint64(conn_node_doufachangsrv::doufachang_rank2_key, 1, &player_id, &rank) == 0
		&& rank != 0)
	{
		return rank;
	}
	if (doufachang_rank_full)
	{
		return DOUFACHANG_MAX_RANK;
	}
	uint64_t num;
	int ret = sg_redis_client.size(conn_node_doufachangsrv::doufachang_rank_key);
	if (ret < 0)
	{
		LOG_ERR("%s: get doufachang rank failed", __FUNCTION__);
		return (-1);
	}
	num = ret;
	if (num >= DOUFACHANG_MAX_RANK)
	{
		doufachang_rank_full = true;
		return DOUFACHANG_MAX_RANK;
	}
	num++;
	if (sg_redis_client.mset_uint64(conn_node_doufachangsrv::doufachang_rank2_key, 1, &player_id, &num) != 0)
	{
		LOG_ERR("%s: get doufachang rank failed", __FUNCTION__);
		return -1;
	}
	if (sg_redis_client.mset_uint64(conn_node_doufachangsrv::doufachang_rank_key, 1, &num, &player_id) != 0)
	{
		LOG_ERR("%s: get doufachang rank failed", __FUNCTION__);
		return -1;
	}

	return num;
}

// static void	send_player_default_doufachang_info(conn_node_base* connecter, EXTERN_DATA *extern_data, int rank)
// {
//	DoufachangInfoAnswer ans;
//	doufachang_info_answer__init(&ans);
//	ans.challenge_count = DEFAULT_CHALLENGE_COUNT;
//	ans.current_rank = rank;
//	fast_send_msg(connecter, extern_data, MSG_ID_DOUFACHANG_INFO_ANSWER, doufachang_info_answer__pack, ans);
// }

const static int delta1 = DOUFACHANG_MAX_RANK * 3 / 10;
static int get_rank_player(int cur_rank, uint64_t player_id, uint64_t *rank_player_id, uint64_t *rank_player_rank)
{
//	*num = MAX_CHALLENGE_PLAYER;
//	uint64_t rank[MAX_CHALLENGE_PLAYER];
		//没上榜
	if (cur_rank >= DOUFACHANG_MAX_RANK)
	{
		for (int i = 0; i < MAX_CHALLENGE_PLAYER; ++i)
		{
			rank_player_rank[i] = DOUFACHANG_MAX_RANK - random() % delta1;
		}
		sg_redis_client.mget_uint64(conn_node_doufachangsrv::doufachang_rank_key, MAX_CHALLENGE_PLAYER, rank_player_rank, rank_player_id);
		return (0);
	}

	if (cur_rank <= MAX_CHALLENGE_PLAYER)
	{
		for (int i = 0; i < MAX_CHALLENGE_PLAYER; ++i)
		{
			rank_player_rank[i] = i + 1;
		}
		sg_redis_client.mget_uint64(conn_node_doufachangsrv::doufachang_rank_key, MAX_CHALLENGE_PLAYER, rank_player_rank, rank_player_id);
		return (0);
	}
	

	if (cur_rank <= 30)
	{
		uint64_t size = sg_redis_client.size(conn_node_doufachangsrv::doufachang_rank_key);		
		rank_player_rank[0] = cur_rank - 2;
		rank_player_rank[1] = cur_rank - 1;
		rank_player_rank[2] = cur_rank + 1;
		if (rank_player_rank[2] > size && cur_rank - 3 > 0)
			rank_player_rank[2] = cur_rank - 3;
		rank_player_rank[3] = cur_rank + 2;
		if (rank_player_rank[3] > size && cur_rank - 4 > 0)
			rank_player_rank[3] = cur_rank - 4;		
		sg_redis_client.mget_uint64(conn_node_doufachangsrv::doufachang_rank_key, MAX_CHALLENGE_PLAYER, rank_player_rank, rank_player_id);
		return (0);
	}

	int t = cur_rank / 10;
	int min_rank = cur_rank - t;
	int max_rank = cur_rank + t;
	int size = sg_redis_client.size(conn_node_doufachangsrv::doufachang_rank_key);
	if (size >= DOUFACHANG_MAX_RANK)
		size = DOUFACHANG_MAX_RANK;
	if (max_rank >= size)
		max_rank = size - 1;

	for (int i = 0; i < MAX_CHALLENGE_PLAYER; ++i)
		rank_player_rank[i] = 0;
	
	if (max_rank > min_rank)
	{
		for (int i = 0; i < MAX_CHALLENGE_PLAYER; ++i)
		{
			int n = min_rank + random() % (max_rank - min_rank);
			rank_player_rank[i] = n;
		}
	}

	t = 0;
	for (int i = 0; i < MAX_CHALLENGE_PLAYER; ++i)
	{
		if (rank_player_rank[i] == 0 || (int)rank_player_rank[i] == cur_rank)
		{
			rank_player_rank[i] = min_rank - t;
			t--;
		}
		for (int j = 0; j < i; ++j)
		{
			if (rank_player_rank[i] == rank_player_rank[j])
			{
				rank_player_rank[i] = min_rank - t;
				t--;
				break;
			}
		}
	}
	
	sg_redis_client.mget_uint64(conn_node_doufachangsrv::doufachang_rank_key, MAX_CHALLENGE_PLAYER, rank_player_rank, rank_player_id);

	return (0);
}

int conn_node_doufachangsrv::handle_info_request(EXTERN_DATA *extern_data)
{
	uint64_t now = time_helper::get_micro_time();
	AutoReleaseDoufachangInfo t1;
	PlayerDoufachangInfo *info;

	int rank = add_player_into_rank(extern_data->player_id);

	info = get_player_doufachang_info(extern_data->player_id, doufachang_key, sg_redis_client, t1);
	if (!info)
	{
//		send_player_default_doufachang_info(connecter, extern_data, rank);
//		return (0);
		info = &default_info;
	}
	else
	{
		if (update_doufachang_player_info(extern_data->player_id, info, now / 1000000))
		{
			save_player_doufachang_info(info, extern_data->player_id, doufachang_key, sg_redis_client);
		}
	}

//	int num;
	uint64_t rank_player_id[MAX_CHALLENGE_PLAYER];
	uint64_t rank_player_rank[MAX_CHALLENGE_PLAYER];
	if (get_rank_player(rank, extern_data->player_id, rank_player_id, rank_player_rank) != 0)
	{
		LOG_ERR("%s: get rank target player failed", __FUNCTION__);
	}

	std::set<uint64_t> player_ids;
	for (int i = 0; i < MAX_CHALLENGE_PLAYER; ++i)
	{
		if (rank_player_id[i] != 0)
			player_ids.insert(rank_player_id[i]);
	}
	AutoReleaseBatchRedisPlayer t2;
	std::map<uint64_t, PlayerRedisInfo*> redis_player;
	get_more_redis_player(player_ids, redis_player, server_key, sg_redis_client, t2);

	ChallengePlayer *target_point[MAX_CHALLENGE_PLAYER];
	ChallengePlayer target[MAX_CHALLENGE_PLAYER];
	int i = 0;
	for (std::map<uint64_t, PlayerRedisInfo*>::iterator ite = redis_player.begin();
		 ite != redis_player.end(); ++ite)
	{
		target_point[i] = &target[i];
		challenge_player__init(&target[i]);
		target[i].player_id = ite->first;
		target[i].name = ite->second->name;
		target[i].level = ite->second->lv;
		target[i].fight = ite->second->fighting_capacity;
		target[i].job = ite->second->job;
		target[i].head_icon = ite->second->head_icon;
		target[i].n_partner_id = ite->second->n_partner;
		target[i].partner_id = ite->second->partner;
		for (int j = 0; j < MAX_CHALLENGE_PLAYER; ++j)
		{
			if (target[i].player_id != rank_player_id[j])
				continue;
			target[i].rank = rank_player_rank[j];
			break;
		}
		++i;
	}

	DoufachangInfoAnswer ans;
	doufachang_info_answer__init(&ans);
	ans.challenge_count = info->challenge_count;
	ans.next_add_count = info->next_add_count;
	ans.buy_count = info->buy_count;
	ans.reward_id = info->reward_id;
	ans.current_rank = rank;
	ans.n_player = i;
	ans.player = target_point;

	fast_send_msg(connecter, extern_data, MSG_ID_DOUFACHANG_INFO_ANSWER, doufachang_info_answer__pack, ans);
	return (0);
}
int conn_node_doufachangsrv::handle_get_reward_request(EXTERN_DATA *extern_data)
{
	uint64_t now = time_helper::get_micro_time();
	AutoReleaseDoufachangInfo t1;
	PlayerDoufachangInfo *info;
	info = get_player_doufachang_info(extern_data->player_id, doufachang_key, sg_redis_client, t1);
	if (!info)
	{
		CommAnswer resp;
		comm_answer__init(&resp);
		resp.result = 1;
		fast_send_msg(connecter, extern_data, MSG_ID_DOUFACHANG_GET_REWARD_ANSWER, comm_answer__pack, resp);
		return (0);
	}
	update_doufachang_player_info(extern_data->player_id, info, now / 1000000);
	if (info->reward_id == 0)
	{
		CommAnswer resp;
		comm_answer__init(&resp);
		resp.result = 1;
		fast_send_msg(connecter, extern_data, MSG_ID_DOUFACHANG_GET_REWARD_ANSWER, comm_answer__pack, resp);
		return (0);
	}

	uint32_t reward_id = info->reward_id;

	LOG_INFO("%s: player[%lu] add reward %u", __FUNCTION__, extern_data->player_id, reward_id);	

	info->reward_id = 0;
	save_player_doufachang_info(info, extern_data->player_id, doufachang_key, sg_redis_client);		

	DOUFACHANG_GET_REWARD_REQUEST *req = (DOUFACHANG_GET_REWARD_REQUEST *)get_send_data();
	req->reward_id = reward_id;
	fast_send_msg_base(connecter, extern_data, SERVER_PROTO_DOUFACHANG_ADD_REWARD_REQUEST, sizeof(*req), 0);

	return (0);
}

int conn_node_doufachangsrv::handle_server_add_reward_answer(EXTERN_DATA *extern_data)
{
	DOUFACHANG_GET_REWARD_ANSWER *server_ans = (DOUFACHANG_GET_REWARD_ANSWER *)get_data();
		//如果领奖成功，告诉客户端
	if (server_ans->result == 1)
	{
		CommAnswer resp;
		comm_answer__init(&resp);
		fast_send_msg(connecter, extern_data, MSG_ID_DOUFACHANG_GET_REWARD_ANSWER, comm_answer__pack, resp);
		return (0);
	}

		//如果失败，要恢复领奖ID
	LOG_INFO("%s: player[%lu] add reward failed", __FUNCTION__, extern_data->player_id);
	uint64_t now = time_helper::get_micro_time();
	AutoReleaseDoufachangInfo t1;
	PlayerDoufachangInfo *info;
	info = get_player_doufachang_info(extern_data->player_id, doufachang_key, sg_redis_client, t1);
	if (!info)
	{
		LOG_ERR("%s: player[%lu] get doufachang info failed", __FUNCTION__, extern_data->player_id);
		CommAnswer resp;
		comm_answer__init(&resp);
		resp.result = 1;
		fast_send_msg(connecter, extern_data, MSG_ID_DOUFACHANG_GET_REWARD_ANSWER, comm_answer__pack, resp);
		return (0);
	}
	info->refresh_reward_id = 0;
	if (update_doufachang_player_info(extern_data->player_id, info, now / 1000000))
	{
		save_player_doufachang_info(info, extern_data->player_id, doufachang_key, sg_redis_client);
	}
	
	CommAnswer resp;
	comm_answer__init(&resp);
	resp.result = server_ans->result;
	fast_send_msg(connecter, extern_data, MSG_ID_DOUFACHANG_GET_REWARD_ANSWER, comm_answer__pack, resp);
	return (0);
}

int conn_node_doufachangsrv::handle_server_buy_challenge_answer(EXTERN_DATA *extern_data)
{
	DOUFACHANG_BUY_CHALLENGE_ANSWER *server_ans = (DOUFACHANG_BUY_CHALLENGE_ANSWER *)buf_head();

	DoufachangBuyChallengeAnswer ans;
	doufachang_buy_challenge_answer__init(&ans);

	uint64_t now = time_helper::get_micro_time();
	AutoReleaseDoufachangInfo t1;
	PlayerDoufachangInfo *info;
	info = get_player_doufachang_info(extern_data->player_id, doufachang_key, sg_redis_client, t1);
	if (!info)
	{
		info = &default_info;
	}
	else
	{
		update_doufachang_player_info(extern_data->player_id, info, now / 1000000);
	}

	if (server_ans->result != 0)
	{
		ans.result = server_ans->result;
	}
	else if (info->buy_count + server_ans->count > MAX_CHALLENGE_BUY_COUNT)
	{
			//SERVER_PROTO_UNDO_COST
		LOG_INFO("%s: player[%lu] undo cost %u %u", __FUNCTION__, extern_data->player_id, server_ans->count, server_ans->gold_num);
		ans.result = 1;

		PROTO_UNDO_COST *proto = (PROTO_UNDO_COST*)get_send_buf(SERVER_PROTO_UNDO_COST, 0);
		proto->head.len = ENDION_FUNC_4(sizeof(PROTO_UNDO_COST));
		proto->head.msg_id = SERVER_PROTO_UNDO_COST;
		proto->head.seq = 0;
		memset(&proto->cost, 0, sizeof(proto->cost));
		proto->cost.statis_id = MAGIC_TYPE_DOUFACHANG_BUY_CHALLENGE_COUNT;
		proto->cost.gold = server_ans->gold_num;
		conn_node_base::add_extern_data(&proto->head, extern_data);
		if (connecter->send_one_msg(&proto->head, 1) != (int)ENDION_FUNC_4(proto->head.len))
		{
			LOG_ERR("[%s:%d] send to gamesrv failed err[%d]", __FUNCTION__, __LINE__, errno);
		}
	}
	else
	{
		info->buy_count += server_ans->count;
		info->challenge_count += server_ans->count;
		save_player_doufachang_info(info, extern_data->player_id, doufachang_key, sg_redis_client);
	}
	ans.challenge_count = info->challenge_count;
	ans.buy_count = info->buy_count;

	fast_send_msg(connecter, extern_data, MSG_ID_DOUFACHANG_BUY_CHALLENGE_ANSWER, doufachang_buy_challenge_answer__pack, ans);
	return (0);
}

int conn_node_doufachangsrv::handle_buy_challenge_request(EXTERN_DATA *extern_data)
{
	int count = 1;
	DoufachangBuyChallengeRequest *t = doufachang_buy_challenge_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!t)
	{
		LOG_ERR("%s: [%lu]req unpack failed", __FUNCTION__, extern_data->player_id);
		return -1;
	}
	count = t->count;
	doufachang_buy_challenge_request__free_unpacked(t, NULL);

	DOUFACHANG_BUY_CHALLENGE_REQUEST *req = (DOUFACHANG_BUY_CHALLENGE_REQUEST*)get_send_buf(SERVER_PROTO_DOUFACHANG_BUY_CHALLENGE_REQUEST, 0);
	req->head.len = ENDION_FUNC_4(sizeof(DOUFACHANG_BUY_CHALLENGE_REQUEST));
	req->count = count;
	req->gold_num = count * 20;
	conn_node_base::add_extern_data(&req->head, extern_data);
	if (connecter->send_one_msg(&req->head, 1) != (int)ENDION_FUNC_4(req->head.len))
	{
		LOG_ERR("[%s:%d] send to gamesrv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
	return (0);
}

int conn_node_doufachangsrv::handle_record_request(EXTERN_DATA *extern_data)
{
	AutoReleaseDoufachangRecord t1;
	DoufachangRecordAnswer *info;
	DoufachangRecordAnswer _default;
	info = get_player_doufachang_record(extern_data->player_id, doufachang_record_key, sg_redis_client, t1);
	if (!info)
	{
		info = &_default;
		doufachang_record_answer__init(info);
	}
	fast_send_msg(connecter, extern_data, MSG_ID_DOUFACHANG_RECORD_ANSWER, doufachang_record_answer__pack, *info);
	return (0);
}

int conn_node_doufachangsrv::recv_func(evutil_socket_t fd)
{
	EXTERN_DATA *extern_data;
	PROTO_HEAD *head;
	for (;;) {
		int ret = get_one_buf();
		if (ret == 0) {
			head = (PROTO_HEAD *)buf_head();
			extern_data = get_extern_data(head);
			int cmd = get_cmd();
			LOG_DEBUG("[%s:%d] cmd: %u, player_id: %lu", __FUNCTION__, __LINE__, cmd, extern_data->player_id);
			switch(cmd)
			{
				case MSG_ID_DOUFACHANG_CHALLENGE_REQUEST:
				{
					handle_challenge_request(extern_data);
				}
				break;
				case SERVER_PROTO_DOUFACHANG_CHALLENGE_ANSWER:
				{
					handle_challenge_answer(extern_data);
				}
				break;
				case MSG_ID_DOUFACHANG_INFO_REQUEST:
				{
					handle_info_request(extern_data);
				}
				break;
				case MSG_ID_DOUFACHANG_GET_REWARD_REQUEST:
				{
					handle_get_reward_request(extern_data);
				}
				break;
				case MSG_ID_DOUFACHANG_RECORD_REQUEST:
				{
					handle_record_request(extern_data);
				}
				break;
				case MSG_ID_DOUFACHANG_BUY_CHALLENGE_REQUEST:
				{
					handle_buy_challenge_request(extern_data);
				}
				break;
				case SERVER_PROTO_DOUFACHANG_BUY_CHALLENGE_ANSWER:
				{
					handle_server_buy_challenge_answer(extern_data);
				}
				break;
				case SERVER_PROTO_DOUFACHANG_ADD_REWARD_ANSWER:
				{
					handle_server_add_reward_answer(extern_data);
				}
				break;
			}
		}

		if (ret < 0) {
			LOG_INFO("%s: connect closed from fd %u, err = %d", __FUNCTION__, fd, errno);
			event_base_loopexit(base, NULL);
			return (-1);
		} else if (ret > 0) {
			break;
		}

		ret = remove_one_buf();
	}
	return (0);
}

conn_node_doufachangsrv* conn_node_doufachangsrv::instance(void)
{
	if (connecter == NULL)
	{
		connecter = new conn_node_doufachangsrv();
	}
	return connecter;
}
