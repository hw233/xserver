#ifndef _1_spb_H__
#define _1_spb_H__

#include <stdlib.h>
#include <stdint.h>

struct AchievementFunctionTable;
struct AchievementHierarchyTable;
struct ActiveSkillTable;
struct ActiveTable;
struct ActorAttributeTable;
struct ActorFashionTable;
struct ActorHeadTable;
struct ActorLevelTable;
struct ActorRobotTable;
struct ActorTable;
struct AcupunctureTable;
struct ArenaRewardTable;
struct AttributeTypeTable;
struct AuctionTable;
struct BaguaStarTable;
struct BaguaSuitTable;
struct BaguaTable;
struct BaguaViceAttributeTable;
struct BaseAITable;
struct BattleFieldRank;
struct BattlefieldTable;
struct BiaocheRewardTable;
struct BiaocheTable;
struct BootNameTable;
struct BreakTable;
struct BuffTable;
struct CampDefenseTable;
struct CampTable;
struct CastSpiritTable;
struct ChallengeTable;
struct ChangeSpecialty;
struct CharmTable;
struct ChivalrousTable;
struct CiFuTable;
struct CollectTable;
struct ColourTable;
struct ControlTable;
struct DegreeTable;
struct DonationTable;
struct DropConfigTable;
struct DungeonTable;
struct EquipAttribute;
struct EquipLock;
struct EquipStarLv;
struct EquipmentTable;
struct EscortTask;
struct EventCalendarTable;
struct FactionActivity;
struct FactionBattleTable;
struct FetterTable;
struct FishingTable;
struct FlySkillTable;
struct FunctionUnlockTable;
struct GangsBuildTaskTable;
struct GangsDungeonTable;
struct GangsJurisdictionTable;
struct GangsSkillTable;
struct GangsTable;
struct GemAttribute;
struct GenerateMonster;
struct GiftTable;
struct GodYaoAttributeTable;
struct GradeTable;
struct GrowupTable;
struct ItemsConfigTable;
struct LevelReward;
struct LifeItemTable;
struct LifeMagicTable;
struct LifeProbabilitytable;
struct LifeSkillTable;
struct LimitActivityControlTable;
struct LoginGifts;
struct MGLYdiaoxiangTable;
struct MGLYmaoguiTable;
struct MGLYmaoguiwangTable;
struct MGLYshoulingTable;
struct MGLYyanseTable;
struct MagicAttributeTable;
struct MagicTable;
struct MoneyQuestTable;
struct MonsterIDTable;
struct MonsterPkTypeTable;
struct MonsterTable;
struct MountsTable;
struct NineEightTable;
struct NoticeTable;
struct NpcTalkTable;
struct OnlineTimes;
struct P20076Table;
struct ParameterTable;
struct PartnerLevelTable;
struct PartnerSkillTable;
struct PartnerTable;
struct PassiveSkillTable;
struct PowerMasterTable;
struct PulseTable;
struct QuestionTable;
struct RaidScriptTable;
struct RandomBox;
struct RandomCardRewardTable;
struct RandomCardTable;
struct RandomCollectionTable;
struct RandomDungeonTable;
struct RandomMonsterTable;
struct RankingRewardTable;
struct RecruitTable;
struct RewardBack;
struct RewardTable;
struct RobotPatrolTable;
struct SceneCreateMonsterTable;
struct SceneResTable;
struct ScriptTable;
struct SearchTable;
struct ServerLevelTable;
struct ServerResTable;
struct ShopListTable;
struct ShopTable;
struct SignDay;
struct SignMonth;
struct SkillEffectTable;
struct SkillLevelTable;
struct SkillLvTable;
struct SkillMoveTable;
struct SkillTable;
struct SkillTimeTable;
struct SpecialTitleTable;
struct SpecialtyLevelTable;
struct SpecialtySkillTable;
struct SpiritTable;
struct StageTable;
struct SyntheticTable;
struct TargetInfoEntry;
struct TargetPos;
struct TaskChapterTable;
struct TaskConditionTable;
struct TaskDropTable;
struct TaskEventTable;
struct TaskMonsterTable;
struct TaskRewardTable;
struct TaskTable;
struct TimeReward;
struct TitleFunctionTable;
struct Top10GangsTable;
struct TradingTable;
struct TransferPointTable;
struct TravelTable;
struct TreasureTable;
struct TypeLevelTable;
struct UndergroundTask;
struct WeaponsEffectTable;
struct WeekTable;
struct WorldBossRewardTable;
struct WorldBossTable;
struct raidsrv_config;
struct sproto_config_pos;
void free_AchievementFunctionTable(struct AchievementFunctionTable *p);
void free_AchievementHierarchyTable(struct AchievementHierarchyTable *p);
void free_ActiveSkillTable(struct ActiveSkillTable *p);
void free_ActiveTable(struct ActiveTable *p);
void free_ActorAttributeTable(struct ActorAttributeTable *p);
void free_ActorFashionTable(struct ActorFashionTable *p);
void free_ActorHeadTable(struct ActorHeadTable *p);
void free_ActorLevelTable(struct ActorLevelTable *p);
void free_ActorRobotTable(struct ActorRobotTable *p);
void free_ActorTable(struct ActorTable *p);
void free_AcupunctureTable(struct AcupunctureTable *p);
void free_ArenaRewardTable(struct ArenaRewardTable *p);
void free_AttributeTypeTable(struct AttributeTypeTable *p);
void free_AuctionTable(struct AuctionTable *p);
void free_BaguaStarTable(struct BaguaStarTable *p);
void free_BaguaSuitTable(struct BaguaSuitTable *p);
void free_BaguaTable(struct BaguaTable *p);
void free_BaguaViceAttributeTable(struct BaguaViceAttributeTable *p);
void free_BaseAITable(struct BaseAITable *p);
void free_BattleFieldRank(struct BattleFieldRank *p);
void free_BattlefieldTable(struct BattlefieldTable *p);
void free_BiaocheRewardTable(struct BiaocheRewardTable *p);
void free_BiaocheTable(struct BiaocheTable *p);
void free_BootNameTable(struct BootNameTable *p);
void free_BreakTable(struct BreakTable *p);
void free_BuffTable(struct BuffTable *p);
void free_CampDefenseTable(struct CampDefenseTable *p);
void free_CampTable(struct CampTable *p);
void free_CastSpiritTable(struct CastSpiritTable *p);
void free_ChallengeTable(struct ChallengeTable *p);
void free_ChangeSpecialty(struct ChangeSpecialty *p);
void free_CharmTable(struct CharmTable *p);
void free_ChivalrousTable(struct ChivalrousTable *p);
void free_CiFuTable(struct CiFuTable *p);
void free_CollectTable(struct CollectTable *p);
void free_ColourTable(struct ColourTable *p);
void free_ControlTable(struct ControlTable *p);
void free_DegreeTable(struct DegreeTable *p);
void free_DonationTable(struct DonationTable *p);
void free_DropConfigTable(struct DropConfigTable *p);
void free_DungeonTable(struct DungeonTable *p);
void free_EquipAttribute(struct EquipAttribute *p);
void free_EquipLock(struct EquipLock *p);
void free_EquipStarLv(struct EquipStarLv *p);
void free_EquipmentTable(struct EquipmentTable *p);
void free_EscortTask(struct EscortTask *p);
void free_EventCalendarTable(struct EventCalendarTable *p);
void free_FactionActivity(struct FactionActivity *p);
void free_FactionBattleTable(struct FactionBattleTable *p);
void free_FetterTable(struct FetterTable *p);
void free_FishingTable(struct FishingTable *p);
void free_FlySkillTable(struct FlySkillTable *p);
void free_FunctionUnlockTable(struct FunctionUnlockTable *p);
void free_GangsBuildTaskTable(struct GangsBuildTaskTable *p);
void free_GangsDungeonTable(struct GangsDungeonTable *p);
void free_GangsJurisdictionTable(struct GangsJurisdictionTable *p);
void free_GangsSkillTable(struct GangsSkillTable *p);
void free_GangsTable(struct GangsTable *p);
void free_GemAttribute(struct GemAttribute *p);
void free_GenerateMonster(struct GenerateMonster *p);
void free_GiftTable(struct GiftTable *p);
void free_GodYaoAttributeTable(struct GodYaoAttributeTable *p);
void free_GradeTable(struct GradeTable *p);
void free_GrowupTable(struct GrowupTable *p);
void free_ItemsConfigTable(struct ItemsConfigTable *p);
void free_LevelReward(struct LevelReward *p);
void free_LifeItemTable(struct LifeItemTable *p);
void free_LifeMagicTable(struct LifeMagicTable *p);
void free_LifeProbabilitytable(struct LifeProbabilitytable *p);
void free_LifeSkillTable(struct LifeSkillTable *p);
void free_LimitActivityControlTable(struct LimitActivityControlTable *p);
void free_LoginGifts(struct LoginGifts *p);
void free_MGLYdiaoxiangTable(struct MGLYdiaoxiangTable *p);
void free_MGLYmaoguiTable(struct MGLYmaoguiTable *p);
void free_MGLYmaoguiwangTable(struct MGLYmaoguiwangTable *p);
void free_MGLYshoulingTable(struct MGLYshoulingTable *p);
void free_MGLYyanseTable(struct MGLYyanseTable *p);
void free_MagicAttributeTable(struct MagicAttributeTable *p);
void free_MagicTable(struct MagicTable *p);
void free_MoneyQuestTable(struct MoneyQuestTable *p);
void free_MonsterIDTable(struct MonsterIDTable *p);
void free_MonsterPkTypeTable(struct MonsterPkTypeTable *p);
void free_MonsterTable(struct MonsterTable *p);
void free_MountsTable(struct MountsTable *p);
void free_NineEightTable(struct NineEightTable *p);
void free_NoticeTable(struct NoticeTable *p);
void free_NpcTalkTable(struct NpcTalkTable *p);
void free_OnlineTimes(struct OnlineTimes *p);
void free_P20076Table(struct P20076Table *p);
void free_ParameterTable(struct ParameterTable *p);
void free_PartnerLevelTable(struct PartnerLevelTable *p);
void free_PartnerSkillTable(struct PartnerSkillTable *p);
void free_PartnerTable(struct PartnerTable *p);
void free_PassiveSkillTable(struct PassiveSkillTable *p);
void free_PowerMasterTable(struct PowerMasterTable *p);
void free_PulseTable(struct PulseTable *p);
void free_QuestionTable(struct QuestionTable *p);
void free_RaidScriptTable(struct RaidScriptTable *p);
void free_RandomBox(struct RandomBox *p);
void free_RandomCardRewardTable(struct RandomCardRewardTable *p);
void free_RandomCardTable(struct RandomCardTable *p);
void free_RandomCollectionTable(struct RandomCollectionTable *p);
void free_RandomDungeonTable(struct RandomDungeonTable *p);
void free_RandomMonsterTable(struct RandomMonsterTable *p);
void free_RankingRewardTable(struct RankingRewardTable *p);
void free_RecruitTable(struct RecruitTable *p);
void free_RewardBack(struct RewardBack *p);
void free_RewardTable(struct RewardTable *p);
void free_RobotPatrolTable(struct RobotPatrolTable *p);
void free_SceneCreateMonsterTable(struct SceneCreateMonsterTable *p);
void free_SceneResTable(struct SceneResTable *p);
void free_ScriptTable(struct ScriptTable *p);
void free_SearchTable(struct SearchTable *p);
void free_ServerLevelTable(struct ServerLevelTable *p);
void free_ServerResTable(struct ServerResTable *p);
void free_ShopListTable(struct ShopListTable *p);
void free_ShopTable(struct ShopTable *p);
void free_SignDay(struct SignDay *p);
void free_SignMonth(struct SignMonth *p);
void free_SkillEffectTable(struct SkillEffectTable *p);
void free_SkillLevelTable(struct SkillLevelTable *p);
void free_SkillLvTable(struct SkillLvTable *p);
void free_SkillMoveTable(struct SkillMoveTable *p);
void free_SkillTable(struct SkillTable *p);
void free_SkillTimeTable(struct SkillTimeTable *p);
void free_SpecialTitleTable(struct SpecialTitleTable *p);
void free_SpecialtyLevelTable(struct SpecialtyLevelTable *p);
void free_SpecialtySkillTable(struct SpecialtySkillTable *p);
void free_SpiritTable(struct SpiritTable *p);
void free_StageTable(struct StageTable *p);
void free_SyntheticTable(struct SyntheticTable *p);
void free_TargetInfoEntry(struct TargetInfoEntry *p);
void free_TargetPos(struct TargetPos *p);
void free_TaskChapterTable(struct TaskChapterTable *p);
void free_TaskConditionTable(struct TaskConditionTable *p);
void free_TaskDropTable(struct TaskDropTable *p);
void free_TaskEventTable(struct TaskEventTable *p);
void free_TaskMonsterTable(struct TaskMonsterTable *p);
void free_TaskRewardTable(struct TaskRewardTable *p);
void free_TaskTable(struct TaskTable *p);
void free_TimeReward(struct TimeReward *p);
void free_TitleFunctionTable(struct TitleFunctionTable *p);
void free_Top10GangsTable(struct Top10GangsTable *p);
void free_TradingTable(struct TradingTable *p);
void free_TransferPointTable(struct TransferPointTable *p);
void free_TravelTable(struct TravelTable *p);
void free_TreasureTable(struct TreasureTable *p);
void free_TypeLevelTable(struct TypeLevelTable *p);
void free_UndergroundTask(struct UndergroundTask *p);
void free_WeaponsEffectTable(struct WeaponsEffectTable *p);
void free_WeekTable(struct WeekTable *p);
void free_WorldBossRewardTable(struct WorldBossRewardTable *p);
void free_WorldBossTable(struct WorldBossTable *p);
void free_raidsrv_config(struct raidsrv_config *p);
void free_sproto_config_pos(struct sproto_config_pos *p);
struct AchievementFunctionTable
{
	uint64_t  ID; //1
	uint64_t  FunctionID; //2
	uint64_t  HierarchyID; //3
	uint32_t n_Hierarchys; //4
	uint64_t *Hierarchys; //4
	uint64_t  Condition; //5
	uint64_t  ConditionWay; //6
}__attribute__ ((packed));

struct AchievementHierarchyTable
{
	uint64_t  ID; //1
	uint64_t  HierarchyID; //2
	uint64_t  ConditionType; //3
	uint64_t  ConditionTarget1; //4
	uint64_t  ConditionTarget2; //5
	uint64_t  ConditionNum; //6
	uint64_t  Reward; //7
	uint64_t  RewardValue; //8
	uint64_t  Title; //9
	uint64_t  NoticeID; //10
	uint64_t  ConditionTarget3; //55
}__attribute__ ((packed));

struct ActiveSkillTable
{
	uint64_t  ID; //1
	uint32_t n_SkillAction; //2
	char **SkillAction; //2
	uint32_t n_SkillLength; //3
	uint64_t *SkillLength; //3
	uint32_t n_SkillEffect; //4
	uint64_t *SkillEffect; //4
	uint64_t  WarningEffect; //5
	uint64_t  HitEffect; //6
	uint64_t  FlyId; //9
	uint64_t  CanMove; //10
	uint64_t  NextSkill; //11
	uint64_t  AutoNextSkill; //12
}__attribute__ ((packed));

struct ActiveTable
{
	uint64_t  ID; //1
	uint64_t  Active; //2
	uint32_t n_Reward; //3
	uint64_t *Reward; //3
	uint32_t n_RewardNum; //4
	uint64_t *RewardNum; //4
}__attribute__ ((packed));

struct ActorAttributeTable
{
	uint64_t  ID; //1
	uint64_t  Lv; //2
	uint64_t  DropID; //3
	uint64_t  FlySpeed; //4
	uint64_t  Health; //5
	uint64_t  MaxHP; //6
	uint64_t  Attack; //7
	uint64_t  AtGold; //8
	uint64_t  AtWood; //9
	uint64_t  AtWater; //10
	uint64_t  AtFire; //11
	uint64_t  AtEarth; //12
	uint64_t  AtGoldDf; //13
	uint64_t  AtWoodDf; //14
	uint64_t  AtWaterDf; //15
	uint64_t  AtFireDf; //16
	uint64_t  AtEarthDf; //17
	uint64_t  DfWuDel; //18
	uint64_t  Hit; //19
	uint64_t  Dodge; //20
	uint64_t  DodgeDf; //21
	uint64_t  Critical; //22
	uint64_t  CriticalDf; //23
	double  CtDmg; //24
	double  CtDmgDf; //25
	uint64_t  MoveSpeed; //26
	uint64_t  Dizzy; //27
	uint64_t  Slow; //28
	uint64_t  Mabi; //29
	uint64_t  Hurt; //30
	uint64_t  Can; //31
	uint64_t  DizzyDf; //32
	uint64_t  SlowDf; //33
	uint64_t  MabiDf; //34
	uint64_t  HurtDf; //35
	uint64_t  CanDf; //36
	uint64_t  PVPAt; //37
	uint64_t  PVPDf; //38
	uint64_t  DizzyTime; //39
	uint64_t  SlowTime; //40
	uint64_t  MabiTime; //41
	uint64_t  HurtTime; //42
	uint64_t  CanTime; //43
	uint64_t  DizzyTimeDf; //44
	uint64_t  SlowTimeDf; //45
	uint64_t  MabiTimeDf; //46
	uint64_t  HurtTimeDf; //47
	uint64_t  CanTimeDf; //48
	uint64_t  DeEffDf; //49
	uint64_t  DeTimeDf; //50
	uint64_t  CollectionDrop; //51
	uint64_t  CollectionProbability; //52
}__attribute__ ((packed));

struct ActorFashionTable
{
	uint64_t  ID; //1
	uint64_t  ResType; //2
	uint64_t  ResId; //3
	uint32_t n_ColourID1; //4
	uint64_t *ColourID1; //4
	uint32_t n_ColourID2; //5
	uint64_t *ColourID2; //5
	uint64_t  Occupation; //6
	uint64_t  Type; //7
	uint64_t  Lock; //8
	uint64_t  Colour; //9
	uint32_t n_Time; //10
	uint64_t *Time; //10
	uint32_t n_WingBinding; //11
	uint64_t *WingBinding; //11
	uint64_t  ListAcc; //12
	uint64_t  UnlockType; //13
	uint64_t  UnlockEffect1; //14
	uint64_t  UnlockEffect2; //15
	uint64_t  Charm; //16
}__attribute__ ((packed));

struct ActorHeadTable
{
	uint64_t  ID; //1
	uint32_t n_Vocation; //2
	uint64_t *Vocation; //2
	uint64_t  HeadLock; //3
	uint64_t  UnlockType; //4
	uint64_t  UnlockCondition1; //5
}__attribute__ ((packed));

struct ActorLevelTable
{
	uint64_t  ID; //1
	uint64_t  NeedExp; //2
	uint64_t  ActorLvAttri; //3
	uint64_t  FreeGrid; //4
	uint64_t  LockGrid; //5
	uint64_t  QueLvCoin; //6
	uint64_t  QueLvExp; //7
}__attribute__ ((packed));

struct ActorRobotTable
{
	uint64_t  ID; //1
	uint32_t n_ResId; //2
	uint64_t *ResId; //2
	uint32_t n_HairResId; //3
	uint64_t *HairResId; //3
	uint32_t n_WeaponId; //4
	uint64_t *WeaponId; //4
	uint32_t n_AttributeType; //5
	uint64_t *AttributeType; //5
	uint32_t n_AttributePro; //6
	uint64_t *AttributePro; //6
	uint32_t n_Skill; //7
	uint64_t *Skill; //7
	uint32_t n_PassiveSkill; //8
	uint64_t *PassiveSkill; //8
	uint64_t  InitialHead; //9
	uint32_t n_FightPro; //10
	uint64_t *FightPro; //10
	uint64_t  ActiveAttackRange; //11
	uint64_t  ChaseRange; //12
	uint64_t  Type; //13
}__attribute__ ((packed));

struct ActorTable
{
	uint64_t  ID; //1
	uint64_t  NameId; //2
	uint64_t  DescId; //3
	uint32_t n_ResId; //4
	uint64_t *ResId; //4
	uint32_t n_HairResId; //5
	uint64_t *HairResId; //5
	uint64_t  WeaponId; //6
	uint64_t  BaseAttribute; //7
	uint32_t n_Skill; //8
	uint64_t *Skill; //8
	uint32_t n_PassiveSkill; //9
	uint64_t *PassiveSkill; //9
	uint64_t  InitialHead; //10
	uint64_t  TiLv; //11
	uint64_t  LiLv; //12
	uint64_t  MinLv; //13
	uint64_t  LingLv; //14
	uint64_t  Ti; //15
	uint64_t  Li; //16
	uint64_t  Min; //17
	uint64_t  Ling; //18
	uint32_t n_SkillLevelID; //19
	uint64_t *SkillLevelID; //19
}__attribute__ ((packed));

struct AcupunctureTable
{
	uint64_t  ID; //1
	uint32_t n_AcupunctureAttribute; //2
	uint64_t *AcupunctureAttribute; //2
	uint32_t n_AttributeCeiling; //3
	uint64_t *AttributeCeiling; //3
	uint64_t  GradeNum; //4
	uint64_t  ExpendSilver; //5
	uint64_t  ExpendQi; //6
	uint64_t  Level; //7
}__attribute__ ((packed));

struct ArenaRewardTable
{
	uint64_t  ID; //1
	uint64_t  Max; //2
	uint64_t  Low; //3
	uint32_t n_ItemID; //4
	uint64_t *ItemID; //4
	uint32_t n_Num; //5
	uint64_t *Num; //5
}__attribute__ ((packed));

struct AttributeTypeTable
{
	uint64_t  ID; //1
	double  FightRatio; //2
	uint64_t  Total; //3
}__attribute__ ((packed));

struct AuctionTable
{
	uint64_t  ID; //1
	char  *ItemName; //2
	uint64_t  ItemID; //3
	uint64_t  Num; //4
	uint64_t  DefaultPrice; //5
	uint64_t  Fare; //6
	uint64_t  Price; //7
	uint64_t  Time; //8
}__attribute__ ((packed));

struct BaguaStarTable
{
	uint64_t  ID; //1
	double  AttributeValue; //2
	uint32_t n_StarItem; //3
	uint64_t *StarItem; //3
	uint32_t n_StarNum; //4
	uint64_t *StarNum; //4
	uint64_t  StarCoin; //5
	uint64_t  StatProbability; //6
	uint32_t n_DecomposeCompensation; //7
	uint64_t *DecomposeCompensation; //7
	uint32_t n_DecomposeCompensationNum; //8
	uint64_t *DecomposeCompensationNum; //8
}__attribute__ ((packed));

struct BaguaSuitTable
{
	uint64_t  ID; //1
	uint32_t n_SuitNum; //2
	uint64_t *SuitNum; //2
	uint32_t n_SuitAttributeType; //3
	uint64_t *SuitAttributeType; //3
	uint32_t n_Classification; //4
	uint64_t *Classification; //4
	uint32_t n_AttributeValue; //5
	double *AttributeValue; //5
	uint32_t n_SuitPlus1; //6
	double *SuitPlus1; //6
	uint32_t n_SuitPlus2; //7
	double *SuitPlus2; //7
	uint32_t n_SuitPlus3; //8
	double *SuitPlus3; //8
	uint32_t n_SuitPlus4; //9
	double *SuitPlus4; //9
	uint32_t n_SuitPlus5; //10
	double *SuitPlus5; //10
	uint32_t n_SuitPlus6; //11
	double *SuitPlus6; //11
}__attribute__ ((packed));

struct BaguaTable
{
	uint64_t  ID; //1
	uint64_t  Level; //2
	uint64_t  BaguaPosition; //3
	uint64_t  BaguaType; //4
	uint64_t  BaguaQuality; //5
	uint64_t  StarLv; //6
	uint64_t  Suit; //7
	uint64_t  PrimaryAttributeType; //8
	double  PrimaryAttribute; //9
	uint32_t n_ViceAttributeDatabaseSelection1; //10
	uint64_t *ViceAttributeDatabaseSelection1; //10
	uint32_t n_ViceAttributeDatabaseSelection2; //11
	uint64_t *ViceAttributeDatabaseSelection2; //11
	uint32_t n_ViceAttributeEntry; //12
	uint64_t *ViceAttributeEntry; //12
	double  coefficient; //13
	uint32_t n_DecomposeItem; //14
	uint64_t *DecomposeItem; //14
	uint32_t n_DecomposeNum; //15
	uint64_t *DecomposeNum; //15
	uint32_t n_RecastItem; //16
	uint64_t *RecastItem; //16
	uint32_t n_RecastNum; //17
	uint64_t *RecastNum; //17
	uint64_t  RecastCoin; //18
	uint32_t n_ClearItem1; //19
	uint64_t *ClearItem1; //19
	uint32_t n_ClearNum1; //20
	uint64_t *ClearNum1; //20
	uint64_t  ClearCoin1; //21
	uint32_t n_ClearItem2; //22
	uint64_t *ClearItem2; //22
	uint32_t n_ClearNum2; //23
	uint64_t *ClearNum2; //23
	uint64_t  ClearCoin2; //24
	uint32_t n_AdditionalAttributeDatabaseSelection; //31
	uint64_t *AdditionalAttributeDatabaseSelection; //31
	uint32_t n_AdditionalAttributeEntry; //32
	uint64_t *AdditionalAttributeEntry; //32
	double  Additioncoefficient; //33
}__attribute__ ((packed));

struct BaguaViceAttributeTable
{
	uint64_t  Effect; //1
	uint32_t n_Rand; //2
	double *Rand; //2
}__attribute__ ((packed));

struct BaseAITable
{
	uint64_t  BaseID; //1
	uint64_t  ActiveAttackRange; //2
	uint64_t  ChaseRange; //4
	uint64_t  MovingChange; //5
	uint64_t  Regeneration; //6
	uint64_t  AIType; //7
	uint64_t  GuardRange; //8
	uint64_t  StopMin; //9
	uint64_t  StopMax; //10
	uint64_t  Response; //11
}__attribute__ ((packed));

struct BattleFieldRank
{
	uint64_t  ID; //1
	uint64_t  LowerLimitRank1; //2
	uint64_t  UpperLimitRank1; //3
	uint32_t n_Reward1; //4
	uint64_t *Reward1; //4
	uint32_t n_Num1; //5
	uint64_t *Num1; //5
	uint64_t  LowerLimitRank2; //6
	uint64_t  UpperLimitRank2; //7
	uint32_t n_Reward2; //8
	uint64_t *Reward2; //8
	uint32_t n_Num2; //9
	uint64_t *Num2; //9
	uint64_t  LowerLimitRank3; //10
	uint64_t  UpperLimitRank3; //11
	uint32_t n_Reward3; //12
	uint64_t *Reward3; //12
	uint32_t n_Num3; //13
	uint64_t *Num3; //13
	uint64_t  LowerLimitRank4; //14
	uint64_t  UpperLimitRank4; //15
	uint32_t n_Reward4; //16
	uint64_t *Reward4; //16
	uint32_t n_Num4; //17
	uint64_t *Num4; //17
	uint64_t  LowerLimitRank5; //18
	uint64_t  UpperLimitRank5; //19
	uint32_t n_Reward5; //20
	uint64_t *Reward5; //20
	uint32_t n_Num5; //21
	uint64_t *Num5; //21
}__attribute__ ((packed));

struct BattlefieldTable
{
	uint64_t  ID; //1
	uint64_t  LowerLimitLv; //2
	uint64_t  UpperLimitLv; //3
	uint64_t  Map; //4
	uint32_t n_BirthPoint1; //5
	uint64_t *BirthPoint1; //5
	uint32_t n_BirthPoint2; //6
	uint64_t *BirthPoint2; //6
	uint64_t  ReadyTime; //7
	uint32_t n_WarSet; //8
	uint64_t *WarSet; //8
	uint32_t n_MineSet; //9
	uint64_t *MineSet; //9
	uint32_t n_ForestSet; //10
	uint64_t *ForestSet; //10
	uint64_t  Kill; //11
	uint32_t n_FirstReward; //12
	uint64_t *FirstReward; //12
	uint32_t n_Num; //13
	uint64_t *Num; //13
	double  Ratio; //14
	uint64_t  Assists; //15
	uint64_t  BottomKillMark; //16
	uint32_t n_BottomReward; //17
	uint64_t *BottomReward; //17
	uint32_t n_BottomRewardNum; //18
	uint64_t *BottomRewardNum; //18
	uint64_t  VictoryIntegral; //19
}__attribute__ ((packed));

struct BiaocheRewardTable
{
	uint64_t  ID; //1
	uint64_t  Deposit; //2
	uint32_t n_StartTime; //3
	uint64_t *StartTime; //3
	uint32_t n_EndTime; //4
	uint64_t *EndTime; //4
	uint64_t  RewardMoney1; //5
	uint64_t  RewardExp1; //6
	uint64_t  RewardLv1; //7
	uint32_t n_RewardItem1; //8
	uint64_t *RewardItem1; //8
	uint32_t n_RewardNum1; //9
	uint64_t *RewardNum1; //9
	uint64_t  Collection; //10
	uint64_t  RewardMoney2; //11
	uint64_t  RewardExp2; //12
	uint64_t  RewardLv2; //13
	uint32_t n_RewardItem2; //14
	uint64_t *RewardItem2; //14
	uint32_t n_RewardNum2; //15
	uint64_t *RewardNum2; //15
}__attribute__ ((packed));

struct BiaocheTable
{
	uint64_t  ID; //1
	uint64_t  Type; //2
	uint64_t  ActivityControl; //3
	uint64_t  TaskId; //4
	uint64_t  MonsterId; //5
	uint32_t n_shanfeiId; //6
	uint64_t *shanfeiId; //6
	uint32_t n_level; //7
	uint64_t *level; //7
	uint64_t  Interval; //8
	uint64_t  Probability; //9
	uint64_t  Time; //10
	uint32_t n_Number; //11
	uint64_t *Number; //11
	uint64_t  Range; //12
	uint64_t  Reward; //13
	uint32_t n_Point; //14
	double *Point; //14
}__attribute__ ((packed));

struct BootNameTable
{
	uint64_t  ID; //1
	char  *Name; //2
}__attribute__ ((packed));

struct BreakTable
{
	uint64_t  ID; //1
	uint32_t n_PulseAttribute; //2
	uint64_t *PulseAttribute; //2
	uint32_t n_AttributeLower; //3
	uint64_t *AttributeLower; //3
	uint32_t n_AttributeUpper; //4
	uint64_t *AttributeUpper; //4
	uint64_t  PulseLv; //5
	uint32_t n_AttributeColor; //8
	uint64_t *AttributeColor; //8
	uint32_t n_MeridiansProbability; //9
	uint64_t *MeridiansProbability; //9
	uint32_t n_BloodProbability; //10
	uint64_t *BloodProbability; //10
	uint32_t n_VitalProbability; //11
	uint64_t *VitalProbability; //11
	uint32_t n_MarrowProbability; //12
	uint64_t *MarrowProbability; //12
	uint64_t  Minimum; //13
	uint32_t n_MeridiansMinimum; //14
	uint64_t *MeridiansMinimum; //14
	uint32_t n_BloodMinimum; //15
	uint64_t *BloodMinimum; //15
	uint32_t n_VitalMinimum; //16
	uint64_t *VitalMinimum; //16
	uint32_t n_MarrowMinimum; //17
	uint64_t *MarrowMinimum; //17
	uint64_t  Time; //18
	uint64_t  Secondary; //19
	uint64_t  Lost; //20
}__attribute__ ((packed));

struct BuffTable
{
	uint64_t  ID; //1
	uint64_t  BuffType; //2
	uint64_t  NeedPro; //3
	uint64_t  Time; //4
	uint64_t  Interval; //5
	uint32_t n_EffectID; //6
	uint64_t *EffectID; //6
	uint32_t n_DelEffectID; //7
	uint64_t *DelEffectID; //7
	uint64_t  DfPro; //8
	uint64_t  TimeDelay; //9
	uint64_t  CoverType; //10
	uint32_t n_DeleteType; //11
	uint64_t *DeleteType; //11
	uint64_t  IsDeBuff; //12
	uint64_t  IsControl; //13
	uint64_t  CoverType1; //14
	uint64_t  BuffLv; //15
}__attribute__ ((packed));

struct CampDefenseTable
{
	uint64_t  ID; //1
	uint64_t  TruckID; //2
	uint32_t n_TruckRouteX; //3
	uint64_t *TruckRouteX; //3
	uint32_t n_TruckRouteY; //4
	uint64_t *TruckRouteY; //4
	uint64_t  TruckPlan; //5
	uint64_t  ResurrectionTime; //6
	uint64_t  TruckDrop1; //7
	uint64_t  CollectionIntegral1; //8
	uint64_t  TruckDrop2; //9
	uint64_t  CollectionIntegral2; //10
	uint32_t n_MonsterID; //11
	uint64_t *MonsterID; //11
	uint32_t n_MineralIntegral1; //12
	uint64_t *MineralIntegral1; //12
	uint32_t n_MineralIntegral2; //13
	uint64_t *MineralIntegral2; //13
	uint32_t n_SupportMine; //14
	uint64_t *SupportMine; //14
	uint32_t n_TaskID; //15
	uint64_t *TaskID; //15
	uint64_t  MiningLimit1; //16
	uint64_t  MiningLimit2; //17
	uint64_t  DropNum1; //18
	uint64_t  DropNum2; //19
	uint32_t n_ProtectMonsterID; //20
	uint64_t *ProtectMonsterID; //20
	uint32_t n_ProtectMonsterNum; //21
	uint64_t *ProtectMonsterNum; //21
	uint64_t  ProtectBuff; //22
}__attribute__ ((packed));

struct CampTable
{
	uint64_t  ID; //1
	uint64_t  Level; //2
	uint64_t  FreeLv; //3
	uint64_t  FreeNum; //4
	uint64_t  MaxExp; //5
	uint64_t  Consume; //6
}__attribute__ ((packed));

struct CastSpiritTable
{
	uint64_t  ID; //1
	uint32_t n_CastExpend; //2
	uint64_t *CastExpend; //2
	uint32_t n_Expend1Num; //3
	uint64_t *Expend1Num; //3
	uint32_t n_MountsAttribute; //4
	uint64_t *MountsAttribute; //4
	uint32_t n_AttributeCeiling; //5
	uint64_t *AttributeCeiling; //5
	uint64_t  GradeATttribute; //6
	uint64_t  GradeATttributeNum; //7
}__attribute__ ((packed));

struct ChallengeTable
{
	uint64_t  ID; //1
	uint64_t  TypeLevel; //2
	uint64_t  MonsterID; //3
	uint64_t  OpenLevel; //4
	uint64_t  DungeonID; //5
	uint64_t  Times; //6
}__attribute__ ((packed));

struct ChangeSpecialty
{
	uint64_t  ID; //1
	uint64_t  ChangeCost; //2
	uint64_t  CostValue; //3
}__attribute__ ((packed));

struct CharmTable
{
	uint64_t  ID; //0
	uint64_t  Exp; //1
	uint32_t n_AttributeType; //2
	uint64_t *AttributeType; //2
	uint32_t n_AttributeNum; //3
	uint64_t *AttributeNum; //3
}__attribute__ ((packed));

struct ChivalrousTable
{
	uint64_t  ID; //1
	uint64_t  Condition1; //2
	uint64_t  SingleNum; //3
	uint64_t  MaxNum; //4
}__attribute__ ((packed));

struct CiFuTable
{
	uint64_t  ID; //1
	uint64_t  ControlTable; //2
	uint64_t  Type; //3
	uint64_t  Vaule; //4
}__attribute__ ((packed));

struct CollectTable
{
	uint64_t  ID; //1
	uint32_t n_TaskId; //2
	uint64_t *TaskId; //2
	uint64_t  DisplayLevel; //3
	uint64_t  Level; //4
	uint64_t  Time; //5
	uint64_t  Regeneration; //6
	uint32_t n_DropID; //7
	uint64_t *DropID; //7
	uint64_t  LifeTime; //8
	char  *NameId; //9
	uint64_t  CollectionTeyp; //10
	uint64_t  ConsumeTeyp; //11
	uint32_t n_Parameter1; //12
	uint64_t *Parameter1; //12
	uint32_t n_Parameter2; //13
	uint64_t *Parameter2; //13
	double  CollectionSize; //14
	uint32_t n_TaskIdShow; //15
	uint64_t *TaskIdShow; //15
	uint64_t  DropType; //16
	uint32_t n_Drop1; //17
	uint64_t *Drop1; //17
	uint32_t n_Drop2; //18
	uint64_t *Drop2; //18
}__attribute__ ((packed));

struct ColourTable
{
	uint64_t  Id; //1
	uint64_t  OpenColourItem; //2
	uint64_t  OpenColourNum; //3
	uint64_t  ColourItem; //4
	uint64_t  ColourNum; //5
}__attribute__ ((packed));

struct ControlTable
{
	uint64_t  ID; //1
	uint64_t  MinActor; //2
	uint64_t  MaxActor; //3
	uint32_t n_OpenDay; //4
	uint64_t *OpenDay; //4
	uint32_t n_OpenTime; //5
	uint64_t *OpenTime; //5
	uint32_t n_CloseTime; //6
	uint64_t *CloseTime; //6
	uint64_t  TimeType; //7
	uint64_t  RewardTime; //8
}__attribute__ ((packed));

struct DegreeTable
{
	uint64_t  ID; //1
	uint64_t  Stage; //2
	uint64_t  Value; //3
	uint32_t n_Function; //4
	uint64_t *Function; //4
}__attribute__ ((packed));

struct DonationTable
{
	uint64_t  ID; //1
	uint64_t  Type; //2
	uint64_t  ConsumeType; //3
	uint64_t  ConsumeValue; //4
	uint32_t n_RewardType; //5
	uint64_t *RewardType; //5
	uint32_t n_RewardValue; //6
	uint64_t *RewardValue; //6
	uint64_t  RewardEffect; //7
}__attribute__ ((packed));

struct DropConfigTable
{
	uint64_t  ID; //1
	uint64_t  ProType; //2
	uint32_t n_DropID; //3
	uint64_t *DropID; //3
	uint32_t n_Probability; //4
	uint64_t *Probability; //4
	uint32_t n_NumMin; //5
	uint64_t *NumMin; //5
	uint32_t n_NumMax; //6
	uint64_t *NumMax; //6
}__attribute__ ((packed));

struct DungeonTable
{
	uint64_t  DungeonID; //1
	uint64_t  DengeonType; //2
	uint64_t  DengeonRank; //3
	uint64_t  AddMidway; //4
	uint64_t  CostItemID; //5
	uint64_t  CostNum; //6
	uint32_t n_Score; //7
	uint64_t *Score; //7
	uint32_t n_ScoreValue; //8
	uint64_t *ScoreValue; //8
	uint32_t n_ScoreValue1; //9
	uint64_t *ScoreValue1; //9
	uint32_t n_Rewards; //10
	uint64_t *Rewards; //10
	double  ExitPointX; //11
	double  BirthPointY; //12
	double  BirthPointZ; //13
	double  FaceY; //14
	uint64_t  ExitScene; //15
	uint32_t n_PassType; //16
	uint64_t *PassType; //16
	uint32_t n_PassValue; //17
	uint64_t *PassValue; //17
	uint32_t n_PassValue1; //18
	uint64_t *PassValue1; //18
	uint64_t  RandomGrade; //19
	uint32_t n_RandomID; //20
	uint64_t *RandomID; //20
	uint64_t  RandomNum; //21
	uint32_t n_wanyaoka; //22
	uint64_t *wanyaoka; //22
	uint32_t n_RewardPosition; //23
	double *RewardPosition; //23
	uint64_t  InstantRelive; //24
	uint64_t  AutomaticRelive; //25
	uint32_t n_ReliveTime; //26
	uint64_t *ReliveTime; //26
	uint64_t  ActivityControl; //27
	char  *DungeonPass; //28
	uint64_t  TaskID; //29
	uint32_t n_FailType; //30
	uint64_t *FailType; //30
	uint32_t n_FailValue; //31
	uint64_t *FailValue; //31
	uint32_t n_FailValue1; //32
	uint64_t *FailValue1; //32
	uint32_t n_ItemRewardSection; //33
	uint64_t *ItemRewardSection; //33
	uint64_t  ExpReward; //34
	uint64_t  MoneyReward; //35
	uint64_t  DynamicLevel; //36
	uint64_t  ExpReward1; //37
	uint64_t  MoneyReward1; //38
}__attribute__ ((packed));

struct EquipAttribute
{
	uint64_t  ID; //1
	uint64_t  Database; //2
	uint64_t  Effect; //3
	uint32_t n_Rand; //4
	double *Rand; //4
	uint32_t n_QualityWeight; //5
	uint64_t *QualityWeight; //5
	double  FluctuationValue1; //6
}__attribute__ ((packed));

struct EquipLock
{
	uint64_t  ID; //1
	uint32_t n_LockLv; //2
	uint64_t *LockLv; //2
	uint32_t n_LockQuality; //3
	uint64_t *LockQuality; //3
	uint32_t n_LockStar; //4
	uint64_t *LockStar; //4
	uint32_t n_LockItem; //5
	uint64_t *LockItem; //5
	uint32_t n_LockItemNum; //6
	uint64_t *LockItemNum; //6
	uint32_t n_MosaicType; //7
	uint64_t *MosaicType; //7
	uint32_t n_EnchantQualityLock; //11
	uint64_t *EnchantQualityLock; //11
	uint32_t n_EnchantStarLock; //12
	uint64_t *EnchantStarLock; //12
	uint32_t n_EnchantItem; //13
	uint64_t *EnchantItem; //13
	uint32_t n_EnchantItemNum; //14
	uint64_t *EnchantItemNum; //14
	uint32_t n_EnchantCoin; //15
	uint64_t *EnchantCoin; //15
}__attribute__ ((packed));

struct EquipStarLv
{
	uint64_t  ID; //1
	uint64_t  Level; //2
	uint64_t  StarSchedule; //3
	uint64_t  ConsumeItem; //4
	uint64_t  ConsumeCoin; //5
	uint64_t  Quality; //7
	uint64_t  StarLv; //8
	uint32_t n_DatabaseSelection; //9
	uint64_t *DatabaseSelection; //9
}__attribute__ ((packed));

struct EquipmentTable
{
	uint64_t  ID; //1
	uint64_t  Occupation; //2
	uint64_t  EquipmentPosition; //3
	uint64_t  EquipmentQuality; //4
	uint64_t  AttriEquipType; //5
	double  AttriEquipValue; //6
	uint64_t  StarLvID; //7
}__attribute__ ((packed));

struct EscortTask
{
	uint64_t  ID; //1
	uint64_t  NpcID; //2
	uint64_t  Scene; //3
	uint64_t  MonsterIndex; //4
	uint64_t  Time; //5
	uint64_t  Team; //6
	uint64_t  Distance; //7
	uint64_t  BlockedStop; //8
	uint64_t  Range; //9
	uint32_t n_MonsterID; //10
	uint64_t *MonsterID; //10
	uint32_t n_MonsterID1; //11
	uint64_t *MonsterID1; //11
	uint32_t n_PointXZ1; //12
	double *PointXZ1; //12
	uint32_t n_MonsterID2; //13
	uint64_t *MonsterID2; //13
	uint32_t n_PointXZ2; //14
	double *PointXZ2; //14
	uint32_t n_MonsterID3; //15
	uint64_t *MonsterID3; //15
	uint32_t n_PointXZ3; //16
	double *PointXZ3; //16
	uint32_t n_MonsterID4; //17
	uint64_t *MonsterID4; //17
	uint32_t n_PointXZ4; //18
	double *PointXZ4; //18
	uint32_t n_talkID; //19
	uint64_t *talkID; //19
}__attribute__ ((packed));

struct EventCalendarTable
{
	uint64_t  ID; //1
	uint64_t  RelationID; //2
	uint64_t  ChivalrousID; //3
	uint64_t  HotspotID; //4
	uint64_t  SubtabCondition; //5
	uint64_t  SubtabValue; //6
	uint64_t  ActivityType; //7
	uint64_t  ActivityValue; //8
	uint32_t n_AuxiliaryValue; //9
	uint64_t *AuxiliaryValue; //9
	uint64_t  Sum; //10
	uint64_t  Active; //11
	uint64_t  ResetCD; //12
}__attribute__ ((packed));

struct FactionActivity
{
	uint64_t  ID; //1
	uint64_t  ControlID; //2
	char  *DungeonPass1; //3
	char  *DungeonPass2; //4
	uint32_t n_FailType; //5
	uint64_t *FailType; //5
	uint32_t n_FailValue; //6
	uint64_t *FailValue; //6
	uint32_t n_FailValue1; //7
	uint64_t *FailValue1; //7
}__attribute__ ((packed));

struct FactionBattleTable
{
	uint64_t  ID; //1
	uint64_t  LowerLimitLv; //2
	uint64_t  UpperLimitLv; //3
	uint64_t  Map; //4
	uint32_t n_BirthPoint1; //5
	double *BirthPoint1; //5
	uint32_t n_BirthPoint2; //6
	double *BirthPoint2; //6
	uint64_t  FlagExp; //7
	uint64_t  BoxID; //8
	uint32_t n_BoxReloadX; //9
	double *BoxReloadX; //9
	uint32_t n_BoxReloadY; //10
	double *BoxReloadY; //10
	uint32_t n_BoxReloadZ; //11
	double *BoxReloadZ; //11
	uint64_t  BoxReloadTime; //12
	uint64_t  BoxReloadNum; //13
	uint64_t  BoxExp; //14
	uint64_t  BoxExp2; //15
	uint64_t  BoxOpenNum; //16
	uint64_t  SkillIntegral; //17
	uint64_t  FlagIntegral; //18
	uint64_t  Level; //19
}__attribute__ ((packed));

struct FetterTable
{
	uint64_t  ID; //1
	uint64_t  Relation; //2
	uint32_t n_Partner; //4
	uint64_t *Partner; //4
	uint64_t  AttributeType; //5
	uint64_t  AttributeNum; //6
	uint64_t  LevelItem; //7
	uint64_t  ItemNum; //8
}__attribute__ ((packed));

struct FishingTable
{
	uint64_t  ID; //1
	uint64_t  Stosh; //2
	uint64_t  TimeMin; //3
	uint64_t  TimeMax; //4
	uint64_t  Response; //5
	uint64_t  Drop; //6
	uint64_t  Drop1; //7
}__attribute__ ((packed));

struct FlySkillTable
{
	uint64_t  ID; //1
	uint64_t  DurationTime; //2
	uint64_t  MoveType; //3
	uint64_t  Interval; //4
	uint64_t  MoveDistance; //5
	uint64_t  MoveSpeed; //6
}__attribute__ ((packed));

struct FunctionUnlockTable
{
	uint64_t  ID; //1
	uint64_t  Type; //2
	uint64_t  Level; //3
	uint64_t  IsSoon; //4
	uint64_t  ItemID; //5
	uint64_t  ItemNum; //6
	uint64_t  Quest; //7
}__attribute__ ((packed));

struct GangsBuildTaskTable
{
	uint64_t  ID; //1
	uint32_t n_Level; //2
	uint64_t *Level; //2
	uint32_t n_Tasklibrary; //3
	uint64_t *Tasklibrary; //3
	uint64_t  DropID; //4
	uint64_t  LeveTime; //5
	uint64_t  Times; //6
}__attribute__ ((packed));

struct GangsDungeonTable
{
	uint64_t  ID; //1
	uint64_t  Type; //2
	uint64_t  RankHigh; //3
	uint64_t  RankLow; //4
	uint64_t  mailID; //5
	uint32_t n_RewardID; //6
	uint64_t *RewardID; //6
	uint32_t n_RewardNum; //7
	uint64_t *RewardNum; //7
}__attribute__ ((packed));

struct GangsJurisdictionTable
{
	uint64_t  Position; //1
	uint64_t  Appoint; //2
	uint64_t  NoticeSetup; //3
	uint64_t  BuildingUp; //4
	uint64_t  OpenActivity; //5
	uint64_t  RecruitSetup; //6
	uint64_t  skill; //7
	uint64_t  GangsName; //8
	uint64_t  Recruit; //9
	uint64_t  Expel; //10
	uint64_t  Invitation; //11
	char  *Name; //12
}__attribute__ ((packed));

struct GangsSkillTable
{
	uint64_t  ID; //1
	uint64_t  skillLeve; //2
	uint64_t  skillType; //3
	double  SkillValue; //4
	uint64_t  CreateLeve; //5
	uint64_t  CreateMnoney; //6
	uint64_t  UseMoney1; //7
	uint64_t  UseMoney2; //8
	uint64_t  BuildingLeve; //9
	char  *skillName; //10
	uint64_t  CreateWood; //11
}__attribute__ ((packed));

struct GangsTable
{
	uint64_t  ID; //1
	uint64_t  BuildingType; //2
	uint64_t  BuildingLeve; //4
	uint64_t  HallLeve; //5
	uint64_t  Leve1Expend; //6
	uint64_t  Leve2Expend; //7
	uint64_t  LeveCompetence; //8
	uint64_t  LeveTime; //9
	uint64_t  MaintenanceCosts; //10
	uint64_t  PopularityCost; //11
	uint64_t  parameter1; //12
	uint64_t  parameter2; //13
	uint64_t  parameter3; //14
	uint32_t n_parameter4; //15
	uint64_t *parameter4; //15
	char  *BuildingName; //16
}__attribute__ ((packed));

struct GemAttribute
{
	uint64_t  ID; //1
	uint64_t  GemType; //2
	uint64_t  AttributeType; //3
	double  AttributeValue; //4
	uint64_t  GemSynthetic; //5
	uint64_t  Number; //6
	uint64_t  Consumption; //7
	uint64_t  Level; //8
}__attribute__ ((packed));

struct GenerateMonster
{
	uint64_t  ID; //1
	uint64_t  MonsterPointID; //2
	uint64_t  MonsterID; //6
	uint32_t n_MovePointXZ; //7
	uint64_t *MovePointXZ; //7
	uint64_t  MonsterTime; //9
	uint64_t  MonsterMax; //10
}__attribute__ ((packed));

struct GiftTable
{
	uint64_t  ID; //1
	uint64_t  TypeId; //2
	uint64_t  CoinType; //3
	uint64_t  Value; //4
}__attribute__ ((packed));

struct GodYaoAttributeTable
{
	uint64_t  ID; //1
	uint64_t  AttributeType; //2
	uint64_t  AttributeNum; //3
	uint64_t  Coefficient; //4
}__attribute__ ((packed));

struct GradeTable
{
	uint64_t  ID; //1
	uint64_t  LevelExp; //2
	uint64_t  Level; //3
	uint32_t n_AttributeType; //4
	uint64_t *AttributeType; //4
	uint32_t n_AttributeTypeValue; //5
	uint64_t *AttributeTypeValue; //5
	uint32_t n_DayReward; //6
	uint64_t *DayReward; //6
	uint32_t n_DayRewardNum; //7
	uint64_t *DayRewardNum; //7
	uint32_t n_BreachReward; //8
	uint64_t *BreachReward; //8
	uint32_t n_BreachRewardNum; //9
	uint64_t *BreachRewardNum; //9
	uint64_t  SmallLevel; //10
}__attribute__ ((packed));

struct GrowupTable
{
	uint64_t  ID; //1
	uint64_t  Type; //2
	uint64_t  ConditionType; //3
	uint64_t  ConditionTarget1; //4
	uint64_t  ConditionTarget2; //5
	uint64_t  ConditionNum; //6
	uint32_t n_RewardType; //7
	uint64_t *RewardType; //7
	uint32_t n_RewardValue; //8
	uint64_t *RewardValue; //8
	uint64_t  Reward; //9
	uint32_t n_LevelRange; //10
	uint64_t *LevelRange; //10
	uint64_t  ConditionTarget3; //55
}__attribute__ ((packed));

struct ItemsConfigTable
{
	uint64_t  ID; //1
	char  *Name; //2
	uint64_t  Camp; //3
	uint64_t  ItemLevel; //4
	uint64_t  ItemQuality; //5
	uint64_t  ItemType; //6
	uint64_t  SkillId; //7
	uint64_t  Price; //8
	uint64_t  BindType; //9
	uint64_t  ItemRelation; //10
	uint64_t  UseDegree; //11
	uint64_t  ItemLimit; //12
	uint64_t  Stackable; //13
	uint64_t  TaskId; //14
	uint64_t  DropId; //15
	uint64_t  ItemEffect; //16
	uint32_t n_ParameterEffect; //17
	uint64_t *ParameterEffect; //17
	uint64_t  CostTime; //18
	uint64_t  ItemCD; //19
	uint64_t  BuffId; //20
}__attribute__ ((packed));

struct LevelReward
{
	uint64_t  ID; //1
	uint64_t  Level; //2
	uint32_t n_ItemID; //3
	uint64_t *ItemID; //3
	uint32_t n_ItemValue; //4
	uint64_t *ItemValue; //4
}__attribute__ ((packed));

struct LifeItemTable
{
	uint64_t  ID; //1
	uint64_t  Type; //2
	uint64_t  SkillNeed; //4
	uint64_t  ItemID; //5
}__attribute__ ((packed));

struct LifeMagicTable
{
	uint64_t  ID; //1
	uint32_t n_LifeMagic; //2
	uint64_t *LifeMagic; //2
	uint64_t  NeedJingli; //3
	uint64_t  NeedItem; //4
	uint64_t  NeedItemNum; //5
	uint64_t  Life1; //6
	uint64_t  LifeNum1; //7
	uint32_t n_LifeProbability1; //8
	uint64_t *LifeProbability1; //8
	uint32_t n_Magic1; //9
	uint64_t *Magic1; //9
	uint64_t  Life2; //10
	uint64_t  LifeNum2; //11
	uint32_t n_LifeProbability2; //12
	uint64_t *LifeProbability2; //12
	uint32_t n_Magic2; //13
	uint64_t *Magic2; //13
	uint64_t  Life3; //14
	uint64_t  LifeNum3; //15
	uint32_t n_LifeProbability3; //16
	uint64_t *LifeProbability3; //16
	uint32_t n_Magic3; //17
	uint64_t *Magic3; //17
	uint64_t  Life4; //18
	uint64_t  LifeNum4; //19
	uint32_t n_LifeProbability4; //20
	uint64_t *LifeProbability4; //20
	uint32_t n_Magic4; //21
	uint64_t *Magic4; //21
}__attribute__ ((packed));

struct LifeProbabilitytable
{
	uint64_t  Life; //1
	uint64_t  LifeNum; //2
	uint32_t n_LifeProbability; //3
	uint64_t *LifeProbability; //3
	uint32_t n_Magic; //4
	uint64_t *Magic; //4
}__attribute__ ((packed));

struct LifeSkillTable
{
	uint64_t  ID; //1
	uint64_t  Type; //2
	uint64_t  Lv; //3
	uint64_t  LvMax; //4
	uint64_t  Exp; //5
	uint64_t  ExpAdd; //6
	uint64_t  NeedCoin; //7
	uint64_t  NeedItem; //8
	uint64_t  BreachLv; //9
	uint64_t  BreachItem; //10
	uint64_t  BreachNum; //11
	uint64_t  ProMax; //12
	uint32_t n_LvPro; //13
	uint64_t *LvPro; //13
	uint32_t n_ItemID; //14
	uint64_t *ItemID; //14
	uint64_t  NeedDonation; //15
	uint64_t  NeedJingli; //16
}__attribute__ ((packed));

struct LimitActivityControlTable
{
	uint64_t  ID; //1
	uint64_t  Activity; //2
	uint64_t  OpenType; //3
	uint64_t  ContinuedOpenTime; //4
	uint64_t  ContinuedTime; //5
	uint64_t  Batch; //6
}__attribute__ ((packed));

struct LoginGifts
{
	uint64_t  ID; //1
	uint64_t  LoginDays; //2
	uint64_t  RewardsType; //3
	uint32_t n_lRewards; //4
	uint64_t *lRewards; //4
	uint32_t n_Quantity; //5
	uint64_t *Quantity; //5
	uint64_t  Time; //6
}__attribute__ ((packed));

struct MGLYdiaoxiangTable
{
	uint64_t  ID; //1
	uint64_t  MonsterPointID; //2
	uint64_t  Scene; //3
	uint64_t  MonsterID; //4
	uint32_t n_MovePointX; //5
	double *MovePointX; //5
	uint32_t n_MovePointZ; //6
	double *MovePointZ; //6
	uint32_t n_StopTime; //7
	uint64_t *StopTime; //7
	char  *Effects; //8
	uint64_t  MonsterTime; //9
	uint64_t  MonsterMax; //10
	uint32_t n_EffectsParameter; //11
	uint64_t *EffectsParameter; //11
}__attribute__ ((packed));

struct MGLYmaoguiTable
{
	uint64_t  ID; //1
	uint64_t  MonsterID; //2
	uint64_t  Separate; //3
	uint32_t n_SeparateMonster; //4
	uint64_t *SeparateMonster; //4
	uint64_t  SeparateNum; //5
	uint64_t  SeparateRange; //6
	char  *Effects; //7
	uint32_t n_EffectsParameter; //8
	uint64_t *EffectsParameter; //8
}__attribute__ ((packed));

struct MGLYmaoguiwangTable
{
	uint64_t  ID; //1
	uint64_t  BossID; //2
	uint32_t n_Monster1; //3
	uint64_t *Monster1; //3
	uint32_t n_Monster2; //4
	uint64_t *Monster2; //4
	uint64_t  SeparateRange; //5
	uint64_t  Time; //6
	char  *Effects; //7
	uint32_t n_EffectsParameter; //8
	uint64_t *EffectsParameter; //8
	uint64_t  CallTime; //9
}__attribute__ ((packed));

struct MGLYshoulingTable
{
	uint64_t  ID; //1
	uint64_t  BossID; //2
	uint32_t n_MonsterID; //3
	uint64_t *MonsterID; //3
}__attribute__ ((packed));

struct MGLYyanseTable
{
	uint64_t  ID; //1
	uint64_t  MonsterID; //2
	uint64_t  Type; //3
	uint64_t  Colour; //4
}__attribute__ ((packed));

struct MagicAttributeTable
{
	uint64_t  Effect; //1
	uint32_t n_Rand; //2
	double *Rand; //2
}__attribute__ ((packed));

struct MagicTable
{
	uint64_t  ID; //1
	uint64_t  Quality; //2
	uint64_t  MainAttribute; //3
	uint32_t n_MainAttributeNum; //4
	uint64_t *MainAttributeNum; //4
	uint32_t n_ViceAttribute22; //5
	uint64_t *ViceAttribute22; //5
	uint32_t n_ViceAttribute; //6
	uint64_t *ViceAttribute; //6
}__attribute__ ((packed));

struct MoneyQuestTable
{
	uint64_t  ID; //1
	uint32_t n_LevelSection; //2
	uint64_t *LevelSection; //2
	uint32_t n_QuestGroup; //3
	uint64_t *QuestGroup; //3
	uint64_t  RewardGroup; //4
	uint32_t n_QualityGroup; //5
	uint64_t *QualityGroup; //5
	uint64_t  RewardGroup1; //6
	uint64_t  RewardGroup2; //7
	uint64_t  RewardGroup3; //8
	uint64_t  RewardGroup4; //9
	uint64_t  RewardGroup5; //10
	uint32_t n_ExpReward; //11
	uint64_t *ExpReward; //11
	uint32_t n_MoneyReward; //12
	uint64_t *MoneyReward; //12
}__attribute__ ((packed));

struct MonsterIDTable
{
	uint64_t  ID; //1
	uint32_t n_MonseterID; //2
	uint64_t *MonseterID; //2
}__attribute__ ((packed));

struct MonsterPkTypeTable
{
	uint64_t  ID; //0
	uint64_t  Remark1; //1
	uint64_t  PkType0; //2
	uint64_t  PkType1; //3
	uint64_t  PkType2; //4
	uint64_t  PkType3; //5
	uint64_t  PkType4; //6
	uint64_t  PkType5; //7
	uint64_t  PkType6; //8
	uint64_t  PkType7; //9
	uint64_t  PkType8; //10
	uint64_t  PkType9; //11
	uint64_t  PkType10; //12
	uint64_t  PkType11; //13
	uint64_t  PkType12; //14
	uint64_t  PkType13; //15
	uint64_t  PkType14; //16
	uint64_t  PkType15; //17
	uint64_t  PkType16; //18
	uint64_t  PkType17; //19
	uint64_t  PkType18; //20
	uint64_t  PkType19; //21
	uint64_t  PkType20; //22
}__attribute__ ((packed));

struct MonsterTable
{
	uint64_t  ID; //1
	uint64_t  NameId; //2
	uint64_t  ResId; //3
	uint64_t  BaseAttribute; //4
	uint32_t n_Skill; //5
	uint64_t *Skill; //5
	uint32_t n_PassiveSkill; //6
	uint64_t *PassiveSkill; //6
	uint64_t  MapDisplay; //7
	uint64_t  BaseID; //8
	uint64_t  Recovery; //9
	uint64_t  HateDao; //10
	uint64_t  HateGong; //11
	uint64_t  HateBi; //12
	uint64_t  HateQiang; //13
	uint64_t  HateFazhang; //14
	uint64_t  HateMonster; //15
	uint64_t  HateType; //16
	uint64_t  Camp; //17
	uint64_t  AttackType; //18
	uint64_t  Type; //19
	uint64_t  LifeTime; //20
	struct NpcTalkTable  *talk_config; //21
	uint64_t  PkType; //22
	uint64_t  HateBase; //23
	uint64_t  BirthTime; //24
}__attribute__ ((packed));

struct MountsTable
{
	uint64_t  ID; //1
	uint64_t  BaseAttribute; //2
	uint32_t n_Time; //3
	uint64_t *Time; //3
	uint32_t n_WingBinding; //4
	uint64_t *WingBinding; //4
	uint32_t n_Speed; //5
	uint64_t *Speed; //5
	char  *Name; //6
	uint32_t n_Item; //7
	uint64_t *Item; //7
	uint32_t n_ItemNum; //8
	uint64_t *ItemNum; //8
	uint64_t  CastSpiritLimit; //9
	uint32_t n_Binding; //10
	uint64_t *Binding; //10
}__attribute__ ((packed));

struct NineEightTable
{
	uint64_t  ID; //1
	uint64_t  TaskID; //2
	uint32_t n_Reward; //3
	uint64_t *Reward; //3
	uint32_t n_RewardNum; //4
	uint64_t *RewardNum; //4
}__attribute__ ((packed));

struct NoticeTable
{
	uint64_t  ID; //1
	uint64_t  Priority; //2
	char  *NoticeTxt; //3
	uint32_t n_NoticeChannel; //4
	uint64_t *NoticeChannel; //4
	uint64_t  Effect; //5
}__attribute__ ((packed));

struct NpcTalkTable
{
	uint64_t  ID; //1
	uint64_t  NpcId; //2
	uint64_t  Type; //3
	uint64_t  EventNum1; //4
	uint32_t n_EventNum2; //5
	uint64_t *EventNum2; //5
	struct NpcTalkTable  *next; //6
}__attribute__ ((packed));

struct OnlineTimes
{
	uint64_t  ID; //1
	uint64_t  Times; //2
}__attribute__ ((packed));

struct P20076Table
{
	uint64_t  ID; //1
	uint32_t n_MonsterID; //2
	uint64_t *MonsterID; //2
	uint32_t n_MonsterNum; //3
	uint64_t *MonsterNum; //3
	uint64_t  BirthPointX; //4
	uint64_t  BirthPointZ; //5
	uint64_t  BirthRange; //6
	uint32_t n_Reward1; //7
	uint64_t *Reward1; //7
	uint32_t n_RewardNum1; //8
	uint64_t *RewardNum1; //8
	uint32_t n_Reward2; //9
	uint64_t *Reward2; //9
	uint32_t n_RewardNum2; //10
	uint64_t *RewardNum2; //10
	uint64_t  ExpReward; //11
	uint64_t  MoneyReward; //12
	uint32_t n_MonsterLevel; //13
	uint64_t *MonsterLevel; //13
}__attribute__ ((packed));

struct ParameterTable
{
	uint64_t  ID; //1
	uint32_t n_parameter1; //2
	double *parameter1; //2
	char  *parameter2; //3
}__attribute__ ((packed));

struct PartnerLevelTable
{
	uint64_t  ID; //1
	uint64_t  NeedExp; //2
}__attribute__ ((packed));

struct PartnerSkillTable
{
	uint64_t  ID; //1
	uint64_t  BaseSkill1; //2
	uint64_t  ProtectSkill; //3
	uint64_t  Angerskill; //4
	uint32_t n_SkillProbability; //5
	uint64_t *SkillProbability; //5
	uint32_t n_Skill; //6
	uint64_t *Skill; //6
	uint32_t n_SkillProbability1; //7
	uint64_t *SkillProbability1; //7
}__attribute__ ((packed));

struct PartnerTable
{
	uint64_t  ID; //1
	uint64_t  Grade; //2
	uint32_t n_Skill; //3
	uint64_t *Skill; //3
	uint32_t n_AttributeType; //4
	uint64_t *AttributeType; //4
	uint32_t n_GradeCoefficient; //5
	double *GradeCoefficient; //5
	uint32_t n_TypeCoefficient; //6
	double *TypeCoefficient; //6
	uint32_t n_UpperLimitBase; //7
	double *UpperLimitBase; //7
	uint32_t n_LowerLimitBase; //8
	double *LowerLimitBase; //8
	uint32_t n_DetailedType; //9
	uint64_t *DetailedType; //9
	uint32_t n_DetailedNum; //10
	double *DetailedNum; //10
	uint32_t n_Recruit; //11
	uint64_t *Recruit; //11
	uint32_t n_SeveranceItem; //12
	uint64_t *SeveranceItem; //12
	uint32_t n_SeveranceNum; //13
	uint64_t *SeveranceNum; //13
	uint32_t n_PartnerAttributeType; //14
	uint64_t *PartnerAttributeType; //14
	uint32_t n_PartnerAttributeID; //15
	uint64_t *PartnerAttributeID; //15
	uint32_t n_WashMarrowItem; //16
	uint64_t *WashMarrowItem; //16
	uint32_t n_GodYao; //17
	uint64_t *GodYao; //17
	uint64_t  ThreeCurrencyItem; //18
	uint64_t  ThreeCurrencyItemNum; //19
	uint64_t  SevenCurrencyItem; //20
	uint64_t  SevenCurrencyItemNum; //21
	uint64_t  ThreeExclusiveItem; //22
	uint64_t  ThreeExclusiveItemNum; //23
	uint64_t  SevenExclusiveItem; //24
	uint64_t  SevenExclusiveItemNum; //25
	uint32_t n_QualificationsItem; //26
	uint64_t *QualificationsItem; //26
	uint64_t  Character; //27
	uint32_t n_SkillProbability; //28
	uint64_t *SkillProbability; //28
	uint32_t n_GrowthValue; //29
	uint64_t *GrowthValue; //29
	uint32_t n_GodYaoAttribute; //30
	uint64_t *GodYaoAttribute; //30
	uint32_t n_Fetter; //31
	uint64_t *Fetter; //31
	uint32_t n_FetterReward; //32
	uint64_t *FetterReward; //32
	uint32_t n_PartnerAttribute; //33
	uint64_t *PartnerAttribute; //33
	uint32_t n_PartnerAttributeNum; //34
	uint64_t *PartnerAttributeNum; //34
	uint32_t n_Database1; //35
	uint64_t *Database1; //35
	uint32_t n_Database2; //36
	uint64_t *Database2; //36
	uint32_t n_Database3; //37
	uint64_t *Database3; //37
	uint32_t n_Database4; //38
	uint64_t *Database4; //38
	uint32_t n_Database5; //39
	uint64_t *Database5; //39
	uint64_t  SkillLocking; //40
	uint64_t  LockingItem; //41
	uint64_t  LockingItemNum; //42
	uint32_t n_RecruitReward; //43
	uint64_t *RecruitReward; //43
	uint64_t  PokedexItem; //44
	uint64_t  PokedexItemNum; //45
	uint32_t n_FetterRewardNum; //46
	uint64_t *FetterRewardNum; //46
}__attribute__ ((packed));

struct PassiveSkillTable
{
	uint64_t  ID; //1
	uint64_t  TriggerType; //2
	uint64_t  NeedNum; //3
}__attribute__ ((packed));

struct PowerMasterTable
{
	uint64_t  ID; //1
	uint64_t  PowerCondition; //2
	uint64_t  RewardLimit; //3
	uint32_t n_Reward; //4
	uint64_t *Reward; //4
	uint32_t n_RewardNum; //5
	uint64_t *RewardNum; //5
	uint64_t  Batch; //6
}__attribute__ ((packed));

struct PulseTable
{
	uint64_t  ID; //1
	uint64_t  Break; //2
	uint64_t  BreakCondition; //3
	uint64_t  AcupunctureType; //4
}__attribute__ ((packed));

struct QuestionTable
{
	uint64_t  ID; //1
	uint64_t  Type; //2
	uint64_t  RightAnseer; //3
	uint64_t  Coin; //4
	uint64_t  Exp; //5
	char  *GangsAnseer; //6
}__attribute__ ((packed));

struct RaidScriptTable
{
	uint64_t  ID; //1
	uint64_t  TypeID; //2
	uint32_t n_Parameter1; //3
	double *Parameter1; //3
	uint32_t n_Parameter2; //4
	char **Parameter2; //4
}__attribute__ ((packed));

struct RandomBox
{
	uint64_t  ID; //1
	uint64_t  ItemID; //2
	uint64_t  Num; //3
	uint32_t n_ItemID0; //4
	uint64_t *ItemID0; //4
	uint32_t n_Num0; //5
	uint64_t *Num0; //5
	uint32_t n_DisplayNum0; //6
	uint64_t *DisplayNum0; //6
	uint32_t n_Probability0; //7
	uint64_t *Probability0; //7
}__attribute__ ((packed));

struct RandomCardRewardTable
{
	uint64_t  CardNum; //2
	uint64_t  RewardID; //3
	uint64_t  RewardNum; //4
}__attribute__ ((packed));

struct RandomCardTable
{
	uint64_t  CardID; //1
	uint64_t  CardDengeon; //2
	uint32_t n_Condition; //3
	uint64_t *Condition; //3
	uint32_t n_Parameter1; //4
	uint64_t *Parameter1; //4
	uint32_t n_Parameter2; //5
	uint64_t *Parameter2; //5
	uint64_t  Probability; //6
}__attribute__ ((packed));

struct RandomCollectionTable
{
	uint64_t  ID; //1
	uint64_t  RandomType; //2
	uint64_t  CollectionID; //3
	uint64_t  MapID; //4
	uint32_t n_PointX; //5
	double *PointX; //5
	uint32_t n_PointZ; //6
	double *PointZ; //6
}__attribute__ ((packed));

struct RandomDungeonTable
{
	uint64_t  ID; //1
	uint64_t  TypeLevel; //2
	uint64_t  ResID; //3
	uint64_t  ResProbability; //6
	uint32_t n_PointX; //7
	uint64_t *PointX; //7
	uint32_t n_PointZ; //8
	uint64_t *PointZ; //8
	uint32_t n_FaceY; //9
	uint64_t *FaceY; //9
	uint32_t n_GroupProbability; //10
	uint64_t *GroupProbability; //10
}__attribute__ ((packed));

struct RandomMonsterTable
{
	uint64_t  ID; //1
	uint64_t  TypeLevel; //2
	uint64_t  MonsterID; //3
	uint64_t  Difficulty; //4
	uint64_t  Times; //5
	uint64_t  ActivityReward; //6
	uint64_t  BasicsReward; //7
	uint64_t  MaxLevel; //8
	uint64_t  ExpReward; //9
	uint64_t  MoneyReward; //10
}__attribute__ ((packed));

struct RankingRewardTable
{
	uint64_t  ID; //1
	uint64_t  RankStart; //2
	uint64_t  RankEnd; //3
	uint32_t n_Reward; //4
	uint64_t *Reward; //4
	uint32_t n_RewardNum; //5
	uint64_t *RewardNum; //5
	uint32_t n_RankingTableId; //6
	uint64_t *RankingTableId; //6
	uint32_t n_RankingTableIdName; //7
	char **RankingTableIdName; //7
}__attribute__ ((packed));

struct RecruitTable
{
	uint64_t  ID; //1
	uint64_t  RecruitNum; //2
	uint32_t n_GetItem; //3
	uint64_t *GetItem; //3
	uint32_t n_ConsumeType; //4
	uint64_t *ConsumeType; //4
	uint64_t  ConsumeNum; //5
	uint32_t n_Recruit; //6
	uint64_t *Recruit; //6
	uint32_t n_RecruitGrade; //7
	uint64_t *RecruitGrade; //7
	uint32_t n_RecruitProbability; //8
	uint64_t *RecruitProbability; //8
	uint64_t  BottomTime; //9
	uint32_t n_BottomGrade; //10
	uint64_t *BottomGrade; //10
	uint32_t n_RecruitBottom; //11
	uint64_t *RecruitBottom; //11
	uint64_t  Time; //12
	uint32_t n_TypeProbability; //13
	uint64_t *TypeProbability; //13
	uint32_t n_Item; //14
	uint64_t *Item; //14
	uint32_t n_ItemNum; //15
	uint64_t *ItemNum; //15
	uint32_t n_First; //16
	uint64_t *First; //16
	uint64_t  Discount; //17
	uint64_t  ConsumeNum1; //18
	uint64_t  FreeRecruit; //19
}__attribute__ ((packed));

struct RewardBack
{
	uint64_t  ID; //1
	uint64_t  Type; //2
	uint32_t n_Value; //3
	uint64_t *Value; //3
	uint64_t  Level; //4
	uint64_t  ExpReward; //5
	uint64_t  MoneyReward; //6
	uint64_t  Normal; //7
	uint64_t  NormalExpend; //8
	uint64_t  Perfect; //9
	uint64_t  PerfectExpend; //10
}__attribute__ ((packed));

struct RewardTable
{
	uint64_t  ID; //1
	uint32_t n_RewardType; //2
	uint64_t *RewardType; //2
	uint32_t n_RewardValue; //3
	uint64_t *RewardValue; //3
}__attribute__ ((packed));

struct RobotPatrolTable
{
	uint64_t  ID; //1
	uint32_t n_patrol1; //2
	uint64_t *patrol1; //2
	uint32_t n_patrol2; //3
	uint64_t *patrol2; //3
	uint32_t n_patrol3; //4
	uint64_t *patrol3; //4
	uint32_t n_patrol4; //5
	uint64_t *patrol4; //5
	uint32_t n_patrol5; //6
	uint64_t *patrol5; //6
	uint32_t n_patrol6; //7
	uint64_t *patrol6; //7
	uint32_t n_patrol7; //8
	uint64_t *patrol7; //8
	uint32_t n_patrol8; //9
	uint64_t *patrol8; //9
	uint32_t n_patrol9; //10
	uint64_t *patrol9; //10
	uint32_t n_patrol10; //11
	uint64_t *patrol10; //11
	uint32_t n_patrol11; //12
	uint64_t *patrol11; //12
	uint32_t n_patrol12; //13
	uint64_t *patrol12; //13
	uint32_t n_patrol; //14
	struct sproto_config_pos **patrol; //14
}__attribute__ ((packed));

struct SceneCreateMonsterTable
{
	uint64_t  uid; //1
	uint64_t  ID; //2
	double  PointPosX; //3
	double  PointPosZ; //4
	uint64_t  Level; //5
	uint32_t n_TargetInfoList; //6
	struct TargetInfoEntry **TargetInfoList; //6
	double  PointPosY; //7
	double  Yaw; //8
}__attribute__ ((packed));

struct SceneResTable
{
	uint64_t  SceneID; //1
	char  *ResName; //2
	char  *SceneName; //3
	uint64_t  Level; //5
	double  BirthPointX; //7
	double  BirthPointY; //8
	double  BirthPointZ; //9
	double  FaceY; //10
	uint32_t n_RelivePointX; //11
	double *RelivePointX; //11
	uint32_t n_RelivePointZ; //12
	double *RelivePointZ; //12
	uint32_t n_ReliveRange; //13
	uint64_t *ReliveRange; //13
	uint32_t n_ReliveFaceY; //14
	double *ReliveFaceY; //14
	char  *RefreshPoint; //15
	char  *PlaceNature; //16
	uint64_t  UseMounts; //17
	uint32_t n_UseDelivery; //18
	uint64_t *UseDelivery; //18
	uint64_t  Recovery; //19
	uint64_t  Partner; //20
}__attribute__ ((packed));

struct ScriptTable
{
	uint64_t  ID; //1
	char  *Path; //2
}__attribute__ ((packed));

struct SearchTable
{
	uint64_t  ID; //1
	uint64_t  ItemId; //2
	uint32_t n_TreasureId; //3
	uint64_t *TreasureId; //3
	uint32_t n_Event; //4
	uint64_t *Event; //4
	uint32_t n_Probability; //5
	uint64_t *Probability; //5
	uint32_t n_Parameter1; //6
	uint64_t *Parameter1; //6
	uint32_t n_Parameter2; //7
	uint64_t *Parameter2; //7
	uint32_t n_Parameter3; //8
	uint64_t *Parameter3; //8
}__attribute__ ((packed));

struct ServerLevelTable
{
	uint64_t  ID; //1
	uint64_t  Level; //2
	uint64_t  Time; //3
	uint64_t  Dungeon; //4
	uint64_t  DungeonSchedule; //5
	uint64_t  LevelPlusEXP1; //6
	uint64_t  LevelPlusEXP2; //7
	uint64_t  Title; //8
	uint64_t  Truer; //9
}__attribute__ ((packed));

struct ServerResTable
{
	uint64_t  ID; //1
	uint64_t  OpenTime; //2
}__attribute__ ((packed));

struct ShopListTable
{
	uint64_t  ID; //1
	uint64_t  ShopType; //2
	uint64_t  StartTime; //3
	uint64_t  EndTime; //4
}__attribute__ ((packed));

struct ShopTable
{
	uint64_t  ID; //1
	uint64_t  ItemID; //2
	uint64_t  ShopType; //3
	uint64_t  Acc; //4
	uint64_t  ConsumptionType; //5
	uint64_t  Purchasetype; //6
	uint64_t  Condition; //7
	uint64_t  Price; //8
	uint64_t  Discount; //9
	uint64_t  BuyNum; //10
	uint64_t  Reset; //11
	uint64_t  RestrictionTime; //12
}__attribute__ ((packed));

struct SignDay
{
	uint64_t  ID; //1
	uint64_t  Month; //2
	uint64_t  Days; //3
	uint64_t  ItemID; //4
	uint64_t  ItemValue; //5
	uint64_t  VipDouble; //6
}__attribute__ ((packed));

struct SignMonth
{
	uint64_t  ID; //1
	uint64_t  Month; //2
	uint64_t  Days; //3
	uint64_t  ItemID; //4
	uint64_t  ItemValue; //5
}__attribute__ ((packed));

struct SkillEffectTable
{
	uint64_t  ID; //1
	uint64_t  Type; //2
	uint32_t n_Effect; //3
	uint64_t *Effect; //3
	uint32_t n_EffectAdd; //4
	uint64_t *EffectAdd; //4
	uint32_t n_EffectNum; //5
	uint64_t *EffectNum; //5
}__attribute__ ((packed));

struct SkillLevelTable
{
	uint64_t  LowLevel; //1
	uint64_t  LowNeedExp; //2
	uint64_t  PartnerLevel; //3
	uint64_t  HighLevel; //4
	uint64_t  HighNeedExp; //5
	uint64_t  PartnerLevel1; //6
}__attribute__ ((packed));

struct SkillLvTable
{
	uint64_t  ID; //1
	uint64_t  MonsterID; //7
	uint64_t  MonsterLv; //8
	uint64_t  CostCoin; //9
	uint64_t  NeedFight; //10
	uint64_t  MonsterEff; //11
	uint64_t  CostItem; //12
	uint64_t  CostNum; //13
	uint64_t  NeedLv; //14
	uint64_t  EffectIdPartner; //15
}__attribute__ ((packed));

struct SkillMoveTable
{
	uint64_t  ID; //1
	uint64_t  MoveType; //2
	uint64_t  DmgType; //3
	uint64_t  MoveDistance; //4
	uint64_t  EndType; //5
	double  EndDistance; //6
}__attribute__ ((packed));

struct SkillTable
{
	uint64_t  ID; //1
	uint64_t  levelMax; //2
	uint64_t  SkillType; //3
	uint64_t  SkillAffectId; //4
	uint64_t  PassiveID; //5
	uint32_t n_TargetType; //6
	uint64_t *TargetType; //6
	uint64_t  SearchRadius; //7
	uint64_t  MaxCount; //8
	uint64_t  RangeType; //9
	double  Radius; //10
	double  Angle; //11
	uint64_t  IsMonster; //12
	double  SkillRange; //13
	uint64_t  HateAdd; //14
	uint64_t  HateValue; //15
	uint64_t  SkillLv; //16
	uint64_t  Career; //18
	uint64_t  OpenLv; //19
	uint64_t  SkillRune; //20
	uint32_t n_RuneID; //21
	uint64_t *RuneID; //21
	uint64_t  SkillAcc; //22
	uint64_t  IsRune; //23
	uint64_t  pre_skill; //24
	uint64_t  CD; //25
	uint32_t n_TimeID; //26
	uint64_t *TimeID; //26
	uint32_t n_time_config; //27
	struct SkillTimeTable **time_config; //27
	uint64_t  TotalSkillDelay; //28
}__attribute__ ((packed));

struct SkillTimeTable
{
	uint64_t  ID; //1
	uint64_t  ActionTime; //2
	uint64_t  Frequency; //3
	uint64_t  Interval; //4
	uint32_t n_EffectIdEnemy; //5
	uint64_t *EffectIdEnemy; //5
	uint32_t n_EffectIdFriend; //6
	uint64_t *EffectIdFriend; //6
	uint32_t n_BuffIdEnemy; //7
	uint64_t *BuffIdEnemy; //7
	uint32_t n_BuffIdFriend; //8
	uint64_t *BuffIdFriend; //8
	uint64_t  CallTime; //9
	uint64_t  CallId; //10
}__attribute__ ((packed));

struct SpecialTitleTable
{
	uint64_t  ID; //1
	uint64_t  TitleEffect1; //2
	uint64_t  TitleEffect2; //3
	uint64_t  TitleEffect3; //4
	uint64_t  TitleEffect4; //5
}__attribute__ ((packed));

struct SpecialtyLevelTable
{
	uint64_t  ID; //1
	uint64_t  LevelExp; //2
	uint64_t  SpecialTitle; //3
}__attribute__ ((packed));

struct SpecialtySkillTable
{
	uint64_t  ID; //1
	uint64_t  SpecialtyType; //2
	uint64_t  SpecialtyLevel; //3
	uint64_t  SkillEffect; //5
	uint32_t n_EffectValue; //6
	uint64_t *EffectValue; //6
}__attribute__ ((packed));

struct SpiritTable
{
	uint64_t  ID; //1
	uint64_t  Level; //2
	uint32_t n_SpiritAttribute; //3
	uint64_t *SpiritAttribute; //3
	uint32_t n_AttributeCeiling; //4
	uint64_t *AttributeCeiling; //4
	uint64_t  SpiritExpend; //5
	uint64_t  ExpendNum; //6
	uint32_t n_GradeNum; //7
	uint64_t *GradeNum; //7
	uint32_t n_OrderAttribute; //8
	uint64_t *OrderAttribute; //8
}__attribute__ ((packed));

struct StageTable
{
	uint64_t  StageID; //1
	uint64_t  Stage; //2
	uint64_t  StageLevel; //3
	uint64_t  StageScore; //4
	uint64_t  StageValue; //5
	uint64_t  BasicsCoinValue; //6
	uint64_t  StageReward; //7
	uint32_t n_VictoryTime; //8
	uint64_t *VictoryTime; //8
	uint32_t n_VictoryReward3; //9
	uint64_t *VictoryReward3; //9
	uint32_t n_VictoryReward5; //10
	uint64_t *VictoryReward5; //10
	uint64_t  stageTotalScore; //11
	uint64_t  stageLastScore; //12
}__attribute__ ((packed));

struct SyntheticTable
{
	uint64_t  ID; //1
	uint64_t  SyntheticTarget; //2
	uint32_t n_SyntheticMaterial; //3
	uint64_t *SyntheticMaterial; //3
	uint32_t n_SyntheticMaterialNum; //4
	uint64_t *SyntheticMaterialNum; //4
	uint64_t  Consume; //5
}__attribute__ ((packed));

struct TargetInfoEntry
{
	struct TargetPos  *TargetPos; //1
	uint64_t  RemainTime; //2
}__attribute__ ((packed));

struct TargetPos
{
	double  TargetPosX; //1
	double  TargetPosZ; //2
}__attribute__ ((packed));

struct TaskChapterTable
{
	uint64_t  ChapterID; //2
	uint32_t n_RewardID; //3
	uint64_t *RewardID; //3
}__attribute__ ((packed));

struct TaskConditionTable
{
	uint64_t  ID; //1
	uint64_t  ConditionType; //2
	uint64_t  ConditionTarget; //3
	uint64_t  ConditionNum; //4
	uint64_t  Scene; //5
	double  PointX; //6
	double  PointZ; //7
	uint64_t  Radius; //8
}__attribute__ ((packed));

struct TaskDropTable
{
	uint64_t  ID; //1
	uint64_t  MonsterID; //2
	uint64_t  SceneID; //3
	uint64_t  DropItem; //4
	uint64_t  DropPro; //5
}__attribute__ ((packed));

struct TaskEventTable
{
	uint64_t  ID; //1
	uint64_t  EventClass; //2
	uint64_t  EventType; //3
	uint64_t  EventTarget; //4
	uint64_t  EventNum; //5
}__attribute__ ((packed));

struct TaskMonsterTable
{
	uint64_t  ID; //1
	uint64_t  MonsterID; //2
	uint64_t  MonsterLevel; //3
	double  PointX; //4
	double  PointZ; //5
	double  PointY; //6
	double  Orientation; //7
}__attribute__ ((packed));

struct TaskRewardTable
{
	uint64_t  ID; //1
	uint64_t  RewardEXP; //2
	uint64_t  RewardMoney; //3
	uint32_t n_RewardType; //4
	uint64_t *RewardType; //4
	uint32_t n_RewardTarget; //5
	uint64_t *RewardTarget; //5
	uint32_t n_RewardNum; //6
	uint64_t *RewardNum; //6
}__attribute__ ((packed));

struct TaskTable
{
	uint64_t  ID; //1
	uint64_t  ChapterId; //2
	uint64_t  TaskType; //3
	uint64_t  Level; //4
	uint64_t  TaskTime; //5
	uint64_t  TimeRule; //6
	uint64_t  FollowTask; //7
	uint32_t n_AccessConID; //8
	uint64_t *AccessConID; //8
	uint32_t n_EndConID; //9
	uint64_t *EndConID; //9
	uint32_t n_EventID; //10
	uint64_t *EventID; //10
	uint32_t n_DropID; //11
	uint64_t *DropID; //11
	uint32_t n_RewardID; //12
	uint64_t *RewardID; //12
	uint64_t  Team; //13
	uint32_t n_PostTask; //14
	uint64_t *PostTask; //14
	uint64_t  StartNPC; //15
}__attribute__ ((packed));

struct TimeReward
{
	uint64_t  ID; //1
	uint64_t  Position; //2
	uint64_t  ItemID; //3
	uint64_t  ItemValue; //4
	uint64_t  Probability; //5
}__attribute__ ((packed));

struct TitleFunctionTable
{
	uint64_t  ID; //1
	uint64_t  FunctionID; //2
	uint64_t  ConditionType; //3
	uint64_t  ConditionTarget1; //4
	uint64_t  ConditionTarget2; //5
	uint64_t  ConditionNum; //6
	uint32_t n_Attribute; //7
	uint64_t *Attribute; //7
	uint32_t n_Value1; //8
	double *Value1; //8
	uint64_t  Continued; //9
	uint64_t  Value2; //10
	uint64_t  NoticeID; //11
}__attribute__ ((packed));

struct Top10GangsTable
{
	uint64_t  ID; //1
	uint64_t  RewardStartID; //2
	uint64_t  RewardEndID; //3
	uint32_t n_Reward1; //4
	uint64_t *Reward1; //4
	uint32_t n_RewardNum1; //5
	uint64_t *RewardNum1; //5
	uint32_t n_Reward2; //6
	uint64_t *Reward2; //6
	uint32_t n_RewardNum2; //7
	uint64_t *RewardNum2; //7
	uint64_t  Batch; //8
}__attribute__ ((packed));

struct TradingTable
{
	uint64_t  ID; //1
	uint64_t  ItemID; //2
	uint64_t  GuidePrice; //3
}__attribute__ ((packed));

struct TransferPointTable
{
	uint64_t  ID; //1
	uint32_t n_MapId; //2
	uint64_t *MapId; //2
}__attribute__ ((packed));

struct TravelTable
{
	uint64_t  ID; //1
	uint32_t n_LevelSection; //2
	uint64_t *LevelSection; //2
	uint32_t n_QuestGroup; //3
	uint64_t *QuestGroup; //3
	uint64_t  RewardGroup; //4
	uint64_t  TimeRewardGroup; //5
}__attribute__ ((packed));

struct TreasureTable
{
	uint64_t  ID; //1
	uint64_t  MapId; //2
	uint64_t  PointX; //3
	uint64_t  PointY; //4
	uint64_t  PointZ; //5
	uint64_t  TransferPoint; //6
}__attribute__ ((packed));

struct TypeLevelTable
{
	uint64_t  ID; //1
	uint64_t  Level; //2
	uint64_t  RewardTime; //3
	uint64_t  MinActor; //4
	uint64_t  MaxActor; //5
	uint32_t n_OpenDay; //6
	uint64_t *OpenDay; //6
	uint32_t n_OpenTime; //7
	uint64_t *OpenTime; //7
	uint32_t n_CloseTime; //8
	uint64_t *CloseTime; //8
	uint64_t  OpenProbability; //9
	uint64_t  SpecialtyPlus; //10
	uint64_t  ShowTimes; //11
}__attribute__ ((packed));

struct UndergroundTask
{
	uint64_t  ID; //1
	uint32_t n_LevelSection; //2
	uint64_t *LevelSection; //2
	uint32_t n_TaskID; //3
	uint64_t *TaskID; //3
	uint32_t n_StarProbability; //4
	uint64_t *StarProbability; //4
	uint64_t  DropID; //5
	uint32_t n_ExpReward; //6
	uint64_t *ExpReward; //6
	uint32_t n_MoneyReward; //7
	uint64_t *MoneyReward; //7
	uint32_t n_RewardGroup; //8
	uint64_t *RewardGroup; //8
}__attribute__ ((packed));

struct WeaponsEffectTable
{
	uint64_t  ID; //0
	uint64_t  WeaponsID; //1
	uint64_t  Requirement1; //2
	uint64_t  Requirement2; //3
	uint64_t  Item; //4
	uint64_t  ItemNum; //5
}__attribute__ ((packed));

struct WeekTable
{
	uint64_t  ID; //1
	uint64_t  Type; //2
	uint64_t  Num; //3
	uint64_t  Reward; //4
	uint32_t n_MonsterID; //5
	uint64_t *MonsterID; //5
}__attribute__ ((packed));

struct WorldBossRewardTable
{
	uint64_t  ID; //1
	uint32_t n_ItemID; //2
	uint64_t *ItemID; //2
	uint32_t n_Num; //3
	uint64_t *Num; //3
	uint64_t  Draw; //4
	uint32_t n_Random; //5
	uint64_t *Random; //5
	uint64_t  MailID; //6
}__attribute__ ((packed));

struct WorldBossTable
{
	uint64_t  ID; //1
	char  *Name; //2
	uint64_t  Level; //3
	uint64_t  Type; //4
	uint64_t  SceneID; //5
	uint64_t  MonsterID; //6
	uint32_t n_Time; //7
	uint64_t *Time; //7
	double  Coefficient; //8
	uint64_t  RewardLevel; //9
}__attribute__ ((packed));

struct raidsrv_config
{
	uint64_t  ID; //1
	uint32_t n_raid_id; //2
	uint64_t *raid_id; //2
}__attribute__ ((packed));

struct sproto_config_pos
{
	uint64_t  pos_x; //1
	uint64_t  pos_z; //2
}__attribute__ ((packed));

#endif
