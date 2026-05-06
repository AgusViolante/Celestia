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
}

void UQuestComponent::AcceptQuest(UQuestDataAsset* NewQuest)
{
	if (!NewQuest) return;

	for (const FActiveQuest& Quest : ActiveQuests)
	{
		if (Quest.QuestData == NewQuest) return;
	}
	if (CompletedQuestIDs.Contains(NewQuest->QuestID)) return;
	FActiveQuest NewActiveQuest(NewQuest);

	NewActiveQuest.CurrentObjectives = NewQuest->Objectives;

	for (FQuestObjective& Obj : NewActiveQuest.CurrentObjectives)
	{
		
		Obj.CurrentAmount = 0; 
	}

	ActiveQuests.Add(NewActiveQuest);

	if (!TrackedQuestData)
	{
		TrackQuest(NewQuest);
	}

	OnQuestListUpdated.Broadcast();
}

void UQuestComponent::UpdateObjective(EObjectiveType Type, FName TargetID, TSubclassOf<AItemBase> TargetItemClass, int32 Amount)
{
	for (int32 i = 0; i < ActiveQuests.Num(); ++i)
	{
		FActiveQuest& ActiveQuest = ActiveQuests[i];

		if (ActiveQuest.bIsReadyToTurnIn) continue;

		bool bQuestUpdated = false;

		for (FQuestObjective& Objective : ActiveQuest.CurrentObjectives)
		{
			if (Objective.ObjectiveType == Type && Objective.CurrentAmount < Objective.RequiredAmount)
			{
				bool bIsMatch = false;

				if (Type == EObjectiveType::Collect)
				{
					if (Objective.TargetItemClass == TargetItemClass) bIsMatch = true;
				}
				else if (Type == EObjectiveType::Kill)
				{
					if (Objective.TargetID == TargetID) bIsMatch = true;
				}

				if (bIsMatch)
				{
					Objective.CurrentAmount = FMath::Clamp(Objective.CurrentAmount + Amount, 0, Objective.RequiredAmount);
					bQuestUpdated = true;
				}
			}
		}

		if (bQuestUpdated)
		{
			CheckQuestCompletion(i);

			if (TrackedQuestData == ActiveQuest.QuestData)
			{
				OnObjectiveUpdated.Broadcast(ActiveQuest);
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
					}

					
					OnQuestRewardsGranted.Broadcast(QuestToTurnIn->Rewards.Coins, QuestToTurnIn->Rewards.RewardItems);
					OnQuestItemsConsumed.Broadcast(ActiveQuests[i].CurrentObjectives);
				}

				CompletedQuestIDs.Add(QuestToTurnIn->QuestID);

				if (TrackedQuestData == QuestToTurnIn)
				{
					TrackedQuestData = nullptr;
				}

				ActiveQuests.RemoveAtSwap(i);

				OnQuestListUpdated.Broadcast();
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
