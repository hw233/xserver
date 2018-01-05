#ifndef __GAME_HELPER_H__
#define __GAME_HELPER_H__

#include <stdint.h>
#include "comm_define.h"

struct AttrInfo
{
	uint32_t id;
	double val;
};

struct CommonRandAttrInfo
{
	uint32_t pool;
	uint32_t id;
	double val;
};

union EspecialItemInfo
{
	struct{
		uint32_t star;
		CommonRandAttrInfo minor_attrs[MAX_BAGUAPAI_MINOR_ATTR_NUM]; //副属性
		CommonRandAttrInfo additional_attrs[MAX_BAGUAPAI_ADDITIONAL_ATTR_NUM]; //追加属性
	}baguapai;

	struct{
		AttrInfo main_attr;	   //主属性
		AttrInfo minor_attr[MAX_HUOBAN_FABAO_MINOR_ATTR_NUM];//副属性
	}fabao;

	struct{
		uint32_t item_id;
		uint32_t item_num;
	} box;
};

int get_item_type(uint32_t item_id); //获取道具类型

#endif
