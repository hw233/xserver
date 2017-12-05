#include "game_srv.h"
#include "conn_node_gamesrv.h"
#include "conn_node_dbsrv.h"
//#include "player_manager.h"
#include "time_helper.h"
#include "uuid.h"
#include "game_event.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <queue>
#include <sstream>
#include <math.h>
#include "../proto/msgid.h"
#include "../proto/cast_skill.pb-c.h"
#include "../proto/move.pb-c.h"
#include "../proto/chat.pb-c.h"

conn_node_gamesrv conn_node_gamesrv::connecter;
//char conn_node_gamesrv::global_send_buf[1024 * 64];

conn_node_gamesrv::conn_node_gamesrv()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);
}

conn_node_gamesrv::~conn_node_gamesrv()
{
}

void conn_node_gamesrv::send_to_all_player(uint16_t msg_id, void *data, pack_func func)
{
	PROTO_HEAD* proto_head;
	PROTO_HEAD_CONN_BROADCAST *broadcast_head;
	broadcast_head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
	broadcast_head->msg_id = ENDION_FUNC_2(SERVER_PROTO_BROADCAST_ALL);	
	proto_head = &broadcast_head->proto_head;
	proto_head->msg_id = ENDION_FUNC_2(msg_id);
	proto_head->seq=0;
	size_t size = func(data, (uint8_t *)proto_head->data);
	proto_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);
	broadcast_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD_CONN_BROADCAST) + size);

	if (connecter.send_one_msg((PROTO_HEAD *)broadcast_head, 1) != (int)(ENDION_FUNC_4(broadcast_head->len))) {	
		LOG_ERR("%s %d: send to all failed err[%d]", __FUNCTION__, __LINE__, errno); 
	}																	
}

uint64_t *conn_node_gamesrv::prepare_broadcast_msg_to_players(uint32_t msg_id, void *msg_data, pack_func func)
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
	if (msg_data && func)
	{
		len = func(msg_data, (uint8_t *)real_head->data);
	}
	real_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + len);

	uint64_t *ppp = (uint64_t*)((char *)&head->player_id + len);
	head->num_player_id = 0;
	head->len = sizeof(PROTO_HEAD_CONN_BROADCAST) + len;
	return ppp;
}
uint64_t *conn_node_gamesrv::broadcast_msg_add_players(uint64_t player_id, uint64_t *ppp)
{
	if (get_entity_type(player_id) == ENTITY_TYPE_AI_PLAYER)
		return ppp;
	PROTO_HEAD_CONN_BROADCAST *head;	
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;	
	ppp[head->num_player_id++] = player_id;
	head->len += sizeof(uint64_t);
	return ppp;
}
int conn_node_gamesrv::broadcast_msg_send()
{
	PROTO_HEAD_CONN_BROADCAST *head;	
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;

	if (head->num_player_id == 0)
		return 0;
	
	head->len = ENDION_FUNC_4(head->len);
//	LOG_DEBUG("%s %d: broad [%u] to %d player", __FUNCTION__, __LINE__, ENDION_FUNC_2(head->proto_head.msg_id), head->num_player_id);
	if (conn_node_gamesrv::connecter.send_one_msg((PROTO_HEAD *)head, 1) != (int)(ENDION_FUNC_4(head->len))) {
		LOG_ERR("%s %d: send to all failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
	return (0);
}

#if 0
#define MAX_PLAYER_INFO_EACH_TIME 32
void conn_node_gamesrv::broadcast_msg_to_players(uint32_t msg_id, uint8_t *data, int len, std::vector<uint64_t>& playerlist)
{
	if (playerlist.size() == 0) {
		return ;
	}

	PROTO_HEAD_CONN_BROADCAST *head;
	PROTO_HEAD *real_head;

	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
	head->msg_id = ENDION_FUNC_2(SERVER_PROTO_BROADCAST);
	real_head = &head->proto_head;

	real_head->msg_id = ENDION_FUNC_2(msg_id);
	real_head->seq = 0;
	memcpy(real_head->data, data, len);
	real_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + len);

	std::vector<uint64_t>::const_iterator it=playerlist.begin();	
	uint32_t iPc = 0;

	uint64_t *ppp = (uint64_t*)((char *)&head->player_id + len);
	for (; it!=playerlist.end(); ++it) {
		ppp[iPc++] = *it;

		if (MAX_PLAYER_INFO_EACH_TIME == iPc) {
			head->num_player_id = iPc;
			head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD_CONN_BROADCAST) + len + sizeof(uint64_t) * head->num_player_id);

			LOG_DEBUG("%s %d: broad to %d player", __FUNCTION__, __LINE__, head->num_player_id);

			if (conn_node_gamesrv::connecter.send_one_msg((PROTO_HEAD *)head, 1) != (int)(ENDION_FUNC_4(head->len))) {
				LOG_ERR("%s %d: send to all failed err[%d]", __FUNCTION__, __LINE__, errno);
			}
			iPc=0;
		}
	}

	if (iPc > 0) {
		head->num_player_id = iPc;
		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD_CONN_BROADCAST) + len + sizeof(uint64_t) * head->num_player_id);
		LOG_DEBUG("%s %d: broad to %d player", __FUNCTION__, __LINE__, head->num_player_id);

		if (conn_node_gamesrv::connecter.send_one_msg((PROTO_HEAD *)head, 1) != (int)(ENDION_FUNC_4(head->len))) {
			LOG_ERR("%s %d: send to all failed err[%d]", __FUNCTION__, __LINE__, errno);
		}
	}
}
#endif

int conn_node_gamesrv::recv_func(evutil_socket_t fd)
{
	if (g_game_recv_func)
		return g_game_recv_func(fd, this);
	return (0);
}

void conn_node_gamesrv::send_to_friend(EXTERN_DATA *extern_data, uint16_t msg_id, void *data, pack_func func)
{
	PROTO_HEAD *proto_head;
	PROTO_HEAD *real_head;
	proto_head = (PROTO_HEAD *)conn_node_base::global_send_buf;
	proto_head->msg_id = ENDION_FUNC_2(SERVER_PROTO_GAME_TO_FRIEND);
	proto_head->seq = 0;

	real_head = (PROTO_HEAD *)proto_head->data;
	size_t size = func(data, (uint8_t *)real_head->data);
	real_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);
	real_head->msg_id = ENDION_FUNC_2(msg_id);
	connecter.add_extern_data(real_head, extern_data);

	proto_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + real_head->len);

	if (connecter.send_one_msg(proto_head, 1) != (int)(ENDION_FUNC_4(proto_head->len))) {
		LOG_ERR("%s %d: send to all failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
}

void conn_node_gamesrv::notify_gamesrv_start(void)
{
	EXTERN_DATA ext_data;
	fast_send_msg_base(&connecter, &ext_data, SERVER_PROTO_GAMESRV_START, 0, 0);
}

int conn_node_gamesrv::transfer_to_dbsrv(void)
{
	int ret = 0;
	PROTO_HEAD *head = get_head();
	if (conn_node_dbsrv::connecter.send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len))
	{
		LOG_ERR("[%s:%d] send to dbsrv failed err[%d]", __FUNCTION__, __LINE__, errno);
		ret = -2;
		goto done;
	}

done:
	return (ret);
}

int conn_node_gamesrv::transfer_to_connsrv(void)
{
	int ret = 0;
	PROTO_HEAD *head = get_head();
	if (conn_node_gamesrv::connecter.send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len))
	{
		LOG_ERR("[%s:%d] send to connsrv failed err[%d]", __FUNCTION__, __LINE__, errno);
		ret = -2;
		goto done;
	}

done:
	return (ret);
}

