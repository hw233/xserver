#include "game_helper.h"
#include "comm_define.h"

int get_item_type(uint32_t item_id)
{
	if (item_id == 201010020)
	{
		return ITEM_TYPE_SILVER;		
	}
	if (item_id == 201010001)
	{
		return ITEM_TYPE_COIN;
	}
	if (item_id == 201010002)
	{
		return ITEM_TYPE_BIND_GOLD;
	}
	if (item_id == 201010003)
	{
		return ITEM_TYPE_GOLD;
	}
	if (item_id == 201010004)
	{
		return ITEM_TYPE_EXP;
	}
	if (item_id == 201010005)
	{
		return ITEM_TYPE_GUOYU_EXP;
	}
	if (item_id == 201010006)
	{
		return ITEM_TYPE_CHENGJIE_EXP;
	}
	if (item_id == 201010007)
	{
		return ITEM_TYPE_SHANGJIN_EXP;
	}
	// if (item_id == 201010008)
	// {
	// 	return ITEM_TYPE_GUOYU_COIN;
	// }
	// if (item_id == 201010009)
	// {
	// 	return ITEM_TYPE_CHENGJIE_COIN;
	// }
	if (item_id == 201010010)
	{
		return ITEM_TYPE_LINGSHI;
	}
	if (item_id == 201010011)
	{
		return ITEM_TYPE_GUILD_DONATION;
	}
	if (item_id == 201010012)
	{
		return ITEM_TYPE_GUILD_TREASURE;
	}
	if (item_id == 201010013)
	{
		return ITEM_TYPE_PARTNER_EXP;
	}
	if (item_id == 201010021)
	{
		return ITEM_TYPE_GONGXUN;
	}
	if (item_id == 201010022)
	{
		return ITEM_TYPE_XUEJING;
	}
	if (item_id == 201010023)
	{
		return ITEM_TYPE_SHENGWANG;
	}
	if (item_id >= 201070027 && item_id <= 201070036)
	{
		return ITEM_TYPE_EQUIP;
	}
	return ITEM_TYPE_ITEM;
}

