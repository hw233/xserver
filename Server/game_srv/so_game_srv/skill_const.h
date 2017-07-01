#ifndef SKILL_CONST_H
#define SKILL_CONST_H

//释放开始的时候，检查各种条件是否合法，合法则进入ESP_PREPARING状态
//经过吟唱后进入ESP_CASTING状态
//经过飞行，进入ESP_EFFECT状态
//计算法术效果，施法结束

//法术施放状态类型
enum SpellStatEvent
{
	ESP_NONE = -1,
	ESP_PREPARING,  //准备
	ESP_CASTING,       //施放
	ESP_EFFECT,          //效果
	ESP_MAX
};

// 法术施放结果数据
enum SpellCastResult
{
	ESR_NONE = -3,
	ESR_RELAY = -2,   // 施法定时处理
	ESR_ERR = -1,       // 施法失败
	ESR_SUCESS = 0,        //成功
	ESR_FARAWAY = 1,         //太远了
	ESR_COOLDOWN = 2,      //冷却中
	ESR_OUTOFCAST = 3,      //消耗不足
	ESR_INVAILDID = 4,        //无效ID
	ESR_TARGETERROR = 5,  //目标错误
	ESR_STATELOCKED = 6,  //状态锁定
	ESR_ISDOING = 7,           //正在做
	ESR_NOLEARNED = 8,      //没有掌握该技能
	ESR_CANCELOK = 9,        //取消施法成功
	ESR_CANCELERR  = 10,      //取消施法失败
	ESR_WEAPONNOMATCH = 11, // 武器不匹配
	ESR_CONJUREREPEAT = 12, // 召唤Npc位置重叠
	ESR_CASTERERROR = 13,   //释放者类型与ID定义类型不匹配
	ESR_NOTRIDINGSKL = 14, // 在坐骑上，不能使用该技能
	ESR_NOTGROUNDSKL = 15, // 不在坐骑上，不能使用该技能

	ESR_LOWLEVELERROR ,         //在本地图，无法攻击等级低于50级的玩家
	ESR_PKERROR,  // pk模式不对，无法攻击
	ESR_TOWERERROR, // 国家箭塔不能被攻击
	ESR_SAFEAREAERROR,              //安全区域，无法进行PK行为
	ESR_NEWHANDPROTECTEDERROR,  // 攻击者或被攻击目标处于新手保护阶段（30级），无法进行攻击
	ESR_TARGETNOTSEE, // 施法者与目标中间有障碍
	ESR_MAX
};

// 法术施放的结果类型
enum SpellResultType
{
	ESRT_NONE = -1,
	ESRT_HIT, // 命中
	ESRT_BLOW,//重击
	ESRT_CRIT, // 暴击
	ESRT_DODGE, // 躲闪
	ESRT_IMMUNE, // 免疫
	ESRT_IMBIBE, // 吸收
	ESRT_IGNORE,//忽视
	ESRT_REFLECT,//反射
	ESRT_MISS,//未命中
	ESRT_MAX
};

//法术作用目标类型
enum SpellTargetType
{
	ESTT_NONE = -1,
	ESTT_ENEMY,             //敌人
	ESTT_OWNER,            //施法者本身
	ESTT_PAL,                  //小队队友
	ESTT_RAIDPLAYER,  //团队成员
	ESTT_FRIEND,           // 友方玩家
	ESTT_NPC,                  //NPC
	ESTT_FAMILY,            //家族
	ESTT_GULID,              //公会成员
	ESTT_COUNTRY,           //国家
	ESTT_GULIDLEADER,   //帮主
	    //ESTT_LANSQUENET,   //雇佣兵
	    //ESTT_VEHICLE,          // 骑乘
	ESTT_MAX
};



#endif /* SKILL_CONST_H */
