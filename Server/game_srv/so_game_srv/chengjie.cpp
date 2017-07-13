#include "chengjie.h"

#include "player_manager.h"
#include "scene_manager.h"
#include "../proto/hotel.pb-c.h"
#include "../proto/player_db.pb-c.h"
#include "../proto/chat.pb-c.h"
#include "game_event.h"
#include "msgid.h"
#include "time_helper.h"
#include "app_data_statis.h"
#include "game_config.h"
#include "conn_node_dbsrv.h"
#include "buff_manager.h"
#include "monster.h"
//#include "send_mail.h"
#include <algorithm>

extern int send_mail(conn_node_base *connecter, uint64_t player_id, uint32_t type,
	char *title, char *sender_name, char *content, std::vector<char *> *args,
	std::map<uint32_t, uint32_t> *attachs, uint32_t statis_id);

//ChengJieTaskManage * ChengJieTaskManage::s_ins = NULL;

// ChengJieTaskManage * ChengJieTaskManage::GetInstance()
// {
// 	if (s_ins == NULL)
// 	{
// 		s_ins = new ChengJieTaskManage;
// 	}
// 	return s_ins;
// }

void ChengJieTaskManage::AddTask(STChengJie &task)
{
	if (ChengJieTaskManage_m_contain.insert(std::make_pair(task.id, task)).second)
	{
		ChengJieTaskManage_m_containVt.push_back(task.id);
		ChengJieTaskManage_m_target.insert(std::make_pair(task.pid, 0));
		LOG_INFO("[%s:%d] add guoyu task[%u]", __FUNCTION__, __LINE__, task.id);
	}
}
void ChengJieTaskManage::AddTask(STChengJie &task, uint64_t accepter)
{
	if (ChengJieTaskManage_m_contain.insert(std::make_pair(task.id, task)).second)
	{
		ChengJieTaskManage_m_containVt.push_back(task.id);
		if (!task.complete)
		{
			ChengJieTaskManage_m_target.insert(std::make_pair(task.pid, accepter));
		}
		LOG_INFO("[%s:%d] add guoyu task[%u]", __FUNCTION__, __LINE__, task.id);

		if (GetRoleLevel(task.pid) == 0)
		{
			PROTO_FIND_PLAYER_REQ *todb = (PROTO_FIND_PLAYER_REQ *)&conn_node_base::global_send_buf[0];
			sprintf(todb->name, "%lu", task.pid);
			todb->head.msg_id = ENDION_FUNC_2(SERVER_PROTO_FIND_PLAYER_REQUEST);
			todb->head.len = ENDION_FUNC_4(sizeof(PROTO_FIND_PLAYER_REQ));
			EXTERN_DATA extern_data;
			extern_data.player_id = 0;
			conn_node_base::add_extern_data(&todb->head, &extern_data);
			if (conn_node_dbsrv::connecter.send_one_msg(&todb->head, 1) != (int)ENDION_FUNC_4(todb->head.len)) {
				LOG_ERR("%s %d: send to dbsrv err[%d]", __FUNCTION__, __LINE__, errno);
			}
		}
	}
}

void ChengJieTaskManage::DelTask(uint32_t taskId)
{
	CHENGJIE_CONTAIN::iterator it = ChengJieTaskManage_m_contain.find(taskId);
	if (it == ChengJieTaskManage_m_contain.end())
	{
		return;
	}
	ChengJieTaskManage_m_target.erase((it->second.pid));
	for (CHENGJIE_VECTOR::iterator itv = ChengJieTaskManage_m_containVt.begin(); itv != ChengJieTaskManage_m_containVt.end(); ++itv)
	{
		if (*itv == taskId)
		{
			ChengJieTaskManage_m_containVt.erase(itv);
			break;
		}
	}
	ChengJieTaskManage_m_contain.erase(it);
	LOG_INFO("[%s:%d] del guoyu task[%u]", __FUNCTION__, __LINE__, taskId);
	DelTaskDb(taskId);
}

void ChengJieTaskManage::DelTaskDb(uint32_t taskId)
{
	PROTO_CHENGJIE_ID *todb = (PROTO_CHENGJIE_ID *)&conn_node_base::global_send_buf[0];
	todb->head.msg_id = ENDION_FUNC_2(SERVER_PROTO_DEL_CHENGJIE_REQUEST);
	todb->head.len = ENDION_FUNC_4(sizeof(PROTO_CHENGJIE_ID));
	todb->taskid = taskId;
	EXTERN_DATA extern_data;
	conn_node_base::add_extern_data(&todb->head, &extern_data);
	if (conn_node_dbsrv::connecter.send_one_msg(&todb->head, 1) != (int)ENDION_FUNC_4(todb->head.len)) {
		LOG_ERR("%s %d: send to dbsrv err[%d]", __FUNCTION__, __LINE__, errno);
	}
}

void ChengJieTaskManage::LoadAllTask()
{
	PROTO_CHENGJIE_ID *todb = (PROTO_CHENGJIE_ID *)&conn_node_base::global_send_buf[0];
	todb->head.msg_id = ENDION_FUNC_2(SERVER_PROTO_LOAD_CHENGJIE_REQUEST);
	todb->head.len = ENDION_FUNC_4(sizeof(PROTO_CHENGJIE_ID));
	EXTERN_DATA extern_data;
	conn_node_base::add_extern_data(&todb->head, &extern_data);
	if (conn_node_dbsrv::connecter.send_one_msg(&todb->head, 1) != (int)ENDION_FUNC_4(todb->head.len)) {
		LOG_ERR("%s %d: send to dbsrv err[%d]", __FUNCTION__, __LINE__, errno);
	}
}

void ChengJieTaskManage::TaskFail(player_struct *player, player_struct *target)
{
	if (player == NULL || target == NULL)
	{
		return;
	}

	SetRoleTarget(player->data->chengjie.target, 0);
	STChengJie *pTask = FindTask(player->data->chengjie.cur_task);
	if (pTask != NULL)
	{
		ParameterTable * config = get_config_by_id(161000105, &parameter_config);
		if (config != NULL)
		{
			pTask->acceptCd = time_helper::get_cached_time() / 1000 + config->parameter1[0];
		}
		++pTask->fail;
		pTask->taskTime = 0;
		UpdateTaskDb(*pTask);
	}

	EXTERN_DATA ext_data;
	ext_data.player_id = player->get_uuid();
	ChengjieTaskSucc nty;
	chengjie_task_succ__init(&nty);
	nty.taskid = player->data->chengjie.cur_task;
	nty.succ = false;
	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_CHENGJIE_TASK_COMPLETE_NOTIFY, chengjie_task_succ__pack, nty);

	player->data->chengjie.cur_task = 0;
	player->data->chengjie.target = 0;

	ChengjieKiller send;
	chengjie_killer__init(&send);
	send.playerid = 0;
	ext_data.player_id = target->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_CHENGJIE_KILLER_NOTIFY, chengjie_killer__pack, send);
}

void ChengJieTaskManage::TaskExpire(STChengJie &task)
{
	CHENGJIE_TARGET::iterator it = ChengJieTaskManage_m_target.find(task.pid);
	if (it == ChengJieTaskManage_m_target.end())
	{
		return;
		
	}
	++task.fail;
	player_struct *player = player_manager::get_player_by_id(it->second);
	if (player != NULL)
	{
		EXTERN_DATA ext_data;
		ext_data.player_id = player->get_uuid();
		ChengjieTaskSucc nty;
		chengjie_task_succ__init(&nty);
		nty.taskid = player->data->chengjie.cur_task;
		nty.succ = false;
		fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_CHENGJIE_TASK_COMPLETE_NOTIFY, chengjie_task_succ__pack, nty);

		player->data->chengjie.cur_task = 0;
		player->data->chengjie.target = 0;
		player->clear_watched_list();

		ChengjieKiller send;
		chengjie_killer__init(&send);
		send.playerid = 0;
		ext_data.player_id = task.pid;
		fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_CHENGJIE_KILLER_NOTIFY, chengjie_killer__pack, send);
	}
	player_struct *aventer = player_manager::get_player_by_id(task.investor);
	if (aventer != NULL)
	{
		ClientGetTaskList(aventer, 3);
	}
	
	ParameterTable * config = get_config_by_id(161000105, &parameter_config);
	if (config != NULL)
	{
		task.acceptCd = time_helper::get_cached_time() / 1000 + config->parameter1[0];
	}
	SetRoleTarget(task.pid, 0);
	task.taskTime = 0;
	UpdateTaskDb(task);
}

void send_system_msg(char * str, player_struct *player)
{
	Chat send;
	chat__init(&send);
	send.contain = str;
	send.channel = CHANNEL__system;
	send.sendname = NULL;
	send.sendplayerid = 0;
	send.sendplayerlv = 0;
	send.sendplayerjob = 0;
	if (player == NULL)
	{
		conn_node_gamesrv::send_to_all_player(MSG_ID_CHAT_NOTIFY, (void *)&send, (pack_func)chat__pack);
	}
	else
	{
		EXTERN_DATA extern_data;
		extern_data.player_id = player->get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_CHAT_NOTIFY, chat__pack, send);
	}
}

void ChengJieTaskManage::CommpleteTask(player_struct *player, player_struct *target, STChengJie &task)
{
	if (player == NULL || target == NULL)
	{
		return;
	}

	EXTERN_DATA ext_data;
	ext_data.player_id = player->get_uuid();

	ChengjieTaskComplete toFriend;
	chengjie_task_complete__init(&toFriend);
	toFriend.investor = task.investor;
	toFriend.target = task.pid;
	toFriend.acceptor = player->get_uuid();
	toFriend.anonymous = task.anonymous;
	toFriend.declaration = task.declaration;
	toFriend.step = task.step;
	toFriend.scene = player->scene->res_config->SceneName;
	conn_node_gamesrv::connecter.send_to_friend(&ext_data, SERVER_PROTO_CHENGJIE_TASK_COMPLETE_REQUEST, &toFriend, (pack_func)chengjie_task_complete__pack);

	ChengJieTaskManage_m_target.erase(player->data->chengjie.target);

	ChengjieTaskSucc nty;
	chengjie_task_succ__init(&nty);
	nty.taskid = player->data->chengjie.cur_task;
	nty.succ = true;
	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_CHENGJIE_TASK_COMPLETE_NOTIFY, chengjie_task_succ__pack, nty);

	//SetRoleTarget(player->data->chengjie.target, 0);
	player->data->chengjie.target = 0;

	ParameterTable * config = get_config_by_id(161000089, &parameter_config);
	if (config != NULL)
	{
		target->data->chengjie.rest = time_helper::get_cached_time() / 1000 + config->parameter1[0];
		target->cache_to_dbserver(); //未实现按名字查找 临时存储方便在数据库拿到实时信息
	}
	UpdateTaskDb(task);

	ChengjieKiller send;
	chengjie_killer__init(&send);
	send.playerid = 0;
	ext_data.player_id = target->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_CHENGJIE_KILLER_NOTIFY, chengjie_killer__pack, send);

	target->chengjie_kill = task.step;
}

void ChengJieTaskManage::UpdateTaskDb(STChengJie &task)
{
	PROTO_ADD_CHENGJIE_ANS *todb = (PROTO_ADD_CHENGJIE_ANS *)&conn_node_base::global_send_buf[0];
	todb->head.msg_id = ENDION_FUNC_2(SERVER_PROTO_UPDATE_CHENGJIE_REQUEST);
	todb->head.len = ENDION_FUNC_4(sizeof(PROTO_ADD_CHENGJIE_ANS));
	todb->taskid = task.id;

	ChengjieTaskDb send;
	chengjie_task_db__init(&send);
	send.playerid = task.pid;
	send.fail = task.fail;
	send.shuangjin = task.shuangjin;
	send.exp = task.exp;
	send.courage = task.courage;
	send.cd = task.timeOut;
	send.complete = task.complete;
	send.complete_cd = task.taskTime;
	send.accept_cd = task.acceptCd;
	send.investor = task.investor;
	send.taskid = task.id;
	send.step = task.step;
	CHENGJIE_TARGET::iterator it = ChengJieTaskManage_m_target.find(task.pid);
	if (it != ChengJieTaskManage_m_target.end())
	{
		send.accepter = it->second;
	}
	
	todb->data_size = chengjie_task_db__pack(&send, todb->data);
	todb->head.len += todb->data_size;
	EXTERN_DATA extern_data;
	conn_node_base::add_extern_data(&todb->head, &extern_data);
	if (conn_node_dbsrv::connecter.send_one_msg(&todb->head, 1) != (int)ENDION_FUNC_4(todb->head.len)) {
		LOG_ERR("%s %d: send to dbsrv err[%d]", __FUNCTION__, __LINE__, errno);
	}
}

void ChengJieTaskManage::AddTaskDb(STChengJie &task, EXTERN_DATA *extern_data)
{
	PROTO_ADD_CHENGJIE *todb = (PROTO_ADD_CHENGJIE *)&conn_node_base::global_send_buf[0];
	todb->head.msg_id = ENDION_FUNC_2(SERVER_PROTO_ADD_CHENGJIE_REQUEST);
	todb->head.len = ENDION_FUNC_4(sizeof(PROTO_ADD_CHENGJIE));

	ChengjieTaskDb send;
	chengjie_task_db__init(&send);
	send.playerid = task.pid;
	send.fail = task.fail;
	send.shuangjin = task.shuangjin ;
	send.exp = task.exp;
	send.courage = task.courage;
	send.cd = task.timeOut;
	send.complete = task.complete;
	send.complete_cd = task.taskTime;
	send.accept_cd = task.acceptCd;
	send.investor = task.investor;
	send.anonymous = task.anonymous;
	send.declaration = task.declaration;
	send.step = task.step;
	todb->data_size = chengjie_task_db__pack(&send, todb->data);
	todb->head.len += todb->data_size;
	conn_node_base::add_extern_data(&todb->head, extern_data);
	if (conn_node_dbsrv::connecter.send_one_msg(&todb->head, 1) != (int)ENDION_FUNC_4(todb->head.len)) {
		LOG_ERR("%s %d: send to dbsrv err[%d]", __FUNCTION__, __LINE__, errno);
	}
	ChengJieTaskManage_m_target.insert(std::make_pair(task.pid, 0));
}

void ChengJieTaskManage::ClientGetTaskList(player_struct *player, int type)
{
	if (player == NULL)
	{
		return;
	}

	ChengJieTaskManage::SortList();

	static const int MAX_SEND_CHENGJIE_NUM = 30;
	static const int MAX_SEND_CHENGJIE_NUM_MY = 10;
	CHENGJIE_VECTOR choseArr;
	ParameterTable *param_config = get_config_by_id(161000100, &parameter_config);
	int i = 0;
	for (CHENGJIE_VECTOR::iterator it = ChengJieTaskManage_m_containVt.begin(); it != ChengJieTaskManage_m_containVt.end() && i < MAX_SEND_CHENGJIE_NUM; ++it)
	{
		CHENGJIE_CONTAIN::iterator itMap = ChengJieTaskManage_m_contain.find(*it);
		if (itMap == ChengJieTaskManage_m_contain.end())
		{
			continue;
		}
		if (itMap->first == player->data->chengjie.cur_task)
		{
			continue;
		}

		if (itMap->second.taskTime != 0)
		{
			continue;
		}
		if (itMap->second.complete)
		{
			continue;
		}
		//if (itMap->second.pid == player->get_uuid())
		//{
		//	continue;
		//}
		if (itMap->second.investor == player->get_uuid())
		{
			continue;
		}
		if (param_config != NULL)
		{
			int lv = GetRoleLevel(itMap->second.pid);
			int min = player->get_attr(PLAYER_ATTR_LEVEL);
			min += param_config->parameter1[0];
			if (lv < min  || (uint32_t)lv > player->get_attr(PLAYER_ATTR_LEVEL) + param_config->parameter1[1])
			{
				continue;
			}
		}
		choseArr.push_back(*it);
		++i;
	}

	ChengjieTask arrTask[MAX_SEND_CHENGJIE_NUM + 1];
	ChengjieTask *arrTaskPoint[MAX_SEND_CHENGJIE_NUM + 1];
	ChengjieTask arrTask1[MAX_SEND_CHENGJIE_NUM_MY + 1];
	ChengjieTask *arrTaskPoint1[MAX_SEND_CHENGJIE_NUM_MY + 1];
	ChengjieList send;
	chengjie_list__init(&send);
	send.type = type;
	i = 0;
	int toFr = 0;
	for (CHENGJIE_VECTOR::iterator it = choseArr.begin(); it != choseArr.end() && i < MAX_SEND_CHENGJIE_NUM; ++it, ++i)
	{
		CHENGJIE_CONTAIN::iterator itMap = ChengJieTaskManage_m_contain.find(*it);
		if (itMap == ChengJieTaskManage_m_contain.end())
		{
			continue;
		}
		if (itMap->second.complete)
		{
			continue;
		}
		chengjie_task__init(arrTask + i);
		arrTaskPoint[i] = arrTask + i;

		if (PackTask(itMap->second, arrTask[i]))
		{
			++toFr;
		}
	}

	choseArr.clear();
	if (player->data->chengjie.cur_task != 0)
	{
		choseArr.push_back(player->data->chengjie.cur_task);
	}
	for (CHENGJIE_VECTOR::iterator it = ChengJieTaskManage_m_containVt.begin(); it != ChengJieTaskManage_m_containVt.end(); ++it)
	{
		CHENGJIE_CONTAIN::iterator itMap = ChengJieTaskManage_m_contain.find(*it);
		if (itMap == ChengJieTaskManage_m_contain.end())
		{
			continue;
		}
		if (itMap->second.complete)
		{
			continue;
		}
		if (itMap->second.investor == player->get_uuid())
		{
			choseArr.push_back(*it);
		}
	}
	int j = 0;
	for (CHENGJIE_VECTOR::iterator it = choseArr.begin(); it != choseArr.end() && i < MAX_SEND_CHENGJIE_NUM; ++it, ++j)
	{
		CHENGJIE_CONTAIN::iterator itMap = ChengJieTaskManage_m_contain.find(*it);
		if (itMap == ChengJieTaskManage_m_contain.end())
		{
			continue;
		}
		chengjie_task__init(arrTask1 + j);
		arrTaskPoint1[j] = arrTask1 + j;

		if (PackTask(itMap->second, arrTask1[j]))
		{
			++toFr;
		}
	}
	send.n_task = i;
	send.task = arrTaskPoint;
	send.n_task_myself = j;
	send.task_myself = arrTaskPoint1;
	EXTERN_DATA ext_data;
	ext_data.player_id = player->get_uuid();
	if (toFr > 0)
	{
		conn_node_gamesrv::connecter.send_to_friend(&ext_data, MSG_ID_REFRESH_CHENGJIE_LIST_ANSWER, &send, (pack_func)chengjie_list__pack);
	}
	else
	{
		fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_REFRESH_CHENGJIE_LIST_ANSWER, chengjie_list__pack, send);
	}	
}

bool ChengJieTaskManage::PackTask(STChengJie &task, _ChengjieTask &send)
{
	chengjie_task__init(&send);

	bool toFr = false;
	char str[33] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"; //todo 定义为全局变量 组队也有
	player_struct *target = player_manager::get_player_by_id(task.pid);
	if (target == NULL)
	{
		send.online = false;
		send.name = str;
		toFr = true;
	}
	else
	{
		send.icon = target->get_attr(PLAYER_ATTR_HEAD);
		send.name = target->get_name();
		send.lv = target->get_attr(PLAYER_ATTR_LEVEL);
		send.job = target->get_attr(PLAYER_ATTR_JOB);
		send.online = true;
		send.fight = target->get_attr(PLAYER_ATTR_FIGHTING_CAPACITY);
		send.zhenying = target->get_attr(PLAYER_ATTR_ZHENYING);
	}
	send.playerid = task.pid;
	send.fail = task.fail;
	send.investor = task.investor;

	uint64_t add = 0;
	ParameterTable * config = get_config_by_id(161000087, &parameter_config);
	if (config != NULL)
	{
		int rate = send.fail;
		if (rate > config->parameter1[0])
		{
			rate = config->parameter1[0];
		}
		if (rate > 0)
		{
			add = task.shuangjin * config->parameter1[1] * rate / 10000.0;
		}
	}
	send.shuangjin = task.shuangjin + add;
	send.exp = task.exp;
	send.courage = task.courage;
	send.taskid = task.id;
	if (time_helper::get_cached_time() / 1000 < task.timeOut)
	{
		send.cd = task.timeOut - time_helper::get_cached_time() / 1000;
	}

	send.complete = task.complete;
	if (!task.complete && time_helper::get_cached_time() / 1000 < task.taskTime)
	{
		send.complete_cd = task.taskTime - time_helper::get_cached_time() / 1000;
	}
	else
	{
		send.complete_cd = 0;
	}
	if (time_helper::get_cached_time() / 1000 < task.acceptCd)
	{
		send.accept_cd = task.acceptCd - time_helper::get_cached_time() / 1000;
	}
	else
	{
		send.accept_cd = 0;
	}

	return toFr;
}

void ChengJieTaskManage::pack_yaoshi_to_dbinfo(player_struct *player, _PlayerDBInfo &db_info)
{
	static YaoshiDb send;
	yaoshi_db__init(&send);
	send.cur_major = player->data->cur_yaoshi;
	static GuoyuTypeDb guoYu;
	guoyu_type_db__init(&guoYu);
	send.guoyu = &guoYu;
	send.change_num = player->data->change_special_num;
	guoYu.guoyu_level = player->data->guoyu.guoyu_level;
	guoYu.cur_exp = player->data->guoyu.cur_exp;
	guoYu.cur_task = player->data->guoyu.cur_task;
	guoYu.task_cd = player->data->guoyu.task_timeout;
	guoYu.type = player->data->guoyu.type;
	guoYu.critical_num = player->data->guoyu.critical_num;
	guoYu.guoyu_num = player->data->guoyu.guoyu_num;
	guoYu.map = player->data->guoyu.map;
	guoYu.rand_map = player->data->guoyu.random_map;
	guoYu.critical_cd = player->data->guoyu.critical_cd;
	guoYu.critical_cd_refresh = player->data->guoyu.critical_next_refresh;

	static ChengjieTypeDb chengJie;
	chengjie_type_db__init(&chengJie);
	send.chengjie = &chengJie;
	chengJie.level = player->data->chengjie.level;
	chengJie.num = player->data->chengjie.chengjie_num;
	chengJie.cur_exp = player->data->chengjie.cur_exp;
	chengJie.task = player->data->chengjie.cur_task;
	chengJie.target = player->data->chengjie.target;
	chengJie.first_hit = player->data->chengjie.first_hit;

	static ShangjinTypeDb shangJin;
	shangjin_type_db__init(&shangJin);
	send.shangjin = &shangJin;
	shangJin.level = player->data->shangjin.level;
	shangJin.num = player->data->shangjin.shangjin_num;
	shangJin.cur_exp = player->data->shangjin.cur_exp;
	shangJin.cur_task = player->data->shangjin.cur_task;
	shangJin.accept = player->data->shangjin.accept;
	shangJin.free_refresh = player->data->shangjin.free;
	static ShangjinTaskTypeDb arrTask[MAX_SHANGJIN_NUM];
	static ShangjinTaskTypeDb *arrTaskPoint[MAX_SHANGJIN_NUM] = { arrTask, arrTask + 1, arrTask + 2 };
	shangJin.task = arrTaskPoint;
	shangJin.n_task = MAX_SHANGJIN_NUM;
	static ShangjinTaskAwardDb arrAward[MAX_SHANGJIN_NUM][MAX_SHANGJIN_AWARD_NUM];
	static ShangjinTaskAwardDb *arrAwardPoint[MAX_SHANGJIN_NUM][MAX_SHANGJIN_AWARD_NUM];
	for (int i = 0; i < MAX_SHANGJIN_NUM; ++i)
	{
		shangjin_task_type_db__init(arrTask + i);
		arrTask[i].id = player->data->shangjin.task[i].id;
		arrTask[i].quality = player->data->shangjin.task[i].quality;
		arrTask[i].reduce = player->data->shangjin.task[i].reduce;
		arrTask[i].n_award = player->data->shangjin.task[i].n_award;
		arrTask[i].exp = player->data->shangjin.task[i].exp; 
		arrTask[i].coin = player->data->shangjin.task[i].coin;
		for (uint32_t j = 0; j < player->data->shangjin.task[i].n_award; ++j)
		{
			shangjin_task_award_db__init(&arrAward[i][j]);// + i * MAX_SHANGJIN_AWARD_NUM + j);
			arrAward[i][j].id = player->data->shangjin.task[i].award[j].id;
			arrAward[i][j].num = player->data->shangjin.task[i].award[j].val;
			arrAwardPoint[i][j] = &arrAward[i][j];// arrAward + i * j;
		}
		arrTask[i].award = arrAwardPoint[i];
	}

	db_info.yaoshi = &send;
}

void ChengJieTaskManage::unpack_yaoshi(player_struct *player, _PlayerDBInfo *db_info)
{
	if (player == NULL || db_info == NULL || db_info->yaoshi == NULL)
	{
		return;
	}
	player->data->cur_yaoshi = db_info->yaoshi->cur_major;
	
	player->data->change_special_num = db_info->yaoshi->change_num;
	player->data->guoyu.guoyu_level = db_info->yaoshi->guoyu->guoyu_level;
	player->data->guoyu.cur_exp = db_info->yaoshi->guoyu->cur_exp;
	player->data->guoyu.cur_task = db_info->yaoshi->guoyu->cur_task;
	player->data->guoyu.task_timeout = db_info->yaoshi->guoyu->task_cd;
	player->data->guoyu.critical_num = db_info->yaoshi->guoyu->critical_num;
	player->data->guoyu.guoyu_num = db_info->yaoshi->guoyu->guoyu_num;
	player->data->guoyu.map = db_info->yaoshi->guoyu->map;
	player->data->guoyu.type = db_info->yaoshi->guoyu->type;
	player->data->guoyu.random_map = db_info->yaoshi->guoyu->rand_map;
	player->data->guoyu.critical_cd = db_info->yaoshi->guoyu->critical_cd;
	player->data->guoyu.critical_next_refresh = db_info->yaoshi->guoyu->critical_cd_refresh;

	player->data->chengjie.level = db_info->yaoshi->chengjie->level;
	player->data->chengjie.chengjie_num = db_info->yaoshi->chengjie->num;
	player->data->chengjie.cur_exp = db_info->yaoshi->chengjie->cur_exp;
	player->data->chengjie.cur_task = db_info->yaoshi->chengjie->task;
	player->data->chengjie.target = db_info->yaoshi->chengjie->target;
	player->data->chengjie.first_hit = db_info->yaoshi->chengjie->first_hit;

	player->data->shangjin.level = db_info->yaoshi->shangjin->level;
	player->data->shangjin.shangjin_num = db_info->yaoshi->shangjin->num;
	player->data->shangjin.cur_exp = db_info->yaoshi->shangjin->cur_exp;
	player->data->shangjin.cur_task = db_info->yaoshi->shangjin->cur_task;
	player->data->shangjin.accept = db_info->yaoshi->shangjin->accept;
	player->data->shangjin.free = db_info->yaoshi->shangjin->free_refresh;
	for (uint32_t i = 0; i < db_info->yaoshi->shangjin->n_task; ++i)
	{
		player->data->shangjin.task[i].id = db_info->yaoshi->shangjin->task[i]->id;
		player->data->shangjin.task[i].quality = db_info->yaoshi->shangjin->task[i]->quality;
		player->data->shangjin.task[i].reduce = db_info->yaoshi->shangjin->task[i]->reduce;
		player->data->shangjin.task[i].coin = db_info->yaoshi->shangjin->task[i]->coin;
		player->data->shangjin.task[i].exp = db_info->yaoshi->shangjin->task[i]->exp;
		for (uint32_t j = 0; j < db_info->yaoshi->shangjin->task[i]->n_award; ++j)
		{
			player->data->shangjin.task[i].award[j].id = db_info->yaoshi->shangjin->task[i]->award[j]->id;
			player->data->shangjin.task[i].award[j].val = db_info->yaoshi->shangjin->task[i]->award[j]->num;
		}
		player->data->shangjin.task[i].n_award = db_info->yaoshi->shangjin->task[i]->n_award;
	}
}


STChengJie * ChengJieTaskManage::FindTask(uint32_t taskId)
{
	CHENGJIE_CONTAIN::iterator it = ChengJieTaskManage_m_contain.find(taskId);
	if (it == ChengJieTaskManage_m_contain.end())
	{
		return NULL;
	}
	return &(it->second);
}

bool ChengJieTaskManage::IsTarget(uint64_t pid)
{
	CHENGJIE_TARGET::iterator it = ChengJieTaskManage_m_target.find(pid);
	return it == ChengJieTaskManage_m_target.end() ? false : true;
}

uint64_t ChengJieTaskManage::GetTaskAccept(uint64_t pid)
{
	CHENGJIE_TARGET::iterator it = ChengJieTaskManage_m_target.find(pid);
	return it == ChengJieTaskManage_m_target.end() ? 0 : it->second;
}

int ChengJieTaskManage::CanUseFollowItem(player_struct *player)
{
	if (player == NULL)
	{
		return 1;
	}
	int ret = 0;
	player_struct *target = player_manager::get_player_by_id(player->data->chengjie.target);
	if (player->data->chengjie.cur_task == 0)
	{
		ret = 190500146;
	}
	else
	{
		STChengJie *pTask = ChengJieTaskManage::FindTask(player->data->chengjie.cur_task);
		if (pTask == NULL)
		{
			ret = 190500146;
		}
		else
		{
			if (pTask->complete)
			{
				ret = 190500146;
			}
			else
			{

				if (target == NULL)
				{
					ret = 190500155;
				}
			}
		}
	}


	if (ret != 0)
	{
		return ret;
	}
	else
	{
		ret = 190500154;
		if (target->in_watched_list(target->get_uuid()))
		{
			ret = 190500156;
		}
	}

	return ret;
}

int ChengJieTaskManage::NotifyTargetPos(player_struct *player)
{
	if (player == NULL)
	{
		return 1;
	}
	player_struct *target = player_manager::get_player_by_id(player->data->chengjie.target);
	if (target == NULL)
	{
		return 2;
	}
	CommAnswer send;
	comm_answer__init(&send);
	send.result = 190500154;
	if (!target->add_watched_list(target->get_uuid()))
	{
		send.result = 190500156;
	}
	
	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_ACCEPT_SHANGJIN_TASK_ANSWER, comm_answer__pack, send);

	SpecialPlayerPosNotify nty;
	special_player_pos_notify__init(&nty);
	struct position *pos = target->get_pos();
	nty.scene_id = target->data->scene_id;
	nty.uuid = target->data->player_id;
	nty.pos_x = pos->pos_x;
	nty.pos_z = pos->pos_z;
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_SPECIAL_PLAYER_POS_NOTIFY, special_player_pos_notify__pack, nty);

	return 190500154;
}

void ChengJieTaskManage::NotifyTargetLogin(player_struct *target)
{
	if (target == NULL)
	{
		return;
	}
	CHENGJIE_TARGET::iterator it = ChengJieTaskManage_m_target.find(target->get_uuid());
	if (it == ChengJieTaskManage_m_target.end())
	{
		return;
	}
	player_struct *player = player_manager::get_player_by_id(it->second);
	if (player == NULL)
	{
		return;
	}

	ChengjieKiller send;
	chengjie_killer__init(&send);
	send.playerid = target->get_uuid();
	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_CHENGJIE_TARGET_LOGIN_NOTIFY, chengjie_killer__pack, send);

	ChengjieKiller sendTar;
	chengjie_killer__init(&sendTar);
	sendTar.playerid = player->get_uuid();
	extern_data.player_id = target->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_CHENGJIE_KILLER_NOTIFY, chengjie_killer__pack, sendTar);
}

bool CheckGuoyuTaskOpen(int level, int &cd)
{
	TypeLevelTable *table = get_guoyu_level_table(level);
	if (table == NULL)
	{
		return false;
	}
	bool open = false;
	for (uint32_t i = 0; i < table->n_OpenDay; ++i)
	{
		if (time_helper::getWeek() == table->OpenDay[i])
		{
			open = true;
			break;
		}
	}
	if (!open)
	{
		return false;
	}
	open = false;
	struct tm tm;
	time_t tmp = time_helper::get_cached_time() / 1000;
	localtime_r(&tmp, &tm);
	for (uint32_t i = 0; i < table->n_OpenTime; ++i)
	{
		tm.tm_hour = table->OpenTime[i] / 100;
		tm.tm_min = table->OpenTime[i] % 100;
		tm.tm_sec = 0;
		uint64_t st = mktime(&tm);
		tm.tm_hour = table->CloseTime[i] / 100;
		tm.tm_min = table->CloseTime[i] % 100;
		tm.tm_sec = 59;
		uint64_t end = mktime(&tm);
		if (time_helper::get_cached_time() / 1000 >= st && time_helper::get_cached_time() / 1000 <= end)
		{
			open = true;
			cd = end - time_helper::get_cached_time() / 1000;
			break;
		}
	}
	if (!open)
	{
		cd = 0;
		return false;
	}
	return true;
}

void CheckGuoyuCriticalTask()
{
	int cd = 0;
	if (CheckGuoyuTaskOpen(GUOYU__TASK__TYPE__CRITICAL, cd))
	{
		uint64_t now = time_helper::get_cached_time() / 1000;
		std::map<uint64_t, player_struct *>::iterator it = player_manager_all_players_id.begin();
		for (; it != player_manager_all_players_id.end(); ++it)
		{
			player_struct *player = it->second;
			if (now > player->data->guoyu.critical_next_refresh)
			{
				uint32_t r = rand() % 10000;
				uint32_t all = 0;
				TypeLevelTable *table = get_guoyu_level_table(GUOYU__TASK__TYPE__CRITICAL);
				if (table != NULL)
				{
					all = table->OpenProbability;
					if (player->data->cur_yaoshi == MAJOR__TYPE__GUOYU)
					{
						all += table->SpecialtyPlus;
					}
				}
				if (r <= all)
				{
					player->data->guoyu.critical_cd = now + cd;
					GuoyuFb send;
					guoyu_fb__init(&send);
					send.cd = cd;
					EXTERN_DATA extern_data;
					extern_data.player_id = player->get_uuid();
					fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_GUOYU_CRITICAL_CD_NOTIFY, guoyu_fb__pack, send);
					//conn_node_gamesrv::send_to_all_player(MSG_ID_GUOYU_CRITICAL_CD_NOTIFY, &send, (pack_func)guoyu_fb__pack);
				}
				player->data->guoyu.critical_next_refresh = now + cd;
			}
		}
	}
}

void CheckAwardQuestionTask()
{
	static bool bDone = false;
	uint32_t cd = 0;
	if (!check_active_open(AWARD_QUESTION_ACTIVE_ID, cd))
	{
		if (!bDone)
		{
			std::map<uint64_t, player_struct *>::iterator it = player_manager_all_players_id.begin();
			for (; it != player_manager_all_players_id.end(); ++it)
			{
				player_struct *player = it->second;
				player->clear_award_question();
			}
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

bool SortChengjieList(uint32_t left, uint32_t right)
{
	STChengJie *lTask = ChengJieTaskManage::FindTask(left);
	STChengJie *rTask = ChengJieTaskManage::FindTask(right);
	if (lTask == NULL)
	{
		return false;
	}
	if (rTask == NULL)
	{
		return true;
	}
	player_struct *targetL = player_manager::get_player_by_id(lTask->pid);
	player_struct *targetR = player_manager::get_player_by_id(rTask->pid);
	if ((targetL != NULL && targetR != NULL) || (targetL == NULL && targetR == NULL))
	{
		if (lTask->shuangjin > rTask->shuangjin)
		{
			if (lTask->timeOut > rTask->timeOut)
			{
				if (!lTask->complete)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}

		}
		else
		{
			return false;
		}
	}
	else if (targetL == NULL && targetR != NULL)
	{
		return false;
	}

	return true;
}

void ChengJieTaskManage::SortList()
{
	std::sort(ChengJieTaskManage_m_containVt.begin(), ChengJieTaskManage_m_containVt.end(), SortChengjieList);
}

void ChengJieTaskManage::OnTimer()
{
	CHENGJIE_CONTAIN::iterator it = ChengJieTaskManage_m_contain.begin();
	uint64_t now = time_helper::get_cached_time() / 1000;
	for (; it != ChengJieTaskManage_m_contain.end(); ++it)
	{
		if (it->second.taskTime != 0 && now > it->second.taskTime) //完成CD到
		{
			ChengJieTaskManage::TaskExpire(it->second);
		}
		if (it->second.timeOut < now && !it->second.complete)
		{
			ChengjieMoney send;
			chengjie_money__init(&send);
			send.pid = it->second.investor;
			send.target = it->second.pid;
			send.cd = it->second.timeOut;
			send.money = it->second.shuangjin;
			EXTERN_DATA ext_data;
			ext_data.player_id = send.pid;
			conn_node_gamesrv::connecter.send_to_friend(&ext_data, SERVER_PROTO_CHENGJIE_MONEY_BACK, &send, (pack_func)chengjie_money__pack);
			DelTask(it->first);
			break;
		}
	}
	CheckGuoyuCriticalTask();
	CheckAwardQuestionTask();
}

void ChengJieTaskManage::SetRoleTarget(uint64_t target, uint64_t playerid)
{
	CHENGJIE_TARGET::iterator it = ChengJieTaskManage_m_target.find(target);
	if (it != ChengJieTaskManage_m_target.end())
	{
		it->second = playerid;
	}
}

void ChengJieTaskManage::AddRoleLevel(uint64_t playerid, uint32_t level, uint64_t cd)
{
	CHENGJIE_ROLE_LEVEL::iterator it = ChengJieTaskManage_m_RoleLevel.find(playerid);
	if (it != ChengJieTaskManage_m_RoleLevel.end())
	{
		it->second.level = level;
		it->second.cd = cd;
	}
	else
	{
		ChengjieCd tmp;
		tmp.level = level;
		tmp.cd = cd;
		ChengJieTaskManage_m_RoleLevel.insert(std::make_pair(playerid, tmp));
	}
}

void ChengJieTaskManage::DelRoleLevel(uint64_t playerid)
{
	ChengJieTaskManage_m_RoleLevel.erase(playerid);
}

uint32_t ChengJieTaskManage::GetRoleLevel(uint64_t playerid)
{
	CHENGJIE_ROLE_LEVEL::iterator it = ChengJieTaskManage_m_RoleLevel.find(playerid);
	if (it != ChengJieTaskManage_m_RoleLevel.end())
	{
		return it->second.level ;
	}
	else
	{
		return 0;
	}
}
uint64_t ChengJieTaskManage::GetRoleCd(uint64_t playerid)
{
	CHENGJIE_ROLE_LEVEL::iterator it = ChengJieTaskManage_m_RoleLevel.find(playerid);
	if (it != ChengJieTaskManage_m_RoleLevel.end())
	{
		return it->second.cd;
	}
	else
	{
		return 0;
	}
}

int ChengJieTaskManage::ChengjieAddHurt(player_struct &player, player_struct &target)
{
	if (target.get_uuid() != player.data->chengjie.target)
	{
		return 0;
	}
	if (player.data->scene_id > SCENCE_DEPART)
	{
		return 0;
	}
	SpecialtySkillTable *tableSkill = NULL;
	if (!player.data->chengjie.first_hit)
	{
		player.data->chengjie.first_hit = true;
		tableSkill = get_yaoshi_skill_config(CHENGJIE_FIVE, player.data->chengjie.level);
		if (tableSkill != NULL)
		{
			buff_manager::create_default_buff(tableSkill->EffectValue[0], &player, &target, true);
		}
	}
	tableSkill = get_yaoshi_skill_config(CHENGJIE_SEVEN, player.data->chengjie.level);
	if (tableSkill == NULL)
	{
		return 0;
	}
	return tableSkill->EffectValue[0];
}

int ChengJieTaskManage::ChengjieRedeuceHurt(player_struct &player, player_struct &target)
{
	if (player.get_uuid() != target.data->chengjie.target)
	{
		return 0;
	}
	if (player.data->scene_id > SCENCE_DEPART)
	{
		return 0;
	}
	SpecialtySkillTable *tableSkill = get_yaoshi_skill_config(CHENGJIE_SIX, target.data->chengjie.level);
	if (tableSkill == NULL)
	{
		return 0;
	}
	return tableSkill->EffectValue[0];
}

int ChengJieTaskManage::GuoyuAddHurt(player_struct &player, monster_struct &target)
{
	if (player.data->guoyu.cur_task == 0)
	{
		return 0;
	}
	std::map<uint64_t, struct RandomMonsterTable*>::iterator it = random_monster.find(player.data->guoyu.cur_task);
	if (it == random_monster.end())
	{
		return 0;
	}
	if (it->second->MonsterID != target.data->monster_id)
	{
		return 0;
	}
	SpecialtySkillTable *tableSkill = get_yaoshi_skill_config(GUOYU_ELEVEN, player.data->guoyu.guoyu_level);
	if (tableSkill == NULL)
	{
		return 0;
	}
	return tableSkill->EffectValue[0];
}

int ChengJieTaskManage::GuoyuRedeuceHurt(monster_struct &monster, player_struct &target)
{
	if (target.data->guoyu.cur_task == 0)
	{
		return 0;
	}
	std::map<uint64_t, struct RandomMonsterTable*>::iterator it = random_monster.find(target.data->guoyu.cur_task);
	if (it == random_monster.end())
	{
		return 0;
	}
	if (it->second->MonsterID != monster.data->monster_id)
	{
		return 0;
	}
	SpecialtySkillTable *tableSkill = get_yaoshi_skill_config(GUOYU_TEN, target.data->guoyu.guoyu_level);
	if (tableSkill == NULL)
	{
		return 0;
	}
	return tableSkill->EffectValue[0];
}













void ShangjinManage::RefreshTask(player_struct *player)
{
	MoneyQuestTable *table = get_shangjin_task_table(player->get_attr(PLAYER_ATTR_LEVEL));
	if (table == NULL)
	{
		LOG_ERR("[%s:%d] can not get shangjin task table player[%lu] ", __FUNCTION__, __LINE__, player->get_uuid());
		return;
	}
	SpecialtySkillTable *tableSkill = get_yaoshi_skill_config(SHANGJIN_ONE, player->data->shangjin.level);
	if (tableSkill == NULL)
	{
		LOG_ERR("[%s:%d] can not get shangjin task table player[%lu] ", __FUNCTION__, __LINE__, player->get_uuid());
		return;
	}
	if (table->n_QuestGroup == 0 || table->n_RewardGroup == 0)
	{
		LOG_ERR("[%s:%d] can not get shangjin task table player[%lu] ", __FUNCTION__, __LINE__, player->get_uuid());
		return;
	}
	for (int i = 0; i < MAX_SHANGJIN_NUM; ++i)
	{
		player->data->shangjin.task[i].id = table->QuestGroup[rand() % table->n_QuestGroup];

		//player->data->shangjin.task[i].id = 240130002;
		uint32_t quality = 0;
		uint32_t r = rand() % 10000;
		player->data->shangjin.task[i].quality = 1;
		for (uint32_t q = 0; q < tableSkill->n_EffectValue; ++q)
		{
			quality += tableSkill->EffectValue[q];
			if (r <= quality)
			{
				player->data->shangjin.task[i].quality = 1 + q;
				break;
			}
		}

		uint32_t awardId = 0;
		switch (player->data->shangjin.task[i].quality)
		{
		case 1:
			awardId = table->RewardGroup[rand() % table->n_RewardGroup];
			break;
		case 2:
			awardId = table->RewardGroup1[rand() % table->n_RewardGroup1];
			break;
		case 3:
			awardId = table->RewardGroup2[rand() % table->n_RewardGroup2];
			break;
		case 4:
			awardId = table->RewardGroup3[rand() % table->n_RewardGroup3];
			break;
		case 5:
			awardId = table->RewardGroup4[rand() % table->n_RewardGroup4];
			break;
		case 6:
			awardId = table->RewardGroup5[rand() % table->n_RewardGroup5];
			break;
		}
		player->data->shangjin.task[i].n_award = 0;
		TaskRewardTable *reward_config = get_config_by_id(awardId, &task_reward_config);
		if (reward_config != NULL)
		{
			for (uint32_t j = 0; j < reward_config->n_RewardTarget && j < MAX_SHANGJIN_AWARD_NUM; ++j)
			{
				if (reward_config->RewardTarget[j] == 0)
				{
					continue;
				}
				player->data->shangjin.task[i].award[j].id = reward_config->RewardTarget[j];
				player->data->shangjin.task[i].award[j].val = reward_config->RewardNum[j];
				++player->data->shangjin.task[i].n_award;
			}
			player->data->shangjin.task[i].coin = reward_config->RewardMoney;
			player->data->shangjin.task[i].exp = reward_config->RewardEXP;
		}

		player->data->shangjin.task[i].reduce = 0;
		SpecialtySkillTable *tableReduce = get_yaoshi_skill_config(SHANGJIN_TWO, player->data->shangjin.level);
		if (tableReduce != NULL)
		{
			TaskTable *configTask = get_config_by_id(player->data->shangjin.task[i].id, &task_config);
			if (configTask != NULL)
			{
				TaskConditionTable *configCon = get_config_by_id(configTask->EndConID[0], &task_condition_config);
				if (configCon != NULL) 
				{
					player->data->shangjin.task[i].reduce = configCon->ConditionNum * tableReduce->EffectValue[0] / 10000;
				}
			}
		}
	}
	player->data->shangjin.cur_task = 0;
}

void ShangjinManage::RefreshTaskAndSend(player_struct *player, bool sys)
{
	RefreshTask(player);

	ShangjinList shangJin;
	shangjin_list__init(&shangJin);
	shangJin.ret = 0;
	shangJin.refresh = sys;
	shangJin.cur_task = player->data->shangjin.cur_task;
	shangJin.accept = player->data->shangjin.accept;
	ShangjinTaskType arrTask[MAX_SHANGJIN_NUM];
	ShangjinTaskType *arrTaskPoint[MAX_SHANGJIN_NUM] = { arrTask, arrTask + 1, arrTask + 2 };
	shangJin.task_list = arrTaskPoint;
	shangJin.n_task_list = MAX_SHANGJIN_NUM;
	ShangjinTaskAward arrAward[MAX_SHANGJIN_NUM][MAX_SHANGJIN_AWARD_NUM];
	ShangjinTaskAward *arrAwardPoint[MAX_SHANGJIN_NUM][MAX_SHANGJIN_AWARD_NUM];
	for (int i = 0; i < MAX_SHANGJIN_NUM; ++i)
	{
		shangjin_task_type__init(arrTask + i);
		arrTask[i].id = player->data->shangjin.task[i].id;
		arrTask[i].quality = player->data->shangjin.task[i].quality;
		arrTask[i].reduce = player->data->shangjin.task[i].reduce;
		arrTask[i].n_award = player->data->shangjin.task[i].n_award;
		arrTask[i].coin = player->data->shangjin.task[i].coin;
		arrTask[i].exp = player->data->shangjin.task[i].exp;
		for (uint32_t j = 0; j < player->data->shangjin.task[i].n_award; ++j)
		{
			shangjin_task_award__init(&arrAward[i][j]);// + i * MAX_SHANGJIN_AWARD_NUM + j);
			arrAward[i][j].id = player->data->shangjin.task[i].award[j].id;
			arrAward[i][j].num = player->data->shangjin.task[i].award[j].val;
			arrAwardPoint[i][j] = &arrAward[i][j];// arrAward + i * j;
		}
		arrTask[i].award = arrAwardPoint[i];
	}

	EXTERN_DATA ext_data;
	ext_data.player_id = player->get_uuid();
	fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_REFRESH_SHANGJIN_TASK_ANSWER, shangjin_list__pack, shangJin);
}

void ShangjinManage::CompleteTask(player_struct *player, uint32_t taskid)
{
	if (player == NULL)
	{
		return;
	}
	if (taskid != player->data->shangjin.task[player->data->shangjin.cur_task].id)
	{
		return;
	}

	// 奖励 
	std::map<uint32_t, uint32_t> item_list;

	SpecialtySkillTable *tableSkill = get_yaoshi_skill_config(SHANGJIN_FOUR, player->data->shangjin.level); 
	uint32_t add = 0;
	if (tableSkill != NULL)
	{
		add = tableSkill->EffectValue[0];
	}
	for (uint32_t i = 0; i < player->data->shangjin.task[player->data->shangjin.cur_task].n_award; ++i)
	{
		uint32_t addTwo = add;
		if (get_item_type(player->data->shangjin.task[player->data->shangjin.cur_task].award[i].id) == ITEM_TYPE_SHANGJIN_EXP)
		{
			if (player->data->cur_yaoshi == MAJOR__TYPE__SHUANGJIN)  //专精加成
			{
				SpecialTitleTable *title = get_yaoshi_title_table(player->data->cur_yaoshi, player->data->shangjin.level);
				if (title != NULL)
				{
					addTwo += title->TitleEffect2;
				}
			}
		}
		item_list.insert(std::make_pair(player->data->shangjin.task[player->data->shangjin.cur_task].award[i].id,
			player->data->shangjin.task[player->data->shangjin.cur_task].award[i].val * (addTwo + 10000) / 10000));
	}
	player->add_exp(player->data->shangjin.task[player->data->shangjin.cur_task].exp * (add + 10000) / 10000, MAGIC_TYPE_YAOSHI);
	player->add_coin(player->data->shangjin.task[player->data->shangjin.cur_task].coin * (add + 10000) / 10000,MAGIC_TYPE_YAOSHI);
	player->add_item_list_otherwise_send_mail(item_list, MAGIC_TYPE_YAOSHI, 0, NULL, true);

	if (player->data->shangjin.cur_task + 1 == MAX_SHANGJIN_NUM)
	{
		player->data->shangjin.accept = false;
		player->data->shangjin.cur_task = 0;
		RefreshTaskAndSend(player, false);
	}
	else
	{
		ShangjinTaskId send;
		shangjin_task_id__init(&send);
		send.taskid = ++player->data->shangjin.cur_task;
		EXTERN_DATA ext_data;
		ext_data.player_id = player->get_uuid();
		fast_send_msg(&conn_node_gamesrv::connecter, &ext_data, MSG_ID_CUR_SHANGJIN_TASK_NOTIFY, shangjin_task_id__pack, send);
		
		player->accept_task(player->data->shangjin.task[player->data->shangjin.cur_task].id, false);
	}
}

