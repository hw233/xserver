#ifndef _TRADE_STRUCT_H__
#define _TRADE_STRUCT_H__

#include <stdint.h>
#include <vector>
#include <map>
#include <comm_define.h>
#include "game_helper.h"

enum
{
	Trade_State_Aduit = 0, //审核中
	Trade_State_Sell = 1, //出售中
	Trade_State_Overdue = 2, //已过期
};

//交易操作掩码
enum
{
	Trade_Operate_On_Shelf = 1, //上架
	Trade_Operate_Off_Shelf = 2, //下架
	Trade_Operate_Re_Shelf = 3, //重新上架
	Trade_Operate_Buy = 4, //购买
};
#define TRADE_OPERATE_MASK 100

enum
{
	Auction_Type_Guild = 1, //门宗拍卖
	Auction_Type_Server = 2, //全服拍卖
};

struct TradeItem
{
	uint64_t player_id; //所属玩家ID
	uint32_t shelf_index; //货架索引
	uint32_t item_id; //货品ID
	uint32_t num; //出售数量
	uint32_t price; //单价
	EspecialItemInfo especial; //道具特殊信息
	uint32_t state; //审核状态
	uint32_t time; //倒计时
};

typedef std::multimap<uint32_t, TradeItem *> TradeItemMap;

struct TradeSoldInfo
{
	uint32_t item_id;
	uint32_t num;
	uint32_t price;
};
typedef std::map<uint32_t, TradeSoldInfo *> TradeSoldMap;

struct AuctionLot
{
	uint64_t uuid; //唯一ID
	uint32_t lot_id; //配置ID
	uint32_t price; //当前价格
	uint32_t time; //倒计时
	uint32_t type; //类型，门宗还是全服
	uint32_t type_limit; //如果类型=门宗，则为门宗ID
	uint64_t masters[MAX_AUCTION_MASTER_NUM]; //产出该拍卖品的玩家
	uint64_t bidder_id; //当前竞价玩家ID
	uint64_t operator_id; //当前在操作的玩家ID
};
typedef std::map<uint64_t, AuctionLot *> AuctionLotMap;
typedef std::map<std::pair<uint64_t, uint64_t>, uint32_t> AuctionBidMap; //拍卖竞价<<lot_uuid, player_id>, price>

struct TradePlayer
{
	uint64_t player_id;
	uint32_t shelf_num; //寄售格数
	TradeItem *shelf_items[MAX_TRADE_SHELF_NUM]; //货架列表
	TradeSoldInfo sold_items[MAX_TRADE_SOLD_NUM]; //已出售记录
	uint32_t sold_earning; //已出售收益
	uint32_t auction_bid_money; //拍卖竞价失败返还的钱
};
typedef std::map<uint64_t, TradePlayer *> TradePlayerMap;

#endif
