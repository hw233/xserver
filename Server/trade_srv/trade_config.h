#ifndef __TRADE_CONFIG_H__
#define __TRADE_CONFIG_H__

#include <map>
#include <vector>
#include <stdint.h>
#include "excel_data.h"
#include "lua_template.h"


extern std::map<uint64_t, struct ParameterTable *> parameter_config;
extern std::map<uint64_t, struct ItemsConfigTable *> items_config;
extern std::map<uint64_t, struct TradingTable*> trade_item_config; //交易物品表
extern std::map<uint64_t, struct AuctionTable*> auction_config; //拍卖品表

extern double sg_on_shelf_fee_percent;
extern double sg_trade_tax_percent;
extern uint32_t sg_audit_time[2];
extern uint32_t sg_keep_time;
extern uint32_t sg_shelf_init_num;
extern uint32_t sg_shelf_max_num;
extern uint32_t sg_shelf_enlarge_price;

int read_all_excel_data();

int get_item_config_type(uint32_t item_id);

#endif /* __TRADE_CONFIG_H__ */
