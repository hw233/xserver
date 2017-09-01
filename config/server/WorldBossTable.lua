local WorldBossTable = {
	[510100001] = {
		['ID'] = 510100001,	--索引
		['Name'] = '恶霸头目',	--名称
		['Level'] = 30,	--等级
		['Type'] = 1,	--类型
		['Map'] = 155000005,	--地图
		['Coordinate'] = {85,0,100},	--坐标
		['SceneID'] = 10007,	--场景ID
		['MonsterID'] = 151600001,	--怪物ID
		['Time'] = {1200,2000},	--刷新时间
		['Coefficient'] = 1E-3,	--系数
		['RewardLevel'] = 40	--奖励等级
		},
	[510100002] = {
		['ID'] = 510100002,
		['Name'] = '太极',
		['Level'] = 40,
		['Type'] = 1,
		['Map'] = 155000002,
		['Coordinate'] = {175,0,35},
		['SceneID'] = 10011,
		['MonsterID'] = 151600002,
		['Time'] = {1300,2100},
		['Coefficient'] = 1E-3,
		['RewardLevel'] = 50
		},
	[510100003] = {
		['ID'] = 510100003,
		['Name'] = '僵尸头目',
		['Level'] = 50,
		['Type'] = 1,
		['Map'] = 155000004,
		['Coordinate'] = {95,0,225},
		['SceneID'] = 10006,
		['MonsterID'] = 151600003,
		['Time'] = {1400,2200},
		['Coefficient'] = 1E-3,
		['RewardLevel'] = 60
		},
	[510101001] = {
		['ID'] = 510101001,
		['Name'] = '妖帝真身',
		['Level'] = 60,
		['Type'] = 2,
		['Map'] = 155000008,
		['Coordinate'] = {210,0,345},
		['SceneID'] = 10004,
		['MonsterID'] = 151600005,
		['Time'] = {1500,2300},
		['Coefficient'] = 1E-3,
		['RewardLevel'] = 70
		}
	}
return WorldBossTable
