// Fill out your copyright notice in the Description page of Project Settings.

#include "Dunegons/Public/DungeonExit.h"
#include "Components/BoxComponent.h"
#include "Quests/QuestComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InputComponent.h"

ADungeonExit::ADungeonExit()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerZone"));
	RootComponent = TriggerZone;
	TriggerZone->SetBoxExtent(FVector(150.f, 150.f, 100.f));
	TriggerZone->SetCollisionProfileName(TEXT("Trigger"));
}

void ADungeonExit::BeginPlay()
{
	Super::BeginPlay();

	TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &ADungeonExit::OnOverlapBegin);
	TriggerZone->OnComponentEndOverlap.AddDynamic(this, &ADungeonExit::OnOverlapEnd);
}

bool ADungeonExit::IsDungeonQuestComplete(AActor* PlayerActor)
{
	UQuestComponent* QuestComp = PlayerActor->FindComponentByClass<UQuestComponent>();
	if (!QuestComp) return false;

	for (const FActiveQuest& Quest : QuestComp->ActiveQuests)
	{
		for (const FQuestObjective& Obj : Quest.CurrentObjectives)
		{
			// Busca el objetivo de esta mazmorra
			if (Obj.ObjectiveType == EObjectiveType::Dungeon && Obj.TargetID == DungeonID)
			{
				// Si la cantidad actual es igual o mayor a la requerida, está completa
				if (Obj.CurrentAmount >= Obj.RequiredAmount)
				{
					return true;
				}
			}
		}
	}
	return false;
}

void ADungeonExit::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
	{
		// Solo permite interactuar si la misión de esta mazmorra está completa
		if (IsDungeonQuestComplete(OtherActor))
		{
			if (PromptWidgetClass && !PromptInstance)
			{
				PromptInstance = CreateWidget<UUserWidget>(GetWorld(), PromptWidgetClass);
			}

			if (PromptInstance && !PromptInstance->IsInViewport())
			{
				PromptInstance->AddToViewport();
			}

			APlayerController* PC = Cast<APlayerController>(Cast<ACharacter>(OtherActor)->GetController());
			if (PC)
			{
				EnableInput(PC);

				if (InputComponent)
				{
					InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ADungeonExit::InteractToExit);
				}
			}
		}
	}
}

void ADungeonExit::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
	{
		if (PromptInstance && PromptInstance->IsInViewport())
		{
			PromptInstance->RemoveFromParent();
		}

		APlayerController* PC = Cast<APlayerController>(Cast<ACharacter>(OtherActor)->GetController());
		if (PC)
		{
			DisableInput(PC);
		}
	}
}

void ADungeonExit::InteractToExit()
{
	if (TeleportDestination)
	{
		ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
		if (PlayerCharacter)
		{
			PlayerCharacter->TeleportTo(TeleportDestination->GetActorLocation(), TeleportDestination->GetActorRotation());

			if (PromptInstance && PromptInstance->IsInViewport())
			{
				PromptInstance->RemoveFromParent();
			}

			APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController());
			if (PC)
			{
				DisableInput(PC);
			}
		}
	}
}

