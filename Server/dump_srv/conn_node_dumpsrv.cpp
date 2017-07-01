#include "conn_node_dumpsrv.h"
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
#include "mysql_module.h"
#include "server_proto.h"
#include <openssl/md5.h>
//#include <curl/curl.h>
#include <vector>
//#include "app_data_statis.h"
//#include "ChatType.h"
#include "stl_relation.h"
#include "msgid.h"
#include "error_code.h"
#include "attr_id.h"
//#include "cgi_common.h"
#include "comm_message.pb-c.h"
#include "login.pb-c.h"
#include "player_db.pb-c.h"


conn_node_dumpsrv conn_node_dumpsrv::connecter;
//static char sql[1024*64];

uint32_t sg_server_id = 0;

#define PROTO_STATIS_INFO_COMMIT3(magicId, num1, ullPlayerid)     \
	do {  \
	PROTO_STATIS_INFO* proto = (PROTO_STATIS_INFO*)get_send_buf(SERVER_PROTO_STATIS_INFO, 0);  \
	proto_statis_info__init(proto, COMMIT_MYSQL, GAME_APPID, extern_data->open_id, ullPlayerid, sg_server_id, (uint32_t)time(NULL)  \
	, magicId, num1, 0, 0, 0, 0);  \
	proto->head.len = htons(sizeof(PROTO_STATIS_INFO)); \
	add_extern_data((PROTO_HEAD*)proto, extern_data);  \
	int result  = connecter.send_one_msg((PROTO_HEAD*)proto, 1); \
	if (result != htons(proto->head.len)) {  \
	LOG_ERR("%s: send to client failed err[%d]", __FUNCTION__, errno); \
	}  \
	} while(false)  \


conn_node_dumpsrv::conn_node_dumpsrv()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);
}

conn_node_dumpsrv::~conn_node_dumpsrv()
{
}

int conn_node_dumpsrv::recv_func(evutil_socket_t fd)
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

