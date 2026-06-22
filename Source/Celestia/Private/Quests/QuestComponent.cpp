#include "Quests/QuestComponent.h"
#include "Components/ProgressionComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"

UQuestComponent::UQuestComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UQuestComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		for (UQuestDataAsset* StartQuest : StartingQuests)
		{
			if (StartQuest && CanAcceptQuest(StartQuest))
			{
				Internal_AcceptQuest(StartQuest);
			}
		}
	}

	if (AActor* OwningActor = GetOwner())
	{
		if (UProgressionComponent* ProgComp = OwningActor->FindComponentByClass<UProgressionComponent>())
		{
			ProgComp->OnLevelUp.AddDynamic(this, &UQuestComponent::OnPlayerLevelUp);
		}
	}
}

void UQuestComponent::ClientInitializeQuests()
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		Server_RequestQuestData();
	}
}


bool UQuestComponent::Server_RequestQuestData_Validate() { return true; }
void UQuestComponent::Server_RequestQuestData_Implementation()
{
	TArray<UQuestDataAsset*> Assets;
	TArray<int32> Progresses;
	TArray<bool> Statuses;

	for (const FActiveQuest& Q : ActiveQuests)
	{
		Assets.Add(Q.QuestData);
		Statuses.Add(Q.bIsReadyToTurnIn);
		for (const FQuestObjective& Obj : Q.CurrentObjectives)
		{
			Progresses.Add(Obj.CurrentAmount);
		}
	}

	Client_ReceiveQuestData(Assets, Progresses, Statuses, CompletedQuestIDs);
}

void UQuestComponent::Client_ReceiveQuestData_Implementation(const TArray<UQuestDataAsset*>& ServerActiveQuests, const TArray<int32>& ServerObjectiveProgress, const TArray<bool>& ServerQuestStatus, const TArray<FName>& ServerCompleted)
{
	ActiveQuests.Empty();
	int32 ProgressIndex = 0;

	for (int32 i = 0; i < ServerActiveQuests.Num(); ++i)
	{
		UQuestDataAsset* Asset = ServerActiveQuests[i];
		if (!Asset) continue;

		FActiveQuest NewQuest(Asset);
		NewQuest.CurrentObjectives = Asset->Objectives;
		NewQuest.bIsReadyToTurnIn = ServerQuestStatus[i];

		for (int32 j = 0; j < NewQuest.CurrentObjectives.Num(); ++j)
		{
			if (ServerObjectiveProgress.IsValidIndex(ProgressIndex))
			{
				NewQuest.CurrentObjectives[j].CurrentAmount = ServerObjectiveProgress[ProgressIndex];
				ProgressIndex++;
			}
		}
		ActiveQuests.Add(NewQuest);
	}

	CompletedQuestIDs = ServerCompleted;
	OnQuestListUpdated.Broadcast();

	if (ActiveQuests.Num() > 0 && !TrackedQuestData)
	{
		TrackQuest(ActiveQuests.Last().QuestData);
		OnQuestAccepted.Broadcast(ActiveQuests.Last());
	}
	else if (TrackedQuestData)
	{
		for (const FActiveQuest& Quest : ActiveQuests)
		{
			if (Quest.QuestData == TrackedQuestData)
			{
				OnQuestTracked.Broadcast(Quest);
				break;
			}
		}
	}
}


bool UQuestComponent::CanAcceptQuest(UQuestDataAsset* QuestToCheck) const
{
	if (!QuestToCheck) return false;

	if (!QuestToCheck->bIsRepeatable && CompletedQuestIDs.Contains(QuestToCheck->QuestID))
	{
		return false;
	}

	for (const FActiveQuest& Quest : ActiveQuests)
	{
		if (Quest.QuestData == QuestToCheck) return false;
	}

	if (AActor* OwningActor = GetOwner())
	{
		if (UProgressionComponent* ProgComp = OwningActor->FindComponentByClass<UProgressionComponent>())
		{
			if (ProgComp->CurrentLevel < QuestToCheck->RequiredLevel) return false;
		}
	}

	if (QuestToCheck->PrerequisiteQuest)
	{
		if (!CompletedQuestIDs.Contains(QuestToCheck->PrerequisiteQuest->QuestID))
		{
			return false;
		}
	}

	return true;
}

void UQuestComponent::OnPlayerLevelUp(int32 NewLevel)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	for (UQuestDataAsset* Quest : AutoUnlockByLevelQuests)
	{
		if (Quest && NewLevel >= Quest->RequiredLevel)
		{
			if (CanAcceptQuest(Quest))
			{
				AcceptQuest(Quest);
			}
		}
	}
}


void UQuestComponent::AcceptQuest(UQuestDataAsset* NewQuest)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		Server_AcceptQuest(NewQuest);
		return;
	}

	Internal_AcceptQuest(NewQuest);

	if (NewQuest && NewQuest->bIsGlobalCoop)
	{
		for (TActorIterator<ACharacter> It(GetWorld()); It; ++It)
		{
			if (*It != GetOwner())
			{
				if (UQuestComponent* QC = It->FindComponentByClass<UQuestComponent>())
				{
					if (QC->CanAcceptQuest(NewQuest))
					{
						QC->Internal_AcceptQuest(NewQuest);
					}
				}
			}
		}
	}
}

bool UQuestComponent::Server_AcceptQuest_Validate(UQuestDataAsset* NewQuest) { return true; }
void UQuestComponent::Server_AcceptQuest_Implementation(UQuestDataAsset* NewQuest)
{
	AcceptQuest(NewQuest);
}

void UQuestComponent::Internal_AcceptQuest(UQuestDataAsset* NewQuest)
{
	if (!CanAcceptQuest(NewQuest)) return;

	FActiveQuest NewActiveQuest(NewQuest);
	NewActiveQuest.CurrentObjectives = NewQuest->Objectives;

	for (FQuestObjective& Obj : NewActiveQuest.CurrentObjectives)
	{
		Obj.CurrentAmount = 0;
	}

	ActiveQuests.Add(NewActiveQuest);

	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (OwnerPawn->IsLocallyControlled())
		{
			TrackQuest(NewQuest);
			OnQuestAccepted.Broadcast(ActiveQuests.Last());
			OnQuestListUpdated.Broadcast();
		}
		else
		{
			Client_SyncQuestAccepted(NewQuest);
		}
	}
}

void UQuestComponent::Client_SyncQuestAccepted_Implementation(UQuestDataAsset* NewQuest)
{
	if (!NewQuest) return;

	FActiveQuest NewActiveQuest(NewQuest);
	NewActiveQuest.CurrentObjectives = NewQuest->Objectives;
	for (FQuestObjective& Obj : NewActiveQuest.CurrentObjectives)
	{
		Obj.CurrentAmount = 0;
	}

	ActiveQuests.Add(NewActiveQuest);
	TrackQuest(NewQuest);
	OnQuestAccepted.Broadcast(ActiveQuests.Last());
	OnQuestListUpdated.Broadcast();
}


void UQuestComponent::UpdateObjective(EObjectiveType Type, FName TargetID, TSubclassOf<AItemBase> TargetItemClass, int32 Amount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		Server_UpdateObjective(Type, TargetID, TargetItemClass, Amount);
		return;
	}

	TArray<UQuestDataAsset*> UpdatedGlobalQuests = Internal_UpdateObjective(Type, TargetID, TargetItemClass, Amount);

	for (UQuestDataAsset* GlobalQuest : UpdatedGlobalQuests)
	{
		for (TActorIterator<ACharacter> It(GetWorld()); It; ++It)
		{
			if (*It != GetOwner())
			{
				if (UQuestComponent* QC = It->FindComponentByClass<UQuestComponent>())
				{
					QC->Internal_ForceSyncGlobalObjective(GlobalQuest, Type, TargetID, TargetItemClass, Amount);
				}
			}
		}
	}
}

bool UQuestComponent::Server_UpdateObjective_Validate(EObjectiveType Type, FName TargetID, TSubclassOf<class AItemBase> TargetItemClass, int32 Amount) { return true; }
void UQuestComponent::Server_UpdateObjective_Implementation(EObjectiveType Type, FName TargetID, TSubclassOf<class AItemBase> TargetItemClass, int32 Amount)
{
	UpdateObjective(Type, TargetID, TargetItemClass, Amount);
}

TArray<UQuestDataAsset*> UQuestComponent::Internal_UpdateObjective(EObjectiveType Type, FName TargetID, TSubclassOf<class AItemBase> TargetItemClass, int32 Amount)
{
	TArray<UQuestDataAsset*> GlobalQuestsUpdated;
	TArray<UQuestDataAsset*> QuestsToAutoTurnIn;

	for (int32 i = 0; i < ActiveQuests.Num(); ++i)
	{
		FActiveQuest& ActiveQuest = ActiveQuests[i];
		if (ActiveQuest.bIsReadyToTurnIn) continue;

		bool bThisQuestWasUpdated = false;

		for (int32 j = 0; j < ActiveQuest.CurrentObjectives.Num(); ++j)
		{
			FQuestObjective& Objective = ActiveQuest.CurrentObjectives[j];

			if (Objective.ObjectiveType == Type && Objective.CurrentAmount < Objective.RequiredAmount)
			{
				bool bIsMatch = false;

				if (Type == EObjectiveType::Collect)
				{
					if (Objective.TargetItemClass == TargetItemClass) bIsMatch = true;
				}
				else if (Type == EObjectiveType::Kill || Type == EObjectiveType::Location || Type == EObjectiveType::Talk || Type == EObjectiveType::Dungeon)
				{
					if (Objective.TargetID == TargetID) bIsMatch = true;
				}

				if (bIsMatch)
				{
					Objective.CurrentAmount = FMath::Clamp(Objective.CurrentAmount + Amount, 0, Objective.RequiredAmount);
					bThisQuestWasUpdated = true;
				}
			}
		}

		if (bThisQuestWasUpdated)
		{
			CheckDungeonCompletion();
			CheckQuestCompletion(i);

			if (ActiveQuest.QuestData->bIsGlobalCoop)
			{
				GlobalQuestsUpdated.Add(ActiveQuest.QuestData);
			}

			if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
			{
				if (OwnerPawn->IsLocallyControlled())
				{
					if (TrackedQuestData == ActiveQuest.QuestData)
					{
						OnObjectiveUpdated.Broadcast(ActiveQuest);
					}
					OnQuestListUpdated.Broadcast();
				}
				else
				{
					TArray<int32> Progresses;
					for (const FQuestObjective& Obj : ActiveQuest.CurrentObjectives)
					{
						Progresses.Add(Obj.CurrentAmount);
					}
					Client_SyncObjectiveUpdated(ActiveQuest.QuestData, Progresses, ActiveQuest.bIsReadyToTurnIn);
				}
			}

			if (ActiveQuest.bIsReadyToTurnIn && ActiveQuest.QuestData->bAutoTurnIn)
			{
				QuestsToAutoTurnIn.Add(ActiveQuest.QuestData);
			}
		}
	}

	for (UQuestDataAsset* AutoQuest : QuestsToAutoTurnIn)
	{
		Internal_TurnInQuest(AutoQuest);
	}

	return GlobalQuestsUpdated;
}

void UQuestComponent::Internal_ForceSyncGlobalObjective(UQuestDataAsset* GlobalQuest, EObjectiveType Type, FName TargetID, TSubclassOf<class AItemBase> TargetItemClass, int32 Amount)
{
	for (int32 i = 0; i < ActiveQuests.Num(); ++i)
	{
		if (ActiveQuests[i].QuestData == GlobalQuest && !ActiveQuests[i].bIsReadyToTurnIn)
		{
			Internal_UpdateObjective(Type, TargetID, TargetItemClass, Amount);
			break;
		}
	}
}

void UQuestComponent::Client_SyncObjectiveUpdated_Implementation(UQuestDataAsset* QuestData, const TArray<int32>& NewProgress, bool bIsReadyToTurnIn)
{
	for (FActiveQuest& Quest : ActiveQuests)
	{
		if (Quest.QuestData == QuestData)
		{
			Quest.bIsReadyToTurnIn = bIsReadyToTurnIn;

			for (int32 i = 0; i < Quest.CurrentObjectives.Num() && i < NewProgress.Num(); ++i)
			{
				Quest.CurrentObjectives[i].CurrentAmount = NewProgress[i];
			}

			if (TrackedQuestData == QuestData)
			{
				OnObjectiveUpdated.Broadcast(Quest);
			}
			OnQuestListUpdated.Broadcast();
			break;
		}
	}
}


void UQuestComponent::TurnInQuest(UQuestDataAsset* QuestToTurnIn)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		Server_TurnInQuest(QuestToTurnIn);
		return;
	}

	Internal_TurnInQuest(QuestToTurnIn);

	if (QuestToTurnIn && QuestToTurnIn->bIsGlobalCoop)
	{
		for (TActorIterator<ACharacter> It(GetWorld()); It; ++It)
		{
			if (*It != GetOwner())
			{
				if (UQuestComponent* QC = It->FindComponentByClass<UQuestComponent>())
				{
					QC->Internal_TurnInQuest(QuestToTurnIn);
				}
			}
		}
	}
}

bool UQuestComponent::Server_TurnInQuest_Validate(UQuestDataAsset* QuestToTurnIn) { return true; }
void UQuestComponent::Server_TurnInQuest_Implementation(UQuestDataAsset* QuestToTurnIn)
{
	TurnInQuest(QuestToTurnIn);
}

void UQuestComponent::Internal_TurnInQuest(UQuestDataAsset* QuestToTurnIn)
{
	if (!QuestToTurnIn) return;

	for (int32 i = 0; i < ActiveQuests.Num(); ++i)
	{
		if (ActiveQuests[i].QuestData == QuestToTurnIn)
		{
			if (ActiveQuests[i].bIsReadyToTurnIn)
			{
				if (AActor* OwningActor = GetOwner())
				{
					if (UProgressionComponent* ProgComp = OwningActor->FindComponentByClass<UProgressionComponent>())
					{
						ProgComp->AddXP(QuestToTurnIn->Rewards.Experience);

						if (QuestToTurnIn->Rewards.BonusLevelsToGrant > 0)
						{
							ProgComp->ForceLevelUp(QuestToTurnIn->Rewards.BonusLevelsToGrant);
						}
					}
				}

				CompletedQuestIDs.Add(QuestToTurnIn->QuestID);
				UQuestDataAsset* FollowUpQuest = QuestToTurnIn->NextQuest;

				if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
				{
					if (OwnerPawn->IsLocallyControlled())
					{
						if (TrackedQuestData == QuestToTurnIn)
						{
							TrackedQuestData = nullptr;
							OnQuestUntracked.Broadcast();
						}
						OnQuestListUpdated.Broadcast();
						OnQuestRewardsGranted.Broadcast(QuestToTurnIn->Rewards.Coins, QuestToTurnIn->Rewards.RewardItems);
					}
					else
					{
						Client_SyncQuestTurnedIn(QuestToTurnIn);
						Client_NotifyQuestRewards(QuestToTurnIn->Rewards.Coins, QuestToTurnIn->Rewards.RewardItems);
					}
				}

				ActiveQuests.RemoveAt(i);

				if (FollowUpQuest)
				{
					Internal_AcceptQuest(FollowUpQuest);
				}
			}
			return;
		}
	}
}

void UQuestComponent::Client_SyncQuestTurnedIn_Implementation(UQuestDataAsset* QuestTurnedIn)
{
	CompletedQuestIDs.Add(QuestTurnedIn->QuestID);

	for (int32 i = 0; i < ActiveQuests.Num(); ++i)
	{
		if (ActiveQuests[i].QuestData == QuestTurnedIn)
		{
			ActiveQuests.RemoveAt(i);

			if (TrackedQuestData == QuestTurnedIn)
			{
				TrackedQuestData = nullptr;
				OnQuestUntracked.Broadcast();
			}

			OnQuestListUpdated.Broadcast();
			break;
		}
	}
}

void UQuestComponent::Client_NotifyQuestRewards_Implementation(int32 Coins, const TArray<FItemReward>& Items)
{
	OnQuestRewardsGranted.Broadcast(Coins, Items);
}


void UQuestComponent::TrackQuest(UQuestDataAsset* QuestToTrack)
{
	TrackedQuestData = QuestToTrack;

	for (const FActiveQuest& Quest : ActiveQuests)
	{
		if (Quest.QuestData == QuestToTrack)
		{
			OnQuestTracked.Broadcast(Quest);
			break;
		}
	}
}

void UQuestComponent::CheckDungeonCompletion()
{
	for (int32 i = 0; i < ActiveQuests.Num(); ++i)
	{
		FActiveQuest& Quest = ActiveQuests[i];
		if (Quest.bIsReadyToTurnIn) continue;

		bool bAllOtherTasksComplete = true;
		bool bHasDungeonObjective = false;
		int32 DungeonObjIndex = -1;

		for (int32 j = 0; j < Quest.CurrentObjectives.Num(); ++j)
		{
			if (Quest.CurrentObjectives[j].ObjectiveType == EObjectiveType::Dungeon)
			{
				bHasDungeonObjective = true;
				DungeonObjIndex = j;
			}
			else
			{
				if (Quest.CurrentObjectives[j].CurrentAmount < Quest.CurrentObjectives[j].RequiredAmount)
				{
					bAllOtherTasksComplete = false;
				}
			}
		}

		if (bHasDungeonObjective && bAllOtherTasksComplete)
		{
			FQuestObjective& DungeonObj = Quest.CurrentObjectives[DungeonObjIndex];
			if (DungeonObj.CurrentAmount < DungeonObj.RequiredAmount)
			{
				DungeonObj.CurrentAmount = DungeonObj.RequiredAmount;
			}
		}
	}
}

void UQuestComponent::CheckQuestCompletion(int32 QuestIndex)
{
	if (!ActiveQuests.IsValidIndex(QuestIndex)) return;

	FActiveQuest& ActiveQuest = ActiveQuests[QuestIndex];
	bool bAllComplete = true;

	for (const FQuestObjective& Objective : ActiveQuest.CurrentObjectives)
	{
		if (Objective.CurrentAmount < Objective.RequiredAmount)
		{
			bAllComplete = false;
			break;
		}
	}

	if (bAllComplete)
	{
		ActiveQuest.bIsReadyToTurnIn = true;
	}
}

bool UQuestComponent::GetActiveQuestData(UQuestDataAsset* QuestToCheck, FActiveQuest& OutActiveQuest) const
{
	if (!QuestToCheck) return false;

	for (const FActiveQuest& Quest : ActiveQuests)
	{
		if (Quest.QuestData == QuestToCheck)
		{
			OutActiveQuest = Quest;
			return true;
		}
	}
	return false;
}

bool UQuestComponent::IsQuestCompleted(FName QuestIDToCheck) const
{
	return CompletedQuestIDs.Contains(QuestIDToCheck);
}