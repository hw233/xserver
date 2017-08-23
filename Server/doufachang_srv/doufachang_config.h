#ifndef DOUFACHANG_CONFIG_H
#define DOUFACHANG_CONFIG_H
#include <map>
#include <stdint.h>
#include "excel_data.h"


extern std::map<uint64_t, struct ArenaRewardTable*> doufachang_reward_config; //斗法场奖励

int read_all_config();

#endif /* DOUFACHANG_CONFIG_H */
