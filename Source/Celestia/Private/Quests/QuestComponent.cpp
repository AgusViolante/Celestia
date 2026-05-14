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

bool UQuestComponent::CanAcceptQuest(UQuestDataAsset* QuestToCheck) const
{
	if (!QuestToCheck) return false;

	if (CompletedQuestIDs.Contains(QuestToCheck->QuestID)) return false;

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
			return false; // Le falta la misión anterior
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

	if (!TrackedQuestData)
	{
		TrackQuest(NewQuest);
	}

	OnQuestAccepted.Broadcast(ActiveQuests.Last());
	OnQuestListUpdated.Broadcast();
}
void UQuestComponent::UpdateObjective(EObjectiveType Type, FName TargetID, TSubclassOf<AItemBase> TargetItemClass, int32 Amount)
{
	bool bAnyQuestUpdated = false;

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

				// Lógica de coincidencia según tipo
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
			// Primero checkeamos si esto activó el autocompletado de una Dungeon
			CheckDungeonCompletion();

			// Luego checkeamos si la misión entera se terminó
			CheckQuestCompletion(i);

			// Notificamos a la UI si es la misión que estamos siguiendo
			if (TrackedQuestData == ActiveQuest.QuestData)
			{
				OnObjectiveUpdated.Broadcast(ActiveQuest);
			}
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

		// Escaneamos los objetivos de esta misión
		for (int32 j = 0; j < Quest.CurrentObjectives.Num(); ++j)
		{
			if (Quest.CurrentObjectives[j].ObjectiveType == EObjectiveType::Dungeon)
			{
				bHasDungeonObjective = true;
				DungeonObjIndex = j;
			}
			else
			{
				// Si cualquier objetivo que NO sea Dungeon no está listo, la dungeon no se completa sola
				if (Quest.CurrentObjectives[j].CurrentAmount < Quest.CurrentObjectives[j].RequiredAmount)
				{
					bAllOtherTasksComplete = false;
				}
			}
		}

		// Si tiene objetivo de Dungeon y todo lo demás está 100%
		if (bHasDungeonObjective && bAllOtherTasksComplete)
		{
			FQuestObjective& DungeonObj = Quest.CurrentObjectives[DungeonObjIndex];

			if (DungeonObj.CurrentAmount < DungeonObj.RequiredAmount)
			{
				// Autocompletamos el meta-objetivo
				DungeonObj.CurrentAmount = DungeonObj.RequiredAmount;

				// Refrescamos la UI para que aparezca el check en "Completar Dungeon"
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
		// Opcional: Broadcast de misión lista para entregar
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