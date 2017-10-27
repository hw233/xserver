#include "global_shared_data.h"
#include "mem_pool.h"
#include "game_event.h"
#include <string.h>

struct global_shared_data *global_shared_data;

int init_global_shared_data(unsigned long key)
{
	global_shared_data = (struct global_shared_data *)alloc_shared_mem(0, key, sizeof(struct global_shared_data));
	if (!global_shared_data)
		return (-1);
	LOG_DEBUG("%s: init mem[%d]", __FUNCTION__, sizeof(struct global_shared_data));	
	memset(global_shared_data, 0, sizeof(struct global_shared_data));
	return (0);	
}
int resume_global_shared_data(unsigned long key)
{
	global_shared_data = (struct global_shared_data *)alloc_shared_mem(1, key, sizeof(struct global_shared_data));
	if (!global_shared_data)
		return (-1);	
	return (0);
}
