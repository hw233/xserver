#ifndef GLOBAL_SHARED_DATA_H
#define GLOBAL_SHARED_DATA_H

#include <stdint.h>

struct global_shared_data
{
	uint32_t g_team_id;
};
extern struct global_shared_data *global_shared_data;
int init_global_shared_data(unsigned long key);
int resume_global_shared_data(unsigned long key);

#endif /* GLOBAL_SHARED_DATA_H */
