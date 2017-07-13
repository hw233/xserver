local BaseAI = {
	[153000001] = {
		['BaseID'] = 153000001,	--索引
		['ActiveAttackRange'] = 0,	--主动攻击范围
		['IsChase'] = 1,	--是否追击
		['ChaseRange'] = 20,	--追击范围
		['MovingChange'] = 0,	--移动增减值
		['Regeneration'] = 20,	--重生间隔
		['AIType'] = 1,	--AI类型
		['GuardRange'] = 4,	--巡逻范围
		['StopMin'] = 2000,	--巡逻停留最小值
		['StopMax'] = 6000,	--巡逻停留最大值
		['Response'] = 250	--AI响应时间系数
		},
	[153000002] = {
		['BaseID'] = 153000002,
		['ActiveAttackRange'] = 0,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 20,
		['AIType'] = 1,
		['GuardRange'] = 0,
		['StopMin'] = 2000,
		['StopMax'] = 6000,
		['Response'] = 250
		},
	[153000003] = {
		['BaseID'] = 153000003,
		['ActiveAttackRange'] = 20,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 20,
		['AIType'] = 1,
		['GuardRange'] = 4,
		['StopMin'] = 2000,
		['StopMax'] = 6000,
		['Response'] = 250
		},
	[153000004] = {
		['BaseID'] = 153000004,
		['ActiveAttackRange'] = 20,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 20,
		['AIType'] = 1,
		['GuardRange'] = 0,
		['StopMin'] = 2000,
		['StopMax'] = 6000,
		['Response'] = 250
		},
	[153000005] = {
		['BaseID'] = 153000005,
		['ActiveAttackRange'] = 10,
		['IsChase'] = 0,
		['ChaseRange'] = 0,
		['MovingChange'] = 0,
		['Regeneration'] = 20,
		['AIType'] = 1,
		['GuardRange'] = 0,
		['StopMin'] = 2000,
		['StopMax'] = 6000,
		['Response'] = 250
		},
	[153000006] = {
		['BaseID'] = 153000006,
		['ActiveAttackRange'] = 0,
		['IsChase'] = 0,
		['ChaseRange'] = 0,
		['MovingChange'] = 0,
		['Regeneration'] = 20,
		['AIType'] = 3,
		['GuardRange'] = 0,
		['StopMin'] = 2000,
		['StopMax'] = 6000,
		['Response'] = 250
		},
	[153000007] = {
		['BaseID'] = 153000007,
		['ActiveAttackRange'] = 0,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 20,
		['AIType'] = 2,
		['GuardRange'] = 0,
		['StopMin'] = 2000,
		['StopMax'] = 6000,
		['Response'] = 250
		},
	[153000008] = {
		['BaseID'] = 153000008,
		['ActiveAttackRange'] = 10,
		['IsChase'] = 0,
		['ChaseRange'] = 0,
		['MovingChange'] = -500,
		['Regeneration'] = 10,
		['AIType'] = 6,
		['GuardRange'] = 0,
		['StopMin'] = 2000,
		['StopMax'] = 6000,
		['Response'] = 250
		},
	[153000009] = {
		['BaseID'] = 153000009,
		['ActiveAttackRange'] = 5,
		['IsChase'] = 0,
		['ChaseRange'] = 0,
		['MovingChange'] = -500,
		['Regeneration'] = 10,
		['AIType'] = 4,
		['GuardRange'] = 0,
		['StopMin'] = 2000,
		['StopMax'] = 6000,
		['Response'] = 250
		},
	[153000010] = {
		['BaseID'] = 153000010,
		['ActiveAttackRange'] = 20,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 20,
		['AIType'] = 1,
		['GuardRange'] = 4,
		['StopMin'] = 2000,
		['StopMax'] = 6000,
		['Response'] = 50
		},
	[153000011] = {
		['BaseID'] = 153000011,
		['ActiveAttackRange'] = 0,
		['IsChase'] = 1,
		['ChaseRange'] = 10,
		['MovingChange'] = 0,
		['Regeneration'] = 45,
		['AIType'] = 1,
		['GuardRange'] = 0,
		['StopMin'] = 2000,
		['StopMax'] = 6000,
		['Response'] = 50
		},
	[153000012] = {
		['BaseID'] = 153000012,
		['ActiveAttackRange'] = 20,
		['IsChase'] = 1,
		['ChaseRange'] = 30,
		['MovingChange'] = 0,
		['Regeneration'] = 20,
		['AIType'] = 1,
		['GuardRange'] = 0,
		['StopMin'] = 2000,
		['StopMax'] = 6000,
		['Response'] = 5
		},
	[153000013] = {
		['BaseID'] = 153000013,
		['ActiveAttackRange'] = 20,
		['IsChase'] = 1,
		['ChaseRange'] = 30,
		['MovingChange'] = 0,
		['Regeneration'] = 20,
		['AIType'] = 1,
		['GuardRange'] = 4,
		['StopMin'] = 2000,
		['StopMax'] = 6000,
		['Response'] = 5
		},
	[153100101] = {
		['BaseID'] = 153100101,
		['ActiveAttackRange'] = 0,
		['IsChase'] = 0,
		['ChaseRange'] = 0,
		['MovingChange'] = 0,
		['Regeneration'] = 999,
		['AIType'] = 8,
		['GuardRange'] = 0,
		['StopMin'] = 2000,
		['StopMax'] = 6000,
		['Response'] = 10
		},
	[153100102] = {
		['BaseID'] = 153100102,
		['ActiveAttackRange'] = 20,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = -100,
		['Regeneration'] = 999,
		['AIType'] = 10,
		['GuardRange'] = 4,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 2
		},
	[153100103] = {
		['BaseID'] = 153100103,
		['ActiveAttackRange'] = 20,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 999,
		['AIType'] = 11,
		['GuardRange'] = 4,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 10
		},
	[153100104] = {
		['BaseID'] = 153100104,
		['ActiveAttackRange'] = 20,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 999,
		['AIType'] = 12,
		['GuardRange'] = 4,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 2
		},
	[153100105] = {
		['BaseID'] = 153100105,
		['ActiveAttackRange'] = 20,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 999,
		['AIType'] = 13,
		['GuardRange'] = 4,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 2
		},
	[153100106] = {
		['BaseID'] = 153100106,
		['ActiveAttackRange'] = 0,
		['IsChase'] = 0,
		['ChaseRange'] = 0,
		['MovingChange'] = 0,
		['Regeneration'] = 999,
		['AIType'] = 14,
		['GuardRange'] = 0,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 10
		},
	[153100107] = {
		['BaseID'] = 153100107,
		['ActiveAttackRange'] = 20,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 999,
		['AIType'] = 15,
		['GuardRange'] = 0,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 10
		},
	[153100108] = {
		['BaseID'] = 153100108,
		['ActiveAttackRange'] = 20,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 999,
		['AIType'] = 1,
		['GuardRange'] = 0,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 10
		},
	[153100109] = {
		['BaseID'] = 153100109,
		['ActiveAttackRange'] = 20,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 999,
		['AIType'] = 1,
		['GuardRange'] = 4,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 10
		},
	[153100110] = {
		['BaseID'] = 153100110,
		['ActiveAttackRange'] = 20,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 100,
		['Regeneration'] = 999,
		['AIType'] = 2,
		['GuardRange'] = 0,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 10
		},
	[153100201] = {
		['BaseID'] = 153100201,
		['ActiveAttackRange'] = 20,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 999,
		['AIType'] = 18,
		['GuardRange'] = 0,
		['StopMin'] = 2000,
		['StopMax'] = 6000,
		['Response'] = 5
		},
	[153100202] = {
		['BaseID'] = 153100202,
		['ActiveAttackRange'] = 20,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 999,
		['AIType'] = 19,
		['GuardRange'] = 0,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 2
		},
	[153100203] = {
		['BaseID'] = 153100203,
		['ActiveAttackRange'] = 20,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 999,
		['AIType'] = 20,
		['GuardRange'] = 4,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 2
		},
	[153100204] = {
		['BaseID'] = 153100204,
		['ActiveAttackRange'] = 20,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 999,
		['AIType'] = 21,
		['GuardRange'] = 4,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 5
		},
	[153100205] = {
		['BaseID'] = 153100205,
		['ActiveAttackRange'] = 20,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 999,
		['AIType'] = 17,
		['GuardRange'] = 0,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 10
		},
	[153100206] = {
		['BaseID'] = 153100206,
		['ActiveAttackRange'] = 5,
		['IsChase'] = 0,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 999,
		['AIType'] = 16,
		['GuardRange'] = 1,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 10
		},
	[153100207] = {
		['BaseID'] = 153100207,
		['ActiveAttackRange'] = 5,
		['IsChase'] = 0,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 999,
		['AIType'] = 1,
		['GuardRange'] = 0,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 10
		},
	[153100208] = {
		['BaseID'] = 153100208,
		['ActiveAttackRange'] = 5,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 999,
		['AIType'] = 2,
		['GuardRange'] = 0,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 10
		},
	[153100301] = {
		['BaseID'] = 153100301,
		['ActiveAttackRange'] = 0,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 20,
		['AIType'] = 1,
		['GuardRange'] = 2,
		['StopMin'] = 1000,
		['StopMax'] = 5000,
		['Response'] = 250
		},
	[153100302] = {
		['BaseID'] = 153100302,
		['ActiveAttackRange'] = 0,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 20,
		['AIType'] = 1,
		['GuardRange'] = 0,
		['StopMin'] = 1000,
		['StopMax'] = 5000,
		['Response'] = 250
		},
	[153100303] = {
		['BaseID'] = 153100303,
		['ActiveAttackRange'] = 10,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 20,
		['AIType'] = 1,
		['GuardRange'] = 2,
		['StopMin'] = 300,
		['StopMax'] = 800,
		['Response'] = 50
		},
	[153100304] = {
		['BaseID'] = 153100304,
		['ActiveAttackRange'] = 10,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = -100,
		['Regeneration'] = 20,
		['AIType'] = 1,
		['GuardRange'] = 2,
		['StopMin'] = 300,
		['StopMax'] = 800,
		['Response'] = 50
		},
	[153100305] = {
		['BaseID'] = 153100305,
		['ActiveAttackRange'] = 20,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 999,
		['AIType'] = 1,
		['GuardRange'] = 4,
		['StopMin'] = 300,
		['StopMax'] = 800,
		['Response'] = 50
		},
	[153100306] = {
		['BaseID'] = 153100306,
		['ActiveAttackRange'] = 3,
		['IsChase'] = 1,
		['ChaseRange'] = 5,
		['MovingChange'] = 0,
		['Regeneration'] = 999,
		['AIType'] = 1,
		['GuardRange'] = 0,
		['StopMin'] = 300,
		['StopMax'] = 800,
		['Response'] = 10
		},
	[153100401] = {
		['BaseID'] = 153100401,
		['ActiveAttackRange'] = 10,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 999,
		['AIType'] = 16,
		['GuardRange'] = 0,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 10
		},
	[153100402] = {
		['BaseID'] = 153100402,
		['ActiveAttackRange'] = 10,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 200,
		['Regeneration'] = 999,
		['AIType'] = 22,
		['GuardRange'] = 1,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 10
		},
	[153100403] = {
		['BaseID'] = 153100403,
		['ActiveAttackRange'] = 10,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = 0,
		['Regeneration'] = 999,
		['AIType'] = 1,
		['GuardRange'] = 1,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 10
		},
	[153100501] = {
		['BaseID'] = 153100501,
		['ActiveAttackRange'] = 20,
		['IsChase'] = 1,
		['ChaseRange'] = 20,
		['MovingChange'] = -200,
		['Regeneration'] = 999,
		['AIType'] = 2,
		['GuardRange'] = 0,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 50
		},
	[153100502] = {
		['BaseID'] = 153100502,
		['ActiveAttackRange'] = 0,
		['IsChase'] = 0,
		['ChaseRange'] = 0,
		['MovingChange'] = -100,
		['Regeneration'] = 999,
		['AIType'] = 14,
		['GuardRange'] = 0,
		['StopMin'] = 100,
		['StopMax'] = 500,
		['Response'] = 50
		},
	[153100503] = {
		['BaseID'] = 153100503,
		['ActiveAttackRange'] = 30,
		['IsChase'] = 1,
		['ChaseRange'] = 30,
		['MovingChange'] = 100,
		['Regeneration'] = 999,
		['AIType'] = 23,
		['GuardRange'] = 4,
		['StopMin'] = 1000,
		['StopMax'] = 2000,
		['Response'] = 50
		},
	[153100504] = {
		['BaseID'] = 153100504,
		['ActiveAttackRange'] = 30,
		['IsChase'] = 1,
		['ChaseRange'] = 30,
		['MovingChange'] = 100,
		['Regeneration'] = 999,
		['AIType'] = 23,
		['GuardRange'] = 0,
		['StopMin'] = 1000,
		['StopMax'] = 2000,
		['Response'] = 50
		}
	}
return BaseAI
