local SkillLevelTable = {
	[1] = {
		['LowLevel'] = 1,	--低级技能等级
		['LowNeedExp'] = 15,	--低级技能升级经验
		['PartnerLevel'] = 1,	--低级技能伙伴等级需求
		['HighLevel'] = 1,	--高级技能等级
		['HighNeedExp'] = 15,	--高级技能升级经验
		['PartnerLevel1'] = 1	--高级技能伙伴等级需求
		},
	[2] = {
		['LowLevel'] = 2,
		['LowNeedExp'] = 20,
		['PartnerLevel'] = 2,
		['HighLevel'] = 2,
		['HighNeedExp'] = 20,
		['PartnerLevel1'] = 3
		},
	[3] = {
		['LowLevel'] = 3,
		['LowNeedExp'] = 30,
		['PartnerLevel'] = 3,
		['HighLevel'] = 3,
		['HighNeedExp'] = 30,
		['PartnerLevel1'] = 5
		},
	[4] = {
		['LowLevel'] = 4,
		['LowNeedExp'] = 40,
		['PartnerLevel'] = 4,
		['HighLevel'] = 4,
		['HighNeedExp'] = 40,
		['PartnerLevel1'] = 7
		},
	[5] = {
		['LowLevel'] = 5,
		['LowNeedExp'] = 50,
		['PartnerLevel'] = 5,
		['HighLevel'] = 5,
		['HighNeedExp'] = 50,
		['PartnerLevel1'] = 9
		},
	[6] = {
		['LowLevel'] = 6,
		['LowNeedExp'] = 60,
		['PartnerLevel'] = 6,
		['HighLevel'] = 6,
		['HighNeedExp'] = 60,
		['PartnerLevel1'] = 11
		},
	[7] = {
		['LowLevel'] = 7,
		['LowNeedExp'] = 70,
		['PartnerLevel'] = 7,
		['HighLevel'] = 7,
		['HighNeedExp'] = 70,
		['PartnerLevel1'] = 13
		},
	[8] = {
		['LowLevel'] = 8,
		['LowNeedExp'] = 80,
		['PartnerLevel'] = 8,
		['HighLevel'] = 8,
		['HighNeedExp'] = 80,
		['PartnerLevel1'] = 15
		},
	[9] = {
		['LowLevel'] = 9,
		['LowNeedExp'] = 90,
		['PartnerLevel'] = 9,
		['HighLevel'] = 9,
		['HighNeedExp'] = 90,
		['PartnerLevel1'] = 17
		},
	[10] = {
		['LowLevel'] = 10,
		['LowNeedExp'] = 100,
		['PartnerLevel'] = 10,
		['HighLevel'] = 10,
		['HighNeedExp'] = 100,
		['PartnerLevel1'] = 19
		},
	[11] = {
		['LowLevel'] = 11,
		['LowNeedExp'] = 110,
		['PartnerLevel'] = 11,
		['HighLevel'] = 11,
		['HighNeedExp'] = 110,
		['PartnerLevel1'] = 21
		},
	[12] = {
		['LowLevel'] = 12,
		['LowNeedExp'] = 120,
		['PartnerLevel'] = 12,
		['HighLevel'] = 12,
		['HighNeedExp'] = 120,
		['PartnerLevel1'] = 23
		},
	[13] = {
		['LowLevel'] = 13,
		['LowNeedExp'] = 130,
		['PartnerLevel'] = 13,
		['HighLevel'] = 13,
		['HighNeedExp'] = 130,
		['PartnerLevel1'] = 25
		},
	[14] = {
		['LowLevel'] = 14,
		['LowNeedExp'] = 140,
		['PartnerLevel'] = 14,
		['HighLevel'] = 14,
		['HighNeedExp'] = 140,
		['PartnerLevel1'] = 27
		},
	[15] = {
		['LowLevel'] = 15,
		['LowNeedExp'] = 150,
		['PartnerLevel'] = 15,
		['HighLevel'] = 15,
		['HighNeedExp'] = 150,
		['PartnerLevel1'] = 29
		},
	[16] = {
		['LowLevel'] = 16,
		['LowNeedExp'] = 160,
		['PartnerLevel'] = 16,
		['HighLevel'] = 16,
		['HighNeedExp'] = 160,
		['PartnerLevel1'] = 31
		},
	[17] = {
		['LowLevel'] = 17,
		['LowNeedExp'] = 170,
		['PartnerLevel'] = 17,
		['HighLevel'] = 17,
		['HighNeedExp'] = 170,
		['PartnerLevel1'] = 33
		},
	[18] = {
		['LowLevel'] = 18,
		['LowNeedExp'] = 180,
		['PartnerLevel'] = 18,
		['HighLevel'] = 18,
		['HighNeedExp'] = 180,
		['PartnerLevel1'] = 35
		},
	[19] = {
		['LowLevel'] = 19,
		['LowNeedExp'] = 190,
		['PartnerLevel'] = 19,
		['HighLevel'] = 19,
		['HighNeedExp'] = 190,
		['PartnerLevel1'] = 37
		},
	[20] = {
		['LowLevel'] = 20,
		['LowNeedExp'] = 200,
		['PartnerLevel'] = 20,
		['HighLevel'] = 20,
		['HighNeedExp'] = 200,
		['PartnerLevel1'] = 39
		},
	[21] = {
		['LowLevel'] = 21,
		['LowNeedExp'] = 250,
		['PartnerLevel'] = 21,
		['HighLevel'] = 21,
		['HighNeedExp'] = 250,
		['PartnerLevel1'] = 41
		},
	[22] = {
		['LowLevel'] = 22,
		['LowNeedExp'] = 300,
		['PartnerLevel'] = 22,
		['HighLevel'] = 22,
		['HighNeedExp'] = 300,
		['PartnerLevel1'] = 43
		},
	[23] = {
		['LowLevel'] = 23,
		['LowNeedExp'] = 350,
		['PartnerLevel'] = 23,
		['HighLevel'] = 23,
		['HighNeedExp'] = 350,
		['PartnerLevel1'] = 45
		},
	[24] = {
		['LowLevel'] = 24,
		['LowNeedExp'] = 400,
		['PartnerLevel'] = 24,
		['HighLevel'] = 24,
		['HighNeedExp'] = 400,
		['PartnerLevel1'] = 47
		},
	[25] = {
		['LowLevel'] = 25,
		['LowNeedExp'] = 450,
		['PartnerLevel'] = 25,
		['HighLevel'] = 25,
		['HighNeedExp'] = 450,
		['PartnerLevel1'] = 49
		},
	[26] = {
		['LowLevel'] = 26,
		['LowNeedExp'] = 500,
		['PartnerLevel'] = 26,
		['HighLevel'] = 26,
		['HighNeedExp'] = 500,
		['PartnerLevel1'] = 51
		},
	[27] = {
		['LowLevel'] = 27,
		['LowNeedExp'] = 550,
		['PartnerLevel'] = 27,
		['HighLevel'] = 27,
		['HighNeedExp'] = 550,
		['PartnerLevel1'] = 53
		},
	[28] = {
		['LowLevel'] = 28,
		['LowNeedExp'] = 600,
		['PartnerLevel'] = 28,
		['HighLevel'] = 28,
		['HighNeedExp'] = 600,
		['PartnerLevel1'] = 55
		},
	[29] = {
		['LowLevel'] = 29,
		['LowNeedExp'] = 650,
		['PartnerLevel'] = 29,
		['HighLevel'] = 29,
		['HighNeedExp'] = 650,
		['PartnerLevel1'] = 57
		},
	[30] = {
		['LowLevel'] = 30,
		['LowNeedExp'] = 700,
		['PartnerLevel'] = 30,
		['HighLevel'] = 30,
		['HighNeedExp'] = 700,
		['PartnerLevel1'] = 59
		}
	}
return SkillLevelTable
