#!/bin/awk
{
	t_index = t_index + 1
	printf("	%s %d : %s\n", $1, t_index, $4)
}
