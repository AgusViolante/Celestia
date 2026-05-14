// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NPC/NPCBase.h"
#include "Quests/QuestComponent.h"
#include "UI/UINPCDialogue.h"
#include "Components/CapsuleComponent.h"

ANPCBase::ANPCBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// Configuración básica para que no sea controlado por un jugador y tenga físicas correctas
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	if (GetCapsuleComponent())
	{
		// Aseguramos que bloquee al jugador pero permita interactuar
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}
}

// Esta es la implementación base en C++
void ANPCBase::Interact_Implementation(AActor* Interactor)
{
	if (!Interactor || !DialogueWidgetClass) return;

	if (APawn* InteractorPawn = Cast<APawn>(Interactor))
	{
		if (APlayerController* PC = Cast<APlayerController>(InteractorPawn->GetController()))
		{
			// Si es el jugador local, creamos y mostramos la UI
			if (PC->IsLocalPlayerController())
			{
				UUINPCDialogue* DialogueUI = CreateWidget<UUINPCDialogue>(PC, DialogueWidgetClass);
				if (DialogueUI)
				{
					DialogueUI->AddToViewport();
					DialogueUI->SetupUI(this, Interactor);
				}
			}
		}
	}
}

void ANPCBase::ProcessQuestInteraction(AActor* Interactor)
{
	if (!Interactor) return;

	UQuestComponent* QuestComp = Interactor->FindComponentByClass<UQuestComponent>();
	if (!QuestComp) return;

	for (const FActiveQuest& ActiveQuest : QuestComp->ActiveQuests)
	{
		if (ActiveQuest.bIsReadyToTurnIn && ActiveQuest.QuestData->ReceiverNPC_ID == NPC_ID)
		{
			QuestComp->TurnInQuest(ActiveQuest.QuestData);
			return;
		}
	}

	for (UQuestDataAsset* QuestToGive : AvailableQuests)
	{
		if (QuestToGive && QuestToGive->GiverNPC_ID == NPC_ID)
		{
			if (QuestComp->CanAcceptQuest(QuestToGive))
			{
				QuestComp->AcceptQuest(QuestToGive);
				return;
			}
		}
	}
}