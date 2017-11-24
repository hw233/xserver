local BiaocheRewardTable = {
	[440200001] = {
		['ID'] = 440200001,	--索引
		['Deposit'] = 1000,	--押金
		['StartTime'] = {0},	--多倍奖励开始时间
		['EndTime'] = {0},	--多倍奖励结束时间
		['RewardMoney1'] = 800,	--银两*等级
		['RewardExp1'] = 3000,	--经验*等级
		['RewardLv1'] = 0,	--绑定元宝*等级
		['RewardItem1'] = {0},	--物品ID
		['RewardNum1'] = {0},	--物品数量
		['Collection'] = 0,	--采集物ID
		['RewardMoney2'] = 0,	--银两*等级
		['RewardExp2'] = 0,	--经验*等级
		['RewardLv2'] = 0,	--绑定元宝*等级
		['RewardItem2'] = {0},	--物品ID
		['RewardNum2'] = {0}	--物品数量
		},
	[440200002] = {
		['ID'] = 440200002,
		['Deposit'] = 5000,
		['StartTime'] = {1000},
		['EndTime'] = {1200},
		['RewardMoney1'] = 1200,
		['RewardExp1'] = 2300,
		['RewardLv1'] = 0,
		['RewardItem1'] = {201010021},
		['RewardNum1'] = {5},
		['Collection'] = 154101001,
		['RewardMoney2'] = 0,
		['RewardExp2'] = 0,
		['RewardLv2'] = 0,
		['RewardItem2'] = {0},
		['RewardNum2'] = {0}
		}
	}
return BiaocheRewardTable
