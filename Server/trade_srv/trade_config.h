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
extern uint32_t sg_grab_red_packet_min_level; //领取红包最低等级要求
extern uint32_t sg_red_packet_baocun_max_num; //红包保存最大数量
extern uint32_t sg_red_packet_jilu_max_num;   //红包历史记录最大数量
extern uint32_t sg_red_packet_baocun_time;    //红包保存时间
extern uint32_t sg_red_packet_max_can_recive_num; //所有类型红包中可领取的最大数量
extern uint32_t sg_red_packet_max_recive_replace; //最大数量替换值

int read_all_excel_data();

int get_item_config_type(uint32_t item_id);

#endif /* __TRADE_CONFIG_H__ */
