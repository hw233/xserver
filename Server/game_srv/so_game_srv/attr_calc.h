#ifndef ATTR_CALC_H
#define ATTR_CALC_H

#include <stdint.h>

void get_attr_from_config(uint64_t attr_table_id, double *attr, uint32_t *drop_id); //把配置表属性读到数组中
void add_fight_attr(double *dest_attr, double *src_attr); //把战斗属性相加
uint32_t calculate_fighting_capacity(double *attr); //计算战斗力

#endif /* ATTR_CALC_H */
