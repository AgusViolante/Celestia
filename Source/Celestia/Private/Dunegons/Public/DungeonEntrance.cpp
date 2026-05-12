// Fill out your copyright notice in the Description page of Project Settings.


#include "Dunegons/Public/DungeonEntrance.h"
#include "Components/BoxComponent.h"
#include "Quests/QuestComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InputComponent.h" 

ADungeonEntrance::ADungeonEntrance()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerZone"));
	RootComponent = TriggerZone;
	TriggerZone->SetBoxExtent(FVector(150.f, 150.f, 100.f));
	TriggerZone->SetCollisionProfileName(TEXT("Trigger"));
}

void ADungeonEntrance::BeginPlay()
{
	Super::BeginPlay();

	TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &ADungeonEntrance::OnOverlapBegin);
	TriggerZone->OnComponentEndOverlap.AddDynamic(this, &ADungeonEntrance::OnOverlapEnd);
}

bool ADungeonEntrance::HasDungeonQuest(AActor* PlayerActor)
{
	UQuestComponent* QuestComp = PlayerActor->FindComponentByClass<UQuestComponent>();
	if (!QuestComp) return false;

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
	return false;
}

void ADungeonEntrance::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
	{
		if (HasDungeonQuest(OtherActor))
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
					InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ADungeonEntrance::InteractToEnter);
				}
			}
		}
	}
}

void ADungeonEntrance::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
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

void ADungeonEntrance::InteractToEnter()
{
	if (!MapName.IsNone())
	{
		
		UGameplayStatics::OpenLevel(this, MapName);
	}
}