#include <string.h>
#include "install_partner_ai.h"
//#include "partner_ai.h"
#include "partner.h"
#include "global_param.h"

// 1重义：
// 2争胜：
// 3果敢：
// 4冷静：
// 5谨慎：
// 6护主：
// 7狡诈：
// 8精明：
// 9稳重：
// 10温婉：
// 11不屈：
// 12威严：

extern struct partner_ai_interface partner_ai_1_interface;
extern struct partner_ai_interface partner_ai_2_interface;
extern struct partner_ai_interface partner_ai_3_interface;
extern struct partner_ai_interface partner_ai_4_interface;
extern struct partner_ai_interface partner_ai_9_interface;

void install_partner_ai()
{
	partner_struct::add_ai_interface(1, &partner_ai_1_interface);
	partner_struct::add_ai_interface(2, &partner_ai_2_interface);
	partner_struct::add_ai_interface(3, &partner_ai_3_interface);
	partner_struct::add_ai_interface(4, &partner_ai_4_interface);
	// partner_struct::add_ai_interface(5, &partner_ai_5_interface);
	// partner_struct::add_ai_interface(6, &partner_ai_6_interface);
	// partner_struct::add_ai_interface(7, &partner_ai_7_interface);
	// partner_struct::add_ai_interface(8, &partner_ai_8_interface);
	partner_struct::add_ai_interface(9, &partner_ai_9_interface);
	// partner_struct::add_ai_interface(10, &partner_ai_10_interface);
	// partner_struct::add_ai_interface(11, &partner_ai_11_interface);
	// partner_struct::add_ai_interface(12, &partner_ai_12_interface);							
}

void uninstall_partner_ai()
{
	for (int i = 0; i < MAX_PARTNER_AI_INTERFACE; ++i)
		partner_struct::add_ai_interface(i, NULL);
}
