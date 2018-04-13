#include "conn_node_tradesrv.h"
#include "game_event.h"
#include "msgid.h"
#include "error_code.h"
#include "mysql_module.h"
#include "time_helper.h"
#include <vector>
#include <set>
#include "redis_util.h"
#include "app_data_statis.h"
#include <algorithm>
#include "send_mail.h"
#include "trade_util.h"
#include "trade.pb-c.h"
#include "comm_message.pb-c.h"
#include "send_mail.h"
#include <stdlib.h>
#include "trade_db.pb-c.h"
#include<time.h>
#include<sys/time.h>
#include "chat.pb-c.h"


conn_node_tradesrv conn_node_tradesrv::connecter;
//static char sql[1024];

void notify_trade_info(TradePlayer *player, EXTERN_DATA *extern_data);
static int handle_trade_enlarge_shelf_cost_answer(int data_len, uint8_t *data, int result, EXTERN_DATA *extern_data);
static int handle_auction_bid_cost_answer(int data_len, uint8_t *data, int result, EXTERN_DATA *extern_data);
static int handle_auction_buy_now_cost_answer(int data_len, uint8_t *data, int result, EXTERN_DATA *extern_data);

conn_node_tradesrv::conn_node_tradesrv()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);
	
	add_msg_handle(SERVER_PROTO_PLAYER_ONLINE_NOTIFY, &conn_node_tradesrv::handle_player_online_notify);
	add_msg_handle(SERVER_PROTO_TRADESRV_COST_ANSWER, &conn_node_tradesrv::handle_check_and_cost_answer);
//	add_msg_handle(SERVER_PROTO_GUILDSRV_REWARD_ANSWER, &conn_node_tradesrv::handle_gamesrv_reward_answer);
	add_msg_handle(SERVER_PROTO_GAMESRV_START, &conn_node_tradesrv::handle_gamesrv_start);

	add_msg_handle(MSG_ID_TRADE_ITEM_SUMMARY_REQUEST, &conn_node_tradesrv::handle_trade_item_summary_request);
	add_msg_handle(MSG_ID_TRADE_ITEM_DETAIL_REQUEST, &conn_node_tradesrv::handle_trade_item_detail_request);
	add_msg_handle(SERVER_PROTO_TRADE_ON_SHELF_REQUEST, &conn_node_tradesrv::handle_trade_on_shelf_request);
	add_msg_handle(SERVER_PROTO_TRADE_ON_SHELF_DELETE_ITEM_ANSWER, &conn_node_tradesrv::handle_trade_on_shelf_delete_item_answer);
	add_msg_handle(MSG_ID_TRADE_OFF_SHELF_REQUEST, &conn_node_tradesrv::handle_trade_off_shelf_request);
	add_msg_handle(SERVER_PROTO_TRADE_OFF_SHELF_ADD_ITEM_ANSWER, &conn_node_tradesrv::handle_trade_off_shelf_add_item_answer);
	add_msg_handle(MSG_ID_TRADE_RESHELF_REQUEST, &conn_node_tradesrv::handle_trade_reshelf_request);
	add_msg_handle(SERVER_PROTO_TRADE_RE_SHELF_CHANGE_ANSWER, &conn_node_tradesrv::handle_trade_reshelf_change_answer);
	add_msg_handle(MSG_ID_TRADE_ENLARGE_SHELF_REQUEST, &conn_node_tradesrv::handle_trade_enlarge_shelf_request);
	add_msg_handle(MSG_ID_TRADE_BUY_REQUEST, &conn_node_tradesrv::handle_trade_buy_request);
	add_msg_handle(SERVER_PROTO_TRADE_BUY_EXECUTE_ANSWER, &conn_node_tradesrv::handle_trade_buy_execute_answer);
	add_msg_handle(MSG_ID_TRADE_GET_EARNING_REQUEST, &conn_node_tradesrv::handle_trade_get_earning_request);

	add_msg_handle(SERVER_PROTO_TRADE_LOT_INSERT, &conn_node_tradesrv::handle_auction_lot_insert_request);
	add_msg_handle(MSG_ID_AUCTION_INFO_REQUEST, &conn_node_tradesrv::handle_auction_info_request);
	add_msg_handle(MSG_ID_AUCTION_BID_REQUEST, &conn_node_tradesrv::handle_auction_bid_request);
	add_msg_handle(MSG_ID_AUCTION_BUY_NOW_REQUEST, &conn_node_tradesrv::handle_auction_buy_now_request);

	add_msg_handle(SERVER_PROTO_TRADE_SEND_RED_PACKET_REQUEST, &conn_node_tradesrv::handle_red_packet_send_red_packet_request);
	add_msg_handle(MSG_ID_RED_BACKET_MAIN_JIEMAIN_INFO_REQUEST, &conn_node_tradesrv::handle_red_packet_main_jiemian_info_request);
	add_msg_handle(MSG_ID_RED_BACKET_DETAILED_INFO_REQUEST, &conn_node_tradesrv::handle_red_packet_detalled_info_request);
	add_msg_handle(MSG_ID_RED_BACKET_QIANG_HONGBAO_REQUEST, &conn_node_tradesrv::handle_red_packet_grab_red_packet_request);
	add_msg_handle(MSG_ID_RED_BACKET_HISTORY_INFO_REQUEST, &conn_node_tradesrv::handle_red_packet_recive_record_request);
}

conn_node_tradesrv::~conn_node_tradesrv()
{
}

void conn_node_tradesrv::add_msg_handle(uint32_t msg_id, handle_func func)
{
	connecter.m_handleMap[msg_id] = func;
}

int conn_node_tradesrv::recv_func(evutil_socket_t fd)
{
	EXTERN_DATA *extern_data;
	PROTO_HEAD *head;	
	for (;;) {
		int ret = get_one_buf();
		if (ret == 0) {
			head = (PROTO_HEAD *)buf_head();
			int cmd = get_cmd();
			uint64_t times = time_helper::get_micro_time();
			time_helper::set_cached_time(times / 1000);
			switch (cmd)
			{
				case SERVER_PROTO_GAMESRV_START:
				case SERVER_PROTO_TRADE_LOT_INSERT:
					{
						HandleMap::iterator iter = m_handleMap.find(cmd);
						if (iter != m_handleMap.end())
						{
							(this->*(iter->second))(NULL);
						}
					}
					break;
				default:
					{
						extern_data = get_extern_data(head);
						LOG_DEBUG("[%s:%d] cmd: %u, player_id: %lu", __FUNCTION__, __LINE__, cmd, extern_data->player_id);

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
					break;
			}
		}
		
		if (ret < 0) {
			LOG_INFO("%s: connect closed from fd %u, err = %d", __FUNCTION__, fd, errno);
			clear_module_memory();
			exit(0);
			return (-1);		
		} else if (ret > 0) {
			break;
		}
		
		ret = remove_one_buf();
	}
	return (0);
}

int conn_node_tradesrv::broadcast_message(uint16_t msg_id, void *msg_data, pack_func packer, std::vector<uint64_t> &players)
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
	if (conn_node_tradesrv::connecter.send_one_msg((PROTO_HEAD *)head, 1) != (int)(ENDION_FUNC_4(head->len)))
	{
		LOG_ERR("[%s:%d] send to conn_srv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
	return 0;
}

void conn_node_tradesrv::send_to_all_player(uint16_t msg_id, void *data, pack_func func)
{
	PROTO_HEAD* proto_head;
	PROTO_HEAD_CONN_BROADCAST *broadcast_head;
	broadcast_head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
	broadcast_head->msg_id = ENDION_FUNC_2(SERVER_PROTO_BROADCAST_ALL);
	proto_head = &broadcast_head->proto_head;
	proto_head->msg_id = ENDION_FUNC_2(msg_id);
	proto_head->seq = 0;
	size_t size = func(data, (uint8_t *)proto_head->data);
	proto_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);
	broadcast_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD_CONN_BROADCAST) + size);

	if (connecter.send_one_msg((PROTO_HEAD *)broadcast_head, 1) != (int)(ENDION_FUNC_4(broadcast_head->len))) {
		LOG_ERR("%s %d: send to all failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
}



int conn_node_tradesrv::handle_player_online_notify(EXTERN_DATA *extern_data)
{
	TradePlayer *player = get_trade_player(extern_data->player_id);
	if (!player)
	{
		LOG_ERR("[%s:%d] get player[%lu] failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	notify_trade_info(player, extern_data);
	if (player->auction_bid_money > 0)
	{
		send_player_auction_bid_fail(player->player_id, player->auction_bid_money);
		player->auction_bid_money = 0;
		save_trade_player(player);
	}

	return 0;
}

int conn_node_tradesrv::handle_check_and_cost_answer(EXTERN_DATA *extern_data)
{
	PROTO_SRV_CHECK_AND_COST_RES *res = (PROTO_SRV_CHECK_AND_COST_RES*)get_data();
	int ret = 0;
	switch(res->cost.statis_id)
	{
		case MAGIC_TYPE_TRADE_ENLARGE_SHELF:
			ret = handle_trade_enlarge_shelf_cost_answer(res->data_size, res->data, res->result, extern_data);
			break;
		case MAGIC_TYPE_AUCTION_BID:
			ret = handle_auction_bid_cost_answer(res->data_size, res->data, res->result, extern_data);
			break;
		case MAGIC_TYPE_AUCTION_BUY_NOW:
			ret = handle_auction_buy_now_cost_answer(res->data_size, res->data, res->result, extern_data);
			break;
	}

	if (ret != 0)
	{
		PROTO_UNDO_COST *proto = (PROTO_UNDO_COST*)get_send_data();
		uint32_t data_len = sizeof(PROTO_UNDO_COST);
		memset(proto, 0, data_len);
		memcpy(&proto->cost, &res->cost, sizeof(SRV_COST_INFO));
		fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_UNDO_COST, data_len, 0);
	}

	return 0;
}

int conn_node_tradesrv::handle_gamesrv_reward_answer(EXTERN_DATA *extern_data)
{
	PROTO_SRV_REWARD_RES *res = (PROTO_SRV_REWARD_RES*)get_data();
	switch(res->statis_id)
	{
//		case MAGIC_TYPE_SHOP_BUY:
//			handle_shop_buy_answer(res->data_size, res->data, res->result, extern_data);
//			break;
	}

	return 0;
}


int conn_node_tradesrv::handle_gamesrv_start(EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] game_srv start notify.", __FUNCTION__, __LINE__);

	return 0;
}

void notify_trade_info(TradePlayer *player, EXTERN_DATA *extern_data)
{
	TradeInfoNotify nty;
	trade_info_notify__init(&nty);

	TradeShelfData  shelf_data[MAX_TRADE_SHELF_NUM];
	TradeShelfData* shelf_point[MAX_TRADE_SHELF_NUM];
	TradeSoldData  sold_data[MAX_TRADE_SOLD_NUM];
	TradeSoldData* sold_point[MAX_TRADE_SOLD_NUM];

	nty.shelflist = shelf_point;
	nty.n_shelflist = 0;
	for (uint32_t i = 0; i < MAX_TRADE_SHELF_NUM && i < player->shelf_num; ++i)
	{
		if (player->shelf_items[i] == NULL)
		{
			continue;
		}

		shelf_point[nty.n_shelflist] = &shelf_data[nty.n_shelflist];
		trade_shelf_data__init(&shelf_data[nty.n_shelflist]);
		shelf_data[nty.n_shelflist].index = i;
		shelf_data[nty.n_shelflist].itemid = player->shelf_items[i]->item_id;
		shelf_data[nty.n_shelflist].num = player->shelf_items[i]->num;
		shelf_data[nty.n_shelflist].price = player->shelf_items[i]->price;
		shelf_data[nty.n_shelflist].state = get_trade_state(player->shelf_items[i]->state);
		shelf_data[nty.n_shelflist].time = player->shelf_items[i]->time;
		nty.n_shelflist++;
	}
	nty.shelfnum = player->shelf_num;
	nty.soldlist = sold_point;
	nty.n_soldlist = 0;
	for (int i = 0; i < MAX_TRADE_SOLD_NUM; ++i)
	{
		if (player->sold_items[i].item_id == 0)
		{
			break;
		}

		sold_point[nty.n_soldlist] = &sold_data[nty.n_soldlist];
		trade_sold_data__init(&sold_data[nty.n_soldlist]);
		sold_data[nty.n_soldlist].itemid = player->sold_items[i].item_id;
		sold_data[nty.n_soldlist].num = player->sold_items[i].num;
		sold_data[nty.n_soldlist].price = player->sold_items[i].price;
		nty.n_soldlist++;
	}
	nty.soldearning = player->sold_earning;

	fast_send_msg(&conn_node_tradesrv::connecter, extern_data, MSG_ID_TRADE_INFO_NOTIFY, trade_info_notify__pack, nty);
}

#define MAX_TRADE_ITEM_TYPE 1000
int conn_node_tradesrv::handle_trade_item_summary_request(EXTERN_DATA *extern_data)
{
	TradeItemSummaryAnswer resp;
	trade_item_summary_answer__init(&resp);

	TradeItemSummaryData  summary_data[MAX_TRADE_ITEM_TYPE];
	TradeItemSummaryData* summary_point[MAX_TRADE_ITEM_TYPE];

	resp.result = 0;
	resp.itemlist = summary_point;
	resp.n_itemlist = 0;
	for (std::map<uint64_t, TradingTable *>::iterator iter = trade_item_config.begin(); iter != trade_item_config.end() && resp.n_itemlist < MAX_TRADE_ITEM_TYPE; ++iter)
	{
		TradingTable *config = iter->second;

		summary_point[resp.n_itemlist] = &summary_data[resp.n_itemlist];
		trade_item_summary_data__init(&summary_data[resp.n_itemlist]);
		summary_data[resp.n_itemlist].itemid = config->ID;
		summary_data[resp.n_itemlist].onsellnum = 0;
		std::pair<TradeItemMap::iterator, TradeItemMap::iterator> range = trade_item_map.equal_range(config->ID);
		for (TradeItemMap::iterator iter = range.first; iter != range.second; ++iter)
		{
			TradeItem *item = iter->second;
			if (get_trade_state(item->state) == Trade_State_Sell)
			{
				summary_data[resp.n_itemlist].onsellnum += item->num;
			}
		}

		summary_data[resp.n_itemlist].averageprice = get_trade_item_average_price(config->ID);
		if (summary_data[resp.n_itemlist].averageprice == 0)
		{
			summary_data[resp.n_itemlist].averageprice = config->GuidePrice;
		}
		resp.n_itemlist++;
	}

	fast_send_msg(&conn_node_tradesrv::connecter, extern_data, MSG_ID_TRADE_ITEM_SUMMARY_ANSWER, trade_item_summary_answer__pack, resp);

	return 0;
}

#define MAX_TRADE_SELLER_NUM 10000
int conn_node_tradesrv::handle_trade_item_detail_request(EXTERN_DATA *extern_data)
{
	TradeItemDetailRequest *req = trade_item_detail_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint32_t item_id = req->itemid;
	trade_item_detail_request__free_unpacked(req, NULL);

	TradeItemDetailAnswer resp;
	trade_item_detail_answer__init(&resp);

	TradeSellerData  seller_data[MAX_TRADE_SELLER_NUM];
	TradeSellerData* seller_point[MAX_TRADE_SELLER_NUM];

	resp.result = 0;
	resp.itemid = item_id;
	resp.sellers = seller_point;
	resp.n_sellers = 0;

	std::pair<TradeItemMap::iterator, TradeItemMap::iterator> range = trade_item_map.equal_range(item_id);
	for (TradeItemMap::iterator iter = range.first; iter != range.second && resp.n_sellers < MAX_TRADE_SELLER_NUM; ++iter)
	{
		TradeItem *item = iter->second;
		if (get_trade_state(item->state) != Trade_State_Sell)
		{
			continue;
		}

		seller_point[resp.n_sellers] = &seller_data[resp.n_sellers];
		trade_seller_data__init(&seller_data[resp.n_sellers]);
		seller_data[resp.n_sellers].playerid = item->player_id;
		seller_data[resp.n_sellers].shelfindex = item->shelf_index;
		seller_data[resp.n_sellers].num = item->num;
		seller_data[resp.n_sellers].price = item->price;
		resp.n_sellers++;
	}

	fast_send_msg(&conn_node_tradesrv::connecter, extern_data, MSG_ID_TRADE_ITEM_DETAIL_ANSWER, trade_item_detail_answer__pack, resp);

	return 0;
}

int conn_node_tradesrv::handle_trade_on_shelf_request(EXTERN_DATA *extern_data)
{
	TRADE_ON_SHELF_REQUEST *req = (TRADE_ON_SHELF_REQUEST *)get_data();

	int ret = 0;
	TradePlayer *player = NULL;
	do
	{
		player = get_trade_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] get player[%lu] failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		ret = check_trade_item_price_rational(req->trade_id, req->price);
		if (ret != 0)
		{
			ret = ERROR_ID_TRADE_PRICE;
			LOG_ERR("[%s:%d] player[%lu] price error, item_id:%u, price:%u", __FUNCTION__, __LINE__, extern_data->player_id, req->trade_id, req->price);
			break;
		}

		int free_index = -1;
		for (uint32_t i = 0; i < MAX_TRADE_SHELF_NUM && i < player->shelf_num; ++i)
		{
			if (player->shelf_items[i] == NULL)
			{
				free_index = i;
				break;
			}
		}

		if (free_index < 0)
		{
			ret = 190500433;
			LOG_ERR("[%s:%d] player[%lu] shelf space, trade_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, req->trade_id);
			break;
		}

		TradeItem *item = create_trade_item(player, free_index, req->trade_id, req->num, req->price, &req->especial);
		if (!item)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] create trade item failed, trade_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, req->trade_id);
			break;
		}

		set_trade_operate(item->state, Trade_Operate_On_Shelf);
		save_trade_item(item);
		save_trade_player(player); //save player so we can put item data into it when load db;
		{
			TRADE_ON_SHELF_DELETE_ITEM_REQUEST *del_req = (TRADE_ON_SHELF_DELETE_ITEM_REQUEST *)get_send_data();
			memset(del_req, 0, sizeof(TRADE_ON_SHELF_DELETE_ITEM_REQUEST));
			del_req->bag_index = req->bag_index;
			del_req->num = req->num;
			del_req->price = req->price;
			del_req->fee = req->fee;
			del_req->trade_id = req->trade_id;
			del_req->shelf_index = free_index;

			fast_send_msg_base(&conn_node_tradesrv::connecter, extern_data, SERVER_PROTO_TRADE_ON_SHELF_DELETE_ITEM_REQUEST, sizeof(TRADE_ON_SHELF_DELETE_ITEM_REQUEST), 0);
		}
	} while(0);

	if (ret != 0)
	{
		CommAnswer resp;
		comm_answer__init(&resp);

		resp.result = ret;

		fast_send_msg(&conn_node_tradesrv::connecter, extern_data, MSG_ID_TRADE_ON_SHELF_ANSWER, comm_answer__pack, resp);
	}

	return 0;
}

int conn_node_tradesrv::handle_trade_on_shelf_delete_item_answer(EXTERN_DATA *extern_data)
{
	TRADE_ON_SHELF_DELETE_ITEM_ANSWER *ans = (TRADE_ON_SHELF_DELETE_ITEM_ANSWER*)get_data();

	int ret = 0;
	TradePlayer *player = NULL;
	do
	{
		player = get_trade_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] get player[%lu] failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (ans->shelf_index >= MAX_TRADE_SHELF_NUM)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] shelf index:%u", __FUNCTION__, __LINE__, extern_data->player_id, ans->shelf_index);
			break;
		}

		TradeItem *item = player->shelf_items[ans->shelf_index];
		if (item == NULL)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] shelf not item, index:%u", __FUNCTION__, __LINE__, extern_data->player_id, ans->shelf_index);
			break;
		}
		if (get_trade_operate(item->state) != Trade_Operate_On_Shelf)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] shelf item operate error, index:%u, operate:%u", __FUNCTION__, __LINE__, extern_data->player_id, ans->shelf_index, get_trade_operate(item->state));
			break;
		}

		set_trade_operate(item->state, 0);
		if (ans->result == 0)
		{ //上架成功
			save_trade_item(item);

			update_trade_shelf_notify(item);
		}
		else
		{ //上架失败
			remove_trade_item(item);
			if (ans->result == ERROR_ID_COIN_NOT_ENOUGH)
			{
				ans->result = 190500454;
			}
		}
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = (ans->result != 0 ? ans->result : ret);

	fast_send_msg(&conn_node_tradesrv::connecter, extern_data, MSG_ID_TRADE_ON_SHELF_ANSWER, comm_answer__pack, resp);

	return 0;
}

int conn_node_tradesrv::handle_trade_off_shelf_request(EXTERN_DATA *extern_data)
{
	TradeOffShelfRequest *req = trade_off_shelf_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint32_t shelf_index = req->shelfindex;
	trade_off_shelf_request__free_unpacked(req, NULL);

	int ret = 0;
	TradePlayer *player = NULL;
	do
	{
		player = get_trade_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] get player[%lu] failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (shelf_index >= MAX_TRADE_SHELF_NUM || shelf_index >= player->shelf_num)
		{
			ret = ERROR_ID_TRADE_SHELF_INDEX;
			LOG_ERR("[%s:%d] player[%lu] shelf index[%u]", __FUNCTION__, __LINE__, extern_data->player_id, shelf_index);
			break;
		}

		TradeItem *item = player->shelf_items[shelf_index];
		if (!item)
		{
			ret = ERROR_ID_TRADE_ITEM_OFF_SHELF;
			LOG_ERR("[%s:%d] player[%lu] item not on shelf, index[%u]", __FUNCTION__, __LINE__, extern_data->player_id, shelf_index);
			break;
		}

		if (get_trade_operate(item->state) != 0)
		{
			ret = ERROR_ID_TRADE_ITEM_ADJUSTING;
			LOG_ERR("[%s:%d] player[%lu] item adjusting, index[%u]", __FUNCTION__, __LINE__, extern_data->player_id, shelf_index);
			break;
		}

		set_trade_operate(item->state, Trade_Operate_Off_Shelf);
		save_trade_item(item);

		{
			TRADE_OFF_SHELF_ADD_ITEM_REQUEST *off_req = (TRADE_OFF_SHELF_ADD_ITEM_REQUEST *)get_send_data();
			memset(off_req, 0, sizeof(TRADE_OFF_SHELF_ADD_ITEM_REQUEST));
			off_req->shelf_index = shelf_index;
			off_req->trade_id = item->item_id;
			off_req->num = item->num;
			memcpy(&off_req->especial, &item->especial, sizeof(EspecialItemInfo));

			fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_TRADE_OFF_SHELF_ADD_ITEM_REQUEST, sizeof(TRADE_OFF_SHELF_ADD_ITEM_REQUEST), 0);
		}

	} while(0);

	if (ret != 0)
	{
		CommAnswer resp;
		comm_answer__init(&resp);

		resp.result = ret;

		fast_send_msg(&conn_node_tradesrv::connecter, extern_data, MSG_ID_TRADE_OFF_SHELF_ANSWER, comm_answer__pack, resp);
	}

	return 0;
}

int conn_node_tradesrv::handle_trade_off_shelf_add_item_answer(EXTERN_DATA *extern_data)
{
	TRADE_OFF_SHELF_ADD_ITEM_ANSWER *ans = (TRADE_OFF_SHELF_ADD_ITEM_ANSWER*)get_data();

	int ret = 0;
	TradePlayer *player = NULL;
	do
	{
		player = get_trade_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] get player[%lu] failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (ans->shelf_index >= MAX_TRADE_SHELF_NUM)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] shelf index:%u", __FUNCTION__, __LINE__, extern_data->player_id, ans->shelf_index);
			break;
		}

		TradeItem *item = player->shelf_items[ans->shelf_index];
		if (item == NULL)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] shelf not item, index:%u", __FUNCTION__, __LINE__, extern_data->player_id, ans->shelf_index);
			break;
		}
		if (get_trade_operate(item->state) != Trade_Operate_Off_Shelf)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] shelf item operate error, index:%u, operate:%u", __FUNCTION__, __LINE__, extern_data->player_id, ans->shelf_index, get_trade_operate(item->state));
			break;
		}

		set_trade_operate(item->state, 0);

		if (ans->result == 0)
		{ //下架成功
			remove_trade_item(item);
		}
		else
		{ //下架失败
			save_trade_item(item);
		}
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = (ans->result != 0 ? ans->result : ret);

	fast_send_msg(&conn_node_tradesrv::connecter, extern_data, MSG_ID_TRADE_OFF_SHELF_ANSWER, comm_answer__pack, resp);

	return 0;
}

int conn_node_tradesrv::handle_trade_reshelf_request(EXTERN_DATA *extern_data)
{
	TradeReshelfRequest *req = trade_reshelf_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint32_t shelf_index = req->shelfindex;
	uint32_t num = req->num;
	uint32_t price = req->price;
	trade_reshelf_request__free_unpacked(req, NULL);

	int ret = 0;
	TradePlayer *player = NULL;
	do
	{
		player = get_trade_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] get player[%lu] failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (shelf_index >= MAX_TRADE_SHELF_NUM || shelf_index >= player->shelf_num)
		{
			ret = ERROR_ID_TRADE_SHELF_INDEX;
			LOG_ERR("[%s:%d] player[%lu] shelf index[%u]", __FUNCTION__, __LINE__, extern_data->player_id, shelf_index);
			break;
		}

		TradeItem *item = player->shelf_items[shelf_index];
		if (!item)
		{
			ret = ERROR_ID_TRADE_ITEM_OFF_SHELF;
			LOG_ERR("[%s:%d] player[%lu] item not on shelf, index[%u]", __FUNCTION__, __LINE__, extern_data->player_id, shelf_index);
			break;
		}

		if (get_trade_operate(item->state) != 0)
		{
			ret = ERROR_ID_TRADE_ITEM_ADJUSTING;
			LOG_ERR("[%s:%d] player[%lu] item adjusting, index[%u]", __FUNCTION__, __LINE__, extern_data->player_id, shelf_index);
			break;
		}

		//价格设置是否合理
		if (check_trade_item_price_rational(item->item_id, price))
		{
			ret = ERROR_ID_TRADE_PRICE;
			LOG_ERR("[%s:%d] player[%lu] price, index[%u], item_id:%u, price:%u", __FUNCTION__, __LINE__, extern_data->player_id, shelf_index, item->item_id, item->price);
			break;
		}

		//数量只能减少
		if (item->num < num)
		{
			ret = ERROR_ID_TRADE_ITEM_NUM;
			LOG_ERR("[%s:%d] player[%lu] num, index[%u], num:%u, new_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, shelf_index, item->num, num);
			break;
		}

		set_trade_operate(item->state, Trade_Operate_Re_Shelf);
		save_trade_item(item);

		{
			TRADE_RE_SHELF_CHANGE_REQUEST *change_req = (TRADE_RE_SHELF_CHANGE_REQUEST *)get_send_data();
			memset(change_req, 0, sizeof(TRADE_RE_SHELF_CHANGE_REQUEST));
			change_req->shelf_index = shelf_index;
			change_req->num = num;
			change_req->price = price;
			change_req->trade_id = item->item_id;
			change_req->off_num = item->num - num;
			memcpy(&change_req->especial, &item->especial, sizeof(EspecialItemInfo));

			fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_TRADE_RE_SHELF_CHANGE_REQUEST, sizeof(TRADE_RE_SHELF_CHANGE_REQUEST), 0);
		}

	} while(0);

	if (ret != 0)
	{
		CommAnswer resp;
		comm_answer__init(&resp);

		resp.result = ret;

		fast_send_msg(&conn_node_tradesrv::connecter, extern_data, MSG_ID_TRADE_RESHELF_ANSWER, comm_answer__pack, resp);
	}

	return 0;
}

int conn_node_tradesrv::handle_trade_reshelf_change_answer(EXTERN_DATA *extern_data)
{
	TRADE_RE_SHELF_CHANGE_ANSWER *ans = (TRADE_RE_SHELF_CHANGE_ANSWER*)get_data();

	int ret = 0;
	TradePlayer *player = NULL;
	do
	{
		player = get_trade_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] get player[%lu] failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (ans->shelf_index >= MAX_TRADE_SHELF_NUM || ans->shelf_index >= player->shelf_num)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] shelf index:%u", __FUNCTION__, __LINE__, extern_data->player_id, ans->shelf_index);
			break;
		}

		TradeItem *item = player->shelf_items[ans->shelf_index];
		if (item == NULL)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] shelf not item, index:%u", __FUNCTION__, __LINE__, extern_data->player_id, ans->shelf_index);
			break;
		}
		if (get_trade_operate(item->state) != Trade_Operate_Re_Shelf)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] shelf item operate error, index:%u, operate:%u", __FUNCTION__, __LINE__, extern_data->player_id, ans->shelf_index, get_trade_operate(item->state));
			break;
		}
		if (item->num < ans->num)
		{
			ret = ERROR_ID_TRADE_ITEM_NUM;
			LOG_ERR("[%s:%d] player[%lu] num, index[%u], num:%u, new_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, ans->shelf_index, item->num, ans->num);
			break;
		}

		set_trade_operate(item->state, 0);

		if (ans->result == 0)
		{ //重新上架成功
			bool summary_change = (get_trade_state(item->state) == Trade_State_Sell);
			item->price = ans->price;
			item->num = ans->num;
			item->state = Trade_State_Aduit;
			item->time = (time_helper::get_cached_time() / 1000 + get_trade_aduit_time());
			update_trade_shelf_notify(item);
			save_trade_item(item);
			if (summary_change)
			{
				update_trade_item_summary_broadcast(item->item_id);
			}
		}
		else
		{ //重新上架失败
			save_trade_item(item);
		}
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = (ans->result != 0 ? ans->result : ret);

	fast_send_msg(&conn_node_tradesrv::connecter, extern_data, MSG_ID_TRADE_RESHELF_ANSWER, comm_answer__pack, resp);

	return 0;
}

int conn_node_tradesrv::handle_trade_enlarge_shelf_request(EXTERN_DATA *extern_data)
{
	int ret = 0;
	TradePlayer *player = NULL;
	do
	{
		player = get_trade_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] get player[%lu] failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (player->shelf_num >= MAX_TRADE_SHELF_NUM || player->shelf_num >= sg_shelf_max_num)
		{
			ret = ERROR_ID_TRADE_SHELF_NUM_MAX;
			LOG_ERR("[%s:%d] player[%lu] cur_shelf_num:%u, memory_max:%u, config_max:%u", __FUNCTION__, __LINE__, extern_data->player_id, player->shelf_num, MAX_TRADE_SHELF_NUM, sg_shelf_max_num);
			break;
		}

		{
			//请求扣除消耗
			PROTO_SRV_CHECK_AND_COST_REQ *cost_req = (PROTO_SRV_CHECK_AND_COST_REQ *)get_send_data();
			uint32_t data_len = sizeof(PROTO_SRV_CHECK_AND_COST_REQ);
			memset(cost_req, 0, data_len);
			cost_req->cost.statis_id = MAGIC_TYPE_TRADE_ENLARGE_SHELF;
			cost_req->cost.unbind_gold = sg_shelf_enlarge_price;
			fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_TRADESRV_COST_REQUEST, data_len, 0);
		}

	} while(0);

	if (ret != 0)
	{
		TradeEnlargeShelfAnswer resp;
		trade_enlarge_shelf_answer__init(&resp);

		resp.result = ret;
		if (player)
		{
			resp.shelfnum = player->shelf_num;
		}

		fast_send_msg(&conn_node_tradesrv::connecter, extern_data, MSG_ID_TRADE_ENLARGE_SHELF_ANSWER, trade_enlarge_shelf_answer__pack, resp);
	}

	return 0;
}

static int handle_trade_enlarge_shelf_cost_answer(int data_len, uint8_t *data, int result, EXTERN_DATA *extern_data)
{
	int ret = result;
	bool internal = false;
	TradePlayer *player = NULL;
	do
	{
		if (ret != 0)
		{
			break;
		}
		internal = true;

		player = get_trade_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] get player[%lu] failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (player->shelf_num >= MAX_TRADE_SHELF_NUM || player->shelf_num >= sg_shelf_max_num)
		{
			ret = ERROR_ID_TRADE_SHELF_NUM_MAX;
			LOG_ERR("[%s:%d] player[%lu] cur_shelf_num:%u, memory_max:%u, config_max:%u", __FUNCTION__, __LINE__, extern_data->player_id, player->shelf_num, MAX_TRADE_SHELF_NUM, sg_shelf_max_num);
			break;
		}

		player->shelf_num++;
		save_trade_player(player);
	} while(0);

	TradeEnlargeShelfAnswer resp;
	trade_enlarge_shelf_answer__init(&resp);

	resp.result = ret;
	if (player)
	{
		resp.shelfnum = player->shelf_num;
	}

	fast_send_msg(&conn_node_tradesrv::connecter, extern_data, MSG_ID_TRADE_ENLARGE_SHELF_ANSWER, trade_enlarge_shelf_answer__pack, resp);

	return (internal ? ret : 0);
}

int conn_node_tradesrv::handle_trade_buy_request(EXTERN_DATA *extern_data)
{
	TradeBuyRequest *req = trade_buy_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint64_t seller_id = req->playerid;
	uint32_t shelf_index = req->shelfindex;
	uint32_t buy_num = req->num;
	trade_buy_request__free_unpacked(req, NULL);

	int ret = 0;
	TradePlayer *player = NULL;
	do
	{
		if (buy_num == 0)
		{
			ret = ERROR_ID_TRADE_OPERATE_NUM;
			break;
		}

		if (seller_id == extern_data->player_id)
		{
			ret = 190500432;
			break;
		}

		player = get_trade_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] get player[%lu] failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		TradePlayer *seller = get_trade_player(seller_id);
		if (!seller)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get seller[%lu] failed", __FUNCTION__, __LINE__, extern_data->player_id, seller_id);
			break;
		}

		if (shelf_index >= MAX_TRADE_SHELF_NUM || shelf_index >= seller->shelf_num)
		{
			ret = ERROR_ID_TRADE_SHELF_INDEX;
			LOG_ERR("[%s:%d] player[%lu] buy index error, seller[%lu], shelf index[%u]", __FUNCTION__, __LINE__, extern_data->player_id, seller_id, shelf_index);
			break;
		}

		TradeItem *item = seller->shelf_items[shelf_index];
		if (!item)
		{
			ret = ERROR_ID_TRADE_ITEM_OFF_SHELF;
			LOG_ERR("[%s:%d] player[%lu] buy item off shelf, seller[%lu], index[%u]", __FUNCTION__, __LINE__, extern_data->player_id, seller_id, shelf_index);
			break;
		}

		if (get_trade_operate(item->state) != 0)
		{
			ret = ERROR_ID_TRADE_ITEM_ADJUSTING;
			LOG_ERR("[%s:%d] player[%lu] buy item is adjusting, seller[%lu], index[%u], operate:%u", __FUNCTION__, __LINE__, extern_data->player_id, seller_id, shelf_index, get_trade_operate(item->state));
			break;
		}

		if (get_trade_state(item->state) != Trade_State_Sell)
		{
			ret = ERROR_ID_TRADE_ITEM_OFF_SHELF;
			LOG_ERR("[%s:%d] player[%lu] buy item not selling, seller[%lu], index[%u], state:%u", __FUNCTION__, __LINE__, extern_data->player_id, seller_id, shelf_index, get_trade_state(item->state));
			break;
		}

		if (item->num < buy_num)
		{
			ret = ERROR_ID_TRADE_ITEM_LEFT_NUM;
			LOG_ERR("[%s:%d] player[%lu] buy item left num, seller[%lu], index[%u], buy_num:%u, left_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, seller_id, shelf_index, buy_num, item->num);
			break;
		}

		set_trade_operate(item->state, Trade_Operate_Buy);
		save_trade_item(item);

		{
			TRADE_BUY_EXECUTE_REQUEST *change_req = (TRADE_BUY_EXECUTE_REQUEST *)get_send_data();
			uint32_t data_len = sizeof(TRADE_BUY_EXECUTE_REQUEST);
			memset(change_req, 0, data_len);
			change_req->seller_id = seller_id;
			change_req->shelf_index = shelf_index;
			change_req->buy_num = buy_num;
			change_req->trade_id = item->item_id;
			change_req->buy_price = item->price;
			memcpy(&change_req->especial, &item->especial, sizeof(EspecialItemInfo));

			fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_TRADE_BUY_EXECUTE_REQUEST, data_len, 0);
		}

	} while(0);

	if (ret != 0)
	{
		CommAnswer resp;
		comm_answer__init(&resp);

		resp.result = ret;

		fast_send_msg(&conn_node_tradesrv::connecter, extern_data, MSG_ID_TRADE_BUY_ANSWER, comm_answer__pack, resp);
	}

	return 0;
}

int conn_node_tradesrv::handle_trade_buy_execute_answer(EXTERN_DATA *extern_data)
{
	TRADE_BUY_EXECUTE_ANSWER *ans = (TRADE_BUY_EXECUTE_ANSWER *)get_data();

	uint64_t seller_id = ans->seller_id;
	uint32_t shelf_index = ans->shelf_index;
	uint32_t buy_num = ans->buy_num;

	int ret = 0;
	TradePlayer *player = NULL;
	do
	{
		if (buy_num == 0)
		{
			ret = ERROR_ID_TRADE_OPERATE_NUM;
			break;
		}

		player = get_trade_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] get player[%lu] failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		TradePlayer *seller = get_trade_player(seller_id);
		if (!seller)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get seller[%lu] failed", __FUNCTION__, __LINE__, extern_data->player_id, seller_id);
			break;
		}

		if (shelf_index >= MAX_TRADE_SHELF_NUM || shelf_index >= seller->shelf_num)
		{
			ret = ERROR_ID_TRADE_SHELF_INDEX;
			LOG_ERR("[%s:%d] player[%lu] buy index error, seller[%lu], shelf index[%u]", __FUNCTION__, __LINE__, extern_data->player_id, seller_id, shelf_index);
			break;
		}

		TradeItem *item = seller->shelf_items[shelf_index];
		if (!item)
		{
			ret = ERROR_ID_TRADE_ITEM_OFF_SHELF;
			LOG_ERR("[%s:%d] player[%lu] buy item off shelf, seller[%lu], index[%u]", __FUNCTION__, __LINE__, extern_data->player_id, seller_id, shelf_index);
			break;
		}

		if (get_trade_operate(item->state) != Trade_Operate_Buy)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] buy item operate error, seller[%lu], index[%u], operate:%u", __FUNCTION__, __LINE__, extern_data->player_id, seller_id, shelf_index, get_trade_operate(item->state));
			break;
		}

		//有可能在前面一部分处理的时候还没过期，现在过期了
//		if (get_trade_state(item->state) != Trade_State_Sell)
//		{
//			ret = ERROR_ID_TRADE_ITEM_OFF_SHELF;
//			LOG_ERR("[%s:%d] player[%lu] buy item not selling, seller[%lu], index[%lu], state:%u", __FUNCTION__, __LINE__, extern_data->player_id, seller_id, shelf_index, get_trade_state(item->state));
//			break;
//		}

		if (item->num < buy_num)
		{
			ret = ERROR_ID_TRADE_ITEM_LEFT_NUM;
			LOG_ERR("[%s:%d] player[%lu] buy item left num, seller[%lu], index[%u], buy_num:%u, left_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, seller_id, shelf_index, buy_num, item->num);
			break;
		}

		set_trade_operate(item->state, 0);

		if (ans->result == 0)
		{ //购买成功
			//增加卖家身上的出售记录
			uint32_t seller_earning = (buy_num * item->price * (1.0 - sg_trade_tax_percent)); //要扣除交易税
			for (int i = 0; i < MAX_TRADE_SOLD_NUM; ++i)
			{
				if (seller->sold_items[i].item_id == 0)
				{
					seller->sold_items[i].item_id = item->item_id;
					seller->sold_items[i].num = buy_num;
					seller->sold_items[i].price = seller_earning;
					break;
				}
			}
			seller->sold_earning += seller_earning;

			//发送出售成功通知
			{
				TradeSoldNotify nty;
				trade_sold_notify__init(&nty);

				TradeSoldData sold_data;
				trade_sold_data__init(&sold_data);

				sold_data.itemid = item->item_id;
				sold_data.num = buy_num;
				sold_data.price = seller_earning;
				nty.item = &sold_data;
				nty.earning = seller_earning;

				fast_send_msg(&connecter, extern_data, MSG_ID_TRADE_SOLD_NOTIFY, trade_sold_notify__pack, nty);
			}

			//改变当天已售均价
			add_sold_info_to_average_map(item->item_id, buy_num, item->price);
			
			item->num -= buy_num;
			if (item->num == 0)
			{ //该货品售完
				remove_trade_item(item);
			}
			else
			{ //该货品还有剩
				save_trade_item(item);
				update_trade_shelf_notify(item);
				update_trade_item_summary_broadcast(item->item_id);
			}
		}
		else
		{ //购买失败
			save_trade_item(item);
		}
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = (ans->result != 0 ? ans->result : ret);

	fast_send_msg(&conn_node_tradesrv::connecter, extern_data, MSG_ID_TRADE_BUY_ANSWER, comm_answer__pack, resp);

	return 0;
}

int conn_node_tradesrv::handle_trade_get_earning_request(EXTERN_DATA *extern_data)
{
	int ret = 0;
	TradePlayer *player = NULL;
	do
	{
		player = get_trade_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] get player[%lu] failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (player->sold_earning == 0)
		{
			break;
		}

		//通知game_srv发放奖励
		{
			uint32_t *pData = (uint32_t*)get_send_data();
			uint32_t data_len = sizeof(uint32_t);
			memset(pData, 0, data_len);
			*pData = player->sold_earning;
			
			fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_TRADE_GET_EARNING_GIVE_REQUEST, data_len, 0);
		}

		player->sold_earning = 0;
		memset(&player->sold_items, 0, sizeof(player->sold_items));
		save_trade_player(player);
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;

	fast_send_msg(&conn_node_tradesrv::connecter, extern_data, MSG_ID_TRADE_GET_EARNING_ANSWER, comm_answer__pack, resp);

	return 0;
}

int conn_node_tradesrv::handle_auction_lot_insert_request(EXTERN_DATA * /*extern_data*/)
{
	TRADE_LOT_INSERT *req = (TRADE_LOT_INSERT *)get_data();

	AuctionLot *lot = create_auction_lot(req->lot_id, Auction_Type_Guild, req->guild_id, req->masters, MAX_AUCTION_MASTER_NUM);
	if (!lot)
	{
		LOG_ERR("[%s:%d] create lot failed, lot_id:%u", __FUNCTION__, __LINE__, req->lot_id);
		return -1;
	}

	return 0;
}

int conn_node_tradesrv::handle_auction_info_request(EXTERN_DATA *extern_data)
{
	AuctionInfoRequest *req = auction_info_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint32_t type = req->type;
	auction_info_request__free_unpacked(req, NULL);

	uint32_t type_limit = 0;
	if (type == Auction_Type_Guild)
	{
		AutoReleaseRedisPlayer ar_redis;
		PlayerRedisInfo *redis_player = get_redis_player(extern_data->player_id, sg_player_key, sg_redis_client, ar_redis);
		if (redis_player)
		{
			type_limit = redis_player->guild_id;
		}
	}

	AuctionInfoAnswer resp;
	auction_info_answer__init(&resp);

	AuctionLotData  lot_data[MAX_AUCTION_LOT_NUM];
	AuctionLotData* lot_point[MAX_AUCTION_LOT_NUM];

	resp.result = 0;
	resp.auctionlist = lot_point;
	resp.n_auctionlist = 0;
	for (AuctionLotMap::iterator iter = auction_lot_map.begin(); iter != auction_lot_map.end(); ++iter)
	{
		AuctionLot *lot = iter->second;
		if (lot->type != type)
		{
			continue;
		}
		if (lot->type == Auction_Type_Guild && lot->type_limit != type_limit)
		{
			continue;
		}

		lot_point[resp.n_auctionlist] = &lot_data[resp.n_auctionlist];
		auction_lot_data__init(&lot_data[resp.n_auctionlist]);
		lot_data[resp.n_auctionlist].uuid = lot->uuid;
		lot_data[resp.n_auctionlist].lotid = lot->lot_id;
		lot_data[resp.n_auctionlist].price = lot->price;
		lot_data[resp.n_auctionlist].time = lot->time;
		lot_data[resp.n_auctionlist].state = get_player_auction_lot_state(lot, extern_data->player_id);
		resp.n_auctionlist++;
	}

	fast_send_msg(&connecter, extern_data, MSG_ID_AUCTION_INFO_ANSWER, auction_info_answer__pack, resp);

	return 0;
}

int conn_node_tradesrv::handle_auction_bid_request(EXTERN_DATA *extern_data)
{
	AuctionBidRequest *req = auction_bid_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint64_t uuid = req->uuid;
	uint32_t cur_price = req->curprice;
	auction_bid_request__free_unpacked(req, NULL);

	int ret = 0;
	TradePlayer *player = NULL;
	AuctionLot *lot = NULL;
	do
	{
		player = get_trade_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] get player[%lu] failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		lot = get_auction_lot(uuid);
		if (!lot)
		{
			ret = ERROR_ID_AUCTION_SOLD;
			LOG_ERR("[%s:%d] player[%lu] get lot failed, uuid:%lu", __FUNCTION__, __LINE__, extern_data->player_id, uuid);
			break;
		}

		if (lot->price != cur_price)
		{
			ret = ERROR_ID_AUCTION_PRICE;
			LOG_ERR("[%s:%d] player[%lu] lot price, uuid:%lu, client_price:%u, server_price:%u", __FUNCTION__, __LINE__, extern_data->player_id, uuid, cur_price, lot->price);
			break;
		}

		if (lot->operator_id != 0)
		{
			ret = ERROR_ID_TRADE_ITEM_ADJUSTING;
			LOG_ERR("[%s:%d] player[%lu] lot operating, uuid:%lu, operator_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, uuid, lot->operator_id);
			break;
		}

		if (lot->bidder_id > 0)
		{
			if (lot->bidder_id == player->player_id)
			{
				ret = 190500461;
				LOG_ERR("[%s:%d] player[%lu] is current bidder, uuid:%lu, bidder_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, uuid, lot->bidder_id);
				break;
			}

			TradePlayer *bidder = get_trade_player(lot->bidder_id);
			if (!bidder)
			{
				ret = ERROR_ID_SERVER;
				LOG_ERR("[%s:%d] player[%lu] get bidder fail, uuid:%lu, bidder_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, uuid, lot->bidder_id);
				break;
			}
		}

		AuctionTable *config = get_config_by_id(lot->lot_id, &auction_config);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] config, lot_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, lot->lot_id);
			break;
		}

		//mark
		lot->operator_id = extern_data->player_id;
		uint32_t bid_price = lot->price + config->Fare;
		{
			//请求扣除消耗
			PROTO_SRV_CHECK_AND_COST_REQ *cost_req = (PROTO_SRV_CHECK_AND_COST_REQ *)get_send_data();
			uint32_t data_len = sizeof(PROTO_SRV_CHECK_AND_COST_REQ) + sizeof(TRADE_BID_CHECK_CARRY);
			memset(cost_req, 0, data_len);
			cost_req->cost.statis_id = MAGIC_TYPE_AUCTION_BID;
			cost_req->cost.unbind_gold = bid_price;
			cost_req->data_size = sizeof(TRADE_BID_CHECK_CARRY);
			TRADE_BID_CHECK_CARRY *pCarry = (TRADE_BID_CHECK_CARRY*)cost_req->data;
			pCarry->lot_uuid = uuid;
			pCarry->cur_price = cur_price;
			pCarry->bid_price = bid_price;
			fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_TRADESRV_COST_REQUEST, data_len, 0);
		}
	} while(0);

	if (ret != 0)
	{
		AuctionBidAnswer resp;
		auction_bid_answer__init(&resp);

		AuctionLotData lot_data;
		auction_lot_data__init(&lot_data);

		resp.result = ret;
		resp.lot = &lot_data;
		if (lot == NULL)
		{
			lot_data.uuid = uuid;
		}
		else
		{
			lot_data.uuid = lot->uuid;
			lot_data.lotid = lot->lot_id;
			lot_data.price = lot->price;
			lot_data.time = lot->time;
			lot_data.state = get_player_auction_lot_state(lot, extern_data->player_id);
		}
		fast_send_msg(&connecter, extern_data, MSG_ID_AUCTION_BID_ANSWER, auction_bid_answer__pack, resp);
	}

	return 0;
}

static int handle_auction_bid_cost_answer(int data_len, uint8_t *data, int result, EXTERN_DATA *extern_data)
{
	TRADE_BID_CHECK_CARRY *pCarry = (TRADE_BID_CHECK_CARRY*)data;
	uint32_t uuid = pCarry->lot_uuid;

	int ret = result;
	bool internal = false;
	TradePlayer *player = NULL;
	AuctionLot *lot = NULL;
	do
	{
		if (ret != 0)
		{
			lot = get_auction_lot(uuid);
			if (lot && lot->operator_id == extern_data->player_id)
			{
				lot->operator_id = 0;
			}
			if (ret == ERROR_ID_GOLD_NOT_ENOUGH)
			{
				ret = 190500455;
			}

			break;
		}
		internal = true;

		player = get_trade_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] get player[%lu] failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		lot = get_auction_lot(uuid);
		if (!lot)
		{
			ret = ERROR_ID_AUCTION_SOLD;
			LOG_ERR("[%s:%d] player[%lu] get lot failed, uuid:%u", __FUNCTION__, __LINE__, extern_data->player_id, uuid);
			break;
		}

		if (lot->operator_id != extern_data->player_id)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] not operator, uuid:%u, operator_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, uuid, lot->operator_id);
			break;
		}

		TradePlayer *bidder = NULL;
		if (lot->bidder_id > 0)
		{
			if (lot->bidder_id == player->player_id)
			{
				ret = 190500461;
				LOG_ERR("[%s:%d] player[%lu] is current bidder, uuid:%u, bidder_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, uuid, lot->bidder_id);
				break;
			}

			bidder = get_trade_player(lot->bidder_id);
			if (!bidder)
			{
				ret = ERROR_ID_SERVER;
				LOG_ERR("[%s:%d] player[%lu] get bidder fail, uuid:%u, bidder_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, uuid, lot->bidder_id);
				break;
			}
		}

		//mark
		lot->operator_id = 0;
		
		//把前一个竞价者的钱还给他
		if (lot->bidder_id > 0)
		{
			if (player_is_online(lot->bidder_id))
			{ //玩家在线，发送到游戏服
				send_player_auction_bid_fail(lot->bidder_id, lot->price);
			}
			else
			{ //玩家离线，保存到数据库
				bidder->auction_bid_money = lot->price;
				save_trade_player(bidder);
			}
			uint32_t mail_id = (lot->type == Auction_Type_Guild ? 270100008 : 270100009);
			send_mail(&conn_node_tradesrv::connecter, lot->bidder_id, mail_id, NULL, NULL, NULL, NULL, NULL, 0);
		}

		//记录当前竞价信息
		insert_auction_bid(lot->uuid, extern_data->player_id, pCarry->bid_price);
		lot->price = pCarry->bid_price;
		lot->bidder_id = extern_data->player_id;

		save_auction_lot(lot);
	} while(0);

	AuctionBidAnswer resp;
	auction_bid_answer__init(&resp);

	AuctionLotData lot_data;
	auction_lot_data__init(&lot_data);

	resp.result = ret;
	resp.lot = &lot_data;
	if (lot == NULL)
	{
		lot_data.uuid = uuid;
	}
	else
	{
		lot_data.uuid = lot->uuid;
		lot_data.lotid = lot->lot_id;
		lot_data.price = lot->price;
		lot_data.time = lot->time;
		lot_data.state = get_player_auction_lot_state(lot, extern_data->player_id);
	}
	fast_send_msg(&conn_node_tradesrv::connecter, extern_data, MSG_ID_AUCTION_BID_ANSWER, auction_bid_answer__pack, resp);

	return (internal ? ret : 0);
}

int conn_node_tradesrv::handle_auction_buy_now_request(EXTERN_DATA *extern_data)
{
	AuctionBuyNowRequest *req = auction_buy_now_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint64_t uuid = req->uuid;
	auction_buy_now_request__free_unpacked(req, NULL);

	int ret = 0;
	TradePlayer *player = NULL;
	AuctionLot *lot = NULL;
	do
	{
		player = get_trade_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] get player[%lu] failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		lot = get_auction_lot(uuid);
		if (!lot)
		{
			ret = ERROR_ID_AUCTION_SOLD;
			LOG_ERR("[%s:%d] player[%lu] get lot failed, uuid:%lu", __FUNCTION__, __LINE__, extern_data->player_id, uuid);
			break;
		}

		if (lot->operator_id != 0)
		{
			ret = ERROR_ID_TRADE_ITEM_ADJUSTING;
			LOG_ERR("[%s:%d] player[%lu] lot operating, uuid:%lu, operator_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, uuid, lot->operator_id);
			break;
		}

		if (lot->bidder_id > 0)
		{
			TradePlayer *bidder = get_trade_player(lot->bidder_id);
			if (!bidder)
			{
				ret = ERROR_ID_SERVER;
				LOG_ERR("[%s:%d] player[%lu] get bidder fail, uuid:%lu, bidder_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, uuid, lot->bidder_id);
				break;
			}
		}

		AuctionTable *config = get_config_by_id(lot->lot_id, &auction_config);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] config, lot_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, lot->lot_id);
			break;
		}

		//mark
		lot->operator_id = extern_data->player_id;
		uint32_t buy_price = config->Price;
		{
			//请求扣除消耗
			PROTO_SRV_CHECK_AND_COST_REQ *cost_req = (PROTO_SRV_CHECK_AND_COST_REQ *)get_send_data();
			uint32_t data_len = sizeof(PROTO_SRV_CHECK_AND_COST_REQ) + sizeof(TRADE_BID_CHECK_CARRY);
			memset(cost_req, 0, data_len);
			cost_req->cost.statis_id = MAGIC_TYPE_AUCTION_BUY_NOW;
			cost_req->cost.unbind_gold = buy_price;
			cost_req->data_size = sizeof(TRADE_BID_CHECK_CARRY);
			TRADE_BID_CHECK_CARRY *pCarry = (TRADE_BID_CHECK_CARRY*)cost_req->data;
			pCarry->lot_uuid = uuid;
			pCarry->bid_price = buy_price;
			fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_TRADESRV_COST_REQUEST, data_len, 0);
		}
	} while(0);

	if (ret != 0)
	{
		AuctionBuyNowAnswer resp;
		auction_buy_now_answer__init(&resp);

		resp.result = ret;
		resp.uuid = uuid;
		fast_send_msg(&connecter, extern_data, MSG_ID_AUCTION_BUY_NOW_ANSWER, auction_buy_now_answer__pack, resp);
	}

	return 0;
}

static int handle_auction_buy_now_cost_answer(int data_len, uint8_t *data, int result, EXTERN_DATA *extern_data)
{
	TRADE_BID_CHECK_CARRY *pCarry = (TRADE_BID_CHECK_CARRY*)data;
	uint32_t uuid = pCarry->lot_uuid;

	int ret = result;
	bool internal = false;
	TradePlayer *player = NULL;
	AuctionLot *lot = NULL;
	do
	{
		if (ret != 0)
		{
			lot = get_auction_lot(uuid);
			if (lot && lot->operator_id == extern_data->player_id)
			{
				lot->operator_id = 0;
			}
			break;
		}
		internal = true;

		player = get_trade_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] get player[%lu] failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		lot = get_auction_lot(uuid);
		if (!lot)
		{
			ret = ERROR_ID_AUCTION_SOLD;
			LOG_ERR("[%s:%d] player[%lu] get lot failed, uuid:%u", __FUNCTION__, __LINE__, extern_data->player_id, uuid);
			break;
		}

		if (lot->operator_id != extern_data->player_id)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] not operator, uuid:%u, operator_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, uuid, lot->operator_id);
			break;
		}

		TradePlayer *bidder = NULL;
		if (lot->bidder_id > 0)
		{
			bidder = get_trade_player(lot->bidder_id);
			if (!bidder)
			{
				ret = ERROR_ID_SERVER;
				LOG_ERR("[%s:%d] player[%lu] get bidder fail, uuid:%u, bidder_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, uuid, lot->bidder_id);
				break;
			}
		}

		AuctionTable *config = get_config_by_id(lot->lot_id, &auction_config);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] config, lot_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, lot->lot_id);
			break;
		}

		//mark
		lot->operator_id = 0;
		
		//把前一个竞价者的钱还给他
		if (lot->bidder_id > 0)
		{
			if (player_is_online(lot->bidder_id))
			{ //玩家在线，发送到游戏服
				send_player_auction_bid_fail(lot->bidder_id, lot->price);
			}
			else
			{ //玩家离线，保存到数据库
				bidder->auction_bid_money = lot->price;
				save_trade_player(bidder);
			}
			uint32_t mail_id = (lot->type == Auction_Type_Guild ? 270100008 : 270100009);
			send_mail(&conn_node_tradesrv::connecter, lot->bidder_id, mail_id, NULL, NULL, NULL, NULL, NULL, 0);
		}

		//把拍卖品发给玩家，把得到的钱平均分给激活玩家，然后删除
		uint32_t master_num = 0;
		for (int i = 0; i < MAX_AUCTION_MASTER_NUM; ++i)
		{
			if (lot->masters[i] == 0)
			{
				break;
			}
			master_num++;
		}
		if (master_num > 0)
		{
			uint32_t give_money = std::max((uint32_t)(pCarry->bid_price * (1.0 - sg_trade_tax_percent)) / master_num, (uint32_t)1);
			std::map<uint32_t, uint32_t> attachs;
			attachs.insert(std::make_pair(201010003, give_money));
			std::vector<char *> args;
			std::string sz_sold_price, sz_gain_money;
			std::stringstream ss;
			args.push_back(config->ItemName);
			ss << pCarry->bid_price;
			ss >> sz_sold_price;
			args.push_back(const_cast<char*>(sz_sold_price.c_str()));
			ss.str("");
			ss.clear();
			ss << give_money;
			ss >> sz_gain_money;
			args.push_back(const_cast<char*>(sz_gain_money.c_str()));
			for (int i = 0; i < MAX_AUCTION_MASTER_NUM; ++i)
			{
				if (lot->masters[i] == 0)
				{
					break;
				}
				send_mail(&conn_node_tradesrv::connecter, lot->masters[i], 270100007, NULL, NULL, NULL, &args, &attachs, MAGIC_TYPE_AUCTION_SOLD);
			}
		}
		{
			std::map<uint32_t, uint32_t> attachs;
			uint32_t _id = config->ItemID;
			uint32_t _num = config->Num;
			attachs.insert(std::make_pair(_id, _num));
			std::vector<char *> buyer_mail_args;
			std::string sz_num;
			std::stringstream ss;
			uint32_t buyer_mail_id = (lot->type == Auction_Type_Guild ? 270100003 : 270100004);
			buyer_mail_args.push_back(config->ItemName);
			ss << lot->price;
			ss >> sz_num;
			buyer_mail_args.push_back(const_cast<char*>(sz_num.c_str()));
			send_mail(&conn_node_tradesrv::connecter, extern_data->player_id, buyer_mail_id, NULL, NULL, NULL, &buyer_mail_args, &attachs, MAGIC_TYPE_AUCTION_SOLD);
		}

		remove_auction_lot(lot, true);
	} while(0);

	AuctionBuyNowAnswer resp;
	auction_buy_now_answer__init(&resp);

	resp.result = ret;
	resp.uuid = uuid;
	fast_send_msg(&conn_node_tradesrv::connecter, extern_data, MSG_ID_AUCTION_BUY_NOW_ANSWER, auction_buy_now_answer__pack, resp);

	return (internal ? ret : 0);
}

//发红包请求
int conn_node_tradesrv::handle_red_packet_send_red_packet_request(EXTERN_DATA *extern_data)
{
	RED_PACKET_SEND_DATA_REQUEST *resq = (RED_PACKET_SEND_DATA_REQUEST*)get_data();
	uint64_t send_red_times = time_helper::get_cached_time() / 1000;
	uint32_t red_type = resq->red_type;
	uint32_t red_coin_type = resq->red_coin_type;
	uint32_t money_num = resq->money_num;
	uint32_t red_num = resq->red_num;
	
	AutoReleaseRedisPlayer ar_redis;
	PlayerRedisInfo *redis_player = NULL; 
	uint64_t cur_red_uuid = alloc_red_packet_uuid();
	RedPacketRedisInfo cur_red_packet_info;
	red_packet_redis_info__init(&cur_red_packet_info);
	cur_red_packet_info.red_uuid = cur_red_uuid;
	cur_red_packet_info.player_id = extern_data->player_id;
	cur_red_packet_info.send_red_time = send_red_times;
	cur_red_packet_info.red_typ = red_type;
	cur_red_packet_info.system_or_player = resq->guild_type;
	cur_red_packet_info.red_coin_type = red_coin_type;
	cur_red_packet_info.red_sum_money = money_num;
	cur_red_packet_info.red_use_money = 0;
	cur_red_packet_info.red_sum_num = red_num;
	cur_red_packet_info.red_use_num = 0;
	cur_red_packet_info.player_text = resq->player_text;

	int ret = 0;
	do{
		redis_player = get_redis_player(extern_data->player_id, sg_player_key, sg_redis_client, ar_redis);
		if(redis_player == NULL)
		{
			LOG_ERR("[%s:%d]save red redis faild, get player redis info faild, send red packet player_id[%lu] red_type[%u] red_coin_type[%u] red_money_num[%u] red_num[%u]", __FUNCTION__, __LINE__, extern_data->player_id, red_type, red_coin_type, money_num, red_num);
			ret = 190500584;
			break;
		}
		cur_red_packet_info.player_name = redis_player->name;
		cur_red_packet_info.head_icon = redis_player->head_icon;
		cur_red_packet_info.player_level = redis_player->lv;
		uint32_t guild_id = redis_player->guild_id;
		cur_red_packet_info.guild_id = guild_id;
		//先判断uuid是否被占用
		bool re_uuid_use = false;
		for(std::map<uint32_t, std::map<uint64_t, uint64_t> >::iterator guild_itr = guild_red_packet_time_map.begin(); guild_itr != guild_red_packet_time_map.end(); guild_itr++)
		{
			std::map<uint64_t, uint64_t> guild_red_time_map = guild_itr->second;
			if(guild_red_time_map.find(cur_red_uuid) != guild_red_time_map.end())
			{
				re_uuid_use = true;
				break;
			}

		}
		if(re_uuid_use == true)
		{
			LOG_ERR("[%s:%d]save guild red redis faild, red_uuid already exist, send red packet player_id[%lu] red_type[%u] red_coin_type[%u] red_money_num[%u] red_num[%u]", __FUNCTION__, __LINE__, extern_data->player_id, red_type, red_coin_type, money_num, red_num);
			ret = 190500584;
			break;
		}
		if(normal_red_packet_time_map.find(cur_red_uuid) != normal_red_packet_time_map.end())
		{
			LOG_ERR("[%s:%d]save red redis faild, red_uuid already exist, send red packet player_id[%lu] red_type[%u] red_coin_type[%u] red_money_num[%u] red_num[%u]", __FUNCTION__, __LINE__, extern_data->player_id, red_type, red_coin_type, money_num, red_num);
			ret = 190500584;
			break;
		}


		//普通红包
		if(red_type == 1)
		{
			std::map<uint64_t, uint64_t> &red_map = normal_red_packet_time_map;
			if(red_map.size() >= sg_red_packet_baocun_max_num)
			{
				uint64_t last_red_uuid = 0;
				uint64_t last_red_time = 0xffffffff;
				for(std::map<uint64_t, uint64_t>::iterator itr = red_map.begin(); itr != red_map.end(); itr++)
				{
					if(itr->second < last_red_time)
					{
						last_red_uuid = itr->first;
						last_red_time = itr->second;
					}
				}

				ret = delete_one_red_packet_for_redis(last_red_uuid, sg_normal_red_packet_key, red_map);
				if(ret != 0)
				{
					LOG_ERR("[%s:%d]delete red packet redis error,last_red_uuid[%lu] send red packet player_id[%lu] red_type[%u] red_coin_type[%u] red_money_num[%u] red_num[%u]", __FUNCTION__, __LINE__, last_red_uuid, extern_data->player_id, red_type, red_coin_type, money_num, red_num);
					ret = 190500584;
					break;
				}
			}

			//数据存redis
			ret = save_one_red_packet_for_redis(&cur_red_packet_info, cur_red_uuid, sg_normal_red_packet_key);
			if(ret != 0)
			{
				LOG_ERR("[%s:%d]save normal red redis faild,player_id[%lu] red_type[%u] red_coin_type[%u] red_money_num[%u] red_num[%u]", __FUNCTION__, __LINE__, extern_data->player_id, red_type, red_coin_type, money_num, red_num);
				ret = 190500584;
				break;
			}
			red_map.insert(std::make_pair(cur_red_uuid, send_red_times));
			record_player_recive_red_packet_info(extern_data->player_id, cur_red_uuid, red_coin_type, money_num, true);
			break;
		}


		//帮会红包
		if(guild_id == 0)
		{
			LOG_ERR("[%s:%d]save guild red redis faild, get player guild faild, send red packet player_id[%lu] red_type[%u] red_coin_type[%u] red_money_num[%u] red_num[%u]", __FUNCTION__, __LINE__, extern_data->player_id, red_type, red_coin_type, money_num, red_num);
			ret = 190500584;
			break;
		}

		std::map<uint32_t, std::map<uint64_t, uint64_t> >::iterator itr_red = guild_red_packet_time_map.find(guild_id);
		if(itr_red != guild_red_packet_time_map.end())
		{
			std::map<uint64_t, uint64_t> &guild_red = itr_red->second;

			if(guild_red.size() >= sg_red_packet_baocun_max_num)
			{
				uint64_t last_red_uuid = 0;
				uint64_t last_red_time = 0xffffffff;
				for(std::map<uint64_t, uint64_t>::iterator itr = guild_red.begin(); itr != guild_red.end(); itr++)
				{
					if(itr->second < last_red_time)
					{
						last_red_uuid = itr->first;
						last_red_time = itr->second;
					}
				}

				ret = delete_one_red_packet_for_redis(last_red_uuid, sg_guild_red_packet_key, guild_red);
				if(ret != 0)
				{
					LOG_ERR("[%s:%d]delete red packet redis error,last_red_uuid[%lu] send red packet player_id[%lu] red_type[%u] red_coin_type[%u] red_money_num[%u] red_num[%u]", __FUNCTION__, __LINE__, last_red_uuid, extern_data->player_id, red_type, red_coin_type, money_num, red_num);
					ret = 190500584;
					break;
				}
			}

			//数据存redis
			ret = save_one_red_packet_for_redis(&cur_red_packet_info, cur_red_uuid, sg_guild_red_packet_key);
			if(ret != 0)
			{
				LOG_ERR("[%s:%d]save guild red redis faild,player_id[%lu] red_type[%u] red_coin_type[%u] red_money_num[%u] red_num[%u]", __FUNCTION__, __LINE__, extern_data->player_id, red_type, red_coin_type, money_num, red_num);
				ret = 190500584;
				break;
			}
			guild_red.insert(std::make_pair(cur_red_uuid, send_red_times));
			record_player_recive_red_packet_info(extern_data->player_id, cur_red_uuid, red_coin_type, money_num, true);
			break;
		}


		//数据存redis
		ret = save_one_red_packet_for_redis(&cur_red_packet_info, cur_red_uuid, sg_guild_red_packet_key);
		if(ret != 0)
		{
			LOG_ERR("[%s:%d]save guild red redis faild,player_id[%lu] red_type[%u] red_coin_type[%u] red_money_num[%u] red_num[%u]", __FUNCTION__, __LINE__, extern_data->player_id, red_type, red_coin_type, money_num, red_num);
			ret = 190500584;
			break;
		}
		std::map<uint64_t, uint64_t> new_guild_red;
		new_guild_red.clear();
		new_guild_red[cur_red_uuid] = send_red_times;
		guild_red_packet_time_map.insert(std::make_pair(guild_id, new_guild_red));
		record_player_recive_red_packet_info(extern_data->player_id, cur_red_uuid, red_coin_type, money_num, true);

	}while(0);

	//失败就到game服把钱还给玩家
	if(ret != 0)
	{
		RED_PACKET_PLAYER_ADD_MONEY *add_money = (RED_PACKET_PLAYER_ADD_MONEY *)get_send_data();
		uint32_t data_len = sizeof(RED_PACKET_PLAYER_ADD_MONEY);
		memset(add_money, 0, data_len);
	
		add_money->result = ret;
		add_money->money_type = red_coin_type;
		add_money->money_num = money_num;
		fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_TRADE_SEND_RED_PACKET_FAILED_ANSWER, data_len, 0);
	}
	else 
	{
		//广播消息
		Chat send;
		chat__init(&send);

		char str[300] = "";
		sprintf(str, "{type = \"red Package\",id = %lu ,content = \"%s\",title =\"\",effect=\"\" }", cur_red_uuid, cur_red_packet_info.player_text);
		send.contain = str;
		send.sendname = redis_player->name;
		send.sendplayerid = extern_data->player_id;
		send.sendplayerlv = redis_player->lv;
		send.sendplayerjob = redis_player->job;
		send.sendplayerpicture = redis_player->head_icon;
		send.has_sendplayerpicture = true;
		if(red_type == 1)
		{
			send.channel = CHANNEL__world;
			send_to_all_player(MSG_ID_CHAT_NOTIFY, &send, (pack_func)chat__pack);
		}
		else 
		{
			send.channel = CHANNEL__family;
			fast_send_msg(&connecter, extern_data, SERVER_PROTO_GUILD_CHAT, chat__pack, send);//转到帮会服处理
		}
	
	}
	

	return 0;
}

//红包主界面信息请求
int conn_node_tradesrv::handle_red_packet_main_jiemian_info_request(EXTERN_DATA *extern_data)
{
	RedPacketMainInfoRequest *req = red_packet_main_info_request__unpack(NULL, get_data_len(), (uint8_t*)get_data());

	if(req == NULL)
	{
		LOG_ERR("[%s:%d] red packet main info request unpack faild, player_id[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}
	uint32_t red_packet_type = req->red_type;
	red_packet_main_info_request__free_unpacked(req, NULL);
	if(red_packet_type != 1 && red_packet_type != 5)
	{
		LOG_ERR("[%s:%d] get red packet data error, red packet type error, red_type[%u]", __FUNCTION__, __LINE__, red_packet_type);
		return -2;
	}
	AutoReleaseRedisPlayer ar_redis;
	PlayerRedisInfo *redis_player = get_redis_player(extern_data->player_id, sg_player_key, sg_redis_client, ar_redis);
	if(redis_player == NULL)
	{
		LOG_ERR("[%s:%d] get red packet main info faild, get player redis info faild, player_id[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return -3;
	}

	RedPacketMainInfoAnswer answer;
	RedPacketMainInfo red_packet_info[sg_red_packet_baocun_max_num];
	RedPacketMainInfo *red_packet_info_point[sg_red_packet_baocun_max_num];
	red_packet_main_info_answer__init(&answer);


	std::map<uint64_t, uint64_t> red_uuid_map;
	red_uuid_map.clear();
	char red_packet_key[64];
	memset(red_packet_key, 0, 64);
	AutoReleaseBatchRedRedisInfo pool;
	std::set<uint64_t> red_uuid_set;
	std::map<uint64_t, RedPacketRedisInfo *> red_packet_reids_map;
	int ret = 0;
	do{
		if(red_packet_type == 1) //普通红包
		{
			strcpy(red_packet_key, sg_normal_red_packet_key);
			if(normal_red_packet_time_map.empty())
			{
				break;
			}
			red_uuid_map = normal_red_packet_time_map;
		}
		else					 //帮会红包
		{
			strcpy(red_packet_key, sg_guild_red_packet_key);
			if(redis_player->guild_id == 0)//玩家没有帮会
			{
				break;
			}

			std::map<uint32_t, std::map<uint64_t, uint64_t> >::iterator guild_itr = guild_red_packet_time_map.find(redis_player->guild_id);
			if(guild_itr ==  guild_red_packet_time_map.end())
			{
				break;
			}
			red_uuid_map = guild_itr->second;
		}
		if(red_uuid_map.empty())
			break;
		for(std::map<uint64_t, uint64_t>::iterator itr = red_uuid_map.begin(); itr != red_uuid_map.end(); itr++)
		{
			red_uuid_set.insert(itr->first);
		}
		if(get_more_red_packet_redis_info(red_uuid_set, red_packet_reids_map, red_packet_key, sg_redis_client, pool) != 0)
		{
			ret = 190500584;
			LOG_ERR("[%s:%d] player[%lu] get red packet redis data failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		answer.n_red_info = 0;
		for(std::map<uint64_t, RedPacketRedisInfo *>::iterator ite = red_packet_reids_map.begin(); ite != red_packet_reids_map.end() && answer.n_red_info < sg_red_packet_baocun_max_num; ite++)
		{
			red_packet_info_point[answer.n_red_info] = &red_packet_info[answer.n_red_info];
			red_packet_main_info__init(&red_packet_info[answer.n_red_info]);
			red_packet_info[answer.n_red_info].red_id = ite->second->red_uuid;
			red_packet_info[answer.n_red_info].player_id = ite->second->player_id;
			red_packet_info[answer.n_red_info].player_head_id = ite->second->head_icon;
			red_packet_info[answer.n_red_info].player_name = ite->second->player_name;
			red_packet_info[answer.n_red_info].player_level = ite->second->player_level;
			red_packet_info[answer.n_red_info].red_coin_type = ite->second->red_coin_type;
			red_packet_info[answer.n_red_info].player_text = ite->second->player_text;
			red_packet_info[answer.n_red_info].red_money_num = ite->second->red_sum_money;
			for(size_t i = 0; i < ite->second->n_all_player; i++)
			{
				if(extern_data->player_id == ite->second->all_player[i]->player_id)
				{
					red_packet_info[answer.n_red_info].open_flag = 1;
				}
			}
			if(ite->second->red_use_num >= ite->second->red_sum_num && red_packet_info[answer.n_red_info].open_flag == 0)
			{
				red_packet_info[answer.n_red_info].open_flag = 2;
			}
			answer.n_red_info++;
		}
	
	}while(0);

	answer.result = ret;
	answer.red_type = red_packet_type;
	answer.red_info = red_packet_info_point;
	fast_send_msg(&connecter, extern_data, MSG_ID_RED_BACKET_MAIN_JIEMAIN_INFO_ANSWER, red_packet_main_info_answer__pack, answer);
	return 0;
}

//单个红包详情请求
int conn_node_tradesrv::handle_red_packet_detalled_info_request(EXTERN_DATA *extern_data)
{
	RedPacketDetailedInfoRequest *req = red_packet_detailed_info_request__unpack(NULL, get_data_len(), (uint8_t*)get_data());

	if(req == NULL)
	{
		LOG_ERR("[%s:%d] red packet detailed info request unpack faild, player_id[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}
	uint32_t red_type = req->red_type;
	uint64_t red_uuid = req->red_id;
	red_packet_detailed_info_request__free_unpacked(req, NULL);

	if(red_type != 1 && red_type != 5)
	{
		LOG_ERR("[%s:%d] get red packet detailed info failed, red_type error, red_type[%u]", __FUNCTION__, __LINE__, red_type);
		return -2;
	}
	AutoReleaseRedisPlayer ar_redis;
	PlayerRedisInfo *redis_player = get_redis_player(extern_data->player_id, sg_player_key, sg_redis_client, ar_redis);
	if(redis_player == NULL)
	{
		LOG_ERR("[%s:%d] get red packet detailed info faild, get player redis info faild, player_id[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return -3;
	}

	RedPacketDetailedInfoAnswer answer;
	RedPacketPlayerReciveInfo max_player;
	RedPacketPlayerReciveInfo min_player;
	RedPacketPlayerReciveInfo all_player[sg_red_packet_max_can_recive_num];
	RedPacketPlayerReciveInfo* all_player_point[sg_red_packet_max_can_recive_num];
	red_packet_detailed_info_answer__init(&answer);

	char red_packet_key[64];
	memset(red_packet_key, 0, 64);
	AutoReleaseTradeRedisInfo autoR;
	RedPacketRedisInfo *cur_red_info = NULL;
	int ret = 0;

	do{
		if(red_type == 5 && redis_player->guild_id == 0)
		{
			ret = 190500583;
			break;
		}
		if(red_type == 1)
		{
			std::map<uint64_t, uint64_t>::iterator itr = normal_red_packet_time_map.find(red_uuid);
			if(itr == normal_red_packet_time_map.end())
			{
				ret = 190500583;
				break;
			}
			strcpy(red_packet_key, sg_normal_red_packet_key);
		}
		else 
		{
			std::map<uint32_t, std::map<uint64_t, uint64_t> >::iterator itr = guild_red_packet_time_map.find(redis_player->guild_id);
			if(itr == guild_red_packet_time_map.end())
			{
				ret = 190500583;
				break;
			}
			std::map<uint64_t, uint64_t> guild_red_packet = itr->second;
			
			std::map<uint64_t, uint64_t>::iterator ite = guild_red_packet.find(red_uuid);
			if(ite == guild_red_packet.end())
			{
				ret = 190500583;
				break;
			}
			strcpy(red_packet_key, sg_guild_red_packet_key);
		}

		//到redis取数据
		cur_red_info = get_red_packet_redis_info(red_uuid, red_packet_key, sg_redis_client, autoR);
		if(cur_red_info == NULL)
		{
			LOG_ERR("[%s:%d] get red packet detailed failed, get redis data error, red_packet_type[%u] red_packet_uuid[%lu] player[%lu]", __FUNCTION__, __LINE__, red_type, red_uuid, extern_data->player_id);
			ret = 190500584;
			break;
		}
		answer.red_type = red_type;
		answer.red_id = red_uuid;
		answer.red_money_num = cur_red_info->red_sum_money;
		answer.red_money_use = cur_red_info->red_use_money;
		answer.red_sum_num = cur_red_info->red_sum_num;
		answer.red_use_num = cur_red_info->red_use_num;
		if(cur_red_info->max_player != NULL)
		{
			answer.max_player = &max_player;
			red_packet_player_recive_info__init(&max_player);
			max_player.player_name = cur_red_info->max_player->player_name;
			max_player.money_num = cur_red_info->max_player->money_num;
		}
		if(cur_red_info->min_player != NULL)
		{
			answer.min_player = &min_player;
			red_packet_player_recive_info__init(&min_player);
			min_player.player_name = cur_red_info->min_player->player_name;
			min_player.money_num = cur_red_info->min_player->money_num;
		}
		answer.n_all_player = 0;
		answer.all_player = all_player_point;
		for(size_t i = 0; i < sg_red_packet_max_can_recive_num && i < cur_red_info->n_all_player; i++)
		{
			all_player_point[answer.n_all_player] = &all_player[answer.n_all_player];
			red_packet_player_recive_info__init(&all_player[answer.n_all_player]);
			all_player[answer.n_all_player].player_name =  cur_red_info->all_player[i]->player_name;
			all_player[answer.n_all_player].money_num =  cur_red_info->all_player[i]->money_num;
			answer.n_all_player++;
		}
	
	}while(0);
	answer.result = ret;
	fast_send_msg(&connecter, extern_data, MSG_ID_RED_BACKET_DETAILED_INFO_ANSWER, red_packet_detailed_info_answer__pack, answer);
	return 0;
}

//抢红包
int conn_node_tradesrv::handle_red_packet_grab_red_packet_request(EXTERN_DATA *extern_data)
{
	RedPacketGrabRedRequest *req = red_packet_grab_red_request__unpack(NULL, get_data_len(), (uint8_t*)get_data());

	if(req == NULL)
	{
		LOG_ERR("[%s:%d] red packet grab red  request unpack faild, player_id[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}
	uint32_t red_type = req->red_type;
	uint32_t money_type = 0;
	uint64_t red_uuid = req->red_id;
	uint64_t my_player_id = extern_data->player_id;
	red_packet_grab_red_request__free_unpacked(req, NULL);
	if(red_type != 1 && red_type != 5)
	{
		LOG_ERR("[%s:%d] grab red packet faild, red packet type error red_type[%u]", __FUNCTION__, __LINE__, red_type);
		return -2;
	}
	AutoReleaseRedisPlayer ar_redis;
	PlayerRedisInfo *redis_player = get_redis_player(my_player_id, sg_player_key, sg_redis_client, ar_redis);
	if(redis_player == NULL)
	{
		LOG_ERR("[%s:%d] grab red packet faild, get player redis info faild, player_id[%lu]", __FUNCTION__, __LINE__, my_player_id);
		return -3;
	}

	char red_packet_key[64];
	uint32_t give_player_money = 0;
	memset(red_packet_key, 0, 64);
	AutoReleaseTradeRedisInfo autoR;
	RedPacketRedisInfo *cur_red_info = NULL;
	RedPacketRedisPlayerReciveInfo all_recive_player[sg_red_packet_max_can_recive_num];
	RedPacketRedisPlayerReciveInfo* all_recive_player_point[sg_red_packet_max_can_recive_num];
	RedPacketRedisPlayerReciveInfo max_money_player;
	RedPacketRedisPlayerReciveInfo min_money_player;

	RedPacketRedisInfo updata_red_info;
	red_packet_redis_info__init(&updata_red_info);

	RedPacketGrabRedAnswer answer;
	RedPacketMainInfo red_packet_main_info;
	red_packet_grab_red_answer__init(&answer);
	answer.info = &red_packet_main_info;
	red_packet_main_info__init(&red_packet_main_info);
	int ret = 0;
	do{
		if(redis_player->lv < sg_grab_red_packet_min_level)
		{
			ret = 190500579;
			break;
		}
		if(red_type == 1)
		{
			if(normal_red_packet_time_map.find(red_uuid) == normal_red_packet_time_map.end())
			{
				LOG_INFO("[%s:%d] normal red packet info error faild, player_id[%lu]", __FUNCTION__, __LINE__, my_player_id);
				ret = 190500583;
				break;
			}
			strcpy(red_packet_key, sg_normal_red_packet_key);
		}
		else 
		{
			std::map<uint32_t, std::map<uint64_t, uint64_t> >::iterator iter = guild_red_packet_time_map.find(redis_player->guild_id);

			if(iter == guild_red_packet_time_map.end())
			{
				LOG_INFO("[%s:%d] guild red packet info error faild, player_id[%lu] guild_id[%u]", __FUNCTION__, __LINE__, my_player_id, redis_player->guild_id);
				ret = 190500583;
				break;
			
			}

			std::map<uint64_t, uint64_t> cur_guild_red_packet = iter->second;
			if(cur_guild_red_packet.find(red_uuid) == cur_guild_red_packet.end())
			{
				LOG_INFO("[%s:%d] guild red packet info error faild, player_id[%lu] guild_id[%u]", __FUNCTION__, __LINE__, my_player_id, redis_player->guild_id);
				ret = 190500583;
				break;
			
			}
			strcpy(red_packet_key, sg_guild_red_packet_key);
		}
		//到redis取数据
		cur_red_info = get_red_packet_redis_info(red_uuid, red_packet_key, sg_redis_client, autoR);
		if(cur_red_info == NULL)
		{
			LOG_ERR("[%s:%d] grab red packet  failed, get redis data error, red_packet_type[%u] red_packet_uuid[%lu] player[%lu]", __FUNCTION__, __LINE__, red_type, red_uuid, my_player_id);
			ret = 190500584;
			break;
		}
		red_packet_main_info.red_coin_type = cur_red_info->red_coin_type;
		red_packet_main_info.red_id = cur_red_info->red_uuid;
		red_packet_main_info.open_flag = 0;
		red_packet_main_info.player_id = cur_red_info->player_id;
		red_packet_main_info.player_level = cur_red_info->player_level;
		red_packet_main_info.player_head_id = cur_red_info->head_icon;
		red_packet_main_info.player_name = cur_red_info->player_name;
		red_packet_main_info.player_text = cur_red_info->player_text;
		for(size_t i = 0; i < sg_red_packet_max_can_recive_num && i < cur_red_info->n_all_player; i++)
		{
			if(cur_red_info->all_player[i]->player_id == my_player_id)
			{
				red_packet_main_info.open_flag = 1;
				LOG_INFO("[%s:%d] grad red packet faild,already recive the red packet player_id[%lu] red_type[%u] red_uuild[%lu]", __FUNCTION__, __LINE__, my_player_id, red_type, red_uuid);
				ret = 190500583;
				break;
			}
		}
		if(ret != 0)
		{
			break;
		}

		uint32_t red_packet_shengyu_money = cur_red_info->red_sum_money - cur_red_info->red_use_money;  //红包里面剩余的钱
		uint32_t red_packet_shengyu_num = cur_red_info->red_sum_num - cur_red_info->red_use_num;        //红包剩余个数
		if(red_packet_shengyu_money < red_packet_shengyu_num)
		{
			LOG_ERR("[%s:%d] grab red packet  failed, red packet data error, red_packet_type[%u] red_packet_uuid[%lu] sum_money[%u] shengyu_money[%u] sum_num[%u] shengyu_num[%u]", __FUNCTION__, __LINE__, red_type, red_uuid, cur_red_info->red_sum_money, red_packet_shengyu_money, cur_red_info->red_sum_num, red_packet_shengyu_num);
			ret = 190500584;
			break;
		}
		if(red_packet_shengyu_money <= 0 || red_packet_shengyu_num <= 0)
		{
			red_packet_main_info.open_flag = 2;
			ret = 190500580;
			break;
		}

		//给钱
		if(red_packet_shengyu_num == 1)
		{
			give_player_money = red_packet_shengyu_money;
		}
		else 
		{
			uint32_t sui_ji_money = red_packet_shengyu_money / red_packet_shengyu_num * 2;
			if(sui_ji_money <= red_packet_shengyu_num)
			{
				sui_ji_money = 1;
			}
			else 
			{
				sui_ji_money = sui_ji_money - red_packet_shengyu_num;
			}
			give_player_money = sui_ji_money % rand() +1;
		}
		if(give_player_money == 0)
		{
			LOG_ERR("[%s:%d] grab red packet failed, suiji money num  error, red_packet_type[%u] red_packet_uuid[%lu] shengyu_money[%u] shengyu_num[%u] give_monsy[%u]", __FUNCTION__, __LINE__, red_type, red_uuid, red_packet_shengyu_money, red_packet_shengyu_num, give_player_money);
			ret = 190500584;
			break;
		}
		if(sg_red_packet_max_can_recive_num <= 0 || sg_red_packet_max_can_recive_num < cur_red_info->n_all_player)
		{
			LOG_ERR("[%s:%d] grab red packet failed, max can recive num parameter error, red_packet_type[%u] red_packet_uuid[%lu] max_recive_num[%u]", __FUNCTION__, __LINE__, red_type, red_uuid, sg_red_packet_max_can_recive_num);
			ret = 190500584;
			break;
		}
		
		updata_red_info.red_uuid = cur_red_info->red_uuid;
		updata_red_info.player_id = cur_red_info->player_id;
		updata_red_info.player_name = cur_red_info->player_name;
		updata_red_info.head_icon = cur_red_info->head_icon;
		updata_red_info.player_level = cur_red_info->player_level;
		updata_red_info.send_red_time = cur_red_info->send_red_time;
		updata_red_info.red_typ = cur_red_info->red_typ;
		updata_red_info.guild_id = cur_red_info->guild_id;
		updata_red_info.red_coin_type = cur_red_info->red_coin_type;
		updata_red_info.red_sum_money = cur_red_info->red_sum_money;
		updata_red_info.red_use_money = cur_red_info->red_use_money + give_player_money;
		updata_red_info.red_sum_num = cur_red_info->red_sum_num;
		updata_red_info.red_use_num = cur_red_info->red_use_num + 1;
		updata_red_info.player_text = cur_red_info->player_text;
		updata_red_info.max_player = &max_money_player;
		red_packet_redis_player_recive_info__init(&max_money_player);
		updata_red_info.min_player = &min_money_player;
		red_packet_redis_player_recive_info__init(&min_money_player);
		if(cur_red_info->max_player == NULL)
		{
			max_money_player.player_id = my_player_id;
			max_money_player.player_name = redis_player->name;
			max_money_player.money_num = give_player_money;
		}
		else 
		{
			if(give_player_money > cur_red_info->max_player->money_num)
			{
				max_money_player.player_id = my_player_id;
				max_money_player.player_name = redis_player->name;
				max_money_player.money_num = give_player_money;
				
			}
			else 
			{
				max_money_player.player_id = cur_red_info->max_player->player_id;
				max_money_player.player_name = cur_red_info->max_player->player_name;
				max_money_player.money_num = cur_red_info->max_player->money_num;
			
			}
		}
		if(cur_red_info->min_player == NULL)
		{
			min_money_player.player_id = my_player_id;
			min_money_player.player_name = redis_player->name;
			min_money_player.money_num = give_player_money;
		}
		else 
		{
			if(give_player_money < cur_red_info->min_player->money_num)
			{
				min_money_player.player_id = my_player_id;
				min_money_player.player_name = redis_player->name;
				min_money_player.money_num = give_player_money;
				
			}
			else 
			{
				min_money_player.player_id = cur_red_info->min_player->player_id;
				min_money_player.player_name = cur_red_info->min_player->player_name;
				min_money_player.money_num = cur_red_info->min_player->money_num;
			
			}
		}
		//将玩家领取信息加入领取详情
		updata_red_info.n_all_player = 0;
		updata_red_info.all_player = all_recive_player_point;
		for(; updata_red_info.n_all_player < cur_red_info->n_all_player && updata_red_info.n_all_player < sg_red_packet_max_can_recive_num;)
		{
			all_recive_player_point[updata_red_info.n_all_player] = &all_recive_player[updata_red_info.n_all_player];
			red_packet_redis_player_recive_info__init(&all_recive_player[updata_red_info.n_all_player]);
			all_recive_player[updata_red_info.n_all_player].player_id = cur_red_info->all_player[updata_red_info.n_all_player]->player_id;
			all_recive_player[updata_red_info.n_all_player].player_name = cur_red_info->all_player[updata_red_info.n_all_player]->player_name;
			all_recive_player[updata_red_info.n_all_player].money_num = cur_red_info->all_player[updata_red_info.n_all_player]->money_num;
			updata_red_info.n_all_player++;
		}
		if(updata_red_info.n_all_player < sg_red_packet_max_can_recive_num)
		{
			all_recive_player_point[updata_red_info.n_all_player] = &all_recive_player[updata_red_info.n_all_player];
			red_packet_redis_player_recive_info__init(&all_recive_player[updata_red_info.n_all_player]);
			all_recive_player[updata_red_info.n_all_player].player_id = my_player_id;
			all_recive_player[updata_red_info.n_all_player].player_name = redis_player->name;
			all_recive_player[updata_red_info.n_all_player].money_num = give_player_money;
			updata_red_info.n_all_player++;
		}
		//数据存redis
		uint8_t data_buffer[1024 * 1024];
		size_t data_len = red_packet_redis_info__pack(&updata_red_info, data_buffer);
		if (data_len == (size_t)-1)
		{
			LOG_ERR("[%s:%d]grad red packet failed, save red redis faild, pack error, player_id[%lu] red_type[%u] red_uuid[%lu]", __FUNCTION__, __LINE__, my_player_id, red_type, red_uuid);
			ret = 190500584;
			break;
		}
		char red_field[64];
		sprintf(red_field, "%lu", red_uuid);
		if (sg_redis_client.hset_bin(red_packet_key, red_field, (const char *)data_buffer, (int)data_len) < 0)
		{
			LOG_ERR("[%s:%d]grad red packet failed, save red redis faild, pack error, player_id[%lu] red_type[%u] red_uuid[%lu]", __FUNCTION__, __LINE__, my_player_id, red_type, red_uuid);
			ret = 190500584;
			break;
		}

		money_type = cur_red_info->red_coin_type;

		//存玩家领取记录
		record_player_recive_red_packet_info(my_player_id, red_uuid, money_type, give_player_money, false);
		if(updata_red_info.max_player != NULL && updata_red_info.red_use_num >= updata_red_info.red_sum_num)
		{
			modify_player_red_packet_optimum_record(updata_red_info.max_player->player_id, red_uuid);
		}

	}while(0);

	if(ret == 0)
	{
		LOG_INFO("[%s:%d] grad red packet success, player_id[%lu] red_type[%u] red_money_type[%u]  red_uuid[%lu] get_money_num[%u]", __FUNCTION__, __LINE__, my_player_id, red_type, money_type, red_uuid, give_player_money);
		RED_PACKET_PLAYER_ADD_MONEY *add_money = (RED_PACKET_PLAYER_ADD_MONEY *)get_send_data();
		uint32_t data_len = sizeof(RED_PACKET_PLAYER_ADD_MONEY);
		memset(add_money, 0, data_len);
	
		add_money->result = 0;
		add_money->money_type = money_type;
		add_money->money_num = give_player_money;
		fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_TRADE_GRAB_RED_PACKET_REQUEST, data_len, 0);
	
	}


	answer.result = ret;
	answer.money_num = give_player_money;
	fast_send_msg(&connecter, extern_data, MSG_ID_RED_BACKET_QIANG_HONGBAO_ANSWER, red_packet_grab_red_answer__pack, answer);
	return 0;
}

//玩家领取和发送红包信息记录redis
int conn_node_tradesrv::record_player_recive_red_packet_info(uint64_t player_id, uint64_t red_uuid, uint32_t money_type, uint32_t money_num, bool is_send_red_packet)
{
	if(player_id <= 0 || sg_red_packet_jilu_max_num <= 0 || red_uuid <= 0)
	{
		LOG_ERR("[%s:%d] 玩家领红包信息记录redis失败,玩家id有问题player_id[%lu] 或者记录存储最大值有问题jilu_max_num[%u] 或者红包唯一id有问题[%lu] recive_money_type[%u] recive_money_num[%u]", __FUNCTION__, __LINE__, player_id, sg_red_packet_jilu_max_num, red_uuid, money_type, money_num);
		return -1;
	}
	//记录玩家录取红包的信息
	int player_redis_exist = sg_redis_client.exist(sg_player_red_packet_record_key, player_id);
	if(player_redis_exist < 0)
	{
		LOG_ERR("[%s:%d] 玩家领红包信息记录redis失败,redis信息出错player_id[%lu] recive_money_type[%u] recive_money_num[%u]", __FUNCTION__, __LINE__, player_id, money_type, money_num);
		return -2;
	}
	uint64_t recive_red_times = time_helper::get_cached_time() / 1000;
	RedPacketRedisPlayerReciveRecord player_info_record;
	red_packet_redis_player_recive_record__init(&player_info_record);
	RedPacketRedisPlayeNormalInfo base_info[sg_red_packet_jilu_max_num];
	RedPacketRedisPlayeNormalInfo* base_info_point[sg_red_packet_jilu_max_num];
	//不存在直接插入
	if(player_redis_exist == 0)
	{
		player_info_record.n_info = 0;
		player_info_record.info = base_info_point;
		base_info_point[player_info_record.n_info] = &base_info[player_info_record.n_info];
		red_packet_redis_playe_normal_info__init(&base_info[player_info_record.n_info]);
		base_info[player_info_record.n_info].red_uuid = red_uuid;
		base_info[player_info_record.n_info].grab_time = recive_red_times;
		base_info[player_info_record.n_info].money_type = money_type;
		base_info[player_info_record.n_info].money_num = money_num;
		if(is_send_red_packet)
		{
			base_info[player_info_record.n_info].red_type = 2;
		}
		else 
		{
			base_info[player_info_record.n_info].red_type = 0;
		}
		player_info_record.n_info++;
		//数据存redis
		uint8_t data_buffer[1024 * 1024];
		size_t data_len = red_packet_redis_player_recive_record__pack(&player_info_record, data_buffer);
		if (data_len == (size_t)-1)
		{
			LOG_ERR("[%s:%d] 玩家领红包信息记录redis失败,打包数据失败player_id[%lu] recive_money_type[%u] recive_money_num[%u]", __FUNCTION__, __LINE__, player_id, money_type, money_num);
			return -3;
		}
		char red_field[64];
		sprintf(red_field, "%lu", player_id);
		if (sg_redis_client.hset_bin(sg_player_red_packet_record_key, red_field, (const char *)data_buffer, (int)data_len) < 0)
		{
			LOG_ERR("[%s:%d] 玩家领红包信息记录redis失败,存储失败player_id[%lu] recive_money_type[%u] recive_money_num[%u]", __FUNCTION__, __LINE__, player_id, money_type, money_num);
			return -4;
		}
		return 0;
	}

	//存在，就把数据读出来改变
	AutoReleaseTradeRedisInfo pool;
	RedPacketRedisPlayerReciveRecord *player_recive_info = get_player_red_packet_redis_recive_record(player_id, sg_player_red_packet_record_key, sg_redis_client, pool);
	if(player_recive_info == NULL)
	{
		LOG_ERR("[%s:%d] 玩家领红包信息记录redis失败,获取玩家redis信息失败player_id[%lu] recive_money_type[%u] recive_money_num[%u]", __FUNCTION__, __LINE__, player_id, money_type, money_num);
		return -5;
	}
	if(player_recive_info->n_info < sg_red_packet_jilu_max_num)
	{
		player_info_record.n_info = 0;
		player_info_record.info = base_info_point;
		for(; player_info_record.n_info < player_recive_info->n_info;)
		{
			base_info_point[player_info_record.n_info] = &base_info[player_info_record.n_info];
			red_packet_redis_playe_normal_info__init(&base_info[player_info_record.n_info]);
			base_info[player_info_record.n_info].red_uuid = player_recive_info->info[player_info_record.n_info]->red_uuid;
			base_info[player_info_record.n_info].grab_time = player_recive_info->info[player_info_record.n_info]->grab_time;
			base_info[player_info_record.n_info].money_type = player_recive_info->info[player_info_record.n_info]->money_type;
			base_info[player_info_record.n_info].money_num = player_recive_info->info[player_info_record.n_info]->money_num;
			base_info[player_info_record.n_info].red_type = player_recive_info->info[player_info_record.n_info]->red_type;
			player_info_record.n_info++;
		}
		if(player_info_record.n_info < sg_red_packet_jilu_max_num)
		{
			base_info_point[player_info_record.n_info] = &base_info[player_info_record.n_info];
			red_packet_redis_playe_normal_info__init(&base_info[player_info_record.n_info]);
			base_info[player_info_record.n_info].red_uuid = red_uuid;
			base_info[player_info_record.n_info].grab_time = recive_red_times;
			base_info[player_info_record.n_info].money_type = money_type;
			base_info[player_info_record.n_info].money_num = money_num;
			if(is_send_red_packet)
			{
				base_info[player_info_record.n_info].red_type = 2;
			}
			else 
			{
				base_info[player_info_record.n_info].red_type = 0;
			}
			player_info_record.n_info++;
		}

	}
	else 
	{
		RedPacketRedisPlayeNormalInfo* temp_info = NULL;

		player_info_record.n_info = 0;
		player_info_record.info = base_info_point;
		for(; player_info_record.n_info < player_recive_info->n_info;)
		{
			base_info_point[player_info_record.n_info] = &base_info[player_info_record.n_info];
			red_packet_redis_playe_normal_info__init(&base_info[player_info_record.n_info]);
			base_info[player_info_record.n_info].red_uuid = player_recive_info->info[player_info_record.n_info]->red_uuid;
			base_info[player_info_record.n_info].grab_time = player_recive_info->info[player_info_record.n_info]->grab_time;
			base_info[player_info_record.n_info].money_type = player_recive_info->info[player_info_record.n_info]->money_type;
			base_info[player_info_record.n_info].money_num = player_recive_info->info[player_info_record.n_info]->money_num;
			base_info[player_info_record.n_info].red_type = player_recive_info->info[player_info_record.n_info]->red_type;
			if(temp_info == NULL)
			{
				temp_info = &base_info[player_info_record.n_info];
			}
			else 
			{
				if(temp_info->grab_time < base_info[player_info_record.n_info].grab_time)
				{
					temp_info = &base_info[player_info_record.n_info];
				}
			}
			player_info_record.n_info++;
		}


		if(temp_info != NULL)
		{
			temp_info->red_uuid = red_uuid;
			temp_info->grab_time = recive_red_times;
			temp_info->money_type = money_type;
			temp_info->money_num = money_num;
			if(is_send_red_packet)
			{
				temp_info->red_type = 2;
			}
			else 
			{
				temp_info->red_type = 0;
			}
		}
	
	}
	//数据存redis
	uint8_t data_buffer[1024 * 1024];
	size_t data_len = red_packet_redis_player_recive_record__pack(&player_info_record, data_buffer);
	if (data_len == (size_t)-1)
	{
		LOG_ERR("[%s:%d] 玩家领红包信息记录redis失败,打包数据失败player_id[%lu] recive_money_type[%u] recive_money_num[%u]", __FUNCTION__, __LINE__, player_id, money_type, money_num);
		return -5;
	}
	char red_field[64];
	sprintf(red_field, "%lu", player_id);
	if (sg_redis_client.hset_bin(sg_player_red_packet_record_key, red_field, (const char *)data_buffer, (int)data_len) < 0)
	{
		LOG_ERR("[%s:%d] 玩家领红包信息记录redis失败,存储失败player_id[%lu] recive_money_type[%u] recive_money_num[%u]", __FUNCTION__, __LINE__, player_id, money_type, money_num);
		return -6;
	}

	return 0;
}

//修改玩家领取红包最佳记录
int conn_node_tradesrv::modify_player_red_packet_optimum_record(uint64_t player_id, uint64_t red_uuid)
{
	AutoReleaseTradeRedisInfo pool;
	RedPacketRedisPlayerReciveRecord *player_recive_info = get_player_red_packet_redis_recive_record(player_id, sg_player_red_packet_record_key, sg_redis_client, pool);
	if(player_recive_info == NULL)
	{
		LOG_ERR("[%s:%d] 更新玩家手气最佳红包信息失败,获取玩家redis信息失败player_id[%lu] red_uuid[%lu]", __FUNCTION__, __LINE__, player_id, red_uuid);
		return -1;
	}
	for(size_t i = 0; i < player_recive_info->n_info; i++)
	{
		if(player_recive_info->info[i]->red_uuid == red_uuid && player_recive_info->info[i]->red_type == 0)
		{
			player_recive_info->info[i]->red_type = 1;
		}
	}
	//数据存redis
	uint8_t data_buffer[1024 * 1024];
	size_t data_len = red_packet_redis_player_recive_record__pack(player_recive_info, data_buffer);
	if (data_len == (size_t)-1)
	{
		LOG_ERR("[%s:%d] 更新玩家手气最佳红包信息失败,打包数据失败player_id[%lu] red_uuid[%lu]", __FUNCTION__, __LINE__, player_id, red_uuid);
		return -2;
	}
	char red_field[64];
	sprintf(red_field, "%lu", player_id);
	if (sg_redis_client.hset_bin(sg_player_red_packet_record_key, red_field, (const char *)data_buffer, (int)data_len) < 0)
	{
		LOG_ERR("[%s:%d} 更新玩家手气最佳红包信息失败,存储失败player_id[%lu] red_uuid[%lu]", __FUNCTION__, __LINE__, player_id, red_uuid);
		return -3;
	}
	return 0;
}

int conn_node_tradesrv::handle_red_packet_recive_record_request(EXTERN_DATA *extern_data)
{
	AutoReleaseTradeRedisInfo pool;
	RedPacketRedisPlayerReciveRecord *player_recive_info = get_player_red_packet_redis_recive_record(extern_data->player_id, sg_player_red_packet_record_key, sg_redis_client, pool);
	if(player_recive_info == NULL)
	{
		LOG_ERR("[%s:%d] 请求玩家红包详情失败,获取玩家redis信息失败player_id[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	RedPacketHistoryInfoAnswer answer;
	RedPacketPlayerRedInfo base_info[sg_red_packet_jilu_max_num];
	RedPacketPlayerRedInfo *base_info_point[sg_red_packet_jilu_max_num];
	red_packet_history_info_answer__init(&answer);
	answer.n_info = 0;
	answer.info = base_info_point;
	for(; answer.n_info < sg_red_packet_jilu_max_num && answer.n_info < player_recive_info->n_info;)
	{
		base_info_point[answer.n_info] = &base_info[answer.n_info];
		red_packet_player_red_info__init(&base_info[answer.n_info]);
		base_info[answer.n_info].type = player_recive_info->info[answer.n_info]->red_type;
		base_info[answer.n_info].red_coin_type = player_recive_info->info[answer.n_info]->money_type;
		base_info[answer.n_info].money_num = player_recive_info->info[answer.n_info]->money_num;
		base_info[answer.n_info].time = player_recive_info->info[answer.n_info]->grab_time;
		answer.n_info++;
	}
	fast_send_msg(&connecter, extern_data, MSG_ID_RED_BACKET_HISTORY_INFO_ANSWER, red_packet_history_info_answer__pack, answer);
	return 0;
}

