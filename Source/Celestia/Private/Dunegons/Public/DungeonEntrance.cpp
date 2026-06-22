#include "Dunegons/Public/DungeonEntrance.h"
#include "Components/BoxComponent.h"
#include "Quests/QuestComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "../../../CelestiaCharacter.h"

ADungeonEntrance::ADungeonEntrance()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	TriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerZone"));
	RootComponent = TriggerZone;
	TriggerZone->SetBoxExtent(FVector(150.f, 150.f, 100.f));
	TriggerZone->SetCollisionProfileName(TEXT("Trigger"));
}

void ADungeonEntrance::BeginPlay()
{
	AActor::BeginPlay();

	TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &ADungeonEntrance::OnOverlapBegin);
	TriggerZone->OnComponentEndOverlap.AddDynamic(this, &ADungeonEntrance::OnOverlapEnd);
}

bool ADungeonEntrance::HasDungeonQuest(AActor* PlayerActor)
{
	if (UQuestComponent* QuestComp = PlayerActor->FindComponentByClass<UQuestComponent>())
	{
		for (const FActiveQuest& Quest : QuestComp->ActiveQuests)
		{
			if (Quest.bIsReadyToTurnIn) continue;

			for (const FQuestObjective& Obj : Quest.CurrentObjectives)
			{
				if (Obj.ObjectiveType == EObjectiveType::Dungeon && Obj.TargetID == DungeonID)
				{
					if (Obj.CurrentAmount < Obj.RequiredAmount)
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}

void ADungeonEntrance::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ACharacter* PlayerChar = Cast<ACharacter>(OtherActor))
	{
		if (PlayerChar->IsLocallyControlled() && HasDungeonQuest(PlayerChar))
		{
			if (PromptWidgetClass && !PromptInstance)
			{
				PromptInstance = CreateWidget<UUserWidget>(GetWorld(), PromptWidgetClass);
			}

			if (PromptInstance && !PromptInstance->IsInViewport())
			{
				PromptInstance->AddToViewport();
			}
		}
	}
}

void ADungeonEntrance::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ACharacter* PlayerChar = Cast<ACharacter>(OtherActor))
	{
		if (PlayerChar->IsLocallyControlled() && PromptInstance && PromptInstance->IsInViewport())
		{
			PromptInstance->RemoveFromParent();
		}
	}
}

void ADungeonEntrance::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority()) return;

	if (TeleportDestination && Interactor && HasDungeonQuest(Interactor))
	{
		Interactor->SetActorLocationAndRotation(TeleportDestination->GetActorLocation(), TeleportDestination->GetActorRotation());

		if (ACelestiaCharacter* CelestiaChar = Cast<ACelestiaCharacter>(Interactor))
		{
			CelestiaChar->ResetFallDamageTracking();
		}
	}
}