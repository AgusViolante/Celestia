// Fill out your copyright notice in the Description page of Project Settings.


#include "Quests/QuestLocationTrigger.h"
#include "Components/BoxComponent.h"
#include "Quests/QuestComponent.h"

AQuestLocationTrigger::AQuestLocationTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBox->SetBoxExtent(FVector(200.f, 200.f, 100.f)); 

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AQuestLocationTrigger::OnOverlapBegin);
}

void AQuestLocationTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && LocationID != NAME_None)
	{
		if (UQuestComponent* QuestComp = OtherActor->FindComponentByClass<UQuestComponent>())
		{
			QuestComp->UpdateObjective(EObjectiveType::Location, LocationID, nullptr, 1);
		}
	}
}

