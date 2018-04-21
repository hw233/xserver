#include "trade_util.h"
#include "mysql_module.h"
#include "trade_db.pb-c.h"
#include "trade.pb-c.h"
#include "time_helper.h"
#include "conn_node_tradesrv.h"
#include "msgid.h"
#include "error_code.h"
#include "redis_util.h"
#include "app_data_statis.h"
#include <list>
#include <sstream>

extern int send_mail(conn_node_base *connecter, uint64_t player_id, uint32_t type,
	char *title, char *sender_name, char *content, std::vector<char *> *args,
	std::map<uint32_t, uint32_t> *attachs, uint32_t statis_id);

TradeItemMap trade_item_map; //所有交易道具
TradePlayerMap trade_player_map; //交易服玩家数据
std::map<uint64_t, uint64_t> normal_red_packet_time_map; //普通红包唯一id对应发红包时间
std::map<uint32_t, std::map<uint64_t, uint64_t> > guild_red_packet_time_map; //帮会id对应本帮会所有红包
static TradeSoldMap trade_sold_map; //所有已售道具信息
static std::list<TradeItem *> trade_item_free_list;
static std::list<TradeSoldInfo *> trade_sold_free_list;

AuctionLotMap auction_lot_map; //所有拍卖道具
static AuctionBidMap auction_bid_map; //拍卖竞价列表
static std::list<AuctionLot *> auction_lot_free_list;

char sg_player_key[64];
char sg_normal_red_packet_key[64];
char sg_guild_red_packet_key[64];
char sg_player_red_packet_record_key[64];
char sg_player_red_packet_all_history_key[64];
struct event second_timer;
struct timeval second_timeout;
struct event five_oclock_timer;
struct timeval five_oclock_timeout;
static const uint32_t week_reset_day = 1 * 24 * 3600; //每周一刷新
static const uint32_t daily_reset_clock = 5 * 3600; //每天五点刷新

AutoReleaseTradeRedisInfo::AutoReleaseTradeRedisInfo()
{
	red_packet_redis_info = NULL;
	player_recive_record = NULL;
	player_all_hisotry = NULL;
}

AutoReleaseTradeRedisInfo::~AutoReleaseTradeRedisInfo() 
{
	if(red_packet_redis_info)
		red_packet_redis_info__free_unpacked(red_packet_redis_info, NULL);
	if(player_recive_record)
		red_packet_redis_player_recive_record__free_unpacked(player_recive_record, NULL);
	if(player_all_hisotry)
		red_packet_redis_player_all_jilu_info__free_unpacked(player_all_hisotry, NULL);

}

void AutoReleaseTradeRedisInfo::set_cur_red_packet(RedPacketRedisInfo* r)
{

	if (red_packet_redis_info)
	{
		red_packet_redis_info__free_unpacked(red_packet_redis_info, NULL);
	}
	red_packet_redis_info  = r;
}
void AutoReleaseTradeRedisInfo::set_player_red_packet_record(RedPacketRedisPlayerReciveRecord* r)
{

	if (player_recive_record)
	{
		red_packet_redis_player_recive_record__free_unpacked(player_recive_record, NULL);
	}
	player_recive_record  = r;
}

void AutoReleaseTradeRedisInfo::set_player_red_packet_history(RedPacketRedisPlayerAllJiluInfo* r)
{

	if(player_all_hisotry)
	{
		red_packet_redis_player_all_jilu_info__free_unpacked(player_all_hisotry, NULL);
	}
	player_all_hisotry  = r;
}

AutoReleaseBatchRedRedisInfo::AutoReleaseBatchRedRedisInfo()
{

}

AutoReleaseBatchRedRedisInfo::~AutoReleaseBatchRedRedisInfo()
{
	
	for(std::vector<RedPacketRedisInfo *>::iterator iter = pointer_vec.begin(); iter != pointer_vec.end(); iter++)
	{
		red_packet_redis_info__free_unpacked(*iter, NULL);
	}
}

void AutoReleaseBatchRedRedisInfo::push_back(RedPacketRedisInfo* r)
{
	pointer_vec.push_back(r);
}
RedPacketRedisInfo *get_red_packet_redis_info(uint64_t red_uuid, char *red_packet_key, CRedisClient &rc, AutoReleaseTradeRedisInfo &_pool)
{
	static uint8_t data_buffer[200 * 1024];
	int data_len = sizeof(data_buffer);
	char field[64];
	sprintf(field, "%lu", red_uuid);
	int ret = rc.hget_bin(red_packet_key, field, (char *)data_buffer, &data_len);
	if (ret == 0)
	{
		RedPacketRedisInfo *ret = red_packet_redis_info__unpack(NULL, data_len, data_buffer);
		if(ret)
			_pool.set_cur_red_packet(ret);
		return ret;
	}

	return NULL;
}
int get_more_red_packet_redis_info(std::set<uint64_t> &red_uuid, std::map<uint64_t, RedPacketRedisInfo*> &redis_players, char *red_packet_key, CRedisClient &rc, AutoReleaseBatchRedRedisInfo &_pool)
{
	if (red_uuid.size() == 0)
	{
		return 0;
	}

	std::vector<std::relation_three<uint64_t, char*, int> > red_packet_infos;
	for (std::set<uint64_t>::iterator iter = red_uuid.begin(); iter != red_uuid.end(); ++iter)
	{
		std::relation_three<uint64_t, char*, int> tmp(*iter, NULL, 0);
		red_packet_infos.push_back(tmp);
	}

	int ret = rc.get(red_packet_key, red_packet_infos);
	if (ret != 0)
	{
		LOG_ERR("[%s:%d] hmget failed, ret:%d", __FUNCTION__, __LINE__, ret);
		return -1;
	}

	for (std::vector<std::relation_three<uint64_t, char*, int> >::iterator iter = red_packet_infos.begin(); iter != red_packet_infos.end(); ++iter)
	{
		RedPacketRedisInfo *redis_red_packet = red_packet_redis_info__unpack(NULL, iter->three, (uint8_t*)iter->second);
		if (!redis_red_packet)
		{
			ret = -1;
			LOG_ERR("[%s:%d] unpack redis failed, player_id:%lu", __FUNCTION__, __LINE__, iter->first);
			continue;
		}

		redis_players[iter->first] = redis_red_packet;
		_pool.push_back(redis_red_packet);
	}

	for (std::vector<std::relation_three<uint64_t, char*, int> >::iterator iter = red_packet_infos.begin(); iter != red_packet_infos.end(); ++iter)
	{
		free(iter->second);
	}

	return ret;
}

RedPacketRedisPlayerReciveRecord *get_player_red_packet_redis_recive_record(uint64_t player_id, char *red_packet_key, CRedisClient &rc, AutoReleaseTradeRedisInfo &_pool)
{
	static uint8_t data_buffer[200 * 1024];
	int data_len = sizeof(data_buffer);
	char field[64];
	sprintf(field, "%lu", player_id);
	int ret = rc.hget_bin(red_packet_key, field, (char *)data_buffer, &data_len);
	if (ret == 0)
	{
		RedPacketRedisPlayerReciveRecord *ret = red_packet_redis_player_recive_record__unpack(NULL, data_len, data_buffer);
		if(ret)
			_pool.set_player_red_packet_record(ret);
		return ret;
	}

	return NULL;
}

RedPacketRedisPlayerAllJiluInfo *get_player_red_packet_redis_all_history_record(uint64_t player_id, char *red_packet_key, CRedisClient &rc, AutoReleaseTradeRedisInfo &_pool)
{
	static uint8_t data_buffer[512];
	int data_len = sizeof(data_buffer);
	char field[64];
	sprintf(field, "%lu", player_id);
	int ret = rc.hget_bin(red_packet_key, field, (char *)data_buffer, &data_len);
	if (ret == 0)
	{
		RedPacketRedisPlayerAllJiluInfo *ret = red_packet_redis_player_all_jilu_info__unpack(NULL, data_len, data_buffer);
		if(ret)
			_pool.set_player_red_packet_history(ret);
		return ret;
	}

	return NULL;
}

static void cb_5clock_timer(evutil_socket_t, short, void* /*arg*/)
{
	handle_daily_reset_timeout(false);
}

void handle_daily_reset_timeout(bool start)
{
	uint32_t cur_tick = time_helper::get_micro_time() / 1000 / 1000;
	uint32_t next_tick = time_helper::nextOffsetTime(daily_reset_clock, cur_tick);
	five_oclock_timeout.tv_sec = next_tick - cur_tick;
	five_oclock_timer.ev_callback = cb_5clock_timer;
	add_timer(five_oclock_timeout, &five_oclock_timer, NULL);

	if (start)
	{
		return;
	}

	clear_sold_average_map();
}

void cb_second_timer(evutil_socket_t, short, void* /*arg*/)
{
	uint32_t now = time_helper::get_micro_time() / 1000 / 1000;
	for (TradeItemMap::iterator iter = trade_item_map.begin(); iter != trade_item_map.end(); ++iter)
	{
		TradeItem *item = iter->second;
		if (item->time <= now)
		{ //超时
			uint32_t state = get_trade_state(item->state);
			if (state == Trade_State_Aduit)
			{ //审核时间到，可以出售
				set_trade_state(item->state, Trade_State_Sell);
				item->time = now + sg_keep_time;
				save_trade_item(item);

				update_trade_shelf_notify(item);
				update_trade_item_summary_broadcast(item->item_id);
			}
			else if (state == Trade_State_Sell)
			{ //出售时间到，货品过期
				set_trade_state(item->state, Trade_State_Overdue);
				save_trade_item(item);

				update_trade_shelf_notify(item);
				update_trade_item_summary_broadcast(item->item_id);
			}
		}
	}

	for (AuctionLotMap::iterator iter = auction_lot_map.begin(); iter != auction_lot_map.end(); )
	{
		AuctionLot *lot = iter->second;
		if (lot->time <= now)
		{ //超时
			AuctionTable *config = get_config_by_id(lot->lot_id, &auction_config);
			if (!config)
			{
				iter++;
				continue;
			}

			bool bSold = false;
			uint32_t buyer_mail_id = 0;
			std::vector<char *> buyer_mail_args;
			std::string sz_num;
			std::stringstream ss;
			if (lot->type == Auction_Type_Guild)
			{
				if (lot->bidder_id == 0)
				{ //流拍
					lot->type = Auction_Type_Server;
					lot->time = now + config->Time;
					save_auction_lot(lot);
					iter++;
				}
				else
				{
					bSold = true;
					buyer_mail_id = 270100005;
					buyer_mail_args.push_back(config->ItemName);
					ss.str("");
					ss.clear();
					ss << lot->price;
					ss >> sz_num;
					buyer_mail_args.push_back(const_cast<char*>(sz_num.c_str()));
				}
			}
			else if (lot->type == Auction_Type_Server)
			{
				bSold = true;
				if (lot->bidder_id != 0)
				{
					buyer_mail_id = 270100006;
					buyer_mail_args.push_back(config->ItemName);
					ss.str("");
					ss.clear();
					ss << lot->price;
					ss >> sz_num;
					buyer_mail_args.push_back(const_cast<char*>(sz_num.c_str()));
				}
			}

			if (bSold)
			{
				uint32_t sold_price = lot->price * (1.0 - sg_trade_tax_percent);
				if (lot->bidder_id > 0)
				{ //拍卖成功
					std::map<uint32_t, uint32_t> attachs;
					uint32_t _id = config->ItemID;
					uint32_t _num = config->Num;
					attachs.insert(std::make_pair(_id, _num));
					send_mail(&conn_node_tradesrv::connecter, lot->bidder_id, buyer_mail_id, NULL, NULL, NULL, &buyer_mail_args, &attachs, MAGIC_TYPE_AUCTION_SOLD);
				}
				else
				{ //系统回收
				}

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
					uint32_t give_money = std::max(sold_price / master_num, (uint32_t)1);
					std::map<uint32_t, uint32_t> attachs;
					std::vector<char *> args;
					std::string sz_sold_price, sz_gain_money;
					args.push_back(config->ItemName);
					ss.str("");
					ss.clear();
					ss << lot->price;
					ss >> sz_sold_price;
					args.push_back(const_cast<char*>(sz_sold_price.c_str()));
					ss.str("");
					ss.clear();
					ss << give_money;
					ss >> sz_gain_money;
					args.push_back(const_cast<char*>(sz_gain_money.c_str()));
					attachs.insert(std::make_pair(201010003, give_money));
					for (int i = 0; i < MAX_AUCTION_MASTER_NUM; ++i)
					{
						if (lot->masters[i] == 0)
						{
							break;
						}
						send_mail(&conn_node_tradesrv::connecter, lot->masters[i], 270100007, NULL, NULL, NULL, &args, &attachs, MAGIC_TYPE_AUCTION_SOLD);
					}
				}

				auction_lot_map.erase(iter++);
				remove_auction_lot(lot, false);
			}
		}
		else
		{
			iter++;
		}
	}
	refresh_all_red_packet_redis_data();

	add_timer(second_timeout, &second_timer, NULL);
}

class AutoReleaseDbTradePlayer
{
public:
	AutoReleaseDbTradePlayer(DBTradePlayer *db) : pointer(db) {}
	~AutoReleaseDbTradePlayer() { dbtrade_player__free_unpacked(pointer, NULL); }
private:
	DBTradePlayer *pointer;
};

class AutoReleaseDbTradeItem
{
public:
	AutoReleaseDbTradeItem(DBTradeItem *db) : pointer(db) {}
	~AutoReleaseDbTradeItem() { dbtrade_item__free_unpacked(pointer, NULL); }
private:
	DBTradeItem *pointer;
};
class AutoReleaseDbAuctionLot
{
public:
	AutoReleaseDbAuctionLot(DBAuctionLot *db) : pointer(db) {}
	~AutoReleaseDbAuctionLot() { dbauction_lot__free_unpacked(pointer, NULL); }
private:
	DBAuctionLot *pointer;
};

TradePlayer *find_trade_player(uint64_t player_id)
{
	TradePlayerMap::iterator iter = trade_player_map.find(player_id);
	if (iter != trade_player_map.end())
	{
		return iter->second;
	}
	return NULL;
}

static int unpack_dbdata_to_trade_player(DBTradePlayer *db_player, TradePlayer *player)
{
	player->shelf_num = db_player->shelf_num;
	for (size_t i = 0; i < db_player->n_sold_items && i < MAX_TRADE_SOLD_NUM; ++i)
	{
		player->sold_items[i].item_id = db_player->sold_items[i]->item_id;
		player->sold_items[i].num = db_player->sold_items[i]->num;
		player->sold_items[i].price = db_player->sold_items[i]->price;
	}
	player->sold_earning = db_player->sold_earning;

	return 0;
}

static int unpack_dbdata_to_trade_item(DBTradeItem *db_item, TradeItem *item)
{
	if (db_item->bagua)
	{
		item->especial.baguapai.star = db_item->bagua->star;
		for (size_t i = 0; i < db_item->bagua->n_minor_attrs && i < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++i)
		{
			item->especial.baguapai.minor_attrs[i].pool = db_item->bagua->minor_attrs[i]->pool;
			item->especial.baguapai.minor_attrs[i].id = db_item->bagua->minor_attrs[i]->id;
			item->especial.baguapai.minor_attrs[i].val = db_item->bagua->minor_attrs[i]->val;
		}
		for (size_t i = 0; i < db_item->bagua->n_additional_attrs && i < MAX_BAGUAPAI_ADDITIONAL_ATTR_NUM; ++i)
		{
			item->especial.baguapai.additional_attrs[i].pool = db_item->bagua->additional_attrs[i]->pool;
			item->especial.baguapai.additional_attrs[i].id = db_item->bagua->additional_attrs[i]->id;
			item->especial.baguapai.additional_attrs[i].val = db_item->bagua->additional_attrs[i]->val;
		}
	}
	else if (db_item->fabao)
	{
		item->especial.fabao.main_attr.id = db_item->fabao->main_attr->id;
		item->especial.fabao.main_attr.val = db_item->fabao->main_attr->val;
		for (size_t i = 0; i < db_item->fabao->n_minor_attr && i < MAX_HUOBAN_FABAO_MINOR_ATTR_NUM; ++i)
		{
			item->especial.fabao.minor_attr[i].id = db_item->fabao->minor_attr[i]->id;
			item->especial.fabao.minor_attr[i].val = db_item->fabao->minor_attr[i]->val;
		}
	}

	return 0;
}

static int unpack_dbdata_to_auction_lot(DBAuctionLot *db_lot, AuctionLot *lot)
{
	for (size_t i = 0; i < db_lot->n_masters && i < MAX_AUCTION_MASTER_NUM; ++i)
	{
		lot->masters[i] = db_lot->masters[i];
	}
	return 0;
}

static int load_all_trade_player(void)
{
	char sql[1024];
	unsigned long *lengths;	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;

	sprintf(sql, "select `player_id`, `comm_data` from trade_player;");

	res = query(sql, 1, NULL);
	if (!res)
	{
		LOG_ERR("[%s:%d] query failed, sql: %s", __FUNCTION__, __LINE__, sql);
		return -1;
	}

	while (true)
	{
		row = fetch_row(res);
		if (!row)
		{
//			LOG_ERR("[%s:%d] query sql fetch row failed, sql: %s", __FUNCTION__, __LINE__, sql);
			break;
		}

		uint64_t player_id = strtoull(row[0], NULL, 10);
		lengths = mysql_fetch_lengths(res);
		DBTradePlayer *db_player = dbtrade_player__unpack(NULL, lengths[1], (uint8_t *)row[1]);
		if (!db_player)
		{
			LOG_ERR("[%s:%d] unpack trade player failed, player_id:%lu", __FUNCTION__, __LINE__, player_id);
			continue;
		}
		AutoReleaseDbTradePlayer release_player(db_player);

		TradePlayer *player = (TradePlayer *)malloc(sizeof(TradePlayer));
		if (!player)
		{
			LOG_ERR("[%s:%d] malloc trade player failed, player_id:%lu", __FUNCTION__, __LINE__, player_id);
			continue;
		}
		memset(player, 0, sizeof(TradePlayer));

		unpack_dbdata_to_trade_player(db_player, player);
		player->player_id = player_id;

		trade_player_map.insert(std::make_pair(player_id, player));
	}

	free_query(res);

	return 0;
}

static int load_all_trade_item(void)
{
	char sql[1024];
	unsigned long *lengths;	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;

	sprintf(sql, "select `player_id`, `item_id`, `shelf_index`, `num`, `price`, `state`, UNIX_TIMESTAMP(`time`), `comm_data` from trade_item;");

	res = query(sql, 1, NULL);
	if (!res)
	{
		LOG_ERR("[%s:%d] query failed, sql: %s", __FUNCTION__, __LINE__, sql);
		return -1;
	}

	while (true)
	{
		row = fetch_row(res);
		if (!row)
		{
//			LOG_ERR("[%s:%d] query sql fetch row failed, sql: %s", __FUNCTION__, __LINE__, sql);
			break;
		}

		uint64_t player_id = strtoull(row[0], NULL, 10);
		uint32_t item_id = strtoul(row[1], NULL, 10);
		lengths = mysql_fetch_lengths(res);
		DBTradeItem *db_item = dbtrade_item__unpack(NULL, lengths[7], (uint8_t *)row[7]);
		if (!db_item)
		{
			LOG_ERR("[%s:%d] unpack trade item failed, item_id:%u, player_id:%lu", __FUNCTION__, __LINE__, item_id, player_id);
			continue;
		}
		AutoReleaseDbTradeItem release_item(db_item);

		TradeItem *item = (TradeItem *)malloc(sizeof(TradeItem));
		if (!item)
		{
			LOG_ERR("[%s:%d] malloc trade item failed, item_id:%u, player_id:%lu", __FUNCTION__, __LINE__, item_id, player_id);
			continue;
		}
		memset(item, 0, sizeof(TradeItem));

		unpack_dbdata_to_trade_item(db_item, item);
		item->player_id = player_id;
		item->item_id = item_id;
		item->shelf_index = strtoul(row[2], NULL, 10);
		item->num = strtoull(row[3], NULL, 10);
		item->price = strtoul(row[4], NULL, 10);
		item->state = strtoul(row[5], NULL, 10);
		item->time = strtoul(row[6], NULL, 10);

		trade_item_map.insert(std::make_pair(item_id, item));

		TradePlayerMap::iterator iter = trade_player_map.find(player_id);
		if (iter != trade_player_map.end())
		{
			iter->second->shelf_items[item->shelf_index] = item;
		}
	}

	free_query(res);

	return 0;
}

int load_all_trade_sold(void)
{
	uint32_t last_clear_time = time_helper::lastOffsetTime(daily_reset_clock);
	char sql[1024];
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;

	//先清除过期的数据
	sprintf(sql, "delete from trade_sold where `time` <= FROM_UNIXTIME(%u);", last_clear_time);
	query(sql, 1, NULL);

	//加载数据
	sprintf(sql, "select `item_id`, `num`, `price` from trade_sold;");

	res = query(sql, 1, NULL);
	if (!res)
	{
		LOG_ERR("[%s:%d] query failed, sql: %s", __FUNCTION__, __LINE__, sql);
		return -1;
	}

	while (true)
	{
		row = fetch_row(res);
		if (!row)
		{
//			LOG_ERR("[%s:%d] query sql fetch row failed, sql: %s", __FUNCTION__, __LINE__, sql);
			break;
		}

		uint32_t item_id = strtoul(row[0], NULL, 10);
		uint32_t num = strtoul(row[1], NULL, 10);
		uint32_t price = strtoul(row[2], NULL, 10);

		TradeSoldInfo *item = (TradeSoldInfo *)malloc(sizeof(TradeSoldInfo));
		if (!item)
		{
			LOG_ERR("[%s:%d] malloc trade sold failed, item_id:%u", __FUNCTION__, __LINE__, item_id);
			continue;
		}
		memset(item, 0, sizeof(TradeSoldInfo));

		item->item_id = item_id;
		item->num = num;
		item->price = price;

		trade_sold_map.insert(std::make_pair(item_id, item));
	}

	free_query(res);

	return 0;
}

static int load_all_auction_lot(void)
{
	char sql[1024];
	unsigned long *lengths;	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;

	sprintf(sql, "select `uuid`, `lot_id`, `price`, UNIX_TIMESTAMP(`time`), `type`, `type_limit`, `bidder_id`,  `comm_data` from auction_lot where over = 0;");

	res = query(sql, 1, NULL);
	if (!res)
	{
		LOG_ERR("[%s:%d] query failed, sql: %s", __FUNCTION__, __LINE__, sql);
		return -1;
	}

	while (true)
	{
		row = fetch_row(res);
		if (!row)
		{
//			LOG_ERR("[%s:%d] query sql fetch row failed, sql: %s", __FUNCTION__, __LINE__, sql);
			break;
		}

		uint64_t uuid = strtoull(row[0], NULL, 10);
		uint32_t lot_id = strtoul(row[1], NULL, 10);
		lengths = mysql_fetch_lengths(res);
		DBAuctionLot *db_lot = dbauction_lot__unpack(NULL, lengths[7], (uint8_t *)row[7]);
		if (!db_lot)
		{
			LOG_ERR("[%s:%d] unpack auction lot failed, uuid:%lu, lot_id:%u", __FUNCTION__, __LINE__, uuid, lot_id);
			continue;
		}
		AutoReleaseDbAuctionLot release_lot(db_lot);

		AuctionLot *lot = (AuctionLot *)malloc(sizeof(AuctionLot));
		if (!lot)
		{
			LOG_ERR("[%s:%d] malloc auction lot failed, uuid:%lu, lot_id:%u", __FUNCTION__, __LINE__, uuid, lot_id);
			continue;
		}
		memset(lot, 0, sizeof(AuctionLot));

		unpack_dbdata_to_auction_lot(db_lot, lot);
		lot->uuid = uuid;
		lot->lot_id = lot_id;
		lot->price = strtoul(row[2], NULL, 10);
		lot->time = strtoul(row[3], NULL, 10);
		lot->type = strtoul(row[4], NULL, 10);
		lot->type_limit = strtoul(row[5], NULL, 10);
		lot->bidder_id = strtoul(row[6], NULL, 10);

		auction_lot_map.insert(std::make_pair(uuid, lot));
	}

	free_query(res);

	return 0;
}

static int load_all_auction_bid(void)
{
	char sql[1024];
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;

	for (AuctionLotMap::iterator iter = auction_lot_map.begin(); iter != auction_lot_map.end(); ++iter)
	{
		uint64_t lot_uuid = iter->first;

		sprintf(sql, "select `player_id`, `price` from auction_bid where lot_uuid = %lu order by time desc;", lot_uuid);

		res = query(sql, 1, NULL);
		if (!res)
		{
			LOG_ERR("[%s:%d] query failed, sql: %s", __FUNCTION__, __LINE__, sql);
			continue;
		}

		while (true)
		{
			row = fetch_row(res);
			if (!row)
			{
				break;
			}

			uint64_t player_id = strtoull(row[0], NULL, 10);
			uint32_t price = strtoul(row[1], NULL, 10);

			auction_bid_map.insert(std::make_pair(std::make_pair(lot_uuid, player_id), price));
		}

		free_query(res);
	}

	return 0;
}

void load_trade_module(void)
{
	//先加载玩家数据，再加载道具数据
	load_all_trade_player();
	load_all_trade_item();
	load_all_trade_sold();

	//先加载拍卖品数据，再加载竞价数据
	load_all_auction_lot();
	load_all_auction_bid();
}

int pack_trade_item(TradeItem *item, uint8_t *out_data)
{
	DBTradeItem db_info;
//	DBTradeItem *db_item = &db_info;
	dbtrade_item__init(&db_info);

	DBItemBagua item_bagua_data;
	DBItemPartnerFabao item_fabao_data;
	DBCommonRandAttr item_bagua_attr[MAX_BAGUAPAI_MINOR_ATTR_NUM];
	DBCommonRandAttr* item_bagua_attr_point[MAX_BAGUAPAI_MINOR_ATTR_NUM];
	DBCommonRandAttr  item_bagua_additional_attr[MAX_BAGUAPAI_ADDITIONAL_ATTR_NUM];
	DBCommonRandAttr* item_bagua_additional_attr_point[MAX_BAGUAPAI_ADDITIONAL_ATTR_NUM];
	DBAttr item_fabao_attr[MAX_HUOBAN_FABAO_MINOR_ATTR_NUM];
	DBAttr* item_fabao_attr_point[MAX_HUOBAN_FABAO_MINOR_ATTR_NUM];
	DBAttr fabao_attr;
	
	uint32_t item_type = get_item_config_type(item->item_id);
	switch (item_type)
	{
		case 10: //八卦
			{
				db_info.bagua = &item_bagua_data;
				dbitem_bagua__init(&item_bagua_data);
				item_bagua_data.star = item->especial.baguapai.star;
				uint32_t attr_num = 0;
				for (int j = 0; j < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++j)
				{
					item_bagua_attr_point[attr_num] = &item_bagua_attr[attr_num];
					dbcommon_rand_attr__init(&item_bagua_attr[attr_num]);
					item_bagua_attr[attr_num].pool = item->especial.baguapai.minor_attrs[j].pool;
					item_bagua_attr[attr_num].id = item->especial.baguapai.minor_attrs[j].id;
					item_bagua_attr[attr_num].val = item->especial.baguapai.minor_attrs[j].val;
					attr_num++;
				}
				item_bagua_data.minor_attrs = item_bagua_attr_point;
				item_bagua_data.n_minor_attrs = attr_num;
				attr_num = 0;
				for (int j = 0; j < MAX_BAGUAPAI_ADDITIONAL_ATTR_NUM; ++j)
				{
					item_bagua_additional_attr_point[attr_num] = &item_bagua_additional_attr[attr_num];
					dbcommon_rand_attr__init(&item_bagua_additional_attr[attr_num]);
					item_bagua_additional_attr[attr_num].pool = item->especial.baguapai.additional_attrs[j].pool;
					item_bagua_additional_attr[attr_num].id = item->especial.baguapai.additional_attrs[j].id;
					item_bagua_additional_attr[attr_num].val = item->especial.baguapai.additional_attrs[j].val;
					attr_num++;
				}
				item_bagua_data.additional_attrs = item_bagua_additional_attr_point;
				item_bagua_data.n_additional_attrs = attr_num;
			}
			break;
		case 14: //法宝
			{
				db_info.fabao = &item_fabao_data;
				dbitem_partner_fabao__init(&item_fabao_data);
				item_fabao_data.main_attr = &fabao_attr;
				dbattr__init(&fabao_attr);
				fabao_attr.id = item->especial.fabao.main_attr.id;
				fabao_attr.val = item->especial.fabao.main_attr.val;
				uint32_t attr_num = 0;
				for (int j = 0; j < MAX_HUOBAN_FABAO_MINOR_ATTR_NUM ; ++j)
				{
					item_fabao_attr_point[attr_num] = &item_fabao_attr[attr_num];
					dbattr__init(&item_fabao_attr[attr_num]);
					item_fabao_attr[attr_num].id = item->especial.fabao.minor_attr[j].id;
					item_fabao_attr[attr_num].val = item->especial.fabao.minor_attr[j].val;
					attr_num++;
				}
				item_fabao_data.minor_attr = item_fabao_attr_point;
				item_fabao_data.n_minor_attr = attr_num;
			}
			break;
	}

	return dbtrade_item__pack(&db_info, out_data);
}

int save_trade_item(TradeItem *item, bool insert)
{
	static char save_sql[64 * 1024 + 300];
	static uint8_t save_data[64 * 1024 + 1];
	uint64_t effect = 0;
	int len;
	char *p;
	
	size_t data_size = pack_trade_item(item, save_data);
	if (insert)
	{
		len = sprintf(save_sql, "insert trade_item set `player_id` = %lu, `shelf_index` = %u, `item_id` = %u, `num` = %u, `price` = %u, `state` = %u, `time` = FROM_UNIXTIME(%u), `comm_data` = \'", item->player_id, item->shelf_index, item->item_id, item->num, item->price, item->state, item->time);
		p = save_sql + len;
		p += escape_string(p, (const char *)save_data, data_size);
		len = sprintf(p, "\'");
	}
	else
	{
		len = sprintf(save_sql, "update trade_item set `item_id` = %u, `num` = %u, `price` = %u, `state` = %u, `time` = FROM_UNIXTIME(%u), `comm_data` = \'", item->item_id, item->num, item->price, item->state, item->time);
		p = save_sql + len;
		p += escape_string(p, (const char *)save_data, data_size);
		len = sprintf(p, "\' where player_id = %lu and shelf_index = %u", item->player_id, item->shelf_index);
	}

	query(const_cast<char*>("set names utf8"), 1, NULL);
	query(save_sql, 1, &effect);	
	if (effect != 1) 
	{
		LOG_ERR("[%s:%d] save trade item failed, player_id:%lu, shelf_index:%u, item_id:%u", __FUNCTION__, __LINE__, item->player_id, item->shelf_index, item->item_id);
		return -1;
	}

	return 0;
}

int pack_trade_player(TradePlayer *player, uint8_t *out_data)
{
	DBTradePlayer db_info;
	DBTradePlayer *db_player = &db_info;
	dbtrade_player__init(db_player);

	DBTradeSold  sold_data[MAX_TRADE_SOLD_NUM];
	DBTradeSold* sold_point[MAX_TRADE_SOLD_NUM];

	db_player->shelf_num = player->shelf_num;
	db_player->sold_earning = player->sold_earning;
	db_player->sold_items = sold_point;
	db_player->n_sold_items = 0;
	for (int i = 0; i < MAX_TRADE_SOLD_NUM; ++i)
	{
		if (player->sold_items[i].item_id == 0)
		{
			break;
		}

		sold_point[db_player->n_sold_items] = &sold_data[db_player->n_sold_items];
		dbtrade_sold__init(sold_point[db_player->n_sold_items]);
		sold_data[db_player->n_sold_items].item_id = player->sold_items[i].item_id;
		sold_data[db_player->n_sold_items].num = player->sold_items[i].num;
		sold_data[db_player->n_sold_items].price = player->sold_items[i].price;
		db_player->n_sold_items++;
	}

	return dbtrade_player__pack(db_player, out_data);
}

int save_trade_player(TradePlayer *player)
{
	static char save_sql[64 * 1024 + 300];
	static uint8_t save_data[64 * 1024 + 1];
	uint64_t effect = 0;
	char *p;
	
	size_t data_size = pack_trade_player(player, save_data);
	p = save_sql;
	p += sprintf(save_sql, "replace trade_player set `player_id` = %lu, `comm_data` = \'", player->player_id);
	p += escape_string(p, (const char *)save_data, data_size);
	p += sprintf(p, "\'");

	query(const_cast<char*>("set names utf8"), 1, NULL);
	query(save_sql, 1, &effect);	
	if (effect != 1 && effect != 2) 
	{
		LOG_ERR("[%s:%d] save trade player %lu failed, error:%s", __FUNCTION__, __LINE__, player->player_id, mysql_error());
		return -1;
	}

	return 0;
}

int delete_trade_item(uint64_t player_id, uint32_t shelf_index)
{
	char sql[300];
	uint64_t effect = 0;
	
	sprintf(sql, "delete from trade_item where player_id = %lu and shelf_index = %u", player_id, shelf_index);

	query(const_cast<char*>("set names utf8"), 1, NULL);
	query(sql, 1, &effect);	
	if (effect != 1) 
	{
		LOG_ERR("[%s:%d] delete trade item failed, sql:\"%s\", error:%s", __FUNCTION__, __LINE__, sql, mysql_error());
		return -1;
	}

	return 0;
}

void clear_module_memory(void)
{
	LOG_INFO("[%s:%d]", __FUNCTION__, __LINE__);
	for (TradeItemMap::iterator iter = trade_item_map.begin(); iter != trade_item_map.end(); ++iter)
	{
		free(iter->second);
		iter->second = NULL;
	}
	trade_item_map.clear();
	for (std::list<TradeItem *>::iterator iter = trade_item_free_list.begin(); iter != trade_item_free_list.end(); ++iter)
	{
		free(*iter);
	}
	trade_item_free_list.clear();
	for (TradePlayerMap::iterator iter = trade_player_map.begin(); iter != trade_player_map.end(); ++iter)
	{
		free(iter->second);
		iter->second = NULL;
	}
	trade_player_map.clear();
	for (TradeSoldMap::iterator iter = trade_sold_map.begin(); iter != trade_sold_map.end(); ++iter)
	{
		free(iter->second);
		iter->second = NULL;
	}
	trade_player_map.clear();
	for (std::list<TradeSoldInfo *>::iterator iter = trade_sold_free_list.begin(); iter != trade_sold_free_list.end(); ++iter)
	{
		free(*iter);
	}
	trade_sold_free_list.end();
	for (AuctionLotMap::iterator iter = auction_lot_map.begin(); iter != auction_lot_map.end(); ++iter)
	{
		free(iter->second);
		iter->second = NULL;
	}
	auction_lot_map.clear();
	for (std::list<AuctionLot *>::iterator iter = auction_lot_free_list.begin(); iter != auction_lot_free_list.end(); ++iter)
	{
		free(*iter);
	}
	auction_lot_free_list.clear();
	auction_bid_map.clear();
}

TradePlayer *create_trade_player(uint64_t player_id)
{
	TradePlayer *player = (TradePlayer *)malloc(sizeof(TradePlayer));
	if (!player)
	{
		LOG_ERR("[%s:%d] malloc trade player failed, player_id:%lu", __FUNCTION__, __LINE__, player_id);
		return NULL;
	}
	memset(player, 0, sizeof(TradePlayer));

	player->player_id = player_id;
	player->shelf_num = sg_shelf_init_num;
	trade_player_map.insert(std::make_pair(player_id, player));

	return player;
}

TradePlayer *get_trade_player(uint64_t player_id)
{
	TradePlayerMap::iterator iter = trade_player_map.find(player_id);
	if (iter != trade_player_map.end())
	{
		return iter->second;
	}
	return create_trade_player(player_id);
}

bool player_is_online(uint64_t player_id)
{
	AutoReleaseRedisPlayer ar_redis;
	PlayerRedisInfo *redis_player = get_redis_player(player_id, sg_player_key, sg_redis_client, ar_redis);
	if (redis_player && redis_player->status == 0)
	{
		return true;
	}
	return false;
}

void add_sold_info_to_average_map(uint32_t item_id, uint32_t sold_num, uint32_t sold_price)
{
	uint32_t total_sold_num = 0, total_sold_price = 0;
	TradeSoldMap::iterator sold_iter = trade_sold_map.find(item_id);
	if (sold_iter != trade_sold_map.end())
	{
		sold_iter->second->num += sold_num;
		sold_iter->second->price += sold_num * sold_price;
		total_sold_num = sold_iter->second->num;
		total_sold_price = sold_iter->second->price;
	}
	else
	{
		total_sold_num = sold_num;
		total_sold_price = sold_price;
		TradeSoldInfo *info = NULL;
		if (trade_sold_free_list.empty())
		{
			info = (TradeSoldInfo *)malloc(sizeof(TradeSoldInfo));
		}
		else
		{
			info = trade_sold_free_list.front();
			trade_sold_free_list.pop_front();
		}
		
		if (info)
		{
			info->item_id = item_id;
			info->num = sold_num;
			info->price = sold_num * sold_price;
			trade_sold_map.insert(std::make_pair(info->item_id, info));
		}
	}

	char sql[300];
	uint64_t effect = 0;
	sprintf(sql, "replace trade_sold set `item_id` = %u, `num` = %u, `price` = %u, `time` = now()", item_id, total_sold_num, total_sold_price);
	query(sql, 1, &effect);	
	if (effect <= 0) 
	{
		LOG_ERR("[%s:%d] save failed, sql:%s, effect[%lu]", __FUNCTION__, __LINE__, sql, effect);
		return ;
	}
}

void clear_sold_average_map(void)
{
	for (TradeSoldMap::iterator iter = trade_sold_map.begin(); iter != trade_sold_map.end(); ++iter)
	{
		trade_sold_free_list.push_back(iter->second);
		iter->second = NULL;
	}
	trade_sold_map.clear();

	char sql[300];
	uint64_t effect = 0;
	sprintf(sql, "delete from trade_sold;");
	query(sql, 1, &effect);	
}

uint32_t get_trade_item_average_price(uint32_t item_id)
{
	TradeSoldMap::iterator iter = trade_sold_map.find(item_id);
	if (iter != trade_sold_map.end())
	{
		return (iter->second->price / iter->second->num);
	}

	return 0;
}

int check_trade_item_price_rational(uint32_t item_id, uint32_t price)
{
	int ret = 0;
	uint32_t average_price = 0, guide_price = 0;
	do
	{
		TradingTable *config = get_config_by_id(item_id, &trade_item_config);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] get trade config failed, item_id:%u", __FUNCTION__, __LINE__, item_id);
			break;
		}

		average_price = get_trade_item_average_price(item_id);
		guide_price = config->GuidePrice;
		if (guide_price > average_price)
		{ //当基准价格高于平均价格则只能向下调整50%
			if (price < guide_price * 0.5)
			{
				ret = ERROR_ID_TRADE_PRICE;
				break;
			}
		}
		else if (guide_price < average_price)
		{ //当基准价格低于平均价格则只能向上调整50%
			if (price > guide_price * 1.5)
			{
				ret = ERROR_ID_TRADE_PRICE;
				break;
			}
		}

		//当前道具的卖价不能低于基准价格的50%
		if (price < guide_price * 0.5)
		{
			ret = ERROR_ID_TRADE_PRICE;
			break;
		}
		//当前道具的卖价不能高于基准价格的100%
		if (price > guide_price * 2.0)
		{
			ret = ERROR_ID_TRADE_PRICE;
			break;
		}
	} while(0);

	if (ret != 0)
	{
		LOG_ERR("[%s:%d] price error, item_id:%u, average_price:%u, guide_price:%u, price:%u", __FUNCTION__, __LINE__, item_id, average_price, guide_price, price);
	}

	return ret;
}

TradeItem *create_trade_item(TradePlayer *player, uint32_t shelf_index, uint32_t item_id, uint32_t num, uint32_t price, EspecialItemInfo *especial)
{
	TradeItem *item = NULL;
	if (trade_item_free_list.empty())
	{
		item = (TradeItem *)malloc(sizeof(TradeItem));
		if (!item)
		{
			LOG_ERR("[%s:%d] player[%lu] malloc trade item failed, item_id:%u", __FUNCTION__, __LINE__, player->player_id, item_id);
			return NULL;
		}
	}
	else
	{
		item = trade_item_free_list.front();
		trade_item_free_list.pop_front();
	}

	memset(item, 0, sizeof(TradeItem));
	item->player_id = player->player_id;
	item->shelf_index = shelf_index;
	item->item_id = item_id;
	item->num = num;
	item->price = price;
	memcpy(&item->especial, especial, sizeof(EspecialItemInfo));
	item->state = Trade_State_Aduit;
	item->time = (time_helper::get_cached_time() / 1000 + get_trade_aduit_time());

	trade_item_map.insert(std::make_pair(item_id, item));
	player->shelf_items[shelf_index] = item;
	save_trade_item(item, true);

	return item;
}

void remove_trade_item(TradeItem *&item)
{
	TradePlayer *player = find_trade_player(item->player_id);
	if (player && item->shelf_index <= MAX_TRADE_SHELF_NUM)
	{
		player->shelf_items[item->shelf_index] = NULL;
	}

	std::pair<TradeItemMap::iterator, TradeItemMap::iterator> range = trade_item_map.equal_range(item->item_id);
	for (TradeItemMap::iterator iter = range.first; iter != range.second; ++iter)
	{
		if (iter->second == item)
		{
			trade_item_map.erase(iter);
			break;
		}
	}
	trade_item_free_list.push_back(item);
	delete_trade_item(item->player_id, item->shelf_index);
	delete_trade_shelf_notify(item->player_id, item->shelf_index);
	if (get_trade_state(item->state) == Trade_State_Sell)
	{
		update_trade_item_summary_broadcast(item->item_id);
	}
	item = NULL;
}

void update_trade_item_summary_broadcast(uint32_t item_id)
{
	TradeItemSummaryData nty;
	trade_item_summary_data__init(&nty);

	nty.itemid = item_id;
	nty.onsellnum = 0;
	std::pair<TradeItemMap::iterator, TradeItemMap::iterator> range = trade_item_map.equal_range(item_id);
	for (TradeItemMap::iterator iter = range.first; iter != range.second; ++iter)
	{
		TradeItem *item = iter->second;
		if (get_trade_state(item->state) == Trade_State_Sell)
		{
			nty.onsellnum += item->num;
		}
	}

	nty.averageprice = get_trade_item_average_price(item_id);
	if (nty.averageprice == 0)
	{
		TradingTable *config = get_config_by_id(item_id, &trade_item_config);
		if (config)
		{
			nty.averageprice = config->GuidePrice;
		}
	}

	conn_node_tradesrv::send_to_all_player(MSG_ID_TRADE_ITEM_SUMMARY_UPDATE_NOTIFY, &nty, (pack_func)trade_item_summary_data__pack);
}

void update_trade_shelf_notify(TradeItem *item)
{
	TradeShelfData nty;
	trade_shelf_data__init(&nty);

	nty.index = item->shelf_index;
	nty.itemid = item->item_id;
	nty.num = item->num;
	nty.price = item->price;
	nty.state = get_trade_state(item->state);
	nty.time = item->time;

	EXTERN_DATA ext_data;
	ext_data.player_id = item->player_id;

	fast_send_msg(&conn_node_tradesrv::connecter, &ext_data, MSG_ID_TRADE_SHELF_UPDATE_NOTIFY, trade_shelf_data__pack, nty);
}

void delete_trade_shelf_notify(uint64_t player_id, uint32_t shelf_index)
{
	TradeShelfData nty;
	trade_shelf_data__init(&nty);

	nty.index = shelf_index;
	nty.itemid = 0;
	nty.num = 0;
	nty.price = 0;
	nty.state = 0;
	nty.time = 0;

	EXTERN_DATA ext_data;
	ext_data.player_id = player_id;

	fast_send_msg(&conn_node_tradesrv::connecter, &ext_data, MSG_ID_TRADE_SHELF_UPDATE_NOTIFY, trade_shelf_data__pack, nty);
}

int get_trade_state(uint32_t state)
{
	return (state % TRADE_OPERATE_MASK);
}
void set_trade_state(uint32_t &state, uint32_t state_new)
{
	state = state / TRADE_OPERATE_MASK * TRADE_OPERATE_MASK;
	state = state + state_new;
}
int get_trade_operate(uint32_t state)
{
	return (state / TRADE_OPERATE_MASK);
}
void set_trade_operate(uint32_t &state, uint32_t operate)
{
	state = state % TRADE_OPERATE_MASK;
	state = state + operate * TRADE_OPERATE_MASK;
}

int get_trade_aduit_time(void)
{
	return ((rand() % (sg_audit_time[1] - sg_audit_time[0] + 1)) + sg_audit_time[0]);
}

int pack_auction_lot(AuctionLot *lot, uint8_t *out_data)
{
	DBAuctionLot db_info;
	DBAuctionLot *db_lot = &db_info;
	dbauction_lot__init(&db_info);

	db_lot->masters = lot->masters;
	db_lot->n_masters = 0;
	for (int i = 0; i < MAX_AUCTION_MASTER_NUM; ++i)
	{
		if (lot->masters[i] == 0)
		{
			break;
		}
		db_lot->n_masters++;
	}

	return dbauction_lot__pack(db_lot, out_data);
}

int save_auction_lot(AuctionLot *lot, bool insert)
{
	static char save_sql[32 * 1024 + 300];
	static uint8_t save_data[32 * 1024 + 1];
	uint64_t effect = 0;
	int len;
	char *p;
	
	size_t data_size = pack_auction_lot(lot, save_data);
	if (insert)
	{
		len = sprintf(save_sql, "insert auction_lot set `uuid` = %lu, `lot_id` = %u, `price` = %u, `time` = FROM_UNIXTIME(%u), `type` = %u, `type_limit` = %u, `bidder_id` = %lu, `create_time` = now(), `comm_data` = \'", lot->uuid, lot->lot_id, lot->price, lot->time, lot->type, lot->type_limit, lot->bidder_id);
		p = save_sql + len;
		p += escape_string(p, (const char *)save_data, data_size);
		len = sprintf(p, "\'");
	}
	else
	{
		len = sprintf(save_sql, "update auction_lot set `lot_id` = %u, `price` = %u, `time` = FROM_UNIXTIME(%u), `type` = %u, `type_limit` = %u, `bidder_id` = %lu, `comm_data` = \'", lot->lot_id, lot->price, lot->time, lot->type, lot->type_limit, lot->bidder_id);
		p = save_sql + len;
		p += escape_string(p, (const char *)save_data, data_size);
		len = sprintf(p, "\' where uuid = %lu", lot->uuid);
	}

	query(const_cast<char*>("set names utf8"), 1, NULL);
	query(save_sql, 1, &effect);	
	if (effect != 1) 
	{
		LOG_ERR("[%s:%d] save auction lot failed, uuid:%lu, lot_id:%u", __FUNCTION__, __LINE__, lot->uuid, lot->lot_id);
		return -1;
	}

	return 0;
}

int insert_auction_bid(uint64_t lot_uuid, uint64_t player_id, uint64_t price)
{
	AuctionBidMap::iterator iter = auction_bid_map.find(std::make_pair(lot_uuid, player_id));
	if (iter != auction_bid_map.end())
	{
		iter->second = price;
	}
	else
	{
		auction_bid_map.insert(std::make_pair(std::make_pair(lot_uuid, player_id), price));
	}


	char save_sql[300];
	uint64_t effect = 0;
	
	sprintf(save_sql, "insert auction_bid set `lot_uuid` = %lu, `player_id` = %lu, `price` = %lu, `time` =  now();", lot_uuid, player_id, price);

	query(const_cast<char*>("set names utf8"), 1, NULL);
	query(save_sql, 1, &effect);	
	if (effect != 1) 
	{
		LOG_ERR("[%s:%d] save auction bid failed, uuid:%lu, player_id:%lu, price:%lu", __FUNCTION__, __LINE__, lot_uuid, player_id, price);
		return -1;
	}

	return 0;
}

int delete_auction_lot_from_db(uint64_t lot_uuid)
{
	char sql[300];
	uint64_t effect = 0;
	
	sprintf(sql, "update auction_lot set over = 1 where uuid = %lu", lot_uuid);

	query(const_cast<char*>("set names utf8"), 1, NULL);
	query(sql, 1, &effect);	
	if (effect != 1) 
	{
		LOG_ERR("[%s:%d] mark delete trade item failed, sql:\"%s\", error:%s", __FUNCTION__, __LINE__, sql, mysql_error());
		return -1;
	}
	return 0;
}

uint64_t alloc_lot_uuid()
{
	static uint64_t last_uuid;
	uint32_t t = time_helper::get_cached_time() / 1000 & 0xffffffff;
	if ((last_uuid & 0xffffffff) == t)
	{
		uint64_t high = last_uuid >> 32;
		++high;
		last_uuid = high << 32 | t;
	}
	else
	{
		last_uuid = t;
	}
	LOG_DEBUG("[%s:%d] uuid[%lu]", __FUNCTION__, __LINE__, last_uuid);	
	return (last_uuid);
}

AuctionLot *create_auction_lot(uint32_t lot_id, uint32_t type, uint32_t type_limit, uint64_t *masters, uint32_t master_num)
{
	AuctionTable *config = get_config_by_id(lot_id, &auction_config);
	if (!config)
	{
		return NULL;
	}

	AuctionLot *lot = NULL;
	if (auction_lot_free_list.empty())
	{
		lot = (AuctionLot *)malloc(sizeof(AuctionLot));
		if (!lot)
		{
			LOG_ERR("[%s:%d] malloc auction lot failed, lot_id:%u", __FUNCTION__, __LINE__, lot_id);
			return NULL;
		}
	}
	else
	{
		lot = auction_lot_free_list.front();
		auction_lot_free_list.pop_front();
	}

	memset(lot, 0, sizeof(AuctionLot));
	lot->uuid = alloc_lot_uuid();
	lot->lot_id = lot_id;
	lot->price = config->DefaultPrice;
	lot->time = time_helper::get_cached_time() / 1000 + config->Time;
	lot->type = type;
	lot->type_limit = type_limit;
	for (uint32_t i = 0; i < master_num && i < MAX_AUCTION_MASTER_NUM; ++i)
	{
		lot->masters[i] = masters[i];
	}

	auction_lot_map.insert(std::make_pair(lot->uuid, lot));
	save_auction_lot(lot, true);

	return lot;
}

void remove_auction_lot(AuctionLot *&lot, bool lot_map_erase)
{
	if (lot_map_erase)
	{
		auction_lot_map.erase(lot->uuid);
	}
	auction_lot_free_list.push_back(lot);
	for (AuctionBidMap::iterator iter = auction_bid_map.begin(); iter != auction_bid_map.end(); )
	{
		if (iter->first.first == lot->uuid)
		{
			auction_bid_map.erase(iter++);
		}
		else
		{
			iter++;
		}
	}
	delete_auction_lot_from_db(lot->uuid);
	lot = NULL;
}

AuctionLot *get_auction_lot(uint64_t uuid)
{
	AuctionLotMap::iterator iter = auction_lot_map.find(uuid);
	if (iter != auction_lot_map.end())
	{
		return iter->second;
	}
	return NULL;
}

int get_player_auction_lot_state(AuctionLot *lot, uint64_t player_id)
{
	AuctionBidMap::iterator iter = auction_bid_map.find(std::make_pair(lot->uuid, player_id));
	if (iter != auction_bid_map.end())
	{
		if (lot->bidder_id == player_id)
		{
			return 1; //已竞价
		}
		else
		{
			return 2; //竞价失败
		}
	}

	return 0; //未竞价
}

void send_player_auction_bid_fail(uint64_t player_id, uint32_t price)
{
	uint32_t *pData = (uint32_t *)conn_node_tradesrv::get_send_data();
	uint32_t data_len = sizeof(uint32_t);
	*pData = price;

	EXTERN_DATA ext_data;
	ext_data.player_id = player_id;

	fast_send_msg_base(&conn_node_tradesrv::connecter, &ext_data, SERVER_PROTO_TRADE_BID_FAIL_RETURN, data_len, 0);
}


uint64_t alloc_red_packet_uuid()
{
//	static uint64_t last_uuid;
	uint64_t t = time_helper::get_cached_time() / 1000 & 0xffffffff;

//	{
		static union uuid_data last_ret_data;
		union uuid_data ret_data;

		if (last_ret_data.data2.time == t)
		{
			last_ret_data.data2.type = RED_PACKET_TYPE_UUID;			
			last_ret_data.data2.auto_inc++;
			ret_data.data1 = last_ret_data.data1;
		}
		else
		{
			ret_data.data1 = 0;
			ret_data.data2.time = t;
			ret_data.data2.type = RED_PACKET_TYPE_UUID;			
			last_ret_data.data1 = ret_data.data1;
		}
//	}
	
	// if ((last_uuid & 0xffffffff) == t) {
	// 	uint64_t high = last_uuid >> 32;
	// 	++high;
	// 	last_uuid = high << 32 | t;
	// 	last_uuid |= AI_PLAYER_TYPE_BIT;
	// 	goto done;
	// }
	// last_uuid = t | AI_PLAYER_TYPE_BIT;
//done:
	LOG_DEBUG("%s %d: uuid[%lu]", __FUNCTION__, __LINE__, ret_data.data1);
//	assert(last_uuid == ret_data.data1);
//	return (last_uuid);
	return ret_data.data1;
}

//加载所有红包uuid->time数据
void load_red_packet_redis_data()
{
	normal_red_packet_time_map.clear();
	guild_red_packet_time_map.clear();

	uint64_t cur_times = time_helper::get_micro_time() / 1000000;
	//普通红包
	CAutoRedisReply autoR;		
	redisReply *r = sg_redis_client.hgetall_bin(sg_normal_red_packet_key, autoR);
	if (!r || r->type != REDIS_REPLY_ARRAY)
	{
		LOG_ERR("[%s:%d] hgetall  normal red packet failed", __FUNCTION__, __LINE__);
	}
	else 
	{
		for(size_t i = 0; i + 1 <  r->elements; i = i + 2)
		{
			uint64_t red_uuid = 0;
			struct redisReply *field = r->element[i];
			struct redisReply *value = r->element[i+1];
			if (field->type != REDIS_REPLY_STRING || value->type != REDIS_REPLY_STRING)
				continue;
			red_uuid = strtoull(field->str, NULL, 10);
		
			if(red_uuid == 0)
				continue;
			RedPacketRedisInfo *red_redis_info = red_packet_redis_info__unpack(NULL, value->len, (const uint8_t*)value->str);
			if(red_redis_info == NULL)
			{
				LOG_ERR("[%s:%d] loading 红包信息时,解包红包信息失败 red_uuid[%lu]", __FUNCTION__, __LINE__, red_uuid);
				continue;
			}
			if(cur_times < red_redis_info->send_red_time)
			{
				LOG_ERR("[%s:%d] loading 红包信息时,时间有误 red_uuid[%lu] cur_time[%lu] send_time[%lu]", __FUNCTION__, __LINE__, red_uuid, cur_times, red_redis_info->send_red_time);
				continue;
			}

			//超过限定的存在时间将其在当前redis里面的数据清除
			if(cur_times - red_redis_info->send_red_time >= sg_red_packet_baocun_time )
			{
				if(sg_redis_client.hdel(sg_normal_red_packet_key, red_uuid) < 0)
				{
					LOG_ERR("[%s:%d] loading 红包信息时,删除超时红包出错 red_uuid[%lu] cur_time[%lu] send_time[%lu]", __FUNCTION__, __LINE__, red_uuid, cur_times, red_redis_info->send_red_time);
					continue;
				}

				//把剩余的钱退给发红包的玩家
				if(red_redis_info->red_use_money < red_redis_info->red_sum_money && red_redis_info->red_use_num < red_redis_info->red_sum_num)
				{
					red_packet_surplus_money_give_back_player(red_redis_info->player_id, red_redis_info->red_typ, red_redis_info->red_coin_type, red_redis_info->send_red_time, red_redis_info->red_sum_money, red_redis_info->red_use_money);
				}
				continue;
			}
			normal_red_packet_time_map.insert(std::make_pair(red_uuid, red_redis_info->send_red_time));
		}
	
	}

	//帮派红包
	r = sg_redis_client.hgetall_bin(sg_guild_red_packet_key, autoR);
	if (!r || r->type != REDIS_REPLY_ARRAY)
	{
		LOG_ERR("[%s:%d] hgetall guild red packet failed", __FUNCTION__, __LINE__);
	}
	else 
	{
		for(size_t i = 0; i + 1 <  r->elements; i = i + 2)
		{
			uint64_t red_uuid = 0;
			struct redisReply *field = r->element[i];
			struct redisReply *value = r->element[i+1];
			if (field->type != REDIS_REPLY_STRING || value->type != REDIS_REPLY_STRING)
				continue;
			red_uuid = strtoull(field->str, NULL, 10);
		
			if(red_uuid == 0)
				continue;
			RedPacketRedisInfo *red_redis_info = red_packet_redis_info__unpack(NULL, value->len, (const uint8_t*)value->str);
			if(red_redis_info == NULL)
			{
				LOG_ERR("[%s:%d] loading 红包信息时,解包红包信息失败 red_uuid[%lu]", __FUNCTION__, __LINE__, red_uuid);
				continue;
			}
			if(cur_times < red_redis_info->send_red_time)
			{
				LOG_ERR("[%s:%d] loading 红包信息时,时间有误 red_uuid[%lu] cur_time[%lu] send_time[%lu]", __FUNCTION__, __LINE__, red_uuid, cur_times, red_redis_info->send_red_time);
				continue;
			}

			//超过限定的存在时间将其在当前redis里面的数据清除
			if(cur_times - red_redis_info->send_red_time >= sg_red_packet_baocun_time )
			{
				if(sg_redis_client.hdel(sg_guild_red_packet_key, red_uuid) < 0)
				{
					LOG_ERR("[%s:%d] loading 红包信息时,删除超时红包出错 red_uuid[%lu] cur_time[%lu] send_time[%lu]", __FUNCTION__, __LINE__, red_uuid, cur_times, red_redis_info->send_red_time);
					continue;
				}

				//把剩余的钱退给发红包的玩家
				if(red_redis_info->red_use_money < red_redis_info->red_sum_money && red_redis_info->red_use_num < red_redis_info->red_sum_num && red_redis_info->system_or_player == 0)
				{
					red_packet_surplus_money_give_back_player(red_redis_info->player_id, red_redis_info->red_typ, red_redis_info->red_coin_type, red_redis_info->send_red_time, red_redis_info->red_sum_money, red_redis_info->red_use_money);
				}
				continue;
			}
			std::map<uint32_t, std::map<uint64_t, uint64_t> >::iterator iter = guild_red_packet_time_map.find(red_redis_info->guild_id);
			if(iter != guild_red_packet_time_map.end())
			{
				std::map<uint64_t, uint64_t> &guild_red_map = iter->second;
				guild_red_map[red_uuid] = red_redis_info->send_red_time;
			}
			else 
			{
				std::map<uint64_t, uint64_t> guild_red_map;
				guild_red_map[red_uuid] = red_redis_info->send_red_time;
				guild_red_packet_time_map.insert(std::make_pair(red_redis_info->guild_id, guild_red_map));
			}
		}
	
	}

}

//定时更新过时红包信息
void refresh_all_red_packet_redis_data()
{
	uint64_t cur_times = time_helper::get_micro_time() / 1000000;
	AutoReleaseTradeRedisInfo autoR;
	for(std::map<uint64_t, uint64_t>::iterator itr = normal_red_packet_time_map.begin(); itr != normal_red_packet_time_map.end();)
	{
		uint64_t red_uuid = itr->first;
		uint64_t send_time = itr->second;
		if(cur_times < send_time)
		{
			itr++;
			LOG_ERR("[%s:%d] 更新普通红包信息时,时间有误 red_uuid[%lu] cur_time[%lu] send_time[%lu]", __FUNCTION__, __LINE__, red_uuid, cur_times, send_time);
			continue;
		}
		if(cur_times - send_time > sg_red_packet_baocun_time)
		{
			RedPacketRedisInfo *last_red_info = get_red_packet_redis_info(red_uuid, sg_normal_red_packet_key, sg_redis_client, autoR);
			if(last_red_info == NULL)
			{
				LOG_ERR("[%s:%d] 删除过时普通红包失败,获取redis信息失败 red_uuid[%lu]", __FUNCTION__, __LINE__, red_uuid);
			}
			else{
				if(sg_redis_client.hdel(sg_normal_red_packet_key, red_uuid) < 0)
				{
					LOG_ERR("[%s:%d] 删除过时普通红包失败,删除redis信息失败 red_uuid[%lu]", __FUNCTION__, __LINE__, red_uuid);
				}
				else 
				{
					red_packet_surplus_money_give_back_player(last_red_info->player_id, last_red_info->red_typ, last_red_info->red_coin_type, last_red_info->send_red_time, last_red_info->red_sum_money, last_red_info->red_use_money);
					normal_red_packet_time_map.erase(itr++);
					continue;
				}
			}
		}
		itr++;
	}

	//帮会红包
	for(std::map<uint32_t, std::map<uint64_t, uint64_t> >::iterator itr = guild_red_packet_time_map.begin(); itr != guild_red_packet_time_map.end();itr++)
	{
		std::map<uint64_t, uint64_t> &guild_red_packet_map = itr->second;
		for(std::map<uint64_t, uint64_t>::iterator ite = guild_red_packet_map.begin(); ite != guild_red_packet_map.end();)
		{
			uint64_t red_uuid = ite->first;
			uint64_t send_time = ite->second;
			if(cur_times < send_time)
			{
				ite++;
				LOG_ERR("[%s:%d] 更新门宗红包信息时,时间有误 red_uuid[%lu] cur_time[%lu] send_time[%lu]", __FUNCTION__, __LINE__, red_uuid, cur_times, send_time);
				continue;
			}
			if(cur_times - send_time > sg_red_packet_baocun_time)
			{
				RedPacketRedisInfo *last_red_info = get_red_packet_redis_info(red_uuid, sg_guild_red_packet_key, sg_redis_client, autoR);
				if(last_red_info == NULL)
				{
					LOG_ERR("[%s:%d] 删除过时门宗红包失败,获取redis信息失败 red_uuid[%lu]", __FUNCTION__, __LINE__, red_uuid);
				}
				else{
					if(sg_redis_client.hdel(sg_guild_red_packet_key, red_uuid) < 0)
					{
						LOG_ERR("[%s:%d] 删除过时门宗红包失败,删除redis信息失败 red_uuid[%lu]", __FUNCTION__, __LINE__, red_uuid);
					}
					else 
					{
						if(last_red_info->system_or_player == 0)
						{
							red_packet_surplus_money_give_back_player(last_red_info->player_id, last_red_info->red_typ, last_red_info->red_coin_type, last_red_info->send_red_time, last_red_info->red_sum_money, last_red_info->red_use_money);
						}
						guild_red_packet_map.erase(ite++);
						continue;
					}

				}
			}
			ite++;
		}
	}
}

//将过期的红包里面剩余的金钱还给玩家
void red_packet_surplus_money_give_back_player(uint64_t player_id, uint32_t red_type, uint32_t money_type, uint64_t send_time, uint32_t sum_money, uint32_t use_money)
{
	if(use_money >= sum_money)
	{
		return;
	}
	std::map<uint32_t, uint32_t> mail_item_map;
	std::vector<char *> red_tui_huan_vect;
	struct tm tm;
	time_t befor_send_time = send_time;
	localtime_r(&befor_send_time, &tm);
	char red_time_text[256] = {0};
	char red_type_text[256] = {0};
	if(red_type == 1)
	{
		snprintf(red_type_text, 256, "世界红包");
	}
	else
	{
		snprintf(red_type_text, 256, "帮会红包");
	}
	snprintf(red_time_text, 256, "%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	uint32_t shengyu_money = sum_money - use_money;
	if(money_type == 0)
	{
		mail_item_map[201010003] = shengyu_money;
	}
	else 
	{
		mail_item_map[201010001] = shengyu_money;
	}
	red_tui_huan_vect.push_back(red_time_text);
	red_tui_huan_vect.push_back(red_type_text);
	send_mail(&conn_node_tradesrv::connecter, player_id, 270100012, NULL, NULL, NULL, &red_tui_huan_vect, &mail_item_map, MAGIC_TYPE_RED_PACKET_TUIHUAN_MONEY);

}

int delete_one_red_packet_for_redis(uint64_t last_red_uuid ,char* red_packet_key, std::map<uint64_t, uint64_t> &red_map)
{
	AutoReleaseTradeRedisInfo autoR;
	RedPacketRedisInfo *last_red_info = get_red_packet_redis_info(last_red_uuid, red_packet_key, sg_redis_client, autoR);
	if(last_red_info == NULL)
	{
		LOG_ERR("[%s:%d] 删除红包reids信息失败,获取红包redis信息失败", __FUNCTION__, __LINE__);
		return -1;
	}
	//将其在当前redis里面的数据清除
	if(sg_redis_client.hdel(red_packet_key, last_red_uuid) < 0)
	{
		LOG_ERR("[%s:%d] 删除红包reids信息失败,hdel redis信息失败", __FUNCTION__, __LINE__);
		return -2;
	}
	red_map.erase(last_red_uuid);

	//把剩余的钱退给发红包的玩家
	if(last_red_info->red_use_money < last_red_info->red_sum_money && last_red_info->red_use_num < last_red_info->red_sum_num && last_red_info->system_or_player == 0)
	{
		red_packet_surplus_money_give_back_player(last_red_info->player_id, last_red_info->red_typ, last_red_info->red_coin_type, last_red_info->send_red_time, last_red_info->red_sum_money, last_red_info->red_use_money);
	}
	return 0;

}

int save_one_red_packet_for_redis(RedPacketRedisInfo *redis_info, uint64_t red_uuid, char* red_packet_key)
{
	//数据存redis
	uint8_t data_buffer[200 * 1024];
	size_t data_len = red_packet_redis_info__pack(redis_info, data_buffer);
	if (data_len == (size_t)-1)
	{
		LOG_ERR("[%s:%d] 红包存数据库失败,数据长度有误 data_len[%lu]", __FUNCTION__, __LINE__, data_len);
		return -1;
	}
	char red_field[64];
	sprintf(red_field, "%lu", red_uuid);
	if (sg_redis_client.hset_bin(red_packet_key, red_field, (const char *)data_buffer, (int)data_len) < 0)
	{
		LOG_ERR("[%s:%d] 红包存数据库失败,hset redis 失败 data_len[%lu]", __FUNCTION__, __LINE__, data_len);
		return -2;
	}

	return 0;
}

//修改玩家领取红包最佳记录
int modify_player_red_packet_optimum_record(uint64_t player_id, uint64_t red_uuid)
{
	AutoReleaseTradeRedisInfo pool;
	RedPacketRedisPlayerReciveRecord *player_recive_info = get_player_red_packet_redis_recive_record(player_id, sg_player_red_packet_record_key, sg_redis_client, pool);
	if(player_recive_info == NULL)
	{
		LOG_ERR("[%s:%d] 更新玩家手气最佳红包信息失败,获取玩家redis信息失败player_id[%lu] red_uuid[%lu]", __FUNCTION__, __LINE__, player_id, red_uuid);
		return -1;
	}

	bool flag = false;
	for(size_t i = 0; i < player_recive_info->n_info; i++)
	{
		if(player_recive_info->info[i]->red_uuid == red_uuid && player_recive_info->info[i]->red_type == 0)
		{
			player_recive_info->info[i]->red_type = 1;
			flag = true;
		}
	}
	if(flag == false)
	{
		add_player_red_packet_history_max_num(player_id);
	}
	//数据存redis
	uint8_t data_buffer[200 * 1024];
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

//更新玩家历史红包记录
void updata_player_red_packet_history_info(RedPacketRedisPlayeNormalInfo* temp_info, uint64_t player_id)
{
	if(temp_info == NULL)
	{
		LOG_ERR("[%s:%d] 更新玩家红包历史记录有错 player_id[%lu]", __FUNCTION__, __LINE__, player_id);
		return;
	}
	AutoReleaseTradeRedisInfo pool;
	//将移除的记录计入到历史库
	RedPacketRedisPlayerAllJiluInfo *red_packet_history = get_player_red_packet_redis_all_history_record(player_id, sg_player_red_packet_all_history_key, sg_redis_client, pool);
	RedPacketRedisPlayerAllJiluInfo new_red_packet_history;
	red_packet_redis_player_all_jilu_info__init(&new_red_packet_history);
	if(red_packet_history == NULL)
	{
		if(temp_info->red_type == 0 || temp_info->red_type == 1)
		{
			new_red_packet_history.grab_red_num = 1;
			if(temp_info->red_type == 1)
			{
				new_red_packet_history.grab_max_red_num = 1;
			}
			if(temp_info->money_type == 0)
			{
				new_red_packet_history.grab_gold_num = temp_info->money_num;
			}
			else 
			{
				new_red_packet_history.grab_coin_num = temp_info->money_num;
			}
		}
		else 
		{
			new_red_packet_history.send_red_num = 1;
			if(temp_info->money_type == 0)
			{
				new_red_packet_history.send_gold_num = temp_info->money_num;
			}
			else 
			{
				new_red_packet_history.send_coin_num = temp_info->money_num;
			}
		}
	}
	else 
	{
		new_red_packet_history.send_red_num = red_packet_history->send_red_num;
		new_red_packet_history.send_gold_num = red_packet_history->send_gold_num;
		new_red_packet_history.send_coin_num = red_packet_history->send_coin_num;
		new_red_packet_history.grab_red_num = red_packet_history->grab_red_num;
		new_red_packet_history.grab_gold_num = red_packet_history->grab_gold_num;
		new_red_packet_history.grab_coin_num = red_packet_history->grab_coin_num;
		new_red_packet_history.grab_max_red_num = red_packet_history->grab_max_red_num;
		if(temp_info->red_type == 0 || temp_info->red_type == 1)
		{
			new_red_packet_history.grab_red_num += 1;
			if(temp_info->red_type == 1)
			{
				new_red_packet_history.grab_max_red_num += 1;
			}
			if(temp_info->money_type == 0)
			{
				new_red_packet_history.grab_gold_num += temp_info->money_num;
			}
			else 
			{
				new_red_packet_history.grab_coin_num += temp_info->money_num;
			}
		}
		else 
		{
			new_red_packet_history.send_red_num += 1;
			if(temp_info->money_type == 0)
			{
				new_red_packet_history.send_gold_num += temp_info->money_num;
			}
			else 
			{
				new_red_packet_history.send_coin_num += temp_info->money_num;
			}
		}
		
	}
	//数据存redis
	uint8_t data_buffer[512];
	size_t data_len = red_packet_redis_player_all_jilu_info__pack(&new_red_packet_history, data_buffer);
	if (data_len == (size_t)-1)
	{
		LOG_ERR("[%s:%d] 玩家已移除的红包记录计入历史失败,打包数据失败player_id[%lu] red_id[%lu] red_money_type[%u] red_money_num[%u] time[%lu] type[%u]", __FUNCTION__, __LINE__, player_id, temp_info->red_uuid, temp_info->money_type, temp_info->money_num, temp_info->grab_time, temp_info->red_type);
		return ;
	}
	char red_field[64];
	sprintf(red_field, "%lu", player_id);
	if (sg_redis_client.hset_bin(sg_player_red_packet_all_history_key, red_field, (const char *)data_buffer, (int)data_len) < 0)
	{
		LOG_ERR("[%s:%d] 玩家已移除的红包记录计入历史失败,存储数据失败player_id[%lu] red_id[%lu] red_money_type[%u] red_money_num[%u] time[%lu] type[%u]", __FUNCTION__, __LINE__, player_id, temp_info->red_uuid, temp_info->money_type, temp_info->money_num, temp_info->grab_time, temp_info->red_type);
		return;
	}

}

//增加玩家历史记录里面的手气最佳个数
void add_player_red_packet_history_max_num(uint64_t player_id)
{
	AutoReleaseTradeRedisInfo pool;
	//将移除的记录计入到历史库
	RedPacketRedisPlayerAllJiluInfo *red_packet_history = get_player_red_packet_redis_all_history_record(player_id, sg_player_red_packet_all_history_key, sg_redis_client, pool);
	if(red_packet_history == NULL)
	{
		LOG_ERR("[%s:%d] 增加玩家最佳手气红包数量失败, 获取redis数据失败player_id[%lu]", __FUNCTION__, __LINE__, player_id);
		return;
	}
	red_packet_history->grab_max_red_num += 1;
	//数据存redis
	uint8_t data_buffer[512];
	size_t data_len = red_packet_redis_player_all_jilu_info__pack(red_packet_history, data_buffer);
	if (data_len == (size_t)-1)
	{
		LOG_ERR("[%s:%d] 增加玩家最佳手气红包数量失败,打包数据失败player_id[%lu]", __FUNCTION__, __LINE__, player_id);
		return ;
	}
	char red_field[64];
	sprintf(red_field, "%lu", player_id);
	if (sg_redis_client.hset_bin(sg_player_red_packet_all_history_key, red_field, (const char *)data_buffer, (int)data_len) < 0)
	{
		LOG_ERR("[%s:%d] 增加玩家最佳手气红包数量失败,存储数据失败player_id[%lu]", __FUNCTION__, __LINE__, player_id);
		return;
	}
}

