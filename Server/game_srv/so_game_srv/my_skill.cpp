#include "my_skill.h"

#include "msgid.h"
#include "cast_skill.pb-c.h"
#include "player_db.pb-c.h"
#include "player.h"
#include "game_config.h"
#include "skill_manager.h"
#include "app_data_statis.h"
#include "time_helper.h"


SkillData skillData[MySkill::MAX_SKILL_NUM];
SkillData *skillDataPoint[MySkill::MAX_SKILL_NUM] = { NULL };

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
//			skill_struct * pSkillStruct = skill_manager::create_skill(it->second->ID, m_owner->get_uuid(), 0);
			skill_struct * pSkillStruct = InsertSkill(it->second->ID);
			if (pSkillStruct != NULL)
			{
//				m_skill.insert(std::make_pair(it->first, pSkillStruct));
				
				if (it->second->SkillAcc == 0)
				{
					continue;
				}
				skill_data__init(skillData + i);
				skillData[i].id = it->first;
				skillData[i].lv = pSkillStruct->data->lv;
				skillData[i].cur_fuwen = pSkillStruct->data->cur_fuwen;
				skillData[i].n_fuwen = pSkillStruct->data->fuwen_num;
				skillData[i].fuwen = pSkillStruct->data->fuwen;

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

uint32_t MySkill::GetCurFuwen(uint32_t id)
{
	SKILL_CONTAIN::iterator it = m_skill.find(id);
	if (it == m_skill.end())
	{
		return 0;
	}
	return it->second->data->cur_fuwen;
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
		it->second->data->cur_fuwen = fuwen;
		return 0;
	}
	std::map<uint64_t, struct SkillTable *>::iterator itCon = skill_config.begin();
	if (itCon == skill_config.end())
	{
		return 190500069;
	}
	if (it->second->data->lv < itCon->second->OpenLv)
	{
		return 190500069;
	}
	for (int i = 0; i < it->second->data->fuwen_num; ++i)
	{
		if (it->second->data->fuwen[i] == fuwen)
		{
			it->second->data->cur_fuwen = fuwen;
			return 0;
		}
	}
	return 2;
}

skill_struct *MySkill::GetNoFuwenSkillStruct(uint32_t skill_id)
{
	skill_struct *t = GetSkill(skill_id);
	if (!t || !t->data)
		return NULL;
	if (t->data->cur_fuwen != 0)
		return NULL;
//	return t->data->lv;
	return t;
}

skill_struct *MySkill::GetSkillStructFromFuwen(uint32_t fuwen_id)
{
	struct SkillTable *skill_config = get_config_by_id(fuwen_id, &fuwen_config);
	if (!skill_config)
	{
		return GetNoFuwenSkillStruct(fuwen_id);
	}
	uint32_t skill_id = skill_config->ID;
	skill_struct *t = GetSkill(skill_id);
	if (!t || !t->data)
		return NULL;
	if (t->data->cur_fuwen != fuwen_id)
		return NULL;
//	return t->data->lv;
	return t;
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
		skillData[i].cur_fuwen = it->second->data->cur_fuwen;
		skillData[i].n_fuwen = it->second->data->fuwen_num;
		skillData[i].fuwen = it->second->data->fuwen;

		skillDataPoint[i] = skillData + i;
		++i;
	}
	send.n_data = i;
	send.data = skillDataPoint;

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

uint32_t MySkill::GetLevelUpTo(uint32_t id)
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
	for (uint32_t lv = pSkill->data->lv + 1; lv <= m_owner->get_attr(PLAYER_ATTR_LEVEL); ++lv)
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
		//for (int l = 0; l < 5; ++l)
		//{
		//	if (oldLv + i <= iter->second->IntervalLv[l])
		//	{
		//		cost += iter->second->BasedConsumption[l] + iter->second->GrowthValue[l] * (oldLv + i - 1);
		//		break;
		//	}
		//}
		
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
	for (SKILL_CONTAIN::iterator it = m_skill.begin(); it != m_skill.end(); ++it)
	{
		skill_data__init(skillData + i);
		skillData[i].id = it->first;
		skillData[i].lv = it->second->data->lv;
		skillData[i].cur_fuwen = it->second->data->cur_fuwen;
		skillData[i].n_fuwen = it->second->data->fuwen_num;
		skillData[i].fuwen = it->second->data->fuwen;

		skillDataPoint[i] = skillData + i;
		++i;
	}
	pb.n_skill = i;
	pb.skill = (SkillDbData **)skillDataPoint;
}

void MySkill::UnPackAllSkill(_PlayerDBInfo &pb)
{
	for (uint32_t i = 0; i < pb.n_skill; ++i)
	{
//		skill_struct * pSkillStruct = skill_manager::create_skill(pb.skill[i]->id, m_owner->get_uuid(), 0);
		skill_struct * pSkillStruct = InsertSkill(pb.skill[i]->id);
		if (pSkillStruct != NULL)
		{
//			m_skill.insert(std::make_pair(pb.skill[i]->id, pSkillStruct));

			pSkillStruct->data->cur_fuwen = pb.skill[i]->cur_fuwen;
			pSkillStruct->data->lv = pb.skill[i]->lv;
			pSkillStruct->data->fuwen_num = pb.skill[i]->n_fuwen;
			for (int c = 0; c < pSkillStruct->data->fuwen_num; ++c)
			{
				pSkillStruct->data->fuwen[c] = pb.skill[i]->fuwen[c];
			}
		}
	}
}
