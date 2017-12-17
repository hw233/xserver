#!/bin/awk
{
	if (NF == 4)
	{
		t_index = t_index + 1
		printf("	%s %d : %s\n", $1, t_index, $4)
	}
	else
	{
		printf("%s\n", $0)
	}
}
