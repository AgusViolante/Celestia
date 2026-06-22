#include "ItemBase/ItemBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
	RootComponent = BoxCollision;
	BoxCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	CubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube Mesh"));
	CubeMesh->SetupAttachment(RootComponent);
	CubeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AItemBase::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority() || bIsPickedUp) return;

	bIsPickedUp = true;

	if (Interactor && Interactor->GetClass()->ImplementsInterface(UI_PickUp::StaticClass()))
	{
		II_PickUp::Execute_ReceiveItem(Interactor, Amount, ItemName);
		Destroy();
	}
}