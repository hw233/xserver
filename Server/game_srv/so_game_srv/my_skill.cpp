#include "my_skill.h"

#include "msgid.h"
#include "cast_skill.pb-c.h"
#include "player_db.pb-c.h"
#include "player.h"
#include "game_config.h"
#include "skill_manager.h"
#include "app_data_statis.h"
#include "time_helper.h"

FuwenData fuwenData[MAX_MY_SKILL_NUM][MAX_FUWEN];
FuwenData *fuwenDataPoint[MAX_MY_SKILL_NUM][MAX_FUWEN];
FuwenData fuwenData1[MAX_MY_SKILL_NUM][MAX_FUWEN];
FuwenData *fuwenDataPoint1[MAX_MY_SKILL_NUM][MAX_FUWEN];
SkillData skillData[MAX_MY_SKILL_NUM];
SkillData *skillDataPoint[MAX_MY_SKILL_NUM] = { NULL };
SkillData skillData1[MAX_MY_SKILL_NUM];
SkillData *skillDataPoint1[MAX_MY_SKILL_NUM] = { NULL };

MySkill::MySkill(player_struct *player)
{
	m_owner = player;
}

void MySkill::clear()
{
	for (SKILL_CONTAIN::iterator it = m_skill.begin(); it != m_skill.end(); ++it)
	{
		skill_manager::delete_skill(*it);
	}
	m_skill.clear();
}

void MySkill::Init()
{
	if (!m_skill.empty())
	{
		return;
	}
	m_index = 1;
	std::map<uint64_t, struct SkillTable *>::iterator it = skill_config.begin();
	for (; it != skill_config.end(); ++it)
	{
		if (it->second->IsRune == 1)
		{
			continue;
		}
		if (it->second->OpenLv > m_owner->get_attr(PLAYER_ATTR_LEVEL))
		{
			continue;
		}
		if (it->second->Career != m_owner->get_attr(PLAYER_ATTR_JOB))
		{
			continue;
		}
		InsertSkill(it->second->ID);
	}
}

void MySkill::OpenAllSkill()
{
	clear();
	std::map<uint64_t, struct SkillTable *>::iterator it = skill_config.begin();
	for (; it != skill_config.end(); ++it)
	{
		if (it->second->IsRune == 1)
		{
			continue;
		}
		if (it->second->Career != m_owner->get_attr(PLAYER_ATTR_JOB))
		{
			continue;
		}
		InsertSkill(it->second->ID);
	}
}

int MySkill::Learn(uint32_t id, uint32_t num)
{
	skill_struct *it = GetSkill(id);
	if (it == NULL)
	{
		return 0;
	}
	std::map<uint64_t, struct SkillLvTable *>::iterator iter = skill_lv_config.find(it->config->SkillLv + it->data->lv + num - 1);
	if (iter == skill_lv_config.end())
		return 1;
	if (iter->second->NeedLv > m_owner->get_attr(PLAYER_ATTR_LEVEL))
	{
		return 190500068;
	}
	uint32_t cost = CalcCost(it->config->SkillLv, it->data->lv, num);
	if (m_owner->sub_coin(cost, MAGIC_TYPE_SKILL) != 0)
	{
		return 190500063;
	}
	IteratorLevelUp(id, num);
	m_owner->add_task_progress(TCT_SKILL_LEVEL_UP, get_actor_skill_index(m_owner->get_job(), id), num);
	m_owner->add_achievement_progress(ACType_SKILL_LEVEL_UP, 0, 0, num);
	m_owner->add_achievement_progress(ACType_SKILL_ALL_LEVEL, 0, 0, num);
	m_owner->add_achievement_progress(ACType_SKILL_FUWEN_UNLOCK, 0, 0, GetFuwenUnlockNum());
	return 0;
}
void MySkill::IteratorLevelUp(uint32_t id, uint32_t num)
{
	skill_struct *it = GetSkill(id);
	if (it == NULL)
	{
		return;
	}
	it->data->lv += num;

	std::map<uint64_t, struct ActiveSkillTable *>::iterator itCon  = active_skill_config.find(it->config->SkillAffectId);
	if (itCon == active_skill_config.end())
	{
		return;
	}
	IteratorLevelUp(itCon->second->NextSkill, num);
}

skill_struct *MySkill::InsertSkill(uint32_t id)
{
	skill_struct * pSkillStruct = skill_manager::create_skill(id, m_owner->get_uuid(), 0);
	if (pSkillStruct != NULL)
	{
		m_skill.push_back(pSkillStruct);
	}
	return pSkillStruct;
}

void MySkill::OnPlayerLevelUp(uint32_t lv)
{
	SkillList send;
	skill_list__init(&send);

	int i = 0;
	std::map<uint64_t, struct SkillTable *>::iterator it = skill_config.begin();
	for (; it != skill_config.end(); ++it)
	{
		if (it->second->IsRune == 1)
		{
			continue;
		}
		skill_struct *itS = GetSkill(it->first);
		if (itS != NULL)
		{
			continue;
		}
		if (it->second->OpenLv <= lv && it->second->Career == m_owner->get_attr(PLAYER_ATTR_JOB))
		{
			skill_struct * pSkillStruct = InsertSkill(it->second->ID);
			if (pSkillStruct != NULL)
			{
				if (it->second->SkillAcc == 0)
				{
					continue;
				}
				skill_data__init(skillData + i);
				skillData[i].id = it->first;
				skillData[i].lv = pSkillStruct->data->lv;
				skillData[i].cur_fuwen = pSkillStruct->data->cur_fuwen[m_index - 1];
				skillData[i].n_fuwen = pSkillStruct->data->fuwen_num;
				for (int j = 0; j < pSkillStruct->data->fuwen_num; ++j)
				{
					fuwen_data__init(&fuwenData[i][j]);
					fuwenData[i][j].id = pSkillStruct->data->fuwen[j].id;
					fuwenData[i][j].lv = pSkillStruct->data->fuwen[j].lv;
					fuwenData[i][j].is_new = pSkillStruct->data->fuwen[j].isNew;
					fuwenDataPoint[i][j] = &fuwenData[i][j];
				}
				skillData[i].fuwen = fuwenDataPoint[i];

				skillDataPoint[i] = skillData + i;
				++i;
			}
		}
	}

	send.n_data = i;
	send.data = skillDataPoint;

	if (i > 0)
	{
		EXTERN_DATA ext_data;
		ext_data.player_id = m_owner->get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_NEW_SKILL_NOTIFY, skill_list__pack, send);
	}
}

fuwen_data * MySkill::GetCurFuwen(uint32_t id)
{
	skill_struct *it = GetSkill(id);
	if (it == NULL)
	{
		return NULL;
	}
	if (it->data->cur_fuwen[m_index - 1] == 0)
	{
		return NULL;
	}
	for (int i = 0; i < it->data->fuwen_num; ++i)
	{
		if (it->data->fuwen[i].id == it->data->cur_fuwen[m_index - 1])
		{
			return &it->data->fuwen[i];
		}
	}
	return NULL;
}

int MySkill::SetFuwen(uint32_t id, uint32_t fuwen)
{
	skill_struct *it = GetSkill(id);
	if (it == NULL)
	{
		return 1;
	}

	if (fuwen == 0)
	{
		it->data->cur_fuwen[m_index - 1] = fuwen;
		return 0;
	}

	for (int i = 0; i < it->data->fuwen_num; ++i)
	{
		if (it->data->fuwen[i].id == fuwen)
		{
			std::map<uint64_t, struct SkillTable *>::iterator itCon = skill_config.find(fuwen);
			if (itCon == skill_config.end())
			{
				return 190500069;
			}
			if (it->data->lv < itCon->second->OpenLv)
			{
				return 190500069;
			}
			it->data->cur_fuwen[m_index - 1] = fuwen;
			return 0;
		}
	}
	return 2;
}

// skill_struct *MySkill::GetNoFuwenSkillStruct(uint32_t skill_id)
// {
// 	skill_struct *t = GetSkill(skill_id);
// 	if (!t || !t->data)
// 		return NULL;
// 	if (t->data->cur_fuwen[m_index - 1] != 0)
// 		return NULL;
// //	return t->data->lv;
// 	return t;
// }

skill_struct *MySkill::GetSkillStructFromFuwen(uint32_t fuwen_id)
{
 	skill_struct *t = GetSkill(fuwen_id);
 	if (!t || !t->data)
 		return NULL;
	return t;
// 	struct SkillTable *skill_config = get_config_by_id(fuwen_id, &fuwen_config);
// 	if (!skill_config)
// 	{
// 		return GetNoFuwenSkillStruct(fuwen_id);
// 	}
// 	uint32_t skill_id = skill_config->ID;
// 	skill_struct *t = GetSkill(skill_id);
// 	if (!t || !t->data)
// 		return NULL;
// 	if (t->data->cur_fuwen[m_index - 1] != fuwen_id)
// 		return NULL;
// //	return t->data->lv;
// 	return t;
}

// const static uint32_t avoid_skill[] = {
// 	111100106,   //大刀-躲避-滚地
// 	111100206,  //弓箭-躲避-前滚翻
// 	111100306,   //毛笔-躲避
// 	111100406,   //长枪-躲避
// 	111100506,  //笛子-躲避
// 	111100102,  //大刀-技能1-旋风斩
// 	111100113,  //大刀-技能2-冲锋-第2段
// //	111100103, //大刀-技能2-冲锋
// };

uint32_t MySkill::GetRandSkillId(struct ai_player_data *ai_data)
{
	if (!ai_data)
		return (0);
	
//	if (m_skill.empty())
//		return (0);

//	int size = m_skill.size();
//	int index = random() % m_skill.size();
//	SKILL_CONTAIN::iterator ite = m_skill.begin();
//	for (int i = 0; i < index; ++i)
//		++ite;
//	return ite->first;
//	return 111100101;
		//计算CD的时候有200毫秒误差
	uint64_t now = time_helper::get_cached_time() - 200;

	if (ai_data->normal_skill_id != 0)
	{
		if (now > ai_data->normal_skill_timeout)
		{
			ai_data->normal_skill_id = 0;
		}
		else
		{
			return ai_data->normal_skill_id;
		}
	}
	
		//只有第一个普攻可以被选中, 优先选择非普攻技能
//	bool first_normal_attack = true;
	uint32_t normal_attack_id = 0;
	
	for (SKILL_CONTAIN::iterator ite = m_skill.begin(); ite != m_skill.end(); ++ite)
	{
		if (now < (*ite)->data->cd_time)
			continue;

		if (is_second_skill(*ite))
			continue;

		if ((*ite)->config->SkillType == 1)
		{
//			if (!first_normal_attack)
//				continue;
//			first_normal_attack = false;
			normal_attack_id = (*ite)->data->skill_id;
			continue;
		}
		else if ((*ite)->config->SkillType != 2)
		{
			continue;
		}

		// bool avoid = false;
		// for (size_t i = 0; i < ARRAY_SIZE(avoid_skill); ++i)
		// {
		// 	if ((*ite)->data->skill_id == avoid_skill[i])
		// 	{
		// 		avoid = true;
		// 		break;
		// 	}
		// }
		// if (avoid)
		// 	continue;

//		LOG_DEBUG("%s: jacktang choose skill id = %u cd = %lu now = %lu", __FUNCTION__, (*ite)->data->skill_id, (*ite)->data->cd_time, now);
		return (*ite)->data->skill_id;
	}
	return (normal_attack_id);
}

uint32_t MySkill::GetFirstSkillId()
{
	if (m_skill.empty())
		return (0);
	return (*m_skill.begin())->data->skill_id;
}

int MySkill::GetSkillLevelNum(uint32_t lv)
{
	int num = 0;
	for (SKILL_CONTAIN::iterator iter = m_skill.begin(); iter != m_skill.end(); ++iter)
	{
		skill_struct *pSkill = (*iter);
		if (pSkill->config->IsRune == 1)
		{
			continue;
		}
		if (pSkill->data->lv < lv)
		{
			continue;
		}
		ActiveSkillTable *active_config = get_config_by_id(pSkill->config->SkillAffectId, &active_skill_config);
		if (!active_config || (active_config && active_config->NextSkill == 0))
		{
			num++;
		}
	}
	return num;
}

int MySkill::GetFuwenUnlockNum(void)
{
	int num = 0;
	for (SKILL_CONTAIN::iterator iter = m_skill.begin(); iter != m_skill.end(); ++iter)
	{
		skill_struct *pSkill = (*iter);
		for (int i = 0; i < MAX_FUWEN; ++i)
		{
			uint32_t fuwen_id = pSkill->data->fuwen[i].id;
			if (fuwen_id == 0)
			{
				continue;
			}
			SkillTable *fuwen_config = get_config_by_id(fuwen_id, &skill_config);
			if (!fuwen_config)
			{
				continue;
			}
			if (fuwen_config->OpenLv > pSkill->data->lv)
			{
				continue;
			}
			num++;
		}
	}
	return num;
}

int MySkill::GetFuwenWearNum(void)
{
	int num = 0;
	for (SKILL_CONTAIN::iterator iter = m_skill.begin(); iter != m_skill.end(); ++iter)
	{
		skill_struct *pSkill = *iter;
		for (int i = 0; i < MAX_CUR_FUWEN; ++i)
		{
			if (pSkill->data->cur_fuwen[i] > 0)
			{
				num++;
			}
		}
	}
	return num;
}

int MySkill::GetFuwenLevelNum(uint32_t lv)
{
	int num = 0;
	for (SKILL_CONTAIN::iterator iter = m_skill.begin(); iter != m_skill.end(); ++iter)
	{
		skill_struct *pSkill = *iter;
		for (int i = 0; i < MAX_FUWEN; ++i)
		{
			fuwen_data *pFuwen = &pSkill->data->fuwen[i];
			if (pFuwen->id > 0 && pFuwen->lv >= lv)
			{
				num++;
			}
		}
	}
	return num;
}

void MySkill::SendAllSkill()
{
	SkillList send;
	skill_list__init(&send);

	SKILL_CONTAIN::iterator it = m_skill.begin();
	int i = 0;
	for (; it != m_skill.end(); ++it)
	{
		if ((*it)->config->SkillAcc == 0)
		{
			continue;
		}
		skill_data__init(skillData + i);
		skillData[i].id = (*it)->data->skill_id;
		skillData[i].lv = (*it)->data->lv;
		skillData[i].cur_fuwen = (*it)->data->cur_fuwen[0];
		skillData[i].n_fuwen = (*it)->data->fuwen_num;
		for (int j = 0; j < (*it)->data->fuwen_num; ++j)
		{
			fuwen_data__init(&fuwenData[i][j]);
			fuwenData[i][j].id = (*it)->data->fuwen[j].id;
			fuwenData[i][j].lv = (*it)->data->fuwen[j].lv;
			fuwenData[i][j].is_new = (*it)->data->fuwen[j].isNew;
			fuwenDataPoint[i][j] = &fuwenData[i][j];
		}
		skillData[i].fuwen = fuwenDataPoint[i];
		skillDataPoint[i] = skillData + i;

		skill_data__init(skillData1 + i);
		skillData1[i].id = (*it)->data->skill_id;
		skillData1[i].lv = (*it)->data->lv;
		skillData1[i].cur_fuwen = (*it)->data->cur_fuwen[1];
		skillData1[i].n_fuwen = (*it)->data->fuwen_num;
		for (int j = 0; j < (*it)->data->fuwen_num; ++j)
		{
			fuwen_data__init(&fuwenData1[i][j]);
			fuwenData1[i][j].id = (*it)->data->fuwen[j].id;
			fuwenData1[i][j].lv = (*it)->data->fuwen[j].lv;
			fuwenData1[i][j].is_new = (*it)->data->fuwen[j].isNew;
			fuwenDataPoint1[i][j] = &fuwenData1[i][j];
		}
		skillData1[i].fuwen = fuwenDataPoint1[i];
		skillDataPoint1[i] = skillData1 + i;

		++i;
	}
	send.n_data = i;
	send.n_data_1 = i;
	send.data = skillDataPoint;
	send.data_1 = skillDataPoint1;
	send.num = 1;

	EXTERN_DATA ext_data;
	ext_data.player_id = m_owner->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_SKILL_LIST_NOTIFY, skill_list__pack, send);
}

skill_struct * MySkill::GetSkill(uint32_t id)
{
	for (SKILL_CONTAIN::iterator it = m_skill.begin(); it != m_skill.end(); ++it)
	{
		if ((*it)->data->skill_id == id)
			return (*it);
	}
	return NULL;
}

uint32_t MySkill::GetLevelUpTo(uint32_t id, uint32_t initLv, uint32_t maxLv)
{
	std::map<uint64_t, struct SkillTable *>::iterator it = skill_config.find(id);
	if (it == skill_config.end())
	{
		return 0;
	}
	//skill_struct *pSkill = GetSkill(id);
	//if (pSkill == NULL)
	//{
	//	return 0;
	//}
	uint32_t num = 0;
	uint32_t cost = 0;
	for (uint32_t lv = initLv; lv < maxLv; ++lv)
	{
		std::map<uint64_t, struct SkillLvTable *>::iterator iter = skill_lv_config.find(it->second->SkillLv + lv);
		if (iter == skill_lv_config.end())
			break;
		if (maxLv < iter->second->NeedLv)
		{
			break;
		}
		cost += iter->second->CostCoin;
		//cost += CalcCost(it->second->SkillLv, lv - 1, 1);
		if (cost > m_owner->get_coin())
			return num;
		else
			++num;
	}
	return num;
}

uint32_t MySkill::CalcCost(uint32_t id, uint32_t oldLv, uint32_t num)
{
	uint32_t cost = 0;
	for (uint32_t i = 0; i < num; ++i)
	{
		std::map<uint64_t, struct SkillLvTable *>::iterator iter = skill_lv_config.find(id + oldLv + i);
		if (iter == skill_lv_config.end())
			break;
		cost += iter->second->CostCoin;
	}
	return cost;
}

bool MySkill::is_second_skill(skill_struct *skill)
{
	if (skill->config->pre_skill != 0)
		return true;
	return false;
}

void MySkill::adjust_ai_player_skill()
{
	for (SKILL_CONTAIN::iterator ite = m_skill.begin(); ite != m_skill.end();)
	{
		skill_struct *skill = (*ite);
		if (skill->config->SkillType == 1)  //普攻
		{
			++ite;
		}
		else if (skill->config->SkillType == 2)  //主动技能
		{
			struct ActiveSkillTable *act_config = get_config_by_id(skill->config->SkillAffectId, &active_skill_config);
			if (!act_config)
			{
				ite = m_skill.erase(ite);
				continue;
			}
			if (act_config->CanMove == 1) //旋风斩
			{
				ite = m_skill.erase(ite);
				continue;
			}

				//翻滚类技能
			if (act_config->FlyId != 0 && act_config->CanMove == 2)
			{
				struct SkillMoveTable *move_config = get_config_by_id(act_config->FlyId, &move_skill_config);
				if (!move_config)
				{
					ite = m_skill.erase(ite);
					continue;
				}
				if (move_config->MoveType == 0 || move_config->DmgType == 2)
				{
					ite = m_skill.erase(ite);
					continue;
				}
			}
			++ite;			
		}
		else
		{
			ite = m_skill.erase(ite);
		}
	}

	// for (SKILL_CONTAIN::iterator ite = m_skill.begin(); ite != m_skill.end(); ++ite)
	// {
	// 	LOG_DEBUG("jacktang: skill[%u]", (*ite)->config->ID);
	// }
}

void MySkill::PackAllSkill(_PlayerDBInfo &pb)
{
	int i = 0;
	assert(m_skill.size() <= MAX_MY_SKILL_NUM);
	static SkillOneDbData one[MAX_MY_SKILL_NUM];
	static SkillOneDbData *onePoint[MAX_MY_SKILL_NUM];
	static SkillDbData all;
	skill_db_data__init(&all);
	for (SKILL_CONTAIN::iterator it = m_skill.begin(); it != m_skill.end(); ++it)
	{
		skill_one_db_data__init(one + i);
		one[i].id = (*it)->data->skill_id;
		one[i].lv = (*it)->data->lv;
		one[i].cur_fuwen = (*it)->data->cur_fuwen;
		one[i].n_cur_fuwen = MAX_CUR_FUWEN;
		one[i].n_fuwen = (*it)->data->fuwen_num;
		for (int j = 0; j < (*it)->data->fuwen_num; ++j)
		{
			fuwen_data__init(&fuwenData[i][j]);
			fuwenData[i][j].id = (*it)->data->fuwen[j].id;
			fuwenData[i][j].lv = (*it)->data->fuwen[j].lv;
			fuwenData[i][j].is_new = (*it)->data->fuwen[j].isNew;
			fuwenDataPoint[i][j] = &fuwenData[i][j];
		}
		one[i].fuwen = (FuwenDb**)fuwenDataPoint[i];

		onePoint[i] = one + i;
		++i;
	}
	all.num = this->m_index;
	all.all = onePoint;
	all.n_all = i;
	pb.skill = &all;
}

void MySkill::UnPackAllSkill(_PlayerDBInfo &pb)
{
	if (pb.skill == NULL)
	{
		return;
	}
	for (uint32_t i = 0; i < pb.skill->n_all; ++i)
	{
		skill_struct * pSkillStruct = InsertSkill(pb.skill->all[i]->id);
		if (pSkillStruct != NULL)
		{
			pSkillStruct->data->lv = pb.skill->all[i]->lv;
			pSkillStruct->data->fuwen_num = pb.skill->all[i]->n_fuwen;
			for (int c = 0; c < pSkillStruct->data->fuwen_num; ++c)
			{
				pSkillStruct->data->fuwen[c].id = pb.skill->all[i]->fuwen[c]->id;
				pSkillStruct->data->fuwen[c].lv = pb.skill->all[i]->fuwen[c]->lv;
				pSkillStruct->data->fuwen[c].isNew = pb.skill->all[i]->fuwen[c]->is_new;
			}
			for (uint32_t c = 0; c < pb.skill->all[i]->n_cur_fuwen; ++c)
			{
				pSkillStruct->data->cur_fuwen[c] = pb.skill->all[i]->cur_fuwen[c];
			}
		}
	}
	m_index = pb.skill->num;
}

void MySkill::pack(struct skill_data skill[MAX_MY_SKILL_NUM])
{
	int index = 0;
	for (SKILL_CONTAIN::iterator ite = m_skill.begin(); ite != m_skill.end() && index < MAX_MY_SKILL_NUM; ++ite, ++index)
	{
		skill[index].skill_id = (*ite)->data->skill_id;
		skill[index].lv = (*ite)->data->lv;
		skill[index].fuwen_num = (*ite)->data->fuwen_num;
		for (int i = 0; i < skill[index].fuwen_num; ++i)
		{
			skill[index].fuwen[i].id = (*ite)->data->fuwen[i].id;
			skill[index].fuwen[i].lv = (*ite)->data->fuwen[i].lv;
			skill[index].fuwen[i].isNew = (*ite)->data->fuwen[i].isNew;
		}
		for (int i = 0; i < MAX_CUR_FUWEN; ++i)
		{
			skill[index].cur_fuwen[i] = (*ite)->data->cur_fuwen[i];
		}
	}
	if (index < MAX_MY_SKILL_NUM)
		skill[index].skill_id = 0;
}

void MySkill::insert(skill_struct *skill)
{
	m_skill.push_back(skill);	
}

void MySkill::copy(MySkill *skill)
{
	m_index = skill->m_index;
	for (SKILL_CONTAIN::iterator ite = skill->m_skill.begin(); ite != skill->m_skill.end(); ++ite)
	{
		skill_struct *skill_ = skill_manager::copy_skill(*ite);
		if (!skill_)
			continue;
		m_skill.push_back(skill_);
	}
}
