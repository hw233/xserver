#ifndef MAP_CONFIG_H
#define MAP_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

struct map_block
{
	bool can_walk;
	uint32_t first_block;

		//a* 寻路变量
	uint32_t heap_index;
	uint32_t center_block;
	int value_g;
	int value_f;
	bool closed;
	bool opened;
}__attribute__ ((packed));

struct map_config
{
	uint16_t size_x;
	uint16_t size_z;
	int16_t offset_x;
	int16_t offset_z;
	struct map_block block_data[0];
}__attribute__ ((packed));

struct region_config
{
	uint16_t size_x;
	uint16_t size_z;
	int16_t offset_x;
	int16_t offset_z;
	uint16_t region_id[0];
}__attribute__ ((packed));

struct region_config *create_region_config(char *map_path);
struct map_config *create_map_config(char *map_path);
bool check_pos_valid(struct map_config *config, float pos_x, float pos_z);
int pos_to_block_x(struct map_config *config, float pos);
int pos_to_block_z(struct map_config *config, float pos);
//float block_to_pos_x(struct map_config *config, int pos);
//float block_to_pos_z(struct map_config *config, int pos);
//struct map_block *get_map_block(struct map_config *config, int x, int z);
struct map_block *get_map_block(struct map_config *config, int x, int z);
uint16_t get_region_id(struct map_config *config, struct region_config *region, int x, int z);
int get_block_index(struct map_config *config, struct map_block *block);
int get_block_x_z_from_index(struct map_config *config, int index, int *x, int *z);
int get_block_x_z(struct map_config *config, struct map_block *block, int *x, int *z);

//minheap 操作函数
bool minheap_cmp_map_block(void *a, void *b);
int minheap_get_map_block_index(void *a);
void minheap_set_map_block_index(int index, void *a);
#endif /* MAP_CONFIG_H */
