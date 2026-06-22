#include "Dunegons/Public/DungeonExit.h"
#include "Components/BoxComponent.h"
#include "Quests/QuestComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "../../../CelestiaCharacter.h"

ADungeonExit::ADungeonExit()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	TriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerZone"));
	RootComponent = TriggerZone;
	TriggerZone->SetBoxExtent(FVector(150.f, 150.f, 100.f));
	TriggerZone->SetCollisionProfileName(TEXT("Trigger"));
}

void ADungeonExit::BeginPlay()
{
	AActor::BeginPlay();

	TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &ADungeonExit::OnOverlapBegin);
	TriggerZone->OnComponentEndOverlap.AddDynamic(this, &ADungeonExit::OnOverlapEnd);
}

bool ADungeonExit::IsDungeonQuestComplete(AActor* PlayerActor)
{
	if (UQuestComponent* QuestComp = PlayerActor->FindComponentByClass<UQuestComponent>())
	{
		for (const FActiveQuest& Quest : QuestComp->ActiveQuests)
		{
			for (const FQuestObjective& Obj : Quest.CurrentObjectives)
			{
				if (Obj.ObjectiveType == EObjectiveType::Dungeon && Obj.TargetID == DungeonID)
				{
					if (Obj.CurrentAmount >= Obj.RequiredAmount)
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}

void ADungeonExit::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ACharacter* PlayerChar = Cast<ACharacter>(OtherActor))
	{
		if (PlayerChar->IsLocallyControlled() && IsDungeonQuestComplete(PlayerChar))
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

void ADungeonExit::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ACharacter* PlayerChar = Cast<ACharacter>(OtherActor))
	{
		if (PlayerChar->IsLocallyControlled() && PromptInstance && PromptInstance->IsInViewport())
		{
			PromptInstance->RemoveFromParent();
		}
	}
}

void ADungeonExit::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority()) return;

	if (TeleportDestination && Interactor && IsDungeonQuestComplete(Interactor))
	{
		Interactor->SetActorLocationAndRotation(TeleportDestination->GetActorLocation(), TeleportDestination->GetActorRotation());

		if (ACelestiaCharacter* CelestiaChar = Cast<ACelestiaCharacter>(Interactor))
		{
			CelestiaChar->ResetFallDamageTracking();
		}
	}
}