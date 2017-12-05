#include "guild_land_raid.h"

guild_land_raid_struct::guild_land_raid_struct()
{
}

guild_land_raid_struct::~guild_land_raid_struct()
{
}
int guild_land_raid_struct::init_special_raid_data(player_struct *player)
{
	raid_set_ai_interface(DUNGEON_TYPE_GUILD_LAND);
	init_scene_struct(m_id, true, 0);
	return (0);
}
int guild_land_raid_struct::init_guild_ruqin_active_data()
{
	
	ruqin_data.guild_ruqin = false;
	ruqin_data.zhengying = 0;
	ruqin_data.level = 0;
	ruqin_data.open_time = 0;
	ruqin_data.all_boshu = 0;
	ruqin_data.monster_boshu = 0;
	ruqin_data.monster_id = sg_guild_ruqin_huodui_monster_id;
	ruqin_data.space_time = 0; 
	ruqin_data.pos_x = 0;
	ruqin_data.pos_z = 0;
	ruqin_data.juli = sg_guild_ruqin_huodui_fanwei;
	ruqin_data.huodui_time = 0;
	ruqin_data.exp = sg_guild_ruqin_huodui_exp;
	ruqin_data.status = GUILD_RUQIN_ACTIVE_INIT;
	ruqin_data.player_data.clear();

	clear_monster();
	return 0;
}
void guild_land_raid_struct::clear_monster()
{
	raid_struct::clear_monster();
	for(std::map<uint64_t, player_struct *>::iterator itr =  m_players.begin(); itr != m_players.end(); itr++)
	{
		itr->second->send_clear_sight_monster();			
		itr->second->data->cur_sight_monster = 0;
	}
}


