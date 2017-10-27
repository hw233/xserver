#pragma once

#include <map>

#include "conn_node_gamesrv.h"

class skill_struct;
class player_struct;

struct _PlayerDBInfo;
struct fuwen_data;

class MySkill
{
public:
	static const int MAX_SKILL_NUM = 20;
	typedef std::vector<skill_struct *> SKILL_CONTAIN;

	void copy(MySkill *skill);  //复制一份，机器人用

	MySkill(player_struct *player);
	void clear();
	void Init();
	void OpenAllSkill();
	void OnPlayerLevelUp(uint32_t lv);
	int Learn(uint32_t id, uint32_t num);
	int SetFuwen(uint32_t id, uint32_t fuwen);
	fuwen_data * GetCurFuwen(uint32_t id);
	void SendAllSkill();
	skill_struct *GetSkillStructFromFuwen(uint32_t fuwen_id);  //根据符文id获取对应技能的对象
	uint32_t GetFirstSkillId();
	uint32_t GetRandSkillId(struct ai_player_data *ai_data);  //这个接口已经变成了获取下一个没有在CD的技能ID
	int GetSkillLevelNum(uint32_t lv); //获取超过指定等级的技能数（三段技只算1个）
	int GetFuwenUnlockNum(void); //获取解锁的符文数量
	int GetFuwenWearNum(void); //获取装备的符文数量
	int GetFuwenLevelNum(uint32_t lv); //获取超过指定等级的符文数
	
	skill_struct *InsertSkill(uint32_t id);

	void PackAllSkill(_PlayerDBInfo &pb);
	void UnPackAllSkill(_PlayerDBInfo &pb);

	void adjust_ai_player_skill();	

	skill_struct * GetSkill(uint32_t id);
	uint32_t GetLevelUpTo(uint32_t id, uint32_t initLv, uint32_t maxLv);
	uint32_t CalcCost(uint32_t id, uint32_t oldLv, uint32_t num);
	uint32_t m_index; //套餐下标
private:
//	skill_struct *GetNoFuwenSkillStruct(uint32_t skill_id);  //没有设置符文的技能ID对应的技能等级,被GetFuwenSkillLevel使用
	void IteratorLevelUp(uint32_t id, uint32_t num);  //升级关联技能
	bool is_second_skill(skill_struct *skill);  //是否是第二段技能
	SKILL_CONTAIN m_skill;
	player_struct *m_owner;
};
