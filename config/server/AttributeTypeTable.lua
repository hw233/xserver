local AttributeTypeTable = {
	[1] = {
		['ID'] = 1,	--ID
		['Effect'] = 'Health',	--属性名称
		['FightRatio'] = 0,	--战力系数
		['Total'] = 1,	--总战力计算
		['ShowType'] = 0	--显示类型
		},
	[2] = {
		['ID'] = 2,
		['Effect'] = 'MaxHP',
		['FightRatio'] = 7.4999999999999997E-2,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[3] = {
		['ID'] = 3,
		['Effect'] = 'Attack',
		['FightRatio'] = 1,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[4] = {
		['ID'] = 4,
		['Effect'] = 'DfWuDel',
		['FightRatio'] = 2,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[5] = {
		['ID'] = 5,
		['Effect'] = 'AtGold',
		['FightRatio'] = 1,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[6] = {
		['ID'] = 6,
		['Effect'] = 'AtWood',
		['FightRatio'] = 1,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[7] = {
		['ID'] = 7,
		['Effect'] = 'AtWater',
		['FightRatio'] = 1,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[8] = {
		['ID'] = 8,
		['Effect'] = 'AtFire',
		['FightRatio'] = 1,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[9] = {
		['ID'] = 9,
		['Effect'] = 'AtEarth',
		['FightRatio'] = 1,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[10] = {
		['ID'] = 10,
		['Effect'] = 'AtGoldDf',
		['FightRatio'] = 0.4,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[11] = {
		['ID'] = 11,
		['Effect'] = 'AtWoodDf',
		['FightRatio'] = 0.4,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[12] = {
		['ID'] = 12,
		['Effect'] = 'AtWaterDf',
		['FightRatio'] = 0.4,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[13] = {
		['ID'] = 13,
		['Effect'] = 'AtFireDf',
		['FightRatio'] = 0.4,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[14] = {
		['ID'] = 14,
		['Effect'] = 'AtEarthDf',
		['FightRatio'] = 0.4,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[15] = {
		['ID'] = 15,
		['Effect'] = 'Dodge',
		['FightRatio'] = 2,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[16] = {
		['ID'] = 16,
		['Effect'] = 'Hit',
		['FightRatio'] = 1,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[17] = {
		['ID'] = 17,
		['Effect'] = 'Critical',
		['FightRatio'] = 2,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[18] = {
		['ID'] = 18,
		['Effect'] = 'CriticalDf',
		['FightRatio'] = 2,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[19] = {
		['ID'] = 19,
		['Effect'] = 'CtDmg',
		['FightRatio'] = 3000,
		['Total'] = 1,
		['ShowType'] = 1
		},
	[20] = {
		['ID'] = 20,
		['Effect'] = 'CtDmgDf',
		['FightRatio'] = 2000,
		['Total'] = 1,
		['ShowType'] = 1
		},
	[21] = {
		['ID'] = 21,
		['Effect'] = 'MoveSpeed',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[22] = {
		['ID'] = 22,
		['Effect'] = 'DodgeDf',
		['FightRatio'] = 2,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[23] = {
		['ID'] = 23,
		['Effect'] = 'Dizzy',
		['FightRatio'] = 1,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[24] = {
		['ID'] = 24,
		['Effect'] = 'Slow',
		['FightRatio'] = 1,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[25] = {
		['ID'] = 25,
		['Effect'] = 'Mabi',
		['FightRatio'] = 1,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[26] = {
		['ID'] = 26,
		['Effect'] = 'Hurt',
		['FightRatio'] = 1,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[27] = {
		['ID'] = 27,
		['Effect'] = 'Can',
		['FightRatio'] = 1,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[28] = {
		['ID'] = 28,
		['Effect'] = 'DizzyDf',
		['FightRatio'] = 1,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[29] = {
		['ID'] = 29,
		['Effect'] = 'SlowDf',
		['FightRatio'] = 1,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[30] = {
		['ID'] = 30,
		['Effect'] = 'MabiDf',
		['FightRatio'] = 1,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[31] = {
		['ID'] = 31,
		['Effect'] = 'HurtDf',
		['FightRatio'] = 1,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[32] = {
		['ID'] = 32,
		['Effect'] = 'CanDf',
		['FightRatio'] = 1,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[33] = {
		['ID'] = 33,
		['Effect'] = 'PVPAt',
		['FightRatio'] = 1,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[34] = {
		['ID'] = 34,
		['Effect'] = 'PVPDf',
		['FightRatio'] = 1,
		['Total'] = 1,
		['ShowType'] = 0
		},
	[35] = {
		['ID'] = 35,
		['Effect'] = 'PLAYER_ATTR_REGION_ID',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[36] = {
		['ID'] = 36,
		['Effect'] = 'Fighting_Capacity',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[37] = {
		['ID'] = 37,
		['Effect'] = 'CanTime',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[38] = {
		['ID'] = 38,
		['Effect'] = 'DizzyTimeDf',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[39] = {
		['ID'] = 39,
		['Effect'] = 'SlowTimeDf',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[40] = {
		['ID'] = 40,
		['Effect'] = 'MabiTimeDf',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[41] = {
		['ID'] = 41,
		['Effect'] = 'HurtTimeDf',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[42] = {
		['ID'] = 42,
		['Effect'] = 'CanTimeDf',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[43] = {
		['ID'] = 43,
		['Effect'] = 'DeEffDf',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[44] = {
		['ID'] = 44,
		['Effect'] = 'DeTimeDf',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[45] = {
		['ID'] = 45,
		['Effect'] = 'Level',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[46] = {
		['ID'] = 46,
		['Effect'] = 'FlySpeed',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[47] = {
		['ID'] = 47,
		['Effect'] = 'PLAYER_Camp',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[48] = {
		['ID'] = 48,
		['Effect'] = 'PLAYER_ATTR_PK_TYPE',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[49] = {
		['ID'] = 49,
		['Effect'] = 'attr_fight_max',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[50] = {
		['ID'] = 50,
		['Effect'] = 'Battle',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[51] = {
		['ID'] = 51,
		['Effect'] = 'Job',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[52] = {
		['ID'] = 52,
		['Effect'] = 'yinbi',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[53] = {
		['ID'] = 53,
		['Effect'] = 'Exp',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[54] = {
		['ID'] = 54,
		['Effect'] = 'Gold',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[55] = {
		['ID'] = 55,
		['Effect'] = 'BindGold',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[56] = {
		['ID'] = 56,
		['Effect'] = 'Coin',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[57] = {
		['ID'] = 57,
		['Effect'] = 'Head',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[58] = {
		['ID'] = 58,
		['Effect'] = 'clothes',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[59] = {
		['ID'] = 59,
		['Effect'] = 'clothes_color_up',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[60] = {
		['ID'] = 60,
		['Effect'] = 'hat',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[61] = {
		['ID'] = 61,
		['Effect'] = 'hat_color',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[62] = {
		['ID'] = 62,
		['Effect'] = 'clothes_color_down',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[63] = {
		['ID'] = 63,
		['Effect'] = 'relive_type1',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[64] = {
		['ID'] = 64,
		['Effect'] = 'relive_type2',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[65] = {
		['ID'] = 65,
		['Effect'] = 'Weapon',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[67] = {
		['ID'] = 67,
		['Effect'] = 'Mount',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[68] = {
		['ID'] = 68,
		['Effect'] = 'MountState',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[69] = {
		['ID'] = 69,
		['Effect'] = 'ZhenQi',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[70] = {
		['ID'] = 70,
		['Effect'] = 'PLAYER_ATTR_MURDER',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[71] = {
		['ID'] = 71,
		['Effect'] = 'gongxun',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[72] = {
		['ID'] = 72,
		['Effect'] = 'PLAYER_ATTR_BAGUA',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[73] = {
		['ID'] = 73,
		['Effect'] = 'PLAYER_ATTR_ACTIVENESS',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[74] = {
		['ID'] = 74,
		['Effect'] = 'RegionID',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[75] = {
		['ID'] = 75,
		['Effect'] = 'PLAYER_ATTR_CHIVALRY',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[77] = {
		['ID'] = 77,
		['Effect'] = 'Jingli',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[78] = {
		['ID'] = 78,
		['Effect'] = 'Yongwu',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[79] = {
		['ID'] = 79,
		['Effect'] = 'yaojiCD',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[80] = {
		['ID'] = 80,
		['Effect'] = 'yaopinCD',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[81] = {
		['ID'] = 81,
		['Effect'] = 'player_attr_partner_fight',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[82] = {
		['ID'] = 82,
		['Effect'] = 'player_attr_partner_precedence',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[83] = {
		['ID'] = 83,
		['Effect'] = 'player_attr_partner_anger',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[84] = {
		['ID'] = 84,
		['Effect'] = 'player_attr_weapon_color',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[85] = {
		['ID'] = 85,
		['Effect'] = 'TiLv',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[86] = {
		['ID'] = 86,
		['Effect'] = 'LiLv',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[87] = {
		['ID'] = 87,
		['Effect'] = 'MinLv',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[88] = {
		['ID'] = 88,
		['Effect'] = 'LingLv',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[89] = {
		['ID'] = 89,
		['Effect'] = 'Ti',
		['FightRatio'] = 1.5,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[90] = {
		['ID'] = 90,
		['Effect'] = 'Li',
		['FightRatio'] = 1.5,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[91] = {
		['ID'] = 91,
		['Effect'] = 'Min',
		['FightRatio'] = 1.5,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[92] = {
		['ID'] = 92,
		['Effect'] = 'Ling',
		['FightRatio'] = 1.5,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[93] = {
		['ID'] = 93,
		['Effect'] = 'HealthPro',
		['FightRatio'] = 8000,
		['Total'] = 0,
		['ShowType'] = 1
		},
	[94] = {
		['ID'] = 94,
		['Effect'] = 'AttackPro',
		['FightRatio'] = 8000,
		['Total'] = 0,
		['ShowType'] = 1
		},
	[95] = {
		['ID'] = 95,
		['Effect'] = 'DfWu',
		['FightRatio'] = 2,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[96] = {
		['ID'] = 96,
		['Effect'] = 'AllEff',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[97] = {
		['ID'] = 97,
		['Effect'] = 'AllEffDf',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[98] = {
		['ID'] = 98,
		['Effect'] = 'AllTime',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[99] = {
		['ID'] = 99,
		['Effect'] = 'AllTimeDf',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[100] = {
		['ID'] = 100,
		['Effect'] = 'shengwang',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[101] = {
		['ID'] = 101,
		['Effect'] = 'xuejing',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[102] = {
		['ID'] = 102,
		['Effect'] = 'lingshi',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[103] = {
		['ID'] = 103,
		['Effect'] = 'Title',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[104] = {
		['ID'] = 104,
		['Effect'] = 'ExpChangeZhenqi',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		},
	[200] = {
		['ID'] = 200,
		['Effect'] = 'Player_Max',
		['FightRatio'] = 0,
		['Total'] = 0,
		['ShowType'] = 0
		}
	}
return AttributeTypeTable
