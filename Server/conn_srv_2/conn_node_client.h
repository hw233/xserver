#ifndef _CONN_NODE_CLIENT_H__
#define _CONN_NODE_CLIENT_H__

#include "conn_node.h"
#include "game_event.h"
#include "websocket.h"

#define MAX_CLIENT_SEND_BUFFER_SIZE (1024*256)

class conn_node_client: public conn_node_base
{
public:
	conn_node_client();
	virtual ~conn_node_client();

	virtual int recv_func(evutil_socket_t fd);
	virtual int send_one_msg(PROTO_HEAD *head, uint8_t force);
	virtual struct event* get_write_event() { return &ev_write; }

	static std::map<evutil_socket_t, conn_node_client *> map_fd_nodes;
	static std::map<uint64_t, conn_node_client *> map_player_id_nodes;
	static std::map<uint32_t, conn_node_client *> map_open_id_nodes;	
	
	static int add_map_fd_nodes(conn_node_client *client);
	static int add_map_player_id_nodes(conn_node_client *client);
	static int add_map_open_id_nodes(conn_node_client *client);		
	static conn_node_client * get_nodes_by_fd(evutil_socket_t fd, uint16_t port);
	static conn_node_client * get_nodes_by_player_id(uint64_t player_id);
	static conn_node_client * get_nodes_by_open_id(uint32_t open_id);	


public:
	void send_data_to_client();


public:
	uint16_t login_seq;  //登录的seq号，登录包返回的时候比较这个seq，不一致则丢弃
	uint16_t seq;       //客户端发包的seq号，每次加1
	uint32_t open_id;
	uint64_t player_id;
	int raidsrv_id;      //在raid_srv的ID,负数表示在gamesrv
	struct event ev_write;

		//websocket
	bool handshake;
	uint8_t conn_step;
	frame_t frame;

private:	
	char send_buffer[MAX_CLIENT_SEND_BUFFER_SIZE];
	int32_t send_buffer_begin_pos;
	int32_t send_buffer_end_pos;

private:
	void memmove_data();
	void remove_buflen(int len);

	int send_one_buffer(char *buffer, uint32_t len);	
	int send_one_buffer(uint8_t fin, uint8_t opcode, char *buffer, uint32_t len);
	int respond_websocket_request();
	int recv_from_fd();
	int recv_handshake(evutil_socket_t fd);
	int frame_read_cb(evutil_socket_t fd);
	int decode_and_check_crc(PROTO_HEAD *head);
	int dispatch_message();
	int transfer_to_gameserver();
	int transfer_to_raidserver();	
	int transfer_to_loginserver();
	int transfer_to_dumpserver(PROTO_HEAD* head);
	int transfer_to_mailsrv();
	int transfer_to_friendsrv();		
	int transfer_to_guildsrv();
	int transfer_to_doufachang();	
	int transfer_to_ranksrv();		
	int transfer_to_tradesrv();	
	int transfer_to_activitysrv();	
	int send_player_exit(bool again = false);
	int send_hello_resp();	
};

#endif
