local DegreeWayTable = {
	[380300001] = {
		['ID'] = 380300001,	--索引
		['Type'] = 1,	--类型
		['Value1'] = {20001,20002},	--对应类型值1
		['Value2'] = {1,1},	--对应类型值2
		['DegreeValue'] = 5,	--提升好感度
		['DegreeLimit'] = 10	--途经提升上限
		},
	[380300002] = {
		['ID'] = 380300002,
		['Type'] = 2,
		['Value1'] = {2},
		['Value2'] = {2},
		['DegreeValue'] = 5,
		['DegreeLimit'] = 10
		},
	[380300003] = {
		['ID'] = 380300003,
		['Type'] = 3,
		['Value1'] = {10},
		['Value2'] = {0},
		['DegreeValue'] = 5,
		['DegreeLimit'] = 10
		},
	[380300004] = {
		['ID'] = 380300004,
		['Type'] = 4,
		['Value1'] = {5},
		['Value2'] = {0},
		['DegreeValue'] = 5,
		['DegreeLimit'] = 10
		}
	}
return DegreeWayTable
