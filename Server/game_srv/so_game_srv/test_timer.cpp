#include "test_timer.h"

timer_func all_test_timer10_funcs[1024];
timer_func all_test_timer100_funcs[1024];	
int cur_test_timer10_funcs;
int cur_test_timer100_funcs;	

void test_run_timer10()
{
	for (int i = 0; i < cur_test_timer10_funcs; ++i)
	{
		if (all_test_timer10_funcs[i])
			all_test_timer10_funcs[i]();
	}
}

void test_run_timer100()
{
	for (int i = 0; i < cur_test_timer100_funcs; ++i)
	{
		if (all_test_timer100_funcs[i])
			all_test_timer100_funcs[i]();
	}
}
