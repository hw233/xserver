#include <stdlib.h>
#include <map>
#include "guild_util.h"
#include "game_event.h"
#include "guild_db.pb-c.h"
#include "mysql_module.h"
#include "redis_util.h"
#include "time_helper.h"
#include "error_code.h"
#include "guild.pb-c.h"
#include "conn_node_guildsrv.h"
#include "msgid.h"
#include "shop.pb-c.h"
#include "guild_battle.pb-c.h"

extern int send_mail(conn_node_base *connecter, uint64_t player_id, uint32_t type,
	char *title, char *sender_name, char *content, std::vector<char *> *args,
	std::map<uint32_t, uint32_t> *attachs, uint32_t statis_id);
extern void resp_guild_info(conn_node_guildsrv *node, EXTERN_DATA *extern_data, uint32_t msg_id, uint32_t result, GuildPlayer *player);

static std::map<uint32_t, GuildInfo*> guild_map; //帮会列表
static std::map<uint64_t, GuildPlayer*> guild_player_map; //玩家列表

uint32_t sg_server_id = 0;
CRedisClient sg_redis_client;
char sg_player_key[64];
char sg_rank_guild_battle_key[64]; //帮战积分排行
char sg_guild_battle_final_key[64]; //帮战决赛参与者
bool guild_battle_opening = false;


struct event second_timer;
struct timeval second_timeout;
struct event five_oclock_timer;
struct timeval five_oclock_timeout;
static const uint32_t week_reset_day = 1 * 24 * 3600; //每周一刷新
static const uint32_t daily_reset_clock = 5 * 3600; //每天五点刷新
static void cb_5clock_timer(evutil_socket_t, short, void* /*arg*/)
{
	handle_daily_reset_timeout();
	GuildAnswer::s_Open = 5;
	GuildAnswer::s_nextOpen = time_helper::get_micro_time() / 1000 + 5 * 3600 * 1000; //10点开始答题 
}

void handle_daily_reset_timeout(void)
{
	uint32_t cur_tick = time_helper::get_micro_time() / 1000 / 1000;
	uint32_t next_tick = time_helper::nextOffsetTime(daily_reset_clock, cur_tick);
	five_oclock_timeout.tv_sec = next_tick - cur_tick;
	five_oclock_timer.ev_callback = cb_5clock_timer;
	add_timer(five_oclock_timeout, &five_oclock_timer, NULL);

	std::vector<GuildInfo*> disband_guilds;
	for (std::map<uint32_t, GuildInfo *>::iterator iter = guild_map.begin(); iter != guild_map.end(); ++iter)
	{
		GuildInfo *guild = iter->second;
		uint32_t expect_time = (guild->maintain_time == 0 ? cur_tick : guild->maintain_time);
		expect_time = time_helper::nextOffsetTime(daily_reset_clock, expect_time);
		if (expect_time > cur_tick)
		{
			continue;
		}

		guild->maintain_time = cur_tick;
		uint32_t building_type = Building_Hall;
		uint32_t building_level = get_building_level(guild, building_type);
		GangsTable *config = get_guild_building_config(building_type, building_level);
		if (!config)
		{
			continue;
		}

		if (guild->popularity < (uint32_t)config->PopularityCost)
		{
			disband_guilds.push_back(guild);
			continue;
		}

		sub_guild_popularity(guild, config->PopularityCost);
		if (guild->treasure >= (uint32_t)config->MaintenanceCosts)
		{
			//扣除维护费用，发放每日奖励
			sub_guild_treasure(guild, config->MaintenanceCosts);
			uint32_t vault_level = get_building_level(guild, Building_Vault);
			GangsTable *vault_config = get_guild_building_config(Building_Vault, vault_level);
			if (vault_config)
			{
				std::map<uint32_t, uint32_t> attachs;
				attachs[201010001] = vault_config->parameter3;
				for (uint32_t i = 0; i < guild->member_num; ++i)
				{
					send_mail(&conn_node_guildsrv::connecter, guild->members[i]->player_id, 270300010, NULL, NULL, NULL, NULL, &attachs, 0);
				}
			}
		}
	}

	for (std::vector<GuildInfo*>::iterator iter = disband_guilds.begin(); iter != disband_guilds.end(); ++iter)
	{
		disband_guild(*iter);
	}

	AutoReleaseBatchRedisPlayer t1;
	for (std::map<uint64_t, GuildPlayer *>::iterator iter = guild_player_map.begin(); iter != guild_player_map.end(); ++iter)
	{
		GuildPlayer *player = iter->second;
		bool save = false;
		uint32_t expect_time = 0;
		if (player->join_time > 0)
		{
			expect_time = player->join_time;
		}
		else if (player->exit_time > 0)
		{
			expect_time = player->exit_time;
		}
		else
		{
			expect_time = cur_tick;
		}

		if (player->week_reset_time == 0)
		{
			player->week_reset_time = time_helper::nextWeek(week_reset_day + daily_reset_clock, expect_time);
		}
		if (cur_tick >= player->week_reset_time)
		{
			player->week_reset_time = time_helper::nextWeek(week_reset_day + daily_reset_clock, cur_tick);
			player->cur_week_donation = 0;
			player->cur_week_treasure = 0;
			player->cur_week_task = 0;
			PlayerRedisInfo *redis_player = get_redis_player(player->player_id, sg_player_key, sg_redis_client, t1);
			if (redis_player)
			{
				player->cur_week_task_config_id = get_guild_build_task_id(redis_player->lv);
			}
			sync_guild_task_to_gamesrv(player);
			{
				AttrMap attrs;
				attrs[GUILD_ATTR_TYPE__ATTR_TASK_COUNT] = player->cur_week_task;
				attrs[GUILD_ATTR_TYPE__ATTR_TASK_AMOUNT] = get_guild_build_task_amount(player->cur_week_task_config_id);
				notify_guild_attrs_update(player->player_id, attrs);
			}
			save = true;
		}

		uint32_t hour = time_helper::get_cur_hour(next_tick);
		bool day_reset = false, week_reset = false, month_reset = false;

		if (player->shop_reset.next_day_time == 0)
		{
			player->shop_reset.next_day_time = time_helper::nextOffsetTime(hour * 3600, expect_time);
		}
		if (player->shop_reset.next_week_time == 0)
		{
			player->shop_reset.next_week_time = time_helper::get_next_timestamp_by_week_old(1, hour, 0, expect_time);
		}
		if (player->shop_reset.next_month_time == 0)
		{
			player->shop_reset.next_month_time = time_helper::get_next_timestamp_by_month_old(1, hour, 0, expect_time);
		}

		if (cur_tick >= player->shop_reset.next_day_time) //每天
		{
			player->shop_reset.next_day_time = time_helper::nextOffsetTime(hour * 3600, cur_tick);
			day_reset = true;
			save = true;
		}
		if (cur_tick >= player->shop_reset.next_week_time) //每周一
		{
			player->shop_reset.next_week_time = time_helper::get_next_timestamp_by_week_old(1, hour, 0, cur_tick);
			week_reset = true;
			save = true;
		}
		if (cur_tick >= player->shop_reset.next_month_time) //每月一号
		{
			player->shop_reset.next_month_time = time_helper::get_next_timestamp_by_month_old(1, hour, 0, cur_tick);
			month_reset = true;
			save = true;
		}
		for (int i = 0; i < MAX_GUILD_GOODS_NUM; ++i)
		{
			GuildGoods *info = &player->goods[i];
			if (info->goods_id == 0)
			{
				continue;
			}

			ShopTable *config = get_config_by_id(info->goods_id, &shop_config);
			if (!config)
			{
				continue;
			}

			if (config->Reset != 1)
			{
				continue;
			}

			if ((config->RestrictionTime == 1 && day_reset) || (config->RestrictionTime == 2 && week_reset) || (config->RestrictionTime == 3 && month_reset))
			{
				info->bought_num = 0;
			}
		}
		if(day_reset)
		{
			for (int i = 0; i < MAX_GUILD_LAND_ACTIVE_NUM; ++i)
			{
				if (player->guild_land_active_reward_id[i] == 0)
					break;
				player->guild_land_active_reward_num[i] = 0;
			}

			player->donate_count = 0;
			notify_guild_attr_update(player->player_id, GUILD_ATTR_TYPE__ATTR_DONATE_COUNT, player->donate_count);
		}

		if (save)
		{
			save_guild_player(player);
		}
	}
}

void cb_second_timer(evutil_socket_t, short, void* /*arg*/)
{
	uint32_t now = time_helper::get_micro_time() / 1000 / 1000;
	for (std::map<uint32_t, GuildInfo*>::iterator iter = guild_map.begin(); iter != guild_map.end(); ++iter)
	{
		GuildInfo *guild = iter->second;
		if (guild->building_upgrade_end > 0 && guild->building_upgrade_end <= now)
		{
			upgrade_building_level(guild);
		}
		if (time_helper::get_micro_time() / 1000 > guild->answer.m_cd)
		{
			guild->answer.OnTimer();
		}
	}

	GuildAnswer::CheckOpenAnswer();

	add_timer(second_timeout, &second_timer, NULL);
}


int dbdata_to_guild_player(DBGuildPlayer *db_player, GuildPlayer *player)
{
	player->donation = db_player->donation;
	player->all_history_donation = db_player->all_history_donation;
	player->cur_history_donation = db_player->cur_history_donation;
	player->office = db_player->office;
	player->join_time = db_player->join_time;
	player->exit_time = db_player->exit_time;
	for (size_t i = 0; i < db_player->n_skills && i < MAX_GUILD_SKILL_NUM; ++i)
	{
		player->skills[i].skill_id = db_player->skills[i]->skill_id;
		player->skills[i].skill_lv = db_player->skills[i]->skill_lv;
	}
	for (size_t i = 0; i < db_player->n_goods && i < MAX_GUILD_GOODS_NUM; ++i)
	{
		player->goods[i].goods_id = db_player->goods[i]->goods_id;
		player->goods[i].bought_num = db_player->goods[i]->bought_num;
	}
	if (db_player->shop_reset)
	{
		player->shop_reset.next_day_time = db_player->shop_reset->next_day_time;
		player->shop_reset.next_week_time = db_player->shop_reset->next_week_time;
		player->shop_reset.next_month_time = db_player->shop_reset->next_month_time;
	}

	player->cur_week_donation = db_player->cur_week_donation;
	player->cur_week_treasure = db_player->cur_week_treasure;
	player->cur_week_task = db_player->cur_week_task;
	player->cur_week_task_config_id = db_player->cur_week_task_config_id;
	player->week_reset_time = db_player->week_reset_time;
	player->battle_score = db_player->battle_score;
	player->act_battle_score = db_player->act_battle_score;
	for (uint32_t i = 0; i < db_player->n_guild_land_active_reward_id; ++i)
	{
		player->guild_land_active_reward_id[i] = db_player->guild_land_active_reward_id[i];
		player->guild_land_active_reward_num[i] = db_player->guild_land_active_reward_num[i];
	}
	for (size_t i = 0; i < db_player->n_level_gift && i < MAX_GUILD_LEVEL; ++i)
	{
		player->level_gift[i] = db_player->level_gift[i];
	}

	return 0;
}

int dbdata_to_guild(DBGuild *db_guild, GuildInfo *guild)
{
	guild->rename_time = db_guild->rename_time;
	guild->treasure = db_guild->treasure;
	guild->build_board = db_guild->build_board;
	guild->maintain_time = db_guild->maintain_time;
	for (size_t i = 0; i < db_guild->n_buildings && i < MAX_GUILD_BUILDING_NUM; ++i)
	{
		guild->buildings[i].level = db_guild->buildings[i]->level;
	}
	guild->building_upgrade_id = db_guild->building_upgrade_id;
	guild->building_upgrade_end = db_guild->building_upgrade_end;
	for (size_t i = 0; i < db_guild->n_skills && i < MAX_GUILD_SKILL_NUM; ++i)
	{
		guild->skills[i].skill_id = db_guild->skills[i]->skill_id;
		guild->skills[i].skill_lv = db_guild->skills[i]->skill_lv;
	}
	guild->battle_score = db_guild->battle_score;
	for (size_t i = 0; i < db_guild->n_permissions && i < MAX_GUILD_OFFICE; ++i)
	{
		guild->permissions[i].office = db_guild->permissions[i]->office;
		for (size_t j = 0; j < db_guild->permissions[i]->n_permissions && j < GOPT_END; ++j)
		{
			guild->permissions[i].permission[j] = db_guild->permissions[i]->permissions[j];
		}
	}
	for (size_t i = 0; i < db_guild->n_usual_logs && i < MAX_GUILD_LOG_NUM; ++i)
	{
		guild->usual_logs[i].type = db_guild->usual_logs[i]->type;
		guild->usual_logs[i].time = db_guild->usual_logs[i]->time;
		for (size_t j = 0; j < db_guild->usual_logs[i]->n_args && j < MAX_GUILD_LOG_ARG_NUM; ++j)
		{
			strncpy(guild->usual_logs[i].args[j], db_guild->usual_logs[i]->args[j], MAX_GUILD_LOG_ARG_LEN); 
		}
	}
	for (size_t i = 0; i < db_guild->n_important_logs && i < MAX_GUILD_LOG_NUM; ++i)
	{
		guild->important_logs[i].type = db_guild->important_logs[i]->type;
		guild->important_logs[i].time = db_guild->important_logs[i]->time;
		for (size_t j = 0; j < db_guild->important_logs[i]->n_args && j < MAX_GUILD_LOG_ARG_NUM; ++j)
		{
			strncpy(guild->important_logs[i].args[j], db_guild->important_logs[i]->args[j], MAX_GUILD_LOG_ARG_LEN); 
		}
	}
	guild->bonfire_open_time = db_guild->bonfire_open_time;
	return 0;
}

int pack_guild_info(GuildInfo *guild, uint8_t *out_data)
{
	DBGuild db_info;
	DBGuild *db_guild = &db_info;
	dbguild__init(&db_info);

	DBGuildBuilding building_data[MAX_GUILD_BUILDING_NUM];
	DBGuildBuilding* building_point[MAX_GUILD_BUILDING_NUM];

	DBGuildSkill skill_data[MAX_GUILD_SKILL_NUM];
	DBGuildSkill* skill_point[MAX_GUILD_SKILL_NUM];

	DBGuildPermission  permission_data[MAX_GUILD_OFFICE];
	DBGuildPermission* permission_point[MAX_GUILD_OFFICE];
	DBGuildLog  usual_log_data[MAX_GUILD_LOG_NUM];
	DBGuildLog* usual_log_point[MAX_GUILD_LOG_NUM];
	char*       usual_log_args[MAX_GUILD_LOG_NUM][MAX_GUILD_LOG_NUM];
	DBGuildLog  important_log_data[MAX_GUILD_LOG_NUM];
	DBGuildLog* important_log_point[MAX_GUILD_LOG_NUM];
	char*       important_log_args[MAX_GUILD_LOG_NUM][MAX_GUILD_LOG_NUM];

	db_guild->rename_time = guild->rename_time;
	db_guild->treasure = guild->treasure;
	db_guild->build_board = guild->build_board;
	db_guild->maintain_time = guild->maintain_time;
	db_guild->n_buildings = 0;
	db_guild->buildings = building_point;
	for (int i = 0; i < MAX_GUILD_BUILDING_NUM; ++i)
	{
		building_point[db_guild->n_buildings] = &building_data[db_guild->n_buildings];
		dbguild_building__init(building_point[db_guild->n_buildings]);
		building_point[db_guild->n_buildings]->level = guild->buildings[i].level;
		db_guild->n_buildings++;
	}
	db_guild->building_upgrade_id = guild->building_upgrade_id;
	db_guild->building_upgrade_end = guild->building_upgrade_end;
	db_guild->skills = skill_point;
	db_guild->n_skills = 0;
	for (int i = 0; i < MAX_GUILD_SKILL_NUM; ++i)
	{
		if (guild->skills[i].skill_id == 0)
		{
			break;
		}

		skill_point[db_guild->n_skills] = &skill_data[db_guild->n_skills];
		dbguild_skill__init(skill_point[db_guild->n_skills]);
		skill_data[db_guild->n_skills].skill_id = guild->skills[i].skill_id;
		skill_data[db_guild->n_skills].skill_lv = guild->skills[i].skill_lv;
		db_guild->n_skills++;
	}
	db_guild->battle_score = guild->battle_score;
	db_guild->permissions = permission_point;
	db_guild->n_permissions = 0;
	for (int i = 0; i < MAX_GUILD_OFFICE; ++i)
	{
		permission_point[db_guild->n_permissions] = &permission_data[db_guild->n_permissions];
		dbguild_permission__init(&permission_data[db_guild->n_permissions]);
		permission_data[db_guild->n_permissions].office = guild->permissions[i].office;
		permission_data[db_guild->n_permissions].permissions = guild->permissions[i].permission;
		permission_data[db_guild->n_permissions].n_permissions = GOPT_END;
		db_guild->n_permissions++;
	}
	db_guild->usual_logs = usual_log_point;
	db_guild->n_usual_logs = 0;
	for (int i = 0; i < MAX_GUILD_LOG_NUM; ++i)
	{
		if (guild->usual_logs[i].type == 0)
		{
			break;
		}
		usual_log_point[db_guild->n_usual_logs] = &usual_log_data[db_guild->n_usual_logs];
		dbguild_log__init(&usual_log_data[db_guild->n_usual_logs]);
		usual_log_data[db_guild->n_usual_logs].type = guild->usual_logs[i].type;
		usual_log_data[db_guild->n_usual_logs].time = guild->usual_logs[i].time;
		usual_log_data[db_guild->n_usual_logs].args = usual_log_args[db_guild->n_usual_logs];
		usual_log_data[db_guild->n_usual_logs].n_args = 0;
		for (int j = 0; j < MAX_GUILD_LOG_ARG_NUM; ++j)
		{
			if (guild->usual_logs[i].args[j][0] == '\0')
			{
				break;
			}
			usual_log_data[db_guild->n_usual_logs].args[usual_log_data[db_guild->n_usual_logs].n_args] = guild->usual_logs[i].args[j];
			usual_log_data[db_guild->n_usual_logs].n_args++;
		}
		db_guild->n_usual_logs++;
	}
	db_guild->important_logs = important_log_point;
	db_guild->n_important_logs = 0;
	for (int i = 0; i < MAX_GUILD_LOG_NUM; ++i)
	{
		if (guild->important_logs[i].type == 0)
		{
			break;
		}
		important_log_point[db_guild->n_important_logs] = &important_log_data[db_guild->n_important_logs];
		dbguild_log__init(&important_log_data[db_guild->n_important_logs]);
		important_log_data[db_guild->n_important_logs].type = guild->important_logs[i].type;
		important_log_data[db_guild->n_important_logs].time = guild->important_logs[i].time;
		important_log_data[db_guild->n_important_logs].args = important_log_args[db_guild->n_important_logs];
		important_log_data[db_guild->n_important_logs].n_args = 0;
		for (int j = 0; j < MAX_GUILD_LOG_ARG_NUM; ++j)
		{
			if (guild->important_logs[i].args[j][0] == '\0')
			{
				break;
			}
			important_log_data[db_guild->n_important_logs].args[important_log_data[db_guild->n_important_logs].n_args] = guild->important_logs[i].args[j];
			important_log_data[db_guild->n_important_logs].n_args++;
		}
		db_guild->n_important_logs++;
	}
	db_guild->bonfire_open_time = guild->bonfire_open_time;

	return dbguild__pack(&db_info, out_data);
}

int save_guild_info(GuildInfo *guild)
{
	static char save_sql[64 * 1024 + 300];
	static uint8_t save_data[64 * 1024 + 1];
	uint64_t effect = 0;
	int len;
	char *p;
	
	size_t data_size = pack_guild_info(guild, save_data);
	len = sprintf(save_sql, "update guild set `level` = %u, `comm_data` = \'", get_guild_level(guild));
	p = save_sql + len;
	p += escape_string(p, (const char *)save_data, data_size);
	len = sprintf(p, "\' where guild_id = %u", guild->guild_id);

	query(const_cast<char*>("set names utf8"), 1, NULL);
	query(save_sql, 1, &effect);	
	if (effect != 1) 
	{
		LOG_ERR("[%s:%d] save guild %u failed", __FUNCTION__, __LINE__, guild->guild_id);
		return -1;
	}

	return 0;
}

int pack_guild_player(GuildPlayer *player, uint8_t *out_data)
{
	DBGuildPlayer db_info;
	DBGuildPlayer *db_player = &db_info;
	dbguild_player__init(db_player);

	DBGuildSkill skill_data[MAX_GUILD_SKILL_NUM];
	DBGuildSkill* skill_point[MAX_GUILD_SKILL_NUM];

	DBGuildGoods goods_data[MAX_GUILD_GOODS_NUM];
	DBGuildGoods* goods_point[MAX_GUILD_GOODS_NUM];

	DBGuildShopReset shop_reset_data;
	dbguild_shop_reset__init(&shop_reset_data);

	db_player->donation = player->donation;
	db_player->all_history_donation = player->all_history_donation;
	db_player->cur_history_donation = player->cur_history_donation;
	db_player->office = player->office;
	db_player->join_time = player->join_time;
	db_player->exit_time = player->exit_time;
	db_player->skills = skill_point;
	db_player->n_skills = 0;
	for (int i = 0; i < MAX_GUILD_SKILL_NUM; ++i)
	{
		if (player->skills[i].skill_id == 0)
		{
			break;
		}

		skill_point[db_player->n_skills] = &skill_data[db_player->n_skills];
		dbguild_skill__init(skill_point[db_player->n_skills]);
		skill_data[db_player->n_skills].skill_id = player->skills[i].skill_id;
		skill_data[db_player->n_skills].skill_lv = player->skills[i].skill_lv;
		db_player->n_skills++;
	}
	db_player->goods = goods_point;
	db_player->n_goods = 0;
	for (int i = 0; i < MAX_GUILD_GOODS_NUM; ++i)
	{
		if (player->goods[i].goods_id == 0)
		{
			break;
		}

		goods_point[db_player->n_goods] = &goods_data[db_player->n_goods];
		dbguild_goods__init(goods_point[db_player->n_goods]);
		goods_data[db_player->n_goods].goods_id = player->goods[i].goods_id;
		goods_data[db_player->n_goods].bought_num = player->goods[i].bought_num;
		db_player->n_goods++;
	}
	db_player->shop_reset = &shop_reset_data;
	shop_reset_data.next_day_time = player->shop_reset.next_day_time;
	shop_reset_data.next_week_time = player->shop_reset.next_week_time;
	shop_reset_data.next_month_time = player->shop_reset.next_month_time;

	db_player->cur_week_donation = player->cur_week_donation;
	db_player->cur_week_treasure = player->cur_week_treasure;
	db_player->cur_week_task = player->cur_week_task;
	db_player->cur_week_task_config_id = player->cur_week_task_config_id;
	db_player->week_reset_time = player->week_reset_time;
	db_player->battle_score = player->battle_score;
	db_player->act_battle_score = player->act_battle_score;
	int i;
	for (i = 0; i < MAX_GUILD_LAND_ACTIVE_NUM; ++i)
	{
		if (player->guild_land_active_reward_id[i] == 0)
			break;
	}
	db_info.n_guild_land_active_reward_id = db_info.n_guild_land_active_reward_num = i;
	db_info.guild_land_active_reward_id = &player->guild_land_active_reward_id[0];
	db_info.guild_land_active_reward_num = &player->guild_land_active_reward_num[0];
	db_player->level_gift = player->level_gift;
	db_player->n_level_gift = MAX_GUILD_LEVEL;

	return dbguild_player__pack(db_player, out_data);
}

int save_guild_player(GuildPlayer *player)
{
	static char save_sql[64 * 1024 + 300];
	static uint8_t save_data[64 * 1024 + 1];
	uint64_t effect = 0;
	char *p;
	
	uint32_t guild_id = (player->guild ? player->guild->guild_id : 0);
	size_t data_size = pack_guild_player(player, save_data);
	p = save_sql;
	p += sprintf(save_sql, "replace guild_player set `player_id` = %lu, `guild_id` = %u, `comm_data` = \'", player->player_id, guild_id);
	p += escape_string(p, (const char *)save_data, data_size);
	p += sprintf(p, "\'");

	query(const_cast<char*>("set names utf8"), 1, NULL);
	query(save_sql, 1, &effect);	
	if (effect != 1 && effect != 2) 
	{
		LOG_ERR("[%s:%d] save guild player %lu failed, error:%s", __FUNCTION__, __LINE__, player->player_id, mysql_error());
		return -1;
	}

	return 0;
}

class AutoReleaseDbGuildPlayer
{
public:
	AutoReleaseDbGuildPlayer(DBGuildPlayer *db) : pointer(db) {}
	~AutoReleaseDbGuildPlayer() { dbguild_player__free_unpacked(pointer, NULL); }
private:
	DBGuildPlayer *pointer;
};

int load_all_guild_player(void)
{
	char sql[1024];
	unsigned long *lengths;	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;

	sprintf(sql, "select `player_id`, `guild_id`, `comm_data` from guild_player;");

	res = query(sql, 1, NULL);
	if (!res)
	{
		LOG_ERR("[%s:%d] query failed, sql: %s", __FUNCTION__, __LINE__, sql);
		return -1;
	}

	while (true)
	{
		row = fetch_row(res);
		if (!row)
		{
//			LOG_ERR("[%s:%d] query sql fetch row failed, sql: %s", __FUNCTION__, __LINE__, sql);
			break;
		}

		uint64_t player_id = strtoull(row[0], NULL, 10);
		uint32_t guild_id = strtoul(row[1], NULL, 10);
		lengths = mysql_fetch_lengths(res);
		DBGuildPlayer *db_player = dbguild_player__unpack(NULL, lengths[2], (uint8_t *)row[2]);
		if (!db_player)
		{
			LOG_ERR("[%s:%d] unpack guild player failed, player_id:%lu", __FUNCTION__, __LINE__, player_id);
			continue;
		}
		AutoReleaseDbGuildPlayer release_player(db_player);

		GuildPlayer *player = (GuildPlayer *)malloc(sizeof(GuildPlayer));
		if (!player)
		{
			LOG_ERR("[%s:%d] malloc guild player failed, player_id:%lu", __FUNCTION__, __LINE__, player_id);
			continue;
		}
		memset(player, 0, sizeof(GuildPlayer));

		dbdata_to_guild_player(db_player, player);
		player->player_id = player_id;
		player->guild = get_guild(guild_id);
		if (player->guild)
		{
			player->guild->members[player->guild->member_num] = player;
			player->guild->member_num++;
		}

		guild_player_map[player_id] = player;
	}

	free_query(res);

	return 0;
}

class AutoReleaseDbGuild
{
public:
	AutoReleaseDbGuild(DBGuild *db) : pointer(db) {}
	~AutoReleaseDbGuild() { dbguild__free_unpacked(pointer, NULL); }
private:
	DBGuild *pointer;
};

int load_all_guilds(void)
{
	uint32_t now = time_helper::get_micro_time() / 1000 / 1000;
	char sql[1024];
	unsigned long *lengths;	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;

	sprintf(sql, "select `guild_id`, `icon`, `name`, `master_id`, `popularity`, `approve_state`, `recruit_state`, `recruit_notice`, `announcement`, `comm_data`, `zhenying` from guild;");

	res = query(sql, 1, NULL);
	if (!res)
	{
		LOG_ERR("[%s:%d] query failed, sql: %s", __FUNCTION__, __LINE__, sql);
		return -1;
	}

	while (true)
	{
		row = fetch_row(res);
		if (!row)
		{
//			LOG_ERR("[%s:%d] query sql fetch row failed, sql: %s", __FUNCTION__, __LINE__, sql);
			break;
		}

		uint32_t guild_id = strtoul(row[0], NULL, 10);
		lengths = mysql_fetch_lengths(res);
		DBGuild *db_guild = dbguild__unpack(NULL, lengths[9], (uint8_t *)row[9]);
		if (!db_guild)
		{
			LOG_ERR("[%s:%d] unpack guild failed, guild_id:%u", __FUNCTION__, __LINE__, guild_id);
			continue;
		}
		AutoReleaseDbGuild release_guild(db_guild);

		GuildInfo *guild = (GuildInfo *)malloc(sizeof(GuildInfo));
		if (!guild)
		{
			LOG_ERR("[%s:%d] malloc guild failed, guild_id:%u", __FUNCTION__, __LINE__, guild_id);
			continue;
		}
		memset(guild, 0, sizeof(GuildInfo));

		dbdata_to_guild(db_guild, guild);
		guild->guild_id = guild_id;
		guild->icon = strtoul(row[1], NULL, 10);
		memcpy(guild->name, row[2], lengths[2]);
		guild->master_id = strtoull(row[3], NULL, 10);
		guild->popularity = strtoul(row[4], NULL, 10);
		guild->approve_state = strtoul(row[5], NULL, 10);
		guild->recruit_state = strtoul(row[6], NULL, 10);
		memcpy(guild->recruit_notice, row[7], lengths[7]);
		memcpy(guild->announcement, row[8], lengths[8]);
		guild->zhenying = strtoul(row[10], NULL, 10);
		
		if (guild->zhenying == 0)
		{
			AutoReleaseRedisPlayer release_master;
			PlayerRedisInfo *redis_master = get_redis_player(guild->master_id, sg_player_key, sg_redis_client, release_master);
			if (redis_master)
			{
				guild->zhenying = redis_master->zhenying;
			}
		}
		if (guild->maintain_time == 0)
		{
			guild->maintain_time = now;
		}

		guild_map[guild_id] = guild;
	}

	free_query(res);

	return 0;
}

void load_guild_module(void)
{
	//先加载帮会数据，再加载玩家数据
	load_all_guilds();
	load_all_guild_player();

	sync_all_guild_to_gamesrv();
}

void sync_all_guild_to_gamesrv(void)
{
	PROTO_SYNC_ALL_GUILD *req = (PROTO_SYNC_ALL_GUILD*)conn_node_guildsrv::get_send_buf(SERVER_PROTO_GUILD_SYNC_ALL, 0);
	req->guild_num = 0;
	for (std::map<uint32_t, GuildInfo*>::iterator iter = guild_map.begin(); iter != guild_map.end(); ++iter)
	{
		GuildInfo *guild = iter->second;
		ProtoGuildInfo *info = &req->guilds[req->guild_num];
		memset(info, 0, sizeof(ProtoGuildInfo));
		info->guild_id = guild->guild_id;
		memcpy(info->name, guild->name, sizeof(guild->name));
		info->zhenying = guild->zhenying;
		info->master_id = guild->master_id;
		for(size_t i = 0; i < MAX_GUILD_MEMBER_NUM; i++)
		{
			if(guild->members[i] == NULL)
				break;
			info->player_data[i].player_id = guild->members[i]->player_id;
		}
		req->guild_num++;
	}
	req->head.len = ENDION_FUNC_4(sizeof(PROTO_SYNC_ALL_GUILD) + req->guild_num * sizeof(ProtoGuildInfo));
	if (conn_node_guildsrv::connecter.send_one_msg(&req->head, 1) != (int)ENDION_FUNC_4(req->head.len))
	{
		LOG_ERR("[%s:%d] send to game_srv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
}


// PlayerRedisInfo *get_redis_player(uint64_t player_id)
// {
// 	CRedisClient &rc = sg_redis_client;
// 	static uint8_t data_buffer[32 * 1024];
// 	int data_len = 32 * 1024;
// 	char field[64];
// 	sprintf(field, "%lu", player_id);
// 	int ret = rc.hget_bin(sg_player_key, field, (char *)data_buffer, &data_len);
// 	if (ret == 0)
// 	{
// 		return player_redis_info__unpack(NULL, data_len, data_buffer);
// 	}

// 	return NULL;
// }

// int get_more_redis_player(std::set<uint64_t> &player_ids, std::map<uint64_t, PlayerRedisInfo*> &redis_players)
// {
// 	if (player_ids.size() == 0)
// 	{
// 		return 0;
// 	}

// 	std::vector<std::relation_three<uint64_t, char*, int> > player_infos;
// 	for (std::set<uint64_t>::iterator iter = player_ids.begin(); iter != player_ids.end(); ++iter)
// 	{
// 		std::relation_three<uint64_t, char*, int> tmp(*iter, NULL, 0);
// 		player_infos.push_back(tmp);
// 	}

// 	int ret = sg_redis_client.get(sg_player_key, player_infos);
// 	if (ret != 0)
// 	{
// 		LOG_ERR("[%s:%d] hmget failed, ret:%d", __FUNCTION__, __LINE__, ret);
// 		return -1;
// 	}

// 	for (std::vector<std::relation_three<uint64_t, char*, int> >::iterator iter = player_infos.begin(); iter != player_infos.end(); ++iter)
// 	{
// 		PlayerRedisInfo *redis_player = player_redis_info__unpack(NULL, iter->three, (uint8_t*)iter->second);
// 		if (!redis_player)
// 		{
// 			ret = -1;
// 			LOG_ERR("[%s:%d] unpack redis failed, player_id:%lu", __FUNCTION__, __LINE__, iter->first);
// 			break;
// 		}

// 		redis_players[iter->first] = redis_player;
// 	}

// 	for (std::vector<std::relation_three<uint64_t, char*, int> >::iterator iter = player_infos.begin(); iter != player_infos.end(); ++iter)
// 	{
// 		free(iter->second);
// 	}

// 	return ret;
// }

PlayerRedisInfo *find_redis_from_map(std::map<uint64_t, PlayerRedisInfo*> &redis_players, uint64_t player_id)
{
	std::map<uint64_t, PlayerRedisInfo*>::iterator iter = redis_players.find(player_id);
	if (iter != redis_players.end())
	{
		return iter->second;
	}
	return NULL;
}

void update_redis_player_guild(GuildPlayer *player)
{
	PlayerRedisInfo info;
	player_redis_info__init(&info);
	if (player->guild)
	{
		info.guild_id = player->guild->guild_id;
		info.guild_name = player->guild->name;
	}

	EXTERN_DATA extern_data;
	extern_data.player_id = player->player_id;

	//在数据开头增加一个类型
	uint8_t *pData = conn_node_base::get_send_data();
	size_t data_len = player_redis_info__pack(&info, pData + sizeof(uint32_t));
	if (data_len != (size_t)-1)
	{
		*((uint32_t*)pData) = 2;
		data_len += sizeof(uint32_t);
		fast_send_msg_base(&conn_node_guildsrv::connecter, &extern_data, SERVER_PROTO_REFRESH_PLAYER_REDIS_INFO, data_len, 0);
	}
}

std::map<uint32_t, GuildInfo*> &get_all_guild(void)
{
	return guild_map;
}

GuildInfo *get_guild(uint32_t guild_id)
{
	std::map<uint32_t, GuildInfo*>::iterator iter = guild_map.find(guild_id);
	if (iter != guild_map.end())
	{
		return iter->second;
	}

	return NULL;
}

GuildPlayer *get_guild_player(uint64_t player_id)
{
	std::map<uint64_t, GuildPlayer*>::iterator iter = guild_player_map.find(player_id);
	if (iter != guild_player_map.end())
	{
		return iter->second;
	}

	return NULL;
}

int get_guild_level(GuildInfo *guild)
{
	return get_building_level(guild, Building_Hall);
}

int get_guild_max_member(GuildInfo *guild)
{
	uint32_t hall_level = get_building_level(guild, Building_Hall);
	GangsTable *config = get_guild_building_config(Building_Hall, hall_level);
	if (config)
	{
		return config->parameter1;
	}

	return 0;
}

bool check_guild_join_cd(GuildPlayer *player) //true不在CD中
{
	uint32_t now = time_helper::get_cached_time() / 1000;
	return (player->exit_time == 0 || player->exit_time + sg_guild_join_cd < now);
}

bool check_guild_name(std::string &name)
{
	char sql[1024];
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;
	int len;
	char *p;

	len = sprintf(sql, "select `guild_id` from guild where `name` = \'");
	p = sql + len;
	p += escape_string(p, name.c_str(), name.size());
	len = sprintf(p, "\'");

	query(const_cast<char*>("set names utf8"), 1, NULL);
	res = query(sql, 1, NULL);
	if (!res)
	{
		LOG_ERR("[%s:%d] query failed, sql: %s", __FUNCTION__, __LINE__, sql);
		return false;
	}

	row = fetch_row(res);
	if (row)
	{
		free_query(res);
		return true;
	}
	free_query(res);

	return false;
}

static void clear_timeout_player_join_apply(void)
{
	uint32_t now = time_helper::get_cached_time() / 1000;
	char sql[256];
	sprintf(sql, "delete from guild_join where `apply_time` <= FROM_UNIXTIME(%u)", (now - sg_guild_apply_join_cd));
	query(sql, 1, NULL);
}

void delete_player_join_apply(uint64_t player_id)
{
	char sql[256];
	sprintf(sql, "delete from guild_join where `player_id` = %lu", player_id);
	query(sql, 1, NULL);
}

void delete_guild_join_apply(uint32_t guild_id)
{
	char sql[256];
	sprintf(sql, "delete from guild_join where `guild_id` = %u", guild_id);
	query(sql, 1, NULL);
}

void delete_guild_player_join_apply(uint32_t guild_id, uint64_t player_id)
{
	char sql[256];
	sprintf(sql, "delete from guild_join where `guild_id` = %u and `player_id` = %lu", guild_id, player_id);
	query(sql, 1, NULL);
}

bool check_player_applied_join(uint64_t player_id, uint32_t guild_id)
{
	clear_timeout_player_join_apply();

	char sql[1024];
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;

	sprintf(sql, "select `apply_time` from guild_join where `player_id` = %lu and `guild_id` = %u", player_id, guild_id);

	res = query(sql, 1, NULL);
	if (!res)
	{
		LOG_ERR("[%s:%d] query failed, sql: %s", __FUNCTION__, __LINE__, sql);
		return false;
	}

	row = fetch_row(res);
	if (row)
	{
		free_query(res);
		return true;
	}
	free_query(res);

	return false;
}

int insert_player_join_apply(uint64_t player_id, uint32_t guild_id)
{
	uint64_t effect = 0;
	char sql[256];
	sprintf(sql, "insert into guild_join set `player_id` = %lu, `guild_id` = %u, apply_time = now()", player_id, guild_id);
	query(sql, 1, &effect);
	if (effect != 1)
	{
		LOG_ERR("[%s:%d] query failed, sql: %s, error:%s", __FUNCTION__, __LINE__, sql, mysql_error());
		return ERROR_ID_MYSQL_QUERY;
	}

	//通知帮主？

	return 0;
}

int get_player_join_apply(uint64_t player_id, std::vector<uint32_t> &applyIds)
{
	clear_timeout_player_join_apply();

	char sql[1024];
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;

	sprintf(sql, "select `guild_id` from guild_join where `player_id` = %lu", player_id);

	res = query(sql, 1, NULL);
	if (!res)
	{
		LOG_ERR("[%s:%d] query failed, sql: %s", __FUNCTION__, __LINE__, sql);
		return -1;
	}

	while (true)
	{
		row = fetch_row(res);
		if (!row)
		{
			break;
		}

		uint32_t guild_id = atoi(row[0]);
		applyIds.push_back(guild_id);
	}
	free_query(res);

	return 0;
}

int get_guild_join_apply(uint32_t guild_id, std::vector<uint64_t> &applyIds)
{
	clear_timeout_player_join_apply();

	char sql[1024];
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;

	sprintf(sql, "select `player_id` from guild_join where `guild_id` = %u", guild_id);

	res = query(sql, 1, NULL);
	if (!res)
	{
		LOG_ERR("[%s:%d] query failed, sql: %s", __FUNCTION__, __LINE__, sql);
		return -1;
	}

	while (true)
	{
		row = fetch_row(res);
		if (!row)
		{
			break;
		}

		uint64_t player_id = strtoull(row[0], NULL, 10);
		applyIds.push_back(player_id);
	}
	free_query(res);

	return 0;
}

void broadcast_guild_join_apply(GuildInfo *guild, uint64_t player_id, PlayerRedisInfo *redis_player)
{
	GuildJoinPlayerData nty;
	guild_join_player_data__init(&nty);

	nty.playerid = player_id;
	nty.name = redis_player->name;
	nty.job = redis_player->job;
	nty.level = redis_player->lv;
	nty.fc = redis_player->fighting_capacity;

	std::vector<uint64_t> player_ids;
	for (uint32_t i = 0; i < guild->member_num; ++i)
	{
		if (player_has_permission(guild->members[i], GOPT_DEAL_JOIN))
		{
			player_ids.push_back(guild->members[i]->player_id);
		}
	}
	
	conn_node_guildsrv::broadcast_message(MSG_ID_GUILD_JOIN_NOTIFY, &nty, (pack_func)guild_join_player_data__pack, player_ids);
}

int save_guild_switch(GuildInfo *guild, uint32_t type)
{
	char save_sql[300];
	uint64_t effect = 0;

	if (type == GUILD_SWITCH_TYPE__APPROVE_SWITCH)
	{
		sprintf(save_sql, "update guild set `approve_state` = %u where `guild_id` = %u", guild->approve_state, guild->guild_id);
	}
	else
	{
		sprintf(save_sql, "update guild set `recruit_state` = %u where `guild_id` = %u", guild->recruit_state, guild->guild_id);
	}

	query(save_sql, 1, &effect);	
	if (effect != 1) 
	{
		LOG_ERR("[%s:%d] update guild failed, sql:%s, guild_id:%u, error:%s", __FUNCTION__, __LINE__, save_sql, guild->guild_id, mysql_error());
		return -1;
	}

	return 0;
}

int save_announcement(GuildInfo *guild, uint32_t type)
{
	char save_sql[500];
	uint64_t effect = 0;
	char *p;

	p = save_sql;
	char *pWord = NULL;
	if (type == GUILD_WORDS_TYPE__RECRUIT_NOTICE)
	{
		p += sprintf(save_sql, "update guild set `recruit_notice` = \'");
		pWord = guild->recruit_notice;
	}
	else
	{
		p += sprintf(save_sql, "update guild set `announcement` = \'");
		pWord = guild->announcement;
	}

	p += escape_string(p, (const char *)pWord, strlen(pWord));
	p += sprintf(p, "\' where `guild_id` = %u", guild->guild_id);

	query(const_cast<char*>("set names utf8"), 1, NULL);
	query(save_sql, 1, &effect);	
	if (effect != 1) 
	{
		LOG_ERR("[%s:%d] update guild failed, guild_id:%u, error:%s", __FUNCTION__, __LINE__, guild->guild_id, mysql_error());
		return -1;
	}

	return 0;
}

void replace_guild_master(uint32_t guild_id, uint64_t master_id)
{
	uint64_t effect = 0;
	char sql[256];
	sprintf(sql, "update guild set `master_id` = %lu where `guild_id` = %u", master_id, guild_id);
	query(sql, 1, &effect);
	if (effect != 1)
	{
		LOG_ERR("[%s:%d] update guild failed, guild_id:%u, error:%s", __FUNCTION__, __LINE__, guild_id, mysql_error());
		return ;
	}
}

int save_guild_name(GuildInfo *guild)
{
	char save_sql[500];
	uint64_t effect = 0;
	char *p = NULL;

	p = save_sql;
	p += sprintf(save_sql, "update guild set `name` = \'");

	p += escape_string(p, (const char *)guild->name, strlen(guild->name));
	p += sprintf(p, "\' where `guild_id` = %u", guild->guild_id);

	query(const_cast<char*>("set names utf8"), 1, NULL);
	query(save_sql, 1, &effect);	
	if (effect != 1) 
	{
		LOG_ERR("[%s:%d] update guild failed, sql:%s, guild_id:%u, error:%s", __FUNCTION__, __LINE__, save_sql, guild->guild_id, mysql_error());
		return -1;
	}

	return 0;
}

int delete_guild(uint32_t guild_id)
{
	delete_guild_join_apply(guild_id);

	char save_sql[500];
	uint64_t effect = 0;

	sprintf(save_sql, "delete from guild where `guild_id` = %u", guild_id);
	query(save_sql, 1, &effect);	
	if (effect != 1) 
	{
		LOG_ERR("[%s:%d] delete guild failed, sql:%s, guild_id:%u, error:%s", __FUNCTION__, __LINE__, save_sql, guild_id, mysql_error());
		return -1;
	}

	return 0;
}

int save_guild_popularity(GuildInfo *guild)
{
	uint64_t effect = 0;
	char sql[256];
	sprintf(sql, "update guild set `popularity` = %u where `guild_id` = %u", guild->popularity, guild->guild_id);
	query(sql, 1, &effect);
	if (effect != 1)
	{
		LOG_ERR("[%s:%d] update guild failed, guild_id:%u, error:%s", __FUNCTION__, __LINE__, guild->guild_id, mysql_error());
		return -1;
	}

	return 0;
}

bool player_has_permission(GuildPlayer *player, uint32_t type)
{
	if (!player->guild)
	{
		return false;
	}
	if (type >= GOPT_END)
	{
		return false;
	}

	for (int i = 0; i < MAX_GUILD_OFFICE; ++i)
	{
		if (player->guild->permissions[i].office == player->office)
		{
			return (player->guild->permissions[i].permission[type] == 1);
		}
	}

	return false;
}

static void clear_timeout_player_invite(void)
{
	uint32_t now = time_helper::get_cached_time() / 1000;
	char sql[256];
	sprintf(sql, "delete from guild_invite where `invite_time` <= FROM_UNIXTIME(%u)", (now - sg_guild_invite_cd));
	query(sql, 1, NULL);
}

int insert_one_invite(uint64_t inviter_id, uint64_t invitee_id, uint32_t guild_id) //插入一条邀请信息
{
	uint64_t effect = 0;
	char sql[256];
	sprintf(sql, "insert into guild_invite set `inviter_id` = %lu, `invitee_id` = %lu, `guild_id` = %u, invite_time = now()", inviter_id, invitee_id, guild_id);
	query(sql, 1, &effect);
	if (effect != 1)
	{
		LOG_ERR("[%s:%d] query failed, sql: %s, error:%s", __FUNCTION__, __LINE__, sql, mysql_error());
		return ERROR_ID_MYSQL_QUERY;
	}
	return 0;
}

void mark_invite_deal(uint64_t inviter_id, uint64_t invitee_id, uint32_t guild_id, uint32_t deal_type)
{
	uint64_t effect = 0;
	char sql[256];
	sprintf(sql, "update guild_invite set `deal_type` = %u where `inviter_id` = %lu and `invitee_id` = %lu and `guild_id` = %u", deal_type, inviter_id, invitee_id, guild_id);
	query(sql, 1, &effect);
	if (effect != 1)
	{
		LOG_ERR("[%s:%d] query failed, sql: %s, error:%s", __FUNCTION__, __LINE__, sql, mysql_error());
	}
}

uint32_t get_invite_cd_time(uint64_t inviter_id, uint64_t invitee_id, uint32_t guild_id)
{
	clear_timeout_player_invite();

	char sql[1024];
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;

	sprintf(sql, "select `invite_time` from guild_invite where `inviter_id` = %lu and `invitee_id` = %lu and `guild_id` = %u", inviter_id, invitee_id, guild_id);

	res = query(sql, 1, NULL);
	if (!res)
	{
		return 0;
	}

	row = fetch_row(res);
	if (row)
	{
		uint32_t time = strtoul(row[0], NULL, 10);
		free_query(res);
		return (time + sg_guild_invite_cd);
	}
	free_query(res);

	return 0;
}

bool check_invite_is_deal(uint64_t inviter_id, uint64_t invitee_id, uint32_t guild_id)
{
	clear_timeout_player_invite();

	char sql[1024];
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;

	sprintf(sql, "select `deal_type` from guild_invite where `inviter_id` = %lu and `invitee_id` = %lu and `guild_id` = %u", inviter_id, invitee_id, guild_id);

	res = query(sql, 1, NULL);
	if (!res)
	{
		return true;
	}

	row = fetch_row(res);
	if (row)
	{
		uint32_t deal_type = strtoul(row[0], NULL, 10);
		free_query(res);
		return (deal_type != 0);
	}
	free_query(res);

	return true;
}

void sync_guild_rename_to_gamesrv(GuildInfo *guild)
{
	PROTO_SYNC_GUILD_RENAME *pData = (PROTO_SYNC_GUILD_RENAME *)conn_node_base::get_send_data();
	memset(pData, 0, sizeof(PROTO_SYNC_GUILD_RENAME));
	pData->guild_id = guild->guild_id;
	memcpy(pData->name, guild->name, sizeof(guild->name));
	pData->member_num = guild->member_num;
	for (uint32_t i = 0; i < guild->member_num; ++i)
	{
		pData->member_ids[i] = guild->members[i]->player_id;
	}

	EXTERN_DATA ext_data;

	fast_send_msg_base(&conn_node_guildsrv::connecter, &ext_data, SERVER_PROTO_GUILD_RENAME, sizeof(PROTO_SYNC_GUILD_RENAME), 0);
}

void sync_guild_info_to_gamesrv(GuildPlayer *player)
{
	PROTO_SYNC_GUILD_INFO *req = (PROTO_SYNC_GUILD_INFO *)conn_node_base::get_send_buf(SERVER_PROTO_SYNC_GUILD_INFO, 0);
	req->head.len = ENDION_FUNC_4(sizeof(PROTO_SYNC_GUILD_INFO));
	memset(req->head.data, 0, sizeof(PROTO_SYNC_GUILD_INFO) - sizeof(PROTO_HEAD));
	if (player->guild)
	{
		req->guild_id = player->guild->guild_id;
		req->guild_office = player->office;
	}

	EXTERN_DATA ext_data;
	ext_data.player_id = player->player_id;
	conn_node_base::add_extern_data(&req->head, &ext_data);

	if (conn_node_guildsrv::connecter.send_one_msg(&req->head, 1) != (int)ENDION_FUNC_4(req->head.len))
	{
		LOG_ERR("[%s:%d] send to conn_srv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
}

void sync_guild_task_to_gamesrv(GuildPlayer *player)
{
	GUILD_SYNC_TASK *req = (GUILD_SYNC_TASK *)conn_node_guildsrv::get_send_data();
	uint32_t data_len = sizeof(GUILD_SYNC_TASK);
	memset(req, 0, data_len);
	req->task_count = player->cur_week_task;
	req->config_id = player->cur_week_task_config_id;

	EXTERN_DATA ext_data;
	ext_data.player_id = player->player_id;
	fast_send_msg_base(&conn_node_guildsrv::connecter, &ext_data, SERVER_PROTO_GUILD_SYNC_TASK, data_len, 0);
}

void refresh_guild_redis_info(GuildInfo *guild)
{
	RedisGuildInfo req;
	redis_guild_info__init(&req);

	std::vector<uint64_t> member_ids;
	req.guild_id = guild->guild_id;
	req.name = guild->name;
	req.level = get_building_level(guild, Building_Hall);
	req.zhenying = guild->zhenying;
	req.master_id = guild->master_id;
	req.head = guild->icon;
	for (uint32_t i = 0; i < guild->member_num; ++i)
	{
		member_ids.push_back(guild->members[i]->player_id);
	}
	req.member_ids = &member_ids[0];
	req.n_member_ids = member_ids.size();

	EXTERN_DATA ext_data;
	fast_send_msg(&conn_node_guildsrv::connecter, &ext_data, SERVER_PROTO_REFRESH_GUILD_REDIS_INFO, redis_guild_info__pack, req);
}

void sync_guild_create_to_gamesrv(GuildInfo *guild)
{
	ProtoGuildInfo *pData = (ProtoGuildInfo *)conn_node_base::get_send_data();
	memset(pData, 0, sizeof(ProtoGuildInfo));
	pData->guild_id = guild->guild_id;
	memcpy(pData->name, guild->name, sizeof(guild->name));
	pData->zhenying = guild->zhenying;
	pData->master_id = guild->master_id;

	EXTERN_DATA ext_data;

	fast_send_msg_base(&conn_node_guildsrv::connecter, &ext_data, SERVER_PROTO_GUILD_CREATE, sizeof(ProtoGuildInfo), 0);
}

static int insert_new_guild_to_db(GuildInfo *guild)
{
	static char save_sql[64 * 1024 + 300];
	static uint8_t save_data[64 * 1024 + 1];
	uint64_t effect = 0;
	char *p;

	size_t data_size = pack_guild_info(guild, save_data);
	p = save_sql;
	p += sprintf(save_sql, "insert into guild set `icon` = %u, `master_id` = %lu, `level` = %u, `popularity` = %u, `approve_state` = %u, `recruit_state` = %u, `zhenying` = %u, `recruit_notice` = \'", 
			guild->icon, guild->master_id, get_guild_level(guild), guild->popularity, guild->approve_state, guild->recruit_state, guild->zhenying);
	p += escape_string(p, (const char *)guild->recruit_notice, strlen(guild->recruit_notice));
	p += sprintf(p, "\', `announcement` = \'");
	p += escape_string(p, (const char *)guild->announcement, strlen(guild->announcement));
	p += sprintf(p, "\', `name` = \'");
	p += escape_string(p, (const char *)guild->name, strlen(guild->name));
	p += sprintf(p, "\', `comm_data` = \'");
	p += escape_string(p, (const char *)save_data, data_size);
	p += sprintf(p, "\'");

	query(const_cast<char*>("set names utf8"), 1, NULL);
	query(save_sql, 1, &effect);	
	if (effect != 1) 
	{
		LOG_ERR("[%s:%d] insert guild failed, error:%s", __FUNCTION__, __LINE__, mysql_error());
		return -1;
	}

	//获取帮会ID
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;	
	res = query((char *)("SELECT LAST_INSERT_ID()"), 1, NULL);
	if (!res)
	{
		LOG_ERR("[%s:%d] query failed, error:%s", __FUNCTION__, __LINE__, mysql_error());
		return -200;
	}

	row = fetch_row(res);
	if (!row)
	{
		LOG_ERR("[%s:%d] fetch row failed, error:%s", __FUNCTION__, __LINE__, mysql_error());
		free_query(res);
		return -300;
	}

	guild->guild_id = atoi(row[0]);
	free_query(res);

	return 0;
}

int create_guild(uint64_t player_id, uint32_t icon, std::string &name, GuildPlayer *&player)
{
	int ret = 0;
	bool player_new = false;
	GuildInfo *guild = NULL;
	do
	{
		AutoReleaseRedisPlayer arp_redis;
		PlayerRedisInfo *redis_player = get_redis_player(player_id, sg_player_key, sg_redis_client, arp_redis);
		if (!redis_player)
		{
			ret = ERROR_ID_SERVER;
			break;
		}
		
		player = get_guild_player(player_id);
		if (!player)
		{
			player = (GuildPlayer*)malloc(sizeof(GuildPlayer));
			memset(player, 0, sizeof(GuildPlayer));
			player_new = true;
			if (!player)
			{
				ret = ERROR_ID_SERVER;
				break;
			}
		}

		guild = (GuildInfo*)malloc(sizeof(GuildInfo));
		memset(guild, 0, sizeof(GuildInfo));
		if (!guild)
		{
			ret = ERROR_ID_SERVER;
			break;
		}

		uint32_t now = time_helper::get_cached_time() / 1000;
		memcpy(guild->name, name.c_str(), name.size());
		guild->name[name.size()] = '\0';
		guild->icon = icon;
		guild->zhenying = redis_player->zhenying;
		guild->master_id = player_id;
		guild->approve_state = 0;
		guild->recruit_state = 1;
		guild->maintain_time = now;
		guild->popularity = sg_guild_init_popularity;
		if (sg_guild_recruit_notice && strlen(sg_guild_recruit_notice) <= MAX_GUILD_ANNOUNCEMENT_LEN)
		{
			strcpy(guild->recruit_notice, sg_guild_recruit_notice);
		}
		else
		{
			guild->recruit_notice[0] = '\0';
		}
		if (sg_guild_announcement && strlen(sg_guild_announcement) <= MAX_GUILD_ANNOUNCEMENT_LEN)
		{
			strcpy(guild->announcement, sg_guild_announcement);
		}
		else
		{
			guild->announcement[0] = '\0';
		}
		guild->member_num = 1;
		guild->members[0] = player;
		init_guild_permission(guild);
		init_guild_building(guild);

		GuildLog *log = &guild->important_logs[0];
		log->type = GILT_CREATE;
		log->time = now;
		snprintf(log->args[0], MAX_GUILD_LOG_ARG_LEN, "%s", guild->name);

		player->player_id = player_id;
		player->guild = guild;
		player->office = GUILD_OFFICE_TYPE__OFFICE_MASTER;
		player->join_time = now;
		bool sync_task = false;
		if (player->cur_week_task_config_id == 0)
		{
			player->cur_week_task_config_id = get_guild_build_task_id(redis_player->lv);
			sync_task = true;
		}

		if (insert_new_guild_to_db(guild) != 0)
		{
			ret = ERROR_ID_MYSQL_QUERY;
			break;
		}
		if (save_guild_player(player) != 0)
		{
			ret = ERROR_ID_MYSQL_QUERY;
			break;
		}

		if (player_new)
		{
			guild_player_map[player->player_id] = player;
		}
		guild_map[guild->guild_id] = guild;
		delete_player_join_apply(player->player_id);
		sync_guild_create_to_gamesrv(guild);
		refresh_guild_redis_info(guild);
		sync_guild_info_to_gamesrv(player);
		update_redis_player_guild(player);
		if (sync_task)
		{
			sync_guild_task_to_gamesrv(player);
		}
	} while(0);

	if (ret != 0)
	{
		if (player_new)
		{
			free(player);
		}
		else
		{
			player->guild = NULL;
			player->office = 0;
			player->join_time = 0;
		}

		if (guild)
		{
			free(guild);
		}
	}

	return ret;
}

int join_guild(uint64_t player_id, GuildInfo *guild)
{
	int ret = 0;
	bool player_new = false;
	GuildPlayer *player = NULL;
	AutoReleaseBatchRedisPlayer t1;
	do
	{
		PlayerRedisInfo *redis_player = get_redis_player(player_id, sg_player_key, sg_redis_client, t1);
		if (!redis_player)
		{
			ret = ERROR_ID_SERVER;
			break;
		}

		player = get_guild_player(player_id);
		if (!player)
		{
			player = (GuildPlayer*)malloc(sizeof(GuildPlayer));
			memset(player, 0, sizeof(GuildPlayer));
			player_new = true;
			if (!player)
			{
				ret = ERROR_ID_SERVER;
				break;
			}

			player->player_id = player_id;
		}

		player->guild = guild;
		player->office = GUILD_OFFICE_TYPE__OFFICE_MASS;
		player->join_time = time_helper::get_cached_time() / 1000;
		bool sync_task = false;
		if (player->cur_week_task_config_id == 0)
		{
			player->cur_week_task_config_id = get_guild_build_task_id(redis_player->lv);
			sync_task = true;
		}

		if (save_guild_player(player) != 0)
		{
			ret = ERROR_ID_MYSQL_QUERY;
			break;
		}

		guild->members[guild->member_num++] = player;

		if (player_new)
		{
			guild_player_map[player->player_id] = player;
		}
		delete_player_join_apply(player->player_id);
		sync_guild_info_to_gamesrv(player);
		update_redis_player_guild(player);
		refresh_guild_redis_info(guild);
		if (sync_task)
		{
			sync_guild_task_to_gamesrv(player);
		}

		//通知加入玩家帮会信息，这个消息要最先发
		if (redis_player->status == 0)
		{
			EXTERN_DATA ext_data;
			ext_data.player_id = player_id;
			resp_guild_info(&conn_node_guildsrv::connecter, &ext_data, MSG_ID_GUILD_INFO_ANSWER, 0, player);
			
			std::vector<char *> args;
			args.push_back(guild->name);
			conn_node_guildsrv::send_system_notice(player_id, 190500249, &args);
		}

		broadcast_guild_attr_update(guild, GUILD_ATTR_TYPE__ATTR_MEMBER_NUM, guild->member_num, player->player_id);

		//添加帮会日志
		{
			GuildLog *log = get_usual_insert_log(guild);
			log->type = GULT_JOIN;
			log->time = player->join_time; 
			snprintf(log->args[0], MAX_GUILD_LOG_ARG_LEN, "%s", redis_player->name);
			broadcast_usual_log_add(guild, log);
			save_guild_info(guild);
		}

		//聊天发送
		ParameterTable *param_config = get_config_by_id(161000148, &parameter_config);
		if (param_config)
		{
			char content[1024];
			sprintf(content, param_config->parameter2, redis_player->name);

			Chat req;
			chat__init(&req);
			req.channel = CHANNEL__family;
			req.contain = content;

			broadcast_guild_chat(guild, &req);
		}
	} while(0);

	if (ret != 0)
	{
		if (player_new)
		{
			free(player);
		}
		else
		{
			player->guild = NULL;
			player->office = 0;
			player->join_time = 0;
		}
	}

	return ret;
}

int appoint_office(GuildPlayer *appointor, GuildPlayer *appointee, uint32_t office)
{
	if (appointor->guild != appointee->guild)
	{
		return 190500247;
	}

	//任命官职时，被任命者必须是帮众
//	if (office != GUILD_OFFICE_TYPE__OFFICE_MASS && appointee->office != GUILD_OFFICE_TYPE__OFFICE_MASS)
//	{
//		return ERROR_ID_GUILD_PLAYER_HAS_OFFICE;
//	}
	//只有帮主能转让帮主
	if (office == GUILD_OFFICE_TYPE__OFFICE_MASTER && appointor->office != GUILD_OFFICE_TYPE__OFFICE_MASTER)
	{
		return ERROR_ID_GUILD_PLAYER_NO_PERMISSION;
	}
	if (appointee->office == office)
	{
		return ERROR_ID_GUILD_PLAYER_ALREADY_IS;
	}

	GuildInfo *guild = appointor->guild;
	if (office == GUILD_OFFICE_TYPE__OFFICE_VICE_MASTER || office == GUILD_OFFICE_TYPE__OFFICE_ELDER)
	{
		uint32_t hall_level = get_building_level(guild, Building_Hall);
		GangsTable *config = get_guild_building_config(Building_Hall, hall_level);
		if (!config)
		{
			return ERROR_ID_NO_CONFIG;
		}

		uint32_t limit_num = 0;
		if (office == GUILD_OFFICE_TYPE__OFFICE_VICE_MASTER)
		{
			limit_num = config->parameter2;
		}
		else if (office == GUILD_OFFICE_TYPE__OFFICE_ELDER)
		{
			limit_num = config->parameter3;
		}

		uint32_t has_num = 0;
		for (uint32_t i = 0; i < guild->member_num; ++i)
		{
			if (guild->members[i]->office == office)
			{
				has_num++;
			}
		}

		if (has_num >= limit_num)
		{
			return ERROR_ID_GUILD_OFFICE_MAX;
		}
	}

	if (!player_has_permission(appointor, GOPT_APPOINT) || (office != GUILD_OFFICE_TYPE__OFFICE_MASTER && (office <= appointor->office || appointee->office <= appointor->office)))
	{
		return 190500453;
	}

	uint32_t mail_id = 0;
	std::vector<char *> args;
	if (/*appointee->office == GUILD_OFFICE_TYPE__OFFICE_MASS &&*/ office == GUILD_OFFICE_TYPE__OFFICE_ELDER)
	{ //任命长老
		mail_id = 270300004;
		args.push_back(guild->name);
	}
	else if (/*appointee->office == GUILD_OFFICE_TYPE__OFFICE_MASS &&*/ office == GUILD_OFFICE_TYPE__OFFICE_VICE_MASTER)
	{ //任命副帮主
		mail_id = 270300005;
		args.push_back(guild->name);
	}
	else if (appointee->office == GUILD_OFFICE_TYPE__OFFICE_VICE_MASTER && office == GUILD_OFFICE_TYPE__OFFICE_MASS)
	{ //撤职副帮主
		mail_id = 270300006;
		args.push_back(guild->name);
	}
	else if (appointee->office == GUILD_OFFICE_TYPE__OFFICE_ELDER && office == GUILD_OFFICE_TYPE__OFFICE_MASS)
	{ //撤职长老
		mail_id = 270300007;
		args.push_back(guild->name);
	}
	else if (office == GUILD_OFFICE_TYPE__OFFICE_MASTER)
	{ //转让帮主
		mail_id = 270300009;
		args.push_back(guild->name);
	}

	bool office_up = (office < appointee->office ? true : false);

	appointee->office = office;
	save_guild_player(appointee);
	AutoReleaseBatchRedisPlayer t1;
	PlayerRedisInfo *redis_appointee = get_redis_player(appointee->player_id, sg_player_key, sg_redis_client, t1);
	if (redis_appointee)
	{
		if (redis_appointee->status == 0)
		{
			notify_guild_attr_update(appointee->player_id, GUILD_ATTR_TYPE__ATTR_OFFICE, appointee->office);
		}
	}
	PlayerRedisInfo *redis_appointor = get_redis_player(appointor->player_id, sg_player_key, sg_redis_client, t1);
	GangsJurisdictionTable *config = get_config_by_id(office, &guild_office_config);
	if (redis_appointor && redis_appointee && config)
	{
		uint32_t now = time_helper::get_cached_time() / 1000;
		GuildLog *log = get_important_insert_log(guild);
		log->type = (office_up ? GILT_OFFICE_UP : GILT_OFFICE_DOWN);
		log->time = now;
		snprintf(log->args[0], MAX_GUILD_LOG_ARG_LEN, "%s", redis_appointor->name);
		snprintf(log->args[1], MAX_GUILD_LOG_ARG_LEN, "%s", redis_appointee->name);
		snprintf(log->args[2], MAX_GUILD_LOG_ARG_LEN, "%s", config->Name);
		broadcast_important_log_add(guild, log);
		save_guild_info(guild);

		if (redis_appointee->status == 0)
		{
			std::vector<char *> args;
			if (office == GUILD_OFFICE_TYPE__OFFICE_MASS)
			{
				conn_node_guildsrv::send_system_notice(appointee->player_id, 190500447, NULL);
			}
			else
			{
				args.push_back(config->Name);
				conn_node_guildsrv::send_system_notice(appointee->player_id, 190500446, &args);
			}
		}
	}

	if (office == GUILD_OFFICE_TYPE__OFFICE_MASTER)
	{
		appointor->office = GUILD_OFFICE_TYPE__OFFICE_MASS;
		save_guild_player(appointor);
		notify_guild_attr_update(appointor->player_id, GUILD_ATTR_TYPE__ATTR_OFFICE, appointor->office);

		//转让帮主
		guild->master_id = appointee->player_id;
		replace_guild_master(guild->guild_id, guild->master_id);
		broadcast_guild_attr_update(guild, GUILD_ATTR_TYPE__ATTR_MASTER_ID, guild->master_id);
		refresh_guild_redis_info(guild);
		if (redis_appointee)
		{
			broadcast_guild_str_attr_update(guild, GUILD_STR_ATTR_TYPE__ATTR_MASTER_NAME, redis_appointee->name);
		}
	}

	if (mail_id > 0)
	{
		send_mail(&conn_node_guildsrv::connecter, appointee->player_id, mail_id, NULL, NULL, NULL, &args, NULL, 0);
	}

	return 0;
}

static int find_member_index(GuildInfo *guild, uint64_t player_id)
{
	int mem_idx = -1;
	for (uint32_t i = 0; i < guild->member_num; ++i)
	{
		if (guild->members[i]->player_id == player_id)
		{
			mem_idx = i;
			break;
		}
	}

	return mem_idx;
}

static int deal_exit_player_part(GuildPlayer *player, PlayerExitGuildReason reason)
{
	char *guild_name = player->guild->name;
	player->guild = NULL;
	player->office = 0;
	player->cur_history_donation = 0;
	player->cur_week_donation = 0;
	player->cur_week_treasure = 0;
	player->cur_task_id = 0;
	if (reason == PEGR_QUIT)
	{
		player->exit_time = time_helper::get_cached_time() / 1000;
	}
	save_guild_player(player);
	AutoReleaseRedisPlayer t1;	
	PlayerRedisInfo *redis_player = get_redis_player(player->player_id, sg_player_key, sg_redis_client, t1);
	if (reason == PEGR_KICK && redis_player && redis_player->status == 0)
	{
		EXTERN_DATA ext_data;
		ext_data.player_id = player->player_id;
		fast_send_msg_base(&conn_node_guildsrv::connecter, &ext_data, MSG_ID_GUILD_KICK_NOTIFY, 0, 0);
	}

	//被踢、解散需要发邮件
	if (reason != PEGR_QUIT)
	{
		std::vector<char *> args;
		uint32_t mail_id = 0;
		if (reason == PEGR_DISBAND)
		{
			mail_id = 270300003;
			args.push_back(guild_name);
		}
		else if (reason == PEGR_KICK)
		{
			mail_id = 270300008;
			args.push_back(guild_name);
		}

		send_mail(&conn_node_guildsrv::connecter, player->player_id, mail_id, NULL, NULL, NULL, &args, NULL, 0);
	}

	//解散由 sync_guild_disband_to_gamesrv 来同步，其他方式由玩家同步
	if (reason != PEGR_DISBAND)
	{
		sync_guild_info_to_gamesrv(player);
	}
	update_redis_player_guild(player);

	return 0;
}

static int deal_exit_guild_part(GuildInfo *&guild, int exit_idx)
{
	if (exit_idx < 0)
	{
		guild->master_id = 0;
		guild->member_num = 0;
		refresh_guild_redis_info(guild);
		delete_guild(guild->guild_id);
		guild_map.erase(guild->guild_id);
		free(guild);
		guild = NULL;
	}
	else
	{
		int last_idx = MAX_GUILD_MEMBER_NUM - 1;
		if (exit_idx < last_idx)
		{
			memmove(&guild->members[exit_idx], &guild->members[exit_idx + 1], (last_idx - exit_idx) * sizeof (GuildPlayer *));
		}
		memset(&guild->members[last_idx], 0, sizeof(GuildPlayer *));
		guild->member_num--;
		save_guild_info(guild);
		broadcast_guild_attr_update(guild, GUILD_ATTR_TYPE__ATTR_MEMBER_NUM, guild->member_num);
		refresh_guild_redis_info(guild);
	}

	return 0;
}

int kick_member(GuildPlayer *player, uint64_t kick_id)
{
	if (is_guild_battle_opening())
	{
		LOG_ERR("[%s:%d] player[%lu] can't kick, batting, kick_id:%lu", __FUNCTION__, __LINE__, player->player_id, kick_id);
		return 190411006;
	}
	if (kick_id == player->player_id)
	{
		LOG_ERR("[%s:%d] player[%lu] can't kick self, kick_id:%lu", __FUNCTION__, __LINE__, player->player_id, kick_id);
		return ERROR_ID_GUILD_CAN_NOT_DO_IT_TO_SELF;
	}

	GuildInfo *guild = player->guild;
	if (!guild)
	{
		return ERROR_ID_GUILD_PLAYER_NOT_JOIN;
	}

	int kick_idx = find_member_index(guild, kick_id);
	if (kick_idx < 0)
	{
		LOG_ERR("[%s:%d] player[%lu] kick target not in guild, kick_id:%lu", __FUNCTION__, __LINE__, player->player_id, kick_id);
		return ERROR_ID_GUILD_HAS_NOT_PLAYER;
	}

	GuildPlayer *kick_player = guild->members[kick_idx];
	if (kick_player->office == GUILD_OFFICE_TYPE__OFFICE_MASTER)
	{
		return 190500453;
	}

	if (!player_has_permission(player, GOPT_KICK) || kick_player->office <= player->office)
	{
		LOG_ERR("[%s:%d] player[%lu] no permission, office:%u", __FUNCTION__, __LINE__, player->player_id, player->office);
		return 190500453;
	}

	AutoReleaseBatchRedisPlayer t1;
	PlayerRedisInfo *redis_player = get_redis_player(player->player_id, sg_player_key, sg_redis_client, t1);
	PlayerRedisInfo *redis_kick = get_redis_player(kick_id, sg_player_key, sg_redis_client, t1);
	if (redis_player && redis_kick)
	{
		uint32_t now = time_helper::get_cached_time() / 1000;
		GuildLog *log = get_usual_insert_log(guild);
		log->type = GULT_KICK;
		log->time = now; 
		snprintf(log->args[0], MAX_GUILD_LOG_ARG_LEN, "%s", redis_kick->name);
		snprintf(log->args[1], MAX_GUILD_LOG_ARG_LEN, "%s", redis_player->name);
		broadcast_usual_log_add(guild, log);
	}

	deal_exit_player_part(kick_player, PEGR_KICK);

	deal_exit_guild_part(guild, kick_idx);

	return 0;
}

int exit_guild(GuildPlayer *player)
{
	if (!player || !player->guild)
	{
		return ERROR_ID_GUILD_PLAYER_NOT_JOIN;
	}

	GuildInfo *guild = player->guild;
	int mem_idx = find_member_index(guild, player->player_id);
	if (mem_idx < 0)
	{
		LOG_ERR("[%s:%d] player[%lu] not in guild, guild_id:%u", __FUNCTION__, __LINE__, player->player_id, guild->guild_id);
		return ERROR_ID_GUILD_HAS_NOT_PLAYER;
	}

	AutoReleaseBatchRedisPlayer t1;
	PlayerRedisInfo *redis_player = get_redis_player(player->player_id, sg_player_key, sg_redis_client, t1);
	if (redis_player)
	{
		uint32_t now = time_helper::get_cached_time() / 1000;
		GuildLog *log = get_usual_insert_log(guild);
		log->type = GULT_QUIT;
		log->time = now; 
		snprintf(log->args[0], MAX_GUILD_LOG_ARG_LEN, "%s", redis_player->name);
		broadcast_usual_log_add(guild, log);
	}

	deal_exit_player_part(player, PEGR_QUIT);

	deal_exit_guild_part(guild, mem_idx);

	return 0;
}

static void sync_guild_disband_to_gamesrv(GuildInfo *guild)
{
	PROTO_GUILD_DISBAND *req = (PROTO_GUILD_DISBAND*)conn_node_base::get_send_buf(SERVER_PROTO_GUILD_DISBAND, 0);
	req->guild_id = guild->guild_id;
	req->player_num = guild->member_num;
	uint64_t *pData = (uint64_t*)req->data;
	for (uint32_t i = 0; i < guild->member_num; ++i)
	{
		*pData++ = guild->members[i]->player_id;
	}
	req->head.len = ENDION_FUNC_4(sizeof(PROTO_GUILD_DISBAND) + req->player_num * sizeof(uint64_t));

	EXTERN_DATA ext_data;
	ext_data.player_id = guild->master_id;
	conn_node_base::add_extern_data(&req->head, &ext_data);
	if (conn_node_guildsrv::connecter.send_one_msg(&req->head, 1) != (int)ENDION_FUNC_4(req->head.len))
	{
		LOG_ERR("[%s:%d] send to gamesrv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
}

int disband_guild(GuildInfo *&guild)
{
	LOG_INFO("[%s:%d] guild %u[%s] disband.", __FUNCTION__, __LINE__, guild->guild_id, guild->name);
	sync_guild_disband_to_gamesrv(guild);
	for (uint32_t i = 0; i < guild->member_num; ++i)
	{
		deal_exit_player_part(guild->members[i], PEGR_DISBAND);
	}

	deal_exit_guild_part(guild, -1);

	return 0;
}

void notify_guild_attr_update(uint64_t player_id, uint32_t id, uint32_t val)
{
	GuildUpdateAttrNotify nty;
	guild_update_attr_notify__init(&nty);

	AttrData attr_data[1];
	AttrData* attr_data_point[1];
	attr_data_point[0] = &attr_data[0];
	attr_data__init(&attr_data[0]);
	attr_data[0].id = id;
	attr_data[0].val = val;

	nty.n_attrs = 1;
	nty.attrs = attr_data_point;

	EXTERN_DATA ext_data;
	ext_data.player_id = player_id;

	fast_send_msg(&conn_node_guildsrv::connecter, &ext_data, MSG_ID_GUILD_UPDATE_ATTR_NOTIFY, guild_update_attr_notify__pack, nty);
}

#define MAX_GUILD_ATTR_NUM 5
void notify_guild_attrs_update(uint64_t player_id, AttrMap &attrs)
{
	GuildUpdateAttrNotify nty;
	guild_update_attr_notify__init(&nty);

	AttrData attr_data[MAX_GUILD_ATTR_NUM];
	AttrData* attr_data_point[MAX_GUILD_ATTR_NUM];

	nty.n_attrs = 0;
	nty.attrs = attr_data_point;
	for (AttrMap::iterator iter = attrs.begin(); iter != attrs.end() && nty.n_attrs < MAX_GUILD_ATTR_NUM; ++iter)
	{
		attr_data_point[nty.n_attrs] = &attr_data[nty.n_attrs];
		attr_data__init(&attr_data[nty.n_attrs]);
		attr_data[nty.n_attrs].id = iter->first;
		attr_data[nty.n_attrs].val = iter->second;
		nty.n_attrs++;
	}

	EXTERN_DATA ext_data;
	ext_data.player_id = player_id;

	fast_send_msg(&conn_node_guildsrv::connecter, &ext_data, MSG_ID_GUILD_UPDATE_ATTR_NOTIFY, guild_update_attr_notify__pack, nty);
}

void broadcast_guild_attr_update(GuildInfo *guild, uint32_t id, uint32_t val, uint64_t except_id)
{
	std::vector<uint64_t> player_ids;
	get_guild_broadcast_objects(guild, player_ids, except_id);
	if (player_ids.size() == 0)
	{
		return ;
	}

	GuildUpdateAttrNotify nty;
	guild_update_attr_notify__init(&nty);

	AttrData attr_data[1];
	AttrData* attr_data_point[1];
	attr_data_point[0] = &attr_data[0];
	attr_data__init(&attr_data[0]);
	attr_data[0].id = id;
	attr_data[0].val = val;

	nty.n_attrs = 1;
	nty.attrs = attr_data_point;

	conn_node_guildsrv::broadcast_message(MSG_ID_GUILD_UPDATE_ATTR_NOTIFY, &nty, (pack_func)guild_update_attr_notify__pack, player_ids);
}

void broadcast_guild_str_attr_update(uint32_t id, char *val, std::vector<uint64_t> &player_ids)
{
	GuildUpdateStrAttrNotify nty;
	guild_update_str_attr_notify__init(&nty);

	GuildStrAttrData  attr_data[1];
	GuildStrAttrData* attr_point[1];
	attr_point[0] = &attr_data[0];
	guild_str_attr_data__init(&attr_data[0]);
	attr_data[0].id = id;
	attr_data[0].val = val;

	nty.n_attrs = 1;
	nty.attrs = attr_point;

	conn_node_guildsrv::broadcast_message(MSG_ID_GUILD_UPDATE_STR_ATTR_NOTIFY, &nty, (pack_func)guild_update_str_attr_notify__pack, player_ids);
}

void broadcast_guild_str_attr_update(GuildInfo *guild, uint32_t id, char *val, uint64_t except_id)
{
	std::vector<uint64_t> player_ids;
	get_guild_broadcast_objects(guild, player_ids, except_id);
	if (player_ids.size() == 0)
	{
		return ;
	}

	broadcast_guild_str_attr_update(id, val, player_ids);
}

void broadcast_guild_object_attr_update(GuildInfo *guild, uint32_t type, uint32_t id, uint32_t val, uint64_t except_id)
{
	std::vector<uint64_t> player_ids;
	get_guild_broadcast_objects(guild, player_ids, except_id);
	if (player_ids.size() == 0)
	{
		return ;
	}

	GuildUpdateObjectAttrNotify nty;
	guild_update_object_attr_notify__init(&nty);

	GuildObjectAttrData  attr_data[1];
	GuildObjectAttrData* attr_point[1];
	attr_point[0] = &attr_data[0];
	guild_object_attr_data__init(&attr_data[0]);
	attr_data[0].type = type;
	attr_data[0].id = id;
	attr_data[0].val = val;

	nty.n_attrs = 1;
	nty.attrs = attr_point;

	conn_node_guildsrv::broadcast_message(MSG_ID_GUILD_UPDATE_OBJECT_ATTR_NOTIFY, &nty, (pack_func)guild_update_object_attr_notify__pack, player_ids);
}

void get_guild_broadcast_objects(GuildInfo *guild, std::vector<uint64_t> &player_ids, uint64_t except_id)
{
	player_ids.clear();

	for (uint32_t i = 0; i < guild->member_num; ++i)
	{
		uint64_t player_id = guild->members[i]->player_id;
		if (player_id == except_id)
		{
			continue;
		}

		AutoReleaseRedisPlayer t1;		
		PlayerRedisInfo *redis_player = get_redis_player(player_id, sg_player_key, sg_redis_client, t1);
		if (redis_player)
		{
			if (redis_player->status == 0)
			{
				player_ids.push_back(player_id);
			}
		}
	}
}

int add_guild_popularity(GuildInfo *guild, uint32_t num)
{
	if (num ==0)
	{
		return 0;
	}

	guild->popularity += num;
	broadcast_guild_attr_update(guild, GUILD_ATTR_TYPE__ATTR_POPULARITY, guild->popularity);
	save_guild_popularity(guild);

	return 0;
}

int add_guild_treasure(GuildInfo *guild, uint32_t num)
{
	if (num ==0)
	{
		return 0;
	}

	uint32_t upper_limit = get_guild_resource_upper_limit(guild, GUILD_ATTR_TYPE__ATTR_TREASURE);
	uint32_t val_new = std::min(guild->treasure + num, upper_limit);

	if (val_new != guild->treasure)
	{
		guild->treasure = val_new;
		broadcast_guild_attr_update(guild, GUILD_ATTR_TYPE__ATTR_TREASURE, guild->treasure);
		save_guild_info(guild);
	}

	return 0;
}

int add_guild_build_board(GuildInfo *guild, uint32_t num)
{
	if (num ==0)
	{
		return 0;
	}

	uint32_t upper_limit = get_guild_resource_upper_limit(guild, GUILD_ATTR_TYPE__ATTR_BUILDBOARD);
	uint32_t val_new = std::min(guild->build_board + num, upper_limit);

	if (val_new != guild->build_board)
	{
		guild->build_board = val_new;
		broadcast_guild_attr_update(guild, GUILD_ATTR_TYPE__ATTR_BUILDBOARD, guild->build_board);
		save_guild_info(guild);
	}

	return 0;
}

int add_player_donation(GuildPlayer *player, uint32_t num)
{
	if (num ==0)
	{
		return 0;
	}

	player->donation += num;
	player->all_history_donation += num;
	player->cur_history_donation += num;
	player->cur_week_donation += num;

	AttrMap attrs;
	attrs[GUILD_ATTR_TYPE__ATTR_DONATION] = player->donation;
	attrs[GUILD_ATTR_TYPE__ATTR_HISTORY_DONATION] = player->all_history_donation;
	notify_guild_attrs_update(player->player_id, attrs);
	save_guild_player(player);
	sync_player_donation_to_game_srv(player, 1, num);

//	//系统提示
//	std::vector<char *> args;
//	std::string sz_num;
//	std::stringstream  ss;
//	ss << num;
//	ss >> sz_num;
//	args.push_back(const_cast<char*>(sz_num.c_str()));
//	conn_node_guildsrv::send_system_notice(player->player_id, 190500425, &args);

	return 0;
}

int add_player_contribute_treasure(GuildPlayer *player, uint32_t num)
{
	if (num ==0)
	{
		return 0;
	}

	player->cur_week_treasure += num;

	save_guild_player(player);

//	//系统提示
//	std::vector<char *> args;
//	std::string sz_num;
//	std::stringstream  ss;
//	ss << num;
//	ss >> sz_num;
//	args.push_back(const_cast<char*>(sz_num.c_str()));
//	conn_node_guildsrv::send_system_notice(player->player_id, 190500499, &args);

	return 0;
}

int sub_guild_popularity(GuildInfo *guild, uint32_t num)
{
	if (num == 0)
	{
		return 0;
	}

	if (guild->popularity < num)
	{
		LOG_ERR("[%s:%d] popularity not enough, has:%u, sub:%u", __FUNCTION__, __LINE__, guild->popularity, num);
		return -1;
	}

	guild->popularity -= num;
	broadcast_guild_attr_update(guild, GUILD_ATTR_TYPE__ATTR_POPULARITY, guild->popularity);
	save_guild_popularity(guild);
	
	return 0;
}

int sub_guild_treasure(GuildInfo *guild, uint32_t num, bool save)
{
	if (num == 0)
	{
		return 0;
	}

	if (guild->treasure < num)
	{
		LOG_ERR("[%s:%d] treasure not enough, has:%u, sub:%u", __FUNCTION__, __LINE__, guild->treasure, num);
		return -1;
	}

	guild->treasure -= num;
	broadcast_guild_attr_update(guild, GUILD_ATTR_TYPE__ATTR_TREASURE, guild->treasure);
	if (save)
	{
		save_guild_info(guild);
	}
	
	return 0;
}

int sub_guild_build_board(GuildInfo *guild, uint32_t num, bool save)
{
	if (num == 0)
	{
		return 0;
	}

	if (guild->build_board < num)
	{
		LOG_ERR("[%s:%d] board not enough, has:%u, sub:%u", __FUNCTION__, __LINE__, guild->build_board, num);
		return -1;
	}

	guild->build_board -= num;
	broadcast_guild_attr_update(guild, GUILD_ATTR_TYPE__ATTR_BUILDBOARD, guild->build_board);
	if (save)
	{
		save_guild_info(guild);
	}
	
	return 0;
}

int sub_player_donation(GuildPlayer *player, uint32_t num, bool save)
{
	if (num == 0)
	{
		return 0;
	}

	if (player->donation < num)
	{
		LOG_ERR("[%s:%d] board not enough, player:%lu, has:%u, sub:%u", __FUNCTION__, __LINE__, player->player_id, player->donation, num);
		return -1;
	}

	player->donation -= num;
	notify_guild_attr_update(player->player_id, GUILD_ATTR_TYPE__ATTR_DONATION, player->donation);
	if (save)
	{
		save_guild_player(player);
	}
	sync_player_donation_to_game_srv(player, 2, num);
	
	return 0;
}

int add_guild_battle_score(GuildInfo *guild, uint32_t num)
{
	if (num ==0)
	{
		return 0;
	}

	uint32_t old_score = guild->battle_score;
	guild->battle_score += num;
	save_guild_info(guild);

	int ret = sg_redis_client.zset(sg_rank_guild_battle_key, guild->guild_id, guild->battle_score);
	if (ret != 0)
	{
		LOG_ERR("[%s:%d] update %s %u failed, score:%u, old_score:%u", __FUNCTION__, __LINE__, sg_rank_guild_battle_key, guild->guild_id, guild->battle_score, old_score);
	}
	
	return 0;
}

int add_player_battle_score(GuildPlayer *player, uint32_t num)
{
	player->battle_score += num;
	player->act_battle_score += num;
	save_guild_player(player);
	return 0;
}

void broadcast_guild_battle_score(GuildInfo *guild, std::vector<uint64_t> &player_ids)
{
	if (player_ids.size() == 0)
	{
		return;
	}

	GuildBattleWaitInfoNotify nty;
	guild_battle_wait_info_notify__init(&nty);

	nty.guildscore = guild->battle_score;
	nty.has_guildscore = true;

	conn_node_guildsrv::broadcast_message(MSG_ID_GUILD_BATTLE_WAIT_INFO_NOTIFY, &nty, (pack_func)guild_battle_wait_info_notify__pack, player_ids);
}

void sync_player_donation_to_game_srv(GuildPlayer *player, uint32_t is_change, uint32_t change_val)
{
	PROTO_SYNC_GUILD_DONATION *pData = (PROTO_SYNC_GUILD_DONATION*)conn_node_base::get_send_data();
	pData->cur_donation = player->donation;
	pData->is_change = is_change;
	pData->change_val = change_val;

	EXTERN_DATA ext_data;
	ext_data.player_id = player->player_id;

	fast_send_msg_base(&conn_node_guildsrv::connecter, &ext_data, SERVER_PROTO_GUILD_SYNC_DONATION, sizeof(PROTO_SYNC_GUILD_DONATION), 0);
}

void init_guild_permission(GuildInfo *guild)
{
	for (int office = GUILD_OFFICE_TYPE__OFFICE_MASTER; office <= GUILD_OFFICE_TYPE__OFFICE_MASS; ++office)
	{
		if (office > MAX_GUILD_OFFICE)
		{
			continue;
		}

		GuildPermission *permission = &guild->permissions[office - 1];
		GangsJurisdictionTable *config = get_config_by_id(office, &guild_office_config);
		if (!config)
		{
			uint32_t bit = 0;
			if (office == GUILD_OFFICE_TYPE__OFFICE_MASTER)
			{
				bit = 1;
			}
			permission->office = office;
			for (int i = GOPT_APPOINT; i < GOPT_END; ++i)
			{
				permission->permission[i] = bit;
			}
			continue;
		}

		permission->office = office;
		permission->permission[GOPT_APPOINT] = (config->Appoint == 1 ? 1 : 0);
		permission->permission[GOPT_ANNOUNCEMENT_SETTING] = (config->NoticeSetup == 1 ? 1 : 0);
		permission->permission[GOPT_BUILDING_UP] = (config->BuildingUp == 1 ? 1 : 0);
		permission->permission[GOPT_OPEN_ACTIVITY] = (config->OpenActivity == 1 ? 1 : 0);
		permission->permission[GOPT_RECRUIT_SETTING] = (config->RecruitSetup == 1 ? 1 : 0);
		permission->permission[GOPT_DEVELOP_SKILL] = (config->skill == 1 ? 1 : 0);
		permission->permission[GOPT_RENAME] = (config->GangsName == 1 ? 1 : 0);
		permission->permission[GOPT_DEAL_JOIN] = (config->Recruit == 1 ? 1 : 0);
		permission->permission[GOPT_KICK] = (config->Expel == 1 ? 1 : 0);
		permission->permission[GOPT_INVITATION] = (config->Invitation == 1 ? 1 : 0);
	}
}

void broadcast_permission_update(GuildInfo *guild, uint32_t office, uint32_t type, uint32_t state, uint64_t except_id)
{
	std::vector<uint64_t> player_ids;
	get_guild_broadcast_objects(guild, player_ids, except_id);
	if (player_ids.size() == 0)
	{
		return ;
	}

	GuildUpdatePermissionNotify nty;
	guild_update_permission_notify__init(&nty);

	nty.office = office;
	nty.type = type;
	nty.state = state;

	conn_node_guildsrv::broadcast_message(MSG_ID_GUILD_UPDATE_PERMISSION_NOTIFY, &nty, (pack_func)guild_update_permission_notify__pack, player_ids);
}

GuildLog *get_usual_insert_log(GuildInfo *guild)
{
	int last_idx = MAX_GUILD_LOG_NUM - 1;
	if (guild->usual_logs[last_idx].type == 0)
	{ //未满
		for (int i = 0; i < MAX_GUILD_LOG_NUM; ++i)
		{
			if (guild->usual_logs[i].type == 0)
			{
				return &guild->usual_logs[i];
			}
		}
	}

	//已满
	memmove(&guild->usual_logs[0], &guild->usual_logs[1], (MAX_GUILD_LOG_NUM - 1) * sizeof(GuildLog));
	memset(&guild->usual_logs[last_idx], 0, sizeof(GuildLog));
	return &guild->usual_logs[last_idx];
}

void broadcast_usual_log_add(GuildInfo *guild, GuildLog *log)
{
	std::vector<uint64_t> player_ids;
	get_guild_broadcast_objects(guild, player_ids, 0);
	if (player_ids.size() == 0)
	{
		return ;
	}

	GuildLogData nty;
	guild_log_data__init(&nty);

	char*       log_args[MAX_GUILD_LOG_NUM];
	nty.type = log->type;
	nty.time = log->time;
	nty.args = log_args;
	nty.n_args = 0;
	for (int i = 0; i < MAX_GUILD_LOG_ARG_NUM; ++i)
	{
		if (log->args[i][0] == '\0')
		{
			break;
		}
		nty.args[nty.n_args] = log->args[i];
		nty.n_args++;
	}

	conn_node_guildsrv::broadcast_message(MSG_ID_GUILD_ADD_USUAL_LOG_NOTIFY, &nty, (pack_func)guild_log_data__pack, player_ids);
}

GuildLog *get_important_insert_log(GuildInfo *guild)
{
	int last_idx = MAX_GUILD_LOG_NUM - 1;
	if (guild->important_logs[last_idx].type == 0)
	{ //未满
		for (int i = 0; i < MAX_GUILD_LOG_NUM; ++i)
		{
			if (guild->important_logs[i].type == 0)
			{
				return &guild->important_logs[i];
			}
		}
	}

	//已满
	memmove(&guild->important_logs[0], &guild->important_logs[1], (MAX_GUILD_LOG_NUM - 1) * sizeof(GuildLog));
	memset(&guild->important_logs[last_idx], 0, sizeof(GuildLog));
	return &guild->important_logs[last_idx];
}

void broadcast_important_log_add(GuildInfo *guild, GuildLog *log)
{
	std::vector<uint64_t> player_ids;
	get_guild_broadcast_objects(guild, player_ids, 0);
	if (player_ids.size() == 0)
	{
		return ;
	}

	GuildLogData nty;
	guild_log_data__init(&nty);

	char*       log_args[MAX_GUILD_LOG_NUM];
	nty.type = log->type;
	nty.time = log->time;
	nty.args = log_args;
	nty.n_args = 0;
	for (int i = 0; i < MAX_GUILD_LOG_ARG_NUM; ++i)
	{
		if (log->args[i][0] == '\0')
		{
			break;
		}
		nty.args[nty.n_args] = log->args[i];
		nty.n_args++;
	}

	conn_node_guildsrv::broadcast_message(MSG_ID_GUILD_ADD_IMPORTANT_LOG_NOTIFY, &nty, (pack_func)guild_log_data__pack, player_ids);
}




int init_guild_building(GuildInfo *guild)
{
	for (int i = 0; i < MAX_GUILD_BUILDING_NUM; ++i)
	{
		guild->buildings[i].level = 1;
	}

	return 0;
}

GuildBuilding *get_building_info(GuildInfo *guild, uint32_t type)
{
	return (type >=1 && type <= MAX_GUILD_BUILDING_NUM ? &guild->buildings[type - 1] : NULL);
}

int get_building_level(GuildInfo * guild, uint32_t type)
{
	GuildBuilding *building = get_building_info(guild, type);
	return (building != NULL ? building->level : 0);
}

uint32_t get_guild_resource_upper_limit(GuildInfo* guild, uint32_t type)
{
	uint32_t vault_level = get_building_level(guild, Building_Vault);
	GangsTable *config = get_guild_building_config(Building_Vault, vault_level);
	if (!config)
	{
		return 0;
	}

	switch (type)
	{
		case GUILD_ATTR_TYPE__ATTR_TREASURE:
			return (config->parameter1);
		case GUILD_ATTR_TYPE__ATTR_BUILDBOARD:
			return (config->parameter2);
	}

	return 0;
}

int upgrade_building_level(GuildInfo *guild)
{
	if (guild->building_upgrade_id == 0 || guild->building_upgrade_end == 0)
	{
		return -1;
	}

	uint32_t building_id = guild->building_upgrade_id;
	GuildBuilding *building = get_building_info(guild, guild->building_upgrade_id);
	if (!building)
	{
		return -1;
	}

	building->level++;
	guild->building_upgrade_id = 0;
	guild->building_upgrade_end = 0;
	broadcast_building_upgrade_update(guild);
	broadcast_guild_object_attr_update(guild, GUILD_OBJECT_ATTR_TYPE__ATTR_BUILDING, building_id, building->level);

	GangsTable *config = get_guild_building_config(building_id, building->level);
	if (config)
	{
		uint32_t now = time_helper::get_cached_time() / 1000;
		GuildLog *log = get_important_insert_log(guild);
		log->type = GILT_BUILDING_UP;
		log->time = now;
		snprintf(log->args[0], MAX_GUILD_LOG_ARG_LEN, "%s", config->BuildingName);
		broadcast_important_log_add(guild, log);
	}

	refresh_guild_redis_info(guild);
	save_guild_info(guild);

	return 0;
}

void broadcast_building_upgrade_update(GuildInfo *guild)
{
	std::vector<uint64_t> player_ids;
	get_guild_broadcast_objects(guild, player_ids);
	if (player_ids.size() == 0)
	{
		return ;
	}

	GuildBuildingUpgradeUpdateNotify nty;
	guild_building_upgrade_update_notify__init(&nty);

	nty.upgradeid = guild->building_upgrade_id;
	nty.upgradeend = guild->building_upgrade_end;

	conn_node_guildsrv::broadcast_message(MSG_ID_GUILD_BUILDING_UPGRADE_UPDATE_NOTIFY, &nty, (pack_func)guild_building_upgrade_update_notify__pack, player_ids);
}

int sub_building_upgrade_time(GuildInfo *guild, uint32_t time)
{
	if (time == 0 || guild->building_upgrade_id == 0 || guild->building_upgrade_end == 0)
	{
		return -1;
	}

	uint32_t now = time_helper::get_cached_time() / 1000;
	uint32_t left_time = (guild->building_upgrade_end > now ? guild->building_upgrade_end - now : 0);
	if (time > left_time)
	{
		time = left_time;
	}

	guild->building_upgrade_end -= time;
	if (guild->building_upgrade_end <= now)
	{
		upgrade_building_level(guild);
	}
	else
	{
		broadcast_building_upgrade_update(guild);
		save_guild_info(guild);
	}
	
	return 0;
}


GuildSkill *get_guild_skill_info(GuildInfo *guild, uint32_t skill_id)
{
	for (int i = 0; i < MAX_GUILD_SKILL_NUM; ++i)
	{
		if (guild->skills[i].skill_id == 0 || guild->skills[i].skill_id == skill_id)
		{
			return &guild->skills[i];
		}
	}
	
	return NULL;
}

GuildSkill *get_player_skill_info(GuildPlayer *player, uint32_t skill_id)
{
	for (int i = 0; i < MAX_GUILD_SKILL_NUM; ++i)
	{
		if (player->skills[i].skill_id == 0 || player->skills[i].skill_id == skill_id)
		{
			return &player->skills[i];
		}
	}
	
	return NULL;
}

void broadcast_skill_develop_update(GuildInfo *guild, GuildSkill *skill)
{
	std::vector<uint64_t> player_ids;
	get_guild_broadcast_objects(guild, player_ids);
	if (player_ids.size() == 0)
	{
		return ;
	}

	GuildSkillData nty;
	guild_skill_data__init(&nty);

	nty.skillid = skill->skill_id;
	nty.level = skill->skill_lv;

	conn_node_guildsrv::broadcast_message(MSG_ID_GUILD_SKILL_DEVELOP_NOTIFY, &nty, (pack_func)guild_skill_data__pack, player_ids);
}

void sync_guild_skill_to_gamesrv(GuildPlayer *player)
{
	PROTO_SYNC_GUILD_SKILL *req = (PROTO_SYNC_GUILD_SKILL *)conn_node_base::get_send_buf(SERVER_PROTO_SYNC_GUILD_SKILL, 0);
	req->head.len = ENDION_FUNC_4(sizeof(PROTO_SYNC_GUILD_SKILL));
	memset(req->head.data, 0, sizeof(PROTO_SYNC_GUILD_SKILL) - sizeof(PROTO_HEAD));
	for (int i = 0; i < MAX_GUILD_SKILL_NUM; ++i)
	{
		req->skills[i].skill_id = player->skills[i].skill_id;
		req->skills[i].skill_lv = player->skills[i].skill_lv;
	}

	EXTERN_DATA ext_data;
	ext_data.player_id = player->player_id;
	conn_node_base::add_extern_data(&req->head, &ext_data);

	if (conn_node_guildsrv::connecter.send_one_msg(&req->head, 1) != (int)ENDION_FUNC_4(req->head.len))
	{
		LOG_ERR("[%s:%d] send to conn_srv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
}

void resp_guild_shop_info(uint64_t player_id)
{
	GuildPlayer* player = get_guild_player(player_id);

	SpecialShopInfoAnswer resp;
	special_shop_info_answer__init(&resp);

	GoodsData goods_data[MAX_GUILD_GOODS_NUM];
	GoodsData* goods_data_point[MAX_GUILD_GOODS_NUM];

	resp.result = 0;
	resp.goods = goods_data_point;
	resp.n_goods = 0;
	for (std::map<uint64_t, ShopTable*>::iterator iter = shop_config.begin(); iter != shop_config.end() && resp.n_goods < MAX_GUILD_GOODS_NUM; ++iter)
	{
		ShopTable *config = iter->second;
		uint32_t goods_id = iter->first;
		if (!config)
		{
			continue;
		}

		if (config->ShopType != 280003011)
		{
			continue;
		}

		goods_data_point[resp.n_goods] = &goods_data[resp.n_goods];
		goods_data__init(&goods_data[resp.n_goods]);
		goods_data[resp.n_goods].goodsid = goods_id;

		if (player)
		{
			GuildGoods *info = get_player_goods_info(player, goods_id);
			if (info)
			{
				if (info->goods_id == 0)
				{
					info->goods_id = goods_id;
				}

				goods_data[resp.n_goods].boughtnum = info->bought_num;
			}
			else
			{
				goods_data[resp.n_goods].boughtnum = 0;
			}
		}
		else
		{
			goods_data[resp.n_goods].boughtnum = 0;
		}

		resp.n_goods++;
	}

	EXTERN_DATA ext_data;
	ext_data.player_id = player_id;

	fast_send_msg(&conn_node_guildsrv::connecter, &ext_data, MSG_ID_SPECIAL_SHOP_INFO_ANSWER, special_shop_info_answer__pack, resp);
}

GuildGoods *get_player_goods_info(GuildPlayer* player, uint32_t goods_id)
{
	for (int i = 0; i < MAX_GUILD_GOODS_NUM; ++i)
	{
		if (player->goods[i].goods_id == 0 || player->goods[i].goods_id == goods_id)
		{
			return &player->goods[i];
		}
	}

	return NULL;
}

bool goods_is_on_sell(GuildInfo *guild, uint32_t goods_id)
{
	uint32_t shop_level = get_building_level(guild, Building_Shop);
	GangsTable *config = get_guild_building_config(Building_Shop, shop_level);
	if (!config)
	{
		return false;
	}

	for (size_t i = 0; i < config->n_parameter4; ++i)
	{
		if (config->parameter4[i] == goods_id)
		{
			return true;
		}
	}

	return false;
}

void broadcast_guild_chat(GuildInfo *guild, Chat *msg)
{
	std::vector<uint64_t> player_ids;
	get_guild_broadcast_objects(guild, player_ids);
	if (player_ids.size() == 0)
	{
		return ;
	}

	conn_node_guildsrv::broadcast_message(MSG_ID_CHAT_NOTIFY, msg, (pack_func)chat__pack, player_ids);
}

void broadcast_guild_message(GuildInfo *guild, uint16_t msg_id, void *msg_data, pack_func func, uint64_t except /* = 0 */)
{
	std::vector<uint64_t> player_ids;
	get_guild_broadcast_objects(guild, player_ids);
	if (player_ids.size() == 0)
	{
		return;
	}

	conn_node_guildsrv::broadcast_message(msg_id, msg_data, func, player_ids);
}

void open_all_guild_answer()
{
	//uint32_t question[GuildAnswer::MAX_SEND_GUILD_QUESTION];
	//for (int i = 0; i < GuildAnswer::MAX_SEND_GUILD_QUESTION; ++i)
	//{
	//	question[i] = sg_guild_question[rand() % sg_guild_question.size()];
	//}
	
	for (std::map<uint32_t, GuildInfo*>::iterator iter = guild_map.begin(); iter != guild_map.end(); ++iter)
	{
		GuildInfo *guild = iter->second;
		guild->answer.Start(guild);//, question, GuildAnswer::MAX_SEND_GUILD_QUESTION);
	}
}

bool guild_battle_is_final(uint32_t activity_id)
{
	return (activity_id == 330100025);
}

void clear_player_act_battle_score() //清除玩家本次活动积分
{
	for (std::map<uint64_t, GuildPlayer*>::iterator iter = guild_player_map.begin(); iter != guild_player_map.end(); ++iter)
	{
		if (iter->second->act_battle_score == 0)
		{
			continue;
		}

		iter->second->act_battle_score = 0;
		save_guild_player(iter->second);
	}
}

void clear_player_total_battle_score()
{
	for (std::map<uint64_t, GuildPlayer*>::iterator iter = guild_player_map.begin(); iter != guild_player_map.end(); ++iter)
	{
		if (iter->second->battle_score == 0 && iter->second->act_battle_score == 0)
		{
			continue;
		}

		iter->second->act_battle_score = 0;
		iter->second->battle_score = 0;
		save_guild_player(iter->second);
	}

	for (std::map<uint32_t, GuildInfo*>::iterator iter = guild_map.begin(); iter != guild_map.end(); ++iter)
	{
		if (iter->second->battle_score == 0)
		{
			continue;
		}
		iter->second->battle_score = 0;
		save_guild_info(iter->second);
	}

	sg_redis_client.del(sg_rank_guild_battle_key);
}

void save_guild_battle_final_list(std::vector<uint32_t> &guild_ids)
{
	CRedisClient &rc = sg_redis_client;
	std::ostringstream os;
	for (size_t i = 0; i < guild_ids.size(); ++i)
	{
		if (i != 0)
		{
			os << "&";
		}
		os << guild_ids[i];
	}
	int ret = rc.set(sg_guild_battle_final_key, os.str().c_str());
	if (ret != 0)
	{
		LOG_ERR("[%s:%d] set failed, ret:%d", __FUNCTION__, __LINE__, ret);
	}
}

int load_guild_battle_final_list(std::vector<uint32_t> &guild_ids)
{
	CRedisClient &rc = sg_redis_client;
	int ret = rc.exist(sg_guild_battle_final_key);
	if (ret <= 0)
	{
		return -1;
	}

	std::string str_final = rc.get(sg_guild_battle_final_key);
	if (str_final.size() == 0)
	{
		return 0;
	}

	size_t posBeg = 0;
	size_t posEnd = 0;
	while (true)
	{
		if (posBeg >= str_final.length())
		{
			break;
		}

		posEnd = str_final.find_first_of("&", posBeg);
	    if (posEnd == std::string::npos)
		{
			posEnd = str_final.length();
		}
		
		std::string tmp = str_final.substr(posBeg, posEnd - posBeg);
		posBeg = posEnd + 1;

		uint32_t guild_id = strtoul(tmp.c_str(), NULL, 0);
		guild_ids.push_back(guild_id);
	}

	return 0;
}

bool is_guild_battle_opening()
{
	if (guild_battle_opening)
	{
		return true;
	}
	else
	{
		return is_in_guild_battle_activity_time();
	}

	return false;
}

bool is_in_guild_battle_activity_time()
{
	uint32_t now = time_helper::get_cached_time() / 1000;
	std::vector<uint32_t> act_ids;
	act_ids.push_back(330100024);
	act_ids.push_back(330100025);
	bool in_time = false;
	for (std::vector<uint32_t>::iterator iter = act_ids.begin(); iter != act_ids.end(); ++iter)
	{
		EventCalendarTable *act_config = get_config_by_id(*iter, &activity_config);
		if (!act_config)
		{
			continue;
		}

		ControlTable *ctrl_config = get_config_by_id(act_config->RelationID, &all_control_config);
		if (!ctrl_config)
		{
			continue;
		}

		in_time = control_is_open(ctrl_config, now);
		if (in_time)
		{
			break;
		}
	}

	return in_time;
}

uint32_t get_guild_land_active_reward_count(GuildPlayer *player, uint32_t guild_active_id)
{
	if(player == NULL)
		return 0;
	for (int i = 0; i < MAX_GUILD_LAND_ACTIVE_NUM; ++i)
	{
		if (player->guild_land_active_reward_id[i] == 0)
		{
			return 0;
		}
		if (player->guild_land_active_reward_id[i] == guild_active_id)
		{
			return player->guild_land_active_reward_num[i];
		}
	}
	return 0;
}
void  add_guild_land_active_reward_count(GuildPlayer *player, uint32_t guild_active_id)
{

	if(player == NULL)
		return;
	for (int i = 0; i < MAX_GUILD_LAND_ACTIVE_NUM; ++i)
	{
		if (player->guild_land_active_reward_id[i] == 0)
		{
			player->guild_land_active_reward_id[i] = guild_active_id;
			player->guild_land_active_reward_num[i] = 1;
			return;
		}

		if (player->guild_land_active_reward_id[i] == guild_active_id)
		{
			player->guild_land_active_reward_num[i]++;
			return;
		}
	}
}


int get_player_donate_remain_count(GuildPlayer *player)
{
	ControlTable *config = get_config_by_id(330400058, &all_control_config);
	if (config)
	{
		return ((int)config->RewardTime - (int)player->donate_count);
	}

	return 0;
}

bool guild_bonfire_has_opened(GuildInfo *guild)
{
	return (guild->bonfire_open_time != 0 && time_helper::nextOffsetTime(daily_reset_clock, guild->bonfire_open_time) > time_helper::get_cached_time() / 1000);
}

bool is_in_guild_bonfire_activity_time()
{
	do
	{
		EventCalendarTable *act_config = get_config_by_id(330000043, &activity_config);
		if (!act_config)
		{
			break;
		}

		ControlTable *ctrl_config = get_config_by_id(act_config->RelationID, &all_control_config);
		if (!ctrl_config)
		{
			break;
		}

		uint32_t now = time_helper::get_cached_time() / 1000;
		return control_is_open(ctrl_config, now);
	} while(0);
	return false;
}

