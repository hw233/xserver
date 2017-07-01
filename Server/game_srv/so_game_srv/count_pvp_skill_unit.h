#ifndef COUNT_SKILL_UNIT_H
#define COUNT_SKILL_UNIT_H

#include <vector>
#include <stdint.h>
#include "unit.h"
#include "player.h"
#include "excel_data.h"
#include "game_config.h"

int count_skill_hit_unit(struct position *my_pos, double angle, uint32_t skill_id, std::vector<unit_struct *> *ret, player_struct **team_player);

#endif /* COUNT_SKILL_UNIT_H */
