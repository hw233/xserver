local SpiritTable = {
	[1] = {
		['ID'] = 1,	--索引
		['Level'] = 1,	--角色等级
		['MountQuality'] = 1,	--阶级
		['SpiritAttribute'] = {1,3,4,17},	--修灵属性
		['AttributeCeiling'] = {250,55,110,15},	--属性上限
		['SpiritExpend'] = 201060021,	--修灵消耗
		['ExpendNum'] = 1,	--消耗数量
		['GradeNum'] = {5,5,5,5},	--升级次数
		['OrderAttribute'] = {50,10,20,0}	--进阶属性
		},
	[2] = {
		['ID'] = 2,
		['Level'] = 1,
		['MountQuality'] = 2,
		['SpiritAttribute'] = {1,3,4,17},
		['AttributeCeiling'] = {500,110,220,30},
		['SpiritExpend'] = 201060021,
		['ExpendNum'] = 1,
		['GradeNum'] = {10,10,10,10},
		['OrderAttribute'] = {100,20,40,5}
		},
	[3] = {
		['ID'] = 3,
		['Level'] = 5,
		['MountQuality'] = 3,
		['SpiritAttribute'] = {1,3,4,17},
		['AttributeCeiling'] = {1000,220,440,60},
		['SpiritExpend'] = 201060021,
		['ExpendNum'] = 1,
		['GradeNum'] = {12,12,12,12},
		['OrderAttribute'] = {120,25,50,5}
		},
	[4] = {
		['ID'] = 4,
		['Level'] = 5,
		['MountQuality'] = 4,
		['SpiritAttribute'] = {1,3,4,17},
		['AttributeCeiling'] = {1600,350,700,95},
		['SpiritExpend'] = 201060021,
		['ExpendNum'] = 1,
		['GradeNum'] = {14,14,14,14},
		['OrderAttribute'] = {140,30,60,5}
		},
	[5] = {
		['ID'] = 5,
		['Level'] = 5,
		['MountQuality'] = 5,
		['SpiritAttribute'] = {1,3,4,17},
		['AttributeCeiling'] = {2300,505,1010,135},
		['SpiritExpend'] = 201060021,
		['ExpendNum'] = 1,
		['GradeNum'] = {16,16,16,16},
		['OrderAttribute'] = {160,35,70,5}
		},
	[6] = {
		['ID'] = 6,
		['Level'] = 10,
		['MountQuality'] = 6,
		['SpiritAttribute'] = {1,3,4,17},
		['AttributeCeiling'] = {3100,680,1360,185},
		['SpiritExpend'] = 201060021,
		['ExpendNum'] = 1,
		['GradeNum'] = {18,18,18,18},
		['OrderAttribute'] = {180,35,75,10}
		},
	[7] = {
		['ID'] = 7,
		['Level'] = 10,
		['MountQuality'] = 7,
		['SpiritAttribute'] = {1,3,4,17},
		['AttributeCeiling'] = {4000,880,1760,240},
		['SpiritExpend'] = 201060021,
		['ExpendNum'] = 1,
		['GradeNum'] = {20,20,20,20},
		['OrderAttribute'] = {200,40,85,10}
		},
	[8] = {
		['ID'] = 8,
		['Level'] = 10,
		['MountQuality'] = 8,
		['SpiritAttribute'] = {1,3,4,17},
		['AttributeCeiling'] = {5000,1100,2200,300},
		['SpiritExpend'] = 201060021,
		['ExpendNum'] = 1,
		['GradeNum'] = {22,22,22,22},
		['OrderAttribute'] = {220,45,95,10}
		},
	[9] = {
		['ID'] = 9,
		['Level'] = 15,
		['MountQuality'] = 9,
		['SpiritAttribute'] = {1,3,4,17},
		['AttributeCeiling'] = {6100,1340,2680,365},
		['SpiritExpend'] = 201060021,
		['ExpendNum'] = 1,
		['GradeNum'] = {24,24,24,24},
		['OrderAttribute'] = {240,50,105,10}
		},
	[10] = {
		['ID'] = 10,
		['Level'] = 15,
		['MountQuality'] = 10,
		['SpiritAttribute'] = {1,3,4,17},
		['AttributeCeiling'] = {7300,1605,3210,435},
		['SpiritExpend'] = 201060021,
		['ExpendNum'] = 1,
		['GradeNum'] = {26,26,26,26},
		['OrderAttribute'] = {260,55,110,15}
		},
	[11] = {
		['ID'] = 11,
		['Level'] = 15,
		['MountQuality'] = 11,
		['SpiritAttribute'] = {1,3,4,17},
		['AttributeCeiling'] = {8600,1890,3780,515},
		['SpiritExpend'] = 201060021,
		['ExpendNum'] = 1,
		['GradeNum'] = {28,28,28,28},
		['OrderAttribute'] = {280,60,120,15}
		},
	[12] = {
		['ID'] = 12,
		['Level'] = 20,
		['MountQuality'] = 12,
		['SpiritAttribute'] = {1,3,4,17},
		['AttributeCeiling'] = {10000,2200,4400,600},
		['SpiritExpend'] = 201060021,
		['ExpendNum'] = 1,
		['GradeNum'] = {30,30,30,30},
		['OrderAttribute'] = {300,65,130,15}
		},
	[13] = {
		['ID'] = 13,
		['Level'] = 20,
		['MountQuality'] = 13,
		['SpiritAttribute'] = {1,3,4,17},
		['AttributeCeiling'] = {11500,2530,5060,690},
		['SpiritExpend'] = 201060021,
		['ExpendNum'] = 1,
		['GradeNum'] = {32,32,32,32},
		['OrderAttribute'] = {320,70,140,15}
		},
	[14] = {
		['ID'] = 14,
		['Level'] = 20,
		['MountQuality'] = 14,
		['SpiritAttribute'] = {1,3,4,17},
		['AttributeCeiling'] = {13100,2880,5760,785},
		['SpiritExpend'] = 201060021,
		['ExpendNum'] = 1,
		['GradeNum'] = {34,34,34,34},
		['OrderAttribute'] = {340,70,145,20}
		},
	[15] = {
		['ID'] = 15,
		['Level'] = 30,
		['MountQuality'] = 15,
		['SpiritAttribute'] = {1,3,4,17},
		['AttributeCeiling'] = {14800,3255,6510,885},
		['SpiritExpend'] = 201060021,
		['ExpendNum'] = 1,
		['GradeNum'] = {36,36,36,36},
		['OrderAttribute'] = {360,75,155,20}
		},
	[16] = {
		['ID'] = 16,
		['Level'] = 30,
		['MountQuality'] = 16,
		['SpiritAttribute'] = {1,3,4,17},
		['AttributeCeiling'] = {16600,3650,7300,995},
		['SpiritExpend'] = 201060021,
		['ExpendNum'] = 1,
		['GradeNum'] = {38,38,38,38},
		['OrderAttribute'] = {375,80,165,20}
		},
	[17] = {
		['ID'] = 17,
		['Level'] = 30,
		['MountQuality'] = 17,
		['SpiritAttribute'] = {1,3,4,17},
		['AttributeCeiling'] = {18500,4070,8140,1110},
		['SpiritExpend'] = 201060021,
		['ExpendNum'] = 1,
		['GradeNum'] = {40,40,40,40},
		['OrderAttribute'] = {400,85,175,20}
		},
	[18] = {
		['ID'] = 18,
		['Level'] = 0,
		['MountQuality'] = 18,
		['SpiritAttribute'] = {1,3,4,17},
		['AttributeCeiling'] = {20500,4510,9020,1230},
		['SpiritExpend'] = 201060021,
		['ExpendNum'] = 1,
		['GradeNum'] = {40,40,40,40},
		['OrderAttribute'] = {400,85,175,20}
		}
	}
return SpiritTable
