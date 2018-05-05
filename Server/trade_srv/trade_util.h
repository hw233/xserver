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
#include "trade_db.pb-c.h"
#include "player_redis_info.pb-c.h"

#define RED_PACKET_TYPE_UUID 1 //红包
union uuid_data
{
	uint64_t data1;
	struct
	{
		uint32_t time : 32;		
		uint32_t auto_inc : 18;
		uint32_t type : 4;
	} data2;
};

typedef size_t(*pack_func)(const void *message, uint8_t *out);

extern CRedisClient sg_redis_client;
extern uint32_t sg_server_id;
extern char sg_player_key[];
extern char sg_normal_red_packet_key[];
extern char sg_guild_red_packet_key[];
extern char sg_player_red_packet_record_key[];
extern char sg_player_red_packet_all_history_key[];
extern TradeItemMap trade_item_map; //所有交易道具
extern AuctionLotMap auction_lot_map; //所有拍卖道具
extern std::map<uint64_t, uint64_t> normal_red_packet_time_map;//普通红包唯一id对应发红包时间
extern std::map<uint32_t, std::map<uint64_t, uint64_t> > guild_red_packet_time_map; //帮会id对应本帮会所有红包

class AutoReleaseTradeRedisInfo
{
public:
	AutoReleaseTradeRedisInfo();
	~AutoReleaseTradeRedisInfo();
	void set_cur_red_packet(RedPacketRedisInfo* r);
	void set_player_red_packet_record(RedPacketRedisPlayerReciveRecord* r);
	void set_player_red_packet_history(RedPacketRedisPlayerAllJiluInfo *r);
private:
	RedPacketRedisInfo *red_packet_redis_info;
	RedPacketRedisPlayerReciveRecord *player_recive_record;
	RedPacketRedisPlayerAllJiluInfo *player_all_hisotry;
};

class AutoReleaseBatchRedRedisInfo
{
public:
	AutoReleaseBatchRedRedisInfo();
	~AutoReleaseBatchRedRedisInfo();

	void push_back(RedPacketRedisInfo *player);
private:
	std::vector<RedPacketRedisInfo *> pointer_vec;
};

RedPacketRedisInfo *get_red_packet_redis_info(uint64_t red_uuid, char *red_packet_key, CRedisClient &rc,AutoReleaseTradeRedisInfo &_pool);
int get_more_red_packet_redis_info(std::set<uint64_t> &red_uuid, std::map<uint64_t, RedPacketRedisInfo*> &redis_players, char *red_packet_key, CRedisClient &rc, AutoReleaseBatchRedRedisInfo &_pool);
RedPacketRedisPlayerReciveRecord *get_player_red_packet_redis_recive_record(uint64_t player_id, char *red_packet_key, CRedisClient &rc, AutoReleaseTradeRedisInfo &_pool);
RedPacketRedisPlayerAllJiluInfo *get_player_red_packet_redis_all_history_record(uint64_t player_id, char *red_packet_key, CRedisClient &rc, AutoReleaseTradeRedisInfo &_pool);

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
uint64_t alloc_red_packet_uuid();
void load_red_packet_redis_data();
//将过期的红包里面剩余的金钱还给玩家
void red_packet_surplus_money_give_back_player(uint64_t player_id, uint32_t red_type, uint32_t money_type, uint64_t send_time, uint32_t sum_money, uint32_t use_money);
//定时更新过时红包信息
void refresh_all_red_packet_redis_data();
//加载所有红包uuid->time数据
void load_red_packet_redis_data();
int delete_one_red_packet_for_redis(uint64_t last_red_uuid ,char* red_packet_key, std::map<uint64_t, uint64_t> &red_map);
int save_one_red_packet_for_redis(RedPacketRedisInfo *redis_info, uint64_t red_uuid, char* red_packet_key);
//修改玩家领取红包最佳记录
int modify_player_red_packet_optimum_record(uint64_t player_id, uint64_t red_uuid);
//更新玩家历史红包记录
void updata_player_red_packet_history_info(RedPacketRedisPlayeNormalInfo* temp_info, uint64_t player_id);
//增加玩家历史记录里面的手气最佳个数
void add_player_red_packet_history_max_num(uint64_t player_id);
PlayerRedisInfo *find_redis_from_map(std::map<uint64_t, PlayerRedisInfo*> &redis_players, uint64_t player_id);
//广播给所有在线玩家
void broadcast_message_to_online_player(uint16_t msg_id, void *msg_data, pack_func packer);

#endif
