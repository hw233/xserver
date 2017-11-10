#include "friend_util.h"
#include <stdio.h>
#include "conn_node_friendsrv.h"
#include <list>
#include <set>
#include "friend_config.h"
#include "friend.pb-c.h"
#include "msgid.h"
#include "error_code.h"
#include "chat.pb-c.h"
#include "time_helper.h"
#include "redis_util.h"

char sg_friend_key[64];
char sg_friend_chat_key[64];
char sg_player_cache_key[64]; //玩家离线缓存
char *sg_player_key = conn_node_friendsrv::server_key;

static std::list<FriendPlayer *> friend_player_pool;
static const uint32_t daily_reset_clock = 5 * 3600; //每天五点刷新
static std::map<uint64_t, std::set<uint64_t> > contact_me_map; //加我为好友的玩家
static std::map<uint64_t, std::set<uint64_t> > watch_me_map; //关注我的玩家

AutoReleaseBatchFriendPlayer::AutoReleaseBatchFriendPlayer()
{
}

AutoReleaseBatchFriendPlayer::~AutoReleaseBatchFriendPlayer()
{
	for (std::vector<FriendPlayer*>::iterator iter = pointer_vec.begin(); iter != pointer_vec.end(); ++iter)
	{
		FriendPlayer *player = *iter;
		if (!player)
		{
			continue;
		}

		friend_player_pool.push_back(player);
	}
}

void AutoReleaseBatchFriendPlayer::push_back(FriendPlayer *player)
{
	pointer_vec.push_back(player);
}

void init_redis_keys(uint32_t server_id)
{
	sprintf(conn_node_friendsrv::server_key, "server_%u", server_id);
	sprintf(conn_node_friendsrv::server_wyk_key, "server_wyk_%u", server_id);	
	sprintf(sg_player_cache_key, "s%u_player_cache", server_id);
	sprintf(sg_friend_key, "s%u_friend", server_id);
	sprintf(sg_friend_chat_key, "s%u_friend_chat", server_id);
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

bool player_is_exist(uint64_t player_id)
{
	int ret = sg_redis_client.exist(sg_player_key, player_id);
	return (ret == 1);
}

static int pack_friend_player(FriendPlayer *player, uint8_t *out_data)
{
	RedisFriendPlayer db_info;
	RedisFriendPlayer *db_player = &db_info;
	redis_friend_player__init(db_player);

	RedisFriendUnit contact_data[MAX_FRIEND_CONTACT_NUM];
	RedisFriendUnit *contact_point[MAX_FRIEND_CONTACT_NUM];
	RedisFriendUnit block_data[MAX_FRIEND_BLOCK_NUM];
	RedisFriendUnit *block_point[MAX_FRIEND_BLOCK_NUM];
	RedisFriendGroup group_data[MAX_FRIEND_GROUP_NUM];
	RedisFriendGroup *group_point[MAX_FRIEND_GROUP_NUM];

	db_player->player_id = player->player_id;
	db_player->apply_switch = player->apply_switch;
	db_player->contact_extend = player->contact_extend;
	db_player->n_recents = 0;
	db_player->recents = &player->recents[0];
	for (int i = 0; i < MAX_FRIEND_RECENT_NUM; ++i)
	{
		if (player->recents[i] == 0)
		{
			break;
		}
		db_player->n_recents++;
	}
	db_player->n_contacts = 0;
	db_player->contacts = contact_point;
	for (int i = 0; i < MAX_FRIEND_CONTACT_NUM; ++i)
	{
		if (player->contacts[i].player_id == 0)
		{
			break;
		}
		
		contact_point[db_player->n_contacts] = &contact_data[db_player->n_contacts];
		redis_friend_unit__init(contact_point[db_player->n_contacts]);
		contact_data[db_player->n_contacts].player_id = player->contacts[i].player_id;
		contact_data[db_player->n_contacts].closeness = player->contacts[i].closeness;
		contact_data[db_player->n_contacts].gift_num = player->contacts[i].gift_num;
		contact_data[db_player->n_contacts].group_id = player->contacts[i].group_id;
		db_player->n_contacts++;
	}
	db_player->n_blocks = 0;
	db_player->blocks = block_point;
	for (int i = 0; i < MAX_FRIEND_BLOCK_NUM; ++i)
	{
		if (player->blocks[i].player_id == 0)
		{
			break;
		}
		
		block_point[db_player->n_blocks] = &block_data[db_player->n_blocks];
		redis_friend_unit__init(block_point[db_player->n_blocks]);
		block_data[db_player->n_blocks].player_id = player->blocks[i].player_id;
		block_data[db_player->n_blocks].closeness = player->blocks[i].closeness;
		block_data[db_player->n_blocks].gift_num = player->blocks[i].gift_num;
		block_data[db_player->n_blocks].group_id = player->blocks[i].group_id;
		db_player->n_blocks++;
	}
	db_player->n_enemies = 0;
	db_player->enemies = &player->enemies[0];
	for (int i = 0; i < MAX_FRIEND_ENEMY_NUM; ++i)
	{
		if (player->enemies[i] == 0)
		{
			break;
		}
		db_player->n_enemies++;
	}
	db_player->n_applys= 0;
	db_player->applys = &player->applys[0];
	for (int i = 0; i < MAX_FRIEND_APPLY_NUM; ++i)
	{
		if (player->applys[i] == 0)
		{
			break;
		}
		db_player->n_applys++;
	}
	db_player->n_groups = 0;
	db_player->groups = group_point;
	for (int i = 0; i < MAX_FRIEND_GROUP_NUM; ++i)
	{
		if (player->groups[i].group_id == 0)
		{
			break;
		}

		group_point[db_player->n_groups] = &group_data[db_player->n_groups];
		redis_friend_group__init(group_point[db_player->n_groups]);
		group_data[db_player->n_groups].group_id = player->groups[i].group_id;
		group_data[db_player->n_groups].create_time = player->groups[i].create_time;
		group_data[db_player->n_groups].group_name = player->groups[i].group_name;
		db_player->n_groups++;
	}
	db_player->gift_accept = player->gift_accept;
	db_player->reset_time = player->reset_time;

	return redis_friend_player__pack(db_player, out_data);
}

int save_friend_player(FriendPlayer *player)
{
	CRedisClient &rc = sg_redis_client;
	static uint8_t data_buffer[32 * 1024];
	int data_len = 0;
	char field[64];
	sprintf(field, "%lu", player->player_id);
	data_len = pack_friend_player(player, data_buffer);
	if (data_len <= 0)
	{
		LOG_ERR("[%s:%d] player[%lu] pack friend failed, ret:%u", __FUNCTION__, __LINE__, player->player_id, data_len);
		return -1;
	}

	int ret = rc.hset_bin(sg_friend_key, field, (char *)data_buffer, data_len);
	if (ret < 0)
	{
		LOG_ERR("[%s:%d] player[%lu] save redis failed, ret:%d", __FUNCTION__, __LINE__, player->player_id, ret);
		return -1;
	}

	return 0;
}

int unpack_friend_player(uint8_t *data, int data_len, FriendPlayer *player)
{
	RedisFriendPlayer *redis_friend = redis_friend_player__unpack(NULL, data_len, data);
	if (!redis_friend)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack friend failed, ret:%u", __FUNCTION__, __LINE__, player->player_id, data_len);
		return -1;
	}

	player->player_id = redis_friend->player_id;
	player->apply_switch = redis_friend->apply_switch;
	player->contact_extend = redis_friend->contact_extend;
	for (size_t i = 0; i < redis_friend->n_recents && i < MAX_FRIEND_RECENT_NUM; ++i)
	{
		player->recents[i] = redis_friend->recents[i];
	}
	for (size_t i = 0; i < redis_friend->n_contacts && i < MAX_FRIEND_CONTACT_NUM; ++i)
	{
		player->contacts[i].player_id = redis_friend->contacts[i]->player_id;
		player->contacts[i].closeness = redis_friend->contacts[i]->closeness;
		player->contacts[i].gift_num = redis_friend->contacts[i]->gift_num;
		player->contacts[i].group_id = redis_friend->contacts[i]->group_id;
	}
	for (size_t i = 0; i < redis_friend->n_blocks && i < MAX_FRIEND_BLOCK_NUM; ++i)
	{
		player->blocks[i].player_id = redis_friend->blocks[i]->player_id;
		player->blocks[i].closeness = redis_friend->blocks[i]->closeness;
		player->blocks[i].gift_num = redis_friend->blocks[i]->gift_num;
		player->blocks[i].group_id = redis_friend->blocks[i]->group_id;
	}
	for (size_t i = 0; i < redis_friend->n_enemies && i < MAX_FRIEND_ENEMY_NUM; ++i)
	{
		player->enemies[i] = redis_friend->enemies[i];
	}
	for (size_t i = 0; i < redis_friend->n_applys && i < MAX_FRIEND_APPLY_NUM; ++i)
	{
		player->applys[i] = redis_friend->applys[i];
	}
	for (size_t i = 0; i < redis_friend->n_groups && i < MAX_FRIEND_GROUP_NUM; ++i)
	{
		player->groups[i].group_id = redis_friend->groups[i]->group_id;
		player->groups[i].create_time = redis_friend->groups[i]->create_time;
		strcpy(player->groups[i].group_name, redis_friend->groups[i]->group_name);
	}
	player->gift_accept = redis_friend->gift_accept;
	player->reset_time = redis_friend->reset_time;

	redis_friend_player__free_unpacked(redis_friend, NULL);

	return 0;
}

int load_friend_player(uint64_t player_id, FriendPlayer *player)
{
	CRedisClient &rc = sg_redis_client;
	static uint8_t data_buffer[32 * 1024];
	int data_len = 32 * 1024;
	char field[64];
	sprintf(field, "%lu", player_id);
	int ret = rc.hget_bin(sg_friend_key, field, (char *)data_buffer, &data_len);
	if (ret != 0)
	{
		return -1;
	}

	ret = unpack_friend_player(data_buffer, data_len, player);
	if (ret != 0)
	{
		LOG_ERR("[%s:%d] player[%lu] pack friend failed, ret:%d", __FUNCTION__, __LINE__, player_id, ret);
		return -1;
	}

	return 0;
}

FriendPlayer *get_friend_player(uint64_t player_id)
{
	FriendPlayer *player = NULL;
	if (friend_player_pool.size() > 0)
	{
		player = friend_player_pool.front();
		friend_player_pool.pop_front();
	}
	else
	{
		player = (FriendPlayer *)malloc(sizeof(FriendPlayer));
		if (!player)
		{
			return NULL;
		}
	}

	memset(player, 0, sizeof(FriendPlayer));
	int ret = load_friend_player(player_id, player);
	if (ret != 0)
	{
		memset(player, 0, sizeof(FriendPlayer));
		player->player_id = player_id;
	}

	return player;
}

int save_friend_chat(uint64_t player_id, ChatList *chats)
{
	CRedisClient &rc = sg_redis_client;
	static uint8_t data_buffer[32 * 1024];
	int data_len = 0;
	char field[64];
	sprintf(field, "%lu", player_id);
	data_len = chat_list__pack(chats, data_buffer);
	if (data_len <= 0)
	{
		LOG_ERR("[%s:%d] player[%lu] pack chat failed, ret:%u", __FUNCTION__, __LINE__, player_id, data_len);
		return -1;
	}

	int ret = rc.hset_bin(sg_friend_chat_key, field, (char *)data_buffer, data_len);
	if (ret != 0)
	{
		LOG_ERR("[%s:%d] player[%lu] save redis failed, ret:%d", __FUNCTION__, __LINE__, player_id, ret);
		return -1;
	}

	return 0;
}

ChatList *load_friend_chat(uint64_t player_id)
{
	CRedisClient &rc = sg_redis_client;
	static uint8_t data_buffer[32 * 1024];
	int data_len = 32 * 1024;
	char field[64];
	sprintf(field, "%lu", player_id);
	int ret = rc.hget_bin(sg_friend_chat_key, field, (char *)data_buffer, &data_len);
	if (ret == 0)
	{
		return chat_list__unpack(NULL, data_len, data_buffer);
	}

	return NULL;
}

void delete_friend_chat(uint64_t player_id)
{
	CRedisClient &rc = sg_redis_client;
	char field[64];
	sprintf(field, "%lu", player_id);
	rc.hdel(sg_friend_chat_key, field);
}

void add_friend_offline_chat(uint64_t player_id, Chat *chat)
{
	ChatList *list = load_friend_chat(player_id);
	if (!list)
	{
		list = (ChatList *)malloc(sizeof(ChatList));
		chat_list__init(list);
	}

	if (list->n_chats >= (size_t)MAX_FRIEND_CHAT_NUM)
	{
		free(list->chats[0]);
		list->chats[0] = NULL;
		memmove(&list->chats[0], &list->chats[1], (list->n_chats - 1) * sizeof(Chat*));
	}
	else
	{
		list->n_chats++;
		list->chats = (Chat**)realloc(list->chats, list->n_chats * sizeof(Chat*));
	}
	list->chats[list->n_chats - 1] = chat;
	save_friend_chat(player_id, list);

	list->chats[list->n_chats - 1] = NULL;
	list->n_chats--;
	chat_list__free_unpacked(list, NULL);
}

void add_friend_offline_system(uint64_t player_id, SystemNoticeNotify *sys)
{
	ChatList *list = load_friend_chat(player_id);
	if (!list)
	{
		list = (ChatList *)malloc(sizeof(ChatList));
		chat_list__init(list);
	}

	if (list->n_systems >= (size_t)MAX_FRIEND_SYSTEM_NUM)
	{
		free(list->systems[0]);
		list->systems[0] = NULL;
		memmove(&list->systems[0], &list->systems[1], (list->n_systems - 1) * sizeof(SystemNoticeNotify*));
	}
	else
	{
		list->n_systems++;
		list->systems = (SystemNoticeNotify**)realloc(list->systems, list->n_systems * sizeof(SystemNoticeNotify*));
	}
	list->systems[list->n_systems - 1] = sys;
	save_friend_chat(player_id, list);

	list->systems[list->n_systems - 1] = NULL;
	list->n_systems--;
	chat_list__free_unpacked(list, NULL);
}

void add_friend_offline_gift(uint64_t player_id, FriendGiftData *gift)
{
	ChatList *list = load_friend_chat(player_id);
	if (!list)
	{
		list = (ChatList *)malloc(sizeof(ChatList));
		chat_list__init(list);
	}

	if (list->n_gifts >= (size_t)MAX_FRIEND_GIFT_NUM)
	{
		free(list->gifts[0]);
		list->gifts[0] = NULL;
		memmove(&list->gifts[0], &list->gifts[1], (list->n_gifts - 1) * sizeof(FriendGiftData*));
	}
	else
	{
		list->n_gifts++;
		list->gifts = (FriendGiftData**)realloc(list->gifts, list->n_gifts * sizeof(FriendGiftData*));
	}
	list->gifts[list->n_gifts - 1] = gift;
	save_friend_chat(player_id, list);

	list->gifts[list->n_gifts - 1] = NULL;
	list->n_gifts--;
	chat_list__free_unpacked(list, NULL);
}

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

void get_all_friend_id(FriendPlayer *player, std::set<uint64_t> &player_ids)
{
	for (int i = 0; i < MAX_FRIEND_RECENT_NUM; ++i)
	{
		uint64_t player_id = player->recents[i];
		if (player_id == 0)
		{
			break;
		}
		player_ids.insert(player_id);
	}
	for (int i = 0; i < MAX_FRIEND_CONTACT_NUM; ++i)
	{
		uint64_t player_id = player->contacts[i].player_id;
		if (player_id == 0)
		{
			break;
		}
		player_ids.insert(player_id);
	}
	for (int i = 0; i < MAX_FRIEND_BLOCK_NUM; ++i)
	{
		uint64_t player_id = player->blocks[i].player_id;
		if (player_id == 0)
		{
			break;
		}
		player_ids.insert(player_id);
	}
	for (int i = 0; i < MAX_FRIEND_ENEMY_NUM; ++i)
	{
		uint64_t player_id = player->enemies[i];
		if (player_id == 0)
		{
			break;
		}
		player_ids.insert(player_id);
	}
	for (int i = 0; i < MAX_FRIEND_APPLY_NUM; ++i)
	{
		uint64_t player_id = player->applys[i];
		if (player_id == 0)
		{
			break;
		}
		player_ids.insert(player_id);
	}
}

int get_all_friend_redis_player(FriendPlayer *player, std::map<uint64_t, PlayerRedisInfo*> &redis_players, AutoReleaseBatchRedisPlayer &_pool)
{
	std::set<uint64_t> player_ids;
	get_all_friend_id(player, player_ids);

	return get_more_redis_player(player_ids, redis_players, sg_player_key, sg_redis_client, _pool);
}

int get_friend_closeness(FriendPlayer *player, uint64_t friend_id)
{
	for (int i = 0; i < MAX_FRIEND_CONTACT_NUM; ++i)
	{
		uint64_t player_id = player->contacts[i].player_id;
		if (player_id == 0)
		{
			break;
		}
		if (player_id == friend_id)
		{
			return player->contacts[i].closeness;
		}
	}
	for (int i = 0; i < MAX_FRIEND_BLOCK_NUM; ++i)
	{
		uint64_t player_id = player->blocks[i].player_id;
		if (player_id == 0)
		{
			break;
		}

		if (player_id == friend_id)
		{
			return player->blocks[i].closeness;
		}
	}

	return 0;
}

PlayerRedisInfo *find_redis_from_map(std::map<uint64_t, PlayerRedisInfo*> &redis_players, uint64_t player_id)
{
	std::map<uint64_t, PlayerRedisInfo*>::iterator iter = redis_players.find(player_id);
	if (iter != redis_players.end())
	{
		return iter->second;
	}
	return NULL;
}

void set_proto_friend(PlayerRedisInfo &player, FriendPlayerBriefData &proto_data)
{
	proto_data.name = player.name;
	proto_data.job = player.job;
	proto_data.level = player.lv;
	proto_data.head = player.head_icon;
	proto_data.offlinetime = player.status;
	proto_data.n_tags = player.n_tags;
	proto_data.tags = player.tags;
	proto_data.textintro = player.textintro;
}

#define MAX_FRIEND_CHANGE_LEN 200

void notify_friend_list_change(FriendPlayer *player, FriendListChangeInfo &changes)
{
	if (changes.adds.size() == 0 && changes.dels.size() == 0)
	{
		return ;
	}

	std::set<uint64_t> player_ids;
	for (std::vector<FriendListAdd>::iterator iter = changes.adds.begin(); iter != changes.adds.end(); ++iter)
	{
		player_ids.insert(iter->player_id);
	}

	std::map<uint64_t, PlayerRedisInfo*> redis_players;
	AutoReleaseBatchRedisPlayer t1;
	int ret = get_more_redis_player(player_ids, redis_players, sg_player_key, sg_redis_client, t1);
	if (ret != 0)
	{
		LOG_ERR("[%s:%d] player[%lu] get redis player failed", __FUNCTION__, __LINE__, player->player_id);
		return ;
	}

	FriendListChangeNotify nty;
	friend_list_change_notify__init(&nty);

	FriendListAddData  add_data[MAX_FRIEND_CHANGE_LEN];
	FriendListAddData* add_point[MAX_FRIEND_CHANGE_LEN];
	FriendPlayerBriefData  unit_data[MAX_FRIEND_CHANGE_LEN];
	FriendListDelData  del_data[MAX_FRIEND_CHANGE_LEN];
	FriendListDelData* del_point[MAX_FRIEND_CHANGE_LEN];

	nty.n_adds = 0;
	nty.adds = add_point;
	for (std::vector<FriendListAdd>::iterator iter = changes.adds.begin(); iter != changes.adds.end(); ++iter)
	{
		FriendListAdd *unit = &(*iter);

		add_point[nty.n_adds] = &add_data[nty.n_adds];
		friend_list_add_data__init(&add_data[nty.n_adds]);
		add_data[nty.n_adds].groupid = unit->group_id;
		add_data[nty.n_adds].data = &unit_data[nty.n_adds];
		friend_player_brief_data__init(&unit_data[nty.n_adds]);
		unit_data[nty.n_adds].playerid = unit->player_id;
		unit_data[nty.n_adds].closeness = unit->closeness;

		PlayerRedisInfo *redis_player = find_redis_from_map(redis_players, unit->player_id);
		if (redis_player)
		{
			set_proto_friend(*redis_player, unit_data[nty.n_adds]);
		}
		nty.n_adds++;
	}
	nty.n_dels = 0;
	nty.dels = del_point;
	for (std::vector<FriendListDel>::iterator iter = changes.dels.begin(); iter != changes.dels.end(); ++iter)
	{
		FriendListDel *unit = &(*iter);

		del_point[nty.n_dels] = &del_data[nty.n_dels];
		friend_list_del_data__init(&del_data[nty.n_dels]);
		del_data[nty.n_dels].groupid = unit->group_id;
		del_data[nty.n_dels].playerid = unit->player_id;
		nty.n_dels++;
	}

	EXTERN_DATA ext_data;
	ext_data.player_id = player->player_id;

	fast_send_msg(&conn_node_friendsrv::connecter, &ext_data, MSG_ID_FRIEND_LIST_CHANGE_NOTIFY, friend_list_change_notify__pack, nty);

	// for (std::map<uint64_t, PlayerRedisInfo*>::iterator iter = redis_players.begin(); iter != redis_players.end(); ++iter)
	// {
	// 	player_redis_info__free_unpacked(iter->second, NULL);
	// }
}

void sync_friend_num_to_game_srv(FriendPlayer *player)
{
	PROTO_FRIEND_SYNC_INFO *proto = (PROTO_FRIEND_SYNC_INFO*)conn_node_base::get_send_buf(SERVER_PROTO_FRIEND_SYNC_FRIEND_NUM, 0);
	memset(proto->contacts, 0, sizeof(proto->contacts));
	for (int i = 0; i < MAX_FRIEND_CONTACT_NUM; ++i)
	{
		if (player->contacts[i].player_id == 0)
		{
			break;
		}

		proto->contacts[i].player_id = player->contacts[i].player_id;
		proto->contacts[i].closeness = player->contacts[i].closeness;
	}

	EXTERN_DATA ext_data;
	ext_data.player_id = player->player_id;

	fast_send_msg_base(&conn_node_friendsrv::connecter, &ext_data, SERVER_PROTO_FRIEND_SYNC_FRIEND_NUM, sizeof(PROTO_FRIEND_SYNC_INFO) - sizeof(PROTO_HEAD), 0);
}

bool is_in_contact(FriendPlayer *player, uint64_t target_id)
{
	for (int i = 0; i < MAX_FRIEND_CONTACT_NUM; ++i)
	{
		if (player->contacts[i].player_id == 0)
		{
			break;
		}

		if (player->contacts[i].player_id == target_id)
		{
			return true;
		}
	}

	return false;
}

bool is_in_block(FriendPlayer *player, uint64_t target_id)
{
	for (int i = 0; i < MAX_FRIEND_BLOCK_NUM; ++i)
	{
		if (player->blocks[i].player_id == 0)
		{
			break;
		}

		if (player->blocks[i].player_id == target_id)
		{
			return true;
		}
	}

	return false;
}

int get_recent_limit_num(void)
{
	return std::min(sg_friend_recent_num, (uint32_t)MAX_FRIEND_RECENT_NUM);
}

int get_block_limit_num(void)
{
	return std::min(sg_friend_block_num, (uint32_t)MAX_FRIEND_BLOCK_NUM);
}

int get_enemy_limit_num(void)
{
	return std::min(sg_friend_enemy_num, (uint32_t)MAX_FRIEND_ENEMY_NUM);
}

int get_apply_limit_num(void)
{
	return std::min(sg_friend_apply_num, (uint32_t)MAX_FRIEND_APPLY_NUM);
}

int get_group_limit_num(void)
{
	return std::min(sg_friend_group_num, (uint32_t)MAX_FRIEND_GROUP_NUM);
}

int get_contact_limit_num(FriendPlayer *player)
{
	if (player->contact_extend)
	{
		return sg_friend_contact_extend_num;
	}
	return sg_friend_contact_num;
}

int get_contact_num(FriendPlayer *player)
{
	int num = 0;
	for (int i = 0; i < MAX_FRIEND_CONTACT_NUM; ++i)
	{
		if (player->contacts[i].player_id == 0)
		{
			break;
		}

		num++;
	}

	return num;
}

int get_contact_idx(FriendPlayer *player, uint64_t member_id)
{
	int idx = -1;
	int limit_num = get_contact_limit_num(player);
	for (int i = 0; i < limit_num; ++i)
	{
		if (player->contacts[i].player_id == 0)
		{
			break;
		}
		if (player->contacts[i].player_id == member_id)
		{
			idx = i;
			break;
		}
	}

	return idx;
}

uint32_t get_new_group_id(FriendPlayer *player)
{
	uint32_t id = FRIEND_LIST_TYPE__L_CUSTOM_BEGIN;
	int group_limit_num = get_group_limit_num();
	for (int i = 0; i < group_limit_num; ++i)
	{
		if (player->groups[i].group_id >= id)
		{
			id = player->groups[i].group_id + 1;
		}
	}

	return id;
}

int get_recent_num(FriendPlayer *player)
{
	int num = 0;
	for (int i = 0; i < MAX_FRIEND_RECENT_NUM; ++i)
	{
		if (player->recents[i] == 0)
		{
			break;
		}

		num++;
	}

	return num;
}

int add_contact(FriendPlayer *player, uint64_t target_id, FriendListChangeInfo &change_info, bool bDelApply)
{
	if (!player_is_exist(target_id))
	{
		LOG_ERR("[%s:%d] player[%lu] target no exist, target_id:%lu", __FUNCTION__, __LINE__, player->player_id, target_id);
		return ERROR_ID_FRIEND_ID;
	}

	if (target_id == player->player_id)
	{
		LOG_ERR("[%s:%d] player[%lu] can't add self, target_id:%lu", __FUNCTION__, __LINE__, player->player_id, target_id);
		return ERROR_ID_FRIEND_SELF;
	}

	int contact_empty_idx = -1;
	int contact_limit_num = get_contact_limit_num(player);
	for (int i = 0; i < contact_limit_num; ++i)
	{
		if (player->contacts[i].player_id == 0)
		{
			contact_empty_idx = i;
			break;
		}

		if (player->contacts[i].player_id == target_id)
		{
			LOG_ERR("[%s:%d] player[%lu] target in contact, target_id:%lu", __FUNCTION__, __LINE__, player->player_id, target_id);
			return ERROR_ID_FRIEND_IN_CONTACT;
		}
	}

	if (contact_empty_idx < 0)
	{
		LOG_ERR("[%s:%d] player[%lu] list full, target_id:%lu, cur_num:%u", __FUNCTION__, __LINE__, player->player_id, target_id, contact_limit_num);
		if (player->contact_extend == 0)
		{
			return 190500207;
		}
		else
		{
			return 190500209;
		}
	}

	int idx = -1;
	for (int i = 0; i < MAX_FRIEND_BLOCK_NUM; ++i)
	{
		if (player->blocks[i].player_id == 0)
		{
			break;
		}

		if (player->blocks[i].player_id == target_id)
		{
			idx = i;
			break;
		}
	}

	//如果在黑名单列表里，从那边获取数据，然后删掉黑名单
	if (idx < 0)
	{
		player->contacts[contact_empty_idx].player_id = target_id;
		player->contacts[contact_empty_idx].closeness = 0;
		player->contacts[contact_empty_idx].gift_num = 0;
		player->contacts[contact_empty_idx].group_id = 0;
	}
	else
	{
		player->contacts[contact_empty_idx].player_id = target_id;
		player->contacts[contact_empty_idx].closeness = player->blocks[idx].closeness;
		player->contacts[contact_empty_idx].gift_num = player->blocks[idx].gift_num;
		player->contacts[contact_empty_idx].group_id = 0;
		del_block(player, target_id, change_info);
	}

	//维护反向映射表
	contact_me_map[target_id].insert(player->player_id);
	watch_me_map[target_id].insert(player->player_id);

	FriendListAdd contact_add;
	contact_add.group_id = FRIEND_LIST_TYPE__L_CONTACT;
	contact_add.player_id = player->contacts[contact_empty_idx].player_id;
	contact_add.closeness = player->contacts[contact_empty_idx].closeness;
	change_info.adds.push_back(contact_add);

	sync_friend_num_to_game_srv(player);

	//从申请列表里删除
	if (bDelApply)
	{
		del_apply(player, target_id, &change_info);
	}

	//给对方的申请列表插入一条
	FriendPlayer *target = get_friend_player(target_id);
	if (target)
	{
		AutoReleaseBatchFriendPlayer arb_friend;
		arb_friend.push_back(target);
		add_apply(target, player->player_id);
	}

	return 0;
}

int del_contact(FriendPlayer *player, uint64_t target_id, FriendListChangeInfo &change_info)
{
	int idx = -1;
	for (int i = 0; i < MAX_FRIEND_CONTACT_NUM; ++i)
	{
		if (player->contacts[i].player_id == 0)
		{
			break;
		}

		if (player->contacts[i].player_id == target_id)
		{
			idx = i;
			break;
		}
	}

	if (idx < 0)
	{
		return ERROR_ID_FRIEND_NOT_IN_LIST;
	}

	FriendListDel contact_del;
	contact_del.group_id = (player->contacts[idx].group_id > 0 ? player->contacts[idx].group_id : FRIEND_LIST_TYPE__L_CONTACT);
	contact_del.player_id = target_id;
	change_info.dels.push_back(contact_del);

	int last_idx = MAX_FRIEND_CONTACT_NUM - 1;
	if (idx < last_idx)
	{
		memmove(&player->contacts[idx], &player->contacts[idx+1], (last_idx - idx) * sizeof(FriendUnit));
	}
	memset(&player->contacts[last_idx], 0, sizeof(FriendUnit));

	del_recent(player, target_id, change_info);

	//维护反向映射表
	if (is_friend_gone(player, target_id))
	{
		watch_me_map[target_id].erase(player->player_id);
		contact_me_map[target_id].erase(player->player_id);
	}

	sync_friend_num_to_game_srv(player);

	return 0;
}

int add_apply(FriendPlayer *player, uint64_t target_id)
{
	//申请开关已关闭，不接收申请
	if (player->apply_switch != 0)
	{
		return 0;
	}
	if (target_id == player->player_id)
	{
		return ERROR_ID_FRIEND_SELF;
	}
	if (is_in_contact(player, target_id))
	{
		return ERROR_ID_FRIEND_IN_CONTACT;
	}
	if (is_in_block(player, target_id))
	{
		return ERROR_ID_FRIEND_IN_BLOCK;
	}

	int empty_idx = -1;
	int limit_num = get_apply_limit_num();
	for (int i = 0; i < limit_num; ++i)
	{
		if (player->applys[i] == 0)
		{
			empty_idx = i;
			break;
		}
		if (player->applys[i] == target_id)
		{
			return ERROR_ID_FRIEND_IN_APPLY;
		}
	}

	FriendListChangeInfo change_info;
	bool bNotice = false;
	if (empty_idx < 0)
	{ //玩家不在列表里，没空位，顶掉最早那个
		//必须先把要删掉的玩家信息记录下来
		FriendListDel apply_del;
		apply_del.group_id = FRIEND_LIST_TYPE__L_APPLY;
		apply_del.player_id = player->applys[0];
		change_info.dels.push_back(apply_del);

		uint32_t last_idx = limit_num - 1;
		memmove(&player->applys[0], &player->applys[1], (last_idx) * sizeof(uint64_t));
		memset(&player->applys[last_idx], 0, sizeof(uint64_t));
		empty_idx = last_idx;
		bNotice = true;

		//维护反向映射表
		if (is_friend_gone(player, apply_del.player_id))
		{
			watch_me_map[apply_del.player_id].erase(player->player_id);
		}
	}

	player->applys[empty_idx] = target_id;
	//维护反向映射表
	watch_me_map[target_id].insert(player->player_id);

	FriendListAdd apply_add;
	apply_add.group_id = FRIEND_LIST_TYPE__L_APPLY;
	apply_add.player_id = target_id;
	apply_add.closeness = 0;
	change_info.adds.push_back(apply_add);

	save_friend_player(player);

	AutoReleaseBatchRedisPlayer arb_redis;
	PlayerRedisInfo *redis_player = get_redis_player(player->player_id, conn_node_friendsrv::server_key, sg_redis_client, arb_redis);
	if (redis_player)
	{
		if (redis_player->status == 0)
		{
			notify_friend_list_change(player, change_info);
		}
	}

	if (bNotice)
	{
		SystemNoticeNotify sys;
		system_notice_notify__init(&sys);

		sys.id = 190500224;

		if (redis_player && redis_player->status == 0)
		{ //在线直接发送
			EXTERN_DATA ext_data;
			ext_data.player_id = player->player_id;

			fast_send_msg(&conn_node_friendsrv::connecter, &ext_data, MSG_ID_SYSTEM_NOTICE_NOTIFY, system_notice_notify__pack, sys);
		}
		else
		{ //离线存库
			add_friend_offline_system(player->player_id, &sys);
		}
	}

	PlayerRedisInfo *redis_target = get_redis_player(target_id, conn_node_friendsrv::server_key, sg_redis_client, arb_redis);
	if (redis_target)
	{
		SystemNoticeNotify sys;
		system_notice_notify__init(&sys);

		std::vector<char*> args;
		args.push_back(redis_target->name);
		sys.id = 190500194;
		sys.args = &args[0];
		sys.n_args = args.size();
		sys.targetid = target_id;
		sys.has_targetid = true;

		if (redis_player && redis_player->status == 0)
		{ //在线直接发送
			EXTERN_DATA ext_data;
			ext_data.player_id = player->player_id;

			fast_send_msg(&conn_node_friendsrv::connecter, &ext_data, MSG_ID_SYSTEM_NOTICE_NOTIFY, system_notice_notify__pack, sys);
		}
		else
		{ //离线存库
			add_friend_offline_system(player->player_id, &sys);
		}
	}

	return 0;
}

int del_apply(FriendPlayer *player, uint64_t target_id, FriendListChangeInfo *change_info)
{
	int idx = -1;
	for (int i = 0; i < MAX_FRIEND_APPLY_NUM; ++i)
	{
		if (player->applys[i] == 0)
		{
			break;
		}

		if (player->applys[i] == target_id)
		{
			idx = i;
			break;
		}
	}

	if (idx < 0)
	{
		return ERROR_ID_FRIEND_NOT_IN_LIST;
	}

	if (change_info)
	{
		FriendListDel apply_del;
		apply_del.group_id = FRIEND_LIST_TYPE__L_APPLY;
		apply_del.player_id = target_id;
		change_info->dels.push_back(apply_del);
	}

	int last_idx = MAX_FRIEND_APPLY_NUM - 1;
	if (idx < last_idx)
	{
		memmove(&player->applys[idx], &player->applys[idx + 1], (last_idx - idx) * sizeof(uint64_t));
	}
	memset(&player->applys[last_idx], 0, sizeof(uint64_t));

	//维护反向映射表
	if (is_friend_gone(player, target_id))
	{
		watch_me_map[target_id].erase(player->player_id);
	}

	return 0;
}

int del_apply_idx(FriendPlayer *player, int idx)
{
	if (idx < 0 || idx >= MAX_FRIEND_APPLY_NUM)
	{
		return -1;
	}

	int last_idx = MAX_FRIEND_APPLY_NUM - 1;
	if (idx < last_idx)
	{
		memmove(&player->applys[idx], &player->applys[idx + 1], (last_idx - idx) * sizeof(uint64_t));
	}
	memset(&player->applys[last_idx], 0, sizeof(uint64_t));

	return 0;
}

int add_block(FriendPlayer *player, uint64_t target_id)
{
	if (!player_is_exist(target_id))
	{
		LOG_ERR("[%s:%d] player[%lu] target no exist, target_id:%lu", __FUNCTION__, __LINE__, player->player_id, target_id);
		return ERROR_ID_FRIEND_ID;
	}

	if (target_id == player->player_id)
	{
		LOG_ERR("[%s:%d] player[%lu] can't add self, target_id:%lu", __FUNCTION__, __LINE__, player->player_id, target_id);
		return ERROR_ID_FRIEND_SELF;
	}

	FriendListChangeInfo change_info;
	int block_empty_idx = -1;
	int block_limit_num = get_block_limit_num();
	for (int i = 0; i < block_limit_num; ++i)
	{
		if (player->blocks[i].player_id == 0)
		{
			block_empty_idx = i;
			break;
		}

		if (player->blocks[i].player_id == target_id)
		{
			return ERROR_ID_FRIEND_IN_BLOCK;
		}
	}

	if (block_empty_idx < 0)
	{ //玩家不在列表里，没空位，顶掉最早那个
		//必须先把要删掉的玩家信息记录下来
		FriendListDel block_del;
		block_del.group_id = FRIEND_LIST_TYPE__L_BLOCK;
		block_del.player_id = player->blocks[0].player_id;
		change_info.dels.push_back(block_del);

		uint32_t last_idx = block_limit_num - 1;
		memmove(&player->blocks[0], &player->blocks[1], (last_idx) * sizeof(FriendUnit));
		memset(&player->blocks[last_idx], 0, sizeof(FriendUnit));
		block_empty_idx = last_idx;

		conn_node_friendsrv::connecter.send_system_notice(player->player_id, 190500213, NULL, 0);
		//维护反向映射表
		if (is_friend_gone(player, block_del.player_id))
		{
			watch_me_map[block_del.player_id].erase(player->player_id);
		}
	}

	int contact_idx = -1;
	for (int i = 0; i < MAX_FRIEND_CONTACT_NUM; ++i)
	{
		if (player->contacts[i].player_id == 0)
		{
			break;
		}
		if (player->contacts[i].player_id == target_id)
		{
			contact_idx = i;
			break;
		}
	}

	//如果在好友列表里，从那边获取数据，然后删掉好友
	if (contact_idx < 0)
	{
		player->blocks[block_empty_idx].player_id = target_id;
		player->blocks[block_empty_idx].closeness = 0;
		player->blocks[block_empty_idx].gift_num = 0;
		player->blocks[block_empty_idx].group_id = 0;
	}
	else
	{
		player->blocks[block_empty_idx].player_id = target_id;
		player->blocks[block_empty_idx].closeness = player->contacts[contact_idx].closeness;
		player->blocks[block_empty_idx].gift_num = player->contacts[contact_idx].gift_num;
		player->blocks[block_empty_idx].group_id = 0;
		del_contact(player, target_id, change_info);
	}
	
	//维护反向映射表
	watch_me_map[target_id].insert(player->player_id);

	FriendListAdd block_add;
	block_add.group_id = FRIEND_LIST_TYPE__L_BLOCK;
	block_add.player_id = player->blocks[block_empty_idx].player_id;
	block_add.closeness = player->blocks[block_empty_idx].closeness;
	change_info.adds.push_back(block_add);

	//从最近联系人删除？
	del_recent(player, target_id, change_info);
	
	//从申请列表里删除
	del_apply(player, target_id, &change_info);

	save_friend_player(player);
	
	//如果玩家在线，下发通知
	AutoReleaseRedisPlayer p1;
	PlayerRedisInfo *redis_player = get_redis_player(player->player_id, conn_node_friendsrv::server_key, sg_redis_client, p1);	
	if (redis_player)
	{
		if (redis_player->status == 0)
		{
			notify_friend_list_change(player, change_info);
		}
	}

	return 0;
}

int del_block(FriendPlayer *player, uint64_t target_id, FriendListChangeInfo &change_info)
{
	int idx = -1;
	for (int i = 0; i < MAX_FRIEND_BLOCK_NUM; ++i)
	{
		if (player->blocks[i].player_id == 0)
		{
			break;
		}

		if (player->blocks[i].player_id == target_id)
		{
			idx = i;
			break;
		}
	}

	if (idx < 0)
	{
		return ERROR_ID_FRIEND_NOT_IN_LIST;
	}

	FriendListDel block_del;
	block_del.group_id = FRIEND_LIST_TYPE__L_BLOCK;
	block_del.player_id = target_id;
	change_info.dels.push_back(block_del);

	int last_idx = MAX_FRIEND_BLOCK_NUM - 1;
	if (idx < last_idx)
	{
		memmove(&player->blocks[idx], &player->blocks[idx + 1], (last_idx - idx) * sizeof(FriendUnit));
	}
	memset(&player->blocks[last_idx], 0, sizeof(FriendUnit));

	//维护反向映射表
	if (is_friend_gone(player, target_id))
	{
		watch_me_map[target_id].erase(player->player_id);
	}

	return 0;
}

int add_enemy(FriendPlayer *player, uint64_t target_id)
{
	if (target_id == player->player_id)
	{
		return ERROR_ID_FRIEND_SELF;
	}

	int empty_idx = -1;
	int limit_num = get_enemy_limit_num();
	for (int i = 0; i < limit_num; ++i)
	{
		if (player->enemies[i] == 0)
		{
			empty_idx = i;
			break;
		}
		if (player->enemies[i] == target_id)
		{
			return 0;
		}
	}

	FriendListChangeInfo change_info;
	bool bNotice = false;
	if (empty_idx < 0)
	{ //玩家不在列表里，没空位，顶掉最早那个
		//必须先把要删掉的玩家信息记录下来
		FriendListDel enemy_del;
		enemy_del.group_id = FRIEND_LIST_TYPE__L_ENEMY;
		enemy_del.player_id = player->enemies[0];
		change_info.dels.push_back(enemy_del);

		uint32_t last_idx = limit_num - 1;
		memmove(&player->enemies[0], &player->enemies[1], (last_idx) * sizeof(uint64_t));
		memset(&player->enemies[last_idx], 0, sizeof(uint64_t));
		empty_idx = last_idx;
		bNotice = true;

		//维护反向映射表
		if (is_friend_gone(player, enemy_del.player_id))
		{
			watch_me_map[enemy_del.player_id].erase(player->player_id);
		}
	}

	player->enemies[empty_idx] = target_id;
	//维护反向映射表
	watch_me_map[target_id].insert(player->player_id);

	FriendListAdd enemy_add;
	enemy_add.group_id = FRIEND_LIST_TYPE__L_ENEMY;
	enemy_add.player_id = target_id;
	enemy_add.closeness = 0;
	change_info.adds.push_back(enemy_add);

	save_friend_player(player);

	AutoReleaseRedisPlayer p1;
	PlayerRedisInfo *redis_player = get_redis_player(player->player_id, conn_node_friendsrv::server_key, sg_redis_client, p1);
	if (redis_player)
	{
		if (redis_player->status == 0)
		{
			notify_friend_list_change(player, change_info);
		}
	}

	if (bNotice)
	{
		SystemNoticeNotify sys;
		system_notice_notify__init(&sys);

		sys.id = 190500218;

		if (redis_player && redis_player->status == 0)
		{ //在线直接发送
			EXTERN_DATA ext_data;
			ext_data.player_id = player->player_id;

			fast_send_msg(&conn_node_friendsrv::connecter, &ext_data, MSG_ID_SYSTEM_NOTICE_NOTIFY, system_notice_notify__pack, sys);
		}
		else
		{ //离线存库
			add_friend_offline_system(player->player_id, &sys);
		}
	}

	return 0;
}

int del_enemy(FriendPlayer *player, uint64_t target_id, FriendListChangeInfo &change_info)
{
	int idx = -1;
	for (int i = 0; i < MAX_FRIEND_ENEMY_NUM; ++i)
	{
		if (player->enemies[i] == 0)
		{
			break;
		}

		if (player->enemies[i] == target_id)
		{
			idx = i;
			break;
		}
	}

	if (idx < 0)
	{
		return ERROR_ID_FRIEND_NOT_IN_LIST;
	}

	FriendListDel enemy_del;
	enemy_del.group_id = FRIEND_LIST_TYPE__L_ENEMY;
	enemy_del.player_id = target_id;
	change_info.dels.push_back(enemy_del);

	int last_idx = MAX_FRIEND_ENEMY_NUM - 1;
	if (idx < last_idx)
	{
		memmove(&player->enemies[idx], &player->enemies[idx + 1], (last_idx - idx) * sizeof(uint64_t));
	}
	memset(&player->enemies[last_idx], 0, sizeof(uint64_t));

	//维护反向映射表
	if (is_friend_gone(player, target_id))
	{
		watch_me_map[target_id].erase(player->player_id);
	}

	return 0;
}

int add_recent(FriendPlayer *player, uint64_t target_id)
{
	if (target_id == player->player_id)
	{
		return ERROR_ID_FRIEND_SELF;
	}

	int empty_idx = -1;
	int limit_num = get_recent_limit_num();
	for (int i = 0; i < limit_num; ++i)
	{
		if (player->recents[i] == 0)
		{
			empty_idx = i;
			break;
		}
		if (player->recents[i] == target_id)
		{
			//把联系人提到最新
			int last_idx = get_recent_num(player);
			if (i != last_idx - 1)
			{
				player->recents[i] = player->recents[last_idx - 1];
				player->recents[last_idx - 1] = target_id;
				save_friend_player(player);
			}
			return 0;
		}
	}

	FriendListChangeInfo change_info;
	if (empty_idx < 0)
	{ //玩家不在列表里，没空位，顶掉最早那个
		//必须先把要删掉的玩家信息记录下来
		FriendListDel recent_del;
		recent_del.group_id = FRIEND_LIST_TYPE__L_RECENT;
		recent_del.player_id = player->recents[0];
		change_info.dels.push_back(recent_del);

		uint32_t last_idx = limit_num - 1;
		memmove(&player->recents[0], &player->recents[1], (last_idx) * sizeof(uint64_t));
		memset(&player->recents[last_idx], 0, sizeof(uint64_t));
		empty_idx = last_idx;

		//维护反向映射表
		if (is_friend_gone(player, recent_del.player_id))
		{
			watch_me_map[recent_del.player_id].erase(player->player_id);
		}
	}

	player->recents[empty_idx] = target_id;
	//维护反向映射表
	watch_me_map[target_id].insert(player->player_id);

	FriendListAdd recent_add;
	recent_add.group_id = FRIEND_LIST_TYPE__L_RECENT;
	recent_add.player_id = target_id;
	recent_add.closeness = 0;
	change_info.adds.push_back(recent_add);

	save_friend_player(player);

	AutoReleaseRedisPlayer p1;
	PlayerRedisInfo *redis_player = get_redis_player(player->player_id, conn_node_friendsrv::server_key, sg_redis_client, p1);	
	if (redis_player)
	{
		if (redis_player->status == 0)
		{
			notify_friend_list_change(player, change_info);
		}
	}

	return 0;
}

int del_recent(FriendPlayer *player, uint64_t target_id, FriendListChangeInfo &change_info)
{
	int idx = -1;
	for (int i = 0; i < MAX_FRIEND_RECENT_NUM; ++i)
	{
		if (player->recents[i] == 0)
		{
			break;
		}

		if (player->recents[i] == target_id)
		{
			idx = i;
			break;
		}
	}

	if (idx < 0)
	{
		return ERROR_ID_FRIEND_NOT_IN_LIST;
	}

	FriendListDel recent_del;
	recent_del.group_id = FRIEND_LIST_TYPE__L_RECENT;
	recent_del.player_id = target_id;
	change_info.dels.push_back(recent_del);

	int last_idx = MAX_FRIEND_RECENT_NUM - 1;
	if (idx < last_idx)
	{
		memmove(&player->recents[idx], &player->recents[idx + 1], (last_idx - idx) * sizeof(uint64_t));
	}
	memset(&player->recents[last_idx], 0, sizeof(uint64_t));

	//维护反向映射表
	if (is_friend_gone(player, target_id))
	{
		watch_me_map[target_id].erase(player->player_id);
	}

	return 0;
}

void try_reset_friend_player(FriendPlayer *player)
{
	uint32_t cur_tick = time_helper::get_cached_time() / 1000;
	if (player->reset_time == 0 || cur_tick >= time_helper::nextOffsetTime(daily_reset_clock, player->reset_time))
	{
		player->reset_time = cur_tick;
		for (int i = 0; i < MAX_FRIEND_CONTACT_NUM; ++i)
		{
			if (player->contacts[i].player_id == 0)
			{
				break;
			}

			player->contacts[i].gift_num = 0;
		}

		player->gift_accept = 0;
		save_friend_player(player);
	}
}

void notify_friend_closeness_update(FriendPlayer *player, FriendUnit &unit)
{
	FriendUpdateUnitNotify nty;
	friend_update_unit_notify__init(&nty);

	nty.playerid = unit.player_id;
	nty.closeness = unit.closeness;
	nty.has_closeness = true;

	EXTERN_DATA ext_data;
	ext_data.player_id = player->player_id;

	fast_send_msg(&conn_node_friendsrv::connecter, &ext_data, MSG_ID_FRIEND_UPDATE_UNIT_NOTIFY, friend_update_unit_notify__pack, nty);
}

std::set<uint64_t> *get_contact_me_players(uint64_t player_id)
{
	std::map<uint64_t, std::set<uint64_t> >::iterator iter = contact_me_map.find(player_id);
	if (iter != contact_me_map.end())
	{
		return &iter->second;
	}
	return NULL;
}

std::set<uint64_t> *get_watch_me_players(uint64_t player_id)
{
	std::map<uint64_t, std::set<uint64_t> >::iterator iter = watch_me_map.find(player_id);
	if (iter != watch_me_map.end())
	{
		return &iter->second;
	}
	return NULL;
}

static void rebuild_relative_map(FriendPlayer *player)
{
	for (int i = 0; i < MAX_FRIEND_RECENT_NUM; ++i)
	{
		uint64_t player_id = player->recents[i];
		if (player_id == 0)
		{
			break;
		}
		watch_me_map[player_id].insert(player->player_id);
	}
	for (int i = 0; i < MAX_FRIEND_CONTACT_NUM; ++i)
	{
		uint64_t player_id = player->contacts[i].player_id;
		if (player_id == 0)
		{
			break;
		}

		contact_me_map[player_id].insert(player->player_id);
		watch_me_map[player_id].insert(player->player_id);
	}
	for (int i = 0; i < MAX_FRIEND_BLOCK_NUM; ++i)
	{
		uint64_t player_id = player->blocks[i].player_id;
		if (player_id == 0)
		{
			break;
		}
		watch_me_map[player_id].insert(player->player_id);
	}
	for (int i = 0; i < MAX_FRIEND_ENEMY_NUM; ++i)
	{
		uint64_t player_id = player->enemies[i];
		if (player_id == 0)
		{
			break;
		}
		watch_me_map[player_id].insert(player->player_id);
	}
	for (int i = 0; i < MAX_FRIEND_APPLY_NUM; ++i)
	{
		uint64_t player_id = player->applys[i];
		if (player_id == 0)
		{
			break;
		}
		watch_me_map[player_id].insert(player->player_id);
	}
}

void rebuild_watch_info(void)
{
	contact_me_map.clear();
	watch_me_map.clear();
	CRedisClient &rc = sg_redis_client;

	CAutoRedisReply autoR;	
	redisReply *r = rc.hgetall_bin(sg_friend_key, autoR);
	if (r == NULL || r->type != REDIS_REPLY_ARRAY)
	{
		LOG_ERR("[%s:%d] get redis failed", __FUNCTION__, __LINE__);
		return ;
	}

	FriendPlayer *player = (FriendPlayer *)malloc(sizeof(FriendPlayer));
	AutoReleaseBatchFriendPlayer arb_friend;
	arb_friend.push_back(player);

	uint64_t player_id = 0;
	int ret = 0;
	for (size_t i = 0; i + 1 < r->elements; i = i + 2)
	{
		struct redisReply *field = r->element[i];
		struct redisReply *value = r->element[i+1];
		if (field->type != REDIS_REPLY_STRING || value->type != REDIS_REPLY_STRING)
			continue;

		player_id = strtoull(field->str, NULL, 10);
		if (player_id == 0)
			continue;

		memset(player, 0, sizeof(FriendPlayer));
		ret = unpack_friend_player((uint8_t*)value->str, value->len, player);
		if (ret != 0)
		{
			LOG_ERR("[%s:%d] unpack player[%lu] failed, ret:%d", __FUNCTION__, __LINE__, player_id, ret);
			continue;
		}

		rebuild_relative_map(player);
	}
}

bool is_friend_gone(FriendPlayer *player, uint64_t target_id)
{
	std::set<uint64_t> player_ids;
	get_all_friend_id(player, player_ids);
	return (player_ids.find(target_id) == player_ids.end());
}






