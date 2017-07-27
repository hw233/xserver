local DungeonTable = {
	[20001] = {
		['DungeonID'] = 20001,	--副本id
		['DengeonType'] = 2,	--副本组队类型
		['DengeonRank'] = 0,	--副本类型
		['RandomGrade'] = 0,	--随机主副本难度
		['CloseUI'] = 2,	--是否隐藏UI
		['StoryType'] = 1,	--剧情类型
		['RandomID'] = {0},	--随机关卡ID
		['RandomNum'] = 0,	--随机关卡数量
		['TaskID'] = 0,	--副本任务ID
		['AddMidway'] = 1,	--是否允许中途组人
		['InstantRelive'] = 1,	--是否允许原地复活
		['AutomaticRelive'] = 0,	--是否自动复活
		['ReliveTime'] = {5,0,5},	--副本系统复活CD控制
		['ActivityControl'] = 330400001,	--控制表ID
		['CostItemID'] = 0,	--副本进入消耗道具ID
		['CostNum'] = 0,	--消耗条件数量
		['Score'] = {1,2,2},	--通关评分标准
		['ScoreValue'] = {120,151000001,151000002},	--星星评分值
		['ScoreValue1'] = {0,3,2},	--星星评分值
		['Rewards'] = {220100012,220100011,220100221},	--副本通关奖励ID
		['DungeonPass'] = '',	--副本AI进程
		['DungeonTime'] = 120,	--副本限时
		['PassType'] = {1,1,1},	--通关类型
		['PassValue'] = {151000001,151000002,151000004},	--通关数值
		['PassValue1'] = {3,2,1},	--通关数值1
		['ExitScene'] = 10000,	--退出副本返回场景
		['ExitPointX'] = 71.00,	--退出副本横坐标
		['BirthPointY'] = 58.00,	--退出副本高度坐标
		['BirthPointZ'] = 98.00,	--退出副本纵坐标
		['FaceY'] = 180.00,	--退出副本朝向
		['StartTxt'] = '',	--开始字幕
		['EndTxt'] = '',	--结束字幕
		['RewardPosition'] = {0}	--奖励刷新坐标
		},
	[20002] = {
		['DungeonID'] = 20002,
		['DengeonType'] = 2,
		['DengeonRank'] = 0,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400002,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1,1,1},
		['ScoreValue'] = {32,60,30},
		['ScoreValue1'] = {0,0,0},
		['Rewards'] = {220100013,220100011,220100221},
		['DungeonPass'] = '',
		['DungeonTime'] = 32,
		['PassType'] = {1},
		['PassValue'] = {151000004},
		['PassValue1'] = {1},
		['ExitScene'] = 10000,
		['ExitPointX'] = 71.00,
		['BirthPointY'] = 58.00,
		['BirthPointZ'] = 98.00,
		['FaceY'] = 180.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20003] = {
		['DungeonID'] = 20003,
		['DengeonType'] = 1,
		['DengeonRank'] = 0,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400003,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1,1,1},
		['ScoreValue'] = {90,60,30},
		['ScoreValue1'] = {0,0,0},
		['Rewards'] = {220100014,220100222,220100221},
		['DungeonPass'] = '',
		['DungeonTime'] = 90,
		['PassType'] = {1},
		['PassValue'] = {151000004},
		['PassValue1'] = {1},
		['ExitScene'] = 10000,
		['ExitPointX'] = 71.00,
		['BirthPointY'] = 58.00,
		['BirthPointZ'] = 98.00,
		['FaceY'] = 180.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20004] = {
		['DungeonID'] = 20004,
		['DengeonType'] = 1,
		['DengeonRank'] = 0,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400004,
		['CostItemID'] = 201060014,
		['CostNum'] = 1,
		['Score'] = {1,1,1},
		['ScoreValue'] = {90,60,30},
		['ScoreValue1'] = {0,0,0},
		['Rewards'] = {220100015,220100223,220100221},
		['DungeonPass'] = '',
		['DungeonTime'] = 90,
		['PassType'] = {1},
		['PassValue'] = {151000004},
		['PassValue1'] = {1},
		['ExitScene'] = 10000,
		['ExitPointX'] = 71.00,
		['BirthPointY'] = 58.00,
		['BirthPointZ'] = 98.00,
		['FaceY'] = 180.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20005] = {
		['DungeonID'] = 20005,
		['DengeonType'] = 1,
		['DengeonRank'] = 0,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400005,
		['CostItemID'] = 201060015,
		['CostNum'] = 1,
		['Score'] = {1,1,1},
		['ScoreValue'] = {90,60,30},
		['ScoreValue1'] = {0,0,0},
		['Rewards'] = {220100016,220100224,220100221},
		['DungeonPass'] = '',
		['DungeonTime'] = 90,
		['PassType'] = {1},
		['PassValue'] = {151000004},
		['PassValue1'] = {1},
		['ExitScene'] = 10000,
		['ExitPointX'] = 71.00,
		['BirthPointY'] = 58.00,
		['BirthPointZ'] = 98.00,
		['FaceY'] = 180.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20006] = {
		['DungeonID'] = 20006,
		['DengeonType'] = 2,
		['DengeonRank'] = 0,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400006,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1,1,1},
		['ScoreValue'] = {90,60,30},
		['ScoreValue1'] = {0,0,0},
		['Rewards'] = {220100012,220100011,220100221},
		['DungeonPass'] = '',
		['DungeonTime'] = 90,
		['PassType'] = {1},
		['PassValue'] = {151000004},
		['PassValue1'] = {1},
		['ExitScene'] = 10000,
		['ExitPointX'] = 71.00,
		['BirthPointY'] = 58.00,
		['BirthPointZ'] = 98.00,
		['FaceY'] = 180.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20007] = {
		['DungeonID'] = 20007,
		['DengeonType'] = 1,
		['DengeonRank'] = 0,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400007,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1,1,1},
		['ScoreValue'] = {90,60,30},
		['ScoreValue1'] = {0,0,0},
		['Rewards'] = {220100012,220100011,220100221},
		['DungeonPass'] = '',
		['DungeonTime'] = 90,
		['PassType'] = {1},
		['PassValue'] = {151000004},
		['PassValue1'] = {1},
		['ExitScene'] = 10000,
		['ExitPointX'] = 71.00,
		['BirthPointY'] = 58.00,
		['BirthPointZ'] = 98.00,
		['FaceY'] = 180.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20008] = {
		['DungeonID'] = 20008,
		['DengeonType'] = 1,
		['DengeonRank'] = 8,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 240190002,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400008,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1,1,1},
		['ScoreValue'] = {6000,180,120},
		['ScoreValue1'] = {0,0,0},
		['Rewards'] = {220100012,220100011,220100221},
		['DungeonPass'] = 'P20001Table',
		['DungeonTime'] = 6000,
		['PassType'] = {1},
		['PassValue'] = {151000004},
		['PassValue1'] = {1},
		['ExitScene'] = 10006,
		['ExitPointX'] = 51.00,
		['BirthPointY'] = 41.00,
		['BirthPointZ'] = 195.00,
		['FaceY'] = -90.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20009] = {
		['DungeonID'] = 20009,
		['DengeonType'] = 2,
		['DengeonRank'] = 0,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400009,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1,1,1},
		['ScoreValue'] = {90,60,30},
		['ScoreValue1'] = {0,0,0},
		['Rewards'] = {220100012,220100011,220100221},
		['DungeonPass'] = '',
		['DungeonTime'] = 90,
		['PassType'] = {1},
		['PassValue'] = {151000004},
		['PassValue1'] = {1},
		['ExitScene'] = 10000,
		['ExitPointX'] = 71.00,
		['BirthPointY'] = 58.00,
		['BirthPointZ'] = 98.00,
		['FaceY'] = 180.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20010] = {
		['DungeonID'] = 20010,
		['DengeonType'] = 2,
		['DengeonRank'] = 0,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400010,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1,1,1},
		['ScoreValue'] = {90,60,30},
		['ScoreValue1'] = {0,0,0},
		['Rewards'] = {220100012,220100011,220100221},
		['DungeonPass'] = '',
		['DungeonTime'] = 90,
		['PassType'] = {1},
		['PassValue'] = {151000004},
		['PassValue1'] = {1},
		['ExitScene'] = 10000,
		['ExitPointX'] = 71.00,
		['BirthPointY'] = 58.00,
		['BirthPointZ'] = 98.00,
		['FaceY'] = 180.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20011] = {
		['DungeonID'] = 20011,
		['DengeonType'] = 2,
		['DengeonRank'] = 0,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400011,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1,1,1},
		['ScoreValue'] = {90,60,30},
		['ScoreValue1'] = {0,0,0},
		['Rewards'] = {220100012,220100011,220100221},
		['DungeonPass'] = '',
		['DungeonTime'] = 90,
		['PassType'] = {1},
		['PassValue'] = {151000004},
		['PassValue1'] = {1},
		['ExitScene'] = 10000,
		['ExitPointX'] = 71.00,
		['BirthPointY'] = 58.00,
		['BirthPointZ'] = 98.00,
		['FaceY'] = 180.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20012] = {
		['DungeonID'] = 20012,
		['DengeonType'] = 2,
		['DengeonRank'] = 0,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400012,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1,1,1},
		['ScoreValue'] = {90,60,30},
		['ScoreValue1'] = {0,0,0},
		['Rewards'] = {220100012,220100011,220100221},
		['DungeonPass'] = '',
		['DungeonTime'] = 90,
		['PassType'] = {1},
		['PassValue'] = {151000004},
		['PassValue1'] = {1},
		['ExitScene'] = 10000,
		['ExitPointX'] = 71.00,
		['BirthPointY'] = 58.00,
		['BirthPointZ'] = 98.00,
		['FaceY'] = 180.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20013] = {
		['DungeonID'] = 20013,
		['DengeonType'] = 1,
		['DengeonRank'] = 0,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400013,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1,1,1},
		['ScoreValue'] = {15,10,5},
		['ScoreValue1'] = {0,0,0},
		['Rewards'] = {220100012,220100011,220100221},
		['DungeonPass'] = '',
		['DungeonTime'] = 15,
		['PassType'] = {1},
		['PassValue'] = {151000004},
		['PassValue1'] = {1},
		['ExitScene'] = 10000,
		['ExitPointX'] = 71.00,
		['BirthPointY'] = 58.00,
		['BirthPointZ'] = 98.00,
		['FaceY'] = 180.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20014] = {
		['DungeonID'] = 20014,
		['DengeonType'] = 1,
		['DengeonRank'] = 0,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400014,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1,1,1},
		['ScoreValue'] = {300,180,120},
		['ScoreValue1'] = {0,0,0},
		['Rewards'] = {220100012,220100011,220100221},
		['DungeonPass'] = '',
		['DungeonTime'] = 300,
		['PassType'] = {1},
		['PassValue'] = {151000004},
		['PassValue1'] = {1},
		['ExitScene'] = 10011,
		['ExitPointX'] = 67.00,
		['BirthPointY'] = 14.00,
		['BirthPointZ'] = 22.00,
		['FaceY'] = -30.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20015] = {
		['DungeonID'] = 20015,
		['DengeonType'] = 1,
		['DengeonRank'] = 3,
		['RandomGrade'] = 1,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {20018,20019,20020},
		['RandomNum'] = 3,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400015,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {},
		['ScoreValue'] = {},
		['ScoreValue1'] = {},
		['Rewards'] = {220100012,220100011,220100221},
		['DungeonPass'] = '',
		['DungeonTime'] = 0,
		['PassType'] = {0},
		['PassValue'] = {0},
		['PassValue1'] = {0},
		['ExitScene'] = 10004,
		['ExitPointX'] = 70.00,
		['BirthPointY'] = 21.00,
		['BirthPointZ'] = 120.00,
		['FaceY'] = 25.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20016] = {
		['DungeonID'] = 20016,
		['DengeonType'] = 1,
		['DengeonRank'] = 3,
		['RandomGrade'] = 2,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {20038,20039,20020},
		['RandomNum'] = 3,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400016,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {},
		['ScoreValue'] = {},
		['ScoreValue1'] = {},
		['Rewards'] = {220100012,220100011,220100221},
		['DungeonPass'] = '',
		['DungeonTime'] = 0,
		['PassType'] = {0},
		['PassValue'] = {0},
		['PassValue1'] = {0},
		['ExitScene'] = 10004,
		['ExitPointX'] = 70.00,
		['BirthPointY'] = 21.00,
		['BirthPointZ'] = 120.00,
		['FaceY'] = 25.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20017] = {
		['DungeonID'] = 20017,
		['DengeonType'] = 1,
		['DengeonRank'] = 3,
		['RandomGrade'] = 3,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {20018,20019,20020},
		['RandomNum'] = 3,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400017,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {},
		['ScoreValue'] = {},
		['ScoreValue1'] = {},
		['Rewards'] = {220100012,220100011,220100221},
		['DungeonPass'] = '',
		['DungeonTime'] = 0,
		['PassType'] = {0},
		['PassValue'] = {0},
		['PassValue1'] = {0},
		['ExitScene'] = 10004,
		['ExitPointX'] = 70.00,
		['BirthPointY'] = 21.00,
		['BirthPointZ'] = 120.00,
		['FaceY'] = 25.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20018] = {
		['DungeonID'] = 20018,
		['DengeonType'] = 1,
		['DengeonRank'] = 4,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 0,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {300},
		['ScoreValue1'] = {0},
		['Rewards'] = {},
		['DungeonPass'] = 'P20018Table',
		['DungeonTime'] = 300,
		['PassType'] = {3},
		['PassValue'] = {154000011},
		['PassValue1'] = {30},
		['ExitScene'] = 10004,
		['ExitPointX'] = 70.00,
		['BirthPointY'] = 21.00,
		['BirthPointZ'] = 120.00,
		['FaceY'] = 25.00,
		['StartTxt'] = '在指定时间内挖到足够的矿石，注意矿石守卫',
		['EndTxt'] = '成功夺回矿场',
		['RewardPosition'] = {82.98,17.92,101.78}
		},
	[20019] = {
		['DungeonID'] = 20019,
		['DengeonType'] = 1,
		['DengeonRank'] = 4,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 0,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {500},
		['ScoreValue1'] = {0},
		['Rewards'] = {},
		['DungeonPass'] = 'P20019Table',
		['DungeonTime'] = 500,
		['PassType'] = {0},
		['PassValue'] = {0},
		['PassValue1'] = {0},
		['ExitScene'] = 10004,
		['ExitPointX'] = 70.00,
		['BirthPointY'] = 21.00,
		['BirthPointZ'] = 120.00,
		['FaceY'] = 25.00,
		['StartTxt'] = '在指定时间内护送大富到达安全区，快点，千万别丢了这笔生意',
		['EndTxt'] = '大富哥慢走，大富哥欢迎下次再请我们做保镖',
		['RewardPosition'] = {308.51,23.31,96.152}
		},
	[20020] = {
		['DungeonID'] = 20020,
		['DengeonType'] = 1,
		['DengeonRank'] = 4,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 0,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {300},
		['ScoreValue1'] = {0},
		['Rewards'] = {},
		['DungeonPass'] = 'P20020Table',
		['DungeonTime'] = 300,
		['PassType'] = {1,1},
		['PassValue'] = {151005026,151005027},
		['PassValue1'] = {9,1},
		['ExitScene'] = 10004,
		['ExitPointX'] = 70.00,
		['BirthPointY'] = 21.00,
		['BirthPointZ'] = 120.00,
		['FaceY'] = 25.00,
		['StartTxt'] = '在指定时间内除掉蜂后，注意她身边的蜂王',
		['EndTxt'] = '终于把蛊惑人心的蜂后击杀了',
		['RewardPosition'] = {81.37,4.29,70.72}
		},
	[20021] = {
		['DungeonID'] = 20021,
		['DengeonType'] = 1,
		['DengeonRank'] = 5,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 0,
		['AutomaticRelive'] = 1,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400018,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {300},
		['ScoreValue1'] = {0},
		['Rewards'] = {0},
		['DungeonPass'] = '',
		['DungeonTime'] = 300,
		['PassType'] = {4},
		['PassValue'] = {5},
		['PassValue1'] = {0},
		['ExitScene'] = 10004,
		['ExitPointX'] = 70.00,
		['BirthPointY'] = 21.00,
		['BirthPointZ'] = 120.00,
		['FaceY'] = 25.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20022] = {
		['DungeonID'] = 20022,
		['DengeonType'] = 1,
		['DengeonRank'] = 6,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 0,
		['AutomaticRelive'] = 1,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400019,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {300},
		['ScoreValue1'] = {0},
		['Rewards'] = {0},
		['DungeonPass'] = '',
		['DungeonTime'] = 300,
		['PassType'] = {4},
		['PassValue'] = {5},
		['PassValue1'] = {0},
		['ExitScene'] = 10004,
		['ExitPointX'] = 70.00,
		['BirthPointY'] = 21.00,
		['BirthPointZ'] = 120.00,
		['FaceY'] = 25.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20023] = {
		['DungeonID'] = 20023,
		['DengeonType'] = 1,
		['DengeonRank'] = 7,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400020,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {300},
		['ScoreValue1'] = {0},
		['Rewards'] = {0},
		['DungeonPass'] = '',
		['DungeonTime'] = 300,
		['PassType'] = {1},
		['PassValue'] = {0},
		['PassValue1'] = {1},
		['ExitScene'] = 10004,
		['ExitPointX'] = 70.00,
		['BirthPointY'] = 21.00,
		['BirthPointZ'] = 120.00,
		['FaceY'] = 25.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20024] = {
		['DungeonID'] = 20024,
		['DengeonType'] = 1,
		['DengeonRank'] = 7,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400021,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {300},
		['ScoreValue1'] = {0},
		['Rewards'] = {0},
		['DungeonPass'] = '',
		['DungeonTime'] = 300,
		['PassType'] = {1},
		['PassValue'] = {0},
		['PassValue1'] = {1},
		['ExitScene'] = 10004,
		['ExitPointX'] = 70.00,
		['BirthPointY'] = 21.00,
		['BirthPointZ'] = 120.00,
		['FaceY'] = 25.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20025] = {
		['DungeonID'] = 20025,
		['DengeonType'] = 1,
		['DengeonRank'] = 7,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400022,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {300},
		['ScoreValue1'] = {0},
		['Rewards'] = {0},
		['DungeonPass'] = '',
		['DungeonTime'] = 300,
		['PassType'] = {1},
		['PassValue'] = {0},
		['PassValue1'] = {1},
		['ExitScene'] = 10004,
		['ExitPointX'] = 70.00,
		['BirthPointY'] = 21.00,
		['BirthPointZ'] = 120.00,
		['FaceY'] = 25.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20026] = {
		['DungeonID'] = 20026,
		['DengeonType'] = 1,
		['DengeonRank'] = 7,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400023,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {300},
		['ScoreValue1'] = {0},
		['Rewards'] = {0},
		['DungeonPass'] = '',
		['DungeonTime'] = 300,
		['PassType'] = {1},
		['PassValue'] = {0},
		['PassValue1'] = {1},
		['ExitScene'] = 10004,
		['ExitPointX'] = 70.00,
		['BirthPointY'] = 21.00,
		['BirthPointZ'] = 120.00,
		['FaceY'] = 25.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20027] = {
		['DungeonID'] = 20027,
		['DengeonType'] = 1,
		['DengeonRank'] = 7,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400024,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {300},
		['ScoreValue1'] = {0},
		['Rewards'] = {0},
		['DungeonPass'] = '',
		['DungeonTime'] = 300,
		['PassType'] = {1},
		['PassValue'] = {0},
		['PassValue1'] = {1},
		['ExitScene'] = 10004,
		['ExitPointX'] = 70.00,
		['BirthPointY'] = 21.00,
		['BirthPointZ'] = 120.00,
		['FaceY'] = 25.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20028] = {
		['DungeonID'] = 20028,
		['DengeonType'] = 2,
		['DengeonRank'] = 0,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400025,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1,1,1},
		['ScoreValue'] = {30,300,600},
		['ScoreValue1'] = {0,0,0},
		['Rewards'] = {220100012,220100011,220100221},
		['DungeonPass'] = '',
		['DungeonTime'] = 600,
		['PassType'] = {1,1,1,1,1},
		['PassValue'] = {151100011,151100022,151100012,151100014,151100013},
		['PassValue1'] = {2,2,2,2,1},
		['ExitScene'] = 10000,
		['ExitPointX'] = 71.00,
		['BirthPointY'] = 58.00,
		['BirthPointZ'] = 98.00,
		['FaceY'] = 180.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20029] = {
		['DungeonID'] = 20029,
		['DengeonType'] = 1,
		['DengeonRank'] = 10,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 0,
		['AutomaticRelive'] = 1,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400030,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {600},
		['ScoreValue1'] = {0},
		['Rewards'] = {0},
		['DungeonPass'] = '',
		['DungeonTime'] = 600,
		['PassType'] = {0},
		['PassValue'] = {0},
		['PassValue1'] = {0},
		['ExitScene'] = 10016,
		['ExitPointX'] = 90.00,
		['BirthPointY'] = 28.00,
		['BirthPointZ'] = 63.00,
		['FaceY'] = -70.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20030] = {
		['DungeonID'] = 20030,
		['DengeonType'] = 1,
		['DengeonRank'] = 11,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 0,
		['AutomaticRelive'] = 1,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400030,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {600},
		['ScoreValue1'] = {0},
		['Rewards'] = {0},
		['DungeonPass'] = '',
		['DungeonTime'] = 600,
		['PassType'] = {4},
		['PassValue'] = {10},
		['PassValue1'] = {0},
		['ExitScene'] = 20029,
		['ExitPointX'] = 90.00,
		['BirthPointY'] = 28.00,
		['BirthPointZ'] = 63.00,
		['FaceY'] = -70.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20031] = {
		['DungeonID'] = 20031,
		['DengeonType'] = 1,
		['DengeonRank'] = 12,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 0,
		['AutomaticRelive'] = 1,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400030,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {600},
		['ScoreValue1'] = {0},
		['Rewards'] = {0},
		['DungeonPass'] = '',
		['DungeonTime'] = 600,
		['PassType'] = {0},
		['PassValue'] = {0},
		['PassValue1'] = {0},
		['ExitScene'] = 20029,
		['ExitPointX'] = 90.00,
		['BirthPointY'] = 28.00,
		['BirthPointZ'] = 63.00,
		['FaceY'] = -70.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20032] = {
		['DungeonID'] = 20032,
		['DengeonType'] = 1,
		['DengeonRank'] = 13,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 1,
		['ReliveTime'] = {6,0,6},
		['ActivityControl'] = 330400034,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {120},
		['ScoreValue1'] = {0},
		['Rewards'] = {220100012,220100011,220100221},
		['DungeonPass'] = '',
		['DungeonTime'] = 120,
		['PassType'] = {1},
		['PassValue'] = {151000001},
		['PassValue1'] = {2},
		['ExitScene'] = 10000,
		['ExitPointX'] = 71.00,
		['BirthPointY'] = 58.00,
		['BirthPointZ'] = 98.00,
		['FaceY'] = 180.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {}
		},
	[20033] = {
		['DungeonID'] = 20033,
		['DengeonType'] = 1,
		['DengeonRank'] = 13,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 1,
		['ReliveTime'] = {4,0,4},
		['ActivityControl'] = 330400034,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {60},
		['ScoreValue1'] = {0},
		['Rewards'] = {220100012,220100011,220100221},
		['DungeonPass'] = '',
		['DungeonTime'] = 60,
		['PassType'] = {1},
		['PassValue'] = {151000003},
		['PassValue1'] = {3},
		['ExitScene'] = 10000,
		['ExitPointX'] = 71.00,
		['BirthPointY'] = 58.00,
		['BirthPointZ'] = 98.00,
		['FaceY'] = 180.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {}
		},
	[20034] = {
		['DungeonID'] = 20034,
		['DengeonType'] = 1,
		['DengeonRank'] = 13,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 1,
		['ReliveTime'] = {5,1,10},
		['ActivityControl'] = 330400034,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {120},
		['ScoreValue1'] = {0},
		['Rewards'] = {220100012,220100011,220100221},
		['DungeonPass'] = '',
		['DungeonTime'] = 120,
		['PassType'] = {3},
		['PassValue'] = {154000011},
		['PassValue1'] = {2},
		['ExitScene'] = 10000,
		['ExitPointX'] = 71.00,
		['BirthPointY'] = 58.00,
		['BirthPointZ'] = 98.00,
		['FaceY'] = 180.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {}
		},
	[20035] = {
		['DungeonID'] = 20035,
		['DengeonType'] = 2,
		['DengeonRank'] = 8,
		['RandomGrade'] = 0,
		['CloseUI'] = 1,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 0,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400001,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1,1,1},
		['ScoreValue'] = {30,300,96000000},
		['ScoreValue1'] = {0,0,0},
		['Rewards'] = {220100012,220100011,220100221},
		['DungeonPass'] = 'dfg_geren',
		['DungeonTime'] = 96000000,
		['PassType'] = {0},
		['PassValue'] = {0},
		['PassValue1'] = {0},
		['ExitScene'] = 10012,
		['ExitPointX'] = 107.00,
		['BirthPointY'] = 22.00,
		['BirthPointZ'] = 86.00,
		['FaceY'] = -70.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20036] = {
		['DungeonID'] = 20036,
		['DengeonType'] = 1,
		['DengeonRank'] = 7,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 0,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400015,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {},
		['ScoreValue'] = {},
		['ScoreValue1'] = {},
		['Rewards'] = {0},
		['DungeonPass'] = '',
		['DungeonTime'] = 0,
		['PassType'] = {0},
		['PassValue'] = {0},
		['PassValue1'] = {0},
		['ExitScene'] = 0,
		['ExitPointX'] = .00,
		['BirthPointY'] = .00,
		['BirthPointZ'] = .00,
		['FaceY'] = .00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20037] = {
		['DungeonID'] = 20037,
		['DengeonType'] = 2,
		['DengeonRank'] = 8,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 2,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400001,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1,1,1},
		['ScoreValue'] = {30,300,96000000},
		['ScoreValue1'] = {0,0,0},
		['Rewards'] = {220100012,220100011,220100221},
		['DungeonPass'] = 'P20037Table',
		['DungeonTime'] = 96000000,
		['PassType'] = {0},
		['PassValue'] = {0},
		['PassValue1'] = {0},
		['ExitScene'] = 10012,
		['ExitPointX'] = 107.00,
		['BirthPointY'] = 22.00,
		['BirthPointZ'] = 86.00,
		['FaceY'] = -70.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[20038] = {
		['DungeonID'] = 20038,
		['DengeonType'] = 1,
		['DengeonRank'] = 4,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 0,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {3},
		['ScoreValue'] = {151005052},
		['ScoreValue1'] = {1},
		['Rewards'] = {},
		['DungeonPass'] = 'P20038Table',
		['DungeonTime'] = 0,
		['PassType'] = {2},
		['PassValue'] = {240},
		['PassValue1'] = {0},
		['ExitScene'] = 10004,
		['ExitPointX'] = 70.00,
		['BirthPointY'] = 21.00,
		['BirthPointZ'] = 120.00,
		['FaceY'] = 25.00,
		['StartTxt'] = '抵御妖族的入侵，守卫谷仓不被破坏',
		['EndTxt'] = '妖族退兵了，我们胜利了！',
		['RewardPosition'] = {-72.547,-1.43,115.181}
		},
	[20039] = {
		['DungeonID'] = 20039,
		['DengeonType'] = 1,
		['DengeonRank'] = 4,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 0,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {300},
		['ScoreValue1'] = {0},
		['Rewards'] = {},
		['DungeonPass'] = 'P20039Table',
		['DungeonTime'] = 300,
		['PassType'] = {1},
		['PassValue'] = {151005067},
		['PassValue1'] = {1},
		['ExitScene'] = 10004,
		['ExitPointX'] = 70.00,
		['BirthPointY'] = 21.00,
		['BirthPointZ'] = 120.00,
		['FaceY'] = 25.00,
		['StartTxt'] = '听说尸鬼除了众多手下外，还会各种机关法术，务必小心应对',
		['EndTxt'] = '尸鬼已除，顺利完成任务。',
		['RewardPosition'] = {-82,18.06,84.95}
		},
	[20040] = {
		['DungeonID'] = 20040,
		['DengeonType'] = 1,
		['DengeonRank'] = 4,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 0,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {300},
		['ScoreValue1'] = {0},
		['Rewards'] = {},
		['DungeonPass'] = '',
		['DungeonTime'] = 300,
		['PassType'] = {1},
		['PassValue'] = {151005026},
		['PassValue1'] = {1},
		['ExitScene'] = 10004,
		['ExitPointX'] = 70.00,
		['BirthPointY'] = 21.00,
		['BirthPointZ'] = 120.00,
		['FaceY'] = 25.00,
		['StartTxt'] = '还没开始做',
		['EndTxt'] = '还没开始做',
		['RewardPosition'] = {81.37,4.29,70.72}
		},
	[20041] = {
		['DungeonID'] = 20041,
		['DengeonType'] = 1,
		['DengeonRank'] = 4,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 1,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 0,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {180},
		['ScoreValue1'] = {0},
		['Rewards'] = {},
		['DungeonPass'] = '',
		['DungeonTime'] = 180,
		['PassType'] = {3},
		['PassValue'] = {154000011},
		['PassValue1'] = {1},
		['ExitScene'] = 10004,
		['ExitPointX'] = 70.00,
		['BirthPointY'] = 21.00,
		['BirthPointZ'] = 120.00,
		['FaceY'] = 25.00,
		['StartTxt'] = '还没开始做',
		['EndTxt'] = '还没开始做',
		['RewardPosition'] = {0}
		},
	[30001] = {
		['DungeonID'] = 30001,
		['DengeonType'] = 1,
		['DengeonRank'] = 9,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 0,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400019,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {600},
		['ScoreValue1'] = {0},
		['Rewards'] = {0},
		['DungeonPass'] = '',
		['DungeonTime'] = 600,
		['PassType'] = {4},
		['PassValue'] = {30},
		['PassValue1'] = {0},
		['ExitScene'] = 10000,
		['ExitPointX'] = 71.00,
		['BirthPointY'] = 58.00,
		['BirthPointZ'] = 98.00,
		['FaceY'] = 180.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[30002] = {
		['DungeonID'] = 30002,
		['DengeonType'] = 1,
		['DengeonRank'] = 9,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 0,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400019,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {600},
		['ScoreValue1'] = {0},
		['Rewards'] = {0},
		['DungeonPass'] = '',
		['DungeonTime'] = 600,
		['PassType'] = {4},
		['PassValue'] = {30},
		['PassValue1'] = {0},
		['ExitScene'] = 10000,
		['ExitPointX'] = 71.00,
		['BirthPointY'] = 58.00,
		['BirthPointZ'] = 98.00,
		['FaceY'] = 180.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		},
	[30003] = {
		['DungeonID'] = 30003,
		['DengeonType'] = 1,
		['DengeonRank'] = 9,
		['RandomGrade'] = 0,
		['CloseUI'] = 2,
		['StoryType'] = 1,
		['RandomID'] = {0},
		['RandomNum'] = 0,
		['TaskID'] = 0,
		['AddMidway'] = 1,
		['InstantRelive'] = 0,
		['AutomaticRelive'] = 0,
		['ReliveTime'] = {5,0,5},
		['ActivityControl'] = 330400019,
		['CostItemID'] = 0,
		['CostNum'] = 0,
		['Score'] = {1},
		['ScoreValue'] = {600},
		['ScoreValue1'] = {0},
		['Rewards'] = {0},
		['DungeonPass'] = '',
		['DungeonTime'] = 600,
		['PassType'] = {4},
		['PassValue'] = {30},
		['PassValue1'] = {0},
		['ExitScene'] = 10000,
		['ExitPointX'] = 71.00,
		['BirthPointY'] = 58.00,
		['BirthPointZ'] = 98.00,
		['FaceY'] = 180.00,
		['StartTxt'] = '',
		['EndTxt'] = '',
		['RewardPosition'] = {0}
		}
	}
return DungeonTable
