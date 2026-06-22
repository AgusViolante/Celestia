#include "ItemBase/LightPedestal.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Quests/QuestComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

ALightPedestal::ALightPedestal()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

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

void ALightPedestal::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALightPedestal, bCrystalPlaced);
}

void ALightPedestal::BeginPlay()
{
	Super::BeginPlay();
}

void ALightPedestal::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority() || bCrystalPlaced) return;

	if (Interactor && TryConsumeCrystalFromInventory(Interactor))
	{
		bCrystalPlaced = true;
		OnRep_CrystalPlaced();

		if (UQuestComponent* QuestComp = Interactor->FindComponentByClass<UQuestComponent>())
		{
			QuestComp->UpdateObjective(EObjectiveType::Location, PedestalID, nullptr, 1);
		}
	}
}

void ALightPedestal::OnRep_CrystalPlaced()
{
	if (CrystalMesh)
	{
		CrystalMesh->SetVisibility(bCrystalPlaced);
	}
}