#ifndef CACHED_HIT_EFFECT_H
#define CACHED_HIT_EFFECT_H

#include <stdint.h>
#include "cast_skill.pb-c.h"

extern SkillHitEffect cached_hit_effect[256];
extern SkillHitEffect *cached_hit_effect_point[256];
extern int32_t cached_target_index[256];
//extern PosData cached_attack_pos[256];
extern PosData cached_target_pos[256];
extern uint32_t cached_buff_id[512];

#endif /* CACHED_HIT_EFFECT_H */
