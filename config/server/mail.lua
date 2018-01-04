local mail = {
	[270100001] = {
		['ID'] = 270100001,	--索引
		['MailType'] = 1,	--邮件类型
		['Title'] = '服务器更新说明',	--邮件标题
		['Content'] = '亲爱的玩家：\n服务器将于2017年1月3日10:30:00进行维护更新，预计时间2小时',	--邮件内容
		['Sender'] = '系统',	--发件人
		['RewardID'] = {},	--奖励ID
		['RewardNum'] = {},	--奖励数量
		['time'] = ''	--发送时间
		},
	[270200001] = {
		['ID'] = 270200001,
		['MailType'] = 2,
		['Title'] = '万妖录收集奖励',
		['Content'] = '恭喜你收集到%s张万妖卡，获得奖励如下：',
		['Sender'] = '系统',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270200002] = {
		['ID'] = 270200002,
		['MailType'] = 2,
		['Title'] = '通关副本获得道具',
		['Content'] = '由于您背包不足，副本内获得的以下道具由邮件形式给予',
		['Sender'] = '系统',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300003] = {
		['ID'] = 270300003,
		['MailType'] = 1,
		['Title'] = '帮会解散通知',
		['Content'] = '亲爱的玩家：\n您的帮会"%s"已解散，请重新加入其它帮会',
		['Sender'] = '系统',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300004] = {
		['ID'] = 270300004,
		['MailType'] = 1,
		['Title'] = '帮会任命通知',
		['Content'] = '亲爱的玩家：\n恭喜您，您被任命为"%s"帮会的长老',
		['Sender'] = '系统',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300005] = {
		['ID'] = 270300005,
		['MailType'] = 1,
		['Title'] = '帮会任命通知',
		['Content'] = '亲爱的玩家：\n恭喜您，您被任命为"%s"帮会的副帮主',
		['Sender'] = '系统',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300006] = {
		['ID'] = 270300006,
		['MailType'] = 1,
		['Title'] = '帮会撤职通知',
		['Content'] = '亲爱的玩家：\n您已被取消"%s"帮会的副帮主身份',
		['Sender'] = '系统',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300007] = {
		['ID'] = 270300007,
		['MailType'] = 1,
		['Title'] = '帮会撤职通知',
		['Content'] = '亲爱的玩家：\n您已被取消"%s"帮会的长老身份',
		['Sender'] = '系统',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300008] = {
		['ID'] = 270300008,
		['MailType'] = 1,
		['Title'] = '帮会开除通知',
		['Content'] = '亲爱的玩家：\n"%s"帮会管理员已将您开除帮会',
		['Sender'] = '系统',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300009] = {
		['ID'] = 270300009,
		['MailType'] = 1,
		['Title'] = '帮主转让通知',
		['Content'] = '亲爱的玩家：\n恭喜您，您被任命为"%s"帮会的帮主',
		['Sender'] = '系统',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300010] = {
		['ID'] = 270300010,
		['MailType'] = 1,
		['Title'] = '帮派每日福利',
		['Content'] = '亲爱的玩家：\n恭喜您，获得了今日帮派福利',
		['Sender'] = '系统',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300011] = {
		['ID'] = 270300011,
		['MailType'] = 3,
		['Title'] = '悬赏已过期，退回金额',
		['Content'] = '您在%s月%s日发布对%s玩家的悬赏令已过期，现退回悬赏金额',
		['Sender'] = '系统',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300012] = {
		['ID'] = 270300012,
		['MailType'] = 3,
		['Title'] = '悬赏已达成',
		['Content'] = '%s不负所托，在%s月%s日在%s完成了您发布对%s的悬赏',
		['Sender'] = '系统',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300013] = {
		['ID'] = 270300013,
		['MailType'] = 3,
		['Title'] = '官府悬赏奖励',
		['Content'] = '您在%s月%s日完成官府悬赏，特奉上奖励如下',
		['Sender'] = '系统',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300014] = {
		['ID'] = 270300014,
		['MailType'] = 3,
		['Title'] = '帮战·预赛奖励',
		['Content'] = '恭喜您所在的帮会在帮战·预赛中排第1名，奖励如下：',
		['Sender'] = '系统',
		['RewardID'] = {201061021},
		['RewardNum'] = {100},
		['time'] = ''
		},
	[270300015] = {
		['ID'] = 270300015,
		['MailType'] = 3,
		['Title'] = '帮战·预赛奖励',
		['Content'] = '恭喜您所在的帮会在帮战·预赛中排第2名，奖励如下：',
		['Sender'] = '系统',
		['RewardID'] = {201061021},
		['RewardNum'] = {80},
		['time'] = ''
		},
	[270300016] = {
		['ID'] = 270300016,
		['MailType'] = 3,
		['Title'] = '帮战·预赛奖励',
		['Content'] = '恭喜您所在的帮会在帮战·预赛中排第3名，奖励如下：',
		['Sender'] = '系统',
		['RewardID'] = {201061021},
		['RewardNum'] = {60},
		['time'] = ''
		},
	[270300017] = {
		['ID'] = 270300017,
		['MailType'] = 3,
		['Title'] = '帮战·预赛奖励',
		['Content'] = '恭喜您所在的帮会在帮战·预赛中排第4名，奖励如下：',
		['Sender'] = '系统',
		['RewardID'] = {201061021},
		['RewardNum'] = {50},
		['time'] = ''
		},
	[270300018] = {
		['ID'] = 270300018,
		['MailType'] = 3,
		['Title'] = '帮战·预赛奖励',
		['Content'] = '恭喜您所在的帮会在帮战·预赛中排第5-10名，奖励如下：',
		['Sender'] = '系统',
		['RewardID'] = {201061021},
		['RewardNum'] = {40},
		['time'] = ''
		},
	[270300019] = {
		['ID'] = 270300019,
		['MailType'] = 3,
		['Title'] = '帮战·预赛奖励',
		['Content'] = '恭喜您所在的帮会在帮战·预赛中排第11-20名，奖励如下：',
		['Sender'] = '系统',
		['RewardID'] = {201061021},
		['RewardNum'] = {30},
		['time'] = ''
		},
	[270300020] = {
		['ID'] = 270300020,
		['MailType'] = 3,
		['Title'] = '帮战·预赛奖励',
		['Content'] = '恭喜您所在的帮会在帮战·预赛中排第21-50名，奖励如下：',
		['Sender'] = '系统',
		['RewardID'] = {201061021},
		['RewardNum'] = {25},
		['time'] = ''
		},
	[270300021] = {
		['ID'] = 270300021,
		['MailType'] = 3,
		['Title'] = '帮战·预赛奖励',
		['Content'] = '恭喜您所在的帮会在帮战·预赛中排第51-100名，奖励如下：',
		['Sender'] = '系统',
		['RewardID'] = {201061021},
		['RewardNum'] = {20},
		['time'] = ''
		},
	[270300022] = {
		['ID'] = 270300022,
		['MailType'] = 3,
		['Title'] = '帮战·预赛奖励',
		['Content'] = '恭喜您所在的帮会在帮战·预赛中排第101-500名，奖励如下：',
		['Sender'] = '系统',
		['RewardID'] = {201061021},
		['RewardNum'] = {15},
		['time'] = ''
		},
	[270300023] = {
		['ID'] = 270300023,
		['MailType'] = 3,
		['Title'] = '帮战·决赛奖励',
		['Content'] = '恭喜您所在的帮会在帮战·预赛中排第500名++，奖励如下：',
		['Sender'] = '系统',
		['RewardID'] = {201061021},
		['RewardNum'] = {10},
		['time'] = ''
		},
	[270300024] = {
		['ID'] = 270300024,
		['MailType'] = 3,
		['Title'] = '帮战·决赛奖励',
		['Content'] = '恭喜您所在的帮会在帮战·决赛中排第1名，奖励如下：',
		['Sender'] = '系统',
		['RewardID'] = {201061020},
		['RewardNum'] = {50},
		['time'] = ''
		},
	[270300025] = {
		['ID'] = 270300025,
		['MailType'] = 3,
		['Title'] = '帮战·决赛奖励',
		['Content'] = '恭喜您所在的帮会在帮战·决赛中排第2名，奖励如下：',
		['Sender'] = '系统',
		['RewardID'] = {201061020},
		['RewardNum'] = {40},
		['time'] = ''
		},
	[270300026] = {
		['ID'] = 270300026,
		['MailType'] = 3,
		['Title'] = '帮战·决赛奖励',
		['Content'] = '恭喜您所在的帮会在帮战·决赛中排第3名，奖励如下：',
		['Sender'] = '系统',
		['RewardID'] = {201061020},
		['RewardNum'] = {30},
		['time'] = ''
		},
	[270300027] = {
		['ID'] = 270300027,
		['MailType'] = 3,
		['Title'] = '帮战·决赛奖励',
		['Content'] = '恭喜您所在的帮会在帮战·决赛中排第4名，奖励如下：',
		['Sender'] = '系统',
		['RewardID'] = {201061020},
		['RewardNum'] = {20},
		['time'] = ''
		},
	[270300028] = {
		['ID'] = 270300028,
		['MailType'] = 1,
		['Title'] = '开服礼包',
		['Content'] = '每天早上5:00发放',
		['Sender'] = '祁鹤然',
		['RewardID'] = {201070007},
		['RewardNum'] = {1},
		['time'] = ''
		},
	[270300029] = {
		['ID'] = 270300029,
		['MailType'] = 1,
		['Title'] = '您的坐骑已过期',
		['Content'] = '您的"%s"坐骑已过期',
		['Sender'] = '系统',
		['RewardID'] = {201070007},
		['RewardNum'] = {1},
		['time'] = ''
		},
	[270300030] = {
		['ID'] = 270300030,
		['MailType'] = 1,
		['Title'] = '伙伴招募获得',
		['Content'] = '由于您背包不足，副本内获得的以下道具由邮件形式给予',
		['Sender'] = '系统',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300031] = {
		['ID'] = 270300031,
		['MailType'] = 1,
		['Title'] = '好友赠送获得',
		['Content'] = '由于您背包不足，您的好友%s赠送的以下道具由邮件形式给予',
		['Sender'] = '系统',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300032] = {
		['ID'] = 270300032,
		['MailType'] = 3,
		['Title'] = '世界BOSS排名奖励',
		['Content'] = '恭喜您世界BOSS活动中排第1名，奖励如下：',
		['Sender'] = '虎子',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300033] = {
		['ID'] = 270300033,
		['MailType'] = 3,
		['Title'] = '世界BOSS排名奖励',
		['Content'] = '恭喜您世界BOSS活动中排第2名，奖励如下：',
		['Sender'] = '虎子',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300034] = {
		['ID'] = 270300034,
		['MailType'] = 3,
		['Title'] = '世界BOSS排名奖励',
		['Content'] = '恭喜您世界BOSS活动中排第3名，奖励如下：',
		['Sender'] = '虎子',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300035] = {
		['ID'] = 270300035,
		['MailType'] = 3,
		['Title'] = '世界BOSS排名奖励',
		['Content'] = '恭喜您世界BOSS活动中排第4-10名，奖励如下：',
		['Sender'] = '虎子',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300036] = {
		['ID'] = 270300036,
		['MailType'] = 3,
		['Title'] = '世界BOSS排名奖励',
		['Content'] = '恭喜您世界BOSS活动中排第11-30名，奖励如下：',
		['Sender'] = '虎子',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300037] = {
		['ID'] = 270300037,
		['MailType'] = 3,
		['Title'] = '世界BOSS排名奖励',
		['Content'] = '恭喜您世界BOSS活动中排第31-50名，奖励如下：',
		['Sender'] = '虎子',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300038] = {
		['ID'] = 270300038,
		['MailType'] = 3,
		['Title'] = '世界BOSS排名奖励',
		['Content'] = '恭喜您世界BOSS活动中排第51-70名，奖励如下：',
		['Sender'] = '虎子',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300039] = {
		['ID'] = 270300039,
		['MailType'] = 3,
		['Title'] = '世界BOSS排名奖励',
		['Content'] = '恭喜您世界BOSS活动中排第71-100名，奖励如下：',
		['Sender'] = '虎子',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300040] = {
		['ID'] = 270300040,
		['MailType'] = 3,
		['Title'] = '世界BOSS排名奖励',
		['Content'] = '恭喜您世界BOSS活动中获得参与奖，奖励如下：',
		['Sender'] = '虎子',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300041] = {
		['ID'] = 270300041,
		['MailType'] = 3,
		['Title'] = '世界BOSS击杀奖励',
		['Content'] = '恭喜您世界BOSS活动中击杀了BOSS，奖励如下：',
		['Sender'] = '虎子',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300042] = {
		['ID'] = 270300042,
		['MailType'] = 3,
		['Title'] = '门宗入侵活动奖励',
		['Content'] = '您在门宗入侵活动中获得奖励如下：',
		['Sender'] = '系统',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300043] = {
		['ID'] = 270300043,
		['MailType'] = 3,
		['Title'] = '十大门宗排名奖励',
		['Content'] = '恭喜您所在的门宗在“十大门宗”活动中，获得第%s的名次，奖励如下',
		['Sender'] = '系统',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270300044] = {
		['ID'] = 270300044,
		['MailType'] = 3,
		['Title'] = '门宗答题奖励',
		['Content'] = '您在门宗答题中获得的奖励如下，可要拿好咯！',
		['Sender'] = '门宗内政官',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270100002] = {
		['ID'] = 270100002,
		['MailType'] = 1,
		['Title'] = '拍卖行',
		['Content'] = '您的门宗在门宗活动中激活了门宗拍卖，请去拍卖行查看！',
		['Sender'] = '门宗主管',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270100003] = {
		['ID'] = 270100003,
		['MailType'] = 1,
		['Title'] = '拍卖行',
		['Content'] = '恭喜您从门宗拍卖场购买到%s，您所花费%s',
		['Sender'] = '门宗主管',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270100004] = {
		['ID'] = 270100004,
		['MailType'] = 1,
		['Title'] = '拍卖行',
		['Content'] = '恭喜您从世界拍卖场购买到%s，您所花费%s',
		['Sender'] = '门宗主管',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270100005] = {
		['ID'] = 270100005,
		['MailType'] = 1,
		['Title'] = '拍卖行',
		['Content'] = '恭喜您从门宗拍卖场竞拍到%s，您所花费%s',
		['Sender'] = '门宗主管',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270100006] = {
		['ID'] = 270100006,
		['MailType'] = 1,
		['Title'] = '拍卖行',
		['Content'] = '恭喜您从世界拍卖场竞拍到%s，您所花费%s',
		['Sender'] = '门宗主管',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270100007] = {
		['ID'] = 270100007,
		['MailType'] = 1,
		['Title'] = '拍卖行',
		['Content'] = '恭喜您在本次竞拍的%s，价格为%s，您的分红为%s',
		['Sender'] = '门宗主管',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270100008] = {
		['ID'] = 270100008,
		['MailType'] = 1,
		['Title'] = '拍卖行',
		['Content'] = '您在门宗拍卖竞价已被其他玩家超越。',
		['Sender'] = '门宗主管',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270100009] = {
		['ID'] = 270100009,
		['MailType'] = 1,
		['Title'] = '拍卖行',
		['Content'] = '您在世界拍卖竞价已被其他玩家超越。',
		['Sender'] = '门宗主管',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		},
	[270100010] = {
		['ID'] = 270100010,
		['MailType'] = 1,
		['Title'] = '排行榜排名奖励',
		['Content'] = '恭喜您在"%s"排行榜排名为"%s",奖励如下：',
		['Sender'] = '系统',
		['RewardID'] = {},
		['RewardNum'] = {},
		['time'] = ''
		}
	}
return mail
