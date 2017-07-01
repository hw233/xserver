#ifndef CAMP_JUDGE_H
#define CAMP_JUDGE_H

#include "unit.h"
#include "global_param.h"

#define PK_TYPE_NORMAL 0
#define PK_TYPE_CAMP 1
#define PK_TYPE_MURDER 2


//bool check_can_attack(unit_struct *attack, unit_struct *defence);
UNIT_FIGHT_TYPE get_unit_fight_type(unit_struct *attack, unit_struct *defence);

#endif /* CAMP_JUDGE_H */
