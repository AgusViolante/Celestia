// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemBase/LightPedestal.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Quests/QuestComponent.h"
#include "GameFramework/Character.h"
#include "Components/InputComponent.h"

ALightPedestal::ALightPedestal()
{
	PrimaryActorTick.bCanEverTick = false;

	PedestalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PedestalMesh"));
	RootComponent = PedestalMesh;

	CrystalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrystalMesh"));
	CrystalMesh->SetupAttachment(RootComponent);
	CrystalMesh->SetVisibility(false);

	TriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerZone"));
	TriggerZone->SetupAttachment(RootComponent);
	TriggerZone->SetBoxExtent(FVector(150.f, 150.f, 100.f));
	TriggerZone->SetCollisionProfileName(TEXT("Trigger"));
}

void ALightPedestal::BeginPlay()
{
	Super::BeginPlay();

	TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &ALightPedestal::OnOverlapBegin);
	TriggerZone->OnComponentEndOverlap.AddDynamic(this, &ALightPedestal::OnOverlapEnd);
}

void ALightPedestal::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bCrystalPlaced) return;

	if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
	{
		APlayerController* PC = Cast<APlayerController>(Cast<ACharacter>(OtherActor)->GetController());
		if (PC)
		{
			EnableInput(PC);
			if (InputComponent)
			{
				InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ALightPedestal::InteractToPlaceCrystal);
			}
		}
	}
}

void ALightPedestal::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
	{
		APlayerController* PC = Cast<APlayerController>(Cast<ACharacter>(OtherActor)->GetController());
		if (PC)
		{
			DisableInput(PC);
		}
	}
}

void ALightPedestal::InteractToPlaceCrystal()
{
	if (bCrystalPlaced) return;

	AActor* PlayerActor = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (!PlayerActor) return;

	if (!TryConsumeCrystalFromInventory(PlayerActor))
	{
		return;
	}

	bCrystalPlaced = true;
	CrystalMesh->SetVisibility(true);

	DisableInput(GetWorld()->GetFirstPlayerController());

	UQuestComponent* QuestComp = PlayerActor->FindComponentByClass<UQuestComponent>();
	if (QuestComp)
	{
		QuestComp->UpdateObjective(EObjectiveType::Location, PedestalID, nullptr, 1);
	}
}

