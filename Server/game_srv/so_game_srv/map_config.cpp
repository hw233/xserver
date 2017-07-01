#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "map_config.h"
#include "game_event.h"
struct map_data
{
	uint16_t size_x;
	uint16_t size_z;
	int16_t offset_x;
	int16_t offset_z;
	uint16_t block_data[0];
};

static int calc_merge_block_width(struct map_config *config, int x, int z, bool b)
{
	int ret = 0;
	for (int i = x; i < config->size_x; ++i)
	{
		int index = z * config->size_x + i;
		if (config->block_data[index].first_block != (uint32_t)-1)
			break;
		if (config->block_data[index].can_walk != b)
			break;
		++ret;
	}
	return ret;
}
static int calc_merge_block_height(struct map_config *config, int x, int z, bool b)
{
	int ret = 0;
	for (int i = z; i < config->size_z; ++i)
	{
		int index = i * config->size_x + x;
		if (config->block_data[index].first_block != (uint32_t)-1)
			break;
		if (config->block_data[index].can_walk != b)
			break;
		++ret;
	}
	return ret;
}

static void	set_first_block(struct map_config *config, int first, int x, int z, int width, int height, bool b)
{
	if (width < height)
	{
		for (int i = z; i < z + height; ++i)
		{
			for (int j = x; j < x + width; ++j)
			{
				int index = i * config->size_x + j;
				if (config->block_data[index].first_block != (uint32_t)-1)
					return;
				if (config->block_data[index].can_walk != b)
					return;
			}
			for (int j = x; j < x + width; ++j)
			{
				int index = i * config->size_x + j;
				config->block_data[index].first_block = first;
			}			
		}
	}
	else
	{
		for (int i = x; i < x + width; ++i)
		{
			for (int j = z; j < z + height; ++j)
			{
				int index = j * config->size_x + i;
				if (config->block_data[index].first_block != (uint32_t)-1)
					return;
				if (config->block_data[index].can_walk != b)
					return;
			}
			for (int j = z; j < z + height; ++j)
			{
				int index = j * config->size_x + i;
				config->block_data[index].first_block = first;
			}			
		}
	}
}

static void merge_block(struct map_config *config)
{
	for (int i = 0; i < config->size_z; ++i)
	{
		for (int j = 0; j < config->size_x; ++j)
		{
			int index = i * config->size_x + j;
			if (config->block_data[index].first_block != (uint32_t)-1)
				continue;
			bool b = config->block_data[index].can_walk;
			int width, height;
			width = calc_merge_block_width(config, j+1, i, b);
			height = calc_merge_block_height(config, j, i+1, b);
			if (width <= 0 && height <= 0)
				continue;
			set_first_block(config, index, j, i, width, height, b);
		}
		
	}

	for (int i = 0; i < config->size_z; ++i)
	{
		for (int j = 0; j < config->size_x; ++j)
		{
			int index = i * config->size_x + j;
			if (config->block_data[index].first_block == (uint32_t)-1)
				config->block_data[index].first_block = index;
		}
	}
	
}

struct region_config *create_region_config(char *map_path)
{
	if (!map_path || map_path[0] == '\0')
		return NULL;
	int size;	
	void *buf = NULL;
	struct region_config *ret = NULL;
    int fd = open(map_path, O_RDONLY);
	if (fd <= 0)
	{
		printf("%s: read %s fail", __FUNCTION__, map_path);
		return NULL;
	}
	struct stat stat_buf;
	if (fstat(fd, &stat_buf) != 0)
		goto fail;
	buf = malloc(stat_buf.st_size);
	if (!buf)
		goto fail;
	size = read(fd, buf, stat_buf.st_size);
	if (size != stat_buf.st_size)
		goto fail;
	ret = (struct region_config *)buf;
	close(fd);
	return ret;

fail:
	close(fd);	
	if (buf)
		free(buf);
	return NULL;	
}

struct map_config *create_map_config(char *map_path)
{
	int size;	
	void *buf = NULL;
	struct map_config *ret = NULL;
	struct map_data *map_data;
    int fd = open(map_path, O_RDONLY);
	if (fd <= 0)
		return NULL;
	struct stat stat_buf;
	if (fstat(fd, &stat_buf) != 0)
		goto done;
	buf = malloc(stat_buf.st_size);
	if (!buf)
		return NULL;
	size = read(fd, buf, stat_buf.st_size);
	if (size != stat_buf.st_size)
		goto done;
	map_data = (struct map_data *)buf;
	size = sizeof(map_config) + (map_data->size_x) * (map_data->size_z) * sizeof(struct map_block);
	ret = (struct map_config *)malloc(size);
	if (!ret)
		goto done;
	ret->size_x = map_data->size_x;
	ret->size_z = map_data->size_z;
	ret->offset_x = map_data->offset_x - (map_data->size_x - 1) / 2;
	ret->offset_z = map_data->offset_z - (map_data->size_z - 1) / 2;;
//	printf("%s: %s: %d %d %d %d, offset[%d][%d]\n", __FUNCTION__, map_path, map_data->size_x, map_data->size_z, map_data->offset_x, map_data->offset_z,
//		ret->offset_x, ret->offset_z);
	for (int i = 0; i < ret->size_z; ++i)
	{
		for (int j = 0; j < ret->size_x; ++j)
		{
			ret->block_data[i * ret->size_x + j].can_walk = map_data->block_data[i * map_data->size_x + j];
			ret->block_data[i * ret->size_x + j].first_block = -1;
		}
	}

	merge_block(ret);
done:
	close(fd);
	if (buf)
		free(buf);
	return ret;
}

bool check_pos_valid(struct map_config *config, float pos_x, float pos_z)
{
	float block_x = pos_x - config->offset_x;
	float block_z = pos_z - config->offset_z;	
	if (block_x < 0)
		return false;
	if (block_z < 0)
		return false;
	if (block_x > config->size_x)
		return false;
	if (block_z > config->size_z)
		return false;	
	
	return true;
}

int pos_to_block_x(struct map_config *config, float pos)
{
	assert(pos - config->offset_x >= 0);
	return pos - config->offset_x;
/*	
	pos -= config->offset_x;
	pos += config->size_x / 2.0;
	if (pos < 0 || pos >= config->size_x)
		return -1;
	return pos;
*/	
}
int pos_to_block_z(struct map_config *config, float pos)
{
	assert(pos - config->offset_z >= 0);
	return pos - config->offset_z;
/*	
	pos -= config->offset_z;
	pos += config->size_z / 2.0;	
	if (pos < 0 || pos >= config->size_z)
		return -1;
	return pos;
*/	
}

__attribute__((unused)) static float block_to_pos_x(struct map_config *config, int pos)
{
	return pos + config->offset_x;
/*	
	pos += config->offset_x;
	return (float)pos -  config->size_x / 2.0;
*/	
}

__attribute__((unused)) static float block_to_pos_z(struct map_config *config, int pos)
{
	return pos + config->offset_z;
/*	
	pos += config->offset_z;
	return (float)pos -  config->size_z / 2.0;
*/	
}

uint16_t get_region_id(struct map_config *config, struct region_config *region, int x, int z)
{
	if (!region || !config)
		return (0);
	if (x - config->offset_x < 0)
		return 0;
	if (z - config->offset_z < 0)
		return 0;	
	
	x = pos_to_block_x(config, x);
	z = pos_to_block_z(config, z);	
	if (x >= config->size_x || z >= config->size_z)
		return 0;
	if (x < 0 || z < 0)
		return 0;
	return region->region_id[z * config->size_x + x];
}

struct map_block *get_map_block(struct map_config *config, int x, int z)
{
	if (x - config->offset_x < 0)
		return NULL;
	if (z - config->offset_z < 0)
		return NULL;	
	
	x = pos_to_block_x(config, x);
	z = pos_to_block_z(config, z);	
	if (x >= config->size_x || z >= config->size_z)
		return NULL;
	if (x < 0 || z < 0)
		return NULL;	
	return &config->block_data[z * config->size_x + x];
}

int get_block_index(struct map_config *config, struct map_block *block)
{
	return block - &config->block_data[0];
}
int get_block_x_z_from_index(struct map_config *config, int index, int *x, int *z)
{
	*x = index % config->size_x;
	*z = index / config->size_x;
	return (0);	
}
int get_block_x_z(struct map_config *config, struct map_block *block, int *x, int *z)
{
	int index = get_block_index(config, block);
	return get_block_x_z_from_index(config, index, x, z);
}


//////////////////////////////////////////////
bool minheap_cmp_map_block(void *a, void *b)
{
	struct map_block *aa = (struct map_block *)a;
	struct map_block *bb = (struct map_block *)b;
	if (aa->value_f < bb->value_f)
		return true;
	return false;
}

int minheap_get_map_block_index(void *a)
{
	struct map_block *aa = (struct map_block *)a;
	return aa->heap_index;
}

void minheap_set_map_block_index(int index, void *a)
{
	struct map_block *aa = (struct map_block *)a;
	aa->heap_index = index;	
}
