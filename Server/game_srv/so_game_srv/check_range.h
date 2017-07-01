#ifndef CHECK_RANGE_H
#define CHECK_RANGE_H

#include "unit_path.h"

bool check_distance_in_range(struct position *pos1, struct position *pos2, double range);
bool check_circle_in_range(struct position *pos1, struct position *pos2, double range);

#endif /* CHECK_RANGE_H */
