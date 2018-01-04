#include "conn_node_guildsrv.h"
#include "game_event.h"
#include "msgid.h"
#include "error_code.h"
#include "mysql_module.h"
#include "time_helper.h"
#include <vector>
#include <set>
#include "guild_util.h"
#include "redis_util.h"
#include "guild.pb-c.h"
#include "app_data_statis.h"
#include <algorithm>
#include "shop.pb-c.h"
#include "chat.pb-c.h"
#include "personality.pb-c.h"
#include "guild_answer.h"
#include "guild_battle.pb-c.h"
#include "send_mail.h"
#include "guild.pb-c.h"
#include "role.pb-c.h"
#include "activity_db.pb-c.h"
#include <math.h>


conn_node_guildsrv conn_node_guildsrv::connecter;
//static char sql[1024];

#define GUILD_RUQIN_MAX_REWARD_ITEM_NUM 5 //帮会入侵最大奖励物品数量
static const int GUILD_INTRUSION_CONTROLTABLE_ID = 440500001; //帮会入侵 FactionActivity表id
static int handle_guild_create_cost(int data_len, uint8_t *data, int result, EXTERN_DATA *extern_data);
static int handle_guild_rename_cost(int data_len, uint8_t *data, int result, EXTERN_DATA *extern_data);
static int handle_guild_skill_practice_cost(int data_len, uint8_t *data, int result, EXTERN_DATA *extern_data);
static int handle_shop_buy_answer(int data_len, uint8_t *data, int result, EXTERN_DATA *extern_data);
static int handle_guild_donate_cost(int data_len, uint8_t *data, int result, EXTERN_DATA *extern_data);

conn_node_guildsrv::conn_node_guildsrv()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);
	
	add_msg_handle(MSG_ID_GUILD_LIST_REQUEST, &conn_node_guildsrv::handle_guild_list_request);
	add_msg_handle(MSG_ID_GUILD_INFO_REQUEST, &conn_node_guildsrv::handle_guild_info_request);
	add_msg_handle(MSG_ID_GUILD_MEMBER_LIST_REQUEST, &conn_node_guildsrv::handle_guild_member_list_request);
	add_msg_handle(MSG_ID_GUILD_CREATE_REQUEST, &conn_node_guildsrv::handle_guild_create_request);
	add_msg_handle(MSG_ID_GUILD_JOIN_REQUEST, &conn_node_guildsrv::handle_guild_join_request);
	add_msg_handle(MSG_ID_GUILD_JOIN_LIST_REQUEST, &conn_node_guildsrv::handle_guild_join_list_request);
	add_msg_handle(MSG_ID_GUILD_DEAL_JOIN_REQUEST, &conn_node_guildsrv::handle_guild_deal_join_request);
	add_msg_handle(MSG_ID_GUILD_TURN_SWITCH_REQUEST, &conn_node_guildsrv::handle_guild_turn_switch_request);
	add_msg_handle(MSG_ID_GUILD_SET_WORDS_REQUEST, &conn_node_guildsrv::handle_guild_set_words_request);
	add_msg_handle(MSG_ID_GUILD_APPOINT_OFFICE_REQUEST, &conn_node_guildsrv::handle_guild_appoint_office_request);
	add_msg_handle(MSG_ID_GUILD_KICK_REQUEST, &conn_node_guildsrv::handle_guild_kick_request);
	add_msg_handle(MSG_ID_GUILD_RENAME_REQUEST, &conn_node_guildsrv::handle_guild_rename_request);
	add_msg_handle(MSG_ID_GUILD_EXIT_REQUEST, &conn_node_guildsrv::handle_guild_exit_request);
	add_msg_handle(MSG_ID_GUILD_SET_PERMISSION_REQUEST, &conn_node_guildsrv::handle_guild_set_permission_request);
	add_msg_handle(MSG_ID_GUILD_INVITE_REQUEST, &conn_node_guildsrv::handle_guild_invite_request);
	add_msg_handle(MSG_ID_GUILD_DEAL_INVITE_REQUEST, &conn_node_guildsrv::handle_guild_deal_invite_request);

	add_msg_handle(SERVER_PROTO_PLAYER_ONLINE_NOTIFY, &conn_node_guildsrv::handle_player_online_notify);
	add_msg_handle(SERVER_PROTO_GUILDSRV_COST_ANSWER, &conn_node_guildsrv::handle_check_and_cost_answer);
	add_msg_handle(SERVER_PROTO_GUILDSRV_REWARD_ANSWER, &conn_node_guildsrv::handle_gamesrv_reward_answer);
	add_msg_handle(SERVER_PROTO_ADD_GUILD_RESOURCE, &conn_node_guildsrv::handle_add_guild_resource_request);
	add_msg_handle(SERVER_PROTO_GM_DISBAND_GUILD, &conn_node_guildsrv::handle_disband_request);
	add_msg_handle(SERVER_PROTO_GAMESRV_START, &conn_node_guildsrv::handle_gamesrv_start);

	add_msg_handle(MSG_ID_GUILD_BUILDING_INFO_REQUEST, &conn_node_guildsrv::handle_guild_building_info_request);
	add_msg_handle(MSG_ID_GUILD_BUILDING_UPGRADE_REQUEST, &conn_node_guildsrv::handle_guild_building_upgrade_request);
	add_msg_handle(SERVER_PROTO_SUB_GUILD_BUILDING_TIME, &conn_node_guildsrv::handle_sub_guild_building_time_request);
	add_msg_handle(MSG_ID_GUILD_ACCEPT_TASK_REQUEST, &conn_node_guildsrv::handle_guild_accept_task_request);
	add_msg_handle(SERVER_PROTO_GUILD_ACCEPT_TASK_ANSWER, &conn_node_guildsrv::handle_game_accept_task_answer);
	add_msg_handle(SERVER_PROTO_GUILD_TASK_FINISH, &conn_node_guildsrv::handle_game_task_finish_notify);

	add_msg_handle(MSG_ID_GUILD_SKILL_INFO_REQUEST, &conn_node_guildsrv::handle_guild_skill_info_request);
	add_msg_handle(MSG_ID_GUILD_SKILL_DEVELOP_REQUEST, &conn_node_guildsrv::handle_guild_skill_develop_request);
	add_msg_handle(MSG_ID_GUILD_SKILL_PRACTICE_REQUEST, &conn_node_guildsrv::handle_guild_skill_pratice_request);

	add_msg_handle(MSG_ID_GUILD_SHOP_INFO_REQUEST, &conn_node_guildsrv::handle_guild_shop_info_request);
	add_msg_handle(MSG_ID_GUILD_SHOP_BUY_REQUEST, &conn_node_guildsrv::handle_guild_shop_buy_request);

	add_msg_handle(SERVER_PROTO_GUILD_CHAT, &conn_node_guildsrv::handle_guild_chat_request);
	add_msg_handle(SERVER_PROTO_GUILD_PRODUCE_MEDICINE, &conn_node_guildsrv::handle_guild_prodece_medicine_request);
	add_msg_handle(MSG_ID_GET_OTHER_INFO_ANSWER, &conn_node_guildsrv::handle_get_other_info_request);
	add_msg_handle(SERVER_PROTO_TRADE_LOT_INSERT, &conn_node_guildsrv::handle_trade_lot_insert);

	add_msg_handle(MSG_ID_GUILD_BATTLE_CALL_REQUEST, &conn_node_guildsrv::handle_guild_battle_call_request);
	add_msg_handle(SERVER_PROTO_GUILD_BATTLE_ENTER_WAIT, &conn_node_guildsrv::handle_guild_battle_enter_wait_request);
	add_msg_handle(MSG_ID_GUILD_BATTLE_INFO_REQUEST, &conn_node_guildsrv::handle_guild_battle_info_request);
	add_msg_handle(SERVER_PROTO_GUILD_BATTLE_REWARD, &conn_node_guildsrv::handle_guild_battle_fight_reward_request);
	add_msg_handle(SERVER_PROTO_GUILD_BATTLE_BEGIN, &conn_node_guildsrv::handle_guild_battle_sync_begin);
	add_msg_handle(SERVER_PROTO_GUILD_BATTLE_END, &conn_node_guildsrv::handle_guild_battle_sync_end);
	add_msg_handle(SERVER_PROTO_GUILD_BATTLE_SETTLE, &conn_node_guildsrv::handle_guild_battle_sync_settle);
	add_msg_handle(SERVER_PROTO_GUILD_BATTLE_FINAL_LIST_REQUEST, &conn_node_guildsrv::handle_guild_battle_final_list_request);
	add_msg_handle(SERVER_PROTO_GUILD_ADD_FINAL_BATTLE_GUILD, &conn_node_guildsrv::handle_guild_battle_add_final_id);

	add_msg_handle(MSG_ID_OPEN_FACTION_QUESTION_REQUEST, &conn_node_guildsrv::handle_open_guild_answer_request);
	add_msg_handle(SERVER_PROTO_GUILD_RUQIN_CREAT_MONSTER_LEVEL_REQUEST, &conn_node_guildsrv::handle_guild_ruqin_creat_monster_level_request);
	add_msg_handle(SERVER_PROTO_GUILD_RUQIN_REWARD_INFO_NOTIFY, &conn_node_guildsrv::guild_ruqin_reward_info_notify);
	add_msg_handle(SERVER_PROTO_GUILD_RUQIN_BOSS_CREAT_NOTIFY, &conn_node_guildsrv::guild_ruqin_boss_creat_notify);

	add_msg_handle(MSG_ID_GUILD_DONATE_REQUEST, &conn_node_guildsrv::handle_guild_donate_request);
	add_msg_handle(SERVER_PROTO_ACTIVITY_SHIDAMENZONG_GIVE_REWARD_REQUEST, &conn_node_guildsrv::handle_activity_shidamenzong_give_reward_request);
}

conn_node_guildsrv::~conn_node_guildsrv()
{
}

void conn_node_guildsrv::add_msg_handle(uint32_t msg_id, handle_func func)
{
	connecter.m_handleMap[msg_id] = func;
}

int conn_node_guildsrv::recv_func(evutil_socket_t fd)
{
	EXTERN_DATA *extern_data;
	PROTO_HEAD *head;	
	for (;;) {
		int ret = get_one_buf();
		if (ret == 0) {
			head = (PROTO_HEAD *)buf_head();
			int cmd = get_cmd();
			uint64_t times = time_helper::get_micro_time();
			time_helper::set_cached_time(times / 1000);
			switch (cmd)
			{
				case SERVER_PROTO_GAMESRV_START:
				case SERVER_PROTO_GUILD_BATTLE_REWARD:
				case SERVER_PROTO_GUILD_BATTLE_BEGIN:
				case SERVER_PROTO_GUILD_BATTLE_END:
				case SERVER_PROTO_GUILD_BATTLE_SETTLE:
				case SERVER_PROTO_GUILD_BATTLE_FINAL_LIST_REQUEST:
				case SERVER_PROTO_TRADE_LOT_INSERT:
					{
						HandleMap::iterator iter = m_handleMap.find(cmd);
						if (iter != m_handleMap.end())
						{
							(this->*(iter->second))(NULL);
						}
					}
					break;
				default:
					{
						extern_data = get_extern_data(head);
						LOG_DEBUG("[%s:%d] cmd: %u, player_id: %lu", __FUNCTION__, __LINE__, cmd, extern_data->player_id);

						HandleMap::iterator iter = m_handleMap.find(cmd);
						if (iter != m_handleMap.end())
						{
							(this->*(iter->second))(extern_data);
						}
						else
						{
							LOG_ERR("[%s:%d] cmd %u has no handler", __FUNCTION__, __LINE__, cmd);
						}
					}
					break;
			}
		}
		
		if (ret < 0) {
			LOG_INFO("%s: connect closed from fd %u, err = %d", __FUNCTION__, fd, errno);
			exit(0);
			return (-1);		
		} else if (ret > 0) {
			break;
		}
		
		ret = remove_one_buf();
	}
	return (0);
}

void conn_node_guildsrv::send_system_notice(uint64_t player_id, uint32_t id, std::vector<char*> *args, uint64_t target_id)
{
	SystemNoticeNotify nty;
	system_notice_notify__init(&nty);

	nty.id = id;
	if (args)
	{
		nty.n_args = args->size();
		nty.args = &((*args)[0]);
	}
	if (target_id > 0)
	{
		nty.targetid = target_id;
		nty.has_targetid = true;
	}

	EXTERN_DATA ext_data;
	ext_data.player_id = player_id;

	fast_send_msg(&conn_node_guildsrv::connecter, &ext_data, MSG_ID_SYSTEM_NOTICE_NOTIFY, system_notice_notify__pack, nty);
}

int conn_node_guildsrv::broadcast_message(uint16_t msg_id, void *msg_data, pack_func packer, std::vector<uint64_t> &players)
{
	PROTO_HEAD_CONN_BROADCAST *head;
	PROTO_HEAD *real_head;

	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
	head->msg_id = ENDION_FUNC_2(SERVER_PROTO_BROADCAST);
	real_head = &head->proto_head;

	real_head->msg_id = ENDION_FUNC_2(msg_id);
	real_head->seq = 0;
//	memcpy(real_head->data, msg_data, len);
	size_t len = 0;
	if (msg_data && packer)
	{
		len = packer(msg_data, (uint8_t *)real_head->data);
	}
	real_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + len);

	uint64_t *ppp = (uint64_t*)((char *)&head->player_id + len);
	head->num_player_id = 0;
	for (std::vector<uint64_t>::iterator iter = players.begin(); iter != players.end(); ++iter)
	{
		ppp[head->num_player_id++] = *iter;
	}
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD_CONN_BROADCAST) + len + sizeof(uint64_t) * head->num_player_id);
	if (conn_node_guildsrv::connecter.send_one_msg((PROTO_HEAD *)head, 1) != (int)(ENDION_FUNC_4(head->len)))
	{
		LOG_ERR("[%s:%d] send to conn_srv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
	return 0;
}

#define MAX_GUILD_NUM 5000
int conn_node_guildsrv::handle_guild_list_request(EXTERN_DATA *extern_data)
{
	GuildListAnswer resp;
	guild_list_answer__init(&resp);

	GuildBriefData guild_data[MAX_GUILD_NUM];
	GuildBriefData* guild_data_point[MAX_GUILD_NUM];

	AutoReleaseBatchRedisPlayer t1;			
	resp.result = 0;
	resp.guilds = guild_data_point;
	resp.n_guilds = 0;
	std::map<uint32_t, GuildInfo*> &guild_map = get_all_guild();
	std::vector<uint32_t> applied_guild_ids;
	get_player_join_apply(extern_data->player_id, applied_guild_ids);
	for (std::map<uint32_t, GuildInfo*>::iterator iter = guild_map.begin(); iter != guild_map.end(); ++iter)
	{
		GuildInfo *guild = iter->second;
		guild_data_point[resp.n_guilds] = &guild_data[resp.n_guilds];
		guild_brief_data__init(&guild_data[resp.n_guilds]);
		guild_data[resp.n_guilds].guildid = guild->guild_id;
		guild_data[resp.n_guilds].name = guild->name;
		guild_data[resp.n_guilds].icon = guild->icon;
		guild_data[resp.n_guilds].level = get_guild_level(guild);
		guild_data[resp.n_guilds].membernum = guild->member_num;
		guild_data[resp.n_guilds].popularity = guild->popularity;
		guild_data[resp.n_guilds].approvestate = guild->approve_state;
		guild_data[resp.n_guilds].recruitstate = guild->recruit_state;
		guild_data[resp.n_guilds].recruitnotice = guild->recruit_notice;
		guild_data[resp.n_guilds].joinapplied = (std::find(applied_guild_ids.begin(), applied_guild_ids.end(), guild->guild_id) != applied_guild_ids.end());
		guild_data[resp.n_guilds].masterid = guild->master_id;
		guild_data[resp.n_guilds].mastercamp = guild->zhenying;

		PlayerRedisInfo *redis_player = get_redis_player(guild->master_id, sg_player_key, sg_redis_client, t1);
		if (redis_player)
		{
			guild_data[resp.n_guilds].mastername = redis_player->name;
			guild_data[resp.n_guilds].masterjob = redis_player->job;
			guild_data[resp.n_guilds].masterhead = redis_player->head_icon;
		}

//		for (uint32_t i = 0; i < guild->member_num; ++i)
//		{
//			GuildPlayer *player = guild->members[i];
//			if (!player)
//			{
//				continue;
//			}
//
//			PlayerRedisInfo *redis_player = get_redis_player(player->player_id, sg_player_key, sg_redis_client, t1);
//			if (!redis_player)
//			{
//				continue;
//			}
//
//			guild_data[resp.n_guilds].totalfc += redis_player->fighting_capacity;
//			if (player->player_id == guild->master_id)
//			{
//				guild_data[resp.n_guilds].masterid = player->player_id;
//				guild_data[resp.n_guilds].mastername = redis_player->name;
//				guild_data[resp.n_guilds].masterjob = redis_player->job;
//				guild_data[resp.n_guilds].masterhead = redis_player->head_icon;
//				guild_data[resp.n_guilds].mastercamp = redis_player->zhenying;
//			}
//		}

		resp.n_guilds++;
	}

	fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_LIST_ANSWER, guild_list_answer__pack, resp);

	return 0;
}

static void pack_guild_basic_data(GuildPlayer *player, AutoReleaseBatchRedisPlayer &t1, GuildData &basic_data, GuildPermissionData permission_data[], GuildPermissionData* permission_point[], 
		GuildLogData usual_log_data[], GuildLogData* usual_log_point[], char *usual_log_args[][MAX_GUILD_LOG_ARG_NUM], 
		GuildLogData important_log_data[], GuildLogData* important_log_point[], char *important_log_args[][MAX_GUILD_LOG_ARG_NUM])
{
	guild_data__init(&basic_data);
	GuildInfo *guild = player->guild;
	basic_data.guildid = guild->guild_id;
	basic_data.name = guild->name;
	basic_data.icon = guild->icon;
	basic_data.level = get_guild_level(guild);
	basic_data.membernum = guild->member_num;
	basic_data.popularity = guild->popularity;
	basic_data.approvestate = guild->approve_state;
	basic_data.recruitstate = guild->recruit_state;
	basic_data.recruitnotice = guild->recruit_notice;
	basic_data.announcement = guild->announcement;
	basic_data.treasure = guild->treasure;
	basic_data.buildboard = guild->build_board;
	basic_data.masterid = guild->master_id;
	basic_data.renametime = guild->rename_time;
	PlayerRedisInfo *redis_master = get_redis_player(guild->master_id, sg_player_key, sg_redis_client, t1);
	if (redis_master)
	{
		basic_data.mastername = redis_master->name;
	}
	basic_data.permissions = permission_point;
	basic_data.n_permissions = 0;
	for (int i = 0; i < MAX_GUILD_OFFICE; ++i)
	{
		permission_point[basic_data.n_permissions] = &permission_data[basic_data.n_permissions];
		guild_permission_data__init(&permission_data[basic_data.n_permissions]);
		permission_data[basic_data.n_permissions].office = guild->permissions[i].office;
		permission_data[basic_data.n_permissions].bits = &guild->permissions[i].permission[GOPT_APPOINT];
		permission_data[basic_data.n_permissions].n_bits = GOPT_END - 1;
		basic_data.n_permissions++;
	}
	basic_data.usual_logs = usual_log_point;
	basic_data.n_usual_logs = 0;
	for (int i = MAX_GUILD_LOG_NUM - 1; i >= 0; --i)
	{
		if (guild->usual_logs[i].type == 0)
		{
			continue;
		}
		usual_log_point[basic_data.n_usual_logs] = &usual_log_data[basic_data.n_usual_logs];
		guild_log_data__init(&usual_log_data[basic_data.n_usual_logs]);
		usual_log_data[basic_data.n_usual_logs].type = guild->usual_logs[i].type;
		usual_log_data[basic_data.n_usual_logs].time = guild->usual_logs[i].time;
		usual_log_data[basic_data.n_usual_logs].args = usual_log_args[basic_data.n_usual_logs];
		usual_log_data[basic_data.n_usual_logs].n_args = 0;
		for (int j = 0; j < MAX_GUILD_LOG_ARG_NUM; ++j)
		{
			if (guild->usual_logs[i].args[j][0] == '\0')
			{
				break;
			}
			usual_log_data[basic_data.n_usual_logs].args[usual_log_data[basic_data.n_usual_logs].n_args] = guild->usual_logs[i].args[j];
			usual_log_data[basic_data.n_usual_logs].n_args++;
		}
		basic_data.n_usual_logs++;
	}
	basic_data.important_logs = important_log_point;
	basic_data.n_important_logs = 0;
	for (int i = MAX_GUILD_LOG_NUM - 1; i >= 0; --i)
	{
		if (guild->important_logs[i].type == 0)
		{
			continue;
		}
		important_log_point[basic_data.n_important_logs] = &important_log_data[basic_data.n_important_logs];
		guild_log_data__init(&important_log_data[basic_data.n_important_logs]);
		important_log_data[basic_data.n_important_logs].type = guild->important_logs[i].type;
		important_log_data[basic_data.n_important_logs].time = guild->important_logs[i].time;
		important_log_data[basic_data.n_important_logs].args = important_log_args[basic_data.n_important_logs];
		important_log_data[basic_data.n_important_logs].n_args = 0;
		for (int j = 0; j < MAX_GUILD_LOG_ARG_NUM; ++j)
		{
			if (guild->important_logs[i].args[j][0] == '\0')
			{
				break;
			}
			important_log_data[basic_data.n_important_logs].args[important_log_data[basic_data.n_important_logs].n_args] = guild->important_logs[i].args[j];
			important_log_data[basic_data.n_important_logs].n_args++;
		}
		basic_data.n_important_logs++;
	}
}

static void pack_guild_personal_data(GuildPlayer *player, GuildPersonalData &personal_data)
{
	guild_personal_data__init(&personal_data);
	personal_data.office = player->office;
	personal_data.donation = player->donation;
	personal_data.allhistorydonation = player->all_history_donation;
	personal_data.curhistorydonation = player->cur_history_donation;
	personal_data.jointime = player->join_time;
	personal_data.exittime = player->exit_time;
	personal_data.taskcount = player->cur_week_task;
	personal_data.taskamount = get_guild_build_task_amount(player->cur_week_task_config_id);
	personal_data.curtaskid = player->cur_task_id;
	personal_data.donatecount = player->donate_count;
}

static void pack_guild_building_data(GuildPlayer *player, GuildBuildingData **&buildings, size_t &n_buildings, GuildBuildingData building_data[], GuildBuildingData* building_data_point[])
{
	buildings = building_data_point;
	n_buildings = 0;
	for (int i = 0; i < MAX_GUILD_BUILDING_NUM; ++i)
	{
		building_data_point[n_buildings] = &building_data[n_buildings];
		guild_building_data__init(&building_data[n_buildings]);
		building_data[n_buildings].buildingid = i + 1;
		building_data[n_buildings].level = player->guild->buildings[i].level;
		n_buildings++;
	}
}

void resp_guild_info(conn_node_guildsrv *node, EXTERN_DATA *extern_data, uint32_t msg_id, uint32_t result, GuildPlayer *player)
{
	GuildInfoAnswer resp;
	guild_info_answer__init(&resp);

	GuildData basic_data;
	guild_data__init(&basic_data);
	GuildPersonalData personal_data;
	guild_personal_data__init(&personal_data);

	GuildPermissionData  permission_data[MAX_GUILD_OFFICE];
	GuildPermissionData* permission_point[MAX_GUILD_OFFICE];
	GuildLogData  usual_log_data[MAX_GUILD_LOG_NUM];
	GuildLogData* usual_log_point[MAX_GUILD_LOG_NUM];
	char*         usual_log_args[MAX_GUILD_LOG_NUM][MAX_GUILD_LOG_ARG_NUM];
	GuildLogData  important_log_data[MAX_GUILD_LOG_NUM];
	GuildLogData* important_log_point[MAX_GUILD_LOG_NUM];
	char*         important_log_args[MAX_GUILD_LOG_NUM][MAX_GUILD_LOG_ARG_NUM];
	GuildBuildingData building_data[MAX_GUILD_BUILDING_NUM];
	GuildBuildingData* building_data_point[MAX_GUILD_BUILDING_NUM];

	AutoReleaseBatchRedisPlayer t1;			
	resp.result = result;
	if (player)
	{
		if (player->guild)
		{
			resp.basicinfo = &basic_data;
			pack_guild_basic_data(player, t1, basic_data, permission_data, permission_point, usual_log_data, usual_log_point, usual_log_args, important_log_data, important_log_point, important_log_args);

			pack_guild_building_data(player, resp.buildings, resp.n_buildings, building_data, building_data_point);
		}

		resp.personalinfo = &personal_data;
		pack_guild_personal_data(player, personal_data);
	}

	fast_send_msg(node, extern_data, msg_id, guild_info_answer__pack, resp);
}

int conn_node_guildsrv::handle_guild_info_request(EXTERN_DATA *extern_data)
{
	GuildPlayer *player = get_guild_player(extern_data->player_id);
	resp_guild_info(&connecter, extern_data, MSG_ID_GUILD_INFO_ANSWER, 0, player);

	return 0;
}

int conn_node_guildsrv::handle_guild_member_list_request(EXTERN_DATA *extern_data)
{
	int ret = 0;
	GuildPlayer *player = NULL;
	GuildInfo *guild = NULL;
	do
	{
		player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not join guild yet", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		guild = player->guild;
	} while(0);

	GuildMemberListAnswer resp;
	guild_member_list_answer__init(&resp);

	GuildMemberData member_data[MAX_GUILD_MEMBER_NUM];
	GuildMemberData* member_data_point[MAX_GUILD_MEMBER_NUM];

	AutoReleaseBatchRedisPlayer t1;
	resp.result = ret;
	if (ret == 0)
	{
		resp.members = member_data_point;
		resp.n_members = 0;
		for (uint32_t i = 0; i < guild->member_num; ++i)
		{
			GuildPlayer *member = guild->members[i];
			member_data_point[resp.n_members] = &member_data[resp.n_members];
			guild_member_data__init(&member_data[resp.n_members]);
			member_data[resp.n_members].playerid = member->player_id;
			member_data[resp.n_members].office = member->office;
			member_data[resp.n_members].donation = member->donation;
			member_data[resp.n_members].curhistorydonation = member->cur_history_donation;
			member_data[resp.n_members].curweekdonation = member->cur_week_donation;
			member_data[resp.n_members].curweektreasure = member->cur_week_treasure;
			member_data[resp.n_members].taskcount = member->cur_week_task;
			member_data[resp.n_members].jointime = member->join_time;

			PlayerRedisInfo *redis_player = get_redis_player(member->player_id, sg_player_key, sg_redis_client, t1);
			if (redis_player)
			{
				member_data[resp.n_members].name = redis_player->name;
				member_data[resp.n_members].job = redis_player->job;
				member_data[resp.n_members].level = redis_player->lv;
				member_data[resp.n_members].head = redis_player->head_icon;
				member_data[resp.n_members].fc = redis_player->fighting_capacity;
				member_data[resp.n_members].offlinetime = redis_player->status;
			}

			resp.n_members++;
		}
	}

	fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_MEMBER_LIST_ANSWER, guild_member_list_answer__pack, resp);

	return 0;
}

int conn_node_guildsrv::handle_guild_create_request(EXTERN_DATA *extern_data)
{
	GuildCreateRequest *req = guild_create_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

//	uint32_t icon = req->icon;
	std::string name(req->name);
	guild_create_request__free_unpacked(req, NULL);

	int ret = 0;
	GuildPlayer *player = NULL;
	AutoReleaseBatchRedisPlayer t1;
	do
	{
		player = get_guild_player(extern_data->player_id);
		if (player)
		{
			if (player->guild)
			{
				ret = ERROR_ID_GUILD_PLAYER_ALREADY_JOIN;
				LOG_ERR("[%s:%d] player[%lu] already in guild", __FUNCTION__, __LINE__, extern_data->player_id);
				break;
			}

			if (!check_guild_join_cd(player))
			{
				ret = 190500255;
				LOG_ERR("[%s:%d] player[%lu] join cd, exit_time:%u", __FUNCTION__, __LINE__, extern_data->player_id, player->exit_time);
				break;
			}
		}

		if (name.size() == 0)
		{
			ret = 190500227;
			break;
		}
		if (name.size() >= MAX_GUILD_NAME_LEN)
		{
			ret = 190500228;
			break;
		}

		//检查是否同名
		if (check_guild_name(name))
		{
			ret = 190500230;
			LOG_ERR("[%s:%d] player[%lu] guild name exist, name:%s", __FUNCTION__, __LINE__, extern_data->player_id, name.c_str());
			break;
		}

		PlayerRedisInfo *redis_player = get_redis_player(extern_data->player_id, sg_player_key, sg_redis_client, t1);
		if (!redis_player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get redis player failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		uint32_t need_level = sg_guild_create_level;
		uint32_t player_level = redis_player->lv;
		if (player_level < need_level)
		{
			ret = 190500231;
			LOG_ERR("[%s:%d] player[%lu] level not enough, need_level:%u, player_level:%u", __FUNCTION__, __LINE__, extern_data->player_id, need_level, player_level);
			break;
		}

		if (redis_player->zhenying == 0)
		{
			ret = 190500345;
			LOG_ERR("[%s:%d] player[%lu] not join zhenying", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (sg_guild_create_gold == 0)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] config error, coin:%u", __FUNCTION__, __LINE__, extern_data->player_id, sg_guild_create_gold);
			break;
		}
	} while(0);

	if (ret != 0)
	{
		resp_guild_info(&connecter, extern_data, MSG_ID_GUILD_CREATE_ANSWER, ret, player);
	}
	else
	{
		//请求扣除消耗
		PROTO_SRV_CHECK_AND_COST_REQ *cost_req = (PROTO_SRV_CHECK_AND_COST_REQ *)get_send_data();
		uint32_t data_len = sizeof(PROTO_SRV_CHECK_AND_COST_REQ) + get_data_len();
		memset(cost_req, 0, data_len);
		cost_req->cost.statis_id = MAGIC_TYPE_GUILD_CREATE;
		cost_req->cost.gold = sg_guild_create_gold;
		cost_req->data_size = get_data_len();
		memcpy(cost_req->data, get_data(), cost_req->data_size);

		fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_GUILDSRV_COST_REQUEST, data_len, 0);
	}

	return 0;
}

static int handle_guild_create_cost(int data_len, uint8_t *data, int result, EXTERN_DATA *extern_data)
{
	GuildCreateRequest *req = guild_create_request__unpack(NULL, data_len, data);
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}
	
	uint32_t icon = req->icon;
	std::string name(req->name);
	guild_create_request__free_unpacked(req, NULL);

	int ret = result;
	GuildPlayer *player = NULL;
	bool internal = false;
	do
	{
		if (ret != 0)
		{
			if (ret == ERROR_ID_GOLD_NOT_ENOUGH)
			{
				ret = 190500232;
			}
			break;
		}
		internal = true;

		ret = create_guild(extern_data->player_id, icon, name, player);
	} while(0);

	resp_guild_info(&conn_node_guildsrv::connecter, extern_data, MSG_ID_GUILD_CREATE_ANSWER, ret, player);

	return (internal ? ret : 0);
}

struct GuildCmpInfo
{
	GuildInfo *guild;
	uint32_t level;
	uint32_t fc;
};

bool guild_cmp_func(const GuildCmpInfo &l, const GuildCmpInfo &r)
{
	if (l.guild->approve_state < r.guild->approve_state)
	{
		return true;
	}
	else if (l.guild->approve_state > r.guild->approve_state)
	{
		return false;
	}
	else
	{
		if (l.level > r.level)
		{
			return true;
		}
		else if (l.level < r.level)
		{
			return false;
		}
		else
		{
			if (l.fc > r.fc)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	return false;
}

int conn_node_guildsrv::handle_guild_join_request(EXTERN_DATA *extern_data)
{
	GuildJoinRequest *req = guild_join_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint32_t guild_id = req->guildid;
	guild_join_request__free_unpacked(req, NULL);

	int ret = 0;
	std::vector<uint32_t> applyIds;
	AutoReleaseBatchRedisPlayer t1;		
	do
	{
		GuildPlayer *player = get_guild_player(extern_data->player_id);
		if (player)
		{
			if (player->guild)
			{
				ret = ERROR_ID_GUILD_PLAYER_ALREADY_JOIN;
				LOG_ERR("[%s:%d] player[%lu] in guild, guild_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild_id);
				break;
			}

			if (!check_guild_join_cd(player))
			{
				ret = 190500255;
				LOG_ERR("[%s:%d] player[%lu] join cd, guild_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild_id);
				break;
			}
		}

		PlayerRedisInfo *redis_player = get_redis_player(extern_data->player_id, sg_player_key, sg_redis_client, t1);
		if (!redis_player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get redis info failed, guild_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild_id);
			break;
		}

		uint32_t need_level = sg_guild_create_level;
		uint32_t player_level = redis_player->lv;
		if (player_level < need_level)
		{
			ret = 190500231;
			LOG_ERR("[%s:%d] player[%lu] level not enough, need_level:%u, player_level:%u", __FUNCTION__, __LINE__, extern_data->player_id, need_level, player_level);
			break;
		}

		if (guild_id > 0)
		{
			GuildInfo *guild = get_guild(guild_id);
			if (!guild)
			{
				ret = ERROR_ID_GUILD_ID;
				LOG_ERR("[%s:%d] player[%lu] guild not exist, guild_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild_id);
				break;
			}

			if (guild->recruit_state == 0)
			{
				ret = 190500243;
				LOG_ERR("[%s:%d] player[%lu] guild close recruit, guild_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild_id);
				break;
			}

			uint32_t max_member = get_guild_max_member(guild);
			if (guild->member_num >= max_member || guild->member_num >= MAX_GUILD_MEMBER_NUM)
			{
				ret = 190500248;
				LOG_ERR("[%s:%d] player[%lu] guild member max, guild_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild_id);
				break;
			}

			//检查阵营是否符合
			if (redis_player->zhenying == 0)
			{
				ret = 190500347;
				break;
			}
			if (redis_player->zhenying != guild->zhenying)
			{
				ret = 190500346;
				LOG_ERR("[%s:%d] player[%lu] camp not match, guild_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild_id);
				break;
			}

			//无需审核，直接加入帮会
			if (guild->approve_state == 0)
			{
				ret = join_guild(extern_data->player_id, guild);
				break;
			}

			//检查是否已申请
			if (check_player_applied_join(extern_data->player_id, guild_id))
			{
				ret = ERROR_ID_GUILD_PLAYER_APPLIED_JOIN;
				LOG_ERR("[%s:%d] player[%lu] applied join, guild_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild_id);
				break;
			}

			ret = insert_player_join_apply(extern_data->player_id, guild_id);
			if (ret == 0)
			{
				applyIds.push_back(guild_id);
				broadcast_guild_join_apply(guild, extern_data->player_id, redis_player);
			}
		}
		else
		{
			if (redis_player->zhenying == 0)
			{
				ret = 190500347;
				break;
			}

			//全部申请处理
			std::vector<GuildCmpInfo> sort_vec;
			//筛选符合条件的帮会
			std::map<uint32_t, GuildInfo*> &guild_map = get_all_guild();
			for (std::map<uint32_t, GuildInfo*>::iterator iter = guild_map.begin(); iter != guild_map.end(); ++iter)
			{
				GuildInfo *guild = iter->second;
				if (guild->recruit_state == 0)
				{
					continue;
				}

				uint32_t max_member = get_guild_max_member(guild);
				if (guild->member_num >= max_member || guild->member_num >= MAX_GUILD_MEMBER_NUM)
				{
					continue;
				}

				//检查阵营是否符合
				if (redis_player->zhenying != guild->zhenying)
				{
					continue;
				}

				//检查是否已申请
				if (guild->approve_state != 0 && check_player_applied_join(extern_data->player_id, guild_id))
				{
					continue;
				}

				GuildCmpInfo cmp;
				cmp.guild = guild;
				cmp.level = get_guild_level(guild);
				cmp.fc = 0;
				for (uint32_t i = 0; i < guild->member_num; ++i)
				{
					PlayerRedisInfo *tmp_redis_player = get_redis_player(guild->members[i]->player_id, sg_player_key, sg_redis_client, t1);
					if (tmp_redis_player)
					{
						cmp.fc += tmp_redis_player->fighting_capacity;
					}
				}
				sort_vec.push_back(cmp);
			}

			//对符合条件的帮会进行排序
			std::sort(sort_vec.begin(), sort_vec.end(), guild_cmp_func);
			for (std::vector<GuildCmpInfo>::iterator iter = sort_vec.begin(); iter != sort_vec.end(); ++iter)
			{
				GuildCmpInfo *cmp = &(*iter);
				if (cmp->guild->approve_state == 0)
				{
					ret = join_guild(extern_data->player_id, cmp->guild);
					break;
				}
				else
				{
					ret = insert_player_join_apply(extern_data->player_id, cmp->guild->guild_id);
					if (ret == 0)
					{
						applyIds.push_back(cmp->guild->guild_id);
						broadcast_guild_join_apply(cmp->guild, extern_data->player_id, redis_player);
					}
				}
			}
		}
	} while(0);

	GuildJoinAnswer resp;
	guild_join_answer__init(&resp);

	GuildData basic_data;
	guild_data__init(&basic_data);
	GuildPersonalData personal_data;
	guild_personal_data__init(&personal_data);

	GuildPermissionData  permission_data[MAX_GUILD_OFFICE];
	GuildPermissionData* permission_point[MAX_GUILD_OFFICE];
	GuildLogData  usual_log_data[MAX_GUILD_LOG_NUM];
	GuildLogData* usual_log_point[MAX_GUILD_LOG_NUM];
	char*         usual_log_args[MAX_GUILD_LOG_NUM][MAX_GUILD_LOG_ARG_NUM];
	GuildLogData  important_log_data[MAX_GUILD_LOG_NUM];
	GuildLogData* important_log_point[MAX_GUILD_LOG_NUM];
	char*         important_log_args[MAX_GUILD_LOG_NUM][MAX_GUILD_LOG_ARG_NUM];
	GuildBuildingData building_data[MAX_GUILD_BUILDING_NUM];
	GuildBuildingData* building_data_point[MAX_GUILD_BUILDING_NUM];

	resp.result = ret;
	GuildPlayer *player = get_guild_player(extern_data->player_id);
	if (player)
	{
		if (player->guild)
		{
			resp.basicinfo = &basic_data;
			pack_guild_basic_data(player, t1, basic_data, permission_data, permission_point, usual_log_data, usual_log_point, usual_log_args, important_log_data, important_log_point, important_log_args);

			pack_guild_building_data(player, resp.buildings, resp.n_buildings, building_data, building_data_point);
		}

		resp.personalinfo = &personal_data;
		pack_guild_personal_data(player, personal_data);
	}

	if (ret == 0 && applyIds.size() > 0)
	{
		resp.n_applyids = applyIds.size();
		resp.applyids = &applyIds[0];
	}

	fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_JOIN_ANSWER, guild_join_answer__pack, resp);

	return 0;
}

#define MAX_JOIN_LIST_SIZE 100
int conn_node_guildsrv::handle_guild_join_list_request(EXTERN_DATA *extern_data)
{
	int ret = 0;
	std::vector<uint64_t> applyIds;
	AutoReleaseBatchRedisPlayer t1;			
	do
	{
		GuildPlayer *player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not in guild", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

//		if (!player_has_permission(player, GOPT_DEAL_JOIN))
//		{
//			ret = ERROR_ID_GUILD_PLAYER_NO_PERMISSION;
//			LOG_ERR("[%s:%d] player[%lu] no permission, office:%u", __FUNCTION__, __LINE__, extern_data->player_id, player->office);
//			break;
//		}

		ret = get_guild_join_apply(player->guild->guild_id, applyIds);
	} while(0);

	GuildJoinListAnswer resp;
	guild_join_list_answer__init(&resp);

	GuildJoinPlayerData join_data[MAX_JOIN_LIST_SIZE];
	GuildJoinPlayerData* join_data_point[MAX_JOIN_LIST_SIZE];

	resp.result = ret;

	if (ret == 0 && applyIds.size() > 0)
	{
		resp.n_joins = 0;
		resp.joins = join_data_point;
		for (size_t i = 0; i < applyIds.size(); ++i)
		{
			uint64_t player_id = applyIds[i];
			PlayerRedisInfo *redis_player = get_redis_player(player_id, sg_player_key, sg_redis_client, t1);
			if (!redis_player)
			{
				continue;
			}

			join_data_point[resp.n_joins] = &join_data[resp.n_joins];
			guild_join_player_data__init(&join_data[resp.n_joins]);
			join_data[resp.n_joins].playerid = player_id;
			join_data[resp.n_joins].name = redis_player->name;
			join_data[resp.n_joins].job = redis_player->job;
			join_data[resp.n_joins].level = redis_player->lv;
			join_data[resp.n_joins].fc = redis_player->fighting_capacity;
			resp.n_joins++;
		}
	}

	fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_JOIN_LIST_ANSWER, guild_join_list_answer__pack, resp);

	return 0;
}

int conn_node_guildsrv::handle_guild_deal_join_request(EXTERN_DATA *extern_data)
{
	GuildDealJoinRequest *req = guild_deal_join_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint64_t deal_id = req->playerid;
	GuildJoinDealType deal_type = req->deal;
	guild_deal_join_request__free_unpacked(req, NULL);

	int ret = 0;
	std::vector<uint64_t> dealIds;
	uint32_t member_num = 0;
	uint64_t del_id = 0;
	do
	{
		if (deal_type != GUILD_JOIN_DEAL_TYPE__APPROVE_JOIN && deal_type != GUILD_JOIN_DEAL_TYPE__REJECT_JOIN)
		{
			ret = (-1);
			break;
		}

		GuildPlayer *player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not in guild", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (!player_has_permission(player, GOPT_DEAL_JOIN))
		{
			ret = 190500452;
			LOG_ERR("[%s:%d] player[%lu] no permission, office:%u", __FUNCTION__, __LINE__, extern_data->player_id, player->office);
			break;
		}

		GuildInfo *guild = player->guild;
		uint32_t max_member = get_guild_max_member(guild);
		if (deal_id > 0)
		{
			if (deal_type == GUILD_JOIN_DEAL_TYPE__APPROVE_JOIN)
			{
				GuildPlayer *deal_player = get_guild_player(deal_id);
				if (deal_player && deal_player->guild)
				{
					ret = 190500247;
					LOG_ERR("[%s:%d] player[%lu] already in guild, deal_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, deal_id);
					delete_player_join_apply(deal_id);
					break;
				}

				if (!check_player_applied_join(deal_id, guild->guild_id))
				{
					ret = 190500253;
					LOG_ERR("[%s:%d] player[%lu] no this player, deal_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, deal_id);
					break;
				}

				if (guild->member_num >= max_member)
				{
					ret = 190500248;
					LOG_ERR("[%s:%d] player[%lu] guild member max, deal_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, deal_id);
					break;
				}

				ret = join_guild(deal_id, guild);
			}
			else if (deal_type == GUILD_JOIN_DEAL_TYPE__REJECT_JOIN)
			{
				delete_guild_player_join_apply(guild->guild_id, deal_id);
			}
		}
		else
		{
			if (deal_type == GUILD_JOIN_DEAL_TYPE__APPROVE_JOIN)
			{
				std::vector<uint64_t> applyIds;
				get_guild_join_apply(guild->guild_id, applyIds);
				for (std::vector<uint64_t>::iterator iter = applyIds.begin(); iter != applyIds.end(); ++iter)
				{
					uint64_t tmp_id = *iter;
					GuildPlayer *deal_player = get_guild_player(tmp_id);
					if (deal_player && deal_player->guild)
					{
						del_id = tmp_id;
						continue;
					}

					if (guild->member_num >= max_member)
					{
						break;
					}

					join_guild(tmp_id, guild);
					del_id = tmp_id;
				}
			}
			else if (deal_type == GUILD_JOIN_DEAL_TYPE__REJECT_JOIN)
			{
				delete_guild_join_apply(guild->guild_id);
			}
		}

		member_num = guild->member_num;
	} while(0);

	GuildDealJoinAnswer resp;
	guild_deal_join_answer__init(&resp);

	resp.result = ret;
	resp.playerid = deal_id;
	resp.deal = deal_type;
	resp.membernum = member_num;
	resp.delid = del_id;

	fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_DEAL_JOIN_ANSWER, guild_deal_join_answer__pack, resp);

	return 0;
}

int conn_node_guildsrv::handle_guild_turn_switch_request(EXTERN_DATA *extern_data)
{
	GuildTurnSwitchRequest *req = guild_turn_switch_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	GuildSwitchType type = req->type;
	guild_turn_switch_request__free_unpacked(req, NULL);

	int ret = 0;
	uint32_t *pSwitch = NULL;
	do
	{
		if (type != GUILD_SWITCH_TYPE__APPROVE_SWITCH && type != GUILD_SWITCH_TYPE__RECRUIT_SWITCH)
		{
			ret = (-1);
			break;
		}

		GuildPlayer *player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not in guild", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (!player_has_permission(player, GOPT_RECRUIT_SETTING))
		{
			ret = ERROR_ID_GUILD_PLAYER_NO_PERMISSION;
			LOG_ERR("[%s:%d] player[%lu] no permission, office:%u", __FUNCTION__, __LINE__, extern_data->player_id, player->office);
			break;
		}

		GuildInfo *guild = player->guild;
		pSwitch = NULL;
		uint32_t attr_id = 0;
		if (type == GUILD_SWITCH_TYPE__APPROVE_SWITCH)
		{
			pSwitch = &guild->approve_state;
			attr_id = GUILD_ATTR_TYPE__ATTR_APPROVE_STATE;
		}
		else if (type == GUILD_SWITCH_TYPE__RECRUIT_SWITCH)
		{
			pSwitch = &guild->recruit_state;
			attr_id = GUILD_ATTR_TYPE__ATTR_RECRUIT_STATE;
		}

		if (*pSwitch == 0)
		{
			*pSwitch = 1;
		}
		else
		{
			*pSwitch = 0;
		}

		save_guild_switch(guild, type);
		broadcast_guild_attr_update(guild, attr_id, *pSwitch, player->player_id);
	} while(0);

	GuildTurnSwitchAnswer resp;
	guild_turn_switch_answer__init(&resp);

	resp.result = ret;
	resp.type = type;
	if (pSwitch)
	{
		resp.state = *pSwitch;
	}

	fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_TURN_SWITCH_ANSWER, guild_turn_switch_answer__pack, resp);
	return 0;
}

int conn_node_guildsrv::handle_guild_set_words_request(EXTERN_DATA *extern_data)
{
	GuildSetWordsRequest *req = guild_set_words_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	GuildWordsType type = req->type;
	std::string words(req->words);
	guild_set_words_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		if (type != GUILD_WORDS_TYPE__RECRUIT_NOTICE && type != GUILD_WORDS_TYPE__ANNOUNCEMENT)
		{
			ret = (-1);
			break;
		}

		GuildPlayer *player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not in guild", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (words.size() >= MAX_GUILD_ANNOUNCEMENT_LEN)
		{
			ret = -1;
			break;
		}

		uint32_t permission_type = 0;
		if (type == GUILD_WORDS_TYPE__RECRUIT_NOTICE)
		{
			permission_type = GOPT_RECRUIT_SETTING;
		}
		else if (type == GUILD_WORDS_TYPE__ANNOUNCEMENT)
		{
			permission_type = GOPT_ANNOUNCEMENT_SETTING;
		}
		if (!player_has_permission(player, permission_type))
		{
			ret = ERROR_ID_GUILD_PLAYER_NO_PERMISSION;
			LOG_ERR("[%s:%d] player[%lu] no permission, office:%u", __FUNCTION__, __LINE__, extern_data->player_id, player->office);
			break;
		}

		GuildInfo *guild = player->guild;
		char *pWord = NULL;
		uint32_t attr_id = 0;
		if (type == GUILD_WORDS_TYPE__RECRUIT_NOTICE)
		{
			pWord = guild->recruit_notice;
			attr_id = GUILD_STR_ATTR_TYPE__ATTR_RECRUIT_NOTICE;
		}
		else if (type == GUILD_WORDS_TYPE__ANNOUNCEMENT)
		{
			pWord = guild->announcement;
			attr_id = GUILD_STR_ATTR_TYPE__ATTR_ANNOUNCEMENT;
		}

		strcpy(pWord, words.c_str());
		save_announcement(guild, type);
		broadcast_guild_str_attr_update(guild, attr_id, pWord);
	} while(0);

	GuildSetWordsAnswer resp;
	guild_set_words_answer__init(&resp);

	resp.result = ret;
	resp.type = type;

	fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_SET_WORDS_ANSWER, guild_set_words_answer__pack, resp);
	return 0;
}

int conn_node_guildsrv::handle_guild_appoint_office_request(EXTERN_DATA *extern_data)
{
	GuildAppointOfficeRequest *req = guild_appoint_office_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint64_t appoint_id = req->playerid;
	GuildOfficeType office = req->office;
	guild_appoint_office_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		GuildPlayer *player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not in guild", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (appoint_id == extern_data->player_id)
		{
			ret = ERROR_ID_GUILD_CAN_NOT_DO_IT_TO_SELF;
			LOG_ERR("[%s:%d] player[%lu] can't appoint self, appoint_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, appoint_id);
			break;
		}

		GuildPlayer *appointee = get_guild_player(appoint_id);
		if (!appointee || !appointee->guild)
		{
			ret = 190500274;
			LOG_ERR("[%s:%d] player[%lu] appoint not in guild, appoint_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, appoint_id);
			break;
		}

		if (appointee->guild != player->guild)
		{
			ret = 190500247;
			LOG_ERR("[%s:%d] player[%lu] appoint not in same guild, appoint_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, appoint_id);
			break;
		}

		ret = appoint_office(player, appointee, office);
	} while(0);

	GuildAppointOfficeAnswer resp;
	guild_appoint_office_answer__init(&resp);

	resp.result = ret;
	resp.playerid = appoint_id;
	resp.playeroffice = office;

	fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_APPOINT_OFFICE_ANSWER, guild_appoint_office_answer__pack, resp);
	return 0;
}

int conn_node_guildsrv::handle_guild_kick_request(EXTERN_DATA *extern_data)
{
	GuildKickReuqest *req = guild_kick_reuqest__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint64_t kick_id = req->playerid;
	guild_kick_reuqest__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		GuildPlayer *player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not in guild", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		ret = kick_member(player, kick_id);
	} while(0);

	GuildKickAnswer resp;
	guild_kick_answer__init(&resp);

	resp.result = ret;
	resp.playerid = kick_id;

	fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_KICK_ANSWER, guild_kick_answer__pack, resp);
	return 0;
}

int conn_node_guildsrv::handle_guild_rename_request(EXTERN_DATA *extern_data)
{
	GuildRenameRequest *req = guild_rename_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	std::string name(req->name);
	guild_rename_request__free_unpacked(req, NULL);

	int ret = 0;
	GuildInfo *guild = NULL;
	do
	{
		GuildPlayer *player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not in guild", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		guild = player->guild;
		if (!player_has_permission(player, GOPT_RENAME))
		{
			ret = ERROR_ID_GUILD_PLAYER_NO_PERMISSION;
			LOG_ERR("[%s:%d] player[%lu] no permission, office:%u", __FUNCTION__, __LINE__, extern_data->player_id, player->office);
			break;
		}

		uint32_t now = time_helper::get_cached_time() / 1000;
		if (now < player->guild->rename_time + sg_guild_rename_cd)
		{
			ret = 190500245;
			LOG_ERR("[%s:%d] player[%lu] rename cd, guild_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, player->guild->guild_id);
			break;
		}

		if (name.size() == 0)
		{
			ret = 190500227;
			break;
		}
		if (name.size() >= MAX_GUILD_NAME_LEN)
		{
			ret = 190500228;
			break;
		}

		//检查是否同名
		if (check_guild_name(name))
		{
			ret = 190500230;
			LOG_ERR("[%s:%d] player[%lu] guild name exist, name:%s", __FUNCTION__, __LINE__, extern_data->player_id, name.c_str());
			break;
		}

		if (sg_guild_rename_item_id == 0 || sg_guild_rename_item_num == 0)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] config error, item_id:%u, item_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, sg_guild_rename_item_id, sg_guild_rename_item_num);
			break;
		}
	} while(0);

	if (ret == 0)
	{
		//请求扣除消耗
		PROTO_SRV_CHECK_AND_COST_REQ *cost_req = (PROTO_SRV_CHECK_AND_COST_REQ *)get_send_data();
		uint32_t data_len = sizeof(PROTO_SRV_CHECK_AND_COST_REQ) + get_data_len();
		memset(cost_req, 0, data_len);
		cost_req->cost.statis_id = MAGIC_TYPE_GUILD_RENAME;
		cost_req->cost.item_id[0] = sg_guild_rename_item_id;
		cost_req->cost.item_num[0] = sg_guild_rename_item_num;
		cost_req->data_size = get_data_len();
		memcpy(cost_req->data, get_data(), cost_req->data_size);
		fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_GUILDSRV_COST_REQUEST, data_len, 0);
	}
	else
	{
		GuildRenameAnswer resp;
		guild_rename_answer__init(&resp);

		resp.result = ret;
		resp.name = const_cast<char*>(name.c_str());
		if (guild)
		{
			resp.renametime = guild->rename_time;
		}

		fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_RENAME_ANSWER, guild_rename_answer__pack, resp);
	}

	return 0;
}

static int handle_guild_rename_cost(int data_len, uint8_t *data, int result, EXTERN_DATA *extern_data)
{
	GuildRenameRequest *req = guild_rename_request__unpack(NULL, data_len, data);
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	std::string name(req->name);
	guild_rename_request__free_unpacked(req, NULL);

	int ret = result;
	GuildInfo *guild = NULL;
	bool internal = false;
	AutoReleaseBatchRedisPlayer t1;				
	do
	{
		if (ret != 0)
		{
			if (ret == ERROR_ID_PROP_NOT_ENOUGH)
			{
				ret = 190500244;
			}
			break;
		}
		internal = true;

		GuildPlayer *player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not in guild", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (name.size() >= MAX_GUILD_NAME_LEN)
		{
			ret = -1;
			break;
		}

		guild = player->guild;
		strcpy(player->guild->name, name.c_str());
		save_guild_name(player->guild);
		uint32_t now = time_helper::get_cached_time() / 1000;
		player->guild->rename_time = now;

		PlayerRedisInfo *redis_player = get_redis_player(player->player_id, sg_player_key, sg_redis_client, t1);
		if (redis_player)
		{
			GuildLog *log = get_important_insert_log(guild);
			log->type = GILT_RENAME;
			log->time = now;
			snprintf(log->args[0], MAX_GUILD_LOG_ARG_LEN, "%s", redis_player->name);
			snprintf(log->args[1], MAX_GUILD_LOG_ARG_LEN, "%s", player->guild->name);
			broadcast_important_log_add(guild, log);
		}

		save_guild_info(player->guild);

		sync_guild_rename_to_gamesrv(guild);
		refresh_guild_redis_info(guild);
		std::vector<uint64_t> broadcast_ids;
		for (uint32_t i = 0; i < guild->member_num; ++i)
		{
			GuildPlayer *member = guild->members[i];
			//更新所有玩家的redis数据
			update_redis_player_guild(member);
			//同步所有在线玩家数据到game_srv
			if (member->player_id == player->player_id)
			{
				//sync_guild_info_to_gamesrv(member);
			}
			else
			{
				PlayerRedisInfo *redis_member = get_redis_player(member->player_id, sg_player_key, sg_redis_client, t1);
				if (redis_member)
				{
					if (redis_member->status == 0)
					{
						//sync_guild_info_to_gamesrv(member);
						broadcast_ids.push_back(member->player_id);
					}
				}
			}
		}

		//向所有在线玩家广播帮会名字
		if (broadcast_ids.size() > 0)
		{
			broadcast_guild_str_attr_update(GUILD_STR_ATTR_TYPE__ATTR_GUILD_NAME, guild->name, broadcast_ids);
		}
	} while(0);

	GuildRenameAnswer resp;
	guild_rename_answer__init(&resp);

	resp.result = ret;
	resp.name = const_cast<char*>(name.c_str());
	if (guild)
	{
		resp.renametime = guild->rename_time;
	}

	fast_send_msg(&conn_node_guildsrv::connecter, extern_data, MSG_ID_GUILD_RENAME_ANSWER, guild_rename_answer__pack, resp);

	return (internal ? ret : 0);
}

int conn_node_guildsrv::handle_guild_exit_request(EXTERN_DATA *extern_data)
{
	int ret = 0;
	GuildPlayer *player = NULL;
	do
	{
		player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not in guild", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (player->office == (uint32_t)GUILD_OFFICE_TYPE__OFFICE_MASTER)
		{
			ret = ERROR_ID_GUILD_MASTER_CANNOT_EXIT;
			LOG_ERR("[%s:%d] player[%lu] is master office[%u]", __FUNCTION__, __LINE__, extern_data->player_id, player->office);
			break;
		}

		ret = exit_guild(player);
	} while(0);

	GuildExitAnswer resp;
	guild_exit_answer__init(&resp);

	resp.result = ret;
	if (player)
	{
		resp.exittime = player->exit_time;
	}

	fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_EXIT_ANSWER, guild_exit_answer__pack, resp);

	return 0;
}

int conn_node_guildsrv::handle_guild_set_permission_request(EXTERN_DATA *extern_data)
{
	GuildSetPermissionRequest *req = guild_set_permission_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint32_t office = req->office;
	uint32_t type = req->type;

	guild_set_permission_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		GuildPlayer *player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not in guild", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		GuildInfo *guild = player->guild;
		if (player->player_id != guild->master_id)
		{
			ret = ERROR_ID_GUILD_PLAYER_NO_PERMISSION;
			LOG_ERR("[%s:%d] player[%lu] has not permission", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (office == GUILD_OFFICE_TYPE__OFFICE_MASTER)
		{
			ret = 1;
			LOG_ERR("[%s:%d] player[%lu] can not change master permission", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		GuildPermission *pOffice = NULL;
		for (int i = 0; i < MAX_GUILD_OFFICE; ++i)
		{
			if (guild->permissions[i].office == office)
			{
				pOffice = &guild->permissions[i];
				break;
			}
		}

		if (!pOffice)
		{
			ret = 1;
			LOG_ERR("[%s:%d] player[%lu] office error, office:%u", __FUNCTION__, __LINE__, extern_data->player_id, office);
			break;
		}

		if (type == 0 || type >= GOPT_END)
		{
			ret = 1;
			LOG_ERR("[%s:%d] player[%lu] type error, office:%u, type:%u", __FUNCTION__, __LINE__, extern_data->player_id, office, type);
			break;
		}

		if (pOffice->permission[type] == 0)
		{
			pOffice->permission[type] = 1;
		}
		else
		{
			pOffice->permission[type] = 0;
		}

		broadcast_permission_update(guild, office, type, pOffice->permission[type]);
		save_guild_info(guild);
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;

	fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_SET_PERMISSION_ANSWER, comm_answer__pack, resp);
	return 0;
}

int conn_node_guildsrv::handle_guild_invite_request(EXTERN_DATA *extern_data)
{
	GuildInviteRequest *req = guild_invite_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint64_t invitee_id = req->inviteeid;
	guild_invite_request__free_unpacked(req, NULL);

	int ret = 0;
	uint32_t error_arg = 0;
	AutoReleaseBatchRedisPlayer t1;		
	do
	{
		GuildPlayer *player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = 190500453;
			LOG_ERR("[%s:%d] player[%lu] not in guild", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		PlayerRedisInfo *redis_player = get_redis_player(extern_data->player_id, sg_player_key, sg_redis_client, t1);
		if (!redis_player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get redis info failed, invitee_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, invitee_id);
			break;
		}

		GuildInfo *guild = player->guild;
		if (!player_has_permission(player, GOPT_INVITATION))
		{
			ret = 190500453;
			LOG_ERR("[%s:%d] player[%lu] no permission, office:%u", __FUNCTION__, __LINE__, extern_data->player_id, player->office);
			break;
		}

		uint32_t invite_cd = get_invite_cd_time(player->player_id, invitee_id, guild->guild_id);
		if (invite_cd > 0)
		{
			ret = 190500314;
			error_arg = invite_cd;
			LOG_ERR("[%s:%d] player[%lu] in cd, invitee_id:%lu, guild_id:%u, cd:%u", __FUNCTION__, __LINE__, extern_data->player_id, invitee_id, guild->guild_id, invite_cd);
			break;
		}

		uint32_t max_member = get_guild_max_member(guild);
		if (guild->member_num >= max_member || guild->member_num >= MAX_GUILD_MEMBER_NUM)
		{
			ret = 190500248;
			LOG_ERR("[%s:%d] player[%lu] guild member max, guild_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id);
			break;
		}

		PlayerRedisInfo *redis_invitee = get_redis_player(invitee_id, sg_player_key, sg_redis_client, t1);
		if (!redis_invitee)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get invitee redis info failed, invitee_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, invitee_id);
			break;
		}
		//检查阵营是否符合
		if (redis_invitee->zhenying != guild->zhenying)
		{
			ret = 190500464;
			LOG_ERR("[%s:%d] player[%lu] invitee camp not match, invitee_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, invitee_id);
			break;
		}

		uint32_t need_level = sg_guild_create_level;
		uint32_t invitee_level = redis_invitee->lv;
		if (invitee_level < need_level)
		{
			ret = 190500465;
			LOG_ERR("[%s:%d] player[%lu] invitee level not enough, need_level:%u, invitee_level:%u", __FUNCTION__, __LINE__, extern_data->player_id, need_level, invitee_level);
			break;
		}

		GuildPlayer *invitee = get_guild_player(invitee_id);
		if (invitee)
		{
			if (!check_guild_join_cd(invitee))
			{
				ret = 190500466;
				error_arg = invitee->exit_time + sg_guild_join_cd;
				LOG_ERR("[%s:%d] player[%lu] invitee join cd, invitee_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, invitee_id);
				break;
			}

			if (invitee->guild)
			{
				ret = 190500247;
				break;
			}
		}

		if (redis_invitee->status != 0)
		{
			ret = 190500034;
			break;
		}

		//通知被邀请者
		{
			GuildInviteNotify nty;
			guild_invite_notify__init(&nty);

			nty.inviterid = player->player_id;
			nty.invitername = redis_player->name;
			nty.guildid = guild->guild_id;
			nty.guildname = guild->name;

			EXTERN_DATA ext_data;
			ext_data.player_id = invitee_id;
			fast_send_msg(&connecter, &ext_data, MSG_ID_GUILD_INVITE_NOTIFY, guild_invite_notify__pack, nty);
		}

		insert_one_invite(player->player_id, invitee_id, guild->guild_id);
	} while(0);

	GuildInviteAnswer resp;
	guild_invite_answer__init(&resp);

	resp.result = ret;
	resp.errorarg = error_arg;
	resp.inviteeid = invitee_id;

	fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_INVITE_ANSWER, guild_invite_answer__pack, resp);

	return 0;
}

int conn_node_guildsrv::handle_guild_deal_invite_request(EXTERN_DATA *extern_data)
{
	GuildDealInviteRequest *req = guild_deal_invite_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint64_t inviter_id = req->inviterid;
	uint32_t guild_id = req->guildid;
	uint32_t deal_type = req->deal;
	guild_deal_invite_request__free_unpacked(req, NULL);

	int ret = 0;
	AutoReleaseBatchRedisPlayer t1;		
	do
	{
		if (deal_type != 1 && deal_type != 2)
		{
			ret = (-1);
			break;
		}

		PlayerRedisInfo *redis_player = get_redis_player(extern_data->player_id, sg_player_key, sg_redis_client, t1);
		if (!redis_player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get redis info failed, inviter_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, inviter_id);
			break;
		}

		//拒绝
		if (deal_type == 2)
		{
			GuildPlayer *inviter = get_guild_player(inviter_id);
			if (inviter && inviter->guild && inviter->guild->guild_id == guild_id)
			{ //通知邀请者
				std::vector<char *> args;
				args.push_back(redis_player->name);
				conn_node_guildsrv::send_system_notice(inviter_id, 190500032, &args);
			}

			mark_invite_deal(inviter_id, extern_data->player_id, guild_id, deal_type);
			break;
		}

		GuildPlayer *player = get_guild_player(extern_data->player_id);
		if (player)
		{
			if (player->guild)
			{
				ret = 190500470;
				LOG_ERR("[%s:%d] player[%lu] in guild, guild_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild_id);
				break;
			}

			if (!check_guild_join_cd(player))
			{
				ret = 190500255;
				LOG_ERR("[%s:%d] player[%lu] join cd, guild_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild_id);
				break;
			}
		}

		GuildInfo *guild = get_guild(guild_id);
		if (!guild)
		{
			ret = 190500462;
			LOG_ERR("[%s:%d] player[%lu] guild dismiss, guild_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild_id);
			break;
		}

		uint32_t max_member = get_guild_max_member(guild);
		if (guild->member_num >= max_member || guild->member_num >= MAX_GUILD_MEMBER_NUM)
		{
			ret = 190500467;
			LOG_ERR("[%s:%d] player[%lu] guild member max, guild_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id);
			break;
		}

		uint32_t invite_cd = get_invite_cd_time(inviter_id, extern_data->player_id, guild_id);
		if (invite_cd == 0)
		{
			ret = 999;
			LOG_ERR("[%s:%d] player[%lu] invite expire, inviter_id:%lu, guild_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, inviter_id, guild_id);
			break;
		}

		if (check_invite_is_deal(inviter_id, extern_data->player_id, guild_id))
		{
			ret = 998;
			LOG_ERR("[%s:%d] player[%lu] invite has dealed, inviter_id:%lu, guild_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, inviter_id, guild_id);
			break;
		}

		uint32_t need_level = sg_guild_create_level;
		uint32_t player_level = redis_player->lv;
		if (player_level < need_level)
		{
			ret = 190500231;
			LOG_ERR("[%s:%d] player[%lu] level not enough, need_level:%u, player_level:%u", __FUNCTION__, __LINE__, extern_data->player_id, need_level, player_level);
			break;
		}

		//检查阵营是否符合
		if (redis_player->zhenying != guild->zhenying)
		{
			ret = 190500464;
			LOG_ERR("[%s:%d] player[%lu] camp not match, inviter_id:%lu, guild_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, inviter_id, guild_id);
			break;
		}

		ret = join_guild(extern_data->player_id, guild);
		if (ret != 0)
		{
			break;
		}

		mark_invite_deal(inviter_id, extern_data->player_id, guild_id, deal_type);
	} while(0);

	GuildDealInviteAnswer resp;
	guild_deal_invite_answer__init(&resp);

	resp.result = ret;
	resp.guildid = guild_id;

	fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_DEAL_INVITE_ANSWER, guild_deal_invite_answer__pack, resp);

	return 0;
}

int conn_node_guildsrv::handle_open_guild_answer_request(EXTERN_DATA *extern_data)
{
	int ret = 0;
	GuildPlayer *player = NULL;
	do
	{
		player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not in guild", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (player->office != (uint32_t)GUILD_OFFICE_TYPE__OFFICE_MASTER)
		{
			ret = ERROR_ID_GUILD_MASTER_CANNOT_EXIT;
			LOG_ERR("[%s:%d] player[%lu] is master office[%u]", __FUNCTION__, __LINE__, extern_data->player_id, player->office);
			break;
		}
	} while (0);

	CommAnswer resp;
	comm_answer__init(&resp);
	resp.result = ret;
	if (ret == 0)
	{
		//uint32_t question[GuildAnswer::MAX_SEND_GUILD_QUESTION];
		//for (int i = 0; i < GuildAnswer::MAX_SEND_GUILD_QUESTION; ++i)
		//{
		//	question[i] = sg_guild_question[rand() % sg_guild_question.size()];
		//}
		//broadcast_guild_message(player->guild, MSG_ID_FACTION_QUESTION_OPEN_NOTIFY, &resp, (pack_func)comm_answer__pack);
		ParameterTable *table = get_config_by_id(161000410, &parameter_config);
		if (table != NULL)
		{
			if (player->guild->treasure < table->parameter1[0])
			{
				ret = 190500484;
			}
			else
			{
				sub_guild_treasure(player->guild, table->parameter1[0], true);
				player->guild->answer.Start(player->guild);//, question, GuildAnswer::MAX_SEND_GUILD_QUESTION);
			}
		}

	}
	fast_send_msg(&connecter, extern_data, MSG_ID_OPEN_FACTION_QUESTION_ANSWER, comm_answer__pack, resp);

	return 0;
}


int conn_node_guildsrv::handle_player_online_notify(EXTERN_DATA *extern_data)
{
	PLAYER_ONLINE_NOTIFY *proto = (PLAYER_ONLINE_NOTIFY*)get_data();
	GuildPlayer *player = get_guild_player(extern_data->player_id);
	AutoReleaseBatchRedisPlayer t1;
	if (player)
	{
		player->cur_task_id = proto->cur_guild_build_task;
		sync_guild_info_to_gamesrv(player);
		sync_guild_skill_to_gamesrv(player);
		sync_player_donation_to_game_srv(player);
		sync_guild_task_to_gamesrv(player);
		{
			uint32_t *pData = (uint32_t *)get_send_data();
			uint32_t data_len = sizeof(uint32_t);
			*pData = get_guild_land_active_reward_count(player, GUILD_INTRUSION_CONTROLTABLE_ID);
			fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_GUILD_RUQIN_SYNC_COUNT, data_len, 0);
		}
		//聊天发送
		do
		{
			if (!player->guild)
			{
				break;
			}

			GuildInfo *guild = player->guild;
			if (player->player_id != guild->master_id)
			{
				break;
			}

			ParameterTable *param_config = get_config_by_id(161000149, &parameter_config);
			if (!param_config)
			{
				break;
			}
			PlayerRedisInfo *redis_player = get_redis_player(player->player_id, sg_player_key, sg_redis_client, t1);
			if (!redis_player)
			{
				break;
			}

			char content[1024];
			sprintf(content, param_config->parameter2, redis_player->name);

			Chat req;
			chat__init(&req);
			req.channel = CHANNEL__family;
			req.contain = content;

			broadcast_guild_chat(guild, &req);
		} while(0);

		if (player->guild)
		{
			player->guild->answer.OnPlayerLogin(player, extern_data);
		}
	}

	resp_guild_info(&connecter, extern_data, MSG_ID_GUILD_INFO_ANSWER, 0, player);

	return 0;
}

int conn_node_guildsrv::handle_check_and_cost_answer(EXTERN_DATA *extern_data)
{
	PROTO_SRV_CHECK_AND_COST_RES *res = (PROTO_SRV_CHECK_AND_COST_RES*)get_data();
	int ret = 0;
	switch(res->cost.statis_id)
	{
		case MAGIC_TYPE_GUILD_CREATE:
			ret = handle_guild_create_cost(res->data_size, res->data, res->result, extern_data);
			break;
		case MAGIC_TYPE_GUILD_RENAME:
			ret = handle_guild_rename_cost(res->data_size, res->data, res->result, extern_data);
			break;
		case MAGIC_TYPE_GUILD_SKILL_PRACTICE:
			ret = handle_guild_skill_practice_cost(res->data_size, res->data, res->result, extern_data);
			break;
		case MAGIC_TYPE_GUILD_DONATE:
			ret = handle_guild_donate_cost(res->data_size, res->data, res->result, extern_data);
			break;
	}

	if (ret != 0)
	{
		PROTO_UNDO_COST *proto = (PROTO_UNDO_COST*)get_send_data();
		uint32_t data_len = sizeof(PROTO_UNDO_COST);
		memset(proto, 0, data_len);
		memcpy(&proto->cost, &res->cost, sizeof(SRV_COST_INFO));
		fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_UNDO_COST, data_len, 0);
	}

	return 0;
}

int conn_node_guildsrv::handle_gamesrv_reward_answer(EXTERN_DATA *extern_data)
{
	PROTO_GUILDSRV_REWARD_RES *res = (PROTO_GUILDSRV_REWARD_RES*)buf_head();
	switch(res->statis_id)
	{
		case MAGIC_TYPE_SHOP_BUY:
			handle_shop_buy_answer(res->data_size, res->data, res->result, extern_data);
			break;
	}

	return 0;
}

int conn_node_guildsrv::handle_add_guild_resource_request(EXTERN_DATA *extern_data)
{
	PROTO_HEAD *head = get_head();
	uint32_t *pData = (uint32_t *)head->data;
	uint32_t type = *(pData++);
	uint32_t num = *(pData++);

	GuildPlayer *player = get_guild_player(extern_data->player_id);
	if (!player || !player->guild)
	{
		return -1;
	}

	switch (type)
	{
		case 1:
			add_guild_popularity(player->guild, num);
			break;
		case 2:
			add_guild_treasure(player->guild, num);
			add_player_contribute_treasure(player, num);
			break;
		case 3:
			add_guild_build_board(player->guild, num);
			break;
		case 4:
			add_player_donation(player, num);
			break;
	}
	return 0;
}

int conn_node_guildsrv::handle_disband_request(EXTERN_DATA *extern_data)
{
	PROTO_HEAD *head = get_head();
	uint32_t *pData = (uint32_t *)head->data;
	uint32_t guild_id = *(pData++);
	GuildInfo *guild = get_guild(guild_id);
	if (guild)
	{
		disband_guild(guild);
	}
	return 0;
}

int conn_node_guildsrv::handle_gamesrv_start(EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d] game_srv start notify.", __FUNCTION__, __LINE__);

	sync_all_guild_to_gamesrv();
	return 0;
}


int conn_node_guildsrv::handle_guild_building_info_request(EXTERN_DATA *extern_data)
{
	int ret = 0;
	GuildPlayer *player = NULL;
	do
	{
		player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not join guild yet", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
	} while(0);

	GuildBuildingInfoAnswer resp;
	guild_building_info_answer__init(&resp);

	GuildBuildingData building_data[MAX_GUILD_BUILDING_NUM];
	GuildBuildingData* building_data_point[MAX_GUILD_BUILDING_NUM];

	resp.result = ret; 
	if (player && player->guild)
	{
		resp.buildings = building_data_point;
		resp.n_buildings = 0;
		for (int i = 0; i < MAX_GUILD_BUILDING_NUM; ++i)
		{
			building_data_point[resp.n_buildings] = &building_data[resp.n_buildings];
			guild_building_data__init(&building_data[resp.n_buildings]);
			building_data[resp.n_buildings].buildingid = i + 1;
			building_data[resp.n_buildings].level = player->guild->buildings[i].level;
			resp.n_buildings++;
		}

		resp.upgradeid = player->guild->building_upgrade_id;
		resp.upgradeend = player->guild->building_upgrade_end;
	}

	fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_BUILDING_INFO_ANSWER, guild_building_info_answer__pack, resp);

	return 0;
}

int conn_node_guildsrv::handle_guild_building_upgrade_request(EXTERN_DATA *extern_data)
{
	GuildBuildingUpgradeRequest *req = guild_building_upgrade_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint32_t building_id = req->buildingid;
	guild_building_upgrade_request__free_unpacked(req, NULL);

	int ret = 0;
	GuildPlayer *player = NULL;
	do
	{
		player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not join guild yet", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		GuildInfo *guild = player->guild;
		if (guild->building_upgrade_id > 0)
		{
			ret = 190500250;
			LOG_ERR("[%s:%d] player[%lu] guild has building upgrade, upgrade_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->building_upgrade_id);
			break;
		}

		uint32_t building_level = get_building_level(guild, building_id);
		if (get_guild_building_config(building_id, building_level + 1) == NULL)
		{
			ret = ERROR_ID_GUILD_BUILDING_LEVEL_MAX;
			LOG_ERR("[%s:%d] player[%lu] guild building level max, building_id:%u, level:%u", __FUNCTION__, __LINE__, extern_data->player_id, building_id, building_level);
			break;
		}

		GangsTable *config = get_guild_building_config(building_id, building_level);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] guild building config failed, building_id:%u, level:%u", __FUNCTION__, __LINE__, extern_data->player_id, building_id, building_level);
			break;
		}

		if (player->office > (uint32_t)config->LeveCompetence)
		{
			ret = ERROR_ID_GUILD_PLAYER_NO_PERMISSION;
			LOG_ERR("[%s:%d] player[%lu] has not permission, building_id:%u, level:%u", __FUNCTION__, __LINE__, extern_data->player_id, building_id, building_level);
			break;
		}

		uint32_t hall_level = get_building_level(guild, Building_Hall);
		uint32_t need_hall_level = config->HallLeve;
		if (hall_level < need_hall_level)
		{
			ret = 190500239;
			LOG_ERR("[%s:%d] player[%lu] hall level not enough, building_id:%u, level:%u, hall_level:%u, need_hall_level:%u", __FUNCTION__, __LINE__, extern_data->player_id, building_id, building_level, hall_level, need_hall_level);
			break;
		}

		uint32_t need_treasure = config->Leve1Expend;
		if (guild->treasure < need_treasure)
		{
			ret = 190500238;
			LOG_ERR("[%s:%d] player[%lu] guild treasure not enough, building_id:%u, level:%u, has_treasure:%u, need_treasure:%u", __FUNCTION__, __LINE__, extern_data->player_id, building_id, building_level, guild->treasure, need_treasure);
			break;
		}

		uint32_t need_board = config->Leve2Expend;
		if (guild->build_board < need_board)
		{
			ret = 190500238;
			LOG_ERR("[%s:%d] player[%lu] guild board not enough, building_id:%u, level:%u, has_board:%u, need_board:%u", __FUNCTION__, __LINE__, extern_data->player_id, building_id, building_level, guild->build_board, need_board);
			break;
		}

		sub_guild_treasure(guild, need_treasure, false);
		sub_guild_build_board(guild, need_board, false);

		guild->building_upgrade_id = building_id;
		guild->building_upgrade_end = time_helper::get_cached_time() / 1000 + config->LeveTime;
		save_guild_info(guild);
	} while(0);

	GuildBuildingUpgradeAnswer resp;
	guild_building_upgrade_answer__init(&resp);

	resp.result = ret; 
	if (player && player->guild)
	{
		resp.upgradeid = player->guild->building_upgrade_id;
		resp.upgradeend = player->guild->building_upgrade_end;
	}

	fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_BUILDING_UPGRADE_ANSWER, guild_building_upgrade_answer__pack, resp);
	return 0;
}

int conn_node_guildsrv::handle_sub_guild_building_time_request(EXTERN_DATA *extern_data)
{
	PROTO_HEAD *head = get_head();
	uint32_t *pData = (uint32_t *)head->data;
	uint32_t time = *(pData++);

	GuildPlayer *player = get_guild_player(extern_data->player_id);
	if (!player || !player->guild)
	{
		return -1;
	}

	sub_building_upgrade_time(player->guild, time);

	return 0;
}

int conn_node_guildsrv::handle_guild_accept_task_request(EXTERN_DATA *extern_data)
{
	int ret = 0;
	GuildPlayer *player = NULL;
	do
	{
		player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not join guild yet", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		GangsBuildTaskTable *config = get_config_by_id(player->cur_week_task_config_id, &guild_build_task_config);
		if (!config || config->n_Tasklibrary == 0)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get task config failed, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, player->cur_week_task_config_id);
			break;
		}

		uint32_t total_count = config->Times;
		if (player->cur_week_task >= total_count)
		{
			ret = 190500444;
			LOG_ERR("[%s:%d] player[%lu] already finish all, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, player->cur_week_task_config_id);
			break;
		}

		uint32_t task_idx = rand() % config->n_Tasklibrary;
		uint32_t task_id = config->Tasklibrary[task_idx];

		{
			uint32_t *pData = (uint32_t *)get_send_data();
			uint32_t data_len = sizeof(uint32_t);
			memset(pData, 0, data_len);
			*pData = task_id;
			fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_GUILD_ACCEPT_TASK_REQUEST, data_len, 0);
		}
	} while(0);

	if (ret != 0)
	{
		CommAnswer resp;
		comm_answer__init(&resp);

		resp.result = ret;
		fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_ACCEPT_TASK_ANSWER, comm_answer__pack, resp);
	}

	return 0;
}

int conn_node_guildsrv::handle_game_accept_task_answer(EXTERN_DATA *extern_data)
{
	GUILD_ACCEPT_TASK_ANSWER *ans = (GUILD_ACCEPT_TASK_ANSWER*)get_data();

	int ret = 0;
	GuildPlayer *player = NULL;
	do
	{
		if (ans->result != 0)
		{
			ret = ans->result;
			break;
		}

		player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not join guild yet", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		player->cur_task_id = ans->task_id;
		notify_guild_attr_update(player->player_id, GUILD_ATTR_TYPE__ATTR_CUR_TASK, player->cur_task_id);
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;
	fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_ACCEPT_TASK_ANSWER, comm_answer__pack, resp);

	return 0;
}

int conn_node_guildsrv::handle_game_task_finish_notify(EXTERN_DATA *extern_data)
{
	GuildPlayer *player = NULL;
	do
	{
		player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			LOG_ERR("[%s:%d] player[%lu] not join guild yet", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		GangsBuildTaskTable *config = get_config_by_id(player->cur_week_task_config_id, &guild_build_task_config);
		if (!config)
		{
			LOG_ERR("[%s:%d] player[%lu] get task config failed, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, player->cur_week_task_config_id);
			break;
		}

		sub_building_upgrade_time(player->guild, config->LeveTime);
		add_guild_popularity(player->guild, sg_guild_task_popularity);

		player->cur_week_task++;
		player->cur_task_id = 0;
		save_guild_player(player);
		sync_guild_task_to_gamesrv(player);
		{
			AttrMap attrs;
			attrs[GUILD_ATTR_TYPE__ATTR_TASK_COUNT] = player->cur_week_task;
			attrs[GUILD_ATTR_TYPE__ATTR_CUR_TASK] = player->cur_task_id;
			notify_guild_attrs_update(player->player_id, attrs);
		}
	} while(0);

	return 0;
}

int conn_node_guildsrv::handle_guild_skill_info_request(EXTERN_DATA *extern_data)
{
	int ret = 0;
	GuildPlayer *player = NULL;
	do
	{
		player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not join guild yet", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
	} while(0);

	GuildSkillInfoAnswer resp;
	guild_skill_info_answer__init(&resp);

	GuildSkillData develop_data[MAX_GUILD_SKILL_NUM];
	GuildSkillData* develop_data_point[MAX_GUILD_SKILL_NUM];
	GuildSkillData practice_data[MAX_GUILD_SKILL_NUM];
	GuildSkillData* practice_data_point[MAX_GUILD_SKILL_NUM];

	resp.result = ret; 
	if (player)
	{
		if (player->guild)
		{
			resp.develops = develop_data_point;
			resp.n_develops = 0;
			for (int i = 0; i < MAX_GUILD_SKILL_NUM; ++i)
			{
				if (player->guild->skills[i].skill_id == 0)
				{
					break;
				}

				develop_data_point[resp.n_develops] = &develop_data[resp.n_develops];
				guild_skill_data__init(&develop_data[resp.n_develops]);
				develop_data[resp.n_develops].skillid = player->guild->skills[i].skill_id;
				develop_data[resp.n_develops].level = player->guild->skills[i].skill_lv;
				resp.n_develops++;
			}
		}

		resp.practices = practice_data_point;
		resp.n_practices = 0;
		for (int i = 0; i < MAX_GUILD_SKILL_NUM; ++i)
		{
			if (player->skills[i].skill_id == 0)
			{
				break;
			}

			practice_data_point[resp.n_practices] = &practice_data[resp.n_practices];
			guild_skill_data__init(&practice_data[resp.n_practices]);
			practice_data[resp.n_practices].skillid = player->skills[i].skill_id;
			practice_data[resp.n_practices].level = player->skills[i].skill_lv;
			resp.n_practices++;
		}
	}

	fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_SKILL_INFO_ANSWER, guild_skill_info_answer__pack, resp);

	return 0;
}

int conn_node_guildsrv::handle_guild_skill_develop_request(EXTERN_DATA *extern_data)
{
	GuildSkillUpgradeRequest *req = guild_skill_upgrade_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint32_t skill_id = req->skillid;
	guild_skill_upgrade_request__free_unpacked(req, NULL);

	int ret = 0;
	GuildPlayer *player = NULL;
	GuildSkill *pSkill = NULL;
	do
	{
		player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not join guild yet", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		GuildInfo *guild = player->guild;
		if (!player_has_permission(player, GOPT_DEVELOP_SKILL))
		{
			ret = ERROR_ID_GUILD_PLAYER_NO_PERMISSION;
			LOG_ERR("[%s:%d] player[%lu] no permission, office:%u", __FUNCTION__, __LINE__, extern_data->player_id, player->office);
			break;
		}

		pSkill = get_guild_skill_info(guild, skill_id);
		if (!pSkill)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] guild skill memory exhaust, guild_id:%u, skill_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id, skill_id);
			break;
		}

		uint32_t next_lv = pSkill->skill_lv;
		if (get_guild_skill_config(skill_id, pSkill->skill_lv + 1) == NULL)
		{
			ret = ERROR_ID_GUILD_SKILL_LEVEL_MAX;
			LOG_ERR("[%s:%d] player[%lu] get skill config failed, guild_id:%u, skill_id:%u, skill_lv:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id, skill_id, pSkill->skill_lv + 1);
			break;
		}

		GangsSkillTable *config = get_guild_skill_config(skill_id, next_lv);
		if (!config)
		{
			ret = ERROR_ID_GUILD_SKILL_LEVEL_MAX;
			LOG_ERR("[%s:%d] player[%lu] get skill config failed, guild_id:%u, skill_id:%u, skill_lv:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id, skill_id, next_lv);
			break;
		}

		uint32_t need_lib_lv = config->BuildingLeve;
		uint32_t library_level = get_building_level(guild, Building_Library);
		if (library_level < need_lib_lv)
		{
			ret = 190500237;
			LOG_ERR("[%s:%d] player[%lu] guild library level, guild_id:%u, skill_id:%u, skill_lv:%u, need_lv:%u, lib_lv:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id, skill_id, next_lv, need_lib_lv, library_level);
			break;
		}

		uint32_t need_treasure = config->CreateMnoney;
		if (guild->treasure < need_treasure)
		{
			ret = 190500236;
			LOG_ERR("[%s:%d] player[%lu] guild treasure not enough, guild_id:%u, skill_id:%u, skill_id:%u, has_treasure:%u, need_treasure:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id, skill_id, next_lv, guild->treasure, need_treasure);
			break;
		}

		sub_guild_treasure(guild, need_treasure, true);

		pSkill->skill_id = skill_id;
		pSkill->skill_lv++;
		broadcast_skill_develop_update(guild, pSkill);
		broadcast_guild_object_attr_update(guild, GUILD_OBJECT_ATTR_TYPE__ATTR_SKILL_DEVELOP, skill_id, pSkill->skill_lv);

		uint32_t now = time_helper::get_cached_time() / 1000;
		GuildLog *log = get_important_insert_log(guild);
		log->type = GILT_SKILL_DEVELOP;
		log->time = now;
		snprintf(log->args[0], MAX_GUILD_LOG_ARG_LEN, "%s", config->skillName);
		broadcast_important_log_add(guild, log);

		save_guild_info(guild);
	} while(0);

	GuildSkillUpgradeAnswer resp;
	guild_skill_upgrade_answer__init(&resp);

	resp.result = ret; 
	resp.skillid = skill_id;
	if (pSkill)
	{
		resp.level = pSkill->skill_lv;
	}

	fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_SKILL_DEVELOP_ANSWER, guild_skill_upgrade_answer__pack, resp);
	return 0;
}

int conn_node_guildsrv::handle_guild_skill_pratice_request(EXTERN_DATA *extern_data)
{
	GuildSkillPracticeRequest *req = guild_skill_practice_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint32_t skill_id = req->skillid;
	uint32_t level_num = req->level;
	guild_skill_practice_request__free_unpacked(req, NULL);

	int ret = 0;
	GuildPlayer *player = NULL;
	GuildSkill *pSkill = NULL;
	do
	{
		if (level_num == 0)
		{
			ret = 1;
			break;
		}

		player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not join guild yet", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		GuildInfo *guild = player->guild;
		GuildSkill *pGuildSkill = get_guild_skill_info(guild, skill_id);
		if (!pGuildSkill)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] guild skill memory exhaust, guild_id:%u, skill_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id, skill_id);
			break;
		}

		pSkill = get_player_skill_info(player, skill_id);
		if (!pSkill)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] skill memory exhaust, guild_id:%u, skill_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id, skill_id);
			break;
		}

		uint32_t need_coin = 0;
		uint32_t need_donation = 0;
		uint32_t cur_lv = pSkill->skill_lv;
		for (uint32_t i = 0; i < level_num; ++i)
		{
			if (cur_lv >= pGuildSkill->skill_lv)
			{
				ret = 190500235;
				LOG_ERR("[%s:%d] player[%lu] skill develop level, guild_id:%u, skill_id:%u, skill_lv:%u, develop_lv:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id, skill_id, cur_lv, pGuildSkill->skill_lv);
				break;
			}

			GangsSkillTable *config = get_guild_skill_config(skill_id, cur_lv);
			if (!config)
			{
				ret = ERROR_ID_GUILD_SKILL_LEVEL_MAX;
				LOG_ERR("[%s:%d] player[%lu] get skill config failed, guild_id:%u, skill_id:%u, skill_lv:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id, skill_id, cur_lv);
				break;
			}

			need_donation += config->UseMoney1;
			need_coin += config->UseMoney2;
			cur_lv++;
		}

		if (ret != 0)
		{
			break;
		}

		if (player->donation < need_donation)
		{
			ret = 190500234;
			LOG_ERR("[%s:%d] player[%lu] donation not enough, guild_id:%u, skill_id:%u, skill_id:%u, has_donation:%u, need_donation:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id, skill_id, cur_lv, player->donation, need_donation);
			break;
		}

		{
			//请求扣除消耗
			PROTO_SRV_CHECK_AND_COST_REQ *cost_req = (PROTO_SRV_CHECK_AND_COST_REQ *)get_send_data();
			uint32_t data_len = sizeof(PROTO_SRV_CHECK_AND_COST_REQ) + sizeof(GUILD_SKILL_PRACTICE_CARRY);
			memset(cost_req, 0, data_len);
			cost_req->cost.statis_id = MAGIC_TYPE_GUILD_SKILL_PRACTICE;
			cost_req->cost.coin = need_coin;
			cost_req->data_size = sizeof(GUILD_SKILL_PRACTICE_CARRY);
			GUILD_SKILL_PRACTICE_CARRY *pCarry = (GUILD_SKILL_PRACTICE_CARRY*)cost_req->data;
			pCarry->skill_id = skill_id;
			pCarry->level_num = level_num;
			pCarry->need_donation = need_donation;
			fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_GUILDSRV_COST_REQUEST, data_len, 0);
		}
	} while(0);

	if (ret != 0)
	{
		GuildSkillUpgradeAnswer resp;
		guild_skill_upgrade_answer__init(&resp);

		resp.result = ret; 
		resp.skillid = skill_id;
		if (pSkill)
		{
			resp.level = pSkill->skill_lv;
		}

		fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_SKILL_PRACTICE_ANSWER, guild_skill_upgrade_answer__pack, resp);
	}
	return 0;
}

static int handle_guild_skill_practice_cost(int data_len, uint8_t *data, int result, EXTERN_DATA *extern_data)
{
	GUILD_SKILL_PRACTICE_CARRY *pCarry = (GUILD_SKILL_PRACTICE_CARRY *)data;
	uint32_t skill_id = pCarry->skill_id;
	uint32_t level_num = pCarry->level_num;
	uint32_t need_donation = pCarry->need_donation;

	int ret = result;
	GuildSkill *pSkill = NULL;
	bool internal = false;
	do
	{
		if (ret != 0)
		{
			if (ret == ERROR_ID_COIN_NOT_ENOUGH)
			{
				ret = 190500234;
			}
			break;
		}
		internal = true;

		GuildPlayer *player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not join guild yet", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		GuildInfo *guild = player->guild;
		pSkill = get_player_skill_info(player, skill_id);
		if (!pSkill)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] skill memory exhaust, guild_id:%u, skill_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id, skill_id);
			break;
		}

		if (player->donation < need_donation)
		{
			ret = ERROR_ID_GUILD_PLAYER_DONATION;
			LOG_ERR("[%s:%d] player[%lu] donation not enough, guild_id:%u, skill_id:%u, skill_lv:%u, level_num:%u, has_donation:%u, need_donation:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id, skill_id, pSkill->skill_lv, level_num, player->donation, need_donation);
			break;
		}

		sub_player_donation(player, need_donation, false);

		pSkill->skill_id = skill_id;
		pSkill->skill_lv += level_num;
		save_guild_player(player);

		sync_guild_skill_to_gamesrv(player);
		fast_send_msg_base(&conn_node_guildsrv::connecter, extern_data, SERVER_PROTO_GUILD_SKILL_LEVEL_UP, 0, 0);
	} while(0);
	
	GuildSkillUpgradeAnswer resp;
	guild_skill_upgrade_answer__init(&resp);

	resp.result = ret; 
	resp.skillid = skill_id;
	if (pSkill)
	{
		resp.level = pSkill->skill_lv;
	}

	fast_send_msg(&conn_node_guildsrv::connecter, extern_data, MSG_ID_GUILD_SKILL_PRACTICE_ANSWER, guild_skill_upgrade_answer__pack, resp);

	return (internal ? ret : 0);
}

int conn_node_guildsrv::handle_guild_shop_info_request(EXTERN_DATA *extern_data)
{
	resp_guild_shop_info(extern_data->player_id);
	return 0;
}

int conn_node_guildsrv::handle_guild_shop_buy_request(EXTERN_DATA *extern_data)
{
	ShopBuyRequest *req = shop_buy_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint32_t goods_id = req->goodsid;
	uint32_t buy_num = req->goodsnum;
	shop_buy_request__free_unpacked(req, NULL);

	int ret = 0;
	GuildPlayer *player = NULL;
	do
	{
		if (buy_num == 0)
		{
			ret = ERROR_ID_SHOP_GOODS_BUY_NUM;
			break;
		}

		player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = 190500269;
			LOG_ERR("[%s:%d] player[%lu] not join guild yet", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		GuildInfo *guild = player->guild;
		if (!goods_is_on_sell(guild, goods_id))
		{
			ret = 190500268;
			LOG_ERR("[%s:%d] player[%lu] goods not on sell, guild_id:%u, goods_id:%u, buy_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id, goods_id, buy_num);
			break;
		}

		ShopTable *config = get_config_by_id(goods_id, &shop_config);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get goods config failed, guild_id:%u, goods_id:%u, buy_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id, goods_id, buy_num);
			break;
		}

		uint32_t need_history_donation = config->Condition;
		if (player->all_history_donation < need_history_donation)
		{
			ret = ERROR_ID_GUILD_HISTORY_DONATION;
			LOG_ERR("[%s:%d] player[%lu] history donation, guild_id:%u, goods_id:%u, buy_num:%u, need:%u, has:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id, goods_id, buy_num, need_history_donation, player->all_history_donation);
			break;
		}

		GuildGoods *pGoods = get_player_goods_info(player, goods_id);
		if (!pGoods)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] goods memory exhaust, guild_id:%u, goods_id:%u, buy_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id, goods_id, buy_num);
			break;
		}

		uint32_t limit_num = config->BuyNum;
		if ((int64_t)config->BuyNum > 0 && pGoods->bought_num + buy_num > limit_num)
		{
			ret = ERROR_ID_SHOP_GOODS_REMAIN;
			LOG_ERR("[%s:%d] player[%lu] goods remain, guild_id:%u, goods_id:%u, buy_num:%u, bought_num:%u, limit_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id, goods_id, buy_num, pGoods->bought_num, limit_num);
			break;
		}

		uint32_t need_donation = config->Discount * buy_num;
		if (player->donation < need_donation)
		{
			ret = ERROR_ID_GUILD_PLAYER_DONATION;
			LOG_ERR("[%s:%d] player[%lu] donation not enough, guild_id:%u, goods_id:%u, has:%u, need:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id, goods_id, player->donation, need_donation);
			break;
		}

		{
			//请求发放货物
			PROTO_GUILDSRV_REWARD_REQ *reward_req = (PROTO_GUILDSRV_REWARD_REQ *)get_send_buf(SERVER_PROTO_GUILDSRV_REWARD_REQUEST, get_seq());
			reward_req->head.len = ENDION_FUNC_4(sizeof(PROTO_GUILDSRV_REWARD_REQ) + sizeof(GUILD_SHOP_BUY_CARRY));
			memset(reward_req->head.data, 0, sizeof(PROTO_GUILDSRV_REWARD_REQ) - sizeof(PROTO_HEAD));
			reward_req->statis_id = MAGIC_TYPE_SHOP_BUY;
			reward_req->item_id[0] = config->ItemID;
			reward_req->item_num[0] = buy_num;
			reward_req->data_size = sizeof(GUILD_SHOP_BUY_CARRY);
			GUILD_SHOP_BUY_CARRY *pCarry = (GUILD_SHOP_BUY_CARRY*)reward_req->data;
			pCarry->goods_id = goods_id;
			pCarry->buy_num = buy_num;
			pCarry->need_donation = need_donation;
			add_extern_data(&reward_req->head, extern_data);
			if (connecter.send_one_msg(&reward_req->head, 1) != (int)ENDION_FUNC_4(reward_req->head.len))
			{
				LOG_ERR("[%s:%d] send to gamesrv failed err[%d]", __FUNCTION__, __LINE__, errno);
			}
		}
	} while(0);

	if (ret != 0)
	{
		ShopBuyAnswer resp;
		shop_buy_answer__init(&resp);

		resp.result = ret; 
		resp.goodsid = goods_id;
		resp.boughtnum = 0;

		fast_send_msg(&connecter, extern_data, MSG_ID_SHOP_BUY_ANSWER, shop_buy_answer__pack, resp);
	}

	return 0;
}

static int handle_shop_buy_answer(int data_len, uint8_t *data, int result, EXTERN_DATA *extern_data)
{
	GUILD_SHOP_BUY_CARRY *pCarry = (GUILD_SHOP_BUY_CARRY*)data;
	uint32_t goods_id = pCarry->goods_id;
	uint32_t buy_num = pCarry->buy_num;
	uint32_t need_donation = pCarry->need_donation;

	int ret = result;
	GuildPlayer *player = NULL;
	GuildGoods *pGoods = NULL;
	do
	{
		if (ret != 0)
		{
			break;
		}
		
		player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not join guild yet", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		GuildInfo *guild = player->guild;
		ShopTable *config = get_config_by_id(goods_id, &shop_config);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get goods config failed, guild_id:%u, goods_id:%u, buy_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id, goods_id, buy_num);
			break;
		}

		pGoods = get_player_goods_info(player, goods_id);
		if (!pGoods)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] goods memory exhaust, guild_id:%u, goods_id:%u, buy_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, guild->guild_id, goods_id, buy_num);
			break;
		}

		sub_player_donation(player, need_donation, false);

		if ((int64_t)config->BuyNum > 0)
		{
			pGoods->goods_id = goods_id;
			pGoods->bought_num += buy_num;
			save_guild_player(player);
		}
	} while(0);

	ShopBuyAnswer resp;
	shop_buy_answer__init(&resp);

	resp.result = ret; 
	resp.goodsid = goods_id;
	if (pGoods)
	{
		resp.boughtnum = pGoods->bought_num;
	}

	fast_send_msg(&conn_node_guildsrv::connecter, extern_data, MSG_ID_SHOP_BUY_ANSWER, shop_buy_answer__pack, resp);

	return 0;
}

int conn_node_guildsrv::handle_guild_chat_request(EXTERN_DATA *extern_data)
{
	Chat *req = chat__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	int ret = 0;
	do
	{
		GuildPlayer *player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = CHAR_RET_CODE__noGuild;
			LOG_ERR("[%s:%d] player[%lu] not join guild yet", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		broadcast_guild_chat(player->guild, req);
		player->guild->answer.Answer(extern_data, req->contain, req->sendname);
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);
	
	resp.result = ret;
	fast_send_msg(&conn_node_guildsrv::connecter, extern_data, MSG_ID_CHAT_ANSWER, comm_answer__pack, resp);

	chat__free_unpacked(req, NULL);

	return 0;
}

int conn_node_guildsrv::handle_guild_prodece_medicine_request(EXTERN_DATA *extern_data)
{
	PROTO_UNDO_COST *req = (PROTO_UNDO_COST*)get_data();

	do
	{
		GuildPlayer *player = get_guild_player(extern_data->player_id);
		if (!player)
		{
			LOG_ERR("[%s:%d] can't find guild player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
			continue;
		}

		if (player->guild == NULL)
		{
			LOG_ERR("[%s:%d] player[%lu] is not guild", __FUNCTION__, __LINE__, extern_data->player_id);
			continue;
		}

		if (sub_player_donation(player, req->cost.gold) == 0)
		{
			req->cost.gold = req->cost.coin;
			req->cost.coin = 0;
		}
	} while (0);
	
	PROTO_HEAD *head = get_head();
	if (conn_node_guildsrv::connecter.send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len))
	{
		LOG_ERR("[%s:%d] send to gamesrv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}

	return 0;
}

int conn_node_guildsrv::handle_get_other_info_request(EXTERN_DATA *extern_data)
{
	GetOtherInfoAnswer *req = get_other_info_answer__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	if (req->result != 0)
	{
		PROTO_HEAD *head = get_head();
		if (connecter.send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len))
		{
			LOG_ERR("[%s:%d] send to client failed err[%d]", __FUNCTION__, __LINE__, errno);
		}
	}
	else
	{
		GuildPlayer *target = get_guild_player(req->data->playerid);
		if (target && target->guild)
		{
			req->data->guildid = target->guild->guild_id;
			req->data->guildoffice = target->office;

			int name_len = strlen(target->guild->name);
			if (name_len == 0)
			{
				if (req->data->guildname)
				{
					req->data->guildname[0] = '\0';
				}
			}
			else
			{
				req->data->guildname = (char*)realloc(req->data->guildname, name_len + 1);
				strcpy(req->data->guildname, target->guild->name);
			}
		}

		fast_send_msg(&conn_node_guildsrv::connecter, extern_data, MSG_ID_GET_OTHER_INFO_ANSWER, get_other_info_answer__pack, *req);
	}

	get_other_info_answer__free_unpacked(req, NULL);
	return 0;
}

int conn_node_guildsrv::handle_trade_lot_insert(EXTERN_DATA * /*extern_data*/)
{
	TRADE_LOT_INSERT *req = (TRADE_LOT_INSERT *)get_data();

	GuildInfo *guild = get_guild(req->guild_id);
	if (guild)
	{
		for (uint32_t i = 0; i < guild->member_num; ++i)
		{
			send_mail(&connecter, guild->members[i]->player_id, 270100002, NULL, NULL, NULL, NULL, NULL, 0);
		}
	}

	return 0;
}

int conn_node_guildsrv::handle_guild_battle_call_request(EXTERN_DATA *extern_data)
{
	int ret = 0;
	do
	{
		GuildPlayer *player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not join guild yet", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		GuildInfo *guild = player->guild;
		if (player->office != GUILD_OFFICE_TYPE__OFFICE_MASTER)
		{
			ret = ERROR_ID_GUILD_PLAYER_NO_PERMISSION;
			LOG_ERR("[%s:%d] player[%lu] not join guild yet", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		//发送消息到game_srv
		{
			PROTO_HEAD *head = (PROTO_HEAD*)get_send_buf(MSG_ID_GUILD_BATTLE_CALL_REQUEST, 0);
			uint32_t *player_num = (uint32_t*)head->data;
			*player_num = 0;
			uint64_t *player_id = (uint64_t*)((char*)head->data + sizeof(uint32_t));
			for (uint32_t i = 0; i < guild->member_num; ++i)
			{
				*player_id++ = guild->members[i]->player_id;
				(*player_num)++;
			}
			head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + sizeof(uint32_t) + sizeof(uint64_t) * (*player_num));
			conn_node_base::add_extern_data(head, extern_data);
			if (connecter.send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len))
			{
				LOG_ERR("[%s:%d] send to game_srv failed err[%d]", __FUNCTION__, __LINE__, errno);
			}
		}
	} while(0);

	if (ret != 0)
	{
		CommAnswer resp;
		comm_answer__init(&resp);

		resp.result = ret;
		fast_send_msg(&conn_node_guildsrv::connecter, extern_data, MSG_ID_GUILD_BATTLE_CALL_ANSWER, comm_answer__pack, resp);
	}

	return 0;
}

int conn_node_guildsrv::handle_guild_battle_enter_wait_request(EXTERN_DATA *extern_data)
{
	uint8_t *pData = get_data();
	bool settling = (bool)(*pData);

	do
	{
		GuildPlayer *player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			LOG_ERR("[%s:%d] player[%lu] not join guild yet", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		GuildInfo *guild = player->guild;

		GuildBattleWaitInfoNotify nty;
		guild_battle_wait_info_notify__init(&nty);
		nty.guildscore = guild->battle_score;
		nty.has_guildscore = true;
		fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_BATTLE_WAIT_INFO_NOTIFY, guild_battle_wait_info_notify__pack, nty);
	} while(0);

	if (settling)
	{
		notify_guild_battle_activity_settle(extern_data);
	}

	return 0;
}

static void fill_player_base_data(PlayerRedisInfo &info, PlayerBaseData &data)
{
	data.name = info.name;
	data.n_attrs = 0;
	data.attrs[data.n_attrs]->id = PLAYER_ATTR_LEVEL;
	data.attrs[data.n_attrs]->val = info.lv;
	data.n_attrs++;
	data.attrs[data.n_attrs]->id = PLAYER_ATTR_JOB;
	data.attrs[data.n_attrs]->val = info.job;
	data.n_attrs++;
	data.attrs[data.n_attrs]->id = PLAYER_ATTR_HEAD;
	data.attrs[data.n_attrs]->val = info.head_icon;
	data.n_attrs++;
	data.attrs[data.n_attrs]->id = PLAYER_ATTR_FIGHTING_CAPACITY;
	data.attrs[data.n_attrs]->val = info.fighting_capacity;
	data.n_attrs++;
}

#define MAX_RANK_GET_NUM  100
int conn_node_guildsrv::handle_guild_battle_info_request(EXTERN_DATA *extern_data)
{
	int ret = 0;
	std::vector<std::pair<uint64_t, uint32_t> > rank_info;
	AutoReleaseBatchRedisPlayer t1;		
	std::map<uint64_t, PlayerRedisInfo *> redis_players;
	uint32_t my_rank = 0;
	GuildPlayer *player = NULL;
	GuildInfo *guild = NULL;
	do
	{
		player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not join guild yet", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		guild = player->guild;
		char *rank_key = sg_rank_guild_battle_key;
		int ret2 = sg_redis_client.zget(rank_key, 0, 99, rank_info);
		if (ret2 != 0)
		{
			ret = ERROR_ID_RANK_REDIS;
			LOG_ERR("[%s:%d] player[%lu] get rank failed, rank_key:%s", __FUNCTION__, __LINE__, extern_data->player_id, rank_key);
			break;
		}

		for (size_t i = 0; i < rank_info.size(); ++i)
		{
			if (rank_info[i].first == guild->guild_id)
			{
				my_rank = i + 1;
			}
		}

		std::set<uint64_t> playerIds;
		for (uint32_t i = 0; i < guild->member_num; ++i)
		{
			playerIds.insert(guild->members[i]->player_id);
		}
		if (get_more_redis_player(playerIds, redis_players, sg_player_key, sg_redis_client, t1) != 0)
		{
			ret = ERROR_ID_RANK_REDIS;
			LOG_ERR("[%s:%d] player[%lu] get player failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
	} while(0);

	GuildBattleInfoAnswer resp;
	guild_battle_info_answer__init(&resp);

	GuildBattleRankData rank_data[MAX_RANK_GET_NUM];
	GuildBattleRankData* rank_point[MAX_RANK_GET_NUM];
	GuildBattleContributionData member_data[MAX_GUILD_MEMBER_NUM];
	GuildBattleContributionData* member_point[MAX_GUILD_MEMBER_NUM];
	PlayerBaseData base_data[MAX_GUILD_MEMBER_NUM];
	AttrData attr_data[MAX_GUILD_MEMBER_NUM][MAX_PLAYER_BASE_ATTR_NUM];
	AttrData* attr_point[MAX_GUILD_MEMBER_NUM][MAX_PLAYER_BASE_ATTR_NUM];

	resp.result = ret;
	if (ret == 0)
	{
		resp.ranks = rank_point;
		resp.n_ranks = 0;
		for (size_t i = 0; i < rank_info.size(); ++i)
		{
			GuildInfo *tmp_guild = get_guild(rank_info[i].first);
			if (!tmp_guild)
			{
				continue;
			}

			rank_point[resp.n_ranks] = &rank_data[resp.n_ranks];
			guild_battle_rank_data__init(&rank_data[resp.n_ranks]);
			rank_data[resp.n_ranks].rank = i + 1;
			rank_data[resp.n_ranks].guildid = tmp_guild->guild_id;
			rank_data[resp.n_ranks].guildname = tmp_guild->name;
			rank_data[resp.n_ranks].guildscore = rank_info[i].second;
			resp.n_ranks++;
		}

		resp.contributions = member_point;
		resp.n_contributions = 0;
		for (uint32_t i = 0; i < guild->member_num; ++i)
		{
			GuildPlayer *tmp_player = guild->members[i];
			if (tmp_player->battle_score == 0)
			{
				continue;
			}

			member_point[resp.n_contributions] = &member_data[resp.n_contributions];
			guild_battle_contribution_data__init(&member_data[resp.n_contributions]);

			member_data[resp.n_contributions].baseinfo = &base_data[resp.n_contributions];
			player_base_data__init(&base_data[resp.n_contributions]);
			base_data[resp.n_contributions].attrs = attr_point[resp.n_contributions];
			base_data[resp.n_contributions].n_attrs = 0;
			for (int j = 0; j < MAX_PLAYER_BASE_ATTR_NUM; ++j)
			{
				attr_point[resp.n_contributions][j] = &attr_data[resp.n_contributions][j];
				attr_data__init(&attr_data[resp.n_contributions][j]);
			}
			PlayerRedisInfo *redis_player = find_redis_from_map(redis_players, tmp_player->player_id);
			if (redis_player)
			{
				fill_player_base_data(*redis_player, base_data[resp.n_contributions]);
			}
			base_data[resp.n_contributions].playerid = tmp_player->player_id;

			member_data[resp.n_contributions].score = tmp_player->battle_score;
			resp.n_contributions++;
		}

		resp.myguildrank = my_rank;
		resp.myguildscore = guild->battle_score;
		resp.myscore = player->battle_score;
	}

	fast_send_msg(&conn_node_guildsrv::connecter, extern_data, MSG_ID_GUILD_BATTLE_INFO_ANSWER, guild_battle_info_answer__pack, resp);

	// for (std::map<uint64_t, PlayerRedisInfo*>::iterator iter = redis_players.begin(); iter != redis_players.end(); ++iter)
	// {
	// 	player_redis_info__free_unpacked(iter->second, NULL);
	// }
	return 0;
}

int conn_node_guildsrv::handle_guild_battle_fight_reward_request(EXTERN_DATA * /*extern_data*/)
{
	PROTO_GUILD_BATTLE_REWARD *req = (PROTO_GUILD_BATTLE_REWARD*)buf_head();
	LOG_INFO("[%s:%d] guild %u", __FUNCTION__, __LINE__, req->guild_id);
	uint32_t activity_id = req->activity_id;
	
	GuildInfo *guild = get_guild(req->guild_id);
	AutoReleaseBatchRedisPlayer t1;
	do
	{
		if (!guild)
		{
			LOG_ERR("[%s:%d] can't find guild[%u]", __FUNCTION__, __LINE__, req->guild_id);
			break;
		}

		uint32_t total_treasure = 0, total_score = 0, total_popularity = 0;
		std::set<uint64_t> count_teams;
		for (uint32_t i = 0; i < req->player_num; ++i)
		{
			GuildPlayer *player = get_guild_player(req->player_id[i]);
			if (!player)
			{
				LOG_ERR("[%s:%d] can't find guild player[%lu]", __FUNCTION__, __LINE__, req->player_id[i]);
				continue;
			}

			if (player->guild != guild)
			{
				LOG_ERR("[%s:%d] player[%lu] guild[%u] is not guild[%u]", __FUNCTION__, __LINE__, req->player_id[i], player->guild->guild_id, guild->guild_id);
				continue;
			}

			add_player_donation(player, req->donation[i]);
			add_player_battle_score(player, req->score[i]);
			total_treasure += req->treasure[i];
			add_player_contribute_treasure(player, req->treasure[i]);
			total_score += req->score[i];

			if (req->team_id[i] == 0 || count_teams.find(req->team_id[i]) == count_teams.end())
			{
				count_teams.insert(req->team_id[i]);
				if (!guild_battle_is_final(activity_id))
				{
					switch (req->result[i])
					{
						case 1:
							total_popularity += sg_guild_battle_preliminary_popularity[0];
							break;
						case 2:
							total_popularity += sg_guild_battle_preliminary_popularity[1];
							break;
						case 3:
							total_popularity += sg_guild_battle_preliminary_popularity[2];
							break;
						case 4:
							total_popularity += sg_guild_battle_preliminary_popularity[3];
							break;
					}
				}
				else
				{
					switch (req->result[i])
					{
						case 0:
							total_popularity += sg_guild_battle_final_popularity[4];
							break;
						case 1:
							total_popularity += sg_guild_battle_final_popularity[0];
							break;
						case 2:
							total_popularity += sg_guild_battle_final_popularity[1];
							break;
						case 3:
							total_popularity += sg_guild_battle_final_popularity[2];
							break;
						case 4:
							total_popularity += sg_guild_battle_final_popularity[3];
							break;
					}
				}
			}
		}

		//为了减少消息广播和数据存库的次数，统计好一次过加
		add_guild_treasure(guild, total_treasure);
		add_guild_battle_score(guild, total_score);
		add_guild_popularity(guild, total_popularity);

		if (req->broadcast_num > 0)
		{
			std::vector<uint64_t> broadcast_ids;
			for (uint32_t i = 0; i < req->broadcast_num; ++i)
			{
				broadcast_ids.push_back(req->broadcast_id[i]);
			}

			broadcast_guild_battle_score(guild, broadcast_ids);
		}
	} while(0);

	do
	{
		if (!guild_battle_is_final(activity_id))
		{
			break;
		}

		std::vector<std::pair<uint64_t, uint32_t> > rank_info;
		char *rank_key = sg_rank_guild_battle_key;
		int ret2 = sg_redis_client.zget(rank_key, 0, 3, rank_info);
		if (ret2 != 0)
		{
			LOG_ERR("[%s:%d] get rank failed, rank_key:%s", __FUNCTION__, __LINE__, rank_key);
			break;
		}

		std::vector<uint32_t> guild_ids;
		load_guild_battle_final_list(guild_ids);
		if (guild_ids.size() > rank_info.size())
		{
			for (size_t i = 0; i < guild_ids.size(); ++i)
			{
				bool has = false;
				for (size_t j = 0; j < rank_info.size(); ++j)
				{
					if (rank_info[j].first == guild_ids[i])
					{
						has = true;
						break;
					}
				}

				if (!has)
				{
					rank_info.push_back(std::make_pair(guild_ids[i], 0));
				}
			}
		}

		GuildBattleRoundFinishNotify nty;
		GuildBattleRankData rank_data[MAX_GUILD_BATTLE_FINAL_GUILD_NUM];
		GuildBattleRankData* rank_point[MAX_GUILD_BATTLE_FINAL_GUILD_NUM];

		for (uint32_t i = 0; i < req->player_num; ++i)
		{
			if (req->result[i] == 0)
			{
				continue;
			}

			GuildPlayer *player = get_guild_player(req->player_id[i]);
			if (!player)
			{
				LOG_ERR("[%s:%d] can't find guild player[%lu]", __FUNCTION__, __LINE__, req->player_id[i]);
				continue;
			}

			if (player->guild != guild)
			{
				LOG_ERR("[%s:%d] player[%lu] guild[%u] is not guild[%u]", __FUNCTION__, __LINE__, req->player_id[i], player->guild->guild_id, guild->guild_id);
				continue;
			}

			guild_battle_round_finish_notify__init(&nty);

			nty.result = req->result[i];
			nty.score = req->score[i];
			nty.guildtreasure = req->treasure[i];
			nty.guilddonation = req->donation[i];
			nty.ranks = rank_point;
			nty.n_ranks = 0;
			for (size_t j = 0; j < rank_info.size() && j < MAX_GUILD_BATTLE_FINAL_GUILD_NUM; ++j)
			{
				GuildInfo *tmp_guild = get_guild(rank_info[j].first);
				if (!tmp_guild)
				{
					continue;
				}
				PlayerRedisInfo *redis_player = get_redis_player(tmp_guild->master_id, sg_player_key, sg_redis_client, t1);
				if (!redis_player)
				{
					continue;
				}

				rank_point[nty.n_ranks] = &rank_data[nty.n_ranks];
				guild_battle_rank_data__init(&rank_data[nty.n_ranks]);
				rank_data[nty.n_ranks].rank = j + 1;
				rank_data[nty.n_ranks].guildid = tmp_guild->guild_id;
				rank_data[nty.n_ranks].guildname = tmp_guild->name;
				rank_data[nty.n_ranks].guildscore = rank_info[j].second;
				rank_data[nty.n_ranks].guildcamp = redis_player->zhenying;
				nty.n_ranks++;
			}

			EXTERN_DATA ext_data;
			ext_data.player_id = player->player_id;

			fast_send_msg(&conn_node_guildsrv::connecter, &ext_data, MSG_ID_GUILD_BATTLE_ROUND_FINISH_NOTIFY, guild_battle_round_finish_notify__pack, nty);
		}
	} while(0);

	return 0;
}

int conn_node_guildsrv::handle_guild_battle_sync_begin(EXTERN_DATA * /*extern_data*/)
{
	PROTO_HEAD *head = (PROTO_HEAD*)buf_head();
	uint32_t *pData = (uint32_t*)(head->data);
	uint32_t activity_id = *pData;
	LOG_INFO("[%s:%d] activity %u", __FUNCTION__, __LINE__, activity_id);

	guild_battle_opening = true;
	if (!guild_battle_is_final(activity_id))
	{ //预赛
	}
	else
	{ //决赛
		/* 决赛开始，保存参赛名单，清除积分排行榜 */
		std::vector<uint32_t> guild_ids;
		do
		{
			std::vector<std::pair<uint64_t, uint32_t> > rank_info;
			char *rank_key = sg_rank_guild_battle_key;
			int ret2 = sg_redis_client.zget(rank_key, 0, 3, rank_info);
			if (ret2 != 0)
			{
				break;
			}

			for (size_t i = 0; i < rank_info.size(); ++i)
			{
				guild_ids.push_back(rank_info[i].first);
			}
		} while(0);

		save_guild_battle_final_list(guild_ids);
		clear_player_total_battle_score();
	}

	return 0;
}

int conn_node_guildsrv::handle_guild_battle_sync_end(EXTERN_DATA * /*extern_data*/)
{
	PROTO_HEAD *head = (PROTO_HEAD*)buf_head();
	uint32_t *pData = (uint32_t*)(head->data);
	uint32_t activity_id = *pData;
	LOG_INFO("[%s:%d] activity %u", __FUNCTION__, __LINE__, activity_id);

	guild_battle_opening = false;
	uint32_t param_id = (guild_battle_is_final(activity_id) ? 161000319 : 161000318);
	ParameterTable *param_config = get_config_by_id(param_id, &parameter_config);
	if (param_config)
	{
		Chat nty;
		chat__init(&nty);
		nty.channel = CHANNEL__family;
		nty.contain = param_config->parameter2;

		std::map<uint32_t, GuildInfo*> &guild_map = get_all_guild();
		for (std::map<uint32_t, GuildInfo*>::iterator iter = guild_map.begin(); iter != guild_map.end(); ++iter)
		{
			GuildInfo *guild = iter->second;
			for (uint32_t i = 0; i < guild->member_num; ++i)
			{
				if (guild->members[i]->act_battle_score > 0)
				{
					broadcast_guild_chat(guild, &nty);
					break;
				}
			}
		}
	}

	if (!guild_battle_is_final(activity_id))
	{ //预赛
		//清除本次活动的玩家积分
		clear_player_act_battle_score();
	}
	else
	{ //决赛
		/* 决赛结束，清除参赛名单，清除积分排行榜 */
		clear_player_total_battle_score();
		sg_redis_client.del(sg_guild_battle_final_key);
	}

	return 0;
}

int conn_node_guildsrv::handle_guild_battle_sync_settle(EXTERN_DATA * /*extern_data*/)
{
	PROTO_GUILD_BATTLE_SETTLE *proto = (PROTO_GUILD_BATTLE_SETTLE*)buf_head();
	uint32_t activity_id = proto->activity_id;
	LOG_INFO("[%s:%d] activity %u", __FUNCTION__, __LINE__, activity_id);

	std::vector<std::pair<uint64_t, uint32_t> > rank_info;
	int ret = 0;
	do
	{
		CRedisClient &rc = sg_redis_client;
		char *rank_key = sg_rank_guild_battle_key;
		uint32_t rank_end;
		ret =  rc.zcard(rank_key, rank_end);
		if (ret != 0)
		{
			LOG_ERR("[%s:%d] rank_key:%s, ret:%d", __FUNCTION__, __LINE__, rank_key, ret);
			break;
		}

		ret = rc.zget(rank_key, 0, rank_end, rank_info);
		if (ret != 0)
		{
			LOG_ERR("[%s:%d] rank_key:%s, ret:%d", __FUNCTION__, __LINE__, rank_key, ret);
			break;
		}

		for (std::map<uint64_t, GangsDungeonTable*>::iterator iter = guild_battle_reward_config.begin(); iter != guild_battle_reward_config.end(); ++iter)
		{
			GangsDungeonTable *config = iter->second;
			if (guild_battle_is_final(activity_id))
			{
				if (config->Type != 1)
				{
					continue;
				}
			}
			else
			{
				if (config->Type != 0)
				{
					continue;
				}
			}

			if ((uint32_t)config->RankHigh > rank_end)
			{
				continue;
			}

			for (uint32_t i = (uint32_t)config->RankHigh - 1; i < (uint32_t)config->RankLow && i < rank_end; ++i)
			{
				std::map<uint32_t, uint32_t> attachs;
				for (uint32_t j = 0; j < config->n_RewardID; ++j)
				{
					attachs[config->RewardID[j]] += config->RewardNum[j];
				}

				uint32_t guild_id = rank_info[i].first;
				GuildInfo *guild = get_guild(guild_id);
				if (!guild)
				{
					continue;
				}

				for (uint32_t k = 0; k < guild->member_num; ++k)
				{
					GuildPlayer *player = guild->members[k];
					if (player->act_battle_score == 0)
					{
						continue;
					}

					send_mail(&connecter, player->player_id, config->mailID, NULL, NULL, NULL, NULL, &attachs, MAGIC_TYPE_GUILD_BATTLE_ACTIVITY);
				}
			}
		}
	} while(0);

	do
	{
		EXTERN_DATA ext_data;
		uint64_t *ppp = proto->broadcast_id;
		for (uint32_t i = 0; i < proto->broadcast_num; ++i)
		{
			ext_data.player_id = *ppp++;
			notify_guild_battle_activity_settle(&ext_data);
		}
	} while(0);

	if (!guild_battle_is_final(activity_id))
	{ //预赛
	}
	else
	{ //决赛
	}

	return 0;
}

int conn_node_guildsrv::notify_guild_battle_activity_settle(EXTERN_DATA *extern_data)
{
	std::vector<std::pair<uint64_t, uint32_t> > rank_info;
	uint32_t my_rank = 0;
	GuildPlayer *player = NULL;
	GuildInfo *guild = NULL;
	AutoReleaseBatchRedisPlayer t1;
	do
	{
		player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			LOG_ERR("[%s:%d] player[%lu] not join guild yet", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		guild = player->guild;
		char *rank_key = sg_rank_guild_battle_key;
		int ret2 = sg_redis_client.zget(rank_key, 0, 3, rank_info);
		if (ret2 != 0)
		{
			LOG_ERR("[%s:%d] player[%lu] get rank failed, rank_key:%s", __FUNCTION__, __LINE__, extern_data->player_id, rank_key);
			break;
		}

		for (size_t i = 0; i < rank_info.size(); ++i)
		{
			if (rank_info[i].first == guild->guild_id)
			{
				my_rank = i + 1;
			}
		}

		if (my_rank == 0)
		{
			ret2 = sg_redis_client.zget_rank(rank_key, guild->guild_id, my_rank);
			if (ret2 == 0)
			{
				my_rank++;
			}
		}
	} while(0);

	GuildBattleActivityFinishNotify resp;
	guild_battle_activity_finish_notify__init(&resp);

	GuildBattleRankData rank_data[MAX_GUILD_BATTLE_FINAL_GUILD_NUM];
	GuildBattleRankData* rank_point[MAX_GUILD_BATTLE_FINAL_GUILD_NUM];

	resp.ranks = rank_point;
	resp.n_ranks = 0;
	for (size_t i = 0; i < rank_info.size() && i < MAX_GUILD_BATTLE_FINAL_GUILD_NUM; ++i)
	{
		GuildInfo *tmp_guild = get_guild(rank_info[i].first);
		if (!tmp_guild)
		{
			continue;
		}

		PlayerRedisInfo *redis_player = get_redis_player(tmp_guild->master_id, sg_player_key, sg_redis_client, t1);
		if (!redis_player)
		{
			continue;
		}

		rank_point[resp.n_ranks] = &rank_data[resp.n_ranks];
		guild_battle_rank_data__init(&rank_data[resp.n_ranks]);
		rank_data[resp.n_ranks].rank = i + 1;
		rank_data[resp.n_ranks].guildid = tmp_guild->guild_id;
		rank_data[resp.n_ranks].guildname = tmp_guild->name;
		rank_data[resp.n_ranks].guildscore = rank_info[i].second;
		rank_data[resp.n_ranks].guildcamp = redis_player->zhenying;
		resp.n_ranks++;
	}

	resp.guildrank = my_rank;
	resp.guildscore = guild->battle_score;

	fast_send_msg(&conn_node_guildsrv::connecter, extern_data, MSG_ID_GUILD_BATTLE_ACTIVITY_FINISH_NOTIFY, guild_battle_activity_finish_notify__pack, resp);
	return 0;
}

int conn_node_guildsrv::handle_guild_battle_final_list_request(EXTERN_DATA * /*extern_data*/)
{
	std::vector<uint32_t> guild_ids;
	if (load_guild_battle_final_list(guild_ids) != 0)
	{
		std::vector<std::pair<uint64_t, uint32_t> > rank_info;
		char *rank_key = sg_rank_guild_battle_key;
		int ret2 = sg_redis_client.zget(rank_key, 0, 3, rank_info);
		if (ret2 != 0)
		{
			LOG_ERR("[%s:%d] get final list from %s failed, ret:%d", __FUNCTION__, __LINE__, rank_key, ret2);
		}
		else
		{
			for (size_t i = 0; i < rank_info.size(); ++i)
			{
				guild_ids.push_back(rank_info[i].first);
			}
		}
	}

	PROTO_GUILD_BATTLE_RANK *rank_req = (PROTO_GUILD_BATTLE_RANK*)get_send_buf(SERVER_PROTO_GUILD_BATTLE_FINAL_LIST_ANSWER, 0);
	rank_req->head.len = ENDION_FUNC_4(sizeof(PROTO_GUILD_BATTLE_RANK));
	memset(rank_req->head.data, 0, sizeof(PROTO_GUILD_BATTLE_RANK) - sizeof(PROTO_HEAD));
	for (size_t i = 0; i < guild_ids.size() && i < 4; ++i)
	{
		rank_req->guild_id[i] = guild_ids[i];
	}
	if (connecter.send_one_msg(&rank_req->head, 1) != (int)ENDION_FUNC_4(rank_req->head.len))
	{
		LOG_ERR("[%s:%d] send to game_srv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}

	return 0;
}

int conn_node_guildsrv::handle_guild_battle_add_final_id(EXTERN_DATA *extern_data)
{
	PROTO_HEAD *head = (PROTO_HEAD*)buf_head();
	uint32_t *pData = (uint32_t*)(head->data);
	uint32_t guild_id = *pData;

	do
	{
		GuildInfo *guild = get_guild(guild_id);
		if (!guild)
		{
			break;
		}

		std::vector<uint32_t> guild_ids;
		load_guild_battle_final_list(guild_ids);
		if (guild_ids.size() >= 4)
		{
			break;
		}

		guild_ids.push_back(guild_id);
		save_guild_battle_final_list(guild_ids);
	} while(0);

	return 0;
}

int conn_node_guildsrv::handle_guild_ruqin_creat_monster_level_request(EXTERN_DATA *extern_data)
{
	uint32_t *pData = (uint32_t*)get_data();
	uint32_t guild_id = *pData++;
	
	GuildInfo *guild = get_guild(guild_id);
	AutoReleaseBatchRedisPlayer t1;
	uint32_t all_level = 0;
	uint32_t all_num   = 0;
	uint32_t now_time =time_helper::get_cached_time() / 1000;
	if(guild == NULL)
	{
		LOG_ERR("[%s:%d] 帮会入侵活动获取game刷怪等级失败,没找到帮会数据,guild_id[%d]", __FUNCTION__, __LINE__,guild_id );
		return -1;
	}
	for(size_t i = 0; i < MAX_GUILD_MEMBER_NUM; i++)
	{
		if(guild->members[i] == NULL)
			break;
		if(guild->members[i]->player_id == 0)
			break;
		PlayerRedisInfo *redis_player = get_redis_player(guild->members[i]->player_id, sg_player_key, sg_redis_client, t1);
		if(redis_player == NULL)
			continue;
		if(redis_player->status ==0)
		{
			all_level += redis_player->lv;
			all_num += 1;
		}
		else 
		{
			if(now_time - redis_player->status < 3 * 3600 * 24)
			{
				all_level += redis_player->lv;
				all_num += 1; 
			}
			
		}
	}
	if(all_level == 0 || all_num ==0)
	{
		LOG_ERR("[%s:%d] 帮会入侵活动获取game刷怪等级,总等级或者总人数出错all_level[%d] all_num[%d]", __FUNCTION__, __LINE__,all_level, all_num );
		return -2;
	}


	uint32_t level = all_level / all_num;


	PROTO_HEAD_RUQIN *req = (PROTO_HEAD_RUQIN*)conn_node_base::get_send_buf(SERVER_PROTO_GUILD_RUQIN_CREAT_MONSTER_LEVEL_ANSWER, 0);
	req->guild_id = guild_id;
	req->level    = level;
	req->head.len = ENDION_FUNC_4(sizeof(PROTO_HEAD_RUQIN));

	EXTERN_DATA ext_data;
	conn_node_base::add_extern_data(&req->head, &ext_data);
	if (conn_node_guildsrv::connecter.send_one_msg(&req->head, 1) != (int)ENDION_FUNC_4(req->head.len))
	{
		LOG_ERR("[%s:%d] send to gamesrv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
	return 0;
}

int conn_node_guildsrv::guild_ruqin_reward_info_notify(EXTERN_DATA *extern_data)
{
	GuildRuqinActiveRewardNotify *req = guild_ruqin_active_reward_notify__unpack(NULL, get_data_len(), get_data());
	if(req == NULL)
	{
		LOG_ERR("[%s:%d]  unpack failed", __FUNCTION__, __LINE__);
		return -1;
	}
	do{
		AutoReleaseBatchRedisPlayer t1;
		uint32_t guild_id = req->guild_id;
		double all_damage = req->all_damage;
		uint32_t all_boshu = req->all_bushu;
		uint32_t boshu = req->boshu;
		uint32_t member_num = req->n_all_palyer;
		ParameterTable *param_config = NULL;
		if(boshu <=0 || all_boshu <=0)
		{
			LOG_ERR("[%s:%d]  帮会入侵发奖波数出错boshu[%u] all_boshu[%u]", __FUNCTION__, __LINE__, boshu, all_boshu);
			break;
		}
		param_config = get_config_by_id(161000349, &parameter_config);
		if(param_config == NULL || param_config->n_parameter1 <1)
		{
			LOG_ERR("[%s:%d]  获取参数表配置错误,id[161000349]", __FUNCTION__, __LINE__);
			break;
		}
		double canshu_a = param_config->parameter1[0];

		param_config = get_config_by_id(161000350, &parameter_config);
		if(param_config == NULL || param_config->n_parameter1 <1)
		{
			LOG_ERR("[%s:%d]  获取参数表配置错误,id[161000350]", __FUNCTION__, __LINE__);
			break;
		}
		double canshu_b = param_config->parameter1[0];

		param_config = get_config_by_id(161000351, &parameter_config);
		if(param_config == NULL || param_config->n_parameter1 <1)
		{
			LOG_ERR("[%s:%d]  获取参数表配置错误,id[161000351]", __FUNCTION__, __LINE__);
			break;
		}
		double canshu_c = param_config->parameter1[0];

		param_config = get_config_by_id(161000352, &parameter_config);
		if(param_config == NULL || param_config->n_parameter1 <1)
		{
			LOG_ERR("[%s:%d]  获取参数表配置错误,id[161000352]", __FUNCTION__, __LINE__);
			break;
		}
		double canshu_d = param_config->parameter1[0];

		param_config = get_config_by_id(161000353, &parameter_config);
		if(param_config == NULL || param_config->n_parameter1 <1)
		{
			LOG_ERR("[%s:%d]  获取参数表配置错误,id[161000353]", __FUNCTION__, __LINE__);
			break;
		}
		double canshu_e = param_config->parameter1[0];

		param_config = get_config_by_id(161000354, &parameter_config);
		if(param_config == NULL || param_config->n_parameter1 <1)
		{
			LOG_ERR("[%s:%d]  获取参数表配置错误,id[161000354]", __FUNCTION__, __LINE__);
			break;
		}
		double canshu_f = param_config->parameter1[0];

		param_config = get_config_by_id(161000356, &parameter_config);
		if(param_config == NULL || param_config->n_parameter1 <3)
		{
			LOG_ERR("[%s:%d]  获取参数表配置错误,id[161000356]", __FUNCTION__, __LINE__);
			break;
		}
		uint32_t coin_item_id = param_config->parameter1[0];
		uint32_t exp_item_id = param_config->parameter1[1];
		uint32_t banggong_item_id = param_config->parameter1[2];


		FactionActivity *faction_table = get_config_by_id(GUILD_INTRUSION_CONTROLTABLE_ID, &guild_land_active_config);
		if(faction_table == NULL)
		{
			LOG_ERR("[%s:%d]  获取帮会领地活动参数表出错", __FUNCTION__, __LINE__);
			break;
		}
		ControlTable *contro_table =  get_config_by_id(faction_table->ControlID, &all_control_config);
		if(contro_table == NULL)
		{
			LOG_ERR("[%s:%d]  获取帮会领地活动控制表出错", __FUNCTION__, __LINE__);
			break;
		}
		uint32_t reward_type = contro_table->TimeType;
		uint32_t all_reward_num  = contro_table->RewardTime;

		std::vector<uint64_t> braod_player; //需要广播奖励信息的玩家
		braod_player.clear();
		GuildRuqinActiveRewardAndRankNotify notify;
		GuildRuqinPlayerRewardInfo player_reward_info[MAX_GUILD_MEMBER_NUM];
		GuildRuqinPlayerRewardInfo *player_reward_info_point[MAX_GUILD_MEMBER_NUM];
		GuildRuqinRewardItemInfo   reward_item_info[MAX_GUILD_MEMBER_NUM][GUILD_RUQIN_MAX_REWARD_ITEM_NUM];
		GuildRuqinRewardItemInfo *reward_item_info_point[MAX_GUILD_MEMBER_NUM][GUILD_RUQIN_MAX_REWARD_ITEM_NUM];
		char name[MAX_GUILD_MEMBER_NUM][MAX_PLAYER_NAME_LEN];
		uint32_t player_num = 0;
		guild_ruqin_active_reward_and_rank_notify__init(&notify);
		notify.reward_info = player_reward_info_point;
		for(size_t i = 0; i < req->n_all_palyer && req->all_palyer[i] != NULL && player_num < MAX_GUILD_MEMBER_NUM; i++)
		{
			PlayerRedisInfo *redis_player = get_redis_player(req->all_palyer[i]->player_id, sg_player_key, sg_redis_client, t1);
			if(redis_player == NULL)
			{
				LOG_ERR("[%s:%d]帮会入侵活动给奖励失败，获取玩家redis信息失败player_id[%lu]", __FUNCTION__, __LINE__, req->all_palyer[i]->player_id);
				continue;
			}
			if(guild_id != redis_player->guild_id)
			{
				LOG_INFO("[%s:%d]帮会入侵活动给奖励失败，玩家换了帮会或者退帮了this_guild_id[%u] player_guild_id[%u]", __FUNCTION__, __LINE__, guild_id, redis_player->guild_id);
				continue;
			}
			GuildPlayer *guild_player_data = get_guild_player(req->all_palyer[i]->player_id);
			if(guild_player_data == NULL)
			{
				LOG_ERR("[%s:%d]帮会入侵活动给奖励失败，获取玩家GuildPlayer数据失败player_id[%lu]", __FUNCTION__, __LINE__, req->all_palyer[i]->player_id);
				continue;
			}
			uint32_t reward_num = get_guild_land_active_reward_count(guild_player_data, GUILD_INTRUSION_CONTROLTABLE_ID);

			uint64_t level = redis_player->lv;
			uint64_t id = 1041*100000 + level;
			ActorLevelTable *level_config = get_config_by_id(id, &actor_level_config);
			if(level_config == NULL)
			{
				LOG_INFO("[%s:%d]帮会入侵活动给奖励失败，获取ActorLevelTable等级配置失败id[%lu]", __FUNCTION__, __LINE__, id);
				continue;
			}

			uint64_t quelvexp = level_config->QueLvExp;
			uint64_t quelvcoin = level_config->QueLvCoin;
			double damage_bi = req->all_palyer[i]->damage/all_damage;
			double reward_xishu = damage_bi * member_num * canshu_a;
			double real_reward_xishu = (reward_xishu < canshu_b ? reward_xishu : canshu_b);
			uint32_t  exp_reward = ceil((quelvexp * real_reward_xishu + quelvexp * member_num * canshu_c * damage_bi) * (boshu -1) / all_boshu);
			uint32_t  coin_reward = ceil((quelvcoin * real_reward_xishu + quelvcoin * member_num * canshu_c * damage_bi) * (boshu -1) / all_boshu);
			double banggong_xishu = damage_bi * member_num * canshu_e;
			double real_banggong_xishu = banggong_xishu < canshu_f ? banggong_xishu : canshu_f;
			uint32_t banggong_reward = ceil(canshu_d * real_banggong_xishu * (boshu -1) / all_boshu);

			if(reward_type == 2)
			{
				if(reward_num >= all_reward_num)
				{
					coin_reward = 0;
					exp_reward = 0;
					banggong_reward = 0;
				}
			}
			std::map<uint32_t, uint32_t> attachs;
			if(exp_reward != 0 || coin_reward != 0 || banggong_reward !=0)
			{
				if(reward_type == 2)
				{
					add_guild_land_active_reward_count(guild_player_data, GUILD_INTRUSION_CONTROLTABLE_ID);
					EXTERN_DATA ext_data;
					ext_data.player_id = req->all_palyer[i]->player_id;
					fast_send_msg_base(&connecter, &ext_data, SERVER_PROTO_GUILD_RUQIN_ADD_COUNT, 0, 0);
				}
				attachs[coin_item_id] += coin_reward;
				attachs[exp_item_id] += exp_reward;
				attachs[banggong_item_id] += banggong_reward;
				send_mail(&connecter, req->all_palyer[i]->player_id, 270300042, NULL, NULL, NULL, NULL, &attachs, MAGIC_TYPE_GUILD_RUQIN_REWARD);

			}

			//奖励信息需要广播给当前正在帮会领地以及有输出伤害的玩家
			if(req->all_palyer[i]->on_land == true)
			{
				braod_player.push_back(req->all_palyer[i]->player_id);
			}
			player_reward_info_point[player_num] = &player_reward_info[player_num];
			guild_ruqin_player_reward_info__init(&player_reward_info[player_num]);
			player_reward_info[player_num].damage = req->all_palyer[i]->damage;
			player_reward_info[player_num].lv = level;
			player_reward_info[player_num].job = redis_player->head_icon;
			player_reward_info[player_num].item = reward_item_info_point[player_num];
			strcpy(name[player_num], redis_player->name);
			player_reward_info[player_num].name = name[player_num];
			uint32_t item_num = 0;
			for(std::map<uint32_t, uint32_t>::iterator itr = attachs.begin(); itr != attachs.end() && item_num < GUILD_RUQIN_MAX_REWARD_ITEM_NUM; itr++)
			{
				reward_item_info_point[player_num][item_num] = &reward_item_info[player_num][item_num];
				guild_ruqin_reward_item_info__init(&reward_item_info[player_num][item_num]);
				reward_item_info[player_num][item_num].item_id = itr->first;		
				reward_item_info[player_num][item_num].item_num = itr->second;		
				item_num++;
			}
			player_reward_info[player_num].n_item = item_num;
			player_num++;
		}
		notify.n_reward_info = player_num;
		if(braod_player.size() != 0 && player_num !=0)
		{
			broadcast_message(MSG_ID_GUILD_RUQIN_REWARD_INFO_NOTIFY, &notify, (pack_func)guild_ruqin_active_reward_and_rank_notify__pack, braod_player);
		}


		std::vector<uint64_t> braod_guild_player; //广播给本帮会的玩家
		GuildInfo *guild = get_guild(guild_id);
		if(guild == NULL)
		{
			LOG_ERR("[%s:%d] 帮会入侵活动结束公告失败", __FUNCTION__, __LINE__);
			break;
		}
		for(size_t i = 0; i < MAX_GUILD_MEMBER_NUM; i++)
		{
			if(guild->members[i] == NULL)
				break;
			if(guild->members[i]->player_id == 0)
				break;
			braod_guild_player.push_back(guild->members[i]->player_id);
		}
		uint64_t id = 0;
		char buff[512];
		Chat send;
		chat__init(&send);
		send.contain = buff;
		send.channel = CHANNEL__family;
		send.sendname = NULL;
		send.sendplayerid = 0;
		send.sendplayerlv = 0;
		send.sendplayerjob = 0;
		if(boshu > all_boshu)
		{
			id = 161000362;

		}
		else 
		{
			id =161000363;
		}
		param_config = get_config_by_id(id, &parameter_config);
		if(param_config == NULL || param_config->parameter2 == NULL)
		{
			break;
		}
		strcpy(buff, param_config->parameter2);
		broadcast_message(MSG_ID_CHAT_NOTIFY, &send, (pack_func)chat__pack, braod_guild_player);
	}while(0);
	guild_ruqin_active_reward_notify__free_unpacked(req, NULL);

	return 0;
}
int conn_node_guildsrv::guild_ruqin_boss_creat_notify(EXTERN_DATA *extern_data)
{
	GuildRuqinBossCreatNotify *req = guild_ruqin_boss_creat_notify__unpack(NULL, get_data_len(), get_data());
	if(req == NULL)
	{
		LOG_ERR("[%s:%d]  unpack failed", __FUNCTION__, __LINE__);
		return -1;
	}
	uint32_t guild_id = req->guild_id;
	guild_ruqin_boss_creat_notify__free_unpacked(req, NULL);
	ParameterTable *param_config = get_config_by_id(161000364, &parameter_config);
	if(param_config == NULL || param_config->parameter2 == NULL)
	{
		LOG_ERR("[%s:%d] 帮会入侵活动boss刷新公告获取配置失败", __FUNCTION__, __LINE__);
		return -10;
	}
	std::vector<uint64_t> braod_guild_player; //广播给本帮会的玩家
	GuildInfo *guild = get_guild(guild_id);
	for(size_t i = 0; i < MAX_GUILD_MEMBER_NUM; i++)
	{
		if(guild->members[i] == NULL)
			break;
		if(guild->members[i]->player_id == 0)
			break;
		braod_guild_player.push_back(guild->members[i]->player_id);
	}
	char buff[512];
	Chat send;
	chat__init(&send);
	send.contain = buff;
	send.channel = CHANNEL__family;
	send.sendname = NULL;
	send.sendplayerid = 0;
	send.sendplayerlv = 0;
	send.sendplayerjob = 0;
	strcpy(buff, param_config->parameter2);
	broadcast_message(MSG_ID_CHAT_NOTIFY, &send, (pack_func)chat__pack, braod_guild_player);


	return 0;
}

int conn_node_guildsrv::handle_guild_donate_request(EXTERN_DATA *extern_data)
{
	GuildDonateRequest *req = guild_donate_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint32_t type = req->type;
	guild_donate_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		GuildPlayer *player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not in guild", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		DonationTable *config = get_guild_donate_config(type);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get config failed, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, type);
			break;
		}

		if (!(config->ConsumeType >= 1 && config->ConsumeType <= 4))
		{
			ret = ERROR_ID_CONFIG;
			LOG_ERR("[%s:%d] player[%lu], id:%u, consume_type:%lu", __FUNCTION__, __LINE__, extern_data->player_id, type, config->ConsumeType);
			break;
		}

		if (get_player_donate_remain_count(player) <= 0)
		{
			ret = 2222;
			LOG_ERR("[%s:%d] player[%lu] donate count, count:%u", __FUNCTION__, __LINE__, extern_data->player_id, player->donate_count);
			break;
		}

		{
			//请求扣除消耗
			PROTO_SRV_CHECK_AND_COST_REQ *cost_req = (PROTO_SRV_CHECK_AND_COST_REQ *)get_send_data();
			uint32_t data_len = sizeof(PROTO_SRV_CHECK_AND_COST_REQ) + get_data_len();
			memset(cost_req, 0, data_len);
			cost_req->cost.statis_id = MAGIC_TYPE_GUILD_DONATE;
			switch (config->ConsumeType)
			{
				case 1: //银票
					cost_req->cost.coin = config->ConsumeValue;
					break;
				case 2: //银币
					cost_req->cost.silver = config->ConsumeValue;
					break;
				case 3: //金票
					cost_req->cost.gold = config->ConsumeValue;
					break;
				case 4: //元宝
					cost_req->cost.unbind_gold = config->ConsumeValue;
					break;
				default:
					break;
			}
			cost_req->data_size = get_data_len();
			memcpy(cost_req->data, get_data(), cost_req->data_size);
			fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_GUILDSRV_COST_REQUEST, data_len, 0);
		}
	} while(0);

	if (ret != 0)
	{
		GuildDonateAnswer resp;
		guild_donate_answer__init(&resp);

		resp.result = ret;
		resp.type = type;

		fast_send_msg(&connecter, extern_data, MSG_ID_GUILD_DONATE_ANSWER, guild_donate_answer__pack, resp);
	}

	return 0;
}

static int handle_guild_donate_cost(int data_len, uint8_t *data, int result, EXTERN_DATA *extern_data)
{
	GuildDonateRequest *req = guild_donate_request__unpack(NULL, data_len, data);
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint32_t type = req->type;
	guild_donate_request__free_unpacked(req, NULL);

	int ret = result;
	GuildInfo *guild = NULL;
	bool internal = false;
	AutoReleaseBatchRedisPlayer t1;				
	do
	{
		if (ret != 0)
		{
			break;
		}
		internal = true;

		GuildPlayer *player = get_guild_player(extern_data->player_id);
		if (!player || !player->guild)
		{
			ret = ERROR_ID_GUILD_PLAYER_NOT_JOIN;
			LOG_ERR("[%s:%d] player[%lu] not in guild", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		PlayerRedisInfo *redis_player = get_redis_player(extern_data->player_id, sg_player_key, sg_redis_client, t1);
		if (!redis_player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get redis info failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		guild = player->guild;

		DonationTable *config = get_guild_donate_config(type);
		if (!config)
		{
			ret = ERROR_ID_NO_CONFIG;
			LOG_ERR("[%s:%d] player[%lu] get config failed, id:%u", __FUNCTION__, __LINE__, extern_data->player_id, type);
			break;
		}

		if (get_player_donate_remain_count(player) <= 0)
		{
			ret = 2222;
			LOG_ERR("[%s:%d] player[%lu] donate count, count:%u", __FUNCTION__, __LINE__, extern_data->player_id, player->donate_count);
			break;
		}

		player->donate_count++;
		notify_guild_attr_update(extern_data->player_id, GUILD_ATTR_TYPE__ATTR_DONATE_COUNT, player->donate_count);
		fast_send_msg_base(&conn_node_guildsrv::connecter, extern_data, SERVER_PROTO_GUILD_SYNC_DONATE, 0, 0);

		uint32_t contribute_treasure = 0;
		for (uint32_t i = 0; i < config->n_RewardType; ++i)
		{
			if (config->RewardType[i] == 1) //门宗贡献
			{
				add_player_donation(player, config->RewardValue[i]);
			}
			else if (config->RewardType[i] == 2) //门宗资金
			{
				contribute_treasure = config->RewardValue[i];
				add_guild_treasure(guild, config->RewardValue[i]);
				add_player_contribute_treasure(player, config->RewardValue[i]);
			}
		}

		if (type <= 3)
		{
			add_guild_popularity(guild, sg_guild_donate_popularity[type - 1]);
		}

		uint32_t now = time_helper::get_cached_time() / 1000;
		GuildLog *log = get_usual_insert_log(guild);

		char content[1024];
		Chat chat_req;
		chat__init(&chat_req);
		chat_req.channel = CHANNEL__family;
		chat_req.contain = content;
		uint32_t param_id = 0;

		switch (type)
		{
			case 1:
				{
					log->type = GULT_DONATE1;
					param_id = 161000379;
				}
				break;
			case 2:
				{
					log->type = GULT_DONATE2;
					param_id = 161000380;
				}
				break;
			case 3:
				{
					log->type = GULT_DONATE3;
					param_id = 161000381;
				}
				break;
		}
		{
			snprintf(log->args[0], MAX_GUILD_LOG_ARG_LEN, "%s", redis_player->name);
			snprintf(log->args[1], MAX_GUILD_LOG_ARG_LEN, "%lu", config->ConsumeValue);
			snprintf(log->args[2], MAX_GUILD_LOG_ARG_LEN, "%u", contribute_treasure);

			log->time = now; 
			broadcast_usual_log_add(guild, log);

			ParameterTable *param_config = get_config_by_id(param_id, &parameter_config);
			if (param_config)
			{
				sprintf(content, param_config->parameter2, log->args[0], log->args[1], log->args[2]);
				broadcast_guild_chat(guild, &chat_req);
			}
		}

		save_guild_info(guild);
	} while(0);

	GuildDonateAnswer resp;
	guild_donate_answer__init(&resp);

	resp.result = ret;
	resp.type = type;

	fast_send_msg(&conn_node_guildsrv::connecter, extern_data, MSG_ID_GUILD_DONATE_ANSWER, guild_donate_answer__pack, resp);

	return (internal ? ret : 0);
}

int conn_node_guildsrv::handle_activity_shidamenzong_give_reward_request(EXTERN_DATA *extern_data)
{
	GiveShidamenzongReward *req = give_shidamenzong_reward__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	do
	{
		std::map<uint32_t, uint32_t> master_attachs, mass_attachs;
		std::vector<char *> mail_args;
		std::stringstream ss;
		char sz_rank[12];
		mail_args.push_back(sz_rank);
		std::map<uint32_t, uint32_t> *pAttach = NULL;
		for (size_t i = 0; i < req->n_rewards; ++i)
		{
			master_attachs.clear();
			mass_attachs.clear();
			for (size_t k = 0; k < req->rewards[i]->n_master_reward_id; ++k)
			{
				master_attachs[req->rewards[i]->master_reward_id[k]] += req->rewards[i]->master_reward_num[k];
			}
			for (size_t k = 0; k < req->rewards[i]->n_mass_reward_id; ++k)
			{
				mass_attachs[req->rewards[i]->mass_reward_id[k]] += req->rewards[i]->mass_reward_num[k];
			}

			for (uint32_t rank = req->rewards[i]->start_rank; rank <= req->rewards[i]->stop_rank; ++rank)
			{
				if (rank >= req->n_guild_ids)
				{
					continue;
				}

				GuildInfo *guild = get_guild(req->guild_ids[rank - 1]);
				if (!guild)
				{
					continue;
				}

				ss.str("");
				ss.clear();
				ss << rank;
				ss >> sz_rank;
				for (uint32_t j = 0; j < guild->member_num; ++j)
				{
					GuildPlayer *player = guild->members[j];
					if (player->player_id == guild->master_id)
					{
						pAttach = &master_attachs;
					}
					else
					{
						pAttach = &mass_attachs;
					}

					send_mail(&connecter, player->player_id, 270300043, NULL, NULL, NULL, &mail_args, pAttach, MAGIC_TYPE_SHIDAMENZONG_REWARD);
				}
			}
		}
	} while(0);

	uint32_t *resp = (uint32_t *)get_send_data();
	uint32_t data_len = sizeof(uint32_t);
	memset(resp, 0, data_len);
	*resp = req->activity_id;
	fast_send_msg_base(&conn_node_guildsrv::connecter, extern_data, SERVER_PROTO_ACTIVITY_SHIDAMENZONG_GIVE_REWARD_ANSWER, data_len, 0);

	give_shidamenzong_reward__free_unpacked(req, NULL);

	return 0;
}

