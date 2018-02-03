#include <string.h>
#include "global_param.h"
#include "install_monster_ai.h"
#include "monster_ai.h"
#include "monster.h"

// 0=6
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
// 23=优先攻击护送怪的AI
// 24=定时刷怪的AI 配置在GenerateMonster表里面
// 25=定时刷出的怪, 先走到指定坐标，然后再执行自己的AI
// 26=阵营战守卫
// 27=猫鬼乐园-猫鬼雕像
// 28=猫鬼乐园-可爱猫鬼
// 29=猫鬼乐园-恐惧猫鬼
// 30=阵营战镖车
// 31=猫鬼乐园召唤怪首领
// 32=60%血的时候休息一下，说句话
// 33=跟随玩家
// 34=阵营战镖车护送怪
void install_monster_ai()
{
	monster_struct::add_ai_interface(AI_TYPE_NORMAL, &monster_ai_normal_interface);
	monster_struct::add_ai_interface(AI_TYPE_CIRCLE, &monster_ai_circle_interface);
	monster_struct::add_ai_interface(3, &monster_ai_3_interface);	
	monster_struct::add_ai_interface(6, &monster_ai_0_interface);
	monster_struct::add_ai_interface(4, &monster_ai_4_interface);
	monster_struct::add_ai_interface(5, &monster_ai_5_interface);
//	monster_struct::add_ai_interface(7, &monster_ai_7_interface);
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
	monster_struct::add_ai_interface(23, &monster_ai_23_interface);
	monster_struct::add_ai_interface(24, &monster_ai_24_interface);
	monster_struct::add_ai_interface(25, &monster_ai_25_interface);
	monster_struct::add_ai_interface(26, &monster_ai_26_interface);					
	monster_struct::add_ai_interface(27, &monster_ai_27_interface);
	monster_struct::add_ai_interface(28, &monster_ai_28_interface);
	monster_struct::add_ai_interface(29, &monster_ai_29_interface);		
	monster_struct::add_ai_interface(30, &monster_ai_30_interface);
	monster_struct::add_ai_interface(31, &monster_ai_31_interface);
	monster_struct::add_ai_interface(32, &monster_ai_32_interface);
	monster_struct::add_ai_interface(33, &monster_ai_33_interface);
	monster_struct::add_ai_interface(34, &monster_ai_34_interface);
}
void uninstall_monster_ai()
{
	for (int i = 0; i < MAX_MONSTER_AI_INTERFACE; ++i)
	{
		monster_struct::add_ai_interface(i, NULL);		
	}
}
