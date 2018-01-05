local CampDefenseTable = {
	[360600001] = {
		['ID'] = 360600001,	--索引
		['TruckID'] = 151500009,	--矿车ID
		['TruckRouteX'] = {123,82,54,53,123,111,90,108,128},	--矿车路径X
		['TruckRouteY'] = {200,200,178,131,131,107,80,62,62},	--矿车路径Y
		['TruckPlan'] = 100,	--矿车进度所需积分
		['ResurrectionTime'] = 30,	--矿车复活
		['SupportMine'] = {10,5,5,5},	--护矿
		['TruckDrop1'] = 154000043,	--成功护送矿车掉落
		['DropNum1'] = 10,	--守方采集点掉落个数
		['CollectionIntegral1'] = 20,	--成功护送矿车的采集积分
		['TruckDrop2'] = 154000043,	--被摧毁的矿车掉落
		['DropNum2'] = 10,	--攻方采集点掉落个数
		['CollectionIntegral2'] = 20,	--被摧毁的矿车采集积分
		['TaskID'] = 240130407,	--任务ID
		['MonsterID'] = {151500008,151500011,151500012},	--矿石怪ID
		['MineralIntegral1'] = {2,4,8},	--守方采集得分
		['MineralIntegral2'] = {1,2,4},	--攻方采集得分
		['MiningLimit1'] = 20,	--守方挖矿次数上限
		['MiningLimit2'] = 10	--攻方挖矿次数上限
		},
	[360600002] = {
		['ID'] = 360600002,
		['TruckID'] = 151500010,
		['TruckRouteX'] = {123,162,185,196,123,140,163,149,130},
		['TruckRouteY'] = {200,183,156,130,130,108,80,70,62},
		['TruckPlan'] = 100,
		['ResurrectionTime'] = 30,
		['SupportMine'] = {10,5,5,5},
		['TruckDrop1'] = 154000043,
		['DropNum1'] = 10,
		['CollectionIntegral1'] = 20,
		['TruckDrop2'] = 154000043,
		['DropNum2'] = 10,
		['CollectionIntegral2'] = 20,
		['TaskID'] = 240130407,
		['MonsterID'] = {151500008,151500011,151500012},
		['MineralIntegral1'] = {2,4,8},
		['MineralIntegral2'] = {1,2,4},
		['MiningLimit1'] = 20,
		['MiningLimit2'] = 10
		}
	}
return CampDefenseTable
