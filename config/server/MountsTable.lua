local MountsTable = {
	[180000001] = {
		['ID'] = 180000001,	--索引
		['Name'] = '凤凰',	--名称
		['ResId'] = 130500001,	--模型编号
		['BaseAttribute'] = 102200001,	--属性组id
		['MountsAttribute'] = {21,46,2,3,4,15},	--坐骑属性
		['AttributeCeiling'] = {8,10,0,0,0,0},	--属性上限
		['Time'] = {0,604800,2592000},	--有效时间
		['WingBinding'] = {10000,1000,5000},	--绑定元宝
		['Binding'] = {10000,1000,5000},	--元宝
		['Item'] = {},	--道具兑换
		['ItemNum'] = {},	--道具数量
		['ItemQuality'] = 3,	--道具品质
		['Lock'] = 2,	--锁定状态
		['ListAcc'] = 2,	--列表显示
		['PrintMounts'] = 1	--显示坐骑
		},
	[180000002] = {
		['ID'] = 180000002,
		['Name'] = '飞剑',
		['ResId'] = 130500002,
		['BaseAttribute'] = 102200001,
		['MountsAttribute'] = {21,46,2,3,4,15},
		['AttributeCeiling'] = {10,12,250,55,110,15},
		['Time'] = {0,604800,2592000},
		['WingBinding'] = {2380,480,980},
		['Binding'] = {2380,480,980},
		['Item'] = {201060196,201060196,201060196},
		['ItemNum'] = {50,10,30},
		['ItemQuality'] = 3,
		['Lock'] = 1,
		['ListAcc'] = 1,
		['PrintMounts'] = 1
		}
	}
return MountsTable
