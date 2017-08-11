#include "my_skill.h"

#include "msgid.h"
#include "cast_skill.pb-c.h"
#include "player_db.pb-c.h"
#include "player.h"
#include "game_config.h"
#include "skill_manager.h"
#include "app_data_statis.h"
#include "time_helper.h"

FuwenData fuwenData[MySkill::MAX_SKILL_NUM][MAX_FUWEN];
FuwenData *fuwenDataPoint[MySkill::MAX_SKILL_NUM][MAX_FUWEN];
FuwenData fuwenData1[MySkill::MAX_SKILL_NUM][MAX_FUWEN];
FuwenData *fuwenDataPoint1[MySkill::MAX_SKILL_NUM][MAX_FUWEN];
SkillData skillData[MySkill::MAX_SKILL_NUM];
SkillData *skillDataPoint[MySkill::MAX_SKILL_NUM] = { NULL };
SkillData skillData1[MySkill::MAX_SKILL_NUM];
SkillData *skillDataPoint1[MySkill::MAX_SKILL_NUM] = { NULL };

MySkill::MySkill(player_struct *player)
{
	m_owner = player;
}

void MySkill::clear()
{
	for (SKILL_CONTAIN::iterator it = m_skill.begin(); it != m_skill.end(); ++it)
	{
		skill_manager::delete_skill(it->second);
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
		if (it->second->Career != m_owner->get_attr(PLAYER_ATTR_JOB))
		{
			continue;
		}
		InsertSkill(it->second->ID);
	}
}

int MySkill::Learn(uint32_t id, uint32_t num)
{
	SKILL_CONTAIN::iterator it = m_skill.find(id);
	if (it == m_skill.end())
	{
		return 0;
	}
	if (it->second->data->lv + num > m_owner->get_attr(PLAYER_ATTR_LEVEL))
	{
		return 190500068;
	}
	uint32_t cost = CalcCost(it->second->config->SkillLv, it->second->data->lv, num);
	if (m_owner->sub_coin(cost, MAGIC_TYPE_SKILL) != 0)
	{
		return 190500063;
	}
	IteratorLevelUp(id, num);
	m_owner->add_task_progress(TCT_SKILL_LEVEL_UP, get_actor_skill_index(m_owner->get_job(), id), num);
	return 0;
}
void MySkill::IteratorLevelUp(uint32_t id, uint32_t num)
{
	SKILL_CONTAIN::iterator it = m_skill.find(id);
	if (it == m_skill.end())
	{
		return;
	}
	it->second->data->lv += num;

	std::map<uint64_t, struct ActiveSkillTable *>::iterator itCon  = active_skill_config.find(it->second->config->SkillAffectId);
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
		m_skill.insert(std::make_pair(id, pSkillStruct));
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
		SKILL_CONTAIN::iterator itS = m_skill.find(it->first);
		if (itS != m_skill.end())
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
	SKILL_CONTAIN::iterator it = m_skill.find(id);
	if (it == m_skill.end())
	{
		return NULL;
	}
	if (it->second->data->cur_fuwen[m_index - 1] == 0)
	{
		return NULL;
	}
	for (int i = 0; i < it->second->data->fuwen_num; ++i)
	{
		if (it->second->data->fuwen[i].id == it->second->data->cur_fuwen[m_index - 1])
		{
			return &it->second->data->fuwen[i];
		}
	}
	return NULL;
}

int MySkill::SetFuwen(uint32_t id, uint32_t fuwen)
{
	SKILL_CONTAIN::iterator it = m_skill.find(id);
	if (it == m_skill.end())
	{
		return 1;
	}

	if (fuwen == 0)
	{
		it->second->data->cur_fuwen[m_index - 1] = fuwen;
		return 0;
	}

	for (int i = 0; i < it->second->data->fuwen_num; ++i)
	{
		if (it->second->data->fuwen[i].id == fuwen)
		{
			std::map<uint64_t, struct SkillTable *>::iterator itCon = skill_config.find(fuwen);
			if (itCon == skill_config.end())
			{
				return 190500069;
			}
			if (it->second->data->lv < itCon->second->OpenLv)
			{
				return 190500069;
			}
			it->second->data->cur_fuwen[m_index - 1] = fuwen;
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

uint32_t MySkill::GetRandSkillId()
{
//	if (m_skill.empty())
//		return (0);

//	int size = m_skill.size();
//	int index = random() % m_skill.size();
//	SKILL_CONTAIN::iterator ite = m_skill.begin();
//	for (int i = 0; i < index; ++i)
//		++ite;
//	return ite->first;
	uint64_t now = time_helper::get_cached_time();
	
	for (SKILL_CONTAIN::iterator ite = m_skill.begin(); ite != m_skill.end(); ++ite)
	{
		if (now < ite->second->data->cd_time)
			continue;
		return ite->first;
	}
	return (0);
}

uint32_t MySkill::GetFirstSkillId()
{
	if (m_skill.empty())
		return (0);
	return m_skill.begin()->second->data->skill_id;
}

void MySkill::SendAllSkill()
{
	SkillList send;
	skill_list__init(&send);

	SKILL_CONTAIN::iterator it = m_skill.begin();
	int i = 0;
	for (; it != m_skill.end(); ++it)
	{
		if (it->second->config->SkillAcc == 0)
		{
			continue;
		}
		skill_data__init(skillData + i);
		skillData[i].id = it->first;
		skillData[i].lv = it->second->data->lv;
		skillData[i].cur_fuwen = it->second->data->cur_fuwen[0];
		skillData[i].n_fuwen = it->second->data->fuwen_num;
		for (int j = 0; j < it->second->data->fuwen_num; ++j)
		{
			fuwen_data__init(&fuwenData[i][j]);
			fuwenData[i][j].id = it->second->data->fuwen[j].id;
			fuwenData[i][j].lv = it->second->data->fuwen[j].lv;
			fuwenData[i][j].is_new = it->second->data->fuwen[j].isNew;
			fuwenDataPoint[i][j] = &fuwenData[i][j];
		}
		skillData[i].fuwen = fuwenDataPoint[i];
		skillDataPoint[i] = skillData + i;

		skill_data__init(skillData1 + i);
		skillData1[i].id = it->first;
		skillData1[i].lv = it->second->data->lv;
		skillData1[i].cur_fuwen = it->second->data->cur_fuwen[1];
		skillData1[i].n_fuwen = it->second->data->fuwen_num;
		for (int j = 0; j < it->second->data->fuwen_num; ++j)
		{
			fuwen_data__init(&fuwenData1[i][j]);
			fuwenData1[i][j].id = it->second->data->fuwen[j].id;
			fuwenData1[i][j].lv = it->second->data->fuwen[j].lv;
			fuwenData1[i][j].is_new = it->second->data->fuwen[j].isNew;
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
	SKILL_CONTAIN::iterator it = m_skill.find(id);
	if (it == m_skill.end())
	{
		return NULL;
	}
	return it->second;
}

uint32_t MySkill::GetLevelUpTo(uint32_t id, uint32_t initLv, uint32_t maxLv)
{
	std::map<uint64_t, struct SkillTable *>::iterator it = skill_config.find(id);
	if (it == skill_config.end())
	{
		return 0;
	}
	skill_struct *pSkill = GetSkill(id);
	if (pSkill == NULL)
	{
		return 0;
	}
	uint32_t num = 0;
	uint32_t cost = 0;
	for (uint32_t lv = initLv + 1; lv <= maxLv; ++lv)
	{
		cost += CalcCost(it->second->SkillLv, lv - 1, 1);
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
		std::map<uint64_t, struct SkillLvTable *>::iterator iter = skill_lv_config.find(id + oldLv + i - 1);
		if (iter == skill_lv_config.end())
			break;
		cost += iter->second->CostCoin;
	}
	return cost;
}

void MySkill::PackAllSkill(_PlayerDBInfo &pb)
{
	int i = 0;
	assert(m_skill.size() <= MAX_SKILL_NUM);
	static SkillOneDbData one[MAX_SKILL_NUM];
	static SkillOneDbData *onePoint[MAX_SKILL_NUM];
	static SkillDbData all;
	skill_db_data__init(&all);
	for (SKILL_CONTAIN::iterator it = m_skill.begin(); it != m_skill.end(); ++it)
	{
		skill_one_db_data__init(one + i);
		one[i].id = it->first;
		one[i].lv = it->second->data->lv;
		one[i].cur_fuwen = it->second->data->cur_fuwen;
		one[i].n_cur_fuwen = MAX_CUR_FUWEN;
		one[i].n_fuwen = it->second->data->fuwen_num;
		for (int j = 0; j < it->second->data->fuwen_num; ++j)
		{
			fuwen_data__init(&fuwenData[i][j]);
			fuwenData[i][j].id = it->second->data->fuwen[j].id;
			fuwenData[i][j].lv = it->second->data->fuwen[j].lv;
			fuwenData[i][j].is_new = it->second->data->fuwen[j].isNew;
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
