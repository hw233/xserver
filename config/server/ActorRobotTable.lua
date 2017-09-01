local ActorRobotTable = {
	[107000001] = {
		['ID'] = 107000001,	--索引
		['WeaponId'] = {103010101,103010102},	--随机武器
		['HairResId'] = {103010201,103010202},	--随机头饰
		['ResId'] = {103010301,103010302},	--随机时装
		['InitialHead'] = 105900001,	--机器人初始头像
		['Skill'] = {111100103,111100157},	--主动技能
		['PassiveSkill'] = {0},	--被动技能
		['FightPro'] = {-100,100},	--战力系数
		['AttributeType'] = {2,3},	--分配的属性
		['AttributePro'] = {10,1},	--分配的比例
		['ActiveAttackRange'] = 5,	--主动攻击范围
		['ChaseRange'] = 10	--追击范围
		},
	[107000002] = {
		['ID'] = 107000002,
		['WeaponId'] = {103020101,103020102},
		['HairResId'] = {103020201,103020202},
		['ResId'] = {103020301,103020302},
		['InitialHead'] = 105900003,
		['Skill'] = {111100202,111100257},
		['PassiveSkill'] = {0},
		['FightPro'] = {-100,100},
		['AttributeType'] = {2,3},
		['AttributePro'] = {8,1},
		['ActiveAttackRange'] = 5,
		['ChaseRange'] = 10
		},
	[107000003] = {
		['ID'] = 107000003,
		['WeaponId'] = {103030101},
		['HairResId'] = {103030201},
		['ResId'] = {103030301},
		['InitialHead'] = 105900001,
		['Skill'] = {111100302,111100357},
		['PassiveSkill'] = {0},
		['FightPro'] = {-100,100},
		['AttributeType'] = {2,3},
		['AttributePro'] = {8,1},
		['ActiveAttackRange'] = 5,
		['ChaseRange'] = 10
		},
	[107000004] = {
		['ID'] = 107000004,
		['WeaponId'] = {103040101,103040102,103040103},
		['HairResId'] = {103040201,103040202},
		['ResId'] = {103040301},
		['InitialHead'] = 105900003,
		['Skill'] = {111100402,111100457},
		['PassiveSkill'] = {0},
		['FightPro'] = {-100,100},
		['AttributeType'] = {2,3},
		['AttributePro'] = {10,1},
		['ActiveAttackRange'] = 5,
		['ChaseRange'] = 10
		},
	[107000005] = {
		['ID'] = 107000005,
		['WeaponId'] = {103040101,103040102,103040103},
		['HairResId'] = {103040201,103040202},
		['ResId'] = {103040301},
		['InitialHead'] = 105900003,
		['Skill'] = {111100402,111100457},
		['PassiveSkill'] = {0},
		['FightPro'] = {-100,100},
		['AttributeType'] = {2,3},
		['AttributePro'] = {8,1},
		['ActiveAttackRange'] = 5,
		['ChaseRange'] = 10
		}
	}
return ActorRobotTable
