#include "guild_answer.h"
#include <string.h>
#include "time_helper.h"
#include "guild_util.h"
#include "../proto/msgid.h"
#include "../proto/answer.pb-c.h"
#include "../proto/comm_message.pb-c.h"
#include "conn_node_guildsrv.h"
//#include "guild_config.h"

uint32_t GuildAnswer::s_Open = 5; //开启次数
bool GuildAnswer::s_OpenMust; //必开
uint64_t GuildAnswer::s_nextOpen;
uint64_t GuildAnswer::s_nextUpdate;

GuildAnswer::GuildAnswer()
{
	m_state = 0;
	//m_questionNum = 0;
	m_questionIndex = 0;
	m_freeOpen = 1;	
}

void GuildAnswer::Start(GuildInfo *guild)//, uint32_t *arrQuestion, uint32_t num)
{
	CommAnswer resp;
	comm_answer__init(&resp);
	broadcast_guild_message(guild, MSG_ID_FACTION_QUESTION_OPEN_NOTIFY, &resp, (pack_func)comm_answer__pack);

	n_memSize = 0;
	pName[0] = name[0];
	pName[1] = name[1];
	pName[2] = name[2];

	memset(participators, 0, sizeof(participators));

	//uint32_t tmp = num;
	//if (tmp > MAX_GUILD_QUESTION_NUM)
	//{
	//	tmp = MAX_GUILD_QUESTION_NUM;
	//}
	ParameterTable *table = get_config_by_id(161000201, &parameter_config);
	if (table == NULL)
	{
		return;
	}

	//memcpy(m_questionId, arrQuestion, tmp * sizeof(uint32_t));
	//m_questionNum = tmp;
	m_state = GUILD_ANSWER_REST;
	m_cd = time_helper::get_micro_time() / 1000 + table->parameter1[0] * 1000;
	m_questionIndex = 0;
	m_guild = guild;
	//准备
	FactionQuestionRest send;
	faction_question_rest__init(&send);
	send.num = m_questionIndex + 1;
	send.cd = table->parameter1[0];
	broadcast_guild_message(m_guild, MSG_ID_FACTION_QUESTION_REST_NOTIFY, &send, (pack_func)faction_question_rest__pack);
}

void GuildAnswer::OnTimer()
{
	ParameterTable *table = get_config_by_id(161000411, &parameter_config); //161000201
	if (table == NULL)
	{
		return;
	}
	switch (m_state)
	{
	case GUILD_ANSWER_CLOSE:
		break;
	case GUILD_ANSWER_REST:
	{
		m_cd = time_helper::get_micro_time() / 1000 + table->parameter1[1] * 1000;
		m_state = GUILD_ANSWER_UNDER_WAY;
		m_questionId = sg_guild_question[rand() % sg_guild_question.size()];
		//发题
		FactionQuestion send;
		faction_question__init(&send);
		send.qid = m_questionId;// [m_questionIndex];
		send.cd = table->parameter1[1];
		broadcast_guild_message(m_guild, MSG_ID_FACTION_QUESTION_NOTIFY, &send, (pack_func)faction_question__pack);
	}
	break;
	case GUILD_ANSWER_UNDER_WAY:
	{
		m_cd = time_helper::get_micro_time() / 1000 + table->parameter1[2] * 1000;
		m_state = GUILD_ANSWER_AWARD;
		//一题结束 计算前3名
		OneFactionQuestionEnd send;
		one_faction_question_end__init(&send);
		send.name = pName;
		//char tmp[33] = "aaaaaaaaaaaa";
		//pName[0] = tmp;
		if (n_memSize > 3)
		{
			send.n_name = 3;
		}
		else
		{
			send.n_name = n_memSize;
		}
		send.qid = m_questionId;// [m_questionIndex];
		broadcast_guild_message(m_guild, MSG_ID_ONE_FACTION_QUESTION_END_NOTIFY, &send, (pack_func)one_faction_question_end__pack);
		Award();
	}
	break;
	case GUILD_ANSWER_AWARD:
	{
		n_memSize = 0;
		ParameterTable *tableNum = get_config_by_id(161000413, &parameter_config);
		if (tableNum == NULL)
		{
			return;
		}
		FactionQuestionRest send;
		faction_question_rest__init(&send);
		if (m_questionIndex >= tableNum->parameter1[0] - 1)
		{
			//活动完毕
			m_state = GUILD_ANSWER_CLOSE;
			broadcast_guild_message(m_guild, MSG_ID_FACTION_QUESTION_END_NOTIFY, &send, (pack_func)faction_question_rest__pack);
		}
		else
		{
			//下一题
			m_state = GUILD_ANSWER_REST;
			m_cd = time_helper::get_micro_time() / 1000 + table->parameter1[0] * 1000;
			++m_questionIndex;
			
			send.num = m_questionIndex + 1;
			send.cd = table->parameter1[0];
			broadcast_guild_message(m_guild, MSG_ID_FACTION_QUESTION_REST_NOTIFY, &send, (pack_func)faction_question_rest__pack);
		}
		break;
	}
	}
}

void GuildAnswer::Answer(EXTERN_DATA *extern_data, char *answer, char *name)
{
	if (extern_data == NULL)
	{
		return;
	}
	if (m_state != GUILD_ANSWER_UNDER_WAY)
	{
		return;
	}
	QuestionTable *table = get_config_by_id(m_questionId,&questions_config);
	if (table == NULL)
	{
		return; 
	}
	uint32_t i = 0;
	for (; i < n_memSize; ++i)
	{
		if (members[i] == extern_data->player_id)
		{
			return;
		}
	}
	for (int i = 0; i < MAX_GUILD_MEMBER_NUM; ++i)
	{
		if (participators[i] == 0)
		{
			participators[i] = extern_data->player_id;
			fast_send_msg_base(&conn_node_guildsrv::connecter, extern_data, SERVER_PROTO_GUILD_SYNC_PARTICIPATE_ANSWER, 0, 0);
			break;
		}
		else if (participators[i] == extern_data->player_id)
		{
			break;
		}
	}
	if (strcmp(answer, table->GangsAnseer) == 0)
	{
		members[i] = extern_data->player_id;
		++n_memSize;
		if (i < 3)
		{
			strncpy(this->name[i], name, MAX_PLAYER_NAME_LEN);
		}
	}
	
	//FactionQuestionResult send;
	//faction_question_result__init(&send);
	//send.add = false;
	//send.result = false;
	//if (strcmp(answer, table->GangsAnseer) == 0)
	//{
	//	//给奖励
	//	send.result = true;
	//	if (i < 3)
	//	{
	//		strncpy(this->name[i], name, MAX_PLAYER_NAME_LEN);
	//		send.add = true;
	//	}

	//	PROTO_GUILD_DISBAND *req = (PROTO_GUILD_DISBAND*)conn_node_base::get_send_buf(SERVER_PROTO_GUILD_ANSWER_AWARD, 0);
	//	uint8_t *pData = (uint8_t *)req->data;
	//	
	//	req->player_num = faction_question_result__pack(&send, pData);
	//	req->head.len = ENDION_FUNC_4(sizeof(PROTO_GUILD_DISBAND) + req->player_num);

	//	conn_node_base::add_extern_data(&req->head, extern_data);
	//	if (conn_node_guildsrv::connecter.send_one_msg(&req->head, 1) != (int)ENDION_FUNC_4(req->head.len))
	//	{
	//		LOG_ERR("[%s:%d] send to gamesrv failed err[%d]", __FUNCTION__, __LINE__, errno);
	//	}
	//}
	//fast_send_msg(&conn_node_guildsrv::connecter, extern_data, MSG_ID_FACTION_QUESTION_ANSWER, faction_question_result__pack, send);
}

void GuildAnswer::Award()
{
	PROTO_GUILD_DISBAND *req = (PROTO_GUILD_DISBAND*)conn_node_base::get_send_buf(SERVER_PROTO_GUILD_ANSWER_AWARD, 0);
	req->player_num = n_memSize;
	memcpy(req->data, members, n_memSize);
	req->head.len = ENDION_FUNC_4(sizeof(PROTO_GUILD_DISBAND)+req->player_num * sizeof(uint64_t));
	EXTERN_DATA extern_data;
	conn_node_base::add_extern_data(&req->head, &extern_data);
	if (conn_node_guildsrv::connecter.send_one_msg(&req->head, 1) != (int)ENDION_FUNC_4(req->head.len))
	{
		LOG_ERR("[%s:%d] send to gamesrv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
}

void GuildAnswer::OnPlayerLogin(GuildPlayer * player, EXTERN_DATA *extern_data)
{
	if (player == NULL)
	{
		return;
	}

	switch (m_state)
	{

	case GUILD_ANSWER_REST:
	{
		//准备
		FactionQuestionRest send;
		faction_question_rest__init(&send);
		send.num = m_questionIndex + 1;
		if (time_helper::get_micro_time() / 1000 < m_cd)
		{
			send.cd = m_cd - time_helper::get_micro_time() / 1000;
			send.cd /= 1000;
		}
		else
		{
			send.cd = 0;
		}
		
		fast_send_msg(&conn_node_guildsrv::connecter, extern_data, MSG_ID_FACTION_QUESTION_REST_NOTIFY, faction_question_rest__pack, send);
	}
	break;
	case GUILD_ANSWER_UNDER_WAY:
	{
		//发题
		FactionQuestion send;
		faction_question__init(&send);
		send.qid = m_questionId;// [m_questionIndex];
		if (time_helper::get_micro_time() / 1000 < m_cd)
		{
			send.cd = m_cd - time_helper::get_micro_time() / 1000;
			send.cd /= 1000;
		}
		else
		{
			send.cd = 0;
		}
		fast_send_msg(&conn_node_guildsrv::connecter, extern_data, MSG_ID_FACTION_QUESTION_NOTIFY, faction_question__pack, send);
	}
	break;
	
	}
}

void GuildAnswer::CheckOpenAnswer()
{
	static bool bDone = false;
	uint32_t cd = 0;
	if (check_active_open(330400033, cd))
	{
		if (!bDone)
		{
			open_all_guild_answer();
			bDone = true;
		}
	}
	else
	{
		if (bDone)
		{
			bDone = false;
		}
	}
}
