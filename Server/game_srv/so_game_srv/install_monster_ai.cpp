#include <string.h>
#include "install_monster_ai.h"
#include "monster_ai.h"
#include "monster.h"

// 0=不属于任何类型
// 1=随机巡逻
// 2=固定点巡逻
// 3=木桩类型
// 4=陷阱
// 5=任务召唤怪物，其他人不可见
// 6=箭塔
// 7=伙伴，预留
// 8=飞箭陷阱
// 9=火焰陷阱
// 10=墓穴副本僵尸头目
// 11=侍神拔小怪（不死，昏迷）
// 12=侍神拔首领
// 13=墓穴副本最终boss
// 14=护送任务
// 15=僵尸头目召唤的小怪
// 16=优先攻击怪物的AI
// 17=战斗结束后跟随玩家
// 18=秋戈的特殊AI
// 19=太极BOS  AI
// 20=妖帝AI
// 21=蒋雳夫等4人AI
// 22=定点护送，可在路途中自动攻击对立方

void install_monster_ai()
{
	monster_struct::add_ai_interface(AI_TYPE_NORMAL, &monster_ai_normal_interface);
	monster_struct::add_ai_interface(AI_TYPE_CIRCLE, &monster_ai_circle_interface);
	monster_struct::add_ai_interface(6, &monster_ai_0_interface);
	monster_struct::add_ai_interface(4, &monster_ai_4_interface);
	monster_struct::add_ai_interface(5, &monster_ai_5_interface);
	monster_struct::add_ai_interface(7, &monster_ai_7_interface);
	monster_struct::add_ai_interface(8, &monster_ai_8_interface);
	monster_struct::add_ai_interface(10, &monster_ai_10_interface);
	monster_struct::add_ai_interface(11, &monster_ai_11_interface);
	monster_struct::add_ai_interface(12, &monster_ai_12_interface);
	monster_struct::add_ai_interface(13, &monster_ai_13_interface);
	monster_struct::add_ai_interface(14, &monster_ai_14_interface);
	monster_struct::add_ai_interface(15, &monster_ai_15_interface);
	monster_struct::add_ai_interface(16, &monster_ai_16_interface);
	monster_struct::add_ai_interface(17, &monster_ai_17_interface);
	monster_struct::add_ai_interface(18, &monster_ai_18_interface);
	monster_struct::add_ai_interface(19, &monster_ai_19_interface);
	monster_struct::add_ai_interface(20, &monster_ai_20_interface);
	monster_struct::add_ai_interface(21, &monster_ai_21_interface);		
	monster_struct::add_ai_interface(22, &monster_ai_22_interface);		
}
void uninstall_monster_ai()
{
	monster_struct::add_ai_interface(AI_TYPE_NORMAL, NULL);
	monster_struct::add_ai_interface(AI_TYPE_CIRCLE, NULL);
	monster_struct::add_ai_interface(6, NULL);
	monster_struct::add_ai_interface(4, NULL);
	monster_struct::add_ai_interface(5, NULL);
	monster_struct::add_ai_interface(7, NULL);
	monster_struct::add_ai_interface(8, NULL);
	monster_struct::add_ai_interface(10, NULL);
	monster_struct::add_ai_interface(11, NULL);
	monster_struct::add_ai_interface(12, NULL);
	monster_struct::add_ai_interface(13, NULL);
	monster_struct::add_ai_interface(14, NULL);
	monster_struct::add_ai_interface(15, NULL);
	monster_struct::add_ai_interface(16, NULL);
	monster_struct::add_ai_interface(17, NULL);
	monster_struct::add_ai_interface(18, NULL);
	monster_struct::add_ai_interface(19, NULL);
	monster_struct::add_ai_interface(20, NULL);
	monster_struct::add_ai_interface(21, NULL);	
	monster_struct::add_ai_interface(22, NULL);	
}
