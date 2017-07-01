#ifndef _CONN_NODE_LOGINSRV_H__
#define _CONN_NODE_LOGINSRV_H__

#include "conn_node.h"
#include "ResMapTempete.h"
#include <string>


extern uint32_t sg_server_id;
extern uint32_t sg_max_conn;

extern std::string sg_server_key;
/// map<playerid, pair<day, ts> >
extern std::map<uint32_t, std::pair<uint32_t, uint32_t> > sg_filter_list;
extern long sg_filter_map_locker;


extern struct event sg_event_timer;
extern struct timeval sg_timeout;	
extern void cb_on_timer(evutil_socket_t, short, void *arg);


//extern std::set<uint32_t>  filterPlayerList;

class conn_node_loginsrv: public conn_node_base
{
public:
	conn_node_loginsrv();
	virtual ~conn_node_loginsrv();

	virtual int recv_func(evutil_socket_t fd);
	
	static conn_node_loginsrv connecter;
private:
	int handle_login(EXTERN_DATA *extern_data);
	int handle_create_player(EXTERN_DATA *extern_data);
	int handle_list_player(EXTERN_DATA *extern_data);
	int handle_delete_player(EXTERN_DATA *extern_data);
	int handle_enter_game(EXTERN_DATA *extern_data);
	int handle_get_player_info_request(EXTERN_DATA *extern_data);

	int select_player_base_info(uint32_t open_id, size_t *n_playerinfo, uint64_t player_id = 0);

private:
	int insert_user_to_map(int nChannel, const char* source, const char* szOpenId,  uint32_t* uOpenId = NULL);

	int get_user_info(EXTERN_DATA *extern_data, uint32_t oped_id, uint64_t player_id, uint32_t msgId, uint32_t& playerlv, uint32_t platform=0, uint32_t channel=0);

	bool check_open_id_exist(uint32_t open_id);
	int  get_player_count(uint32_t open_id);
	int check_can_create_player();
	int check_user_exist_map(uint32_t openid);
};



#endif

