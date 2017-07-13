#ifndef COUNT_SKILL_DAMAGE_H
#define COUNT_SKILL_DAMAGE_H

#include "unit.h"
#include "camp_judge.h"
#include <stdint.h>

#define SKILL_EFFECT_CRIT 1
#define SKILL_EFFECT_MISS 2
#define SKILL_EFFECT_ADDHP 3
//	required uint32 effect = 4;     //命中效果，闪避，暴击, 回复
//extern int32_t count_skill_total_damage(uint32_t skill_id, double *attack, double *defence, uint32_t *effect, uint32_t buff_add[], uint32_t *n_buff_add);

extern void get_skill_configs(uint32_t skill_lv, uint32_t skill_id, struct SkillTable **ski_config,
	struct SkillLvTable **lv_config1, struct PassiveSkillTable **pas_config,
	struct SkillLvTable **lv_config2, struct ActiveSkillTable **act_config);

extern int32_t count_skill_total_damage(UNIT_FIGHT_TYPE type, struct SkillTable *skillconfig, struct SkillLvTable *act_lvconfig,
	struct PassiveSkillTable *pas_config, struct SkillLvTable *pas_lvconfig, unit_struct *attack_unit,
	unit_struct *defence_unit, uint32_t *effect, uint32_t buff_add[], uint32_t buff_add_end_time[], uint32_t *n_buff_add, int32_t other_rate);

extern int32_t count_other_skill_damage_effect(unit_struct *attack, unit_struct *defence);

extern int32_t count_skill_effect(const double *attack, const double *defence,
	const double *buff_fight_attack, const double *buff_fight_defence,
	struct SkillEffectTable *effectconfig);

#endif /* COUNT_SKILL_DAMAGE_H */
