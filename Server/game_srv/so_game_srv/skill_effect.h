#ifndef SKILL_EFFECT_H
#define SKILL_EFFECT_H

#include <stdint.h>

class skill_effect
{
public:
	static int calc_skill_effect(uint64_t id, uint64_t target);	
};
#endif /* SKILL_EFFECT_H */
