#include "Characters/NPC/NPCBase.h"
#include "Quests/QuestComponent.h"
#include "UI/UINPCDialogue.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/CapsuleComponent.h"

ANPCBase::ANPCBase()
{
	PrimaryActorTick.bCanEverTick = false;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bReplicates = true;

	QuestIndicatorIcon = CreateDefaultSubobject<UBillboardComponent>(TEXT("QuestIndicatorIcon"));
	QuestIndicatorIcon->SetupAttachment(RootComponent);
	QuestIndicatorIcon->SetRelativeLocation(FVector(0.f, 0.f, 130.f));
	QuestIndicatorIcon->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.0f));
	QuestIndicatorIcon->bHiddenInGame = true;

	static ConstructorHelpers::FObjectFinder<UTexture2D> DefaultIcon(TEXT("/Engine/EditorMaterials/TargetIcon"));
	if (DefaultIcon.Succeeded())
	{
		QuestIndicatorIcon->SetSprite(DefaultIcon.Object);
	}

	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}
}

void ANPCBase::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(QuestIndicatorTimerHandle, this, &ANPCBase::CheckQuestStatus, 1.0f, true);
}

void ANPCBase::Interact_Implementation(AActor* Interactor)
{
	if (!Interactor || !DialogueWidgetClass) return;

	if (APawn* InteractorPawn = Cast<APawn>(Interactor))
	{
		if (APlayerController* PC = Cast<APlayerController>(InteractorPawn->GetController()))
		{
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

void ANPCBase::UpdateQuestIndicator(UQuestComponent* PlayerQuestComp)
{
	if (!QuestIndicatorIcon || !PlayerQuestComp) return;

	bool bHasTurnIn = false;
	bool bHasNewQuest = false;
	bool bHasTalkObjective = false;

	for (const FActiveQuest& ActiveQuest : PlayerQuestComp->ActiveQuests)
	{
		if (ActiveQuest.bIsReadyToTurnIn && ActiveQuest.QuestData && ActiveQuest.QuestData->ReceiverNPC_ID == NPC_ID)
		{
			bHasTurnIn = true;
			break;
		}
	}

	if (!bHasTurnIn)
	{
		for (const FActiveQuest& ActiveQuest : PlayerQuestComp->ActiveQuests)
		{
			if (ActiveQuest.bIsReadyToTurnIn) continue;

			for (const FQuestObjective& Obj : ActiveQuest.CurrentObjectives)
			{
				if (Obj.ObjectiveType == EObjectiveType::Talk && Obj.TargetID == NPC_ID && Obj.CurrentAmount < Obj.RequiredAmount)
				{
					bHasTalkObjective = true;
					break;
				}
			}
			if (bHasTalkObjective) break;
		}
	}

	if (!bHasTurnIn && !bHasTalkObjective)
	{
		for (UQuestDataAsset* QuestToGive : AvailableQuests)
		{
			if (QuestToGive && PlayerQuestComp->CanAcceptQuest(QuestToGive))
			{
				bHasNewQuest = true;
				break;
			}
		}
	}

	if (bHasTurnIn)
	{
		if (Icon_TurnInQuest) QuestIndicatorIcon->SetSprite(Icon_TurnInQuest);
		QuestIndicatorIcon->SetHiddenInGame(false);
	}
	else if (bHasNewQuest || bHasTalkObjective)
	{
		if (Icon_NewQuest) QuestIndicatorIcon->SetSprite(Icon_NewQuest);
		QuestIndicatorIcon->SetHiddenInGame(false);
	}
	else
	{
		QuestIndicatorIcon->SetHiddenInGame(true);
	}
}

void ANPCBase::CheckQuestStatus()
{
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (UQuestComponent* QuestComp = PlayerPawn->FindComponentByClass<UQuestComponent>())
		{
			UpdateQuestIndicator(QuestComp);
		}
	}
}