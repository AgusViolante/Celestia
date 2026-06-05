// Fill out your copyright notice in the Description page of Project Settings.


#include "Quests/QuestComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/ProgressionComponent.h"

UQuestComponent::UQuestComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UQuestComponent::BeginPlay()
{
	Super::BeginPlay();
	
	for (UQuestDataAsset* StartQuest : StartingQuests)
	{
		if (StartQuest && CanAcceptQuest(StartQuest))
		{
			AcceptQuest(StartQuest);
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

bool UQuestComponent::CanAcceptQuest(UQuestDataAsset* QuestToCheck) const
{
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

void UQuestComponent::AcceptQuest(UQuestDataAsset* NewQuest)
{
	if (!CanAcceptQuest(NewQuest)) return;

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
	bool bAnyQuestUpdated = false;
	TArray<UQuestDataAsset*> QuestsToAutoTurnIn;

	for (int32 i = 0; i < ActiveQuests.Num(); ++i)
	{
		FActiveQuest& ActiveQuest = ActiveQuests[i];
		if (ActiveQuest.bIsReadyToTurnIn) continue;

		bool bThisQuestWasUpdated = false;

		for (FQuestObjective& Objective : ActiveQuest.CurrentObjectives)
		{
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
					bAnyQuestUpdated = true;
				}
			}
		}

		if (bThisQuestWasUpdated)
		{
			
			CheckDungeonCompletion();

			CheckQuestCompletion(i);

			
			if (TrackedQuestData == ActiveQuest.QuestData)
			{
				OnObjectiveUpdated.Broadcast(ActiveQuest);
			}

			if (ActiveQuests[i].bIsReadyToTurnIn && ActiveQuests[i].QuestData->bAutoTurnIn)
			{
				QuestsToAutoTurnIn.Add(ActiveQuests[i].QuestData);
			}
		}
	}
	for (UQuestDataAsset* AutoQuest : QuestsToAutoTurnIn)
	{
		TurnInQuest(AutoQuest);
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

				
				if (TrackedQuestData == Quest.QuestData)
				{
					OnObjectiveUpdated.Broadcast(Quest);
				}
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

void UQuestComponent::TurnInQuest(UQuestDataAsset* QuestToTurnIn)
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

					OnQuestRewardsGranted.Broadcast(QuestToTurnIn->Rewards.Coins, QuestToTurnIn->Rewards.RewardItems);
					OnQuestItemsConsumed.Broadcast(ActiveQuests[i].CurrentObjectives);
				}

				CompletedQuestIDs.Add(QuestToTurnIn->QuestID);

				if (TrackedQuestData == QuestToTurnIn)
				{
					TrackedQuestData = nullptr;
					OnQuestUntracked.Broadcast();
				}

				UQuestDataAsset* FollowUpQuest = QuestToTurnIn->NextQuest;

				ActiveQuests.RemoveAt(i);
				OnQuestListUpdated.Broadcast();

				if (FollowUpQuest)
				{
					AcceptQuest(FollowUpQuest);
				}
			}
			return;
		}
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

void UQuestComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UQuestComponent, ActiveQuests);
	DOREPLIFETIME(UQuestComponent, CompletedQuestIDs);
}
void UQuestComponent::OnPlayerLevelUp(int32 NewLevel)
{
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