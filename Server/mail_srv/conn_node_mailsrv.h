#ifndef _CONN_NODE_MAILSRV_H__ 
#define _CONN_NODE_MAILSRV_H__

#include "conn_node.h"

class conn_node_mailsrv: public conn_node_base
{
	typedef int (conn_node_mailsrv::*handle_func)(EXTERN_DATA*);
	typedef std::map<uint32_t, handle_func> HandleMap;
public:
	conn_node_mailsrv();
	virtual ~conn_node_mailsrv();

	void add_msg_handle(uint32_t msg_id, handle_func func);

	virtual int recv_func(evutil_socket_t fd);

	int clear_mail_state(EXTERN_DATA *extern_data); //清除邮件的领取中状态
	
	static conn_node_mailsrv connecter;
private:
	int handle_mail_insert(EXTERN_DATA *extern_data);
	int handle_mail_list_request(EXTERN_DATA *extern_data);
	int handle_mail_read_request(EXTERN_DATA *extern_data);
	int handle_mail_get_attach_request(EXTERN_DATA *extern_data);
	int handle_mail_give_attach_answer(EXTERN_DATA *extern_data);
	int handle_mail_del_request(EXTERN_DATA *extern_data);

private:
	HandleMap   m_handleMap;
};



#endif

