local BiaocheTable = {
	[440100001] = {
		['ID'] = 440100001,	--索引
		['Type'] = 1,	--镖车类型
		['ActivityControl'] = 330400035,	--控制表ID
		['TaskId'] = 240130014,	--任务ID
		['MonsterId'] = 151000047,	--镖车ID
		['Deposit'] = 1000,	--押金
		['StartTime'] = {0},	--多倍奖励开始时间
		['EndTime'] = {0},	--多倍奖励结束时间
		['shanfeiId'] = {151005042,151005043,151005044,151005045},	--劫镖山匪ID
		['level'] = {5,5},	--劫镖山匪出现等级段
		['Interval'] = 15,	--劫镖山匪出现间隔
		['Probability'] = 5000,	--劫镖山匪出现概率
		['Time'] = 5,	--劫镖山匪出现最大次数
		['Number'] = {2,3,4,5,6},	--劫镖山匪出现数量
		['Range'] = 3,	--劫镖山匪出现范围
		['Reward'] = 440200001	--奖励ID
		},
	[440100002] = {
		['ID'] = 440100002,
		['Type'] = 2,
		['ActivityControl'] = 330400036,
		['TaskId'] = 240130015,
		['MonsterId'] = 151000048,
		['Deposit'] = 5000,
		['StartTime'] = {1000},
		['EndTime'] = {1200},
		['shanfeiId'] = {0},
		['level'] = {0},
		['Interval'] = 0,
		['Probability'] = 0,
		['Time'] = 0,
		['Number'] = {0},
		['Range'] = 0,
		['Reward'] = 440200002
		}
	}
return BiaocheTable
