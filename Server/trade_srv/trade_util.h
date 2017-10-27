#ifndef _TRADE_UTIL_H__
#define _TRADE_UTIL_H__

#include "trade_struct.h"
#include "trade_config.h"
#include "redis_client.h"
#include <string>
#include <vector>
#include <set>
#include <map>
#include <list>

typedef size_t(*pack_func)(const void *message, uint8_t *out);

extern CRedisClient sg_redis_client;
extern uint32_t sg_server_id;
extern char sg_player_key[];
extern TradeItemMap trade_item_map; //所有交易道具
extern AuctionLotMap auction_lot_map; //所有拍卖道具

void handle_daily_reset_timeout(bool start);
void cb_second_timer(evutil_socket_t, short, void* /*arg*/);

void load_trade_module(void);
int save_trade_item(TradeItem *item, bool insert = false);
int save_trade_player(TradePlayer *player);
int delete_trade_item(uint64_t player_id, uint32_t shelf_index);
void clear_module_memory(void); //回收内存

TradePlayer *get_trade_player(uint64_t player_id);
bool player_is_online(uint64_t player_id);

void add_sold_info_to_average_map(uint32_t item_id, uint32_t sold_num, uint32_t sold_price);
void clear_sold_average_map(void);
uint32_t get_trade_item_average_price(uint32_t item_id);
int check_trade_item_price_rational(uint32_t item_id, uint32_t price);

TradeItem *create_trade_item(TradePlayer *player, uint32_t shelf_index, uint32_t item_id, uint32_t num, uint32_t price, EspecialItemInfo *especial);
void remove_trade_item(TradeItem *&item);

void update_trade_item_summary_broadcast(uint32_t item_id);
void update_trade_shelf_notify(TradeItem *item);
void delete_trade_shelf_notify(uint64_t player_id, uint32_t shelf_index);
int get_trade_state(uint32_t state);
void set_trade_state(uint32_t &state, uint32_t state_new);
int get_trade_operate(uint32_t state);
void set_trade_operate(uint32_t &state, uint32_t operate);

int get_trade_aduit_time(void);

int save_auction_lot(AuctionLot *lot, bool insert = false);
int insert_auction_bid(uint64_t lot_uuid, uint64_t player_id, uint64_t price);
int delete_auction_lot_from_db(uint64_t lot_uuid);

uint64_t alloc_lot_uuid(void); //分配拍卖品的唯一ID
AuctionLot *create_auction_lot(uint32_t lot_id, uint32_t type, uint32_t type_limit, uint64_t *masters, uint32_t master_num);
void remove_auction_lot(AuctionLot *&lot, bool lot_map_erase);
AuctionLot *get_auction_lot(uint64_t uuid);
int get_player_auction_lot_state(AuctionLot *lot, uint64_t player_id); //根据拍卖品查找玩家的竞价状态
void send_player_auction_bid_fail(uint64_t player_id, uint32_t price);

#endif
