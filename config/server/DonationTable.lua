local DonationTable = {
	[600200001] = {
		['ID'] = 600200001,	--索引
		['Type'] = 1,	--祈福类型
		['ConsumeType'] = 1,	--祈福消耗类型
		['ConsumeValue'] = 1000,	--消耗类型值
		['RewardType'] = {1,2,3},	--奖励类型
		['RewardValue'] = {10,50,10},	--奖励类型值
		['RewardEffect'] = 0	--普通同庆效果
		},
	[600200002] = {
		['ID'] = 600200002,
		['Type'] = 2,
		['ConsumeType'] = 4,
		['ConsumeValue'] = 50,
		['RewardType'] = {1,2,3},
		['RewardValue'] = {50,150,20},
		['RewardEffect'] = 0
		},
	[600200003] = {
		['ID'] = 600200003,
		['Type'] = 3,
		['ConsumeType'] = 4,
		['ConsumeValue'] = 200,
		['RewardType'] = {1,2,3},
		['RewardValue'] = {200,300,50},
		['RewardEffect'] = 1
		}
	}
return DonationTable
