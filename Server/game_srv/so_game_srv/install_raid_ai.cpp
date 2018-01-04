#include <string.h>
#include "install_raid_ai.h"
#include "raid_ai.h"
#include "raid.h"

void install_raid_ai()
{
	raid_struct::raid_add_ai_interface(0, &raid_ai_normal_interface);
	raid_struct::raid_add_ai_interface(3, &raid_ai_wanyaogu_interface);
	raid_struct::raid_add_ai_interface(5, &raid_ai_pvp3_interface);
	raid_struct::raid_add_ai_interface(6, &raid_ai_pvp3_interface);	
	raid_struct::raid_add_ai_interface(DUNGEON_TYPE_YAOSHI, &raid_ai_guoyu_interface);
	raid_struct::raid_add_ai_interface(8, &raid_ai_script_interface);
	raid_struct::raid_add_ai_interface(DUNGEON_TYPE_ZHENYING, &raid_ai_zhenying_interface);

	raid_struct::raid_add_ai_interface(10, &raid_ai_guild_wait_interface);			
	raid_struct::raid_add_ai_interface(11, &raid_ai_guild_interface);
	raid_struct::raid_add_ai_interface(12, &raid_ai_guild_final_interface);	
	raid_struct::raid_add_ai_interface(13, &raid_ai_xunbao_interface);
	raid_struct::raid_add_ai_interface(14, &raid_ai_doufachang_interface);
	raid_struct::raid_add_ai_interface(DUNGEON_TYPE_BATTLE, &raid_ai_battle_interface);
	raid_struct::raid_add_ai_interface(17, &raid_ai_guild_intrusion_interface);
	raid_struct::raid_add_ai_interface(18, &raid_ai_maogui_interface);
	raid_struct::raid_add_ai_interface(DUNGEON_TYPE_TOWER, &raid_ai_tower_interface);
}
void uninstall_raid_ai()
{
	for (int i = 0; i < MAX_RAID_AI_INTERFACE; ++i) {
		raid_struct::raid_add_ai_interface(i, NULL);
	}
}

