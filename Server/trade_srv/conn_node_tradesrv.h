#ifndef _CONN_NODE_TRADESRV_H__ 
#define _CONN_NODE_TRADESRV_H__

#include "conn_node.h"
#include <vector>

typedef size_t (*pack_func)(const void *message, uint8_t *out);

class conn_node_tradesrv: public conn_node_base
{
	typedef int (conn_node_tradesrv::*handle_func)(EXTERN_DATA*);
	typedef std::map<uint32_t, handle_func> HandleMap;
public:
	conn_node_tradesrv();
	virtual ~conn_node_tradesrv();

	void add_msg_handle(uint32_t msg_id, handle_func func);

	virtual int recv_func(evutil_socket_t fd);

	static int broadcast_message(uint16_t msg_id, void *msg_data, pack_func packer, std::vector<uint64_t> &players);
	static void send_to_all_player(uint16_t msg_id, void *data, pack_func func);
	
	static conn_node_tradesrv connecter;
private:

	int handle_player_online_notify(EXTERN_DATA *extern_data);
	int handle_check_and_cost_answer(EXTERN_DATA *extern_data);
	int handle_gamesrv_reward_answer(EXTERN_DATA *extern_data);
	int handle_gamesrv_start(EXTERN_DATA *extern_data);

	int handle_trade_item_summary_request(EXTERN_DATA *extern_data);
	int handle_trade_item_detail_request(EXTERN_DATA *extern_data);
	int handle_trade_on_shelf_request(EXTERN_DATA *extern_data);
	int handle_trade_on_shelf_delete_item_answer(EXTERN_DATA *extern_data);
	int handle_trade_off_shelf_request(EXTERN_DATA *extern_data);
	int handle_trade_off_shelf_add_item_answer(EXTERN_DATA *extern_data); //from game_srv
	int handle_trade_reshelf_request(EXTERN_DATA *extern_data);
	int handle_trade_reshelf_change_answer(EXTERN_DATA *extern_data);
	int handle_trade_enlarge_shelf_request(EXTERN_DATA *extern_data);
	int handle_trade_buy_request(EXTERN_DATA *extern_data);
	int handle_trade_buy_execute_answer(EXTERN_DATA *extern_data);
	int handle_trade_get_earning_request(EXTERN_DATA *extern_data);

	int handle_auction_lot_insert_request(EXTERN_DATA *extern_data);
	int handle_auction_info_request(EXTERN_DATA *extern_data);
	int handle_auction_bid_request(EXTERN_DATA *extern_data);
	int handle_auction_buy_now_request(EXTERN_DATA *extern_data);

	int handle_red_packet_send_red_packet_request(EXTERN_DATA *extern_data);
	int handle_red_packet_main_jiemian_info_request(EXTERN_DATA *extern_data);
	int handle_red_packet_detalled_info_request(EXTERN_DATA *extern_data);
	int handle_red_packet_grab_red_packet_request(EXTERN_DATA *extern_data);
	int handle_red_packet_recive_record_request(EXTERN_DATA *extern_data);


	int record_player_recive_red_packet_info(uint64_t player_id, uint64_t red_uuid, uint32_t money_type, uint32_t money_num, bool is_send_red_packet);
	int modify_player_red_packet_optimum_record(uint64_t player_id, uint64_t red_uuid);

private:
	HandleMap   m_handleMap;
};



#endif

