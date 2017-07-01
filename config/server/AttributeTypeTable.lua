local AttributeTypeTable = {
	[1] = {
		['ID'] = 1,	--ID
		['Effect'] = 'MaxHP',	--属性名称
		['FightRatio'] = 0.1	--战力系数
		},
	[2] = {
		['ID'] = 2,
		['Effect'] = 'Health',
		['FightRatio'] = 0
		},
	[3] = {
		['ID'] = 3,
		['Effect'] = 'Attack',
		['FightRatio'] = 1
		},
	[4] = {
		['ID'] = 4,
		['Effect'] = 'Defense',
		['FightRatio'] = 1
		},
	[5] = {
		['ID'] = 5,
		['Effect'] = 'AtGold',
		['FightRatio'] = 1
		},
	[6] = {
		['ID'] = 6,
		['Effect'] = 'AtWood',
		['FightRatio'] = 1
		},
	[7] = {
		['ID'] = 7,
		['Effect'] = 'AtWater',
		['FightRatio'] = 1
		},
	[8] = {
		['ID'] = 8,
		['Effect'] = 'AtFire',
		['FightRatio'] = 1
		},
	[9] = {
		['ID'] = 9,
		['Effect'] = 'AtEarth',
		['FightRatio'] = 1
		},
	[10] = {
		['ID'] = 10,
		['Effect'] = 'AtGoldDf',
		['FightRatio'] = 1
		},
	[11] = {
		['ID'] = 11,
		['Effect'] = 'AtWoodDf',
		['FightRatio'] = 1
		},
	[12] = {
		['ID'] = 12,
		['Effect'] = 'AtWaterDf',
		['FightRatio'] = 1
		},
	[13] = {
		['ID'] = 13,
		['Effect'] = 'AtFireDf',
		['FightRatio'] = 1
		},
	[14] = {
		['ID'] = 14,
		['Effect'] = 'AtEarthDf',
		['FightRatio'] = 1
		},
	[15] = {
		['ID'] = 15,
		['Effect'] = 'Dodge',
		['FightRatio'] = 1
		},
	[16] = {
		['ID'] = 16,
		['Effect'] = 'Hit',
		['FightRatio'] = 1
		},
	[17] = {
		['ID'] = 17,
		['Effect'] = 'Critical',
		['FightRatio'] = 1
		},
	[18] = {
		['ID'] = 18,
		['Effect'] = 'CriticalDf',
		['FightRatio'] = 1
		},
	[19] = {
		['ID'] = 19,
		['Effect'] = 'CtDmg',
		['FightRatio'] = 500
		},
	[20] = {
		['ID'] = 20,
		['Effect'] = 'CtDmgDf',
		['FightRatio'] = 10000
		},
	[21] = {
		['ID'] = 21,
		['Effect'] = 'MoveSpeed',
		['FightRatio'] = 1
		},
	[22] = {
		['ID'] = 22,
		['Effect'] = 'Recover',
		['FightRatio'] = 1
		},
	[23] = {
		['ID'] = 23,
		['Effect'] = 'Cure',
		['FightRatio'] = 1
		},
	[24] = {
		['ID'] = 24,
		['Effect'] = 'CureAdd',
		['FightRatio'] = 1
		},
	[25] = {
		['ID'] = 25,
		['Effect'] = 'DmgAdd',
		['FightRatio'] = 1
		},
	[26] = {
		['ID'] = 26,
		['Effect'] = 'DmgDf',
		['FightRatio'] = 1
		},
	[27] = {
		['ID'] = 27,
		['Effect'] = 'DmgAddPE',
		['FightRatio'] = 1
		},
	[28] = {
		['ID'] = 28,
		['Effect'] = 'DmgDfPP',
		['FightRatio'] = 1
		},
	[29] = {
		['ID'] = 29,
		['Effect'] = 'Dizzy',
		['FightRatio'] = 10000
		},
	[30] = {
		['ID'] = 30,
		['Effect'] = 'Slow',
		['FightRatio'] = 10000
		},
	[31] = {
		['ID'] = 31,
		['Effect'] = 'Chaos',
		['FightRatio'] = 10000
		},
	[32] = {
		['ID'] = 32,
		['Effect'] = 'Poison',
		['FightRatio'] = 10000
		},
	[33] = {
		['ID'] = 33,
		['Effect'] = 'Vampire',
		['FightRatio'] = 10000
		},
	[34] = {
		['ID'] = 34,
		['Effect'] = 'Bounce',
		['FightRatio'] = 10000
		},
	[35] = {
		['ID'] = 35,
		['Effect'] = 'HitRepel',
		['FightRatio'] = 10000
		},
	[36] = {
		['ID'] = 36,
		['Effect'] = 'Hitfly',
		['FightRatio'] = 10000
		},
	[37] = {
		['ID'] = 37,
		['Effect'] = 'DizzyDf',
		['FightRatio'] = 10000
		},
	[38] = {
		['ID'] = 38,
		['Effect'] = 'SlowDf',
		['FightRatio'] = 10000
		},
	[39] = {
		['ID'] = 39,
		['Effect'] = 'ChaosDf',
		['FightRatio'] = 10000
		},
	[40] = {
		['ID'] = 40,
		['Effect'] = 'PoisonDf',
		['FightRatio'] = 10000
		},
	[41] = {
		['ID'] = 41,
		['Effect'] = 'VampireDf',
		['FightRatio'] = 10000
		},
	[42] = {
		['ID'] = 42,
		['Effect'] = 'BounceDf',
		['FightRatio'] = 10000
		},
	[43] = {
		['ID'] = 43,
		['Effect'] = 'HitRepelDf',
		['FightRatio'] = 10000
		},
	[44] = {
		['ID'] = 44,
		['Effect'] = 'HitflyDf',
		['FightRatio'] = 10000
		},
	[45] = {
		['ID'] = 45,
		['Effect'] = 'Level',
		['FightRatio'] = 0
		},
	[46] = {
		['ID'] = 46,
		['Effect'] = 'FlySpeed',
		['FightRatio'] = 0
		},
	[47] = {
		['ID'] = 47,
		['Effect'] = 'PLAYER_Camp',
		['FightRatio'] = 0
		},
	[48] = {
		['ID'] = 48,
		['Effect'] = 'PLAYER_ATTR_PK_TYPE',
		['FightRatio'] = 0
		},
	[51] = {
		['ID'] = 51,
		['Effect'] = 'Job',
		['FightRatio'] = 0
		},
	[53] = {
		['ID'] = 53,
		['Effect'] = 'Exp',
		['FightRatio'] = 0
		},
	[54] = {
		['ID'] = 54,
		['Effect'] = 'Gold',
		['FightRatio'] = 0
		},
	[55] = {
		['ID'] = 55,
		['Effect'] = 'BindGold',
		['FightRatio'] = 0
		},
	[56] = {
		['ID'] = 56,
		['Effect'] = 'Coin',
		['FightRatio'] = 0
		},
	[57] = {
		['ID'] = 57,
		['Effect'] = 'Head',
		['FightRatio'] = 0
		},
	[58] = {
		['ID'] = 58,
		['Effect'] = 'clothes',
		['FightRatio'] = 0
		},
	[59] = {
		['ID'] = 59,
		['Effect'] = 'clothes_color_up',
		['FightRatio'] = 0
		},
	[60] = {
		['ID'] = 60,
		['Effect'] = 'hat',
		['FightRatio'] = 0
		},
	[61] = {
		['ID'] = 61,
		['Effect'] = 'hat_color',
		['FightRatio'] = 0
		},
	[62] = {
		['ID'] = 62,
		['Effect'] = 'clothes_color_down',
		['FightRatio'] = 0
		},
	[63] = {
		['ID'] = 63,
		['Effect'] = 'relive_type1',
		['FightRatio'] = 0
		},
	[64] = {
		['ID'] = 64,
		['Effect'] = 'relive_type2',
		['FightRatio'] = 0
		},
	[65] = {
		['ID'] = 65,
		['Effect'] = 'Weapon',
		['FightRatio'] = 0
		},
	[66] = {
		['ID'] = 66,
		['Effect'] = 'Fighting_Capacity',
		['FightRatio'] = 0
		},
	[67] = {
		['ID'] = 67,
		['Effect'] = '',
		['FightRatio'] = 0
		},
	[68] = {
		['ID'] = 68,
		['Effect'] = '',
		['FightRatio'] = 0
		},
	[69] = {
		['ID'] = 69,
		['Effect'] = 'ZhenQi',
		['FightRatio'] = 0
		},
	[70] = {
		['ID'] = 70,
		['Effect'] = 'PLAYER_ATTR_MURDER',
		['FightRatio'] = 0
		},
	[71] = {
		['ID'] = 71,
		['Effect'] = 'PLAYER_ATTR_COURAGE_GOLD',
		['FightRatio'] = 0
		},
	[72] = {
		['ID'] = 72,
		['Effect'] = 'PLAYER_ATTR_BAGUA',
		['FightRatio'] = 0
		},
	[73] = {
		['ID'] = 73,
		['Effect'] = 'PLAYER_ATTR_ACTIVENESS',
		['FightRatio'] = 0
		},
	[74] = {
		['ID'] = 74,
		['Effect'] = 'PLAYER_ATTR_REGION_ID',
		['FightRatio'] = 0
		},
	[75] = {
		['ID'] = 75,
		['Effect'] = 'PLAYER_ATTR_CHIVALRY',
		['FightRatio'] = 0
		},
	[77] = {
		['ID'] = 77,
		['Effect'] = 'Jingli',
		['FightRatio'] = 0
		},
	[78] = {
		['ID'] = 78,
		['Effect'] = 'Yongwu',
		['FightRatio'] = 0
		},
	[79] = {
		['ID'] = 79,
		['Effect'] = '',
		['FightRatio'] = 0
		},
	[80] = {
		['ID'] = 80,
		['Effect'] = '',
		['FightRatio'] = 0
		},
	[81] = {
		['ID'] = 81,
		['Effect'] = '',
		['FightRatio'] = 0
		},
	[82] = {
		['ID'] = 82,
		['Effect'] = '',
		['FightRatio'] = 0
		},
	[83] = {
		['ID'] = 83,
		['Effect'] = '',
		['FightRatio'] = 0
		},
	[84] = {
		['ID'] = 84,
		['Effect'] = 'player_attr_weapon_color',
		['FightRatio'] = 0
		},
	[100] = {
		['ID'] = 100,
		['Effect'] = 'Player_Max',
		['FightRatio'] = 0
		}
	}
return AttributeTypeTable
