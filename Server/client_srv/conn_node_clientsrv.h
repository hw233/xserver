#ifndef _CONN_NODE_CLIENT_H__ 
#define _CONN_NODE_CLIENT_H__

#include "conn_node.h"
#include <string>
#include <vector>
#include "attr_id.h"
#include "../proto/move.pb-c.h"

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

//void client_on_timer(evutil_socket_t, short, void* /*arg*/);
struct player_struct
{
	uint64_t player_id;
	char name[MAX_PLAYER_NAME_LEN + 1];    //名字
	uint32_t scene_id;
	double attrData[PLAYER_ATTR_MAX]; //属性
};

class conn_node_clientsrv : public conn_node_base
{
//	friend int main(int argc, char **argv);
public:
	conn_node_clientsrv();
	virtual ~conn_node_clientsrv();

	virtual int recv_func(evutil_socket_t fd);

	static conn_node_clientsrv connecter;

public:
	uint32_t seq;
	uint32_t open_id;
	uint64_t player_id;
	uint32_t job; //职业
	float posx;	//玩家当前x坐标
	float posz;	//玩家当前z坐标
	uint32_t move_lag; //移动标记
	uint32_t skillid; //当前使用技能ID
	uint32_t chat_flag;	//传送出新手副本标志
	int first_use_skill;	//第一次使用技能的标志
	int only_one_move;		//仅在出生时移动一次，防止太多人站在一起
	int first_borth;		//第一次创建角色
	int pk_modle;			//设置pk模式
	uint32_t interval_time;	//间隔时间

//private:
	int send_login_request(uint32_t open_id);
	int send_player_list_request(void);
	int send_player_create_request(uint32_t job, std::string name);
	int send_enter_game_request(uint64_t player_id);
	int send_move_request();
	int send_chat_info_request();
	int handle_player_useskill_request();
	int handle_player_setpkmodle_request();
	int handle_player_mingzhong_request();
	int borth_send_move_request();

	int handle_login_answer(void);
	int handle_player_list_answer(void);
	int handle_player_create_answer(void);
	int handle_enter_game_answer(void);
	int handle_player_move_answer(void);
	int handle_player_useskill_answer();
	int send_chat_info_answer();
	int handle_player_sight_changed_answer();
	int handle_player_shill_mingzhong_answer();
private:
	std::map<uint64_t, SightPlayerBaseInfo*> sight_player;	//视野类玩家


};
#endif

