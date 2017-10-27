#ifndef __GAME_HELPER_H__
#define __GAME_HELPER_H__

#include <stdint.h>
#include "comm_define.h"

struct AttrInfo
{
	uint32_t id;
	double val;
};

union EspecialItemInfo
{
	struct{
		uint32_t star;
		double main_attr_val;
		AttrInfo minor_attrs[MAX_BAGUAPAI_MINOR_ATTR_NUM];
	}baguapai;

	struct{
		AttrInfo main_attr;	   //主属性
		AttrInfo minor_attr[MAX_HUOBAN_FABAO_MINOR_ATTR_NUM];//副属性
	}fabao;
};

int get_item_type(uint32_t item_id); //获取道具类型

#endif
