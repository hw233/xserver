#include "cached_hit_effect.h"

SkillHitEffect cached_hit_effect[256];
SkillHitEffect *cached_hit_effect_point[256];
//PosData cached_attack_pos[256];
PosData cached_target_pos[256];
int32_t cached_target_index[256];
uint32_t cached_buff_id[512];
uint32_t cached_buff_end_time[512];
