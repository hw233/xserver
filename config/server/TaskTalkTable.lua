local TaskTalkTable = {
	[240400001] = {
		['ID'] = 240400001,	--对话ID
		['TaskID'] = 240100001,	--任务ID
		['TalkType'] = 1,	--对话类型
		['TalkObject'] = 152000001,	--说话人
		['TalkNote'] = '你去找一趟帆叔',	--说话内容
		['Voice'] = '',	--语音资源
		['TalkID'] = 240400002	--后续对话ID
		},
	[240400002] = {
		['ID'] = 240400002,
		['TaskID'] = 240100001,
		['TalkType'] = 1,
		['TalkObject'] = 0,
		['TalkNote'] = '好哒~',
		['Voice'] = '',
		['TalkID'] = 0
		},
	[240400003] = {
		['ID'] = 240400003,
		['TaskID'] = 240100001,
		['TalkType'] = 2,
		['TalkObject'] = 152000001,
		['TalkNote'] = '你怎么还不去找帆叔',
		['Voice'] = '',
		['TalkID'] = 0
		},
	[240400004] = {
		['ID'] = 240400004,
		['TaskID'] = 240100001,
		['TalkType'] = 3,
		['TalkObject'] = 152000003,
		['TalkNote'] = '你找错人了，我不是帆叔',
		['Voice'] = '',
		['TalkID'] = 240400005
		},
	[240400005] = {
		['ID'] = 240400005,
		['TaskID'] = 240100001,
		['TalkType'] = 3,
		['TalkObject'] = 0,
		['TalkNote'] = '哦，你不是啊',
		['Voice'] = '',
		['TalkID'] = 0
		},
	[240400006] = {
		['ID'] = 240400006,
		['TaskID'] = 240100002,
		['TalkType'] = 1,
		['TalkObject'] = 152000003,
		['TalkNote'] = '你帮我把东西送给NPC1号',
		['Voice'] = '',
		['TalkID'] = 240400007
		},
	[240400007] = {
		['ID'] = 240400007,
		['TaskID'] = 240100002,
		['TalkType'] = 1,
		['TalkObject'] = 0,
		['TalkNote'] = '好哒~',
		['Voice'] = '',
		['TalkID'] = 0
		},
	[240400008] = {
		['ID'] = 240400008,
		['TaskID'] = 240100002,
		['TalkType'] = 2,
		['TalkObject'] = 152000003,
		['TalkNote'] = '东西送到了么？',
		['Voice'] = '',
		['TalkID'] = 0
		},
	[240400009] = {
		['ID'] = 240400009,
		['TaskID'] = 240100002,
		['TalkType'] = 3,
		['TalkObject'] = 152000001,
		['TalkNote'] = '找到帆叔了么？',
		['Voice'] = '',
		['TalkID'] = 240400010
		},
	[240400010] = {
		['ID'] = 240400010,
		['TaskID'] = 240100002,
		['TalkType'] = 3,
		['TalkObject'] = 0,
		['TalkNote'] = '木有，NPC2号有东西给你',
		['Voice'] = '',
		['TalkID'] = 240400011
		},
	[240400011] = {
		['ID'] = 240400011,
		['TaskID'] = 240100002,
		['TalkType'] = 3,
		['TalkObject'] = 152000001,
		['TalkNote'] = '那敢情好啊',
		['Voice'] = '',
		['TalkID'] = 0
		},
	[240400012] = {
		['ID'] = 240400012,
		['TaskID'] = 240100003,
		['TalkType'] = 1,
		['TalkObject'] = 152000001,
		['TalkNote'] = '你去杀五个怪，没杀完不要回来。',
		['Voice'] = '',
		['TalkID'] = 0
		},
	[240400013] = {
		['ID'] = 240400013,
		['TaskID'] = 240100003,
		['TalkType'] = 2,
		['TalkObject'] = 152000001,
		['TalkNote'] = '说了没杀完不要回来。',
		['Voice'] = '',
		['TalkID'] = 0
		},
	[240400014] = {
		['ID'] = 240400014,
		['TaskID'] = 240100003,
		['TalkType'] = 3,
		['TalkObject'] = 152000001,
		['TalkNote'] = '杀完了？',
		['Voice'] = '',
		['TalkID'] = 240400015
		},
	[240400015] = {
		['ID'] = 240400015,
		['TaskID'] = 240100003,
		['TalkType'] = 3,
		['TalkObject'] = 0,
		['TalkNote'] = '那必须的。',
		['Voice'] = '',
		['TalkID'] = 0
		},
	[240400016] = {
		['ID'] = 240400016,
		['TaskID'] = 240100004,
		['TalkType'] = 1,
		['TalkObject'] = 152000001,
		['TalkNote'] = '杀五个怪，收集五件道具。',
		['Voice'] = '',
		['TalkID'] = 0
		},
	[240400017] = {
		['ID'] = 240400017,
		['TaskID'] = 240100004,
		['TalkType'] = 2,
		['TalkObject'] = 152000001,
		['TalkNote'] = '事情办完了么？',
		['Voice'] = '',
		['TalkID'] = 0
		},
	[240400018] = {
		['ID'] = 240400018,
		['TaskID'] = 240100004,
		['TalkType'] = 3,
		['TalkObject'] = 152000003,
		['TalkNote'] = '办的不错，我很满意。',
		['Voice'] = '',
		['TalkID'] = 0
		},
	[240400019] = {
		['ID'] = 240400019,
		['TaskID'] = 240100005,
		['TalkType'] = 1,
		['TalkObject'] = 152000003,
		['TalkNote'] = '使用道具接任务',
		['Voice'] = '',
		['TalkID'] = 0
		},
	[240400020] = {
		['ID'] = 240400020,
		['TaskID'] = 240100005,
		['TalkType'] = 2,
		['TalkObject'] = 152000003,
		['TalkNote'] = '任务做完了没。',
		['Voice'] = '',
		['TalkID'] = 0
		},
	[240400021] = {
		['ID'] = 240400021,
		['TaskID'] = 240100005,
		['TalkType'] = 3,
		['TalkObject'] = 152000003,
		['TalkNote'] = '安全了，谢谢你。',
		['Voice'] = '',
		['TalkID'] = 0
		},
	[240400022] = {
		['ID'] = 240400022,
		['TaskID'] = 240100007,
		['TalkType'] = 1,
		['TalkObject'] = 152000002,
		['TalkNote'] = '给你一个buff啊。',
		['Voice'] = '',
		['TalkID'] = 0
		},
	[240400023] = {
		['ID'] = 240400023,
		['TaskID'] = 240100007,
		['TalkType'] = 2,
		['TalkObject'] = 152000002,
		['TalkNote'] = '带着buff去杀怪吧。',
		['Voice'] = '',
		['TalkID'] = 0
		},
	[240400024] = {
		['ID'] = 240400024,
		['TaskID'] = 240100007,
		['TalkType'] = 3,
		['TalkObject'] = 152000002,
		['TalkNote'] = '办的不错。',
		['Voice'] = '',
		['TalkID'] = 0
		},
	[240400025] = {
		['ID'] = 240400025,
		['TaskID'] = 240100008,
		['TalkType'] = 1,
		['TalkObject'] = 152000003,
		['TalkNote'] = '杀一个枪女。',
		['Voice'] = '',
		['TalkID'] = 0
		},
	[240400026] = {
		['ID'] = 240400026,
		['TaskID'] = 240100008,
		['TalkType'] = 2,
		['TalkObject'] = 152000003,
		['TalkNote'] = '干掉了么？',
		['Voice'] = '',
		['TalkID'] = 0
		},
	[240400027] = {
		['ID'] = 240400027,
		['TaskID'] = 240100008,
		['TalkType'] = 3,
		['TalkObject'] = 152000003,
		['TalkNote'] = '很好，我很满意。',
		['Voice'] = '',
		['TalkID'] = 0
		}
	}
return TaskTalkTable
