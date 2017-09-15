#ifndef RAID_H
#define RAID_H
#include <list>
#include "scene.h"
#include "player.h"
#include "monster.h"
#include "team.h"

struct raid_player_info
{
	uint64_t player_id;
	char name[MAX_PLAYER_NAME_LEN + 1];    //名字
	uint32_t headicon;    //头像ID
	uint16_t lv;
	uint8_t job;
	uint32_t damage;
	uint32_t injured;
	uint32_t cure;
	uint32_t dead_count;
};

/* "0=普通副本 */
/* 3=随机主副本 */
/* 4=随机小关卡 */
/* 5=3V3 PVP副本 */
/* 6=5V5 PVP副本 */
/* 7=妖师客栈 */
/* 8=走AI流程副本 */
/* 9=战场副本 */
/* 10=公会准备区副本 */
/* 11=预赛 */
/* 12=决赛" */
/* 15=普通阵营战 */
/* 16=新手阵营战 */
enum DUNGEON_TYPE_DEFINE
{
	DUNGEON_TYPE_NORMAL = 0,
	DUNGEON_TYPE_RAND_MASTER = 3,
	DUNGEON_TYPE_RAND_SLAVE = 4,
	DUNGEON_TYPE_PVP_3 = 5,
	DUNGEON_TYPE_PVP_5 = 6,
	DUNGEON_TYPE_YAOSHI = 7,
	DUNGEON_TYPE_SCRIPT = 8,
	DUNGEON_TYPE_ZHENYING = 9,
	DUNGEON_TYPE_GUILD_WAIT = 10,
	DUNGEON_TYPE_GUILD_RAID = 11,
	DUNGEON_TYPE_GUILD_FINAL_RAID = 12,	
	DUNGEON_TYPE_BATTLE = 15,
	DUNGEON_TYPE_BATTLE_NEW = 16,
};

enum RAID_STATE_DEFINE
{
	RAID_STATE_START,  //副本开始
	RAID_STATE_PASS,  //副本胜利
//	RAID_STATE_FAIL,  //副本失败
};

#define MAX_ITEM_REWARD_PER_RAID 20
#define MAX_WANYAOGU_RAID_NUM 3
#define MAX_SCRIPT_COND_NUM 5
#define WANYAOGU_DATA data->ai_data.wanyaogu_data
#define PVP_DATA data->ai_data.pvp_data
#define DOUFACHANG_DATA data->ai_data.doufachang_data
#define SCRIPT_DATA data->ai_data.script_data
#define ZHENYING_DATA data->ai_data.zhenying_data
#define GUILD_DATA data->ai_data.guild_data
#define GUILD_FINAL_DATA data->ai_data.guild_final_data
#define GUILD_WAIT_DATA data->ai_data.guild_wait_data

enum WANYAOGU_STATE
{
	WANYAOGU_STATE_INIT,  //还没开始
	WANYAOGU_STATE_WAIT_START,  //开始10秒倒计时
	WANYAOGU_STATE_START,    //关卡开始
	WANYAOGU_STATE_BBQ,      //关卡结束，挂机得经验
	WANYAOGU_STATE_FINISH,   //所有关卡结束
};

enum PVP_RAID_STATE
{
	PVP_RAID_STATE_INIT,  //还没开始
	PVP_RAID_STATE_WAIT_START,  //开始10秒倒计时
	PVP_RAID_STATE_START,    //关卡开始
	PVP_RAID_STATE_FINISH,   //所有关卡结束
};

struct assist_data
{
	uint64_t player_id;   //造成伤害的玩家ID
	uint32_t damage_time;  //造成伤害的时间
};

struct pvp_player_praise_record
{
	bool praise[MAX_TEAM_MEM * 2];  //有没有点赞
};

#define MAX_RAID_REGION 20
struct raid_script_data
{
		//脚本配置部分
	std::vector<struct RaidScriptTable *> *script_config;
	uint16_t cur_index;
	uint32_t cur_finished_num[MAX_SCRIPT_COND_NUM];
	uint32_t collect_callback_event;   //采集回调的操作 1: 打断雷鸣鼓
	uint32_t dead_monster_id;  //死亡的怪物在判断怪物血量的时候也要算进去
	struct RaidScriptTable *region_config[MAX_RAID_REGION];
	uint8_t cur_region_config;
};

union raid_ai_data
{
	struct 
	{
//		int star;
		WANYAOGU_STATE wanyaogu_state;
		uint64_t timer1;   //计时器1
		uint64_t timer2;   //计时器2		
		struct DungeonTable* m_config;		//当前关卡的配置
//		struct ControlTable *m_control_config;		//当前关卡的配置
		uint32_t wanyaogu_raid_id[MAX_WANYAOGU_RAID_NUM];  //随机的关卡ID
		uint32_t raid_pass[MAX_WANYAOGU_RAID_NUM];  //是否通关
		uint32_t wanyaoka_id[MAX_WANYAOKA_EACH_TIME];  //获得的万妖卡
		uint32_t wanyaoka_cond_param[MAX_WANYAOKA_COND_PARAM];  //当前关卡的条件参数完成情况
		struct raid_script_data script_data;
	} wanyaogu_data;

	struct
	{
		PVP_RAID_STATE pvp_raid_state;		
		uint32_t refresh_monster_time;
		uint32_t red_buff_relive_time;
		uint32_t blue_buff_relive_time;
		uint8_t kill_record[MAX_TEAM_MEM * 2];  //击杀记录
		uint8_t dead_record[MAX_TEAM_MEM * 2];  //死亡次数记录
		uint8_t assist_record[MAX_TEAM_MEM * 2];   //助攻次数记录
		struct assist_data assist_data[MAX_TEAM_MEM * 2][MAX_TEAM_MEM]; //伤害记录，用来计算助攻
		struct pvp_player_praise_record praise_index[MAX_TEAM_MEM * 2];  //对应的玩家下标
		bool pvp_raid_ready[MAX_TEAM_MEM * 2];  //是否已经进入游戏了
	} pvp_data;

	struct
	{
		PVP_RAID_STATE pvp_raid_state;
	} doufachang_data;

	struct
	{
		uint8_t kill_record[MAX_TEAM_MEM * 2];  //击杀记录		
//		struct assist_data assist_data[MAX_TEAM_MEM * 2][MAX_TEAM_MEM]; //伤害记录，用来计算助攻
		uint32_t guild_id[2]; //对应的工会ID
	} guild_data;

	struct
	{
		bool air_wall_close[4];  //4个空气墙是否关闭
		uint8_t kill_record[MAX_TEAM_MEM * 4];  //击杀玩家记录
		uint8_t monster_record[MAX_TEAM_MEM * 4];  //击杀小怪记录
		uint32_t boss_record[MAX_TEAM_MEM * 4];  //BOSS伤害纪录
		uint32_t guild_id[4]; //对应的工会ID
		uint32_t boss_maxhp; //boss的最大血量
		uint64_t boss_killer; //击杀boss的玩家
	} guild_final_data;
	
	struct
	{
		struct raid_script_data script_data;		
//		std::vector<struct RaidScriptTable *> *script_config;
//		uint16_t cur_index;
//		uint32_t cur_finished_num[MAX_SCRIPT_COND_NUM];
	} script_data;

	struct
	{
//		uint16_t cur_player_num;  //当前人数
		int m_line;   //第几条线		
	} zhenying_data;
	struct
	{
		uint32_t step;   //
		uint32_t room;   //
	} battle_data;

	struct
	{
		uint32_t guild_id; //对应的工会ID
	} guild_wait_data;

	struct
	{
		bool note_boss;   //
		uint32_t target;
	}guoyu_data;
};

struct raid_data
{
	uint64_t uuid;
	uint32_t ID;
	uint64_t start_time;  //副本开始时间
	uint64_t delete_time;  //副本没人了，等待销毁的时间
	RAID_STATE_DEFINE state;
	uint32_t dead_count;   //死亡次数		

	struct raid_player_info player_info[MAX_TEAM_MEM];
	struct raid_player_info player_info2[MAX_TEAM_MEM];
	struct raid_player_info player_info3[MAX_TEAM_MEM];
	struct raid_player_info player_info4[MAX_TEAM_MEM];		

	uint64_t team_id;
	uint64_t team2_id;
	uint64_t team3_id;
	uint64_t team4_id;	
	bool delete_team1;   //副本结束后是否解散队伍1
	bool delete_team2;	 //副本结束后是否解散队伍2
	bool delete_team3;	 //副本结束后是否解散队伍2
	bool delete_team4;	 //副本结束后是否解散队伍2

	uint8_t star_bits;   //当前副本星级 按比特位计算
	uint8_t star_param[3];   //计算副本星级需要的参数
	
	uint32_t pass_index;  //通关类型下标
	uint32_t pass_value;  //通关参数对应完成的数值
	int ai_type;
	uint64_t raid_ai_event; //有些副本ai事件需要客户端执行完毕后通知后台继续副本ai，次参数用来记录客户端发回的副本事件id
	union raid_ai_data ai_data;
};

class raid_struct;

typedef void(*raid_ai_tick)(raid_struct *);
typedef void(*raid_ai_init)(raid_struct *, player_struct *);
typedef void(*raid_ai_finished)(raid_struct *);
typedef void(*raid_ai_failed)(raid_struct *);
typedef void(*raid_ai_player_enter)(raid_struct *, player_struct *);
typedef void(*raid_ai_player_leave)(raid_struct *, player_struct *);
typedef void(*raid_ai_player_dead)(raid_struct *, player_struct *, unit_struct *);
typedef void(*raid_ai_player_relive)(raid_struct *, player_struct *, uint32_t);
typedef void(*raid_ai_monster_dead)(raid_struct *, monster_struct *, unit_struct *);
typedef void(*raid_ai_collect)(raid_struct *, player_struct *, Collect *);
typedef void(*raid_ai_attack)(raid_struct *, player_struct *, unit_struct *, int);
typedef void(*raid_ai_player_region_changed)(raid_struct *, player_struct *, uint32_t, uint32_t);
typedef void(*raid_ai_monster_region_changed)(raid_struct *, monster_struct *, uint32_t, uint32_t);
typedef void(*raid_ai_escort_stop)(raid_struct *, player_struct *, uint32_t, bool);
typedef void(*raid_ai_npc_talk)(raid_struct *, player_struct *, uint32_t);
typedef struct DungeonTable* (*raid_ai_get_config)(raid_struct *);

struct raid_ai_interface
{
	raid_ai_init raid_on_init; //初始化
	raid_ai_tick raid_on_tick; //定时驱动
	raid_ai_player_enter raid_on_player_enter;  //玩家进入
	raid_ai_player_leave raid_on_player_leave;  //玩家离开
	raid_ai_player_dead raid_on_player_dead;  //玩家死亡
	raid_ai_player_relive raid_on_player_relive;  //玩家复活
	raid_ai_monster_dead raid_on_monster_dead; //怪物死亡
	raid_ai_collect raid_on_raid_collect; //采集
	raid_ai_player_enter raid_on_player_ready;  //玩家进入场景(客户端loading完)
	raid_ai_finished raid_on_finished; //完成
	raid_ai_attack raid_on_player_attack; //玩家攻击
	raid_ai_player_region_changed raid_on_player_region_changed; //区域变化
	raid_ai_escort_stop raid_on_escort_stop; //护送结果
	raid_ai_npc_talk raid_on_npc_talk; //和npc对话
	raid_ai_get_config raid_get_config; //获取配置，主要是万妖谷的配置
	raid_ai_failed raid_on_failed; //失败
	raid_ai_monster_region_changed raid_on_monster_region_changed; //区域变化	
};

class raid_struct : public scene_struct
{
public:
	virtual void clear();
	virtual int init_special_raid_data(player_struct *player);
	virtual bool check_raid_need_delete();
	virtual	int player_offline(player_struct *player);
	virtual uint32_t get_area_width();

	virtual int init_raid(player_struct *player);
	void raid_set_ai_interface(int ai_type);
	static void raid_add_ai_interface(int ai_type, struct raid_ai_interface *ai);
	int team_enter_raid(Team *team);
	void team_destoryed(Team *team);
	/* int team2_enter_raid(Team *team); */
	/* int team3_enter_raid(Team *team); */
	/* int team4_enter_raid(Team *team);		 */
	int player_return_raid(player_struct *player);
	int player_enter_raid(player_struct *player, double pos_x, double pos_z, double direct = 0);
	int player_enter_raid_impl(player_struct *player, int index, double pos_x, double pos_z, double direct = 0);
	int player_leave_raid(player_struct *player);
	bool is_monster_alive(uint32_t id);	
	int get_id_monster_num(uint32_t id);	
	int get_id_collect_num(uint32_t id);
	int check_all_monster_region_buff(struct RaidScriptTable *config);
	int add_monster_to_scene(monster_struct *monster, uint32_t effectid);
//	int add_player_to_scene(player_struct *player);
	int add_collect_to_scene(Collect *pCollect);
	int delete_monster_from_scene(monster_struct *monster, bool send_msg);	
//	int delete_player_from_scene(player_struct *player);
//	int delete_collect_to_scene(Collect *pCollect);

	void on_player_enter_raid(player_struct *player);
	void on_player_leave_raid(player_struct *player);	
	int on_raid_finished();
	int on_raid_failed(uint32_t score_param);
	bool check_can_add_team_mem(player_struct *player);  //队伍能否在该副本中途加入队员
	SCENE_TYPE_DEFINE get_scene_type();
	void on_tick();
	void on_player_attack(player_struct *player, unit_struct *target, int damage);
	void on_monster_attack(monster_struct *monster, player_struct *player, int damage);
	void on_monster_dead(monster_struct *monster, unit_struct *killer);
	void on_player_dead(player_struct *player, unit_struct *killer);
	void on_collect(player_struct *player, Collect *collect);
	void send_raid_pass_param(player_struct *player);
	bool need_show_star();
	void send_star_changed_notify(uint32_t star_param[3], uint32_t score_param[3]);	
	bool add_raid_pass_value(uint32_t pass_type, struct DungeonTable* config);	  //true表示副本结束
	bool check_raid_failed();  //副本是否已经失败
	int check_cond_finished(int index, uint64_t cond_type, uint64_t cond_value, uint64_t cond_value1, uint32_t *ret_param);  //判断指定的完成条件是否达成
	int calc_raid_star(uint32_t star_param[3], uint32_t score_param[3]);
	virtual int broadcast_to_raid(uint32_t msg_id, void *msg_data, pack_func func);
	int init_common_script_data(const char *script_name, struct raid_script_data *script_data);	
	struct DungeonTable *get_raid_config();	
	void stop_monster_ai();
	void start_monster_ai();
	void stop_player_ai();
	void start_player_ai();		
	void clear_monster();
	int set_player_info(player_struct *player, struct raid_player_info *info);	
	int get_monster_num() { return m_monster.size(); }
	uint16_t get_cur_player_num() {return player_num;};
//	int get_raid_player_pos(uint64_t player_id);
	struct raid_player_info *get_raid_player_info(uint64_t player_id, int *pos);
	bool is_guild_battle_raid(); //是否帮战的副本
	bool is_guild_battle_fight_raid(); //是否帮战的战斗副本
	monster_struct *get_first_boss();
	
	struct raid_ai_interface *ai;
	struct raid_data *data;
	player_struct *m_player[MAX_TEAM_MEM];   //在线玩家，但是有可能还在loading
	player_struct *m_player2[MAX_TEAM_MEM];
	player_struct *m_player3[MAX_TEAM_MEM];
	player_struct *m_player4[MAX_TEAM_MEM];		
	Team *m_raid_team;
	Team *m_raid_team2;
	Team *m_raid_team3;
	Team *m_raid_team4;		
	struct DungeonTable* m_config;
	struct ControlTable *m_control_config;
	std::set<monster_struct *> m_monster;
	int mark_finished;   //副本是否结束了, 0表示没结束，1表示失败了，其他表示通过结束了
	
protected:
	uint16_t player_num;  //记录玩家数目，没有玩家了才可以删除
	void delete_raid_collect_safe(uint32_t uuid);
	int init_script_data();
	int	init_wanyaogu_data();
	int	init_pvp_raid_data_3();
	int	init_pvp_raid_data_5();
	int init_guild_raid_data();
	int init_guild_final_raid_data();
	int	init_guoyu_raid_data(player_struct *player);
	virtual int set_m_player_and_player_info(player_struct *player, int index);
	virtual int clear_m_player_and_player_info(player_struct *player, bool clear_player_info);	
	int get_free_player_pos();
	void broadcast_player_hit_statis_changed(struct raid_player_info *info, player_struct *player);
};

#endif /* RAID_H */
