#ifndef SKILL_H
#define SKILL_H

#include <stdint.h>
#include "game_config.h"
#include "skill_const.h"
#include "unit_path.h"

static const int MAX_FUWEN = 3;
struct skill_data
{
	uint32_t skill_id;
//	SpellStatEvent state;
//	uint64_t skill_cast_time;    //施法开始时间，即吟唱结束时间
//	uint64_t skill_hit_time;    //飞行结束时间，即命中时间
	uint64_t owner; 		/* 施法者 resume用*/
//	uint64_t target;  		/* 施法目标 */
//	struct position pos;  		/* 目的地址 */
	uint32_t lv;
	uint32_t fuwen[MAX_FUWEN];
	int fuwen_num;
	uint32_t cur_fuwen;
	uint64_t cd_time;
};

class skill_struct
{
public:
	int init_skill(uint32_t id, uint64_t owner, uint64_t target);
	int add_cd(struct SkillLvTable *lv_config, struct ActiveSkillTable *active_config);
//	void on_tick();
	struct SkillTable *config; 		/* 技能配置 */
	struct skill_data *data;
private:
//	int check_skill_condition();  //检查施法条件
//	int calc_skill_effect();      //计算施法效果
//	int calc_buff_effect(uint64_t id, uint64_t target);	
};

#endif /* SKILL_H */
