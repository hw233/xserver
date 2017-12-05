#ifndef __CHAT_H__
#define __CHAT_H__

#include <stdint.h>

class player_struct;
#define MAX_GM_ARGV  100

class chat_mod
{
public:
	chat_mod();
	~chat_mod();

	static void parse_cmd(char *line, int *argc, char *argv[]); 
	static int do_gm_cmd(player_struct *player, int argc, char *argv[]);
	static int do_one_gm_cmd( player_struct *player, int argc, char *argv[] );

	static void add_coin(player_struct *player, int val);
	static void add_bind_gold(player_struct *player, int val);
	static void add_gold(player_struct *player, int val);
	static void add_prop(player_struct *player, int prop_id, int prop_num);
	static void add_exp(player_struct *player, int val);
	static void gm_add_monster(player_struct *player, int val);
	static void gm_add_collect(player_struct *player, int id);
	static void gm_add_19_monster(player_struct *player, int type);
	static void gm_add_sight_space_monster(player_struct *player, int val);
	static void gm_del_sight_space(player_struct *player);
	static void gm_accept_task(player_struct *player, int task_id);
	static void gm_enter_raid(player_struct *player, int raid_id);
	static void gm_leave_raid(player_struct *player);
	static void gm_add_equip(player_struct *player, int type);
	static void gm_add_pet_monster(player_struct *player);
	static void gm_add_buff(player_struct *player, int buffid);
	static void gm_set_attr(player_struct *player, int id, int value);
	static void gm_go_task(player_struct *player, int task_id);
	static void gm_add_wanyaoka(player_struct *player, int id);
	static void gm_add_zhenqi(player_struct *player, int val);
	static void gm_sub_exp(player_struct *player, int val);
	static void gm_enter_3v3(player_struct *player);
	static void gm_pvp_ready(player_struct *player);	
	static void gm_add_guild_popularity(player_struct *player, int val);	
	static void gm_add_guild_treasure(player_struct *player, int val);	
	static void gm_add_guild_build_board(player_struct *player, int val);	
	static void gm_add_guild_donation(player_struct *player, int val);	
	static void gm_blink(player_struct *player, float pos_x, float pos_z);
	static void gm_goto(player_struct *player, uint32_t scene_id);
	static void gm_leave(player_struct *player);
};

#endif
