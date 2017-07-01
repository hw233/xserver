#ifndef _GUILD_ANSWER_H__
#define _GUILD_ANSWER_H__

#include <stdint.h>
#include "comm_define.h"
#include "server_proto.h"

enum GuildAnswerState
{
	GUILD_ANSWER_CLOSE = 0,
	GUILD_ANSWER_UNDER_WAY = 1, //答题
	GUILD_ANSWER_REST = 2, //准备放题
	GUILD_ANSWER_AWARD = 3, //公布前3名
};

class GuildPlayer;
class GuildInfo;

class GuildAnswer
{
public:
	static const int MAX_GUILD_QUESTION_NUM = 20;
	static const int MAX_SEND_GUILD_QUESTION = 5;
	void Start(GuildInfo *guild, uint32_t *arrQuestion, uint32_t num);
	void Answer(EXTERN_DATA *extern_data, char *answer, char *name);

	GuildAnswer();

	void OnPlayerLogin(GuildPlayer * player, EXTERN_DATA *extern_data);

	void OnTimer();

	static void CheckOpenAnswer();

	uint64_t m_cd;
	
	static uint32_t s_Open; //开启次数
	static bool s_OpenMust; //必开
	static uint64_t s_nextOpen;
	static uint64_t s_nextUpdate;

protected:
private:
	int m_state;
	uint32_t m_questionId[MAX_GUILD_QUESTION_NUM];
	uint32_t m_questionNum;
	uint32_t m_questionIndex;
	GuildInfo *m_guild;
	uint32_t m_freeOpen; //免费开启次数
	uint64_t members[MAX_GUILD_MEMBER_NUM]; //帮会成员
	uint32_t n_memSize;
	char name[3][MAX_PLAYER_NAME_LEN + 1];
	char *pName[3];

	
};

#endif //_GUILD_ANSWER_H__
