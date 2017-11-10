local GangsJurisdictionTable = {
	[1] = {
		['Position'] = 1,	--职位标识
		['Name'] = '宗主',	--门宗职位名称
		['Appoint'] = 1,	--任命
		['NoticeSetup'] = 1,	--公告设置
		['BuildingUp'] = 1,	--建筑升级
		['OpenActivity'] = 1,	--活动开启
		['RecruitSetup'] = 1,	--招募设置
		['skill'] = 1,	--技能研发
		['GangsName'] = 1,	--帮会名称
		['Recruit'] = 1,	--入宗审批
		['Expel'] = 1,	--请离门宗
		['Invitation'] = 1	--邀请
		},
	[2] = {
		['Position'] = 2,
		['Name'] = '副宗主',
		['Appoint'] = 1,
		['NoticeSetup'] = 1,
		['BuildingUp'] = 2,
		['OpenActivity'] = 1,
		['RecruitSetup'] = 1,
		['skill'] = 2,
		['GangsName'] = 1,
		['Recruit'] = 1,
		['Expel'] = 2,
		['Invitation'] = 1
		},
	[3] = {
		['Position'] = 3,
		['Name'] = '长老',
		['Appoint'] = 2,
		['NoticeSetup'] = 1,
		['BuildingUp'] = 2,
		['OpenActivity'] = 2,
		['RecruitSetup'] = 1,
		['skill'] = 2,
		['GangsName'] = 2,
		['Recruit'] = 1,
		['Expel'] = 2,
		['Invitation'] = 1
		},
	[4] = {
		['Position'] = 4,
		['Name'] = '门众',
		['Appoint'] = 2,
		['NoticeSetup'] = 2,
		['BuildingUp'] = 2,
		['OpenActivity'] = 2,
		['RecruitSetup'] = 2,
		['skill'] = 2,
		['GangsName'] = 2,
		['Recruit'] = 2,
		['Expel'] = 2,
		['Invitation'] = 2
		}
	}
return GangsJurisdictionTable
