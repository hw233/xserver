#include "chat.h"
#include <math.h>
#include <sstream>
#include "app_data_statis.h"
#include "buff_manager.h"
#include "cash_truck_manager.h"
#include "chat.pb-c.h"
#include "collect.h"
#include "conn_node_dbsrv.h"
#include "guild_battle_manager.h"
#include "guild_land_active_manager.h"
#include "guild_wait_raid_manager.h"
#include "monster_manager.h"
#include "msgid.h"
#include "partner_manager.h"
#include "player.h"
#include "player_manager.h"
#include "pvp_match_manager.h"
#include "raid.h"
#include "raid_manager.h"
#include "server_level.h"
#include "sight_space_manager.h"
#include "time_helper.h"
#include "zhenying_battle.h"
#include "zhenying_raid_manager.h"

chat_mod::chat_mod() {}

chat_mod::~chat_mod() {}

void chat_mod::parse_cmd( char *line, int *argc, char *argv[] )
{
	int n     = 0;
	argv[ 0 ] = line;
	while ( *line && n < MAX_GM_ARGV )
	{
		switch ( *line )
		{
			case ' ':
			case '	':
			case '\n':
				*line = '\0';
				if ( argv[ n ][ 0 ] != '\0' ) ++n;
				++line;
				argv[ n ] = line;
				break;
			default:
				++line;
		}
	}
	*argc = n + 1;
}


static void send_chat_content( player_struct *player, char *content ) { player->send_chat( CHANNEL__area, content ); }

static void send_script_info( player_struct *player, struct raid_script_data *data )
{
	RaidScriptTable *config = NULL;
	if ( data->cur_index < data->script_config->size() )
	{
		config = ( *data->script_config )[ data->cur_index ];
	}

	std::ostringstream os;
	os << "ID:" << ( config ? config->ID : 0 ) << " Type:" << ( config ? config->TypeID : 0 ) << " finish:[";

	for ( int i = 0; i < MAX_SCRIPT_COND_NUM; ++i )
	{
		os << data->cur_finished_num[ i ] << " ";
	}
	os << "]";
	send_chat_content( player, const_cast<char *>( os.str().c_str() ) );
}

static void do_test2_cmd( player_struct *player, uint64_t target_id )
{
	DOUFACHANG_LOAD_PLAYER_REQUEST *req = (DOUFACHANG_LOAD_PLAYER_REQUEST *) conn_node_dbsrv::connecter.get_send_data();
	req->player_id                      = player->get_uuid();
	req->target_id                      = target_id;

	EXTERN_DATA extern_data;
	extern_data.player_id = req->player_id;
	fast_send_msg_base( &conn_node_dbsrv::connecter, &extern_data, SERVER_PROTO_DOUFACHANG_LOAD_PLAYER_REQUEST, sizeof( *req ), 0 );
}

static void do_test_cmd( player_struct *player ) { return; }

extern int send_mail( conn_node_base *connecter, uint64_t player_id, uint32_t type, char *title, char *sender_name, char *content, std::vector<char *> *args, std::map<uint32_t, uint32_t> *attachs, uint32_t statis_id );

int chat_mod::do_one_gm_cmd( player_struct *player, int argc, char *argv[] )
{
	int ret = 1;

	if ( argc < 1 ) return -1;

	uint32_t now = time_helper::get_cached_time() / 1000;
	if ( strcasecmp( argv[ 0 ], "test" ) == 0 )
	{
		do_test_cmd( player );
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "test2" ) == 0 )
	{
		do_test2_cmd( player, strtoul( argv[ 1 ], 0, 0 ) );
		ret = 2;
	}
	else if ( strcasecmp( argv[ 0 ], "check_shangjin" ) == 0 )
	{
		std::map<uint64_t, MoneyQuestTable *>::iterator iter = shangjin_task_config.begin();
		for ( ; iter != shangjin_task_config.end(); ++iter )
		{
			for ( uint32_t i = 0; i < iter->second->n_QuestGroup; ++i )
			{
				TaskTable *main_config = get_config_by_id( iter->second->QuestGroup[ i ], &task_config );
				if ( main_config == NULL )
				{
					LOG_ERR( "[%s:%d] can not get shangjin task table player[%lu], taskid = %lu, table = %lu", __FUNCTION__, __LINE__, player->get_uuid(), iter->second->QuestGroup[ i ], iter->first );
				}
			}
		}
	}
	else if ( strcasecmp( argv[ 0 ], "pass" ) == 0 )
	{
		player->broadcast_one_attr_changed( PLAYER_ATTR_PK_TYPE, 0, false, false );
		player->broadcast_one_attr_changed( PLAYER_ATTR_ZHENYING, 0, false, false );
		player->data->noviceraid_flag = 1;
		player->set_attr( PLAYER_ATTR_PK_TYPE, 0 );
		player->set_attr( PLAYER_ATTR_ZHENYING, 0 );

		raid_struct *raid = player->get_raid();
		if ( raid && raid->data->ID == 20035 )
		{
			raid->clear_monster();
			raid->m_player[ 0 ]->delete_one_buff( 114400018, true );
			raid->player_leave_raid( raid->m_player[ 0 ] );
		}
	}
	else if ( strcasecmp( argv[ 0 ], "pass1" ) == 0 )
	{
		raid_struct *raid = player->get_raid();
		if ( raid ) raid->on_raid_finished();
	}
	else if ( strcasecmp( argv[ 0 ], "restart" ) == 0 )
	{
		char      buff[ 512 ] = "一分钟后重启服务器 。。。";
		ChatHorse send;
		chat_horse__init( &send );
		send.id                   = 0;
		send.prior                = 1;
		send.content              = buff;
		uint32_t c[ MAX_CHANNEL ] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
		send.channel              = c;
		send.n_channel            = 9;
		if ( argc == 2 )
		{
			snprintf( buff, 510, "%s", argv[ 1 ] );
		}
		conn_node_gamesrv::send_to_all_player( MSG_ID_CHAT_HORSE_NOTIFY, &send, (pack_func) chat_horse__pack );
	}
	else if ( strcasecmp( argv[ 0 ], "mon5" ) == 0 )
	{
		// raid_struct *raid = (raid_struct *)player->scene;
		CampDefenseTable *table = get_config_by_id( 360600001, &zhenying_daily_config );
		monster_struct *  pMon  = monster_manager::create_monster_at_pos( player->scene, table->TruckID, 1, table->TruckRouteX[ 0 ], table->TruckRouteY[ 0 ], 0, NULL, 0 );
		pMon->create_config     = get_daily_zhenying_truck_config( 360600001 );
		// pMon->ai_state = AI_WAIT_STATE;
	}
	else if ( strcasecmp( argv[ 0 ], "uid" ) == 0 )
	{
		LOG_ERR( "%s: player uid = %lu 0x%lx", __FUNCTION__, player->get_uuid(), player->get_uuid() );
		return -1;
	}
	else if ( argc >= 3 && strcasecmp( argv[ 0 ], "add" ) == 0 && strcasecmp( argv[ 1 ], "coin" ) == 0 )
	{
		add_coin( player, atoi( argv[ 2 ] ) );
		ret = 3;
	}
	else if ( argc >= 3 && strcasecmp( argv[ 0 ], "add" ) == 0 && strcasecmp( argv[ 1 ], "bind_gold" ) == 0 )
	{
		add_bind_gold( player, atoi( argv[ 2 ] ) );
		ret = 3;
	}
	else if ( argc >= 3 && strcasecmp( argv[ 0 ], "add" ) == 0 && strcasecmp( argv[ 1 ], "gold" ) == 0 )
	{
		add_gold( player, atoi( argv[ 2 ] ) );
		ret = 3;
	}
	else if ( argc >= 4 && strcasecmp( argv[ 0 ], "add" ) == 0 && strcasecmp( argv[ 1 ], "prop" ) == 0 )
	{
		add_prop( player, atoi( argv[ 2 ] ), atoi( argv[ 3 ] ) );
		ret = 4;
	}
	else if ( argc >= 3 && strcasecmp( argv[ 0 ], "add" ) == 0 && strcasecmp( argv[ 1 ], "exp" ) == 0 )
	{
		add_exp( player, atoi( argv[ 2 ] ) );
		ret = 3;
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "addtruck" ) == 0 )
	{
		cash_truck_struct *pTruck = cash_truck_manager::create_cash_truck_at_pos( player->scene, 440100002, *player );
		if ( pTruck != NULL )
		{
			player->data->truck.truck_id  = pTruck->get_uuid();
			player->data->truck.active_id = 440100002;
			player->data->truck.jiefei    = 0;
		}
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "addmonster" ) == 0 )
	{
		gm_add_monster( player, atoi( argv[ 1 ] ) );
		ret = 2;
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "addsight" ) == 0 )
	{
		LOG_DEBUG( "%s %lu addsight %s", __FUNCTION__, player->get_uuid(), argv[ 1 ] );
		gm_add_sight_space_monster( player, atoi( argv[ 1 ] ) );
		ret = 2;
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "delsight" ) == 0 )
	{
		gm_del_sight_space( player );
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "addcollect" ) == 0 )
	{
		gm_add_collect( player, atoi( argv[ 1 ] ) );
		ret = 2;
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "accept_task" ) == 0 )
	{
		gm_accept_task( player, atoi( argv[ 1 ] ) );
		ret = 2;
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "finish_task" ) == 0 )
	{
		TaskInfo *info = player->get_task_info( atoi( argv[ 1 ] ) );
		if ( info )
		{
			info->status = TASK_STATUS__ACHIEVED;
			player->touch_task_event( info->id, TEC_ACHIEVE );
			player->task_update_notify( info );
		}
		ret = 2;
	}
	else if ( argc >= 3 && strcasecmp( argv[ 0 ], "add" ) == 0 && strcasecmp( argv[ 1 ], "equip" ) == 0 )
	{
		gm_add_equip( player, atoi( argv[ 2 ] ) );
		ret = 3;
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "addpet" ) == 0 )
	{
		gm_add_pet_monster( player );
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "oneday" ) == 0 )
	{
		player->refresh_oneday_job();
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "addbuff" ) == 0 )
	{
		gm_add_buff( player, atoi( argv[ 1 ] ) );
		ret = 2;
	}
	else if ( argc >= 3 && strcasecmp( argv[ 0 ], "setattr" ) == 0 )
	{
		gm_set_attr( player, atoi( argv[ 1 ] ), atoi( argv[ 2 ] ) );
		ret = 3;
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "getattr" ) == 0 )
	{
		int  attr = player->get_attr( atoi( argv[ 1 ] ) );
		char tmpbuf[ 128 ];
		sprintf( tmpbuf, "attr %s = %d", argv[ 1 ], attr );
		send_chat_content( player, tmpbuf );
		ret = 2;
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "goto" ) == 0 )
	{
		// EXTERN_DATA extern_data;
		// extern_data.player_id = player->get_uuid();
		// int id = atoi(argv[1]);
		// player->transfer_to_new_scene_by_config(id, &extern_data);
		gm_goto( player, atoi( argv[ 1 ] ) );
		ret = 2;
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "le**e" ) == 0 )
	{
		gm_leave( player );
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "leave" ) == 0 )
	{
		gm_leave( player );
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "go_task" ) == 0 )
	{
		gm_go_task( player, atoi( argv[ 1 ] ) );
		ret = 2;
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "wanyaoka" ) == 0 )
	{
		gm_add_wanyaoka( player, atoi( argv[ 1 ] ) );
		ret = 2;
	}
	else if ( argc >= 3 && strcasecmp( argv[ 0 ], "add" ) == 0 && strcasecmp( argv[ 1 ], "zhenqi" ) == 0 )
	{
		gm_add_zhenqi( player, atoi( argv[ 2 ] ) );
		ret = 3;
	}
	else if ( argc >= 3 && strcasecmp( argv[ 0 ], "add" ) == 0 && strcasecmp( argv[ 1 ], "gather" ) == 0 )
	{
		Collect::CreateCollectByPos( player->scene, 154000022, player->get_pos()->pos_x, atoi( argv[ 2 ] ), player->get_pos()->pos_z, 0, player );
		ret = 3;
		// Collect::CreateCollectByPos(player->scene, 154000023, player->get_pos()->pos_x, atoi(argv[2]), player->get_pos()->pos_z, 0);
	}
	else if ( argc >= 3 && strcasecmp( argv[ 0 ], "sub" ) == 0 && strcasecmp( argv[ 1 ], "exp" ) == 0 )
	{
		gm_sub_exp( player, atoi( argv[ 2 ] ) );
		ret = 3;
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "3v3" ) == 0 )
	{
		gm_enter_3v3( player );
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "ready" ) == 0 )
	{
		gm_pvp_ready( player );
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "pvp_score" ) == 0 )
	{
		player->change_pvp_raid_score( PVP_TYPE_DEFINE_3, atoi( argv[ 1 ] ) );
		ret = 2;
	}
	else if ( argc >= 3 && strcasecmp( argv[ 0 ], "add" ) == 0 && strcasecmp( argv[ 1 ], "guild_popularity" ) == 0 )
	{
		gm_add_guild_popularity( player, atoi( argv[ 2 ] ) );
		ret = 3;
	}
	else if ( argc >= 3 && strcasecmp( argv[ 0 ], "add" ) == 0 && strcasecmp( argv[ 1 ], "guild_treasure" ) == 0 )
	{
		gm_add_guild_treasure( player, atoi( argv[ 2 ] ) );
		ret = 3;
	}
	else if ( argc >= 3 && strcasecmp( argv[ 0 ], "add" ) == 0 && strcasecmp( argv[ 1 ], "guild_build_board" ) == 0 )
	{
		gm_add_guild_build_board( player, atoi( argv[ 2 ] ) );
		ret = 3;
	}
	else if ( argc >= 3 && strcasecmp( argv[ 0 ], "add" ) == 0 && strcasecmp( argv[ 1 ], "guild_donation" ) == 0 )
	{
		gm_add_guild_donation( player, atoi( argv[ 2 ] ) );
		ret = 3;
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "sub_guild_building_time" ) == 0 )
	{
		player->sub_guild_building_time( atoi( argv[ 1 ] ) );
		ret = 2;
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "zhenying" ) == 0 )
	{
		zhenying_raid_manager::add_player_to_zhenying_raid( player );
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "add_guild_raid" ) == 0 )
	{
		add_to_guild_battle_waiting( player );
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "start_guild_raid" ) == 0 )
	{
		start_guild_battle_match();
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "guild_wait" ) == 0 )
	{
		guild_wait_raid_manager::add_player_to_guild_wait_raid( player, false );
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "disband_guild" ) == 0 )
	{
		player->disband_guild( atoi( argv[ 1 ] ) );
		ret = 2;
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "open_guild_battle" ) == 0 )
	{
		start_guild_battle_activity();
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "open_final_guild_battle" ) == 0 )
	{
		start_final_guild_battle_activity();
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "join_final_guild_battle" ) == 0 )
	{
		add_final_guild_id( player->data->guild_id );
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "start_escort" ) == 0 )
	{
		player->start_escort( atoi( argv[ 1 ] ) );
		ret = 2;
	}
	else if ( argc >= 3 && strcasecmp( argv[ 0 ], "stop_escort" ) == 0 )
	{
		player->stop_escort( atoi( argv[ 1 ] ), atoi( argv[ 2 ] ) );
		ret = 3;
	}
	else if ( argc >= 3 && strcasecmp( argv[ 0 ], "blink" ) == 0 )
	{
		gm_blink( player, atof( argv[ 1 ] ), atof( argv[ 2 ] ) );
		ret = 3;
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "add_partner" ) == 0 )
	{
		player->add_partner( atoi( argv[ 1 ] ) );
		ret = 2;
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "anger" ) == 0 )
	{
		player->add_partner_anger( 1000, true );
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "partner_skill" ) == 0 )
	{
		partner_struct *partner  = player->get_battle_partner();
		uint32_t        skill_id = partner->config->Angerskill;
		unit_struct *   target   = partner->ai->choose_target( partner );
		if ( !target )
		{
			return -1;
		}
		partner->attack_target( skill_id, -1, target );
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "show_partner" ) == 0 )
	{
		uint64_t        uuid;
		partner_struct *partner = NULL;
		uint32_t        id      = atoi( argv[ 1 ] );
		for ( PartnerMap::iterator ite = player->m_partners.begin(); ite != player->m_partners.end(); ++ite )
		{
			if ( ite->second->data->partner_id == id && !ite->second->scene )
			{
				partner = ite->second;
				uuid    = partner->get_uuid();
				break;
			}
		}

		if ( !partner )
		{
			player->add_partner( id, &uuid );
			partner = partner_manager::get_partner_by_uuid( uuid );
		}
		if ( !partner ) return -1;

		for ( int i = 0; i < MAX_PARTNER_BATTLE_NUM; ++i )
		{
			if ( player->data->partner_battle[ i ] == 0 )
			{
				player->data->partner_battle[ i ] = uuid;
				break;
			}
		}

		player->add_partner_to_scene( uuid );
		ret = 2;
		// struct position pos;
		// partner->calc_target_pos(&pos);
		// partner->set_pos(pos.pos_x, pos.pos_z);
		// player->scene->add_partner_to_scene(partner);
		// partner->set_timer(time_helper::get_cached_time() + 1000 + random() % 1500);
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "pkmode" ) == 0 )
	{
		ret = 2;
		player->set_attr( PLAYER_ATTR_PK_TYPE, atoi( argv[ 1 ] ) );
		player->set_attr( PLAYER_ATTR_ZHENYING, 1 );
		AttrMap nty_list;
		nty_list[ PLAYER_ATTR_ZHENYING ] = player->get_attr( PLAYER_ATTR_ZHENYING );
		nty_list[ PLAYER_ATTR_PK_TYPE ]  = player->get_attr( PLAYER_ATTR_PK_TYPE );
		player->notify_attr( nty_list, true, true );
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "show_script_raid" ) == 0 )
	{
		raid_struct *raid = player->get_raid();
		if ( raid && raid->m_config->DengeonRank == DUNGEON_TYPE_SCRIPT )
		{
			send_script_info( player, &raid->data->ai_data.script_data.script_data );
			// RaidScriptTable *config = NULL;
			// if (raid->data->ai_data.script_data.script_data.cur_index < raid->data->ai_data.script_data.script_data.script_config->size())
			// {
			//	config = (*raid->data->ai_data.script_data.script_data.script_config)[raid->data->ai_data.script_data.script_data.cur_index];
			// }

			// std::ostringstream os;
			// os << "ID:" << (config ? config->ID : 0) << " Type:" << (config ? config->TypeID : 0) << " finish:[";

			// for (int i = 0; i < MAX_SCRIPT_COND_NUM; ++i)
			// {
			//	os << raid->data->ai_data.script_data.script_data.cur_finished_num[i] << " ";
			// }
			// os << "]";

			// send_chat_content(player, const_cast<char*>(os.str().c_str()));
		}
		else if ( raid && raid->m_config->DengeonRank == DUNGEON_TYPE_RAND_MASTER )
		{
			send_script_info( player, &raid->WANYAOGU_DATA.script_data );
		}
		else if ( raid && raid->m_config->DengeonRank == DUNGEON_TYPE_GUILD_LAND && raid->GUILD_LAND_DATA.script_data.script_config )
		{
			send_script_info( player, &raid->data->ai_data.guild_land_data.script_data );
		}
		else if ( raid && raid->m_config->DengeonRank == DUNGEON_TYPE_MAOGUI_LEYUAN )
		{
			send_script_info( player, &raid->data->ai_data.maogui_data.script_data );
		}
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "set_script_raid" ) == 0 )
	{
		raid_struct *raid = player->get_raid();
		if ( raid && raid->m_config->DengeonRank == DUNGEON_TYPE_SCRIPT )
		{
			raid->data->ai_data.script_data.script_data.cur_index = atoi( argv[ 1 ] );
			for ( int i = 0; i < MAX_SCRIPT_COND_NUM; ++i ) raid->SCRIPT_DATA.script_data.cur_finished_num[ i ] = 0;
		}
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "get_pos" ) == 0 )
	{
		std::ostringstream os;
		os << "scene:" << player->data->scene_id << " pos_x:" << player->get_pos()->pos_x << " pos_z:" << player->get_pos()->pos_z;

		send_chat_content( player, const_cast<char *>( os.str().c_str() ) );
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "kill_partner" ) == 0 )
	{
		ret = 2;
		int index = atoi( argv[ 1 ] );
		if ( index >= 0 && index < MAX_PARTNER_BATTLE_NUM )
		{
			uint64_t        partner_uuid = player->data->partner_battle[ index ];
			partner_struct *partner      = player->get_partner_by_uuid( partner_uuid );
			if ( partner && partner->scene )
			{
				partner->set_attr( PLAYER_ATTR_HP, 0 );
				partner->notify_one_attr_changed( PLAYER_ATTR_HP, 0 );
				partner->on_dead( player );
			}
		}
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "clear_bag" ) == 0 )
	{
		for ( int i = 0; i < MAX_BAG_GRID_NUM; ++i )
		{
			if ( player->data->bag[ i ].id > 0 )
			{
				player->del_item_grid( i, true );
			}
		}
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "break_level" ) == 0 )
	{
		break_server_level();
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "add_title" ) == 0 )
	{
		ret = 2;
		uint32_t keep_time = 0;
		if ( argc >= 3 )
		{
			keep_time = atoi( argv[ 2 ] );
		}
		player->add_title( atoi( argv[ 1 ] ), keep_time );
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "sub_title" ) == 0 )
	{
		ret = 2;
		player->expire_title( atoi( argv[ 1 ] ) );
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "add_murder" ) == 0 )
	{
		ret = 2;
		int num = atoi( argv[ 1 ] );
		if ( num > 0 )
		{
			player->add_murder_num( num );
		}
		else
		{
			player->sub_murder_num( -num );
		}
	}
	else if ( argc >= 3 && strcasecmp( argv[ 0 ], "add_horse" ) == 0 )
	{
		ret = 3;
		player->add_horse( atoi( argv[ 1 ] ), atoi( argv[ 2 ] ) );
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "sub_horse" ) == 0 )
	{
		ret = 2;
		uint32_t horse_id = atoi( argv[ 1 ] );
		for ( uint32_t i = 0; i < player->data->n_horse; ++i )
		{
			if ( player->data->horse[ i ].id == horse_id )
			{
				player->data->horse[ i ].timeout = now;
				break;
			}
		}
	}
	else if ( argc >= 4 && strcasecmp( argv[ 0 ], "add_fashion" ) == 0 )
	{
		ret = 4;		
		player->add_fashion( atoi( argv[ 1 ] ), atoi( argv[ 2 ] ), atoi( argv[ 3 ] ) );
	}
	else if ( strcasecmp( argv[ 0 ], "battle" ) == 0 )
	{
		ZhenyingBattle::GetInstance()->GmStartBattle();
	}
	else if ( strcasecmp( argv[ 0 ], "intobattle" ) == 0 )
	{
		ZhenyingBattle::GetInstance()->IntoBattle( *player );
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "guild_ruqin_open" ) == 0 )
	{
		guild_land_active_manager::guild_ruqin_active_open();
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "guild_ruqin_stop" ) == 0 )
	{
		guild_land_active_manager::guild_ruqin_active_stop();
	}
	else if ( argc >= 2 && strcasecmp( argv[ 0 ], "add_lot" ) == 0 )
	{
		ret = 2;		
		uint32_t      lot_id = atoi( argv[ 1 ] );
		AuctionTable *config = get_config_by_id( lot_id, &auction_config );
		if ( player->data->guild_id > 0 && config )
		{
			TRADE_LOT_INSERT *proto    = (TRADE_LOT_INSERT *) conn_node_gamesrv::get_send_data();
			uint32_t          data_len = sizeof( TRADE_LOT_INSERT );
			memset( proto, 0, data_len );
			proto->lot_id       = lot_id;
			proto->guild_id     = player->data->guild_id;
			proto->masters[ 0 ] = player->data->player_id;
			EXTERN_DATA extern_data;
			fast_send_msg_base( &conn_node_gamesrv::connecter, &extern_data, SERVER_PROTO_TRADE_LOT_INSERT, data_len, 0 );
		}
	}
	else if ( argc >= 1 && strcasecmp( argv[ 0 ], "show_server_time" ) == 0 )
	{
		time_t now_time_r = time_helper::get_cached_time() / 1000;
		tm mow_time_tm;
		localtime_r(&now_time_r, &mow_time_tm);
		char buff[ 501 ];
		snprintf( buff, sizeof(buff), "后台时间:%d-%d-%d-%d-%d-%d", mow_time_tm.tm_year + 1900, mow_time_tm.tm_mon + 1, mow_time_tm.tm_mday, mow_time_tm.tm_hour, mow_time_tm.tm_min, mow_time_tm.tm_sec);
		send_chat_content( player,buff );
		
	}
	else
	{
		return ( -1 );
	}
	return ( ret );
}

int chat_mod::do_gm_cmd( player_struct *player, int argc, char *argv[] )
{
	if ( argc < 1 ) return -1;
	int ret = -1;

	for (;;)
	{
		int t = do_one_gm_cmd(player, argc, argv);
		if (t <= 0)
			break;
		ret = 0;
		argc -= t;
		argv += t;
	}
	return ( ret );
}

void chat_mod::add_coin( player_struct *player, int val )
{
	if ( val > 0 )
	{
		player->add_coin( val, MAGIC_TYPE_GM );
	}
	else
	{
		player->sub_coin( -val, MAGIC_TYPE_GM );
	}
}

void chat_mod::add_bind_gold( player_struct *player, int val ) { player->add_bind_gold( val, MAGIC_TYPE_GM ); }

void chat_mod::add_gold( player_struct *player, int val ) { player->add_unbind_gold( val, MAGIC_TYPE_GM ); }

void chat_mod::add_prop( player_struct *player, int prop_id, int prop_num ) { player->add_item( prop_id, prop_num, MAGIC_TYPE_GM ); }

void chat_mod::add_exp( player_struct *player, int val ) { player->add_exp( val, MAGIC_TYPE_GM ); }

void chat_mod::gm_del_sight_space( player_struct *player )
{
	if ( player->sight_space ) sight_space_manager::del_player_from_sight_space( player->sight_space, player, true );
}

void chat_mod::gm_add_sight_space_monster( player_struct *player, int val )
{
	if ( player->sight_space ) return;
	sight_space_struct *sight = sight_space_manager::create_sight_space( player );
	if ( !sight ) return;

	//	uint32_t monster_id[] = {151000001,	151000033,	151000034,	151000036};
	//	uint32_t monster_id[] = {151005042};
	//	uint32_t monster_id[] = {};

	//	for (size_t i = 0; i < ARRAY_SIZE(monster_id); ++i)
	//	{
	monster_manager::create_sight_space_monster( sight, player->scene, val, 90, player->get_pos()->pos_x, player->get_pos()->pos_z );
	//	if (!monster)
	//	{
	//		LOG_ERR("%s: create monster fail", __FUNCTION__);
	//		return;
	//	}
	//	}
	return;
}

extern void set_leixinye_type( monster_struct *monster, uint32_t type );
void chat_mod::gm_add_19_monster( player_struct *player, int type )
{
	monster_struct *monster = monster_manager::add_monster( 151001007, 1, NULL );
	//	monster_struct *monster = monster_manager::add_monster(151001016, 1, NULL);
	if ( !monster ) return;
	monster->born_pos.pos_x = player->get_pos()->pos_x;
	monster->born_pos.pos_z = player->get_pos()->pos_z;
	monster->set_pos( player->get_pos()->pos_x, player->get_pos()->pos_z );
	//	set_leixinye_type(monster, type);
	if ( player->scene->add_monster_to_scene( monster, 0 ) != 0 )
	{
		LOG_ERR( "%s: uuid[%lu] monster[%u] scene[%u]", __FUNCTION__, monster->data->player_id, 151001016, player->scene->m_id );
	}
}

void chat_mod::gm_add_collect( player_struct *player, int id )
{
	if ( player->sight_space )
	{
		Collect::create_sight_space_collect( player->sight_space, id, player->get_pos()->pos_x, 0, player->get_pos()->pos_z, 0 );
	}
}

void chat_mod::gm_add_monster( player_struct *player, int val )
{
	MonsterTable *monster_table = get_config_by_id( val, &monster_config );
	if ( monster_table == NULL ) return;
	BaseAITable *baseai_table = get_config_by_id( monster_table->BaseID, &base_ai_config );
	if ( baseai_table == NULL ) return;

	if ( baseai_table->AIType == 2 || baseai_table->AIType == 14 || baseai_table->AIType == 22 )
	{
		LOG_ERR( "[%s:%d]gm召唤怪物失败，不能召唤有巡逻路径的怪物,monster_id:%d BaseAI:%lu", __FUNCTION__, __LINE__, val, baseai_table->BaseID );
		return;
	}
	monster_struct *monster = monster_manager::add_monster( val, 1, NULL );
	if ( !monster ) return;
	monster->born_pos.pos_x = player->get_pos()->pos_x;
	monster->born_pos.pos_z = player->get_pos()->pos_z;
	monster->set_pos( player->get_pos()->pos_x, player->get_pos()->pos_z );
	if ( player->scene->add_monster_to_scene( monster, 0 ) != 0 )
	{
		LOG_ERR( "%s: uuid[%lu] monster[%u] scene[%u]", __FUNCTION__, monster->data->player_id, val, player->scene->m_id );
	}
	LOG_INFO( "gm召唤怪,怪物uuid [monster_uuid=%lu]", monster->data->player_id );
}

void chat_mod::gm_accept_task( player_struct *player, int task_id ) { player->accept_task( task_id, false ); }

void chat_mod::gm_add_equip( player_struct *player, int type )
{
	if (type == 0)
	{
		for (int i = ET_WEAPON; i <= ET_NECKLACE; ++i)
		{
			player->add_equip( i, MAGIC_TYPE_GM );
		}
	}
	else
	{
		player->add_equip( type, MAGIC_TYPE_GM );
	}
}

void chat_mod::gm_add_pet_monster( player_struct *player )
{
	monster_struct *monster = monster_manager::add_monster( 151000017, 1, player );
	if ( !monster ) return;
	monster->born_pos.pos_x = player->get_pos()->pos_x;
	monster->born_pos.pos_z = player->get_pos()->pos_z;
	monster->set_pos( player->get_pos()->pos_x, player->get_pos()->pos_z );
	monster->set_ai_interface( 7 );
	if ( player->scene->add_monster_to_scene( monster, 0 ) != 0 )
	{
		LOG_ERR( "%s: uuid[%lu] monster[%u] scene[%u]", __FUNCTION__, monster->data->player_id, 151000017, player->scene->m_id );
	}
}

void chat_mod::gm_add_buff( player_struct *player, int buffid ) { buff_manager::create_default_buff( buffid, player, player, true ); }

void chat_mod::gm_set_attr( player_struct *player, int id, int value )
{
	if ( id < MAX_BUFF_FIGHT_ATTR ) player->data->buff_fight_attr[ id ] = value;

	player->set_attr( id, value );
	player->notify_one_attr_changed( id, value );
}

void chat_mod::gm_add_wanyaoka( player_struct *player, int id )
{
	player->add_wanyaoka( (uint32_t *) &id, 1 );
	// PROTO_HEAD *proto_head;
	// PROTO_HEAD *real_head;
	// proto_head = (PROTO_HEAD *)conn_node_base::global_send_buf;
	// proto_head->msg_id = ENDION_FUNC_2(SERVER_PROTO_GAME_TO_FRIEND);
	// proto_head->seq = 0;

	// real_head = (PROTO_HEAD *)proto_head->data;
	// PROTO_ADD_WANYAOKA *info = (PROTO_ADD_WANYAOKA *)real_head->data;
	// size_t size = sizeof(PROTO_ADD_WANYAOKA);
	// real_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);
	// real_head->msg_id = ENDION_FUNC_2(SERVER_PROTO_ADD_WANYAOKA);

	// memset(info, 0, sizeof(*info));
	// info->player_id = player->get_uuid();
	// info->wanyaoka[0] = id;

	// proto_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + real_head->len);
	// if (conn_node_gamesrv::connecter.send_one_msg(proto_head, 1) != (int)(ENDION_FUNC_4(proto_head->len))) {
	//	LOG_ERR("%s %d: send to friend failed err[%d]", __FUNCTION__, __LINE__, errno);
	// }
	//	conn_node_gamesrv::connecter.send_to_friend(&extern_data, SERVER_PROTO_ADD_WANYAOKA, &info, NULL);
}

void chat_mod::gm_go_task( player_struct *player, int task_id )
{
	if ( !task_is_trunk( task_id ) || player->task_is_finish( task_id ) )
	{
		return;
	}

	TaskInfo *task_info = NULL;
	for ( int i = 0; i < MAX_TASK_ACCEPTED_NUM; ++i )
	{
		uint32_t tmp_id = player->data->task_list[ i ].id;
		if ( tmp_id > 0 && task_is_trunk( tmp_id ) )
		{
			task_info = &player->data->task_list[ i ];
			break;
		}
	}

	task_info->status = TASK_STATUS__FINISH;
	player->task_update_notify( task_info );
	uint32_t cur_task = task_info->id;
	memset( task_info, 0, sizeof( TaskInfo ) );

	while ( cur_task != (uint32_t) task_id )
	{
		if ( player->task_is_finish( cur_task ) )
		{
			break;
		}

		player->add_finish_task( cur_task );
		TaskTable *config = get_config_by_id( cur_task, &task_config );
		if ( !config )
		{
			break;
		}

		cur_task = config->FollowTask;
	}

	player->add_task( task_id, TASK_STATUS__NOT_ACCEPT_YET, true );
}

void chat_mod::gm_add_zhenqi( player_struct *player, int val ) { player->add_zhenqi( val, MAGIC_TYPE_GM ); }

void chat_mod::gm_sub_exp( player_struct *player, int val ) { player->sub_exp( val, MAGIC_TYPE_GM ); }

void chat_mod::gm_enter_3v3( player_struct *player ) { pvp_match_single_ai_player_3( player ); }

void chat_mod::gm_pvp_ready( player_struct *player ) { pvp_match_player_set_ready( player ); }

void chat_mod::gm_add_guild_popularity( player_struct *player, int val ) { player->add_guild_resource( 1, val ); }

void chat_mod::gm_add_guild_treasure( player_struct *player, int val ) { player->add_guild_resource( 2, val ); }

void chat_mod::gm_add_guild_build_board( player_struct *player, int val ) { player->add_guild_resource( 3, val ); }

void chat_mod::gm_add_guild_donation( player_struct *player, int val ) { player->add_guild_resource( 4, val ); }

void chat_mod::gm_blink( player_struct *player, float pos_x, float pos_z )
{
	if ( !player->scene )
	{
		return;
	}

	player->cur_scene_jump( pos_x, pos_z, 0, NULL );
	//	scene_struct *scene = player->scene;
	//	scene->
	//	player->send_clear_sight();
	//	scene->delete_player_from_scene(player);
	//	player->set_pos(pos_x, pos_z);
	//	scene->add_player_to_scene(player);
	//	player->send_scene_transfer(0, pos_x, 0, pos_z, player->data->scene_id, 0);
}

void chat_mod::gm_leave( player_struct *player )
{
	raid_struct *raid = player->get_raid();
	if ( raid && raid->data )
	{
		if ( get_scene_looks_type( raid->m_id ) == SCENE_TYPE_RAID && raid->m_config->DengeonType != 2 && raid->data->state == RAID_STATE_START && player->m_team )
		{
			//组队副本如果没结束，那么踢出队伍
			player->m_team->RemoveMember( *player, false );
		}
		else
		{
			raid->player_leave_raid( player );
		}
	}
}

void chat_mod::gm_goto( player_struct *player, uint32_t scene_id )
{
	EXTERN_DATA extern_data;
	extern_data.player_id = player->get_uuid();
	player->move_to_scene( scene_id, &extern_data );
}
