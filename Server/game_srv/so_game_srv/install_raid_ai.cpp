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
	raid_struct::raid_add_ai_interface(7, &raid_ai_guoyu_interface);
	raid_struct::raid_add_ai_interface(8, &raid_ai_script_interface);
	raid_struct::raid_add_ai_interface(9, &raid_ai_zhenying_interface);

	raid_struct::raid_add_ai_interface(10, &raid_ai_guild_wait_interface);			
	raid_struct::raid_add_ai_interface(11, &raid_ai_guild_interface);
	raid_struct::raid_add_ai_interface(12, &raid_ai_guild_final_interface);	
	raid_struct::raid_add_ai_interface(13, &raid_ai_xunbao_interface);
	raid_struct::raid_add_ai_interface(14, &raid_ai_doufachang_interface);
	raid_struct::raid_add_ai_interface(15, &raid_ai_battle_interface);
}
void uninstall_raid_ai()
{
	raid_struct::raid_add_ai_interface(0, NULL);
	raid_struct::raid_add_ai_interface(3, NULL);
	raid_struct::raid_add_ai_interface(5, NULL);
	raid_struct::raid_add_ai_interface(6, NULL);	
	raid_struct::raid_add_ai_interface(7, NULL);
	raid_struct::raid_add_ai_interface(8, NULL);
	raid_struct::raid_add_ai_interface(9, NULL);				

	raid_struct::raid_add_ai_interface(10, NULL);	
	raid_struct::raid_add_ai_interface(11, NULL);
	raid_struct::raid_add_ai_interface(12, NULL);	
	raid_struct::raid_add_ai_interface(13, NULL);
	raid_struct::raid_add_ai_interface(14, NULL);
	raid_struct::raid_add_ai_interface(15, NULL);
}

