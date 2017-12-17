#!/bin/awk
{
	if (NF == 5)
	{
		t_index = t_index + 1
		printf("	%s %s %s = %d:\n", $1, $2, $3, t_index)
	}
	else
	{
		printf("%s\n", $0)
	}
}
