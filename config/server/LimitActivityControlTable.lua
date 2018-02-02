local LimitActivityControlTable = {
	[631000001] = {
		['ID'] = 631000001,	--索引
		['Activity'] = 2,	--活动类型
		['OpenType'] = 1,	--开启类型
		['ContinuedOpenTime'] = 1,	--活动开启时间
		['ContinuedTime'] = 100,	--持续时间
		['Batch'] = 1	--批次
		},
	[631000002] = {
		['ID'] = 631000002,
		['Activity'] = 1,
		['OpenType'] = 1,
		['ContinuedOpenTime'] = 1,
		['ContinuedTime'] = 200,
		['Batch'] = 1
		}
	}
return LimitActivityControlTable
