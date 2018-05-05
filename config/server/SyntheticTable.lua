local SyntheticTable = {
	[651000001] = {
		['ID'] = 651000001,	--索引
		['SyntheticTarget'] = 201060002,	--合成目标
		['SyntheticMaterial'] = {201060001},	--合成材料
		['SyntheticMaterialNum'] = {3},	--合成材料所需数量
		['Consume'] = 1000	--单次合成所需银票数量
		},
	[651000002] = {
		['ID'] = 651000002,
		['SyntheticTarget'] = 201060003,
		['SyntheticMaterial'] = {201060002},
		['SyntheticMaterialNum'] = {3},
		['Consume'] = 3000
		},
	[651000003] = {
		['ID'] = 651000003,
		['SyntheticTarget'] = 201060004,
		['SyntheticMaterial'] = {201060003},
		['SyntheticMaterialNum'] = {3},
		['Consume'] = 5000
		},
	[651000004] = {
		['ID'] = 651000004,
		['SyntheticTarget'] = 201060005,
		['SyntheticMaterial'] = {201060020},
		['SyntheticMaterialNum'] = {4},
		['Consume'] = 2000
		},
	[651000005] = {
		['ID'] = 651000005,
		['SyntheticTarget'] = 201060006,
		['SyntheticMaterial'] = {201060005},
		['SyntheticMaterialNum'] = {3},
		['Consume'] = 4000
		},
	[651000006] = {
		['ID'] = 651000006,
		['SyntheticTarget'] = 201060007,
		['SyntheticMaterial'] = {201060006},
		['SyntheticMaterialNum'] = {2},
		['Consume'] = 6000
		},
	[651000008] = {
		['ID'] = 651000008,
		['SyntheticTarget'] = 201050124,
		['SyntheticMaterial'] = {201050121,201050122,201050123},
		['SyntheticMaterialNum'] = {1,1,1},
		['Consume'] = 0
		}
	}
return SyntheticTable
