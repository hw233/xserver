local DonationTable = {
	[600200001] = {
		['ID'] = 600200001,	--索引
		['Type'] = 1,	--祈福类型
		['ConsumeType'] = 1,	--祈福消耗类型
		['ConsumeValue'] = 1000,	--消耗类型值
		['RewardType'] = {1,2,3},	--奖励类型
		['RewardValue'] = {100,1000,100},	--奖励类型值
		['RewardEffect'] = 0	--普通同庆效果
		},
	[600200002] = {
		['ID'] = 600200002,
		['Type'] = 2,
		['ConsumeType'] = 4,
		['ConsumeValue'] = 50,
		['RewardType'] = {1,2,3},
		['RewardValue'] = {600,5000,500},
		['RewardEffect'] = 0
		},
	[600200003] = {
		['ID'] = 600200003,
		['Type'] = 3,
		['ConsumeType'] = 4,
		['ConsumeValue'] = 200,
		['RewardType'] = {1,2,3},
		['RewardValue'] = {3000,20000,2000},
		['RewardEffect'] = 1
		}
	}
return DonationTable
