#include "excel_data.h"

void free_AchievementFunctionTable(struct AchievementFunctionTable *p)
{
    if (!p) return;
    free(p->Hierarchys);
    free(p);
};

void free_AchievementHierarchyTable(struct AchievementHierarchyTable *p)
{
    if (!p) return;
    free(p);
};

void free_ActiveSkillTable(struct ActiveSkillTable *p)
{
    if (!p) return;
    for(size_t i = 0; i < p->n_SkillAction; ++i)
    {
        free(p->SkillAction[i]);
    }
    free(p->SkillAction);
    p->n_SkillAction = 0;
    free(p->SkillLength);
    free(p->SkillEffect);
    free(p);
};

void free_ActiveTable(struct ActiveTable *p)
{
    if (!p) return;
    free(p->Reward);
    free(p->RewardNum);
    free(p);
};

void free_ActorAttributeTable(struct ActorAttributeTable *p)
{
    if (!p) return;
    free(p);
};

void free_ActorFashionTable(struct ActorFashionTable *p)
{
    if (!p) return;
    free(p->ColourID1);
    free(p->ColourID2);
    free(p->Time);
    free(p->WingBinding);
    free(p);
};

void free_ActorHeadTable(struct ActorHeadTable *p)
{
    if (!p) return;
    free(p->Vocation);
    free(p);
};

void free_ActorLevelTable(struct ActorLevelTable *p)
{
    if (!p) return;
    free(p);
};

void free_ActorRobotTable(struct ActorRobotTable *p)
{
    if (!p) return;
    free(p->ResId);
    free(p->HairResId);
    free(p->WeaponId);
    free(p->AttributeType);
    free(p->AttributePro);
    free(p->Skill);
    free(p->PassiveSkill);
    free(p->FightPro);
    free(p);
};

void free_ActorTable(struct ActorTable *p)
{
    if (!p) return;
    free(p->ResId);
    free(p->HairResId);
    free(p->Skill);
    free(p->PassiveSkill);
    free(p->SkillLevelID);
    free(p);
};

void free_AcupunctureTable(struct AcupunctureTable *p)
{
    if (!p) return;
    free(p->AcupunctureAttribute);
    free(p->AttributeCeiling);
    free(p);
};

void free_ArenaRewardTable(struct ArenaRewardTable *p)
{
    if (!p) return;
    free(p->ItemID);
    free(p->Num);
    free(p);
};

void free_AttributeTypeTable(struct AttributeTypeTable *p)
{
    if (!p) return;
    free(p);
};

void free_AuctionTable(struct AuctionTable *p)
{
    if (!p) return;
    free(p->ItemName);
    free(p);
};

void free_BaguaStarTable(struct BaguaStarTable *p)
{
    if (!p) return;
    free(p->StarItem);
    free(p->StarNum);
    free(p->DecomposeCompensation);
    free(p->DecomposeCompensationNum);
    free(p);
};

void free_BaguaSuitTable(struct BaguaSuitTable *p)
{
    if (!p) return;
    free(p->SuitNum);
    free(p->SuitAttributeType);
    free(p->Classification);
    free(p->AttributeValue);
    free(p->SuitPlus1);
    free(p->SuitPlus2);
    free(p->SuitPlus3);
    free(p->SuitPlus4);
    free(p->SuitPlus5);
    free(p->SuitPlus6);
    free(p);
};

void free_BaguaTable(struct BaguaTable *p)
{
    if (!p) return;
    free(p->ViceAttributeDatabaseSelection1);
    free(p->ViceAttributeDatabaseSelection2);
    free(p->ViceAttributeEntry);
    free(p->DecomposeItem);
    free(p->DecomposeNum);
    free(p->RecastItem);
    free(p->RecastNum);
    free(p->ClearItem1);
    free(p->ClearNum1);
    free(p->ClearItem2);
    free(p->ClearNum2);
    free(p->AdditionalAttributeDatabaseSelection);
    free(p->AdditionalAttributeEntry);
    free(p);
};

void free_BaguaViceAttributeTable(struct BaguaViceAttributeTable *p)
{
    if (!p) return;
    free(p->Rand);
    free(p);
};

void free_BaseAITable(struct BaseAITable *p)
{
    if (!p) return;
    free(p);
};

void free_BattleFieldRank(struct BattleFieldRank *p)
{
    if (!p) return;
    free(p->Reward1);
    free(p->Num1);
    free(p->Reward2);
    free(p->Num2);
    free(p->Reward3);
    free(p->Num3);
    free(p->Reward4);
    free(p->Num4);
    free(p->Reward5);
    free(p->Num5);
    free(p);
};

void free_BattlefieldTable(struct BattlefieldTable *p)
{
    if (!p) return;
    free(p->BirthPoint1);
    free(p->BirthPoint2);
    free(p->WarSet);
    free(p->MineSet);
    free(p->ForestSet);
    free(p->FirstReward);
    free(p->Num);
    free(p->BottomReward);
    free(p->BottomRewardNum);
    free(p);
};

void free_BiaocheRewardTable(struct BiaocheRewardTable *p)
{
    if (!p) return;
    free(p->StartTime);
    free(p->EndTime);
    free(p->RewardItem1);
    free(p->RewardNum1);
    free(p->RewardItem2);
    free(p->RewardNum2);
    free(p);
};

void free_BiaocheTable(struct BiaocheTable *p)
{
    if (!p) return;
    free(p->shanfeiId);
    free(p->level);
    free(p->Number);
    free(p->Point);
    free(p);
};

void free_BootNameTable(struct BootNameTable *p)
{
    if (!p) return;
    free(p->Name);
    free(p);
};

void free_BreakTable(struct BreakTable *p)
{
    if (!p) return;
    free(p->PulseAttribute);
    free(p->AttributeLower);
    free(p->AttributeUpper);
    free(p->AttributeColor);
    free(p->MeridiansProbability);
    free(p->BloodProbability);
    free(p->VitalProbability);
    free(p->MarrowProbability);
    free(p->MeridiansMinimum);
    free(p->BloodMinimum);
    free(p->VitalMinimum);
    free(p->MarrowMinimum);
    free(p);
};

void free_BuffTable(struct BuffTable *p)
{
    if (!p) return;
    free(p->EffectID);
    free(p->DelEffectID);
    free(p->DeleteType);
    free(p);
};

void free_CampDefenseTable(struct CampDefenseTable *p)
{
    if (!p) return;
    free(p->TruckRouteX);
    free(p->TruckRouteY);
    free(p->MonsterID);
    free(p->MineralIntegral1);
    free(p->MineralIntegral2);
    free(p->SupportMine);
    free(p->TaskID);
    free(p->ProtectMonsterID);
    free(p->ProtectMonsterNum);
    free(p);
};

void free_CampTable(struct CampTable *p)
{
    if (!p) return;
    free(p);
};

void free_CastSpiritTable(struct CastSpiritTable *p)
{
    if (!p) return;
    free(p->CastExpend);
    free(p->Expend1Num);
    free(p->MountsAttribute);
    free(p->AttributeCeiling);
    free(p);
};

void free_ChallengeTable(struct ChallengeTable *p)
{
    if (!p) return;
    free(p);
};

void free_ChangeSpecialty(struct ChangeSpecialty *p)
{
    if (!p) return;
    free(p);
};

void free_CharmTable(struct CharmTable *p)
{
    if (!p) return;
    free(p->AttributeType);
    free(p->AttributeNum);
    free(p);
};

void free_ChivalrousTable(struct ChivalrousTable *p)
{
    if (!p) return;
    free(p);
};

void free_CiFuTable(struct CiFuTable *p)
{
    if (!p) return;
    free(p);
};

void free_CollectTable(struct CollectTable *p)
{
    if (!p) return;
    free(p->TaskId);
    free(p->DropID);
    free(p->NameId);
    free(p->Parameter1);
    free(p->Parameter2);
    free(p->TaskIdShow);
    free(p->Drop1);
    free(p->Drop2);
    free(p);
};

void free_ColourTable(struct ColourTable *p)
{
    if (!p) return;
    free(p);
};

void free_ControlTable(struct ControlTable *p)
{
    if (!p) return;
    free(p->OpenDay);
    free(p->OpenTime);
    free(p->CloseTime);
    free(p);
};

void free_DegreeTable(struct DegreeTable *p)
{
    if (!p) return;
    free(p->Function);
    free(p);
};

void free_DonationTable(struct DonationTable *p)
{
    if (!p) return;
    free(p->RewardType);
    free(p->RewardValue);
    free(p);
};

void free_DropConfigTable(struct DropConfigTable *p)
{
    if (!p) return;
    free(p->DropID);
    free(p->Probability);
    free(p->NumMin);
    free(p->NumMax);
    free(p);
};

void free_DungeonTable(struct DungeonTable *p)
{
    if (!p) return;
    free(p->Score);
    free(p->ScoreValue);
    free(p->ScoreValue1);
    free(p->Rewards);
    free(p->PassType);
    free(p->PassValue);
    free(p->PassValue1);
    free(p->RandomID);
    free(p->wanyaoka);
    free(p->RewardPosition);
    free(p->ReliveTime);
    free(p->DungeonPass);
    free(p->FailType);
    free(p->FailValue);
    free(p->FailValue1);
    free(p->ItemRewardSection);
    free(p);
};

void free_EquipAttribute(struct EquipAttribute *p)
{
    if (!p) return;
    free(p->Rand);
    free(p->QualityWeight);
    free(p);
};

void free_EquipLock(struct EquipLock *p)
{
    if (!p) return;
    free(p->LockLv);
    free(p->LockQuality);
    free(p->LockStar);
    free(p->LockItem);
    free(p->LockItemNum);
    free(p->MosaicType);
    free(p->EnchantQualityLock);
    free(p->EnchantStarLock);
    free(p->EnchantItem);
    free(p->EnchantItemNum);
    free(p->EnchantCoin);
    free(p);
};

void free_EquipStarLv(struct EquipStarLv *p)
{
    if (!p) return;
    free(p->DatabaseSelection);
    free(p);
};

void free_EquipmentTable(struct EquipmentTable *p)
{
    if (!p) return;
    free(p);
};

void free_EscortTask(struct EscortTask *p)
{
    if (!p) return;
    free(p->MonsterID);
    free(p->MonsterID1);
    free(p->PointXZ1);
    free(p->MonsterID2);
    free(p->PointXZ2);
    free(p->MonsterID3);
    free(p->PointXZ3);
    free(p->MonsterID4);
    free(p->PointXZ4);
    free(p->talkID);
    free(p);
};

void free_EventCalendarTable(struct EventCalendarTable *p)
{
    if (!p) return;
    free(p->AuxiliaryValue);
    free(p);
};

void free_FactionActivity(struct FactionActivity *p)
{
    if (!p) return;
    free(p->DungeonPass1);
    free(p->DungeonPass2);
    free(p->FailType);
    free(p->FailValue);
    free(p->FailValue1);
    free(p);
};

void free_FactionBattleTable(struct FactionBattleTable *p)
{
    if (!p) return;
    free(p->BirthPoint1);
    free(p->BirthPoint2);
    free(p->BoxReloadX);
    free(p->BoxReloadY);
    free(p->BoxReloadZ);
    free(p);
};

void free_FetterTable(struct FetterTable *p)
{
    if (!p) return;
    free(p->Partner);
    free(p);
};

void free_FishingTable(struct FishingTable *p)
{
    if (!p) return;
    free(p);
};

void free_FlySkillTable(struct FlySkillTable *p)
{
    if (!p) return;
    free(p);
};

void free_FunctionUnlockTable(struct FunctionUnlockTable *p)
{
    if (!p) return;
    free(p);
};

void free_GangsBuildTaskTable(struct GangsBuildTaskTable *p)
{
    if (!p) return;
    free(p->Level);
    free(p->Tasklibrary);
    free(p);
};

void free_GangsDungeonTable(struct GangsDungeonTable *p)
{
    if (!p) return;
    free(p->RewardID);
    free(p->RewardNum);
    free(p);
};

void free_GangsJurisdictionTable(struct GangsJurisdictionTable *p)
{
    if (!p) return;
    free(p->Name);
    free(p);
};

void free_GangsSkillTable(struct GangsSkillTable *p)
{
    if (!p) return;
    free(p->skillName);
    free(p);
};

void free_GangsTable(struct GangsTable *p)
{
    if (!p) return;
    free(p->parameter4);
    free(p->BuildingName);
    free(p);
};

void free_GemAttribute(struct GemAttribute *p)
{
    if (!p) return;
    free(p);
};

void free_GenerateMonster(struct GenerateMonster *p)
{
    if (!p) return;
    free(p->MovePointXZ);
    free(p);
};

void free_GiftTable(struct GiftTable *p)
{
    if (!p) return;
    free(p);
};

void free_GodYaoAttributeTable(struct GodYaoAttributeTable *p)
{
    if (!p) return;
    free(p);
};

void free_GradeTable(struct GradeTable *p)
{
    if (!p) return;
    free(p->AttributeType);
    free(p->AttributeTypeValue);
    free(p->DayReward);
    free(p->DayRewardNum);
    free(p->BreachReward);
    free(p->BreachRewardNum);
    free(p);
};

void free_GrowupTable(struct GrowupTable *p)
{
    if (!p) return;
    free(p->RewardType);
    free(p->RewardValue);
    free(p->LevelRange);
    free(p);
};

void free_ItemsConfigTable(struct ItemsConfigTable *p)
{
    if (!p) return;
    free(p->Name);
    free(p->ParameterEffect);
    free(p);
};

void free_LevelReward(struct LevelReward *p)
{
    if (!p) return;
    free(p->ItemID);
    free(p->ItemValue);
    free(p);
};

void free_LifeItemTable(struct LifeItemTable *p)
{
    if (!p) return;
    free(p);
};

void free_LifeMagicTable(struct LifeMagicTable *p)
{
    if (!p) return;
    free(p->LifeMagic);
    free(p->LifeProbability1);
    free(p->Magic1);
    free(p->LifeProbability2);
    free(p->Magic2);
    free(p->LifeProbability3);
    free(p->Magic3);
    free(p->LifeProbability4);
    free(p->Magic4);
    free(p);
};

void free_LifeProbabilitytable(struct LifeProbabilitytable *p)
{
    if (!p) return;
    free(p->LifeProbability);
    free(p->Magic);
    free(p);
};

void free_LifeSkillTable(struct LifeSkillTable *p)
{
    if (!p) return;
    free(p->LvPro);
    free(p->ItemID);
    free(p);
};

void free_LimitActivityControlTable(struct LimitActivityControlTable *p)
{
    if (!p) return;
    free(p);
};

void free_LoginGifts(struct LoginGifts *p)
{
    if (!p) return;
    free(p->lRewards);
    free(p->Quantity);
    free(p);
};

void free_MGLYdiaoxiangTable(struct MGLYdiaoxiangTable *p)
{
    if (!p) return;
    free(p->MovePointX);
    free(p->MovePointZ);
    free(p->StopTime);
    free(p->Effects);
    free(p->EffectsParameter);
    free(p);
};

void free_MGLYmaoguiTable(struct MGLYmaoguiTable *p)
{
    if (!p) return;
    free(p->SeparateMonster);
    free(p->Effects);
    free(p->EffectsParameter);
    free(p);
};

void free_MGLYmaoguiwangTable(struct MGLYmaoguiwangTable *p)
{
    if (!p) return;
    free(p->Monster1);
    free(p->Monster2);
    free(p->Effects);
    free(p->EffectsParameter);
    free(p);
};

void free_MGLYshoulingTable(struct MGLYshoulingTable *p)
{
    if (!p) return;
    free(p->MonsterID);
    free(p);
};

void free_MGLYyanseTable(struct MGLYyanseTable *p)
{
    if (!p) return;
    free(p);
};

void free_MagicAttributeTable(struct MagicAttributeTable *p)
{
    if (!p) return;
    free(p->Rand);
    free(p);
};

void free_MagicTable(struct MagicTable *p)
{
    if (!p) return;
    free(p->MainAttributeNum);
    free(p->ViceAttribute22);
    free(p->ViceAttribute);
    free(p);
};

void free_MoneyQuestTable(struct MoneyQuestTable *p)
{
    if (!p) return;
    free(p->LevelSection);
    free(p->QuestGroup);
    free(p->QualityGroup);
    free(p->ExpReward);
    free(p->MoneyReward);
    free(p);
};

void free_MonsterIDTable(struct MonsterIDTable *p)
{
    if (!p) return;
    free(p->MonseterID);
    free(p);
};

void free_MonsterPkTypeTable(struct MonsterPkTypeTable *p)
{
    if (!p) return;
    free(p);
};

void free_MonsterTable(struct MonsterTable *p)
{
    if (!p) return;
    free(p->Skill);
    free(p->PassiveSkill);
    free_NpcTalkTable(p->talk_config);
    p->talk_config = NULL;
    free(p);
};

void free_MountsTable(struct MountsTable *p)
{
    if (!p) return;
    free(p->Time);
    free(p->WingBinding);
    free(p->Speed);
    free(p->Name);
    free(p->Item);
    free(p->ItemNum);
    free(p->Binding);
    free(p);
};

void free_NineEightTable(struct NineEightTable *p)
{
    if (!p) return;
    free(p->Reward);
    free(p->RewardNum);
    free(p);
};

void free_NoticeTable(struct NoticeTable *p)
{
    if (!p) return;
    free(p->NoticeTxt);
    free(p->NoticeChannel);
    free(p);
};

void free_NpcTalkTable(struct NpcTalkTable *p)
{
    if (!p) return;
    free(p->EventNum2);
    free_NpcTalkTable(p->next);
    p->next = NULL;
    free(p);
};

void free_OnlineTimes(struct OnlineTimes *p)
{
    if (!p) return;
    free(p);
};

void free_P20076Table(struct P20076Table *p)
{
    if (!p) return;
    free(p->MonsterID);
    free(p->MonsterNum);
    free(p->Reward1);
    free(p->RewardNum1);
    free(p->Reward2);
    free(p->RewardNum2);
    free(p->MonsterLevel);
    free(p);
};

void free_ParameterTable(struct ParameterTable *p)
{
    if (!p) return;
    free(p->parameter1);
    free(p->parameter2);
    free(p);
};

void free_PartnerLevelTable(struct PartnerLevelTable *p)
{
    if (!p) return;
    free(p);
};

void free_PartnerSkillTable(struct PartnerSkillTable *p)
{
    if (!p) return;
    free(p->SkillProbability);
    free(p->Skill);
    free(p->SkillProbability1);
    free(p);
};

void free_PartnerTable(struct PartnerTable *p)
{
    if (!p) return;
    free(p->Skill);
    free(p->AttributeType);
    free(p->GradeCoefficient);
    free(p->TypeCoefficient);
    free(p->UpperLimitBase);
    free(p->LowerLimitBase);
    free(p->DetailedType);
    free(p->DetailedNum);
    free(p->Recruit);
    free(p->SeveranceItem);
    free(p->SeveranceNum);
    free(p->PartnerAttributeType);
    free(p->PartnerAttributeID);
    free(p->WashMarrowItem);
    free(p->GodYao);
    free(p->QualificationsItem);
    free(p->SkillProbability);
    free(p->GrowthValue);
    free(p->GodYaoAttribute);
    free(p->Fetter);
    free(p->FetterReward);
    free(p->PartnerAttribute);
    free(p->PartnerAttributeNum);
    free(p->Database1);
    free(p->Database2);
    free(p->Database3);
    free(p->Database4);
    free(p->Database5);
    free(p->RecruitReward);
    free(p->FetterRewardNum);
    free(p);
};

void free_PassiveSkillTable(struct PassiveSkillTable *p)
{
    if (!p) return;
    free(p);
};

void free_PowerMasterTable(struct PowerMasterTable *p)
{
    if (!p) return;
    free(p->Reward);
    free(p->RewardNum);
    free(p);
};

void free_PulseTable(struct PulseTable *p)
{
    if (!p) return;
    free(p);
};

void free_QuestionTable(struct QuestionTable *p)
{
    if (!p) return;
    free(p->GangsAnseer);
    free(p);
};

void free_RaidScriptTable(struct RaidScriptTable *p)
{
    if (!p) return;
    free(p->Parameter1);
    for(size_t i = 0; i < p->n_Parameter2; ++i)
    {
        free(p->Parameter2[i]);
    }
    free(p->Parameter2);
    p->n_Parameter2 = 0;
    free(p);
};

void free_RandomBox(struct RandomBox *p)
{
    if (!p) return;
    free(p->ItemID0);
    free(p->Num0);
    free(p->DisplayNum0);
    free(p->Probability0);
    free(p);
};

void free_RandomCardRewardTable(struct RandomCardRewardTable *p)
{
    if (!p) return;
    free(p);
};

void free_RandomCardTable(struct RandomCardTable *p)
{
    if (!p) return;
    free(p->Condition);
    free(p->Parameter1);
    free(p->Parameter2);
    free(p);
};

void free_RandomCollectionTable(struct RandomCollectionTable *p)
{
    if (!p) return;
    free(p->PointX);
    free(p->PointZ);
    free(p);
};

void free_RandomDungeonTable(struct RandomDungeonTable *p)
{
    if (!p) return;
    free(p->PointX);
    free(p->PointZ);
    free(p->FaceY);
    free(p->GroupProbability);
    free(p);
};

void free_RandomMonsterTable(struct RandomMonsterTable *p)
{
    if (!p) return;
    free(p);
};

void free_RankingRewardTable(struct RankingRewardTable *p)
{
    if (!p) return;
    free(p->Reward);
    free(p->RewardNum);
    free(p->RankingTableId);
    for(size_t i = 0; i < p->n_RankingTableIdName; ++i)
    {
        free(p->RankingTableIdName[i]);
    }
    free(p->RankingTableIdName);
    p->n_RankingTableIdName = 0;
    free(p);
};

void free_RecruitTable(struct RecruitTable *p)
{
    if (!p) return;
    free(p->GetItem);
    free(p->ConsumeType);
    free(p->Recruit);
    free(p->RecruitGrade);
    free(p->RecruitProbability);
    free(p->BottomGrade);
    free(p->RecruitBottom);
    free(p->TypeProbability);
    free(p->Item);
    free(p->ItemNum);
    free(p->First);
    free(p);
};

void free_RewardBack(struct RewardBack *p)
{
    if (!p) return;
    free(p->Value);
    free(p);
};

void free_RewardTable(struct RewardTable *p)
{
    if (!p) return;
    free(p->RewardType);
    free(p->RewardValue);
    free(p);
};

void free_RobotPatrolTable(struct RobotPatrolTable *p)
{
    if (!p) return;
    free(p->patrol1);
    free(p->patrol2);
    free(p->patrol3);
    free(p->patrol4);
    free(p->patrol5);
    free(p->patrol6);
    free(p->patrol7);
    free(p->patrol8);
    free(p->patrol9);
    free(p->patrol10);
    free(p->patrol11);
    free(p->patrol12);
    for(size_t i = 0; i < p->n_patrol; ++i)
    {
        free_sproto_config_pos(p->patrol[i]);
    }
    free(p->patrol);
    p->n_patrol = 0;
    free(p);
};

void free_SceneCreateMonsterTable(struct SceneCreateMonsterTable *p)
{
    if (!p) return;
    for(size_t i = 0; i < p->n_TargetInfoList; ++i)
    {
        free_TargetInfoEntry(p->TargetInfoList[i]);
    }
    free(p->TargetInfoList);
    p->n_TargetInfoList = 0;
    free(p);
};

void free_SceneResTable(struct SceneResTable *p)
{
    if (!p) return;
    free(p->ResName);
    free(p->SceneName);
    free(p->RelivePointX);
    free(p->RelivePointZ);
    free(p->ReliveRange);
    free(p->ReliveFaceY);
    free(p->RefreshPoint);
    free(p->PlaceNature);
    free(p->UseDelivery);
    free(p);
};

void free_ScriptTable(struct ScriptTable *p)
{
    if (!p) return;
    free(p->Path);
    free(p);
};

void free_SearchTable(struct SearchTable *p)
{
    if (!p) return;
    free(p->TreasureId);
    free(p->Event);
    free(p->Probability);
    free(p->Parameter1);
    free(p->Parameter2);
    free(p->Parameter3);
    free(p);
};

void free_ServerLevelTable(struct ServerLevelTable *p)
{
    if (!p) return;
    free(p);
};

void free_ServerResTable(struct ServerResTable *p)
{
    if (!p) return;
    free(p);
};

void free_ShopListTable(struct ShopListTable *p)
{
    if (!p) return;
    free(p);
};

void free_ShopTable(struct ShopTable *p)
{
    if (!p) return;
    free(p);
};

void free_SignDay(struct SignDay *p)
{
    if (!p) return;
    free(p);
};

void free_SignMonth(struct SignMonth *p)
{
    if (!p) return;
    free(p);
};

void free_SkillEffectTable(struct SkillEffectTable *p)
{
    if (!p) return;
    free(p->Effect);
    free(p->EffectAdd);
    free(p->EffectNum);
    free(p->EffectAdd1);
    free(p->EffectNum1);
    free(p);
};

void free_SkillLevelTable(struct SkillLevelTable *p)
{
    if (!p) return;
    free(p);
};

void free_SkillLvTable(struct SkillLvTable *p)
{
    if (!p) return;
    free(p);
};

void free_SkillMoveTable(struct SkillMoveTable *p)
{
    if (!p) return;
    free(p);
};

void free_SkillTable(struct SkillTable *p)
{
    if (!p) return;
    free(p->TargetType);
    free(p->RuneID);
    free(p->TimeID);
    for(size_t i = 0; i < p->n_time_config; ++i)
    {
        free_SkillTimeTable(p->time_config[i]);
    }
    free(p->time_config);
    p->n_time_config = 0;
    free(p);
};

void free_SkillTimeTable(struct SkillTimeTable *p)
{
    if (!p) return;
    free(p->EffectIdEnemy);
    free(p->EffectIdFriend);
    free(p->BuffIdEnemy);
    free(p->BuffIdFriend);
    free(p);
};

void free_SpecialTitleTable(struct SpecialTitleTable *p)
{
    if (!p) return;
    free(p);
};

void free_SpecialtyLevelTable(struct SpecialtyLevelTable *p)
{
    if (!p) return;
    free(p);
};

void free_SpecialtySkillTable(struct SpecialtySkillTable *p)
{
    if (!p) return;
    free(p->EffectValue);
    free(p);
};

void free_SpiritTable(struct SpiritTable *p)
{
    if (!p) return;
    free(p->SpiritAttribute);
    free(p->AttributeCeiling);
    free(p->GradeNum);
    free(p->OrderAttribute);
    free(p);
};

void free_StageTable(struct StageTable *p)
{
    if (!p) return;
    free(p->VictoryTime);
    free(p->VictoryReward3);
    free(p->VictoryReward5);
    free(p);
};

void free_SyntheticTable(struct SyntheticTable *p)
{
    if (!p) return;
    free(p->SyntheticMaterial);
    free(p->SyntheticMaterialNum);
    free(p);
};

void free_TargetInfoEntry(struct TargetInfoEntry *p)
{
    if (!p) return;
    free_TargetPos(p->TargetPos);
    p->TargetPos = NULL;
    free(p);
};

void free_TargetPos(struct TargetPos *p)
{
    if (!p) return;
    free(p);
};

void free_TaskChapterTable(struct TaskChapterTable *p)
{
    if (!p) return;
    free(p->RewardID);
    free(p);
};

void free_TaskConditionTable(struct TaskConditionTable *p)
{
    if (!p) return;
    free(p);
};

void free_TaskDropTable(struct TaskDropTable *p)
{
    if (!p) return;
    free(p);
};

void free_TaskEventTable(struct TaskEventTable *p)
{
    if (!p) return;
    free(p);
};

void free_TaskMonsterTable(struct TaskMonsterTable *p)
{
    if (!p) return;
    free(p);
};

void free_TaskRewardTable(struct TaskRewardTable *p)
{
    if (!p) return;
    free(p->RewardType);
    free(p->RewardTarget);
    free(p->RewardNum);
    free(p);
};

void free_TaskTable(struct TaskTable *p)
{
    if (!p) return;
    free(p->AccessConID);
    free(p->EndConID);
    free(p->EventID);
    free(p->DropID);
    free(p->RewardID);
    free(p->PostTask);
    free(p);
};

void free_TimeReward(struct TimeReward *p)
{
    if (!p) return;
    free(p);
};

void free_TitleFunctionTable(struct TitleFunctionTable *p)
{
    if (!p) return;
    free(p->Attribute);
    free(p->Value1);
    free(p);
};

void free_Top10GangsTable(struct Top10GangsTable *p)
{
    if (!p) return;
    free(p->Reward1);
    free(p->RewardNum1);
    free(p->Reward2);
    free(p->RewardNum2);
    free(p);
};

void free_TradingTable(struct TradingTable *p)
{
    if (!p) return;
    free(p);
};

void free_TransferPointTable(struct TransferPointTable *p)
{
    if (!p) return;
    free(p->MapId);
    free(p);
};

void free_TravelTable(struct TravelTable *p)
{
    if (!p) return;
    free(p->LevelSection);
    free(p->QuestGroup);
    free(p);
};

void free_TreasureTable(struct TreasureTable *p)
{
    if (!p) return;
    free(p);
};

void free_TypeLevelTable(struct TypeLevelTable *p)
{
    if (!p) return;
    free(p->OpenDay);
    free(p->OpenTime);
    free(p->CloseTime);
    free(p);
};

void free_UndergroundTask(struct UndergroundTask *p)
{
    if (!p) return;
    free(p->LevelSection);
    free(p->TaskID);
    free(p->StarProbability);
    free(p->ExpReward);
    free(p->MoneyReward);
    free(p->RewardGroup);
    free(p);
};

void free_WeaponsEffectTable(struct WeaponsEffectTable *p)
{
    if (!p) return;
    free(p);
};

void free_WeekTable(struct WeekTable *p)
{
    if (!p) return;
    free(p->MonsterID);
    free(p);
};

void free_WorldBossRewardTable(struct WorldBossRewardTable *p)
{
    if (!p) return;
    free(p->ItemID);
    free(p->Num);
    free(p->Random);
    free(p);
};

void free_WorldBossTable(struct WorldBossTable *p)
{
    if (!p) return;
    free(p->Name);
    free(p->Time);
    free(p);
};

void free_raidsrv_config(struct raidsrv_config *p)
{
    if (!p) return;
    free(p->raid_id);
    free(p);
};

void free_sproto_config_pos(struct sproto_config_pos *p)
{
    if (!p) return;
    free(p);
};

