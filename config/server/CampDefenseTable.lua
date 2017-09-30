local CampDefenseTable = {
	[360600001] = {
		['ID'] = 360600001,	--索引
		['TruckID'] = 151500009,	--矿车ID
		['TruckRouteX'] = {123,82,54,53,123,111,90,108,128},	--矿车路径X
		['TruckRouteY'] = {200,200,178,131,131,107,80,62,62},	--矿车路径Y
		['TruckPlan'] = 10,	--矿车进度
		['ResurrectionTime'] = 30,	--矿车复活
		['SupportMine'] = {10,5,5,50,5},	--护矿
		['TruckDrop'] = 154000043,	--矿车掉落
		['CollectionIntegral'] = {3,8},	--采集积分
		['MineralIntegral'] = 1,	--矿石积分
		['TaskID'] = 240130407	--任务ID
		},
	[360600002] = {
		['ID'] = 360600002,
		['TruckID'] = 151500010,
		['TruckRouteX'] = {123,162,185,196,123,140,163,149,130},
		['TruckRouteY'] = {200,183,156,130,130,108,80,70,62},
		['TruckPlan'] = 10,
		['ResurrectionTime'] = 30,
		['SupportMine'] = {10,5,5,50,5},
		['TruckDrop'] = 154000043,
		['CollectionIntegral'] = {3,8},
		['MineralIntegral'] = 1,
		['TaskID'] = 240130407
		}
	}
return CampDefenseTable
