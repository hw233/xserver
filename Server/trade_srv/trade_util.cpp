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
static TradeSoldMap trade_sold_map; //所有已售道具信息
static std::list<TradeItem *> trade_item_free_list;
static std::list<TradeSoldInfo *> trade_sold_free_list;

AuctionLotMap auction_lot_map; //所有拍卖道具
static AuctionBidMap auction_bid_map; //拍卖竞价列表
static std::list<AuctionLot *> auction_lot_free_list;

char sg_player_key[64];
struct event second_timer;
struct timeval second_timeout;
struct event five_oclock_timer;
struct timeval five_oclock_timeout;
static const uint32_t week_reset_day = 1 * 24 * 3600; //每周一刷新
static const uint32_t daily_reset_clock = 5 * 3600; //每天五点刷新
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
				uint32_t sold_price = lot->price;
				if (lot->bidder_id > 0)
				{ //拍卖成功
					std::map<uint32_t, uint32_t> attachs;
					attachs.insert(std::make_pair(config->ItemID, config->Num));
					send_mail(&conn_node_tradesrv::connecter, lot->bidder_id, buyer_mail_id, NULL, NULL, NULL, &buyer_mail_args, &attachs, MAGIC_TYPE_AUCTION_SOLD);
				}
				else
				{ //系统回收
					sold_price = sold_price * (1.0 - sg_trade_tax_percent);
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
		item->especial.baguapai.main_attr_val = db_item->bagua->main_attr_val;
		for (size_t i = 0; i < db_item->bagua->n_minor_attrs && i < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++i)
		{
			item->especial.baguapai.minor_attrs[i].id = db_item->bagua->minor_attrs[i]->id;
			item->especial.baguapai.minor_attrs[i].val = db_item->bagua->minor_attrs[i]->val;
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
	DBAttr item_bagua_attr[MAX_BAGUAPAI_MINOR_ATTR_NUM];
	DBAttr* item_bagua_attr_point[MAX_BAGUAPAI_MINOR_ATTR_NUM];
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
				item_bagua_data.main_attr_val = item->especial.baguapai.main_attr_val;
				uint32_t attr_num = 0;
				for (int j = 0; j < MAX_BAGUAPAI_MINOR_ATTR_NUM; ++j)
				{
					item_bagua_attr_point[attr_num] = &item_bagua_attr[attr_num];
					dbattr__init(&item_bagua_attr[attr_num]);
					item_bagua_attr[attr_num].id = item->especial.baguapai.minor_attrs[j].id;
					item_bagua_attr[attr_num].val = item->especial.baguapai.minor_attrs[j].val;
					attr_num++;
				}
				item_bagua_data.minor_attrs = item_bagua_attr_point;
				item_bagua_data.n_minor_attrs = attr_num;
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





