local MGLYmaoguiTable = {
	[560200001] = {
		['ID'] = 560200001,	--索引
		['MonsterID'] = 151005112,	--怪物ID
		['Separate'] = 50,	--分裂血量
		['SeparateMonster'] = {151005113},	--分裂怪物ID
		['SeparateNum'] = 2,	--分裂怪物数量
		['SeparateRange'] = 5,	--分裂怪物范围
		['Effects'] = 'Prefabs/Effect/ef_cjw_xiangying',	--特效路径
		['EffectsParameter'] = {180,3}	--特效参数
		},
	[560200002] = {
		['ID'] = 560200002,
		['MonsterID'] = 151005113,
		['Separate'] = 25,
		['SeparateMonster'] = {151005114,151005115,151005116,151005117,151005118,151005119},
		['SeparateNum'] = 2,
		['SeparateRange'] = 5,
		['Effects'] = 'Prefabs/Effect/ef_cjw_xiangying',
		['EffectsParameter'] = {180,3}
		}
	}
return MGLYmaoguiTable
