#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "path_algorithm.h"
#include "map_config.h"
#include "minheap.h"
#include "game_event.h"
#include "player.h"
#include "unit.h"

//-1 失败
//0 不完全可达，last保存了最后可以到达的位置
//1 完全可达
int get_last_can_walk(const scene_struct *scene, const struct position *begin, const struct position *end, struct position *last)
{
	int xs = begin->pos_x;
	int ys = begin->pos_z;
	int xe = end->pos_x;
	int ye = end->pos_z;
	struct map_block *block_start = get_map_block(scene->map_config, xs, ys);
	if (!block_start)
		return -1;	
	struct map_block *block_end = get_map_block(scene->map_config, xe, ye);
	if (!block_end)
		return -1;
	if (!block_start->can_walk)
		return -1;
	if (block_start->first_block != (uint32_t)-1
		&& block_start->first_block == block_end->first_block)
		return 1;

	last->pos_x = xs;
	last->pos_z = ys;	

	int x = xs;
	int y = ys;
	int triangle_x = abs(xe - xs);
	int triangle_y = abs(ye - ys);
	int sx = 1;//xe - xs;
	int sy = 1;//ye - ys;
	if (xe < xs)
		sx = -1;
	if (ye < ys)
		sy = -1;

	int flag = 0;
	if (triangle_y > triangle_x)
	{
		int tmp = triangle_x;
		triangle_x = triangle_y;
		triangle_y = tmp;
		flag = 1;
	}
	int nerror = 2 * triangle_y - triangle_x;
	for (int i = 1; i <= triangle_x; ++i)
	{
//		printf("[%d][%d] ", x, y);
		struct map_block *block = get_map_block(scene->map_config, x, y);
		if (!block || !block->can_walk)
			return 0;

		last->pos_x = x;
		last->pos_z = y;	

		if (nerror >= 0)
		{
			if (flag)
				x = x + sx;
			else
				y = y + sy;
			nerror = nerror - 2 * triangle_x;
		}
		if (flag)
			y = y + sy;
		else
			x = x + sx;
		nerror = nerror + 2 * triangle_y;
	}
//	printf("\n");
//}
	return 1;
}

bool check_can_walk(const scene_struct *scene, const struct position *begin, const struct position *end)
{
/*
	int xs = pos_to_block_x(scene->map_config, begin->pos_x);
	int ys = pos_to_block_z(scene->map_config, begin->pos_z);
	int xe = pos_to_block_x(scene->map_config, end->pos_x);
	int ye = pos_to_block_z(scene->map_config, end->pos_z);
*/
	int xs = begin->pos_x;
	int ys = begin->pos_z;
	int xe = end->pos_x;
	int ye = end->pos_z;
	struct map_block *block_start = get_map_block(scene->map_config, xs, ys);
	if (!block_start)
		return false;	
	struct map_block *block_end = get_map_block(scene->map_config, xe, ye);
	if (!block_end)
		return false;
	if (!block_start->can_walk)
		return false;
	if (block_start->first_block != (uint32_t)-1
		&& block_start->first_block == block_end->first_block)
		return true;

	int x = xs;
	int y = ys;
	int triangle_x = abs(xe - xs);
	int triangle_y = abs(ye - ys);
	int sx = 1;//xe - xs;
	int sy = 1;//ye - ys;
	if (xe < xs)
		sx = -1;
	if (ye < ys)
		sy = -1;

	int flag = 0;
	if (triangle_y > triangle_x)
	{
		int tmp = triangle_x;
		triangle_x = triangle_y;
		triangle_y = tmp;
		flag = 1;
	}
	int nerror = 2 * triangle_y - triangle_x;
	for (int i = 1; i <= triangle_x; ++i)
	{
//		printf("[%d][%d] ", x, y);
		struct map_block *block = get_map_block(scene->map_config, x, y);
		if (!block || !block->can_walk)
			return false;

		if (nerror >= 0)
		{
			if (flag)
				x = x + sx;
			else
				y = y + sy;
			nerror = nerror - 2 * triangle_x;
		}
		if (flag)
			y = y + sy;
		else
			x = x + sx;
		nerror = nerror + 2 * triangle_y;
	}
//	printf("\n");
//}
	return true;
}
int check_walk_path(player_struct *player, struct unit_path *path)
{
//	struct position pos;
//	pos.pos_x = player->data->pos_x;
//	pos.pos_z = player->data->pos_z;
	if (!check_can_walk(player->scene, player->get_pos(), &path->pos[path->cur_pos]))
		return -1;

	for (int i = path->cur_pos; i < path->max_pos; ++i)
	{
		if (!check_can_walk(player->scene, &path->pos[i], &path->pos[i+1]))
			return i;
	}
	return (path->max_pos);
}

static int cur_closed_array;

void clearn_open_close_flag()
{
	for (size_t i = 0; i < g_minheap.cur_size; ++i)
	{
		struct map_block *block = (struct map_block *)g_minheap.nodes[i];
		block->opened = false;
		block->closed = false;
	}

	for (int i = 0; i < cur_closed_array; ++i)
	{
		closed_map_block[cur_closed_array]->opened = false;
		closed_map_block[cur_closed_array]->closed = false;
	}
}

void astar_count_node(struct map_config *config, int x, int z, int xe, int ye, struct map_block *center_block)
{
	struct map_block *block = get_map_block(config, x, z);
	// 此块不可行走，忽略
	if (!block || !block->can_walk)
		return;

	//省略G参数可以提高速度, 但得不到最优路径, 服务器做怪物短路径寻路不碍事
	int value = center_block->value_g + 1;

	// 如果新的G值比较低，那么要重新打开.
	if (block->closed && block->value_g > value)
	{
		block->closed = false;
	}
	// 已经加入封闭列表，忽略
	if (block->closed)
		return;

	// 在开放列表中，但路径比现在优先，忽略
	if (block->opened && block->value_g < value)
	{
		return;
	}
	// 更新当前节点GHF值，并指向新的中心节点
	block->value_g = value;
	block->value_f = value + abs(x - xe) + abs(z - ye);
	block->center_block = get_block_index(config, center_block);

	// 加入开放列表
	if (block->opened)
	{
		adjust_heap_node(&g_minheap, block);
	}
	else
	{
		push_heap(&g_minheap, block);
		block->opened = true;
	}
	return;
}

int find_path(scene_struct *scene, struct position *begin, struct position *end, struct unit_path *out_path)
{
/*
	int xs = pos_to_block_x(scene->map_config, begin->pos_x);
	int ys = pos_to_block_z(scene->map_config, begin->pos_z);
	int xe = pos_to_block_x(scene->map_config, end->pos_x);
	int ye = pos_to_block_z(scene->map_config, end->pos_z);
*/
	int xs = begin->pos_x;
	int ys = begin->pos_z;
	int xe = end->pos_x;
	int ye = end->pos_z;

	struct map_block *block_start = get_map_block(scene->map_config, xs, ys);
	struct map_block *block_end = get_map_block(scene->map_config, xe, ye);
	if (!block_start || !block_end)
		return -1;
	if (!block_start->can_walk)
		return -10;

	g_minheap.cur_size = 0;
	cur_closed_array = 0;

	struct map_block *center_block = block_start;
	while (1)
	{
		assert(center_block->closed == false);
		// 将当前节点加入封闭列表
		center_block->closed = true;
		closed_map_block[cur_closed_array] = center_block;

		if (center_block->first_block == block_end->first_block)
			break;
		int x, z;
		get_block_x_z(scene->map_config, center_block, &x, &z);
		if (x - 1 >= 0)
			astar_count_node(scene->map_config, x - 1, z, xe, ye, center_block);
		if (x + 1 < scene->map_config->size_x)
			astar_count_node(scene->map_config, x + 1, z, xe, ye, center_block);
		if (z - 1 >= 0)
			astar_count_node(scene->map_config, x, z - 1, xe, ye, center_block);
		if (z + 1 < scene->map_config->size_z)
			astar_count_node(scene->map_config, x, z + 1, xe, ye, center_block);
		center_block = (struct map_block *)pop_heap(&g_minheap);
		if (!center_block)
		{
			LOG_INFO("%s %d: find path failed", __FUNCTION__, __LINE__);
			return -20;
		}
		assert(center_block->opened == true);
		center_block->opened = false;
	}

	clearn_open_close_flag();

		// TODO: 优化路径点，反转center链表
/*
	struct map_block *tmp;
	while (center_block && center_block != block_start)
	{
		tmp = center_block->center_block;

	}
*/
	return (0);
}

static int8_t four_direct[][2] =
{
	{1, 0},
	{-1, 0},
	{0, 1},
	{0, -1},
};

int find_rand_position(scene_struct *scene, struct position *begin, int radius, struct position *out)
{
	int ret = -1;
	struct position pos = {begin->pos_x, begin->pos_z};
	int rand_direct = random() % 4;
	for (int i = 0; i < radius; ++i)
	{
		pos.pos_x += four_direct[rand_direct][0];
		pos.pos_z += four_direct[rand_direct][1];
		struct map_block *block = get_map_block(scene->map_config, pos.pos_x, pos.pos_z);
		if (!block || !block->can_walk)
		{
			break;
		}
		out->pos_x = pos.pos_x;
		out->pos_z = pos.pos_z;
		ret = 0;
	}

	return (ret);
}

#define DIRECTION_NORTH 4
#define DIRECTION_SOUTH 8
#define DIRECTION_WEST 6
#define DIRECTION_EAST 2
#define DIRECTION_NORTH_WEST 5
#define DIRECTION_SOUTH_WEST 7
#define DIRECTION_NORTH_EAST 3
#define DIRECTION_SOUTH_EAST 1

// 计算单位移动方向
int getdirection(const struct position *src, const struct position *dst)
{
#define TAN225  0.414f
#define TAN675  2.414f

	if (src->pos_x == dst->pos_x && src->pos_z == dst->pos_z)
	{
		return 0;
	}

	bool grow_z = dst->pos_z > src->pos_z;

	if (dst->pos_x == src->pos_x)
	{
		return grow_z ? DIRECTION_SOUTH : DIRECTION_NORTH;
	}

	float iStartX = src->pos_x;
	float iStartY = src->pos_z;
	float iEndX = dst->pos_x;
	float iEndY = dst->pos_z;

	float K = (float)(iStartY - iEndY) / (float)(iEndX - iStartX);

	bool grow_x = dst->pos_x > src->pos_x;

	if (K >= -TAN225 && K <= TAN225)
	{
		return grow_x? DIRECTION_EAST : DIRECTION_WEST;
	}
	else if (K >= TAN225 && K <= TAN675)
	{
		return grow_x? DIRECTION_NORTH_EAST : DIRECTION_SOUTH_WEST;
	}
	else if (K >= -TAN675 && K <= -TAN225)
	{
		return grow_x? DIRECTION_SOUTH_EAST : DIRECTION_NORTH_WEST;
	}
	else
	{
		return grow_z? DIRECTION_SOUTH : DIRECTION_NORTH;
	}
}

bool get_direct_position_v2(scene_struct *scene, double angle, const struct position *start, const struct position *center, float radius, struct position *ret_pos)
{
	double z = qFastSin(angle) * radius;
	double x = qFastCos(angle) * radius;
	struct position pos;
	pos.pos_x = center->pos_x + x;
	pos.pos_z = center->pos_z + z;	

	if (check_can_walk(scene, start, &pos))
	{
		ret_pos->pos_x = pos.pos_x;
		ret_pos->pos_z = pos.pos_z;
		return true;
	}
	return false;
}

bool get_circle_random_position_v3(scene_struct *scene, const struct position *center, float radius, struct position *ret_pos)
{
	int x, z;
	int r = radius * 1000;
	int half_r = r / 2;
		//最多尝试10次
	for (int i = 0; i < 10; ++i)
	{
		x = center->pos_x + (random() % r - half_r) / 1000.0;
		z = center->pos_z + (random() % r - half_r) / 1000.0;
		struct map_block *block = get_map_block(scene->map_config, x, z);
		if (block && block->can_walk)
		{
			ret_pos->pos_x = x;
			ret_pos->pos_z = z;			
			return true;
		}
	}
	return false;
}

bool get_circle_random_position_v2(scene_struct *scene, const struct position *start, const struct position *center, float radius, struct position *ret_pos)
{
	double angle = pos_to_angle(start->pos_x - center->pos_x, start->pos_z - center->pos_z);	
	double angle_start = angle - 1.4;//M_PI / 2;
//	double angle_end = angle + M_PI / 2;
	double rand = random();
	angle  = (fmod(rand, 2.8/*M_PI*/) + angle_start);

	rand = random();
	radius = fmod(rand, radius / 2) + radius / 2;
	return get_direct_position_v2(scene, angle, start, center, radius, ret_pos);
}

// 以rstCenterPosition为圆心, 在Direction方向上选择一个位置.
// Direction为0表示不限制方向.
// bCheckMask是判断目标坐标上是否已经有怪物, 用于怪物AI追击
// 返回值: true  - rstRandPosition 为 找到的随机位置
//         false - rstRandPosition 为 rstCenterPosition
bool get_circle_random_position(scene_struct *scene, int iDirection, const struct position *start,
	const struct position *center, float radius, struct position *ret_pos)
{
//    rstRandPosition = rstCenterPosition;

	// 16个方向的坐标转换系数
	const int iRandDirectionNumber = 16;
	static float RadiusToXYFactor[iRandDirectionNumber][2] =
	{
		{0.707f,    0.707f},
		{0.924f,    0.383f},
		{1.0f,      0.0f},
		{0.924f,    -0.383f},
		{0.707f,    -0.707f},
		{0.383f,    -0.924f},
		{0.0f,      -1.0f},
		{-0.383f,   -0.924f},
		{-0.707f,   -0.707f},

		{-0.924f,   -0.383f},
		{-1.0f,     0.0f},
		{-0.924f,   0.383f},
		{-0.707f,   0.707f},
		{-0.383f,   0.924f},
		{0.0f,      1.0f},
		{0.383f,    0.924f},
	};

	float i_x = center->pos_x;
	float i_z = center->pos_z;

	// 先在指定方向上查看, 将8方向值转为16方向值
	int iStartDirection = iDirection * 2 - 2;
	if (iStartDirection < 0 || iStartDirection >= iRandDirectionNumber)
	{
		return false;
	}

	float iDeltaX = (radius * RadiusToXYFactor[iStartDirection][0]);
	float iDeltaZ = (radius * RadiusToXYFactor[iStartDirection][1]);

	ret_pos->pos_x = i_x + iDeltaX;
	ret_pos->pos_z = i_z + iDeltaZ;

	if (check_can_walk(scene, start, ret_pos))
		return true;

	// 然后依次查看剩余的方向
	static bool bClockWise = false;
	bClockWise = !bClockWise;

	int iSwapTimes = 0;
	int iDeltaDir = 1;
	int iMaxiDeltaDir = iRandDirectionNumber / 2;
	int iRandDirection;

	while (iDeltaDir <= iMaxiDeltaDir)
	{
		if (!bClockWise)
		{
			iRandDirection = (iStartDirection + iDeltaDir) % iRandDirectionNumber;
		}
		else
		{
			iRandDirection = iStartDirection - iDeltaDir;
			if (iRandDirection < 0)
			{
				iRandDirection = iRandDirectionNumber - iDeltaDir;
			}
		}

		iDeltaX = (radius * RadiusToXYFactor[iRandDirection][0]);
		iDeltaZ = (radius * RadiusToXYFactor[iRandDirection][1]);

		ret_pos->pos_x = i_x + iDeltaX;
		ret_pos->pos_z = i_z + iDeltaZ;

		if (check_can_walk(scene, start, ret_pos))
			return true;

		// 变换方向
		bClockWise = !bClockWise;

		// 扩展范围
		iSwapTimes++;
		if (iSwapTimes == 2)
		{
			iSwapTimes = 0;
			iDeltaDir++;
		}
	}
	return false;
}

float get_pos_height(const scene_struct *scene, const struct position *pos)
{
	struct map_block *b = get_map_block(scene->map_config, pos->pos_x, pos->pos_z);
	if (b)
	{
		return b->height;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
bool FindPathSlow(const TUnitPosition &rstStartPosition,
									 const TUnitPosition &rstEndPosition,
									 TUnitPath &rstPath)
{
	int iStartBlockX = rstStartPosition.m_uiX / MIN_SCENE_BLOCK_SIZE;
	int iStartBlockY = rstStartPosition.m_uiY / MIN_SCENE_BLOCK_SIZE;

	int iEndBlockX = rstEndPosition.m_uiX / MIN_SCENE_BLOCK_SIZE;
	int iEndBlockY = rstEndPosition.m_uiY / MIN_SCENE_BLOCK_SIZE;

	TSceneBlock *pstStartBlock = &m_astSceneBlock[iStartBlockY][iStartBlockX];
	TSceneBlock *pstEndBlock = &m_astSceneBlock[iEndBlockY][iEndBlockX];

	m_stPathMinHeap.Initialize();

	// 依次处理当前节点的周围4个节点, 直到到达最后节点
	TSceneBlock *pstCenterBlock = pstStartBlock;
	while (1)
	{
		// 将当前节点加入封闭列表
		pstCenterBlock->stAStar.bClosed = true;

		int iCenterX = BLOCK_X(pstCenterBlock);
		int iCenterY = BLOCK_Y(pstCenterBlock);

		// 到达终点所属的块
		if (pstCenterBlock->iFirstBlock == pstEndBlock->iFirstBlock)
		{
			break;
		}

		if (iCenterX - 1 >= 0)
		{
			AStarCountNode(iCenterX - 1, iCenterY, iEndBlockX, iEndBlockY, pstCenterBlock);
		}

		if (iCenterX + 1 < m_iWidthBlocks)
		{
			AStarCountNode(iCenterX + 1, iCenterY, iEndBlockX, iEndBlockY, pstCenterBlock);
		}

		if (iCenterY - 1 >= 0)
		{
			AStarCountNode(iCenterX, iCenterY - 1, iEndBlockX, iEndBlockY, pstCenterBlock);
		}

		if (iCenterY + 1 < m_iHeightBlocks)
		{
			AStarCountNode(iCenterX, iCenterY + 1, iEndBlockX, iEndBlockY, pstCenterBlock);
		}

		// 取开放列表中路径最小的作为当前节点
		pstCenterBlock = m_stPathMinHeap.PopHeap();
		if (pstCenterBlock == NULL)
		{
			LOGDEBUG("Cannot find valid path\n");

			pstStartBlock->stAStar.bClosed = false;
			return false;
		}
	}

	pstStartBlock->stAStar.bClosed = false;

	// A* 算法结束, 进行路径优化

	// 去掉终点块
	if (pstCenterBlock == pstEndBlock)
	{
		pstCenterBlock = pstCenterBlock->stAStar.pstCenterNode;
	}

	// 统计路径点个数
	g_iTempPathNumber = 0;
	TSceneBlock *pstPathBlock = pstCenterBlock;
	while (pstPathBlock && pstPathBlock != pstStartBlock)
	{
		g_iTempPathNumber++;
		pstPathBlock = pstPathBlock->stAStar.pstCenterNode;
	}

	if (g_iTempPathNumber >= MAX_TEMP_PATH_NUMBER - 1)
	{
		return false;
	}

	// 将路径点反向连起来
	pstPathBlock = pstCenterBlock;
	for (int i = g_iTempPathNumber - 1; i >= 0; i--)
	{
		int iCenterX = BLOCK_X(pstPathBlock) * MIN_SCENE_BLOCK_SIZE + MIN_SCENE_BLOCK_SIZE / 2;
		int iCenterY = BLOCK_Y(pstPathBlock) * MIN_SCENE_BLOCK_SIZE + MIN_SCENE_BLOCK_SIZE / 2;

		g_astTempPosition[i].m_uiX = iCenterX;
		g_astTempPosition[i].m_uiY = iCenterY;

		pstPathBlock = pstPathBlock->stAStar.pstCenterNode;
	}

	// 添加终点
	g_iTempPathNumber++;
	g_astTempPosition[g_iTempPathNumber - 1] = rstEndPosition;

	OptimizeAStarPath(rstStartPosition, rstPath);

	return true;
}

void CScenePathManager::AStarCountNode(int iX, int iY, int iEndX, int iEndY, TSceneBlock *pstCenterBlock)
{
	TSceneBlock *pstNeighborBlock = &m_astSceneBlock[iY][iX];

	// 此块不可行走，忽略
	if (pstNeighborBlock->iWalkable == 0)
	{
		return;
	}

	TAStarNode &stAStar = pstNeighborBlock->stAStar;

	// 已经加入封闭列表，忽略
	if (stAStar.bClosed)
	{
		return;
	}

	//省略G参数可以提高速度, 但得不到最优路径, 服务器做怪物短路径寻路不碍事
	int iValueG = pstCenterBlock->stAStar.iValueG + 1;

	// 在开放列表中，但路径比现在优先，忽略
	if (stAStar.bOpened && stAStar.iValueG < iValueG)
	{
		return;
	}

	// 更新当前节点GHF值，并指向新的中心节点
	stAStar.iValueG = iValueG;
	// stAStar.iValueH = ABS(iX, iEndX) + ABS(iY, iEndY);
	stAStar.iValueF = stAStar.iValueG + ABS(iX, iEndX) + ABS(iY, iEndY);

	stAStar.pstCenterNode = pstCenterBlock;

	// 加入开放列表
	if (!stAStar.bOpened)
	{
		m_stPathMinHeap.PushHeap(pstNeighborBlock);
		stAStar.bOpened = true;
	}

	return;
}

void CScenePathManager::OptimizeAStarPath(const TUnitPosition &rstStartPosition, TUnitPath &rstPath)
{
	int iSavedNumber = g_iTempPathNumber;

	// 优化起始点
	int iFirstPoint = 0;
	int iSecondPoint = iFirstPoint + 1;
	while (g_iTempPathNumber > 1 && CanWalk(rstStartPosition, g_astTempPosition[iSecondPoint]))
	{
		iFirstPoint = iSecondPoint;
		iSecondPoint = iFirstPoint + 1;
		g_iTempPathNumber--;
	}

	int iSavedFirstPoint = iFirstPoint;

	// 优化中间路径点
	int iThirdPoint;

	while (g_iTempPathNumber > 2)
	{
		iThirdPoint = iSecondPoint + 1;

		if (iThirdPoint >= iSavedNumber)
		{
			break;
		}

		if (CanWalk(g_astTempPosition[iFirstPoint], g_astTempPosition[iThirdPoint]))
		{
			g_astTempPosition[iSecondPoint].m_uiX = (unsigned int)-1;

			iSecondPoint = iThirdPoint;

			g_iTempPathNumber--;
			continue;
		}

		iFirstPoint = iSecondPoint;
		iSecondPoint = iFirstPoint + 1;
	}

	// 收集所有有效路径点
	int i = iSavedFirstPoint;
	rstPath.m_iNumber = 0;
	while (rstPath.m_iNumber < g_iTempPathNumber)
	{
		if (g_astTempPosition[i].m_uiX != (unsigned int)-1)
		{
			rstPath.m_astPosition[rstPath.m_iNumber++] = g_astTempPosition[i];
		}

		i++;
	}
}

void CScenePathManager::OptimizePath(const TUnitPosition &rstStartPosition, TUnitPath &rstPath)
{
	int iSavedNumber = rstPath.m_iNumber;

	// 优化路径的起始点
	// 如果当前位置点能直接到达第二个路径点, 则将第二个路径点设置为第一个路径点
	int iFirstPoint = 0;
	int iSecondPoint = iFirstPoint + 1;
	while (rstPath.m_iNumber > 1 && CanWalk(rstStartPosition, rstPath.m_astPosition[iSecondPoint]))
	{
		iFirstPoint = iSecondPoint;
		iSecondPoint = iFirstPoint + 1;
		rstPath.m_iNumber--;
	}

	int iSavedFirstPoint = iFirstPoint;

	// 优化中间路径点
	// 如果第一个点能直接到达第三个点,  则去除第二个点
	int iThirdPoint;
	int iFourthPoint;

	while (rstPath.m_iNumber > 2)
	{
		iThirdPoint = iSecondPoint + 1;
		iFourthPoint = iThirdPoint + 1;

		if (iThirdPoint >= iSavedNumber)
		{
			break;
		}

		if (iFourthPoint < iSavedNumber && CanWalk(
			rstPath.m_astPosition[iFirstPoint],
			rstPath.m_astPosition[iFourthPoint]))
		{
			rstPath.m_astPosition[iSecondPoint].m_uiX = (unsigned int)-1;
			rstPath.m_astPosition[iThirdPoint].m_uiX = (unsigned int)-1;

			iSecondPoint = iFourthPoint;

			rstPath.m_iNumber--;
			rstPath.m_iNumber--;
			continue;
		}

		if (CanWalk(rstPath.m_astPosition[iFirstPoint], rstPath.m_astPosition[iThirdPoint]))
		{
			rstPath.m_astPosition[iSecondPoint].m_uiX = (unsigned int)-1;

			iSecondPoint = iThirdPoint;

			rstPath.m_iNumber--;
			continue;
		}

		iFirstPoint = iSecondPoint;
		iSecondPoint = iFirstPoint + 1;
	}

	// 收集所有有效路径点, 无效的路径点X左边已经被置为-1
	int i = iSavedFirstPoint;
	int j = 0;
	while (j < rstPath.m_iNumber)
	{
		if (rstPath.m_astPosition[i].m_uiX != (unsigned int)-1)
		{
			rstPath.m_astPosition[j++] = rstPath.m_astPosition[i];
		}

		i++;
	}
}
#endif
