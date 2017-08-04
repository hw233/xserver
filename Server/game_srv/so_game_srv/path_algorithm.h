#ifndef PATH_ALGORITHM_H
#define PATH_ALGORITHM_H

#include <stdint.h>
#include "unit_path.h"
#include "scene.h"
#include "player.h"

//-1 失败
//0 不完全可达，last保存了最后可以到达的位置
//1 完全可达
int get_last_can_walk(const scene_struct *scene, const struct position *begin, const struct position *end, struct position *last);

//是否直线可达
bool check_can_walk(const scene_struct *scene, const struct position *begin, const struct position *end);
//路径是否合法，返回可以到达的最远的点。path->cur_pos表示可以到第一个点。-1表示全部不可达。path->max_pos表示全部可达
int check_walk_path(player_struct *player, struct unit_path *path);
//寻路算法
int find_path(scene_struct *scene, struct position *begin, struct position *end, struct unit_path *out_path);

//给怪物找个随机直线可达的点
int find_rand_position(scene_struct *scene, struct position *begin, int radius, struct position *out);

int getdirection(const struct position *src, const struct position *dst);
bool get_circle_random_position(scene_struct *scene, int iDirection, const struct position *start,
	const struct position *center, float radius, struct position *ret_pos);

bool get_circle_random_position_v2(scene_struct *scene, const struct position *start,
	const struct position *center, float radius, struct position *ret_pos);

bool get_direct_position_v2(scene_struct *scene, double angle, const struct position *start,
	const struct position *center, float radius, struct position *ret_pos);

bool get_circle_random_position_v3(scene_struct *scene, const struct position *center, float radius,
	struct position *ret_pos);

//获取某个地点的高度信息
float get_pos_height(const scene_struct *scene, const struct position *pos);

#endif /* PATH_ALGORITHM_H */
