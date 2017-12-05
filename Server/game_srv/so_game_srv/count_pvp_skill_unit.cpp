#include "count_pvp_skill_unit.h"
#include "game_event.h"
#include "unit_path.h"
//#include "team.h"

static int count_rect_unit(struct position *my_pos, double angle, std::vector<unit_struct *> *ret, uint max, double length, double width, player_struct **team_player, int team_num)
{
	double cos = qFastCos(angle);
	double sin = qFastSin(angle);
//	LOG_DEBUG("jacktang sin = %.2f cos = %.2f", sin, cos);
//	double point_x1 = cos*(my_pos->pos_x)-sin*(my_pos->pos_z);
//	double point_z1 = cos*(my_pos->pos_z)+sin*(my_pos->pos_x);
	double x1, x2;
	double z1, z2;
//	x1 = point_x1;
//	x2 = point_x1 + length;
//	z1 = point_z1 - width;
//	z2 = point_z1 + width;

	x1 = 0;	
	x2 = x1 + length;
	z1 = -width;
	z2 = width;	

	for (int i = 0; i < team_num; ++i)	
//	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		player_struct *player = team_player[i];
		if (!player || !player->is_alive())
		{
			continue;
		}
		struct position *pos = player->get_pos();

		double pos_x = pos->pos_x - my_pos->pos_x;
		double pos_z = pos->pos_z - my_pos->pos_z;
		double target_x1 = cos*(pos_x)-sin*(pos_z);
		double target_z1 = cos*(pos_z)+sin*(pos_x);		
		
//		double target_x1 = cos*(pos->pos_x)-sin*(pos->pos_z);
//		double target_z1 = cos*(pos->pos_z)+sin*(pos->pos_x);

		if (target_x1 >= x1 && target_x1 <= x2 && target_z1 >= z1 && target_z1 <= z2)
		{
//			LOG_DEBUG("%s:  jacktang hit: angle[%.2f] x1[%.2f] x2[%.2f] x[%.2f] z1[%.2f] z2[%.2f] z[%.2f]",
//				__FUNCTION__, angle, x1, x2, target_x1, z1, z2, target_z1);					
			ret->push_back(player);
			if (ret->size() >= max)
				return (0);
		}
		else
		{
//			LOG_DEBUG("%s: jacktang miss: angle[%.2f] x1[%.2f] x2[%.2f] x[%.2f] z1[%.2f] z2[%.2f] z[%.2f]",
//				__FUNCTION__, angle, x1, x2, target_x1, z1, z2, target_z1);		
		}
	}	
	return (0);
}
static int count_circle_unit(struct position *target_pos, std::vector<unit_struct *> *ret, uint max, double radius, player_struct **team_player, int team_num)
{
	radius = radius * radius;
	for (int i = 0; i < team_num; ++i)
	{
		player_struct *player = team_player[i];
		if (!player || !player->is_alive())
		{
			continue;
		}
		double x = target_pos->pos_x - player->get_pos()->pos_x;
		double z = target_pos->pos_z - player->get_pos()->pos_z;
		if (x * x + z * z > radius)
			continue;
		ret->push_back(player);
		if (ret->size() >= max)
			return (0);		
	}
	return (0);	
}
static int count_fan_unit(struct position *my_pos, std::vector<unit_struct *> *ret, uint max, double radius, double angle, player_struct **team_player, int team_num)
{
	radius = radius * radius;
	double my_angle = pos_to_angle(my_pos->pos_x, my_pos->pos_z);
	double angle_min = my_angle - angle;
	double angle_max = my_angle + angle;
	
	for (int i = 0; i < team_num; ++i)
	{
		player_struct *player = team_player[i];
		if (!player || !player->is_alive())
		{
			continue;
		}
		double x = my_pos->pos_x - player->get_pos()->pos_x;
		double z = my_pos->pos_z - player->get_pos()->pos_z;
		if (x * x + z * z > radius)
			continue;
		double angle_target = pos_to_angle(player->get_pos()->pos_x, player->get_pos()->pos_z);
		if (angle_target >= angle_min && angle_target <= angle_max)
		{
			ret->push_back(player);
			if (ret->size() >= max)
				return (0);
		}
	}
	return (0);	
}

int count_skill_hit_unit(player_struct *player, struct position *target_pos, double angle, uint32_t skill_id, std::vector<unit_struct *> *ret, player_struct **team_player, int team_num)
{
	struct SkillTable *_config = get_config_by_id(skill_id, &skill_config);
	if (!_config)
		return (-1);

	if (!team_player)
	{
		switch (_config->RangeType)
		{
			case SKILL_RANGE_TYPE_RECT:
				return player->count_rect_unit(angle, ret, _config->MaxCount, _config->Radius, _config->Angle, false);
			case SKILL_RANGE_TYPE_CIRCLE:
				return player->count_circle_unit(ret, _config->MaxCount, target_pos, _config->Radius, false);			
			case SKILL_RANGE_TYPE_FAN:
				return player->count_fan_unit(ret, _config->MaxCount, _config->Radius, _config->Angle, false);
			default:
				return -10;
		}
		return (0);
	}
	
	switch (_config->RangeType)
	{
		case SKILL_RANGE_TYPE_RECT:
			return count_rect_unit(player->get_pos(), angle, ret, _config->MaxCount, _config->Radius, _config->Angle, team_player, team_num);
		case SKILL_RANGE_TYPE_CIRCLE:
			return count_circle_unit(target_pos, ret, _config->MaxCount, _config->Radius, team_player, team_num);			
		case SKILL_RANGE_TYPE_FAN:
			return count_fan_unit(player->get_pos(), ret, _config->MaxCount, _config->Radius, _config->Angle, team_player, team_num);
		default:
			return -10;
	}
	
	return (0);
}

