#ifndef _CONN_NODE_DBSRV_H__
#define _CONN_NODE_DBSRV_H__

#include "conn_node.h"
//处理和db服务器连接的对象
class conn_node_dbsrv: public conn_node_base
{
public:
	conn_node_dbsrv();
	virtual ~conn_node_dbsrv();

	virtual int recv_func(evutil_socket_t fd);
	
	static conn_node_dbsrv *connecter;
//	static char send_buf[1024];

	int transfer_to_connsrv(void);

};



#endif
