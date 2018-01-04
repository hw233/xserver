#include "conn_node_activitysrv.h"
#include "game_event.h"
#include "msgid.h"
#include "error_code.h"
#include "time_helper.h"
#include <vector>
#include <map>
#include <set>
#include "activity.pb-c.h"
#include "player_redis_info.pb-c.h"
#include "redis_util.h"
#include "activity_db.pb-c.h"
#include "send_mail.h"
#include "app_data_statis.h"
#include "activity_config.h"
#include "activity_util.h" 

extern char sg_player_key[];
extern CRedisClient sg_redis_client;

conn_node_activitysrv conn_node_activitysrv::connecter;

static int handle_zhanlidaren_reward_answer(int data_len, uint8_t *data, int result, EXTERN_DATA *extern_data);

conn_node_activitysrv::conn_node_activitysrv()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);
	
	add_msg_handle(SERVER_PROTO_PLAYER_ONLINE_NOTIFY, &conn_node_activitysrv::handle_player_online_notify);
	add_msg_handle(SERVER_PROTO_ACTIVITYSRV_REWARD_ANSWER, &conn_node_activitysrv::handle_gamesrv_reward_answer);
	add_msg_handle(SERVER_PROTO_ACTIVITY_SHIDAMENZONG_GIVE_REWARD_ANSWER, &conn_node_activitysrv::handle_guildsrv_give_shidamenzong_reward_answer);

	add_msg_handle(MSG_ID_ZHANLIDAREN_GET_REWARD_REQUEST, &conn_node_activitysrv::handle_zhanlidaren_get_reward_request);
}

conn_node_activitysrv::~conn_node_activitysrv()
{
}

void conn_node_activitysrv::add_msg_handle(uint32_t msg_id, handle_func func)
{
	connecter.m_handleMap[msg_id] = func;
}

int conn_node_activitysrv::recv_func(evutil_socket_t fd)
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
			if (cmd == SERVER_PROTO_GAMESRV_START)
			{
				LOG_INFO("[%s:%d] game_srv start notify.", __FUNCTION__, __LINE__);
			}

			uint64_t times = time_helper::get_micro_time();
			time_helper::set_cached_time(times / 1000);

			HandleMap::iterator iter = m_handleMap.find(cmd);
			if (iter != m_handleMap.end())
			{
				(this->*(iter->second))(extern_data);
			}
			else
			{
				LOG_ERR("[%s:%d] cmd %u has no handler", __FUNCTION__, __LINE__, cmd);
			}
		}
		
		if (ret < 0) {
			LOG_INFO("%s: connect closed from fd %u, err = %d", __FUNCTION__, fd, errno);
			exit(0);
			return (-1);		
		} else if (ret > 0) {
			break;
		}
		
		ret = remove_one_buf();
	}
	return (0);
}

PlayerRedisInfo *find_redis_from_map(std::map<uint64_t, PlayerRedisInfo*> &redis_players, uint64_t player_id)
{
	std::map<uint64_t, PlayerRedisInfo*>::iterator iter = redis_players.find(player_id);
	if (iter != redis_players.end())
	{
		return iter->second;
	}
	return NULL;
}

int conn_node_activitysrv::broadcast_message(uint16_t msg_id, void *msg_data, pack_func packer, std::vector<uint64_t> &players)
{
	PROTO_HEAD_CONN_BROADCAST *head;
	PROTO_HEAD *real_head;

	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
	head->msg_id = ENDION_FUNC_2(SERVER_PROTO_BROADCAST);
	real_head = &head->proto_head;

	real_head->msg_id = ENDION_FUNC_2(msg_id);
	real_head->seq = 0;
//	memcpy(real_head->data, msg_data, len);
	size_t len = 0;
	if (msg_data && packer)
	{
		len = packer(msg_data, (uint8_t *)real_head->data);
	}
	real_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + len);

	uint64_t *ppp = (uint64_t*)((char *)&head->player_id + len);
	head->num_player_id = 0;
	for (std::vector<uint64_t>::iterator iter = players.begin(); iter != players.end(); ++iter)
	{
		ppp[head->num_player_id++] = *iter;
	}
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD_CONN_BROADCAST) + len + sizeof(uint64_t) * head->num_player_id);
	if (conn_node_activitysrv::connecter.send_one_msg((PROTO_HEAD *)head, 1) != (int)(ENDION_FUNC_4(head->len)))
	{
		LOG_ERR("[%s:%d] send to conn_srv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
	return 0;
}

int conn_node_activitysrv::broadcast_message_to_all(uint16_t msg_id, void *msg_data, pack_func packer)
{
	PROTO_HEAD_CONN_BROADCAST *head;
	PROTO_HEAD *real_head;

	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
	head->msg_id = ENDION_FUNC_2(SERVER_PROTO_BROADCAST_ALL);
	real_head = &head->proto_head;

	real_head->msg_id = ENDION_FUNC_2(msg_id);
	real_head->seq = 0;
//	memcpy(real_head->data, msg_data, len);
	size_t len = 0;
	if (msg_data && packer)
	{
		len = packer(msg_data, (uint8_t *)real_head->data);
	}
	real_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + len);

	head->num_player_id = 0;
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD_CONN_BROADCAST) + len);
	if (conn_node_activitysrv::connecter.send_one_msg((PROTO_HEAD *)head, 1) != (int)(ENDION_FUNC_4(head->len)))
	{
		LOG_ERR("[%s:%d] send to conn_srv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
	return 0;
}

int conn_node_activitysrv::handle_player_online_notify(EXTERN_DATA *extern_data)
{
	do
	{
		ActivityPlayer *player = get_activity_player(extern_data->player_id);
		if (!player)
		{
			break;
		}

		notify_activity_info_to_client(player, extern_data);
	} while(0);
	
	return 0;
}

int conn_node_activitysrv::handle_gamesrv_reward_answer(EXTERN_DATA *extern_data)
{
	PROTO_SRV_REWARD_RES *res = (PROTO_SRV_REWARD_RES*)get_data();
	switch(res->statis_id)
	{
		case MAGIC_TYPE_ZHANLIDAREN_REWARD:
			handle_zhanlidaren_reward_answer(res->data_size, res->data, res->result, extern_data);
			break;
	}

	return 0;
}

int conn_node_activitysrv::handle_guildsrv_give_shidamenzong_reward_answer(EXTERN_DATA *extern_data)
{
	uint32_t *res = (uint32_t*)get_data();
	uint32_t activity_id = *res;

	ActivityInfo *activity = get_activity_info(activity_id);
	if (activity)
	{
		activity->data.shidamenzong.reward_time = 0;
		save_activity_info(activity);
	}

	return 0;
}

int conn_node_activitysrv::handle_zhanlidaren_get_reward_request(EXTERN_DATA *extern_data)
{
	ZhanlidarenGetRewardRequest *req = zhanlidaren_get_reward_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint32_t activity_id = req->activityid;
	uint32_t gift_id = req->id;
	zhanlidaren_get_reward_request__free_unpacked(req, NULL);

	int ret = 0;
	AutoReleaseBatchRedisPlayer t1;
	do
	{
		ActivityInfo *activity = get_activity_info(activity_id);
		if (!activity)
		{
			ret = 1;
			LOG_ERR("[%s:%d] player[%lu] get activity failed, activity_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, activity_id);
			break;
		}

		ActivityPlayer *player = get_activity_player(extern_data->player_id);
		if (!player)
		{
			ret = 1;
			LOG_ERR("[%s:%d] player[%lu] get player failed, activity_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, activity_id);
			break;
		}

		PlayerRedisInfo *redis_player = get_redis_player(extern_data->player_id, sg_player_key, sg_redis_client, t1);
		if (!redis_player)
		{
			ret = 1;
			LOG_ERR("[%s:%d] player[%lu] get redis player, activity_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, activity_id);
			break;
		}

		if (activity->state < TLAS_BEGIN || activity->state >= TLAS_END)
		{
			ret = 1;
			LOG_ERR("[%s:%d] player[%lu] activity is end, activity_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, activity_id);
			break;
		}

		PowerMasterTable *config = get_zhanlidaren_config(activity_id, gift_id);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] config miss, activity_id:%u, gift_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, activity_id, gift_id);
			break;
		}

		uint32_t got_num = get_zhanlidaren_gift_reward_num(activity, gift_id);
		if (got_num >= (uint32_t)config->RewardLimit)
		{
			ret = 2;
			LOG_ERR("[%s:%d] player[%lu] activity gift empty, activity_id:%u, gift_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, activity_id, gift_id);
			break;
		}
		
		if (redis_player->fighting_capacity < (uint32_t)config->PowerCondition)
		{
			ret = 3;
			LOG_ERR("[%s:%d] player[%lu] condition fail, activity_id:%u, gift_id:%u, need_fc:%u, player_fc:%u", __FUNCTION__, __LINE__, extern_data->player_id, activity_id, gift_id, (uint32_t)config->PowerCondition, redis_player->fighting_capacity);
			break;
		}

		if (get_player_zhanlidaren_gift_is_get(player, activity_id, gift_id))
		{
			ret = 4;
			LOG_ERR("[%s:%d] player[%lu] gift has get, activity_id:%u, gift_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, activity_id, gift_id);
			break;
		}

		add_zhanlidaren_gift_reward_num(activity, gift_id);
		mark_player_zhanlidaren_gift_get(player, activity_id, gift_id);

		{
			//请求发放奖励
			PROTO_SRV_REWARD_REQ *reward_req = (PROTO_SRV_REWARD_REQ *)get_send_data();
			uint32_t data_len = sizeof(PROTO_SRV_REWARD_REQ) + get_data_len();
			memset(reward_req, 0, data_len);
			reward_req->statis_id = MAGIC_TYPE_ZHANLIDAREN_REWARD;
			for (uint32_t i = 0; i < config->n_Reward && i < ARRAY_SIZE(reward_req->item_id); ++i)
			{
				reward_req->item_id[i] = config->Reward[i];
				reward_req->item_num[i] = config->RewardNum[i];
			}
			reward_req->data_size = get_data_len();
			memcpy(reward_req->data, get_data(), reward_req->data_size);
			fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_ACTIVITYSRV_REWARD_REQUEST, data_len, 0);
		}
	} while(0);

	if (ret != 0)
	{
		ZhanlidarenGetRewardAnswer resp;
		zhanlidaren_get_reward_answer__init(&resp);

		resp.result = ret;
		resp.activityid = activity_id;
		resp.id = gift_id;

		fast_send_msg(&connecter, extern_data, MSG_ID_ZHANLIDAREN_GET_REWARD_ANSWER, zhanlidaren_get_reward_answer__pack, resp);
	}

	return 0;
}

static int handle_zhanlidaren_reward_answer(int data_len, uint8_t *data, int result, EXTERN_DATA *extern_data)
{
	ZhanlidarenGetRewardRequest *req = zhanlidaren_get_reward_request__unpack(NULL, data_len, data);
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint32_t activity_id = req->activityid;
	uint32_t gift_id = req->id;
	zhanlidaren_get_reward_request__free_unpacked(req, NULL);

	if (result == 0)
	{
		ActivityInfo *activity = get_activity_info(activity_id);
		if (activity)
		{
			broadcast_zhanlidaren_gift_num_change(activity, gift_id);
		}
	}
	else
	{
		ActivityInfo *activity = get_activity_info(activity_id);
		if (activity)
		{
			sub_zhanlidaren_gift_reward_num(activity, gift_id);
		}

		ActivityPlayer *player = get_activity_player(extern_data->player_id);
		if (player)
		{
			unmark_player_zhanlidaren_gift_get(player, activity_id, gift_id);
		}
	}

	ZhanlidarenGetRewardAnswer resp;
	zhanlidaren_get_reward_answer__init(&resp);

	resp.result = result;
	resp.activityid = activity_id;
	resp.id = gift_id;

	fast_send_msg(&conn_node_activitysrv::connecter, extern_data, MSG_ID_ZHANLIDAREN_GET_REWARD_ANSWER, zhanlidaren_get_reward_answer__pack, resp);

	return 0;
}

