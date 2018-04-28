local WorldBossTable = {
	[510102001] = {
		['ID'] = 510102001,	--索引
		['Name'] = '恶霸头目',	--名称
		['Level'] = 30,	--等级
		['Type'] = 2,	--类型
		['Map'] = 155000008,	--地图
		['Coordinate'] = {358,19,479},	--坐标
		['SceneID'] = 10004,	--场景ID
		['MonsterID'] = 151600001,	--怪物ID
		['Time'] = {1200,1800},	--刷新时间
		['Coefficient'] = 1E-3,	--系数
		['RewardLevel'] = 40	--奖励等级
		},
	[510102002] = {
		['ID'] = 510102002,
		['Name'] = '太极',
		['Level'] = 40,
		['Type'] = 2,
		['Map'] = 155000005,
		['Coordinate'] = {79,0,100},
		['SceneID'] = 10007,
		['MonsterID'] = 151600005,
		['Time'] = {1300,1900},
		['Coefficient'] = 1E-3,
		['RewardLevel'] = 50
		},
	[510102003] = {
		['ID'] = 510102003,
		['Name'] = '僵尸头目',
		['Level'] = 50,
		['Type'] = 2,
		['Map'] = 155000004,
		['Coordinate'] = {180,0,180},
		['SceneID'] = 10006,
		['MonsterID'] = 151600003,
		['Time'] = {1400,2000},
		['Coefficient'] = 1E-3,
		['RewardLevel'] = 60
		},
	[510101001] = {
		['ID'] = 510101001,
		['Name'] = '阳炎',
		['Level'] = 30,
		['Type'] = 1,
		['Map'] = 155000008,
		['Coordinate'] = {194,47,435},
		['SceneID'] = 10004,
		['MonsterID'] = 151601001,
		['Time'] = {1200,1800},
		['Coefficient'] = 1E-3,
		['RewardLevel'] = 40
		},
	[510101002] = {
		['ID'] = 510101002,
		['Name'] = '玄武',
		['Level'] = 40,
		['Type'] = 1,
		['Map'] = 155000005,
		['Coordinate'] = {61,41.7,433},
		['SceneID'] = 10007,
		['MonsterID'] = 151601005,
		['Time'] = {1300,1900},
		['Coefficient'] = 1E-3,
		['RewardLevel'] = 50
		},
	[510101003] = {
		['ID'] = 510101003,
		['Name'] = '魔妖',
		['Level'] = 50,
		['Type'] = 1,
		['Map'] = 155000004,
		['Coordinate'] = {32,25.7,90},
		['SceneID'] = 10006,
		['MonsterID'] = 151601003,
		['Time'] = {1400,2000},
		['Coefficient'] = 1E-3,
		['RewardLevel'] = 60
		}
	}
return WorldBossTable
