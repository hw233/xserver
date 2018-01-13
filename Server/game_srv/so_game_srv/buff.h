#ifndef BUFF_H
#define BUFF_H

#include <stdint.h>
#include "excel_data.h"
#include "unit.h"
#include "scene.h"

#define BUFF_DEL_TYPE_DEAD 0
#define BUFF_DEL_TYPE_CHANGED_SCENE 1
#define BUFF_DEL_TYPE_LOGOUT 2
#define BUFF_DEL_TYPE_BEATTACK 3
#define BUFF_DEL_TYPE_BEHIT 4

//眩晕，不能移动不能攻击
#define BUFF_STATE_STUN 0x01
//无敌，不受任何伤害
#define BUFF_STATE_GOD 0x2
//嘲讽
#define BUFF_STATE_TAUNT 0x4
//暴击
#define BUFF_STATE_CRIT 0x8
//吸血
//#define BUFF_STATE_LIFE_STEAL 0x10
//反伤
//#define BUFF_STATE_DAMAGE_RETURN 0x20
//免控
#define BUFF_STATE_AVOID_TRAP 0x10
//-免疫PVP阴区域BUFF
#define BUFF_STATE_AVOID_BLUE_BUFF 0x20
//-免疫PVP阳区域BUFF
#define BUFF_STATE_AVOID_RED_BUFF 0x40
//剩余一滴血不死
#define BUFF_STATE_ONEBLOOD 0x80

struct buff_data
{
	uint32_t buff_id;
	uint64_t start_time;
	uint32_t lv;
	uint64_t end_time;
	uint64_t owner;
	uint64_t attacker;	

	union
	{
		struct 
		{
			uint32_t attr_id;
			double added_attr_value;  //修改的属性值，buff结束的时候要恢复
		} attr_effect;   //effect_config->Type == 170000008
		
		struct
		{
			int32_t hp_delta;     //每次修改的血量 
		} hp_effect;     //!is_recoverable_buff()

		struct
		{
			uint32_t state;
//			uint32_t value;
		} buff_state;
//		uint32_t buff_state;
	} effect;

	int heap_index;
	uint64_t ontick_time;
};

class buff_struct
{
public:
	static uint32_t get_skill_effect_by_buff_state(int state);
	void del_buff();
	int init_buff(struct BuffTable *buffconfig, uint64_t end_time, unit_struct *attack, unit_struct *owner);
	int reinit_buff(struct BuffTable *buffconfig, uint64_t end_time, unit_struct *attack);
	int reinit_type3_buff(struct BuffTable *buffconfig);
	int add_lv();
	bool is_recoverable_buff();   //需要恢复的buff, 包括属性和状态
	bool is_attr_buff();         //属性变更的buff，恢复的时候要恢复属性
	bool is_hp_buff();          //血量变化的buff，不需要恢复
	void on_tick();
	void on_dead();
	void on_beattack();
	void on_scene_changed();	
	void set_next_timer();
	buff_data *data;
	struct BuffTable *config;
	struct SkillEffectTable *effect_config;
	unit_struct *m_owner;
//	unit_struct *m_attacker;	//attacker下线，死亡等不会清除这个指针，注意，也许记录个UUID更好
private:
	bool do_hp_buff_effect(bool check_dead);
	unit_struct *get_attacker();
	void deal_with_del_effect();
	void deal_with_recover_effect();		
};

#endif /* BUFF_H */
