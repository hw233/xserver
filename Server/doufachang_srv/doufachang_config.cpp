#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "sproto.h"
#include "sprotoc_common.h"
#include "doufachang_config.h"
#include "lua_load.h"

#define READ_SPB_MAX_LEN (1024 * 1024)
typedef std::map<uint64_t, void *> *config_type;
std::map<uint64_t, struct ArenaRewardTable*> doufachang_reward_config; //斗法场奖励

int read_all_config()
{
	char *buf = (char *)malloc(READ_SPB_MAX_LEN);
	if (!buf)
		return -1;
	int fd = open("../excel_data/1.spb", O_RDONLY);
	if (fd <= 0) {
		printf("open 1.spb failed, err = %d\n", errno);
		return (-1);
	}
	size_t size =  read(fd, buf, READ_SPB_MAX_LEN);
	struct sproto *sp = sproto_create(&buf[0], size);
	close(fd);
    lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	int ret;

	struct sproto_type *type = sproto_type(sp, "ArenaRewardTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/ArenaRewardTable.lua", (config_type)&doufachang_reward_config);
	assert(ret == 0);

	lua_close(L);
	sproto_release(sp);
	free(buf);
	return (0);
}
