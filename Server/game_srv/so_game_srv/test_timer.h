#ifndef TEST_TIMER_H
#define TEST_TIMER_H

typedef void (*timer_func)();

extern timer_func all_test_timer10_funcs[1024];
extern timer_func all_test_timer100_funcs[1024];	
extern int cur_test_timer10_funcs;
extern int cur_test_timer100_funcs;	

void test_run_timer100();
void test_run_timer10();

#endif /* TEST_TIMER_H */
