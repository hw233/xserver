#ifndef __MSG_ID_H__
#define __MSG_ID_H__

//登录模块
#define MSG_ID_LOGIN_REQUEST  			10000 //登录请求  LoginRequest
#define MSG_ID_LOGIN_ANSWER  			10001 //登录应答  LoginAnswer
#define MSG_ID_PLAYER_LIST_REQUEST  	10002 //角色列表请求  
#define MSG_ID_PLAYER_LIST_ANSWER  		10003 //角色列表应答  PlayerListAnswer
#define MSG_ID_PLAYER_CREATE_REQUEST 	10004 //创建角色请求  PlayerCreateRequest
#define MSG_ID_PLAYER_CREATE_ANSWER  	10005 //创建角色应答  PlayerCreateAnswer
#define MSG_ID_ENTER_GAME_REQUEST  		10006 //进入游戏请求  EnterGameRequest
#define MSG_ID_ENTER_GAME_ANSWER  		10007 //进入游戏应答  EnterGameAnswer
#define MSG_ID_ACCOUNT_LOGIN_AGAIN_NOTIFY  		10008 //账号再次登录通知
#define MSG_ID_HEARTBEAT_NOTIFY  		10009 //心跳

//移动模块（场景）
#define MSG_ID_MOVE_REQUEST 			10100 //移动请求
#define MSG_ID_MOVE_ANSWER  			10101 //移动回复 move_answer
#define MSG_ID_MOVE_NOTIFY  			10102 //移动通知
#define MSG_ID_SIGHT_CHANGED_NOTIFY 	10103 //视野变化
#define MSG_ID_ENTER_SCENE_READY_REQUEST 	10104 //通知后台已经准备好加入场景

#define MSG_ID_MOVE_START_REQUEST 			10105 //移动开始请求
#define MSG_ID_MOVE_START_ANSWER  			10106 //移动开始回复  move_answer
#define MSG_ID_MOVE_START_NOTIFY  			10107 //移动开始通知
#define MSG_ID_MOVE_STOP_REQUEST 			10108 //移动停止请求
#define MSG_ID_MOVE_STOP_ANSWER  			10109 //移动停止回复  move_answer
#define MSG_ID_MOVE_STOP_NOTIFY  			10110 //移动停止通知
#define MSG_ID_TRANSFER_REQUEST  			10111 //场景跳转的请求 scene_transfer_request
#define MSG_ID_TRANSFER_ANSWER  			10112 //场景跳转的回复 scene_transfer_answer
#define MSG_ID_SYNC_PLAYER_POS_NOTIFY  		10113 //同步某个玩家的坐标，组队同步队长坐标用
#define MSG_ID_MOVE_Y_START_REQUEST         10114 //开始Y轴的移动  move_y_start_request
#define MSG_ID_MOVE_Y_START_NOTIFY                  10115 //开始Y轴的移动  move_y_start_notify
#define MSG_ID_MOVE_Y_STOP_REQUEST                  10116 //停止Y轴的移动 move_y_stop_request
#define MSG_ID_MOVE_Y_STOP_NOTIFY                  10117  //停止Y轴的移动 move_y_stop_notify
//#define MSG_ID_ENTER_REGION_NOTIFY                 10118   //玩家进入特定区域通知 enter_region_notify
#define MSG_ID_TRANSFER_TO_PLAYER_SCENE_REQUEST                 10119   //玩家传送到其他玩家所在场景请求 transfer_to_player_scene_request
#define MSG_ID_TRANSFER_TO_PLAYER_SCENE_ANSWER                 10120   //玩家传送到其他玩家所在场景失败时候的应答 comm_answer, 1表示玩家不在线，2表示在同一场景，3表示在副本
#define MSG_ID_SPECIAL_MONSTER_POS_NOTIFY                   10121   //场景里特殊怪物位置信息通知 special_monster_pos_notify
#define MSG_ID_SPECIAL_PLAYER_POS_NOTIFY                   10122   //场景里特殊玩家位置信息通知 special_player_pos_notify
#define MSG_ID_ADD_NPC_NOTIFY	                           10123   //场景增加NPC sight_npc_info
#define MSG_ID_SIGHT_PLAYER_CHANGE_NOTIFY	           10124   //视野玩家信息变更通知 sight_player_info_change_notify
#define MSG_ID_MONSTER_ID_CHANGED_NOTIFY               10125   //视野中的怪物ID变更(怪物变身) sight_monster_id_changed_notify
#define MSG_ID_MONSTER_TALK_NOTIFY                     10126   //视野中的怪物冒泡说话  monster_talk_notify
#define MSG_ID_BOATING_START_REQUEST                   10127   //开始划船，从视野删除 boating_start_request
#define MSG_ID_BOATING_START_ANSWER                    10128   //comm_answer
#define MSG_ID_BOATING_STOP_REQUEST                    10129   //划船结束  NULL


//技能模块
#define MSG_ID_SKILL_CAST_REQUEST 		10200   //施法请求  skill_cast_request
#define MSG_ID_SKILL_CAST_ANSWER  		10201   //施法应答  comm_answer
#define MSG_ID_SKILL_CAST_NOTIFY  		10202   //施法通知  skill_cast_notify
#define MSG_ID_SKILL_HIT_REQUEST 		10203   //命中请求  skill_hit_request
#define MSG_ID_SKILL_HIT_ANSWER  		10204   //命中应答  comm_answer
#define MSG_ID_SKILL_HIT_NOTIFY  		10205   //命中通知  skill_hit_notify
#define MSG_ID_SKILL_HIT_IMMEDIATE_NOTIFY  		10206   //立即命中通知  skill_hit_immediate_notify
//#define MSG_ID_SKILL_CAST_DELAY_NOTIFY  		10207   //怪物释放技能，需要客户端确认命中 skill_cast_delay_notify
//#define MSG_ID_SKILL_HIT_DELAY_REQUEST  		10208   //客户端确认命中, skill_hit_delay_request
//#define MSG_ID_SKILL_MOVE_REQUEST               10209   //位移技能的位移请求 skill_move_request
//#define MSG_ID_SKILL_MOVE_NOTIFY                10210   //位移技能的位移广播 skill_move_notify
#define MSG_ID_ADD_BUFF_NOTIFY                10211   //添加buff通知，主要是道具buff使用 add_buff_notify
#define MSG_ID_DEL_BUFF_NOTIFY                10212   //删除buff通知，主要是buff时间没到，需要提前删除的buff add_buff_notify
#define MSG_ID_SKILL_LIST_NOTIFY                10213   //已学技能 skill_list
#define MSG_ID_LEARN_SKILL_REQUEST 		10214   // 升级技能请求 learn_skill_req
#define MSG_ID_LEARN_SKILL_ANSWER 		10215   // 升级技能返回 learn_skill_ans
#define MSG_ID_NEW_SKILL_NOTIFY                10216   //开放新技能 skill_list
#define MSG_ID_SET_FUWEN_REQUEST 		10217   // 设置符文请求 fuwen
#define MSG_ID_SET_FUWEN_ANSWER 		10218   // 设置符文返回 fuwen_ans
//#define MSG_ID_CALL_ATTACK_REQUEST 		10219   // 客户端发送召唤物攻击消息 skill_call_attack_request
//#define MSG_ID_CALL_ATTACK_NOTIFY 		10220   // 服务端发送召唤物攻击消息 skill_call_attack_notify
#define MSG_ID_BUFF_STATE_NOTIFY 		10221   // buff状态变化通知, 比如眩晕 buff_state_changed_notify
#define MSG_ID_OPEN_SKILL_FOR_GUIDE 		10222   // 新手副本开放技能 learn_skill_req
#define MSG_ID_PARTNER_SKILL_CAST_REQUEST 		10223   //伙伴施法请求  NULL
#define MSG_ID_PARTNER_SKILL_CAST_ANSWER  		10224   //伙伴施法应答  comm_answer, 190500320表示无目标
#define MSG_ID_ADD_SPEED_BUFF_REQUEST  		10225   //请求添加加速buff  NULL
#define MSG_ID_DEL_SPEED_BUFF_REQUEST  		10226   //请求删除加速buff  NULL
#define MSG_ID_LEARN_FUWEN_REQUEST 		10227   // 升级符文请求 learn_fuwen_req
#define MSG_ID_LEARN_FUWEN_ANSWER 		10228   // 升级符文返回 learn_fuwen_ans
#define MSG_ID_SET_SKILL_MENU_REQUEST 		10229   // 设置技能套餐请求 skill_menu
#define MSG_ID_SET_SKILL_MENU_ANSWER 		10230   // 设置技能套餐返回 skill_menu_ans
#define MSG_ID_SET_FUWEN_OLD_REQUEST 		10231   // 设置旧符文请求 learn_fuwen_req
#define MSG_ID_SET_FUWEN_OLD_ANSWER 		10232   // 设置旧符文返回 learn_fuwen_ans

//生活技能
#define MSG_ID_LEARN_LIVE_SKILL_REQUEST 	10250   // 升级生活技能请求 learn_live_req
#define MSG_ID_LEARN_LIVE_SKILL_ANSWER 		10251   // 升级生活技能返回 AnsLearnLiveSkill
#define MSG_ID_LIVE_SKILL_BREAK_REQUEST 	10252   // 突破生活技能请求 learn_live_req
#define MSG_ID_LIVE_SKILL_BREAK_ANSWER 		10253   // 突破生活技能返回 AnsLearnLiveSkill
#define MSG_ID_PRODUCE_MEDICINE_REQUEST 	10254   // 生产药品请求 learn_live_req
#define MSG_ID_PRODUCE_MEDICINE_ANSWER 		10255   // 生产药品返回 comm_answer
#define MSG_ID_LIVE_SKILL_NOTIFY            10256   // 生活技能信息 live_skill

//背包模块
#define MSG_ID_BAG_INFO_REQUEST				10300 //背包信息请求 NULL
#define MSG_ID_BAG_INFO_ANSWER				10301 //背包信息应答 BagInfoAnswer
#define MSG_ID_BAG_GRID_UPDATE_NOTIFY		10302 //背包格子更新 BagGrid
#define MSG_ID_BAG_UNLOCK_GRID_REQUEST		10303 //背包开格子请求 NULL
#define MSG_ID_BAG_UNLOCK_GRID_ANSWER		10304 //背包开格子应答 BagUnlockGridAnswer
#define MSG_ID_BAG_SELL_REQUEST     		10305 //背包出售物品请求 BagSellRequest
#define MSG_ID_BAG_SELL_ANSWER      		10306 //背包出售物品应答 BagSellAnswer
#define MSG_ID_BAG_USE_REQUEST     		    10307 //背包使用物品请求 BagUseRequest
#define MSG_ID_BAG_USE_ANSWER      	    	10308 //背包使用物品应答 BagUseAnswer
#define MSG_ID_BAG_STACK_REQUEST     		10309 //背包堆叠物品请求 BagStackRequest
#define MSG_ID_BAG_STACK_ANSWER      		10310 //背包堆叠物品应答 comm_answer
#define MSG_ID_BAG_TIDY_REQUEST      		10311 //背包整理请求 NULL
#define MSG_ID_BAG_TIDY_ANSWER      		10312 //背包整理应答 NULL
#define MSG_ID_BAG_SHOW_REQUEST      		10313 //展示物品请求 ShowItemRequest
#define MSG_ID_BAG_SHOW_ANSWER      		10314 //展示物品 ShowItemAnswer
#define MSG_ID_ITEM_FLOW_TO_BAG_NOTIFY      10315 //道具飞向背包通知 ItemFlowToBagNotify

//角色信息
#define MSG_ID_PLAYER_ATTR_NOTIFY           10400 //属性变更通知 playerattrnotify
#define MSG_ID_PLAYER_NAME_NOTIFY           10401 //名字变更通知 PlayerNameNotify
#define MSG_ID_PLAYER_RENAME_REQUEST        10402 //改名请求 PlayerRenameRequest
#define MSG_ID_PLAYER_RENAME_ANSWER         10403 //改名应答 comm_answer
#define MSG_ID_HEAD_ICON_REPLACE_REQUEST    10404 //更换头像请求 HeadIconReplaceRequest
#define MSG_ID_HEAD_ICON_REPLACE_ANSWER     10405 //更换头像应答 comm_answer
#define MSG_ID_HEAD_ICON_UNLOCK_NOTIFY      10406 //头像解锁通知 HeadIconUnlockNotify
#define MSG_ID_HEAD_ICON_OLD_REQUEST        10407 //头像新解锁状态消失请求 HeadIconReplaceRequest
#define MSG_ID_HEAD_ICON_OLD_ANSWER         10408 //头像新解锁状态消失应答 HeadIconOldAnswer
#define MSG_ID_HEAD_ICON_INFO_REQUEST       10409 //头像信息请求 NULL
#define MSG_ID_HEAD_ICON_INFO_ANSWER        10410 //头像信息应答 HeadIconInfoAnswer
//#define MSG_ID_SET_FASHION        10411 //设置时装 SetFashion
#define MSG_ID_PLAYER_ONLINE_STATE_NOTIFY   10415 //玩家在线状态 PlayerOnlineState
#define MSG_ID_SYSTEM_NOTICE_NOTIFY         10416 //系统提示通知 SystemNoticeNotify
#define MSG_ID_SAVE_CLIENT_DATA_REQUEST           10417 //保存客户端数据请求 SaveClientDataRequest
#define MSG_ID_SAVE_CLIENT_DATA_ANSWER            10418 //保存客户端数据应答 comm_answer
#define MSG_ID_LOAD_CLIENT_DATA_REQUEST           10419 //获取客户端数据请求 NULL
#define MSG_ID_LOAD_CLIENT_DATA_ANSWER            10420 //获取客户端数据应答 LoadClientDataAnswer
#define MSG_ID_SETTING_SWITCH_NOTIFY              10421 //设置开关信息通知 SettingSwitchNotify
#define MSG_ID_SETTING_TURN_SWITCH_REQUEST        10422 //设置开关信息请求 SettingTurnSwitchRequest
#define MSG_ID_SETTING_TURN_SWITCH_ANSWER         10423 //切换设置开关应答 SettingTurnSwitchAnswer
#define MSG_ID_TRANSFER_OUT_STUCK_REQUEST         10424 //脱离卡死请求 NULL
#define MSG_ID_TRANSFER_OUT_STUCK_ANSWER          10425 //脱离卡死应答 TransferOutStuckAnswer
#define MSG_ID_TRANSFER_OUT_STUCK_INFO_NOTIFY     10426 //脱离卡死信息通知 TransferOutStuckInfoNotify
#define MSG_ID_SERVER_LEVEL_INFO_NOTIFY     10427 //服务器等级信息通知 ServerLevelInfoNotify
#define MSG_ID_SERVER_LEVEL_BREAK_NOTIFY    10428 //服务器等级突破通知 NULL
#define MSG_ID_FISHING_REWARD_REQUEST        10429 //钓鱼收杆请求 FishingRewardRequest
#define MSG_ID_FISHING_REWARD_ANSWER         10430 //钓鱼收杆应答 FishingRewardAnswer
#define MSG_ID_FISHING_SET_BAIT_REQUEST      10431 //钓鱼设置鱼饵请求 FishingSetBaitRequest
#define MSG_ID_FISHING_SET_BAIT_ANSWER       10432 //钓鱼设置鱼饵应答 FishingSetBaitAnswer
#define MSG_ID_PLAY_DRAMA_BEGIN_REQUEST      10433 //播放剧情开始请求 NULL
#define MSG_ID_PLAY_DRAMA_END_REQUEST        10434 //播放剧情结束请求 NULL

//复活
#define MSG_ID_RELIVE_REQUEST               10500  //复活 relive_request
#define MSG_ID_RELIVE_ANSWER                10501
#define MSG_ID_RELIVE_NOTIFY                10502  //复活 relive_notify

//聊天
#define MSG_ID_CHAT_REQUEST               10550  //聊天 Chat
#define MSG_ID_CHAT_NOTIFY                10551  //聊天 Chat
#define MSG_ID_CHAT_ANSWER                10552  //聊天返回 comm_answer 0成功 1玩家不在线
#define MSG_ID_CHAT_BROADCAST_REQUEST                10553  //广播 Chat
#define MSG_ID_CHAT_BROADCAST_NOTIFY                10554  //广播 Chat
#define MSG_ID_CHAT_HORSE_NOTIFY                10555  //跑马灯 ChatHorse

//采集点
#define MSG_ID_COLLECT_BEGIN_REQUEST                10560  //开始采集 StartCollect
#define MSG_ID_COLLECT_BEGIN_ANSWER                10561  //开始采集 comm_answer 0成功 1:正被采集 2:等级不够
#define MSG_ID_COLLECT_BEGIN_NOTIFY                10562  //开始采集 NotifyCollect
#define MSG_ID_COLLECT_INTERRUPT_REQUEST                10563  //中断采集 CollectId
#define MSG_ID_COLLECT_INTERRUPT_NOTIFY                10564  //中断采集 NotifyCollect
#define MSG_ID_COLLECT_COMMPLETE_NOTIFY                10565  //采集完成 CollectComplete
#define MSG_ID_COLLECT_COMMPLETE_REQUEST                10566  //采集完成 CollectId
#define MSG_ID_COLLECT_COMMPLETE_ANSWER                10567  //采集完成 comm_answer 0:成功 1:不是采集者 2:时间未到

//任务
#define MSG_ID_TASK_UPDATE_NOTIFY           10600 //任务更新通知 TaskUpdateNotify
#define MSG_ID_TASK_LIST_REQUEST            10601 //任务列表请求 NULL
#define MSG_ID_TASK_LIST_ANSWER             10602 //任务列表应答 TaskListAnswer
#define MSG_ID_TASK_ACCEPT_REQUEST          10603 //接任务请求 TaskCommRequest
#define MSG_ID_TASK_ACCEPT_ANSWER           10604 //接任务应答 TaskCommAnswer
#define MSG_ID_TASK_SUBMIT_REQUEST          10605 //提交任务请求 TaskCommRequest
#define MSG_ID_TASK_SUBMIT_ANSWER           10606 //提交任务应答 TaskCommAnswer
#define MSG_ID_TASK_ABANDON_REQUEST         10607 //放弃任务请求 TaskCommRequest
#define MSG_ID_TASK_ABANDON_ANSWER          10608 //放弃任务应答 TaskCommAnswer
#define MSG_ID_TASK_MONSTER_REQUEST         10609 //任务刷怪请求 TaskEventRequest
#define MSG_ID_TASK_MONSTER_ANSWER          10610 //任务刷怪应答 TaskCommAnswer
#define MSG_ID_TASK_COMPLETE_REQUEST        10611 //任务完成请求 TaskCompleteRequest
#define MSG_ID_TASK_COMPLETE_ANSWER         10612 //任务完成应答 TaskCommAnswer
#define MSG_ID_TASK_REMOVE_NOTIFY           10613 //任务删除通知 TaskRemoveNotify
#define MSG_ID_TASK_FAIL_REQUEST            10614 //任务失败请求 TaskCommRequest
#define MSG_ID_TASK_FAIL_ANSWER             10615 //任务失败应答 TaskCommAnswer
#define MSG_ID_TASK_UPDATE_FINISH_NOTIFY    10616 //任务完成列表更新通知 TaskUpdateFinishNotify
#define MSG_ID_TASK_CHAPTER_REWARD_REQUEST  10617 //任务章节奖励请求 NULL
#define MSG_ID_TASK_CHAPTER_REWARD_ANSWER   10618 //任务章节奖励应答 comm_answer
#define MSG_ID_TASK_UPDATE_CHAPTER_REWARD_NOTIFY   10619 //任务章节奖励更新通知 TaskUpdateChapterRewardNotify
#define MSG_ID_TASK_ENTER_PLANES_REQUEST    10620 //任务进入位面请求 TaskCommRequest
#define MSG_ID_TASK_ENTER_PLANES_ANSWER     10621 //任务进入位面应答 TaskEnterPlanesAnswer

//组队
#define MSG_ID_CREATE_TEAM_REQUEST         10630 //创建队伍请求 TeamTarget
#define MSG_ID_CREATE_TEAM_ANSWER         10631 //创建队伍返回 comm_answer
#define MSG_ID_TEAM_INFO_NOTIFY         10632 //队伍信息 TeamInfo
#define MSG_ID_APPLY_TEAM_REQUEST         10633 //申请入队伍请求  Teamid
#define MSG_ID_APPLY_TEAM_ANSWER         10634 //申请入队伍返回  TeamApplyAnswer
#define MSG_ID_APPLY_TEAM_NOTIFY         10635 //申请入队伍通知  TeamMemInfo
#define MSG_ID_HANDLE_APPLY_TEAM_REQUEST         10636 //处理申请入队伍  HandleTeamApply
#define MSG_ID_HANDLE_APPLY_TEAM_ANSWER         10677 //处理申请入队伍  comm_answer
#define MSG_ID_TEAM_ADD_MEMBER_NOTIFY         10637 //增加队伍成员信息 TeamMemInfo
#define MSG_ID_INVITE_TEAM_REQUEST         10638 //邀请入队伍请求  TeamPlayerid
#define MSG_ID_INVITE_TEAM_NOTIFY         10639 //邀请入队伍请求  TeamInvite
#define MSG_ID_HANDLEINVITE_TEAM_REQUEST         10640 //处理邀请入队伍请求  HandleTeamInvite
#define MSG_ID_REFUCE_INVITE_TEAM_NOTIFY         10641 //通知拒绝邀请  TeamMemInfo
#define MSG_ID_QUIT_TEAM_REQUEST         10642 //退出队伍请求  NULL
#define MSG_ID_KICK_TEAM_REQUEST         10644 //T出队伍请求  TeamPlayerid
#define MSG_ID_REMOVE_TEAM_MEM_NITIFY         10645 //移除队员  DelTeamPlayer
#define MSG_ID_APPLYERLIST_TEAM_NOTIFY         10646 //申请列表信息 TeamApplyerList
#define MSG_ID_TEAM_BE_LEAD_REQUEST         10647 //请求当队长 NULL
#define MSG_ID_TEAM_BE_LEAD_ANSWER         10675 //请求当队长 TeamNotifyCd
#define MSG_ID_TEAM_CHANGE_LEAD_REQUEST         10648 //请求换队长 TeamPlayerid
#define MSG_ID_TEAM_CHANGE_LEAD_NOTIFY         10649 //换队长 TeamPlayerid
#define MSG_ID_TEAM_BE_LEAD_NOTIFY         10650 //请求当队长 TeamPlayerid
#define MSG_ID_TEAM_LIMITED_REQUEST         10651 //修改队伍限制请求 TeamLimited
#define MSG_ID_TEAM_LIMITED_NOTIFY         10652 //更新队伍限制 TeamLimited
#define MSG_ID_TEAM_LIST_REQUEST         10653 //队伍列表请求 TeamTarget
#define MSG_ID_TEAM_LIST_ANSWER        10654 //队伍列表应答 TeamList
#define MSG_ID_HANDLE_REFUCE_APPLY_TEAM_REQUEST         10655 //处理申请入队伍  NULL
#define MSG_ID_HANDLE_REFUCE_APPLY_TEAM_ANSWER         10656 //处理申请入队伍返回   NULL
#define MSG_ID_INVITE_TEAM_ANSWER         10657 //邀请入队伍返回  comm_answer
#define MSG_ID_TEAM_INFO_REQUEST         10658 //队伍信息请求 NULL
#define MSG_ID_TEAM_HANDLE_BE_LEAD_REQUEST         10659 //处理请求当队长 TeamBeLead
#define MSG_ID_TEAM_HANDLE_BE_LEAD_ANSWER         10668 //处理请求当队长 BeLeadAnswer 0成功 
#define MSG_ID_TEAM_REFUCE_BE_LEAD_NOTIFY         10660 //拒绝请求当队长 NULL
#define MSG_ID_TEAM_HP_NOTIFY         10661 //队员等级血量  TeamHp
#define MSG_ID_TEAM_MATCH_REQUEST         10662 //自动匹配  TeamTarget
#define MSG_ID_REFUCE_APPLY_TEAM_NOTIFY         10663 //拒绝申请入队伍通知  RefuceApplyTeam
#define MSG_ID_CHANGE_TEAMID_NOTIFY         10664 //队伍ID通知  ChangeTeamid
#define MSG_ID_TEAM_MATCH_ANSWER         10665 //自动匹配  MatchAnser
#define MSG_ID_TEAM_CANCEL_MATCH_REQUEST         10666 //取消自动匹配  NULL
#define MSG_ID_TEAM_CANCEL_MATCH_ANSWER         10667 //取消自动匹配  MatchAnser
#define MSG_ID_TEAM_LEAD_POS_REQUEST       10669 // 队长位置 NULL
#define MSG_ID_TEAM_LEAD_POS_ANSWER       10670 // 队长位置 sync_player_pos_notify
#define MSG_ID_TEAM_SET_FOLLOW_REQUEST       10671 // 设置跟随状态 Follow
#define MSG_ID_TEAM_SET_FOLLOW_NOTIFY       10672 // 更新跟随状态 Follow
#define MSG_ID_TEAM_SUMMON_MEM_REQUEST       10673 // 通知队员跟随
#define MSG_ID_TEAM_SUMMON_MEM_NOTIFY       10674 // 
#define MSG_ID_TEAM_SPEEK_CD_NOTIFY       10676 // 自动喊话CD TeamNotifyCd
#define MSG_ID_TEAM_SPEEK_REQUEST       10678 //喊话 Chat
#define MSG_ID_DISBAND_TEAM_NOTIFY         10657 //解散队伍  comm_answer
#define MSG_ID_TEAM_SUMMON_MEM_ANSWER       10679 // 通知队员跟随 BeLeadAnswer
#define MSG_ID_IS_TEAM_LEAD_NOTIFY         10680 //当队长通知  TeamPlayerid
#define MSG_ID_NOT_TEAM_LEAD_NOTIFY         10681 //不当队长通知  TeamPlayerid


//吟唱
#define MSG_ID_SING_BEGIN_NOTIFY            10700 //吟唱开始通知 SingNotify
#define MSG_ID_SING_INTERRUPT_NOTIFY        10701 //吟唱中断通知 SingNotify
#define MSG_ID_SING_END_NOTIFY              10702 //吟唱结束通知 SingNotify
#define MSG_ID_SING_INTERRUPT_REQUEST       10703 //吟唱中断请求 NULL
#define MSG_ID_SING_INTERRUPT_ANSWER        10704 //吟唱中断应答 comm_answer
#define MSG_ID_SING_END_REQUEST             10705 //吟唱结束请求 NULL
#define MSG_ID_SING_END_ANSWER              10706 //吟唱结束应答 comm_answer
#define MSG_ID_SING_BEGIN_REQUEST           10707 //吟唱开始请求 SingBeginRequest
#define MSG_ID_SING_BEGIN_ANSWER            10708 //吟唱开始应答 comm_answer

//副本模块
#define MSG_ID_ENTER_RAID_REQUEST            10800 //进入副本请求  enter_raid_request
#define MSG_ID_ENTER_RAID_ANSWER             10801 //进入副本回复  enter_raid_answer
#define MSG_ID_ENTER_RAID_NOTIFY             10802 //进入副本通知  enter_raid_notify
#define MSG_ID_TRANSFER_TO_LEADER_NOTIFY     10803 //传送至队长通知 team_member_refuse_transfer_notify
#define MSG_ID_TRANSFER_TO_LEADER_REQUEST    10804 //传送至队长请求 transfer_to_leader_request
#define MSG_ID_LEAVE_RAID_REQUEST            10805 //离开副本请求 NULL
#define MSG_ID_LEAVE_RAID_ANSWER             10806 //离开副本回复 comm_answer (暂时没用到)
#define MSG_ID_LEAVE_RAID_NOTIFY             10807 //离开副本通知 NULL 
#define MSG_ID_ENTER_PLANES_RAID_NOTIFY             10808 //进入位面通知 EnterPlanesRaid
#define MSG_ID_LEAVE_PLANES_RAID_NOTIFY             10809 //离开位面通知 NULL
#define MSG_ID_RAID_EARNING_TIMES_CHANGED_NOTIFY             10810 //副本收益次数变更通知  raid_earning_times_changed_notify
#define MSG_ID_RAID_HIT_STATIS_CHANGED_NOTIFY             10811 //副本统计信息变更  raid_hit_statis_changed_notify
#define MSG_ID_RAID_FINISHED_NOTIFY             10812 //副本结算通知  raid_finish_notify
#define MSG_ID_LEAVE_PLANES_RAID_REQUEST             10813 //离开位面请求 NULL
#define MSG_ID_TRANSFER_FAR_TEAM_MEMBER_REQUEST             10814 //队长请求传送远离自己的队员到自己附近 NULL
#define MSG_ID_TEAM_MEMBER_REFUSE_TRANSFER_NOTIFY          10815  //通知队长队员不同意传送  team_member_refuse_transfer_notify
#define MSG_ID_RAID_PASS_PARAM_CHANGED_NOTIFY            10816   //副本通关参数变更  raid_pass_param_changed_notify
#define MSG_ID_RAID_REFRESH_NPC_NOTIFY                          10817   //刷新NPC   raid_refresh_npc_notify
#define MSG_ID_RAID_REFRESH_TRANSFER_NOTIFY                     10818   //刷新传送点   raid_refresh_transfer_notify
#define MSG_ID_RAID_REFRESH_NPC_ALL_NOTIFY                          10819   //刷新全部NPC   NULL
#define MSG_ID_RAID_REFRESH_TRANSFER_ALL_NOTIFY                     10820   //刷新全部传送点   NULL
#define MSG_ID_RAID_SHOW_HIDE_AIR_WALL_NOTIFY                     10821   //显示或者隐藏空气墙通知 raid_show_hide_air_wall_notify
#define MSG_ID_RAID_EVENT_NOTIFY             10822 //副本事件通知 RaidEventNotify
#define MSG_ID_XUNBAO_FB_COUNTDOWN_NOTIFY            10823 //fb CD  FbCD 寻宝副本结束时间通知前端
#define MSG_ID_RAID_SHOW_COUNTDOWN_NOTIFY     10824  //副本显示倒计时 raid_show_countdown_notify
#define MSG_ID_RAID_STAR_CHANGED_NOTIFY     10825  //副本星级变化通知 raid_star_changed_notify
#define MSG_ID_TEAM_RAID_WAIT_READY_NOTIFY           10826   //组队副本等待准备通知 team_raid_wait_ready_notify
#define MSG_ID_TEAM_RAID_READY_REQUEST           10827   //组队副本准备请求
#define MSG_ID_TEAM_RAID_READY_NOTIFY           10828   //组队副本准备通知   team_raid_ready_notify
#define MSG_ID_TEAM_RAID_CANCEL_REQUEST           10829   //组队副本取消准备请求
#define MSG_ID_TEAM_RAID_CANCEL_NOTIFY           10830   //组队副本取消准备通知 team_raid_cancel_notify
#define MSG_ID_NPC_TALK_REQUEST           10831   //告知后台和某个NPC对话了 npc_talk_request
#define MSG_ID_RAID_AI_CONTINUE_REQUEST		   10832 //客户端执行完特定的副本ai请求继续副本   RaidAiContinueRequest
#define MSG_ID_TRANSFER_TO_LEADER_ANSWER    10833 //传送至队长应答 comm_answer

//装备
#define MSG_ID_EQUIP_LIST_REQUEST              10900 //装备信息请求 NULL 
#define MSG_ID_EQUIP_LIST_ANSWER               10901 //装备信息应答 EquipListAnswer
#define MSG_ID_EQUIP_STAR_UP_REQUEST           10902 //装备升星请求 EquipStarUpRequest 
#define MSG_ID_EQUIP_STAR_UP_ANSWER            10903 //装备升星应答 EquipStarUpAnswer
#define MSG_ID_EQUIP_STAIR_UP_REQUEST          10904 //装备进阶请求 NULL 
#define MSG_ID_EQUIP_STAIR_UP_ANSWER           10905 //装备进阶应答 comm_answer
#define MSG_ID_EQUIP_ENCHANT_REQUEST           10906 //装备附魔请求 EquipEnchantRequest 
#define MSG_ID_EQUIP_ENCHANT_ANSWER            10907 //装备附魔应答 EquipEnchantAnswer
#define MSG_ID_EQUIP_ENCHANT_RETAIN_REQUEST    10908 //装备附魔保留请求 EquipEnchantRetainRequest
#define MSG_ID_EQUIP_ENCHANT_RETAIN_ANSWER     10909 //装备附魔保留应答 EquipEnchantRetainAnswer
#define MSG_ID_EQUIP_DRILL_REQUEST             10910 //装备解锁孔位请求 EquipDrillRequest 
#define MSG_ID_EQUIP_DRILL_ANSWER              10911 //装备解锁孔位应答 EquipDrillAnswer
#define MSG_ID_EQUIP_INLAY_REQUEST             10912 //装备镶嵌请求 EquipInlayRequest 
#define MSG_ID_EQUIP_INLAY_ANSWER              10913 //装备镶嵌应答 EquipInlayAnswer
#define MSG_ID_EQUIP_STRIP_REQUEST             10914 //装备剥离请求 EquipStripRequest
#define MSG_ID_EQUIP_STRIP_ANSWER              10915 //装备剥离应答 EquipStripAnswer
#define MSG_ID_EQUIP_GEM_COMPOSE_REQUEST       10916 //宝石合成请求 EquipGemComposeRequest
#define MSG_ID_EQUIP_GEM_COMPOSE_ANSWER        10917 //宝石合成应答 EquipGemComposeAnswer
#define MSG_ID_EQUIP_ADD_NOTIFY                10918 //装备开启通知 EquipData
#define MSG_ID_EQUIP_GEM_ONEKEY_COMPOSE_REQUEST       10919 //宝石一键合成请求 EquipGemOnekeyComposeRequest
#define MSG_ID_EQUIP_GEM_ONEKEY_COMPOSE_ANSWER        10920 //宝石一键合成应答 EquipGemOnekeyComposeAnswer

//邮件
#define MSG_ID_MAIL_INSERT_NOTIFY              11000 //新邮件通知 MailData
#define MSG_ID_MAIL_LIST_REQUEST               11001 //邮件列表请求 NULL 
#define MSG_ID_MAIL_LIST_ANSWER                11002 //邮件列表应答 MailListAnswer
#define MSG_ID_MAIL_READ_REQUEST               11003 //邮件阅读请求 MailCommRequest
#define MSG_ID_MAIL_READ_ANSWER                11004 //邮件阅读应答 MailCommAnswer
#define MSG_ID_MAIL_GET_ATTACH_REQUEST         11005 //邮件领取附件请求 MailCommRequest
#define MSG_ID_MAIL_GET_ATTACH_ANSWER          11006 //邮件领取附件应答 MailMultiAnswer
#define MSG_ID_MAIL_DEL_REQUEST                11007 //邮件删除请求 MailCommRequest
#define MSG_ID_MAIL_DEL_ANSWER                 11008 //邮件删除应答 MailMultiAnswer

//时装
#define MSG_ID_FASHION_LIST_NOTIFY              11100 //时装列表 FashionList
#define MSG_ID_BUY_FASHION_REQUEST              11101 //买时装 BuyFashion
#define MSG_ID_BUY_FASHION_ANSWER              11102   //BuyFashionAnswer
#define MSG_ID_PUTON_FASHION_REQUEST              11103   //穿时装 PutonFashion
#define MSG_ID_PUTON_FASHION_ANSWER              11104    //PutonFashionAns
#define MSG_ID_UNLOCK_COLOR_REQUEST              11105 //解锁颜色 UnlockColor
#define MSG_ID_UNLOCK_COLOR_ANSWER              11106   //UnlockColorAns
#define MSG_ID_COLOR_LIST_NOTIFY              11107 //颜色列表 ColorList
#define MSG_ID_TAKEOFF_FASHION_REQUEST              11108   //脱时装 PutonFashion
#define MSG_ID_TAKEOFF_FASHION_ANSWER              11109   //PutonFashionAns
#define MSG_ID_SET_FASHION_COLOR_ANSWER              11110  //SetFashionColorAnswer
#define MSG_ID_SET_FASHION_COLOR_REQUEST              11111 //时装染色 SetFashionColor
#define MSG_ID_FASHION_OLD_REQUEST              11112   //取消NEW标记 PutonFashion
#define MSG_ID_FASHION_OLD_ANSWER              11113   //PutonFashionAns
#define MSG_ID_SET_COLOR_OLD_ANSWER              11114  //UnlockColor
#define MSG_ID_SET_COLOR_OLD_REQUEST              11115 //时装染色 UnlockColorAns
#define MSG_ID_FACTION_CHARM_NOTIFY              11116 //魅力值 FashionCharm
#define MSG_ID_SET_WEAPON_COLOR_REQUEST              11117 //武器染色 SetFashionColor
#define MSG_ID_SET_WEAPON_COLOR_ANSWER              11118  //SetFashionColorAnswer
#define MSG_ID_WEAPON_COLOR_LIST_NOTIFY              11119 //颜色列表 ColorList
#define MSG_ID_SET_WEAPON_COLOR_OLD_ANSWER              11120  //UnlockColor
#define MSG_ID_SET_WEAPON_COLOR_OLD_REQUEST              11121 //时装染色 UnlockColorAns
#define MSG_ID_UNLOCK_WEAPON_COLOR_REQUEST              11122 //解锁颜色 UnlockColor
#define MSG_ID_UNLOCK_WEAPON_COLOR_ANSWER              11123   //UnlockColorAns


//坐骑
#define MSG_ID_HORSE_LIST_NOTIFY              11200 //已有坐骑列表 HorseList
#define MSG_ID_BUY_HORSE_REQUEST              11201 //买坐骑 BuyHorse
#define MSG_ID_BUY_HORSE_ANSWER              11202 //comm_answer
#define MSG_ID_SET_CUR_HORSE_REQUEST              11203 //幻化 HorseId
#define MSG_ID_SET_CUR_HORSE_ANSWER              11204 //SetCurHorseAns
#define MSG_ID_ADD_HORSE_EXP_REQUEST              11205 //修灵 HorseAttr
#define MSG_ID_ADD_HORSE_EXP_ANSWER              11206 //HorseAttrAns
#define MSG_ID_ADD_HORSE_STEP_REQUEST              11207 //升阶 HorseId
#define MSG_ID_ADD_HORSE_STEP_ANSWER              11208 //HorseStepAns
#define MSG_ID_ADD_HORSE_SOUL_REQUEST              11209 //铸灵 HorseId
#define MSG_ID_ADD_HORSE_SOUL_ANSWER              11210 //HorseSoulAns
#define MSG_ID_ON_HORSE_REQUEST              11211 // 上坐骑 OnHouseRequest
#define MSG_ID_ON_HORSE_NOTIFY              11212 // 上坐骑 OnHorse
#define MSG_ID_DOWN_HORSE_REQUEST              11213 // 下坐骑 NULL
#define MSG_ID_DOWN_HORSE_NOTIFY              11214 // 下坐骑 OnHorse
#define MSG_ID_SET_HORSE_OLD_REQUEST              11215 //置旧 HorseId
#define MSG_ID_SET_HORSE_OLD_ANSWER              11216 //HorseId
#define MSG_ID_ADD_HORSE_SOUL_LEVEL_REQUEST              11217 //铸灵进阶 NULL
#define MSG_ID_ADD_HORSE_SOUL_LEVEL_ANSWER              11218 //HorseSoulAns
#define MSG_ID_SET_HORSE_FLY_REQUEST              11219 //FlyState
#define MSG_ID_HORSE_RESTORE_REQUEST              11220 //还原
#define MSG_ID_ADD_HORSE_NOTIFY              11221 //增加或更新一个坐骑 BuyHorseAns

//商城
#define MSG_ID_SHOP_INFO_REQUEST                11300 //获取商城信息请求 NULL
#define MSG_ID_SHOP_INFO_ANSWER                 11301 //获取商城信息应答 ShopInfoAnswer
#define MSG_ID_SHOP_BUY_REQUEST                 11302 //购买商城商品请求 ShopBuyRequest
#define MSG_ID_SHOP_BUY_ANSWER                  11303 //购买商城商品应答 ShopBuyAnswer
#define MSG_ID_SHOP_GOODS_NOTIFY                11304 //商城商品信息通知 ShopGoodsNotify
#define MSG_ID_GUILD_SHOP_INFO_REQUEST          11310 //帮贡商店信息请求 NULL
#define MSG_ID_SPECIAL_SHOP_INFO_ANSWER         11311 //帮贡商店信息应答 SpecialShopInfoAnswer
#define MSG_ID_GUILD_SHOP_BUY_REQUEST           11312 //帮贡商店购买请求 ShopBuyRequest

//万妖谷
#define MSG_ID_LIST_WANYAOKA_REQUEST                11400 //获取万妖卡列表 NULL
#define MSG_ID_LIST_WANYAOKA_ANSWER                 11401 //获取万妖卡列表 list_wanyaoka_answer
#define MSG_ID_WANYAOKA_START_NOTIFY                 11402 //万妖谷关卡开始通知 wanyaogu_start_notify
#define MSG_ID_WANYAOKA_BBQ_NOTIFY                 11403 //万妖谷关卡火炉挂机通知 wanyaogu_bbq_notify
#define MSG_ID_WANYAOKA_GET_NOTIFY                 11404 //获得万妖卡通知 wanyaoka_get_notify

//御气道
#define MSG_ID_YUQIDAO_INFO_REQUEST             11500 //获取御气道信息请求 NULL
#define MSG_ID_YUQIDAO_INFO_ANSWER              11501 //获取御气道信息应答 YuqidaoInfoAnswer
#define MSG_ID_YUQIDAO_FILL_REQUEST             11502 //御气道灌入真气请求 YuqidaoFillRequest
#define MSG_ID_YUQIDAO_FILL_ANSWER              11503 //御气道灌入真气应答 YuqidaoFillAnswer
#define MSG_ID_YUQIDAO_BREAK_REQUEST            11504 //御气道突破请求 YuqidaoBreakRequest
#define MSG_ID_YUQIDAO_BREAK_ANSWER             11505 //御气道突破应答 YuqidaoBreakAnswer
#define MSG_ID_YUQIDAO_BREAK_RETAIN_REQUEST     11506 //御气道突破保留请求 YuqidaoBreakRetainRequest
#define MSG_ID_YUQIDAO_BREAK_RETAIN_ANSWER      11507 //御气道突破保留应答 YuqidaoBreakRetainAnswer
#define MSG_ID_YUQIDAO_MAI_OPEN_NOTIFY	        11508 //御气道经脉开启通知 YuqidaoMaiOpenNotify

//PK模块
#define MSG_ID_SET_PK_TYPE_REQUEST              11601  //设置PK状态请求 set_pk_type_request
#define MSG_ID_SET_PK_TYPE_ANSWER               11602  //设置PK状态应答 set_pk_type_answer
#define MSG_ID_QIECUO_REQUEST                   11603  //发起切磋请求 qiecuo_request
#define MSG_ID_QIECUO_ANSWER                    11604  //发起切磋应答 comm_answer
#define MSG_ID_QIECUO_NOTIFY                    11605  //发起切磋提示 qiecuo_notify
#define MSG_ID_QIECUO_START_REQUEST             11606  //同意某人的切磋请求, 即开始切磋 qiecuo_start_request
#define MSG_ID_QIECUO_START_ANSWER              11607  //同意某人的切磋应答(失败返回该消息，成功则直接发送MSG_ID_QIECUO_START_NOTIFY) comm_answer
#define MSG_ID_QIECUO_START_NOTIFY              11608  //同意切磋后，通知开始切磋 qiecuo_start_notify
#define MSG_ID_QIECUO_OUT_RANGE_NOTIFY          11609  //切磋距离太远的通知 NULL
#define MSG_ID_QIECUO_FINISH_NOTIFY             11610  //切磋结束的通知 qiecuo_finish_notify
#define MSG_ID_QIECUO_REFUSE_REQUEST            11611  //拒绝切磋的请求 qicuo_refuse_request
#define MSG_ID_QIECUO_REFUSE_NOTIFY             11612  //拒绝切磋的请求 qicuo_refuse_notify
#define MSG_ID_QIECUO_IN_RANGE_NOTIFY           11613  //走回切磋圈的通知 NULL

//PVP活动
#define MSG_ID_PVP_MATCH_START_REQUEST                11700  //进入匹配系统请求 pvp_match_request
#define MSG_ID_PVP_MATCH_START_ANSWER                 11701  //进入匹配系统应答 pvp_match_answer

#define MSG_ID_PVP_SCORE_CHANGED_NOTIFY               11705  //段位积分变化通知 pvp_score_changed_notify
#define MSG_ID_PVP_RANK_REQUEST                       11706  //请求PVP排行榜  pvp_rank_request
#define MSG_ID_PVP_RANK_ANSWER                        11707  //PVP排行榜应答  pvp_rank_answer

#define MSG_ID_PVP_OPEN_LEVEL_REWARD_REQUEST                   11708  //开启PVP段位奖励请求 pvp_open_level_reward_request
#define MSG_ID_PVP_OPEN_LEVEL_REWARD_ANSWER                    11709  //开启PVP段位奖励应答  pvp_open_level_reward_answer
#define MSG_ID_PVP_OPEN_DAILY_BOX_REQUEST                   11710  //开启PVP每日宝箱请求 pvp_open_daily_box_request
#define MSG_ID_PVP_OPEN_DAILY_BOX_ANSWER                    11711  //开启PVP每日宝箱应答  pvp_open_daily_box_answer

#define MSG_ID_PVP_MATCH_SUCCESS_NOTIFY          11712   //匹配成功通知 pvp_match_success_notify
#define MSG_ID_PVP_MATCH_READY_REQUEST           11713   //准备请求
#define MSG_ID_PVP_MATCH_READY_NOTIFY           11714   //准备通知   pvp_match_ready_notify
#define MSG_ID_PVP_MATCH_CANCEL_REQUEST           11715   //取消准备请求
#define MSG_ID_PVP_MATCH_CANCEL_NOTIFY           11716   //取消准备通知 pvp_match_cancel_notify
#define MSG_ID_PVP_RAID_KILL_NOTIFY                   11717   //PVP副本击杀通知 pvp_kill_notify
#define MSG_ID_PVP_RAID_FINISHED_NOTIFY              11718  //PVP副本结算通知 pvp_raid_finish_notify
#define MSG_ID_PVP_RAID_PRAISE_REQUEST              11719  //PVP副本点赞请求 pvp_raid_praise_request
#define MSG_ID_PVP_RAID_PRAISE_NOTIFY              11720  //PVP副本点赞通知 pvp_raid_praise_notify
#define MSG_ID_PVP_RAID_PLAYER_FALL_NOTIFY         11721  //PVP副本玩家掉落悬崖通知 pvp_raid_player_fall_notify
#define MSG_ID_PVP_RAID_BUFF_GET_NOTIFY         11722  //PVP副本玩家获得红蓝buff通知 pvp_raid_buff_get_notify
#define MSG_ID_PVP_RAID_ACE_NOTIFY         11723  //PVP副本团灭对方通知 NULL 
#define MSG_ID_PVP_RAID_BE_ACE_NOTIFY         11724  //PVP副本被团灭通知 NULL
#define MSG_ID_PVP_RAID_START_NOTIFY                 11725 //PVP副本开始通知 pvp_raid_start_notify
#define MSG_ID_PVP_RAID_BUFF_RELIVE_TIME_NOTIFY      11726  //PVP副本阴阳区域中心buff重生时间刷新 pvp_raid_buff_relive_time_notify

//八卦牌
#define MSG_ID_BAGUAPAI_INFO_REQUEST              11800 //获取八卦牌信息请求 NULL
#define MSG_ID_BAGUAPAI_INFO_ANSWER               11801 //获取八卦牌信息应答 BaguapaiInfoAnswer
#define MSG_ID_BAGUAPAI_SWITCH_REQUEST            11802 //八卦牌切换请求 BaguapaiSwitchRequest
#define MSG_ID_BAGUAPAI_SWITCH_ANSWER             11803 //八卦牌切换应答 BaguapaiSwitchAnswer
#define MSG_ID_BAGUAPAI_WEAR_REQUEST              11804 //八卦牌更换请求 BaguapaiWearRequest
#define MSG_ID_BAGUAPAI_WEAR_ANSWER               11805 //八卦牌更换应答 BaguapaiWearAnswer
#define MSG_ID_BAGUAPAI_DECOMPOSE_REQUEST         11806 //八卦牌分解请求 BaguapaiDecomposeRequest
#define MSG_ID_BAGUAPAI_DECOMPOSE_ANSWER          11807 //八卦牌分解应答 BaguapaiDecomposeAnswer
#define MSG_ID_BAGUAPAI_REFINE_STAR_REQUEST       11808 //八卦牌炼星请求 BaguapaiRefineCommonRequest
#define MSG_ID_BAGUAPAI_REFINE_STAR_ANSWER        11809 //八卦牌炼星应答 BaguapaiRefineStarAnswer
#define MSG_ID_BAGUAPAI_REFINE_MAIN_ATTR_REQUEST  11810 //八卦牌重铸请求 BaguapaiRefineCommonRequest
#define MSG_ID_BAGUAPAI_REFINE_MAIN_ATTR_ANSWER   11811 //八卦牌重铸应答 BaguapaiRefineMainAttrAnswer
#define MSG_ID_BAGUAPAI_RETAIN_MAIN_ATTR_REQUEST  11812 //八卦牌重铸保留请求 BaguapaiRefineCommonRequest
#define MSG_ID_BAGUAPAI_RETAIN_MAIN_ATTR_ANSWER   11813 //八卦牌重铸保留应答 BaguapaiRetainMainAttrAnswer
#define MSG_ID_BAGUAPAI_REFINE_MINOR_ATTR_REQUEST 11814 //八卦牌洗炼请求 BaguapaiRefineCommonRequest
#define MSG_ID_BAGUAPAI_REFINE_MINOR_ATTR_ANSWER  11815 //八卦牌洗炼应答 BaguapaiRefineMinorAttrAnswer
#define MSG_ID_BAGUAPAI_RETAIN_MINOR_ATTR_REQUEST 11816 //八卦牌洗炼保留请求 BaguapaiRefineCommonRequest
#define MSG_ID_BAGUAPAI_RETAIN_MINOR_ATTR_ANSWER  11817 //八卦牌洗炼保留应答 BaguapaiRetainMinorAttrAnswer

//妖师客栈——国御
#define MSG_ID_YAOSHI_NOTIFY              11900 //妖师客栈 Yaoshi
#define MSG_ID_ACCECT_GUOYU_TASK_REQUEST            11901 //接受国御任务 ReqChoseGuoyuTask
#define MSG_ID_GUOYU_CRITICAL_CD_NOTIFY            11902 //紧急任务 CD  GuoyuFb
#define MSG_ID_GUOYU_TASK_NOTIFY            11903 //接受国御任务 UpdateGuoyuTask
#define MSG_ID_GIVEUP_GUOYU_TASK_REQUEST            11904 //放弃国御任务 NULL
#define MSG_ID_GIVEUP_GUOYU_TASK_ANSWER            11905 //放弃国御任务 GiveupGuoyuTask
#define MSG_ID_SET_SPECIAL_REQUEST            11906 //设置专精 ReqSetYaoshiSpecial
#define MSG_ID_SET_SPECIAL_ANSWER            11907 //设置专精 AnsYaoshiSpecial
#define MSG_ID_ACCECT_GUOYU_TASK_NOTIFY            11908 //接受国御任务 NULL
#define MSG_ID_AGREED_GUOYU_TASK_REQUEST            11909 //同意接受国御任务 GuoyuSucc
//#define MSG_ID_ENTER_GUOYU_FB_REQUEST            11908 //进入fb NULL
//#define MSG_ID_ENTER_GUOYU_FB_ANSWER            11909 //进入fb comm_answer
#define MSG_ID_YAOSHI_EXP_NOTIFY            11910 //妖师经验等级 YaoshiLevelExp
#define MSG_ID_YAOSHI_NUM_NOTIFY            11911 //妖师收益次数 YaoshiNumber
#define MSG_ID_GUOYU_TASK_SUCC_NOTIFY            11912 //任务成功或失败 GuoyuSucc
#define MSG_ID_GUOYU_BOSS_APPEAR_REQUEST            11913 //发现boss BossId
#define MSG_ID_GUOYU_BOSS_APPEAR_NOTIFY            11914 //发现boss BossId
#define MSG_ID_GUOYU_FB_SUCC_NOTIFY            11915 //fb成功或失败 GuoyuFbSucc
#define MSG_ID_GUOYU_FB_CD_NOTIFY            11916 //fb CD  GuoyuFb
#define MSG_ID_GUOYU_FB_COLSE_NOTIFY            11917 //fb关闭 GuoyuFbSucc
#define MSG_ID_REFUCE_GUOYU_TASK_NOTIFY            11918 //拒绝国御任务 GuoyuName
//妖师客栈——惩戒
#define MSG_ID_REFRESH_CHENGJIE_LIST_REQUEST            11923 //刷新惩戒列表 ChengjieRefreshType
#define MSG_ID_REFRESH_CHENGJIE_LIST_ANSWER            11924 //刷新惩戒列表 ChengjieList
#define MSG_ID_ADD_CHENGJIE_TASK_REQUEST            11925 //发布惩戒任务 ReqAddChengjieTask
#define MSG_ID_ADD_CHENGJIE_TASK_ANSWER            11926 //发布惩戒任务 AnsAddChengjieTask
#define MSG_ID_ACCEPT_CHENGJIE_TASK_REQUEST            11927 //接受惩戒任务 ReqAcceptChengjieTask
#define MSG_ID_ACCEPT_CHENGJIE_TASK_ANSWER            11928 //接受惩戒任务 AnsAcceptChengjieTask
#define MSG_ID_CHENGJIE_FIND_TARGET_REQUEST            11929 //查找目标 ReqFindTarget
#define MSG_ID_CHENGJIE_FIND_TARGET_ANSWER            11930 //查找目标 AnsFindTarget
#define MSG_ID_SUBMIT_CHENGJIE_TASK_REQUEST            11931 //提交惩戒任务 NULL
#define MSG_ID_SUBMIT_CHENGJIE_TASK_ANSWER            11932 //提价惩戒任务 AnsAcceptChengjieTask
#define MSG_ID_CUR_CHENGJIE_TASK_NOTIFY            11933 //当前任务 ChengjieTask
#define MSG_ID_CHENGJIE_TASK_COMPLETE_NOTIFY            11934 //当前任务成功或失败 ChengjieTaskSucc
#define MSG_ID_CHENGJIE_TARGET_LOGIN_NOTIFY            11935 //目标上线 ChengjieKiller
#define MSG_ID_CHENGJIE_KILLER_NOTIFY            11936 //接受悬赏任务者 ChengjieKiller
//妖师客栈——赏金
#define MSG_ID_REFRESH_SHANGJIN_TASK_REQUEST            11943 //刷新赏金任务 NULL
#define MSG_ID_REFRESH_SHANGJIN_TASK_ANSWER            11944 //刷新赏金任务 ShangjinList
#define MSG_ID_ACCEPT_SHANGJIN_TASK_REQUEST            11945 //接受赏金任务 NULL
#define MSG_ID_ACCEPT_SHANGJIN_TASK_ANSWER            11946 //接受赏金任务 comm_answer
#define MSG_ID_CUR_SHANGJIN_TASK_NOTIFY            11947 //当前任务 ShangjinTaskId taskid是任务列表下表
#define MSG_ID_YAOSHI_ALL_NUM_NOTIFY            11948 //5点刷新次数 AllYaoshiNum


//活动大厅
#define MSG_ID_ACTIVITY_INFO_NOTIFY               12000 //活动信息通知 ActivityInfoNotify
#define MSG_ID_ACTIVE_REWARD_REQUEST              12001 //领取活跃度奖励请求 ActiveRewardRequest
#define MSG_ID_ACTIVE_REWARD_ANSWER               12002 //领取活跃度奖励应答 ActiveRewardAnswer
#define MSG_ID_ACTIVITY_UPDATE_DAILY_NOTIFY       12003 //更新日常数据通知 DailyActivityData
#define MSG_ID_ACTIVITY_UPDATE_CHIVALRY_NOTIFY    12004 //更新侠义数据通知 ChivalryActivityData

//帮会
#define MSG_ID_GUILD_LIST_REQUEST                 12100 //帮会列表请求 NULL
#define MSG_ID_GUILD_LIST_ANSWER                  12101 //帮会列表应答 GuildListAnswer
#define MSG_ID_GUILD_INFO_REQUEST                 12102 //帮会信息请求 NULL
#define MSG_ID_GUILD_INFO_ANSWER                  12103 //帮会信息应答 GuildInfoAnswer
#define MSG_ID_GUILD_MEMBER_LIST_REQUEST          12104 //帮会成员列表请求 NULL
#define MSG_ID_GUILD_MEMBER_LIST_ANSWER           12105 //帮会成员列表应答 GuildMemberListAnswer
#define MSG_ID_GUILD_CREATE_REQUEST               12106 //帮会创建请求 GuildCreateRequest
#define MSG_ID_GUILD_CREATE_ANSWER                12107 //帮会创建应答 GuildInfoAnswer
#define MSG_ID_GUILD_JOIN_REQUEST                 12108 //帮会入帮请求 GuildJoinRequest
#define MSG_ID_GUILD_JOIN_ANSWER                  12109 //帮会入帮应答 GuildJoinAnswer
#define MSG_ID_GUILD_JOIN_LIST_REQUEST            12110 //帮会入帮申请列表请求 NULL 
#define MSG_ID_GUILD_JOIN_LIST_ANSWER             12111 //帮会入帮申请列表应答 GuildJoinListAnswer
#define MSG_ID_GUILD_DEAL_JOIN_REQUEST            12112 //帮会处理入帮申请请求 GuildDealJoinRequest
#define MSG_ID_GUILD_DEAL_JOIN_ANSWER             12113 //帮会处理入帮申请应答 GuildDealJoinAnswer
#define MSG_ID_GUILD_TURN_SWITCH_REQUEST          12114 //帮会反转开关请求 GuildTurnSwitchRequest
#define MSG_ID_GUILD_TURN_SWITCH_ANSWER           12115 //帮会反转开关应答 GuildTurnSwitchAnswer
#define MSG_ID_GUILD_SET_WORDS_REQUEST            12116 //帮会设置公告请求 GuildSetWordsRequest
#define MSG_ID_GUILD_SET_WORDS_ANSWER             12117 //帮会设置公告应答 GuildSetWordsAnswer
#define MSG_ID_GUILD_APPOINT_OFFICE_REQUEST       12118 //帮会任命请求 GuildAppointOfficeRequest
#define MSG_ID_GUILD_APPOINT_OFFICE_ANSWER        12119 //帮会任命应答 GuildAppointOfficeAnswer
#define MSG_ID_GUILD_KICK_REQUEST                 12120 //帮会踢人请求 GuildKickReuqest
#define MSG_ID_GUILD_KICK_ANSWER                  12121 //帮会踢人应答 GuildKickAnswer
#define MSG_ID_GUILD_KICK_NOTIFY                  12122 //帮会踢人通知 NULL
#define MSG_ID_GUILD_UPDATE_ATTR_NOTIFY           12123 //帮会属性更新通知 GuildUpdateAttrNotify
//#define MSG_ID_GUILD_DAILY_REWARD_REQUEST         12124 //帮会领取每日奖励请求 NULL
//#define MSG_ID_GUILD_DAILY_REWARD_ANSWER          12125 //帮会领取每日奖励应答 comm_answer
#define MSG_ID_GUILD_RENAME_REQUEST               12126 //帮会改名请求 GuildRenameRequest
#define MSG_ID_GUILD_RENAME_ANSWER                12127 //帮会改名应答 GuildRenameAnswer
#define MSG_ID_GUILD_EXIT_REQUEST                 12128 //帮会退出请求 NULL
#define MSG_ID_GUILD_EXIT_ANSWER                  12129 //帮会退出应答 GuildExitAnswer
#define MSG_ID_GUILD_BUILDING_INFO_REQUEST        12130 //帮会建筑信息请求 NULL
#define MSG_ID_GUILD_BUILDING_INFO_ANSWER         12131 //帮会建筑信息应答 GuildBuildingInfoAnswer
#define MSG_ID_GUILD_BUILDING_UPGRADE_REQUEST     12132 //帮会建筑升级请求 GuildBuildingUpgradeRequest
#define MSG_ID_GUILD_BUILDING_UPGRADE_ANSWER      12133 //帮会建筑升级应答 GuildBuildingUpgradeAnswer
#define MSG_ID_GUILD_BUILDING_UPGRADE_UPDATE_NOTIFY      12134 //帮会建筑升级信息更新通知 GuildBuildingUpgradeUpdateNotify
#define MSG_ID_GUILD_SKILL_INFO_REQUEST           12140 //帮会技能信息请求 NULL
#define MSG_ID_GUILD_SKILL_INFO_ANSWER            12141 //帮会技能信息应答 GuildSkillInfoAnswer
#define MSG_ID_GUILD_SKILL_DEVELOP_REQUEST        12142 //帮会技能研发请求 GuildSkillUpgradeRequest
#define MSG_ID_GUILD_SKILL_DEVELOP_ANSWER         12143 //帮会技能研发应答 GuildSkillUpgradeAnswer
#define MSG_ID_GUILD_SKILL_DEVELOP_NOTIFY         12144 //帮会技能研发通知 GuildSkillData
#define MSG_ID_GUILD_SKILL_PRACTICE_REQUEST       12145 //帮会技能修炼请求 GuildSkillPracticeRequest
#define MSG_ID_GUILD_SKILL_PRACTICE_ANSWER        12146 //帮会技能修炼应答 GuildSkillUpgradeAnswer
#define MSG_ID_GUILD_SHORT_INFO_NOTIFY            12147 //帮会简短信息通知 GuildShortInfoNotify
#define MSG_ID_GUILD_UPDATE_STR_ATTR_NOTIFY       12148 //帮会字符属性更新通知 GuildUpdateStrAttrNotify
#define MSG_ID_GUILD_UPDATE_OBJECT_ATTR_NOTIFY    12149 //帮会结构属性更新通知 GuildUpdateObjectAttrNotify
#define MSG_ID_GUILD_SET_PERMISSION_REQUEST       12150 //帮会设置权限请求 GuildSetPermissionRequest
#define MSG_ID_GUILD_SET_PERMISSION_ANSWER        12151 //帮会设置权限应答 comm_answer
#define MSG_ID_GUILD_UPDATE_PERMISSION_NOTIFY     12152 //帮会权限更新通知 GuildUpdatePermissionNotify
#define MSG_ID_GUILD_ADD_USUAL_LOG_NOTIFY         12153 //帮会动态增加通知 GuildLogData
#define MSG_ID_GUILD_ADD_IMPORTANT_LOG_NOTIFY     12154 //帮会大事记增加通知 GuildLogData
#define MSG_ID_GUILD_ACCEPT_TASK_REQUEST          12155 //帮会接取建设任务请求 NULL
#define MSG_ID_GUILD_ACCEPT_TASK_ANSWER           12156 //帮会接取建设任务应答 comm_answer


//阵营
#define MSG_ID_CHOSE_ZHENYING_REQUEST                 12400 //选择阵营请求 ChoseZhenying
#define MSG_ID_CHOSE_ZHENYING_ANSWER                 12401 //选择阵营应答 AnsChoseZhenying
#define MSG_ID_CHANGE_ZHENYING_REQUEST                 12402 //改变阵营请求 ChoseZhenying
#define MSG_ID_CHANGE_ZHENYING_ANSWER                  12403 //改变阵营应答 AnsChoseZhenying
#define MSG_ID_ZHENYING_POWER_REQUEST                  12404 //阵营通用信息请求 NULL
#define MSG_ID_ZHENYING_POWER_ANSWER                  12405 //阵营通用信息应答 ZhenyingPower
#define MSG_ID_ZHENYING_INFO_NOTIFY                  12406 //上线所有阵营信息通知 ZhenyingInfo
#define MSG_ID_ZHENYING_EXP_NOTIFY                  12407 //阵营等级经验通知 ZhenyingExp
//#define MSG_ID_ZHENYING_TASK_COMMPLETE_NOTIFY       12408 //完成阵营任务通知 NULL
#define MSG_ID_INTO_ZHENYING_BATTLE_REQUEST           12409 //进入日常阵营战(野外场景表现)请求 comm_answer 0不接任务 1接
#define MSG_ID_INTO_ZHENYING_BATTLE_ANSWER            12410 //进入日常阵营战应答 comm_answer
#define MSG_ID_ZHENYING_CHANGE_LINE_REQUEST           12411 //阵营战换线请求 ZhenyingLineInfo
#define MSG_ID_ZHENYING_CHANGE_LINE_ANSWER            12412 //阵营战换线应答 comm_answer
#define MSG_ID_ZHENYING_TEAM_INFO_REQUEST           12413 //阵营战队伍信息请求 NULL
#define MSG_ID_ZHENYING_TEAM_INFO_ANSWER            12414 //阵营战队伍信息应答 ZhenyingTeam
//#define MSG_ID_EXIT_ZHENYING_BATTLE_REQUEST           12415 //退出日常阵营战请求 NULL
//#define MSG_ID_EXIT_ZHENYING_BATTLE_ANSWER            12416 //退出日常阵营战应答 comm_answer
#define MSG_ID_ZHENYING_TASK_PROCESS_NOTIFY       12417 //阵营任务进度通知 ZhenyingTaskProcess
#define MSG_ID_NEW_ZHENYING_TASK_NOTIFY          12418 //新阵营任务通知 NewZhenyingTask
#define MSG_ID_ZHENYING_TEAM_INFO_NOTIFY            12419 //阵营战队伍信息通知 ZhenyingTeamInfo
#define MSG_ID_GET_ZHENYING_TASK_AWARD_REQUEST                 12420 //阵营任务领奖请求 NULL
#define MSG_ID_GET_ZHENYING_TASK_AWARD_ANSWER                 12421 //阵营任务领奖应答 comm_answer
#define MSG_ID_ZHENYING_GET_LINE_INFO_REQUEST           12422 //阵营战分线信息请求 NULL
#define MSG_ID_ZHENYING_GET_LINE_INFO_ANSWER            12423 //阵营战分线信息应答 ZhenyingLine

#define MSG_ID_JOIN_ZHENYING_FIGHT_REQUEST           12424 //报名副本阵营战(阵营攻防)请求 NULL
#define MSG_ID_JOIN_ZHENYING_FIGHT_ANSWER            12425 //报名副本阵营战应答 comm_answer
#define MSG_ID_ZHENYING_FIGHT_SET_READY_STATE_REQUEST       12426 //副本阵营战设置准备状态请求 ZhenyingSetReady
#define MSG_ID_ZHENYING_FIGHT_READY_STATE_NOTIFY        12427 //副本阵营战准备人数 ZhenyingReadyState
#define MSG_ID_INTO_ZHENYING_FIGHT_REQUEST           12428 //进入副本阵营战请求 
#define MSG_ID_INTO_ZHENYING_FIGHT_ANSWER            12429 //进入副本阵营战应答 comm_answer
#define MSG_ID_JOIN_ZHENYING_FIGHT_NOTIFY           12430 //报名开始 NULL
#define MSG_ID_ZHENYING_FIGHT_START_NOTIFY          12431 //开始副本阵营战 NULL
#define MSG_ID_ZHENYING_FIGHT_SCORE_NOTIFY          12432 //阵营战双方总积分 TotalScore
#define MSG_ID_ZHENYING_FIGHT_MY_SCORE_NOTIFY          12433 //阵营战我的积分 MyScore
#define MSG_ID_ZHENYING_FIGHT_MYSIDE_SCORE_NOTIFY          12434 //阵营战我方积分 SideScore
#define MSG_ID_ZHENYING_FIGHT_MYSIDE_SCORE_REQUEST           12435 //阵营战我方积分请求 NULL
//#define MSG_ID_ZHENYING_FIGHT_FLAG_NOTIFY          12436 //阵营战旗帜 ZhenYingFlag
#define MSG_ID_ZHENYING_FIGHT_READY_CD_NOTIFY          12437 //阵营战准备cd FbCD
#define MSG_ID_ZHENYING_FIGHT_CD_NOTIFY          12438 //阵营战结束cd FbCD
#define MSG_ID_ZHENYING_FIGHT_SETTLE_NOTIFY          12439 //阵营战结算ZhenYingResult
#define MSG_ID_ZHENYING_FIGHT_START_FLAG_NOTIFY          12440 //阵营战开始夺旗 StartFlag
#define MSG_ID_ZHENYING_FIGHT_INTERUPT_FLAG_NOTIFY          12441 //阵营战中断夺旗 StartFlag
#define MSG_ID_ZHENYING_FIGHT_FINISH_FLAG_NOTIFY          12442 //阵营战完成夺旗 StartFlag
#define MSG_ID_JOIN_ZHENYING_FIGHT_STATE_NOTIFY            12443 //报名副本阵营战cd FbCD
#define MSG_ID_CANCEL_JOIN_ZHENYING_FIGHT_REQUEST           12444 //取消报名副本阵营战(阵营攻防)请求 NULL
#define MSG_ID_CANCEL_JOIN_ZHENYING_FIGHT_ANSWER            12445 //取消报名副本阵营战应答 comm_answer

//日常阵营(野外场景表现)
//#define MSG_ID_INTO_ZHENYING_DAILY_REQUEST           12450 //进入阵营日常请求 
//#define MSG_ID_INTO_ZHENYING_DAILY_ANSWER            12451 //进入阵营日常应答 comm_answer
#define MSG_ID_ZHENYING_DAILY_CD_NOTIFY          12452 //阵营日常结束cd FbCD
#define MSG_ID_ZHENYING_DAILY_SCORE_NOTIFY       12453 //阵营日常积分 DailyScore
#define MSG_ID_ZHENYING_DAILY_MINE_NOTIFY        12454 //阵营日常矿车进度 DailyMine
#define MSG_ID_ZHENYING_DAILY_MINE_STATE_NOTIFY        12455 //阵营日常矿车状态 MineState
#define MSG_ID_ZHENYING_MAX_REQUEST                 12500 //NULL

//答题
#define MSG_ID_GET_COMMON_QUESTION_REQUEST                 12501 //获取普通答题题目请求 NULL
#define MSG_ID_GET_COMMON_QUESTION_ANSWER                 12502 //获取普通答题题目应答 CommonQuestion
#define MSG_ID_ANSWER_COMMON_QUESTION_REQUEST          12503 //回答普通问题请求 ReqAnswer
#define MSG_ID_ANSWER_COMMON_QUESTION_ANSWER          12509 //回答普通问题应答 comm_answer
#define MSG_ID_COMMON_QUESTION_HINT_REQUEST            12504 //普通答题提示请求 NULL
#define MSG_ID_COMMON_QUESTION_HINT_ANSWER             12505 //普通答题提示应答 comm_answer
#define MSG_ID_COMMON_QUESTION_HELP_REQUEST            12514 //普通答题求助请求 NULL
#define MSG_ID_COMMON_QUESTION_HELP_ANSWER             12515 //普通答题求助应答 comm_answer

#define MSG_ID_AWARD_QUESTION_NOTIFY              12506 //有奖答题信息 AwardAnswer
//#define MSG_ID_BEGIN_AWARD_QUESTION_REQUEST            12507 //开始有奖答题请求 NULL
#define MSG_ID_END_AWARD_QUESTION_REQUEST            12508 //放弃有奖答题请求 NULL
#define MSG_ID_GET_AWARD_QUESTION_REQUEST             12510 //获取有奖答题题目请求 NULL
#define MSG_ID_GET_AWARD_QUESTION_ANSWER              12511 //获取有奖答题题目应答 AwardQuestion
#define MSG_ID_ANSWER_AWARD_QUESTION_REQUEST          12512 //回答有奖问题请求 ReqAnswer
#define MSG_ID_ANSWER_AWARD_QUESTION_ANSWER          12513 //回答有奖问题应答 comm_answer
#define MSG_ID_NEXT_AWARD_QUESTION_NOTIFY            12516 //本轮有奖答题结束信息 comm_answer
#define MSG_ID_FIRST_AWARD_QUESTION_REQUEST          12517 //获取答题任务请求 NULL
#define MSG_ID_GET_AWARD_QUESTION_FAILE_NOTIFY       12518 //获奖答题任务接取失败应答 comm_answer

#define MSG_ID_FACTION_QUESTION_NOTIFY       12519 //公会答题题目 FactionQuestion
#define MSG_ID_FACTION_QUESTION_ANSWER       12520 //公会答题结果 FactionQuestionResult
#define MSG_ID_ONE_FACTION_QUESTION_END_NOTIFY       12521 //公会答题一道题结束 OneFactionQuestionEnd
#define MSG_ID_FACTION_QUESTION_OPEN_NOTIFY       12522 //公会答题活动开启通知 NULL
#define MSG_ID_FACTION_QUESTION_END_NOTIFY       12523 //公会答题活动结束通知 NULL
#define MSG_ID_OPEN_FACTION_QUESTION_REQUEST          12524 //手动开启公会答题请求 NULL
#define MSG_ID_OPEN_FACTION_QUESTION_ANSWER          12525 //手动开启公会答题应答 comm_answer
#define MSG_ID_FACTION_QUESTION_REST_NOTIFY       12526 //公会答题放题cd通知 FactionQuestionRest
#define MSG_ID_ANSWER_MAX_REQUEST                 12550 //NULL

//寻宝 
#define MSG_ID_XUNBAO_POS_REQUEST          12551 //寻宝位置请求 BagUseRequest
#define MSG_ID_XUNBAO_POS_ANSWER          12552 //寻宝位置应答 XunbaoPos
#define MSG_ID_XUNBAO_FB_PASS_NOTIFY          12553 //寻宝副本通关 NULL
#define MSG_ID_XUNBAO_USE_NEXT_NOTIFY          12554 //使用下张藏宝图 NULL
#define MSG_ID_XUNBAO_AUTO_COLLECT_NOTIFY          12555 // AutoCollect
#define MSG_ID_XUNBAO_AUTO_MONSTER_NOTIFY          12556 //AutoCollect
 
//好友
#define MSG_ID_FRIEND_INFO_REQUEST                        12601 //好友信息请求 NULL
#define MSG_ID_FRIEND_INFO_ANSWER                         12602 //好友信息应答 FriendInfoAnswer
#define MSG_ID_FRIEND_ADD_CONTACT_REQUEST                 12603 //添加好友请求 FriendOperateRequest
#define MSG_ID_FRIEND_ADD_CONTACT_ANSWER                  12604 //添加好友应答 FriendOperateAnswer
#define MSG_ID_FRIEND_DEL_CONTACT_REQUEST                 12605 //删除好友请求 FriendOperateRequest
#define MSG_ID_FRIEND_DEL_CONTACT_ANSWER                  12606 //删除好友应答 FriendOperateAnswer
#define MSG_ID_FRIEND_ADD_BLOCK_REQUEST                   12607 //添加黑名单请求 FriendOperateRequest
#define MSG_ID_FRIEND_ADD_BLOCK_ANSWER                    12608 //添加黑名单应答 FriendOperateAnswer
#define MSG_ID_FRIEND_DEL_BLOCK_REQUEST                   12609 //删除黑名单请求 FriendOperateRequest
#define MSG_ID_FRIEND_DEL_BLOCK_ANSWER                    12610 //删除黑名单应答 FriendOperateAnswer
#define MSG_ID_FRIEND_DEL_ENEMY_REQUEST                   12611 //删除仇人请求 FriendOperateRequest
#define MSG_ID_FRIEND_DEL_ENEMY_ANSWER                    12612 //删除仇人应答 FriendOperateAnswer
#define MSG_ID_FRIEND_LIST_CHANGE_NOTIFY                  12613 //好友列表变更通知 FriendListChangeNotify
#define MSG_ID_FRIEND_CREATE_GROUP_REQUEST                12614 //新建分组请求 FriendCreateGroupRequest
#define MSG_ID_FRIEND_CREATE_GROUP_ANSWER                 12615 //新建分组应答 FriendCreateGroupAnswer
#define MSG_ID_FRIEND_EDIT_GROUP_REQUEST                  12616 //编辑分组请求 FriendEditGroupRequest
#define MSG_ID_FRIEND_EDIT_GROUP_ANSWER                   12617 //编辑分组应答 FriendEditGroupAnswer
#define MSG_ID_FRIEND_REMOVE_GROUP_REQUEST                12618 //删除分组请求 FriendRemoveGroupRequest
#define MSG_ID_FRIEND_REMOVE_GROUP_ANSWER                 12619 //删除分组应答 comm_answer
#define MSG_ID_FRIEND_MOVE_PLAYER_GROUP_REQUEST           12620 //更改分组请求 FriendMovePlayerGroupRequest
#define MSG_ID_FRIEND_MOVE_PLAYER_GROUP_ANSWER            12621 //更改分组应答 comm_answer
#define MSG_ID_FRIEND_DEAL_APPLY_REQUEST                  12622 //处理请求好友请求 FriendDealApplyRequest
#define MSG_ID_FRIEND_DEAL_APPLY_ANSWER                   12623 //处理请求好友应答 FriendDealApplyAnswer
#define MSG_ID_FRIEND_RECOMMEND_REQUEST                   12624 //推荐好友请求 NULL
#define MSG_ID_FRIEND_RECOMMEND_ANSWER                    12625 //推荐好友应答 FriendRecommendAnswer
#define MSG_ID_FRIEND_SEARCH_REQUEST                      12626 //搜索好友请求 FriendSearchRequest
#define MSG_ID_FRIEND_SEARCH_ANSWER                       12627 //搜索好友应答 FriendSearchAnswer
#define MSG_ID_FRIEND_SEND_GIFT_REQUEST                   12628 //赠送礼物请求 FriendSendGiftRequest
#define MSG_ID_FRIEND_SEND_GIFT_ANSWER                    12629 //赠送礼物应答 FriendSendGiftAnswer
#define MSG_ID_FRIEND_SEND_GIFT_NOTIFY                    12630 //赠送礼物通知 FriendSendGiftNotify
#define MSG_ID_FRIEND_UPDATE_UNIT_NOTIFY                  12631 //更新好友通知 FriendUpdateUnitNotify
#define MSG_ID_FRIEND_CONTACT_EXTEND_NOTIFY               12632 //好友上限扩展通知 NULL
#define MSG_ID_FRIEND_UPDATE_STATUS_NOTIFY                12633 //更新好友状态通知 FriendUpdateStatusNotify

//个人信息
#define MSG_ID_PERSONALITY_INFO_REQUEST                   12701 //个人信息请求 NULL
#define MSG_ID_PERSONALITY_INFO_ANSWER                    12702 //个人信息应答 PersonalityInfoAnswer
#define MSG_ID_PERSONALITY_SET_GENERAL_REQUEST            12703 //设置普通信息请求 PersonalitySetGeneralRequest
#define MSG_ID_PERSONALITY_SET_GENERAL_ANSWER             12704 //设置普通信息应答 comm_answer
#define MSG_ID_PERSONALITY_SET_TAGS_REQUEST               12705 //设置标签信息请求 PersonalitySetTagsRequest
#define MSG_ID_PERSONALITY_SET_TAGS_ANSWER                12706 //设置标签信息应答 comm_answer
#define MSG_ID_PERSONALITY_SET_INTRO_REQUEST              12707 //设置签名信息请求 PersonalitySetIntroRequest
#define MSG_ID_PERSONALITY_SET_INTRO_ANSWER               12708 //设置签名信息应答 comm_answer
#define MSG_ID_GET_OTHER_INFO_REQUEST                     12709 //查看他人个人信息请求 GetOtherInfoRequest
#define MSG_ID_GET_OTHER_INFO_ANSWER                      12710 //查看他人个人信息应答 GetOtherInfoAnswer

//排行榜
#define MSG_ID_RANK_INFO_REQUEST                          12801 //排行榜信息请求 RankInfoRequest
#define MSG_ID_RANK_INFO_ANSWER                           12802 //排行榜信息应答 RankInfoAnswer

//自动补血
#define MSG_ID_AUTO_ADD_HP_SET_REQUEST                    12901 //设置自动补血的请求 AutoAddHpSetRequest
#define MSG_ID_AUTO_ADD_HP_SET_ANSWER                    12902 //设置自动补血的应答 comm_answer
#define MSG_ID_HP_POOL_CHANGED_NOTIFY                     12903 //血池容量变化通知 HpPoolChangedNotify
#define MSG_ID_ENTER_FIGHT_STATE_NOTIFY                     12904 //进入战斗状态通知 NULL
#define MSG_ID_LEAVE_FIGHT_STATE_NOTIFY                     12905 //脱离战斗状态通知 NULL

//帮会战
#define MSG_ID_GUILD_BATTLE_WAIT_INFO_NOTIFY              13001 //帮会战等待区信息通知 GuildBattleWaitInfoNotify
#define MSG_ID_GUILD_BATTLE_INFO_REQUEST                  13002 //帮会战信息请求 NULL
#define MSG_ID_GUILD_BATTLE_INFO_ANSWER                   13003 //帮会战信息应答 GuildBattleInfoAnswer
#define MSG_ID_GUILD_BATTLE_CALL_REQUEST                  13004 //帮会战征召请求 NULL
#define MSG_ID_GUILD_BATTLE_CALL_ANSWER                   13005 //帮会战征召应答 comm_answer
#define MSG_ID_GUILD_BATTLE_CALL_NOTIFY                   13006 //帮会战征召通知 GuildBattleCallNotify
#define MSG_ID_GUILD_BATTLE_MATCH_NOTIFY                  13007 //帮会战参与者信息通知 GuildBattleMatchNotify
#define MSG_ID_GUILD_BATTLE_RECORD_NOTIFY                 13008 //帮会战战绩信息 GuildBattleRecordNotify
#define MSG_ID_GUILD_BATTLE_ROUND_FINISH_NOTIFY           13009 //帮会战每轮结算通知 GuildBattleRoundFinishNotify
#define MSG_ID_GUILD_BATTLE_ACTIVITY_FINISH_NOTIFY        13010 //帮会战活动结算通知 GuildBattleActivityFinishNotify
#define MSG_ID_GUILD_BATTLE_ROUND_INFO_NOTIFY             13011 //帮会战每轮信息通知 GuildBattleRoundInfoNotify
#define MSG_ID_GUILD_BATTLE_KILL_NOTIFY                   13012 //帮会战击杀通知 GuildBattleKillNotify
#define MSG_ID_GUILD_BATTLE_BOSS_DAMAGE_NOTIFY            13013 //帮会战Boss伤害通知 GuildBattleBossDamageNotify
#define MSG_ID_GUILD_BATTLE_KILL_MONSTER_NOTIFY           13014 //帮会战击杀小怪通知 GuildBattleKillNotify

//伙伴
#define MSG_ID_PARTNER_INFO_REQUEST                       13101 //伙伴信息请求 NULL
#define MSG_ID_PARTNER_INFO_ANSWER                        13102 //伙伴信息应答 PartnerInfoAnswer
#define MSG_ID_PARTNER_ADD_NOTIFY                         13103 //伙伴新增通知 PartnerData
#define MSG_ID_PARTNER_DICTIONARY_ADD_NOTIFY              13104 //伙伴图鉴开启通知 PartnerDictionaryAddNotify
#define MSG_ID_PARTNER_TURN_SWITCH_REQUEST                13105 //伙伴翻转开关请求 PartnerTurnSwitchRequest
#define MSG_ID_PARTNER_TURN_SWITCH_ANSWER                 13106 //伙伴翻转开关应答 comm_answer
#define MSG_ID_PARTNER_FORMATION_REQUEST                  13107 //伙伴布阵请求 PartnerFormationRequest
#define MSG_ID_PARTNER_FORMATION_ANSWER                   13108 //伙伴布阵应答 PartnerFormationAnswer
#define MSG_ID_PARTNER_LEARN_SKILL_REQUEST                13109 //伙伴学习技能请求 PartnerLearnSkillRequest
#define MSG_ID_PARTNER_LEARN_SKILL_ANSWER                 13110 //伙伴学习技能应答 PartnerLearnSkillAnswer
#define MSG_ID_PARTNER_USE_EXP_ITEM_REQUEST               13111 //伙伴使用经验丹请求 PartnerUseExpItemRequest
#define MSG_ID_PARTNER_USE_EXP_ITEM_ANSWER                13112 //伙伴使用经验丹应答 comm_answer
#define MSG_ID_PARTNER_DISMISS_REQUEST                    13113 //伙伴遣散请求 PartnerDismissRequest
#define MSG_ID_PARTNER_DISMISS_ANSWER                     13114 //伙伴遣散应答 PartnerDismissAnswer
#define MSG_ID_PARTNER_EXCHANGE_REQUEST                   13115 //伙伴兑换请求 PartnerExchangeRequest
#define MSG_ID_PARTNER_EXCHANGE_ANSWER                    13116 //伙伴兑换应答 PartnerExchangeAnswer
#define MSG_ID_PARTNER_RECRUIT_REQUEST                    13117 //伙伴招募请求 PartnerRecruitRequest
#define MSG_ID_PARTNER_RECRUIT_ANSWER                     13118 //伙伴招募应答 PartnerRecruitAnswer
#define MSG_ID_PARTNER_RESET_ATTR_REQUEST                 13119 //伙伴洗髓请求 PartnerUuid
#define MSG_ID_PARTNER_RESET_ATTR_ANSWER                  13120 //伙伴洗髓应答 ResResetAttr
#define MSG_ID_PARTNER_ADD_ATTR_REQUEST                   13121 //伙伴强化请求 StrongPartner
#define MSG_ID_PARTNER_ADD_ATTR_ANSWER                    13122 //伙伴强化应答 ResStrongPartner
#define MSG_ID_PARTNER_ADD_GOD_REQUEST                    13123 //伙伴升级神曜请求 StrongPartner
#define MSG_ID_PARTNER_ADD_GOD_ANSWER                     13124 //伙伴升级神曜应答 ResStrongPartner 0通用 1专属
#define MSG_ID_PARTNER_SAVE_ATTR_REQUEST                  13125 //保存伙伴洗髓请求 PartnerUuid
#define MSG_ID_PARTNER_SAVE_ATTR_ANSWER                   13126 //保存伙伴洗髓应答 comm_answer
#define MSG_ID_PARTNER_FIGHTING_NOTIFY                    13127 //主战伙伴通知 PartnerUuid
#define MSG_ID_PARTNER_ADD_BASE_NOTIFY                    13128 //升级增加属性值 PartnerAddBaseAttr
#define MSG_ID_PARTNER_RELIVE_TIME_NOTIFY                 13129 //伙伴复活时间通知 PartnerReliveTimeNotify
#define MSG_ID_PARTNER_DEAD_FINISH_REQUEST                13130 //伙伴死亡完成请求 NULL
#define MSG_ID_PARTNER_BOND_ACTIVE_REQUEST                13131 //伙伴激活羁绊请求 PartnerBondActiveRequest
#define MSG_ID_PARTNER_BOND_ACTIVE_ANSWER                 13132 //伙伴激活羁绊应答 PartnerBondActiveAnswer
#define MSG_ID_PARTNER_BOND_REWARD_REQUEST                13133 //伙伴羁绊奖励请求 PartnerBondRewardRequest
#define MSG_ID_PARTNER_BOND_REWARD_ANSWER                 13134 //伙伴羁绊奖励应答 PartnerBondRewardAnswer
#define MSG_ID_PARTNER_COMPOSE_STONE_REQUEST              13135 //伙伴神曜合成请求 PartnerComposeStoneRequest
#define MSG_ID_PARTNER_COMPOSE_STONE_ANSWER               13136 //伙伴神曜合成应答 comm_answer
#define MSG_ID_PARTNER_FABAO_STONE_REQUEST                13137 //伙伴法宝合成请求 PartnerFabaoStoneRequest
#define MSG_ID_PARTNER_FABAO_STONE_ANSWER                 13138 //伙伴法宝合成应答 comm_answer 
#define MSG_ID_PARTNER_FABAO_CHANGE_REQUEST				  13139 //伙伴法宝佩戴或者替换请求 PartnerFabaoChangeRequest
#define MSG_ID_PARTNER_FABAO_CHANGE_ANSWER				  13140 //伙伴法宝佩戴或者替换应答 PartnerFabaoChangeAnswer

//运镖
#define MSG_ID_CASH_TRUCK_INFO_NOTIFY                       13301 //运镖信息 CashTruckInfo
#define MSG_ID_ACCEPT_CASH_TRUCK_REQUEST                       13302 //接镖车请求 AcceptCashTruck
#define MSG_ID_ACCEPT_CASH_TRUCK_ANSWER                       13303 //接镖车应答 ResAcceptCashTruck
#define MSG_ID_SUBMIT_CASH_TRUCK_REQUEST                       13304 //交镖车请求 NULL
#define MSG_ID_SUBMIT_CASH_TRUCK_ANSWER                       13305 //交镖车请求 NULL
#define MSG_ID_GET_ON_CASH_TRUCK_REQUEST                       13306 //上镖车请求 NULL
#define MSG_ID_GET_ON_CASH_TRUCK_ANSWER                       13307 //上镖车应答 comm_answer
#define MSG_ID_GET_ON_CASH_TRUCK_NOTIFY                       13308 //上镖车通知 UpDownCashTruck
#define MSG_ID_GO_DOWND_CASH_TRUCK_REQUEST                   13309 //下镖车请求 NULL
#define MSG_ID_GO_DOWND_CASH_TRUCK_ANSWER                    13310 //下镖车应答 comm_answer
#define MSG_ID_GO_DOWND_CASH_TRUCK_NOTIFY                    13311 //下镖车通知 UpDownCashTruck
#define MSG_ID_CASH_TRUCK_SPEED_UP_REQUEST                   13312 //镖车加速请求 NULL
#define MSG_ID_CASH_TRUCK_ENDURANCE_NOTIFY                   13313 //耐力值 TruckEndurance
#define MSG_ID_CASH_TRUCK_TASK_FAIL_NOTIFY                   13314 //镖车任务失败 comm_answer
#define MSG_ID_CASH_TRUCK_SPEED_UP_ANSWER                    13315 //镖车加速应答 AnsSpeedUp


//即将开启礼包相关
#define MSG_ID_JIJIANGOP_GIFT_INFO_NOTIFY                   13401 //已领取最大奖励礼包信息  GiftCommNotify
#define MSG_ID_JIJIANGOP_GIFT_INFO_REQUEST                  13402 //领取即将开启礼包请求  NULL
#define MSG_ID_JIJIANGOP_GIFT_INFO_ANSWER                   13403 //领奖回复  GiftReceiveAnswer

//客户端请求跳过新手副本
#define MSG_ID_SKIP_NEW_RAID_REQUEST						13501 //请求跳过新手副本 NULL

//成就
#define MSG_ID_ACHIEVEMENT_INFO_NOTIFY                      13601 //成就信息通知 AchievementInfoNotify
#define MSG_ID_ACHIEVEMENT_UPDATE_NOTIFY                    13602 //成就更新通知 AchievementData
#define MSG_ID_ACHIEVEMENT_REWARD_REQUEST                   13603 //成就领取奖励请求 AchievementRewardRequest
#define MSG_ID_ACHIEVEMENT_REWARD_ANSWER                    13604 //成就领取奖励应答 comm_answer

//称号
#define MSG_ID_TITLE_INFO_NOTIFY                            13631 //称号信息通知 TitleInfoNotify
#define MSG_ID_TITLE_UPDATE_NOTIFY                          13632 //称号更新通知 TitleData
#define MSG_ID_TITLE_WEAR_REQUEST                           13633 //称号佩戴请求 TitleWearRequest
#define MSG_ID_TITLE_WEAR_ANSWER                            13634 //称号佩戴应答 TitleWearAnswer
#define MSG_ID_TITLE_MARK_OLD_REQUEST                       13635 //称号置旧请求 TitleMarkOldRequest
#define MSG_ID_TITLE_MARK_OLD_ANSWER                        13636 //称号置旧应答 TitleMarkOldAnswer

//变强
#define MSG_ID_STRONG_INFO_NOTIFY							13661 //变强信息通知 StrongInfoNotify
#define MSG_ID_STRONG_GOAL_UPDATE_NOTIFY                    13662 //变强目标更新通知 StrongGoalData
#define MSG_ID_STRONG_GOAL_REWARD_REQUEST                   13663 //变强目标奖励请求 StrongGoalRewardRequest
#define MSG_ID_STRONG_GOAL_REWARD_ANSWER                    13664 //变强目标奖励应答 comm_answer
#define MSG_ID_STRONG_CHAPTER_UPDATE_NOTIFY                 13665 //变强章节更新通知 StrongChapterData
#define MSG_ID_STRONG_CHAPTER_REWARD_REQUEST                13666 //变强章节奖励请求 StrongChapterRewardRequest
#define MSG_ID_STRONG_CHAPTER_REWARD_ANSWER                 13667 //变强章节奖励应答 comm_answer

//斗法场
#define MSG_ID_DOUFACHANG_INFO_REQUEST						13701  //请求斗法场主界面信息 NULL
#define MSG_ID_DOUFACHANG_INFO_ANSWER						13702  //斗法场主界面信息 doufachang_info_answer
#define MSG_ID_DOUFACHANG_CHALLENGE_REQUEST					13703  //斗法场挑战请求	 doufachang_challenge_request
#define MSG_ID_DOUFACHANG_CHALLENGE_ANSWER					13704  //斗法场挑战	 doufachang_challenge_answer
#define MSG_ID_DOUFACHANG_RAID_FINISHED_NOTIFY				13705  //斗法场副本结束通知	 doufachang_raid_finished_notify
#define MSG_ID_DOUFACHANG_GET_REWARD_REQUEST				13706  //斗法场获取奖励请求 NULL
#define MSG_ID_DOUFACHANG_GET_REWARD_ANSWER					13707  //斗法场获取奖励	 comm_answer
#define MSG_ID_DOUFACHANG_RECORD_REQUEST					13708  //斗法场战报请求	 NULL
#define MSG_ID_DOUFACHANG_RECORD_ANSWER						13709  //斗法场战报	 doufachang_record_answer
#define MSG_ID_DOUFACHANG_BUY_CHALLENGE_REQUEST           13710  //斗法场购买次数请求  doufachang_buy_challenge_request
#define MSG_ID_DOUFACHANG_BUY_CHALLENGE_ANSWER            13711  //斗法场购买次数回复  doufachang_buy_challenge_answer

//世界boss
#define MSG_ID_WORLDBOSS_CUR_RANK_INFO_NOTIFY              13800 //广播世界boss实时排名信息 RankWorldBossNotify
#define MSG_ID_WORLDBOSS_CUR_PLAYER_INFO_NOTIFY			   13801 //玩家在攻击世界boss时实时跟新客户端玩家当前轮，当前世界boss的信息 RankWorldBossPlayerInfo	
#define MSG_ID_WORLDBOSS_REAL_RANK_INFO_REQUEST			   13802 //世界boss实时排名信息和玩家自己实时信息请求 RankWorldBossRealInfoRequest
#define MSG_ID_WORLDBOSS_REAL_RANK_INFO_ANSWER			   13803 //世界boss实时排名信息和玩家自己实时信息应答 RankWorldBossRealInfoAnswer
#define MSG_ID_WORLDBOSS_ZHUJIEMIAN_INFO_REQUEST		   13804 //世界boss主界面信息请求
#define MSG_ID_WORLDBOSS_ZHUJIEMIAN_INFO_ANSWER            13805 //世界boss主界面信息应答 ankWorldBossAllBossInfoAnswer
#define MSG_ID_WORLDBOSS_LAST_RANK_INFO_REQUEST			   13806 //世界boss上轮排名信息和玩家自己上轮信息请求 RankRankWorldBossLastInfoRequest
#define MSG_ID_WORLDBOSS_LAST_RANK_INFO_ANSWER			   13807 //世界boss上轮排名信息和玩家自己上轮信息应答 RankRankWorldBossLastInfoAnswer
#define MSG_ID_WORLDBOSS_PLAYER_RANK_INFO_NOTIFY		   13808 //世界boss刷新或者死亡玩家奖励信息通知 RankWorldBossRewardNotify
#define MSG_ID_WORLDBOSS_REFRESH_NOTIFY		               13809 //世界boss刷新通知 RankWorldBossRefreshNotify


//英雄挑战
#define MSG_ID_HERO_CHALLENGE_ZHUJIEMIAN_INFO_REQUEST	   13900 //英雄挑战主界面信息请求
#define MSG_ID_HERO_CHALLENGE_ZHUJIEMIAN_INFO_ANSWER       13901 //英雄挑战主界面信息应答 HeroChallengeMainInfoAnswer
#define MSG_ID_HERO_CHALLENGE_SWEEP_REQUEST				   13902 //英雄挑战扫荡请求 HeroChallengeSweepRequest
#define MSG_ID_HERO_CHALLENGE_SWEEP_ANSWER			       13903 //英雄挑战扫荡应答 HeroChallengeSweepAnswer
#define MSG_ID_HERO_CHALLENGE_SWEEP_RECIVE_REQUEST		   13904 //英雄挑战扫荡奖励领取请求 HeroChallengeReciveSweepRewardRequest
#define MSG_ID_HERO_CHALLENGE_SWEEP_RECIVE_ANSWER		   13905 //英雄挑战扫荡奖励领取应答 comm_answer
#define MSG_ID_HERO_CHALLENGE_SWEEP_REWARD_INFO_REQUEST	   13906 //英雄挑战扫荡奖励信息请求 HeroChallengeReciveSweepRewardRequest
#define MSG_ID_HERO_CHALLENGE_SWEEP_REWARD_INFO_NOTIFY	   13907 //英雄挑战扫荡奖励信息应答 HeroChallengeSweepAnswer

//秘境修炼任务
#define MSG_ID_MIJING_XIULIAN_TASK_INFO_REQUEST			   14000 //秘境修炼任务信息请求
#define MSG_ID_MIJING_XIULIAN_TASK_INFO_ANSWER			   14001 //秘境修炼任务信息应答 MiJingXiuLianTaskInfoAnswer
#define MSG_ID_MIJING_XIULIAN_TASK_SHUAXING_REQUEST		   14002 //秘境修炼刷星请求
#define MSG_ID_MIJING_XIULIAN_TASK_SHUAXING_ANSWER		   14003 //秘境修炼刷星信息应答 MiJingXiuLianTaskShuaXingAnswer

//交易
#define MSG_ID_TRADE_INFO_NOTIFY						   14101 //交易信息通知 TradeInfoNotify
#define MSG_ID_TRADE_SHELF_UPDATE_NOTIFY				   14102 //货架格子更新通知 TradeShelfData
#define MSG_ID_TRADE_ON_SHELF_REQUEST					   14103 //物品上架请求 TradeOnShelfRequest
#define MSG_ID_TRADE_ON_SHELF_ANSWER					   14104 //物品上架应答 comm_answer
#define MSG_ID_TRADE_OFF_SHELF_REQUEST					   14105 //物品下架请求 TradeOffShelfRequest
#define MSG_ID_TRADE_OFF_SHELF_ANSWER					   14106 //物品下架应答 comm_answer
#define MSG_ID_TRADE_RESHELF_REQUEST					   14107 //物品重新上架请求 TradeReshelfRequest
#define MSG_ID_TRADE_RESHELF_ANSWER					       14108 //物品重新上架应答 comm_answer
#define MSG_ID_TRADE_ENLARGE_SHELF_REQUEST				   14109 //扩充寄售格请求 NULL
#define MSG_ID_TRADE_ENLARGE_SHELF_ANSWER				   14110 //扩充寄售格应答 TradeEnlargeShelfAnswer
#define MSG_ID_TRADE_SOLD_NOTIFY				           14112 //售出通知 TradeSoldNotify
#define MSG_ID_TRADE_ITEM_SUMMARY_REQUEST				   14113 //交易货品总览请求 NULL
#define MSG_ID_TRADE_ITEM_SUMMARY_ANSWER				   14114 //交易货品总览应答 TradeItemSummaryAnswer
#define MSG_ID_TRADE_ITEM_DETAIL_REQUEST				   14115 //交易货品信息请求 TradeItemDetailRequest
#define MSG_ID_TRADE_ITEM_DETAIL_ANSWER				       14116 //交易货品信息应答 TradeItemDetailAnswer
#define MSG_ID_TRADE_BUY_REQUEST				           14117 //交易行购买请求 TradeBuyRequest
#define MSG_ID_TRADE_BUY_ANSWER				               14118 //交易行购买应答 comm_answer
#define MSG_ID_AUCTION_BID_REQUEST				           14119 //拍卖竞价请求 AuctionBidRequest
#define MSG_ID_AUCTION_BID_ANSWER				           14120 //拍卖竞价应答 AuctionBidAnswer
#define MSG_ID_AUCTION_BUY_NOW_REQUEST				       14122 //拍卖一口价请求 AuctionBuyNowRequest
#define MSG_ID_AUCTION_BUY_NOW_ANSWER				       14123 //拍卖一口价应答 AuctionBuyNowAnswer
#define MSG_ID_TRADE_GET_EARNING_REQUEST				   14124 //交易行领取收益请求 NULL
#define MSG_ID_TRADE_GET_EARNING_ANSWER				       14125 //交易行领取收益应答 comm_answer
#define MSG_ID_AUCTION_INFO_REQUEST				           14126 //拍卖信息请求 AuctionInfoRequest
#define MSG_ID_AUCTION_INFO_ANSWER				           14127 //拍卖信息应答 AuctionInfoAnswer
#define MSG_ID_TRADE_ITEM_SUMMARY_UPDATE_NOTIFY			   14128 //交易货品总览更新通知 TradeItemSummaryData

//帮会入侵活动
#define MSG_ID_GUILD_RUQIN_REWARD_INFO_NOTIFY              14200 //帮会入侵活动广播奖励信息 GuildRuqinActiveRewardAndRankNotify
#endif

