#pragma once

#include <map>

#include "conn_node_gamesrv.h"

class skill_struct;
class player_struct;

struct _PlayerDBInfo;

class MySkill
{
public:
	static const int MAX_SKILL_NUM = 20;
	typedef std::map<uint32_t, skill_struct *> SKILL_CONTAIN;

	MySkill(player_struct *player);
	void clear();
	void Init();
	void OpenAllSkill();
	void OnPlayerLevelUp(uint32_t lv);
	int Learn(uint32_t id, uint32_t num);
	int SetFuwen(uint32_t id, uint32_t fuwen);
	uint32_t GetCurFuwen(uint32_t id);
	void SendAllSkill();
	skill_struct *GetSkillStructFromFuwen(uint32_t fuwen_id);  //根据符文id获取对应技能的对象
	uint32_t GetFirstSkillId();
	uint32_t GetRandSkillId();  //这个接口已经变成了获取下一个没有在CD的技能ID
	
	skill_struct *InsertSkill(uint32_t id);

	void PackAllSkill(_PlayerDBInfo &pb);
	void UnPackAllSkill(_PlayerDBInfo &pb);

	skill_struct * GetSkill(uint32_t id);
	uint32_t GetLevelUpTo(uint32_t id);
	uint32_t CalcCost(uint32_t id, uint32_t oldLv, uint32_t num);
private:
	skill_struct *GetNoFuwenSkillStruct(uint32_t skill_id);  //没有设置符文的技能ID对应的技能等级,被GetFuwenSkillLevel使用
	void IteratorLevelUp(uint32_t id, uint32_t num);
	SKILL_CONTAIN m_skill;
	player_struct *m_owner;
};
