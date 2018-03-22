#ifndef SKILL_H
#define SKILL_H

#include <stdint.h>
#include "server_proto.h"
#include "game_config.h"
#include "skill_const.h"
#include "unit_path.h"
class skill_struct
{
public:
	void copy(skill_struct *skill);  //复制一份，机器人用
	int init_skill(uint32_t id, uint64_t owner, uint64_t target);
	int add_cd(uint64_t cd);
	int get_skill_id_and_lv(int fuwen_index, int *id, int *lv);	
//	int get_skill_lv(int fuwen_index);
//	void on_tick();
	struct SkillTable *config; 		/* 技能配置 */
	struct skill_data *data;
	fuwen_data *get_fuwen(uint32_t fuwen);
private:
//	int check_skill_condition();  //检查施法条件
//	int calc_skill_effect();      //计算施法效果
//	int calc_buff_effect(uint64_t id, uint64_t target);	
};

#endif /* SKILL_H */
