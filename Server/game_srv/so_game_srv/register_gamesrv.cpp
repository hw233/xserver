#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <evhttp.h>
#include <signal.h>

#include "game_event.h"
#include "time_helper.h"
#include "conn_node.h"
#include "conn_node_dbsrv.h"
#include "player_manager.h"
#include "install_monster_ai.h"
#include "install_partner_ai.h"
#include "install_raid_ai.h"
#include "pvp_player_ai.h"
#include "doufachang_player_ai.h"
#include "monster.h"
#include "register_gamesrv.h"
#include "msgid.h"
#include "scene.h"
#include "game_config.h"
#include "player_manager.h"
#include "scene_manager.h"
#include "raid_manager.h"
#include "zhenying_battle.h"
#include "zhenying_raid_manager.h"
#include "sight_space_manager.h"
#include "monster_manager.h"
#include "partner_manager.h"
#include "cash_truck_manager.h"
#include "skill_manager.h"
#include "buff_manager.h"
#include "sight_space_manager.h"
#include "cgi_common.h"
#include "time_helper.h"
#include "test_timer.h"
#include "unit.h"
#include "team.h"
#include "collect.h"
#include "pvp_match_manager.h"
#include "guild_battle_manager.h"
#include "guild_wait_raid_manager.h"
#include "chengjie.h"
#include "global_shared_data.h"
#include "team.h"
#include "deamon.h"
#include "flow_record.h"
#include "server_level.h"
//#define run_with_period(_ms_) if ((_ms_ <= 1000/server.hz) || !(server.cronloops%((_ms_)/(1000/server.hz))))
#define run_with_period(_ms_) if (timer_loop_count % _ms_ == 0)

static int default_handle(player_struct *player, EXTERN_DATA *extern_data)
{
	LOG_ERR("%s %d", __FUNCTION__, __LINE__);
	return (0);
}
static void cb_signal2(evutil_socket_t fd, short events, void *arg)
{
	change_mycat();
	LOG_INFO("%s: fd = %d, events = %d, arg = %p", __FUNCTION__, fd, events, arg);
}

static void install_all_msg()
{
	install_msg_handle();
	install_monster_ai();
	install_partner_ai();	
	install_raid_ai();
	install_db_msg_handle();
	install_pvp_ai_player_handle();
	install_doufachang_ai_player_handle();	
}

static void	clear_all_mem()
{
		//player
	for (std::list<player_struct *>::iterator ite = player_manager_player_free_list.begin();
		 ite != player_manager_player_free_list.end(); ++ite)
	{
		delete (*ite);
	}
	for (std::set<player_struct *>::iterator ite = player_manager_player_used_list.begin();
		 ite != player_manager_player_used_list.end(); ++ite)
	{
		(*ite)->clear();
		delete (*ite);
	}

		//buff
	for (std::list<buff_struct *>::iterator ite = buff_manager_buff_free_list.begin();
		 ite != buff_manager_buff_free_list.end(); ++ite)
	{
		delete (*ite);
	}
	for (std::set<buff_struct *>::iterator ite = buff_manager_buff_used_list.begin();
		 ite != buff_manager_buff_used_list.end(); ++ite)
	{
		delete (*ite);
	}

		//monster boss
	for (std::list<monster_struct *>::iterator ite = monster_manager_monster_free_list.begin();
		 ite != monster_manager_monster_free_list.end(); ++ite)
	{
		delete (*ite);
	}
	for (std::set<monster_struct *>::iterator ite = monster_manager_monster_used_list.begin();
		 ite != monster_manager_monster_used_list.end(); ++ite)
	{
		delete (*ite);		
	}
	// for (std::list<boss_struct *>::iterator ite = monster_manager_boss_free_list.begin();
	// 	 ite != monster_manager_boss_free_list.end(); ++ite)
	// {
	// 	delete (*ite);
	// }
	// for (std::set<boss_struct *>::iterator ite = monster_manager_boss_used_list.begin();
	// 	 ite != monster_manager_boss_used_list.end(); ++ite)
	// {
	// 	delete (*ite);		
	// }

		//cash_truck
	for (std::list<cash_truck_struct *>::iterator ite = cash_truck_manager_free_list.begin();
		 ite != cash_truck_manager_free_list.end(); ++ite)
	{
		delete (*ite);		
	}
	for (std::set<cash_truck_struct *>::iterator ite = cash_truck_manager_used_list.begin();
		 ite != cash_truck_manager_used_list.end(); ++ite)
	{
		delete (*ite);		
	}
		
		//raid_struct
	for (std::list<raid_struct *>::iterator ite = raid_manager_raid_free_list.begin();
		 ite != raid_manager_raid_free_list.end(); ++ite)
	{
		delete (*ite);
	}
	for (std::set<raid_struct *>::iterator ite = raid_manager_raid_used_list.begin();
		 ite != raid_manager_raid_used_list.end(); ++ite)
	{
		delete (*ite);		
	}
	
	for (std::map<uint64_t, scene_struct *>::iterator ite = scene_manager_scene_map.begin();
		 ite != scene_manager_scene_map.end(); ++ite)
	{
		delete ite->second;
	}

	for (std::map<uint64_t, scene_struct *>::iterator ite = scene_manager_guild_scene_map.begin();
		 ite != scene_manager_guild_scene_map.end(); ++ite)
	{
		delete ite->second;
	}
		//位面的析构里面会有删除怪物等操作，可能会引发别的问题
	// for (std::vector<sight_space_struct *>::iterator ite = sight_space_manager_mark_delete_sight_space.begin();
	// 	 ite != sight_space_manager_mark_delete_sight_space.end(); ++ite)
	// {
	// 	delete (*ite);
	// }

	for (std::list<skill_struct *>::iterator ite = skill_manager_skill_free_list.begin();
		 ite != skill_manager_skill_free_list.end(); ++ite)
	{
		delete (*ite);
	}
	for (std::set<skill_struct *>::iterator ite = skill_manager_skill_used_list.begin();
		 ite != skill_manager_skill_used_list.end(); ++ite)
	{
		delete (*ite);
	}

		//zhenying_raid
	for (std::list<zhenying_raid_struct *>::iterator ite = zhenying_raid_manager_raid_free_list.begin();
		 ite != zhenying_raid_manager_raid_free_list.end(); ++ite)
	{
		delete(*ite);
	}
	for (std::set<zhenying_raid_struct *>::iterator ite = zhenying_raid_manager_raid_used_list.begin();
		 ite != zhenying_raid_manager_raid_used_list.end(); ++ite)
	{
		delete (*ite);
	}

		//guild wait raid
	for (std::list<guild_wait_raid_struct *>::iterator ite = guild_wait_raid_manager_raid_free_list.begin();
		 ite != guild_wait_raid_manager_raid_free_list.end(); ++ite)
	{
		delete (*ite);
	}
	for (std::set<guild_wait_raid_struct *>::iterator ite = guild_wait_raid_manager_raid_used_list.begin();
		 ite != guild_wait_raid_manager_raid_used_list.end(); ++ite)
	{
		delete (*ite);
	}

	for (TEAM_MAP::iterator ite = team_manager_s_teamContain.begin();
		 ite != team_manager_s_teamContain.end(); ++ite)
	{
		if (ite->second->m_data)
			ite->second->m_data->m_raid_uuid = 0;
		delete ite->second;
	}
	for (COLLECT_MAP::iterator ite = collect_manager_s_collectContain.begin();
		 ite != collect_manager_s_collectContain.end(); ++ite)
	{
		delete ite->second;
	}

		//partner
	for (std::list<partner_struct *>::iterator ite = partner_manager_partner_free_list.begin();
		 ite != partner_manager_partner_free_list.end(); ++ite)
	{
		delete (*ite);
	}
	for (std::set<partner_struct *>::iterator ite = partner_manager_partner_used_list.begin();
		 ite != partner_manager_partner_used_list.end(); ++ite)
	{
		delete (*ite);
	}
}

static int get_server_open_time(void)
{
	ServerResTable *config = get_config_by_id(sg_server_id, &server_res_config);
	if (!config)
	{
		return -1;
	}

	sg_server_open_time = config->OpenTime;

	return 0;
}

extern "C"
{
int uninstall()
{
	LOG_INFO("%s %d", __PRETTY_FUNCTION__, __LINE__);
	uninstall_msg_handle();
	uninstall_monster_ai();
	uninstall_partner_ai();	
	uninstall_raid_ai();
	uninstall_db_msg_handle();

	for (int i = 0; i < MAX_PLAYER_AI_TYPE; ++i)
		player_manager::m_ai_player_handle[i] = NULL;

	return (0);
}
	
int reload()
{
	LOG_INFO("%s %d", __PRETTY_FUNCTION__, __LINE__);

	unit_struct::init_pos_pool();
	unit_struct::init_buff_pool();	
	init_sight_unit_info_point();
	init_heap(&g_minheap, 250000, minheap_cmp_map_block, minheap_get_map_block_index, minheap_set_map_block_index);
	monster_manager::reinit_monster_min_heap();
	monster_manager::reinit_boss_min_heap();	
	buff_manager::reinit_min_heap();
	partner_manager::reinit_partner_min_heap();

	// if (read_all_excel_data() != 0) {
	// 	LOG_ERR("[%s : %d]: read excel data failed", __FUNCTION__, __LINE__);
	// 	return -1;		
	// }
	if (reload_config() != 0) {
	 	LOG_ERR("[%s : %d]: reload config failed", __FUNCTION__, __LINE__);
	}

	install_all_msg();

	monster_manager::reset_all_monster_ai();
	partner_manager::reset_all_partner_ai();	
	raid_manager::reset_all_raid_ai();
	return (0);
}
	
int install(int argc, char **argv)
{
//	LOG_INFO("%s %d", __PRETTY_FUNCTION__, __LINE__);	
	int ret = 0;
	FILE *file = NULL;
//	std::ifstream file;
	char *line;
	int i;
	int dump = 0;	/// dump shared memroy
	int web_port = 0;	
 	std::string szServerIP;
	int port;
	struct sockaddr_in sin;
	int player_num;
	unsigned long player_key;

	//std::string szRedisIp="";
	//int nRedisPort=0;
	//int nTimeOut=0;

	ret = init_signals();
	if (0 != ret) {
		LOG_ERR("[%s : %d]: init signals failed, ret: %d", __FUNCTION__, __LINE__, ret);
		return ret;
	}

	unit_struct::init_pos_pool();
	unit_struct::init_buff_pool();	
	init_sight_unit_info_point();
	init_heap(&g_minheap, 250000, minheap_cmp_map_block, minheap_get_map_block_index, minheap_set_map_block_index);

//	g_config_data.loadData();
		//500 * 500
	if (read_all_excel_data() != 0) {
		LOG_ERR("[%s : %d]: read excel data failed", __FUNCTION__, __LINE__);
		return -1;		
	}

	for (i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-d") == 0) {
			change_to_deamon();
			continue;
//		}
//		else if (strcmp(argv[i], "-r") == 0) {
//			resume = 1;
//			continue;
		} else if(strcmp(argv[i], "-o") == 0) { /// dump shared memory data to db
			dump = 1;
		}
	    else if(strcmp(argv[i], "-t") == 0) { /// test for mem check
			open_err_log_file();
		}
	}

	uint64_t pid = write_pid_file();
    LOG_INFO("game_srv run %lu", pid);

	ret = game_event_init();
	if (ret != 0)
		goto done;

 	file = fopen("../server_info.ini", "r");
 	if (!file) {
 		LOG_ERR("open server_info.ini failed[%d]", errno);
 		ret = -1;
 		goto done;
 	}

	line = get_first_key(file, (char*)"game_srv_id");
	if (!line)
	{
		LOG_ERR("config file wrong, no game_srv_id");
		ret = -1;
		goto done;
	}
	sg_server_id = atoi(get_value(line));

	line = get_first_key(file, (char*)"open_gm_cmd");
	if (line) {
		sg_gm_cmd_open = atoi(get_value(line));
	}


	line = get_first_key(file, (char *)"game_srv_player_num");
	player_num = atoi(get_value(line));
	if (player_num <= 0) {
		LOG_ERR("config file wrong, no game_srv_player_num");
		ret = -1;
		goto done;
	}
	line = get_first_key(file, (char *)"game_srv_player_key");
	player_key = strtoul(get_value(line), NULL, 0);
	if (player_key == 0 || player_key == ULONG_MAX) {
		LOG_ERR("config file wrong, no game_srv_key");
		ret = -1;
		goto done;
	}
	if (player_manager::init_player_struct(player_num, player_key) != 0) {
		LOG_ERR("init player struct failed");
		ret = -1;
		goto done;
	}

	line = get_first_key(file, (char *)"game_srv_raid_num");
	player_num = atoi(get_value(line));
	if (player_num <= 0) {
		LOG_ERR("config file wrong, no game_srv_raid_num");
		ret = -1;
		goto done;
	}
	line = get_first_key(file, (char *)"game_srv_raid_key");
	player_key = strtoul(get_value(line), NULL, 0);
	if (player_key == 0 || player_key == ULONG_MAX) {
		LOG_ERR("config file wrong, no game_srv_raid_key");
		ret = -1;
		goto done;
	}
	if (raid_manager::init_raid_struct(player_num, player_key) != 0) {
		LOG_ERR("init raid struct failed");
		ret = -1;
		goto done;
	}

	line = get_first_key(file, (char *)"global_data_key");
	player_key = strtoul(get_value(line), NULL, 0);
	if (player_key == 0 || player_key == ULONG_MAX) {
		LOG_ERR("config file wrong, no global_data_key");
		ret = -1;
		goto done;
	}
	if (init_global_shared_data(player_key) != 0) {
		LOG_ERR("init global shared data failed");
		ret = -1;
		goto done;		
	}
	

	{
		line = get_first_key(file, (char *)"game_srv_zhenying_raid_num");
		player_num = atoi(get_value(line));
		if (player_num <= 0) {
			LOG_ERR("config file wrong, no game_srv_zhenying_raid_num");
			ret = -1;
			goto done;
		}
		line = get_first_key(file, (char *)"game_srv_zhenying_raid_key");
		player_key = strtoul(get_value(line), NULL, 0);
		if (player_key == 0 || player_key == ULONG_MAX) {
			LOG_ERR("config file wrong, no game_srv_zhenying_raid_key");
			ret = -1;
			goto done;
		}
		if (zhenying_raid_manager::init_zhenying_raid_struct(player_num, player_key) != 0) {
			LOG_ERR("init zhenying raid struct failed");
			ret = -1;
			goto done;
		}
	}

	{
		line = get_first_key(file, (char *)"game_srv_guild_wait_raid_num");
		player_num = atoi(get_value(line));
		if (player_num <= 0) {
			LOG_ERR("config file wrong, no game_srv_guild_wait_raid_num");
			ret = -1;
			goto done;
		}
		line = get_first_key(file, (char *)"game_srv_guild_wait_raid_key");
		player_key = strtoul(get_value(line), NULL, 0);
		if (player_key == 0 || player_key == ULONG_MAX) {
			LOG_ERR("config file wrong, no game_srv_guild_wait_raid_key");
			ret = -1;
			goto done;
		}
		if (guild_wait_raid_manager::init_guild_wait_raid_struct(player_num, player_key) != 0) {
			LOG_ERR("init guild_wait raid struct failed");
			ret = -1;
			goto done;
		}
	}

	line = get_first_key(file, (char *)"game_srv_skill_num");
	player_num = atoi(get_value(line));
	if (player_num <= 0) {
		LOG_ERR("config file wrong, no game_srv_skill_num");
		ret = -1;
		goto done;
	}
	line = get_first_key(file, (char *)"game_srv_skill_key");
	player_key = strtoul(get_value(line), NULL, 0);
	if (player_key == 0 || player_key == ULONG_MAX) {
		LOG_ERR("config file wrong, no skill_srv_key");
		ret = -1;
		goto done;
	}
	if (skill_manager::init_skill_struct(player_num, player_key) != 0) {
		LOG_ERR("init skill struct failed");
		ret = -1;
		goto done;
	}

	line = get_first_key(file, (char *)"game_srv_buff_num");
	player_num = atoi(get_value(line));
	if (player_num <= 0) {
		LOG_ERR("config file wrong, no game_srv_buff_num");
		ret = -1;
		goto done;
	}
	line = get_first_key(file, (char *)"game_srv_buff_key");
	player_key = strtoul(get_value(line), NULL, 0);
	if (player_key == 0 || player_key == ULONG_MAX) {
		LOG_ERR("config file wrong, no buff_srv_key");
		ret = -1;
		goto done;
	}
	if (buff_manager::init_buff_struct(player_num, player_key) != 0) {
		LOG_ERR("%d init buff struct failed", __LINE__);
		ret = -1;
		goto done;
	}

	line = get_first_key(file, (char *)"game_srv_team_num");
	player_num = atoi(get_value(line));
	if (player_num <= 0) {
		LOG_ERR("config file wrong, no game_srv_buff_num");
		ret = -1;
		goto done;
	}
	line = get_first_key(file, (char *)"game_srv_team_key");
	player_key = strtoul(get_value(line), NULL, 0);
	if (player_key == 0 || player_key == ULONG_MAX) {
		LOG_ERR("config file wrong, no buff_srv_key");
		ret = -1;
		goto done;
	}
	if (Team::InitTeamData(player_num * 2, player_key) != 0) {
		LOG_ERR("%d: init TeamData failed", __LINE__);
		ret = -1;
		goto done;
	}

	line = get_first_key(file, (char *)"game_srv_sight_space_num");
	player_num = atoi(get_value(line));
	if (player_num <= 0) {
		LOG_ERR("config file wrong, no game_srv_sight_space_num");
		ret = -1;
		goto done;
	}
	line = get_first_key(file, (char *)"game_srv_sight_space_key");
	player_key = strtoul(get_value(line), NULL, 0);
	if (player_key == 0 || player_key == ULONG_MAX) {
		LOG_ERR("config file wrong, no sight_space_key");
		ret = -1;
		goto done;
	}
	if (sight_space_manager::init_sight_space(player_num, player_key) != 0) {
		LOG_ERR("init sight space struct failed");
		ret = -1;
		goto done;
	}

	line = get_first_key(file, (char *)"game_srv_monster_num");
	player_num = atoi(get_value(line));
	if (player_num <= 0) {
		LOG_ERR("config file wrong, no game_srv_monster_num");
		ret = -1;
		goto done;
	}
	line = get_first_key(file, (char *)"game_srv_monster_key");
	player_key = strtoul(get_value(line), NULL, 0);
	if (player_key == 0 || player_key == ULONG_MAX) {
		LOG_ERR("config file wrong, no game_srv_monster_key");
		ret = -1;
		goto done;
	}
	if (monster_manager::init_monster_struct(player_num, player_key) != 0) {
		LOG_ERR("%d init monster struct failed", __LINE__);
		ret = -1;
		goto done;
	}

	// line = get_first_key(file, (char *)"game_srv_boss_num");
	// player_num = atoi(get_value(line));
	// if (player_num <= 0) {
	// 	LOG_ERR("config file wrong, no game_srv_boss_num");
	// 	ret = -1;
	// 	goto done;
	// }
	// line = get_first_key(file, (char *)"game_srv_boss_key");
	// player_key = strtoul(get_value(line), NULL, 0);
	// if (player_key == 0 || player_key == ULONG_MAX) {
	// 	LOG_ERR("config file wrong, no game_srv_boss_key");
	// 	ret = -1;
	// 	goto done;
	// }
	// if (monster_manager::init_boss_struct(player_num, player_key) != 0) {
	// 	LOG_ERR("%d: init boss struct failed", __LINE__);
	// 	ret = -1;
	// 	goto done;
	// }

	{
		line = get_first_key(file, (char *)"game_srv_partner_num");
		player_num = atoi(get_value(line));
		if (player_num <= 0) {
			LOG_ERR("config file wrong, no game_srv_partner_num");
			ret = -1;
			goto done;
		}
		line = get_first_key(file, (char *)"game_srv_partner_key");
		player_key = strtoul(get_value(line), NULL, 0);
		if (player_key == 0 || player_key == ULONG_MAX) {
			LOG_ERR("config file wrong, no game_srv_partner_key");
			ret = -1;
			goto done;
		}
		if (partner_manager::init_partner_struct(player_num, player_key) != 0) {
			LOG_ERR("init partner struct failed");
			ret = -1;
			goto done;
		}
	}

	{
		line = get_first_key(file, (char *)"game_srv_truck_num");
		player_num = atoi(get_value(line));
		if (player_num <= 0) {
			LOG_ERR("config file wrong, no game_srv_truck_num");
			ret = -1;
			goto done;
		}
		line = get_first_key(file, (char *)"game_srv_truck_key");
		player_key = strtoul(get_value(line), NULL, 0);
		if (player_key == 0 || player_key == ULONG_MAX) {
			LOG_ERR("config file wrong, no game_srv_truck_key");
			ret = -1;
			goto done;
		}
		if (cash_truck_manager::init_cash_truck_struct(player_num, player_key) != 0) {
			LOG_ERR("init truck struct failed");
			ret = -1;
			goto done;
		}
	}

	install_all_msg();
	
	if (add_all_scene() != 0)
	{
		LOG_ERR("add all scene fail");
		goto done;
	}
	zhenying_raid_manager::create_all_line();

	if (!dump)
	{
		line = get_first_key(file, (char *)"conn_srv_game_port");
		port = atoi(get_value(line));
		if (port <= 0) {
			LOG_ERR("config file wrong, no conn_srv_game_port");
			ret = -1;
			goto done;
		}
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		ret = evutil_inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr);
		if (ret != 1) {
			LOG_ERR("evutil_inet_pton failed[%d]", ret);
			goto done;
		}
		ret = game_add_connect_event((struct sockaddr *)&sin, sizeof(sin), &conn_node_gamesrv::connecter);
		if (ret <= 0)
			goto done;

		//connect db_srv
		line = get_first_key(file, (char *)"db_srv_port");
		port = atoi(get_value(line));
		if (port <= 0) {
			LOG_ERR("config file wrong, no db_srv_port");
			ret = -1;
			goto done;
		}
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		ret = evutil_inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr);
		if (ret != 1) {
			LOG_ERR("%s %d: evutil_inet_pton failed[%d]", __FUNCTION__, __LINE__, ret);
			goto done;
		}

		ret = game_add_connect_event((struct sockaddr *)&sin, sizeof(sin), &conn_node_dbsrv::connecter);
		if (ret <= 0)
			goto done;
		/*
		//connect item_srv
		line = get_first_key(file, (char *)"item_srv_game_port");
		port = atoi(get_value(line));
		if (port <= 0) {
			LOG_ERR("config file wrong, no item_srv_game_port");
			ret = -1;
			goto done;
		}
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		ret = evutil_inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr);
		if (ret != 1) {
			LOG_ERR("%s %d: evutil_inet_pton failed[%d]", __FUNCTION__, __LINE__, ret);
			goto done;
		}

		ret = game_add_connect_event((struct sockaddr *)&sin, sizeof(sin), &conn_node_itemsrv::connecter);
		if (ret <= 0)
			goto done;
			*/
	}

	line = get_first_key(file, (char *)"game_srv_tick_time");
	gamesrv_timeout.tv_usec = atoi(get_value(line)) * 1000;

	/*
	if (dump) {
		int size = player_manager::player_manager_all_players_id.size();
		std::map<uint64_t, player_struct *>::iterator it = player_manager::player_manager_all_players_id.begin();
		for (; it!=player_manager::player_manager_all_players_id.end(); ++it) {
			player_struct* player = it->second;
			if (!player)
			{
				LOG_ERR("%s %d: can not find player %lu, size = %d", __FUNCTION__, __LINE__, it->first, size);
				continue;
			}
			LOG_INFO("%s %d: cache  player %lu to db, size = %d", __FUNCTION__, __LINE__, it->first, size);
			player->cache_to_db();
			LOG_INFO("%s %d: finish cache  player %lu to db, size = %d", __FUNCTION__, __LINE__, it->first, size);
		}
		goto done;
	}

	memset(&conn_node_worldsrv::sin, 0, sizeof(conn_node_worldsrv::sin));
	conn_node_worldsrv::sin.sin_family = AF_INET;
	line = get_first_key(file, (char *)"world_srv_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no db_srv_port");
		ret = -1;
		goto done;
	}
	conn_node_worldsrv::sin.sin_port = htons(port);
	line = get_first_key(file, (char *)"world_srv_ip");
	line = get_value(line);
	ret = evutil_inet_pton(AF_INET, line, &conn_node_worldsrv::sin.sin_addr);
	if (ret != 1) {
		LOG_ERR("%s %d: evutil_inet_pton failed[%d][%s]", __FUNCTION__, __LINE__, ret, line);
		goto done;
	}
	conn_node_worldsrv::connecter = new(std::nothrow) conn_node_worldsrv;
	assert(conn_node_worldsrv::connecter);
	ret = game_add_connect_event((struct sockaddr *)&conn_node_worldsrv::sin, sizeof(conn_node_worldsrv::sin), conn_node_worldsrv::connecter);
	if (ret <= 0) {
		LOG_ERR("%s %d: connect to world server failed[%d] ip[%s] port[%u]", __FUNCTION__, __LINE__, ret, line, port);
		delete conn_node_worldsrv::connecter;
//		goto done;
	} else {
		conn_node_worldsrv::connecter->on_connected();		
	}

	conn_node_worldsrv::bind_handle_func();
*/

#if 0
	if (SIG_ERR == signal(SIGPIPE,SIG_IGN)) {
		LOG_ERR("set sigpipe ign failed");
		return (0);
	}
#endif

	add_signal(SIGUSR2, NULL, cb_signal2);


	line = get_first_key(file, (char *)"game_srv_web_port");
	if (line) {
		web_port = atoi(get_value(line));
		if (web_port <= 0) {
			LOG_ERR("config file wrong, no conn_srv_boss_web_port");
//		ret = -1;
//		goto done;
		}
	}
	init_http_server(web_port);
	
	get_server_open_time();
	conn_node_gamesrv::notify_gamesrv_start();
	ChengJieTaskManage::LoadAllTask();
	load_server_level_info();
	init_guild_battle_manager();

//	ret = reload();
	ret = 0;
done:
	fclose(file);
	return ret;
	
}

void on_http_request(struct evhttp_request *req, void *arg)
{
    if (strcmp(req->uri, "/guild_wait") == 0)
	{
		struct evbuffer *returnbuffer = evbuffer_new();
		print_waiting_player(returnbuffer);
		evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);					
		evbuffer_free(returnbuffer);					
	}
    if (strcmp(req->uri, "/object") == 0)
	{
		struct evbuffer *returnbuffer = evbuffer_new();
		evbuffer_add_printf(returnbuffer, "player: %lu/%u<br><br>\n", player_manager_all_players_id.size(), player_manager::get_pool_max_num());
		evbuffer_add_printf(returnbuffer, "monster: %lu/%u<br><br>\n", monster_manager_all_monsters_id.size(), monster_manager::get_monster_pool_max_num());
//		evbuffer_add_printf(returnbuffer, "boss: %lu/%u<br><br>\n", monster_manager_all_boss_id.size(), monster_manager::get_boss_pool_max_num());
		evbuffer_add_printf(returnbuffer, "raid: %lu/%u<br><br>\n", raid_manager_all_raid_id.size(), raid_manager::get_raid_pool_max_num());
		evbuffer_add_printf(returnbuffer, "skill: %u/%u<br><br>\n", skill_manager::get_skill_count(), skill_manager::get_skill_pool_max_num());
		evbuffer_add_printf(returnbuffer, "buff: %u/%u<br><br>\n", buff_manager::get_buff_count(), buff_manager::get_buff_pool_max_num());
		evbuffer_add_printf(returnbuffer, "team: %u/%u<br><br>\n", Team::GetTeamNum(), Team::get_team_pool_max_num());
		evbuffer_add_printf(returnbuffer, "sightspace: %u/%u<br><br>\n", sight_space_manager::get_sight_space_num(), sight_space_manager::get_sight_space_pool_max_num());
		evbuffer_add_printf(returnbuffer, "collect: %lu<br><br>\n", collect_manager_s_collectContain.size());

		evbuffer_add_printf(returnbuffer, "zhenying: %lu/%u<br><br>\n", zhenying_raid_manager_all_raid_id.size(),
			zhenying_raid_manager::get_zhenying_raid_pool_max_num());
		evbuffer_add_printf(returnbuffer, "guild_wait_raid: %lu/%u<br><br>\n", guild_wait_raid_manager_all_raid_id.size(),
			guild_wait_raid_manager::get_guild_wait_raid_pool_max_num());
		evbuffer_add_printf(returnbuffer, "partner: %lu/%u<br><br>\n", partner_manager_all_partner_id.size(), partner_manager_partner_data_pool.num);

		evbuffer_add_printf(returnbuffer, "truck: %lu/%u<br><br>\n", cash_truck_manager_all_id.size(), cash_truck_manager_data_pool.num);		

		evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);					
		evbuffer_free(returnbuffer);			
	}
}
	
void cb_gamesrv_timer()
{
	uint64_t times = time_helper::get_micro_time();
	time_helper::set_cached_time(times / 1000);

//	run_with_period(1)
	{
		player_manager::on_tick_1();
		monster_manager::on_tick_1();
		skill_manager::on_tick_10();	
	}

	run_with_period(5)
	{
		player_manager::on_tick_5();		
		monster_manager::on_tick_5();
	}

	run_with_period(10)
	{
		player_manager::on_tick_10();				
		monster_manager::on_tick_10();
		skill_manager::on_tick_10();
		raid_manager::on_tick_10();
		zhenying_raid_manager::on_tick_10();
		guild_wait_raid_manager::on_tick_10();				
		test_run_timer10();	
		//test
		TeamMatch::Timer();
		guild_battle_manager_on_tick();
		partner_manager::on_tick_5();
		check_server_level();
		monster_manager::add_world_boss_monster();
	}

	run_with_period(30)
	{
		monster_manager::on_tick_30();
		sight_space_manager::on_tick();
		buff_manager::on_tick_30();
	}

	run_with_period(50)
	{
		monster_manager::on_tick_50();
		ChengJieTaskManage::OnTimer();
		Team::Timer();
	}

	run_with_period(100)
	{
		monster_manager::on_tick_100();
//		test_run_timer100();
		Collect::Tick();
		pvp_match_manager_on_tick();
//		guild_battle_manager_on_tick();
		//Team::Timer();
		ZhenyingBattle::GetInstance()->Tick();
	}

	run_with_period(500)
	{
		monster_manager::on_tick_500();
	}

	run_with_period(1000)
	{
		monster_manager::on_tick_1000();
		TeamMatch::Timer();
		//ChengJieTaskManage::SortList();
	}
	/*run_with_period(600)
	{
		monster_manager::add_world_boss_monster();
	}*/
	++timer_loop_count;
}
	
int game_recv_func(evutil_socket_t fd, conn_node_gamesrv *node)
{
	EXTERN_DATA *extern_data;
	PROTO_HEAD *head;
	player_struct *player = NULL;
	for (;;) {
		int ret = node->get_one_buf();
		if (ret == 0) {
			head = (PROTO_HEAD *)node->buf_head();
			int cmd = node->get_cmd();			
			uint64_t times = time_helper::get_micro_time();
			time_helper::set_cached_time(times / 1000);
			switch (cmd)
			{
				case MSG_ID_SAVE_CLIENT_DATA_REQUEST:
				case MSG_ID_LOAD_CLIENT_DATA_REQUEST:
					node->transfer_to_dbsrv();
					break;
				case SERVER_PROTO_GUILD_BATTLE_FINAL_LIST_ANSWER:
				case SERVER_PROTO_RANK_SYNC_CHANGE:
					{
						GameHandleMap::iterator it = m_game_handle_map.find(cmd);
						if (it != m_game_handle_map.end())
						{
							(it->second)(NULL, NULL);
						}
					}
					break;
				default:
					{
						extern_data = node->get_extern_data(head);
						player = player_manager::get_player_by_id(extern_data->player_id);
						//				if (player) {
						//					player->update_rtt(head->time_stamp);
						//				}

						//			if (SERVER_PROTO_ENTER  != cmd && SERVER_PROTO_RELOAD_CONFIG_REQUEST != cmd && SERVER_PROTO_PLAYER_ENTER_REQUEST!=cmd && !player){
						//				LOG_INFO("%s %d: get cmd %d but can not find player, playerid: %lu", __FUNCTION__, __LINE__, cmd, extern_data->player_id);						
						//			} else {
						//				(this->*all_msg_handler[cmd])(player, extern_data);
						GameHandleMap::iterator it = m_game_handle_map.find(cmd);
						if (it != m_game_handle_map.end())
						{
#ifdef FLOW_MONITOR
							uint64_t time = 0;
							time = time_helper::get_micro_time();
#endif							
							(it->second)(player, extern_data);
#ifdef FLOW_MONITOR
							time = time_helper::get_micro_time() - time;
							uint32_t msg_id = ENDION_FUNC_2(cmd);
							add_one_game_srv_msg_time(msg_id, time);
#endif
						}
						else
							default_handle(player, extern_data);
						//			}
					}
					break;
			}
		}
		
		if (ret < 0) {
			LOG_INFO("%s %d: connect closed from fd %u, err = %d", __FUNCTION__, __LINE__, fd, errno);
//			return (-1);

			std::map<uint64_t, player_struct *>::iterator it = player_manager_all_players_id.begin();
			for (; it!=player_manager_all_players_id.end(); ++it) {
				player_struct* player = it->second;
				if (!player)
					continue;

				player->data->status = ONLINE;
		//		player->process_kick_player();
				head = (PROTO_HEAD *)node->buf_head();
	//			extern_data = get_extern_data(head);
				EXTERN_DATA ext_data;
				ext_data.player_id = player->data->player_id;
				player->cache_to_dbserver(false, &ext_data);
			}

			save_server_level_info();
			clear_all_mem();

			event_del(&node->event_recv);
			evutil_closesocket(fd);
#ifdef FLOW_MONITOR
			save_gamesrv_msg_time_to_mysql();
#endif
			exit(0);			
			return (0);
		} else if (ret > 0) {
			break;
		}
		
		ret = node->remove_one_buf();
	}
	return (0);
}

int db_recv_func(evutil_socket_t fd, conn_node_dbsrv *node)
{
	EXTERN_DATA *extern_data;
	PROTO_HEAD *head;
	for (;;) {
		int ret = node->get_one_buf();
		if (ret == 0) {
			head = (PROTO_HEAD *)node->buf_head();
			extern_data = node->get_extern_data(head);
			int cmd = node->get_cmd();
			switch(cmd)
			{
				case MSG_ID_SAVE_CLIENT_DATA_ANSWER:
				case MSG_ID_LOAD_CLIENT_DATA_ANSWER:					
					node->transfer_to_connsrv();
					break;
				default:
					{
						DbHandleMap::iterator it = m_db_handle_map.find(cmd);
						if (it != m_db_handle_map.end())
						{
							(it->second)(extern_data);
						}
						else
						{
							default_handle(NULL, extern_data);
						}
					}
					break;
			}
		}
		
		if (ret < 0) {
			LOG_INFO("%s %d: connect closed from fd %u, err = %d", __FUNCTION__, __LINE__, fd, errno);
			return (-1);		
		} else if (ret > 0) {
			break;
		}
		
		ret = node->remove_one_buf();
	}
	return (0);
}
}


