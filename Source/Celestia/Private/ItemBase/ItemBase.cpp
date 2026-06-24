#include "ItemBase/ItemBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
	RootComponent = BoxCollision;

	BoxCollision->SetCollisionProfileName(TEXT("Trigger"));
	BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxCollision->SetGenerateOverlapEvents(true);

	CubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube Mesh"));
	CubeMesh->SetupAttachment(RootComponent);
	CubeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && BoxCollision)
	{
		BoxCollision->OnComponentBeginOverlap.AddUniqueDynamic(this, &AItemBase::OnOverlapBegin);
		BoxCollision->OnComponentEndOverlap.AddUniqueDynamic(this, &AItemBase::OnOverlapEnd);
	}
}

void AItemBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority() && OtherActor && OtherActor->IsA(APawn::StaticClass()))
	{
		Multicast_ToggleHighlight(OtherActor, true);
	}
}

void AItemBase::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (HasAuthority() && OtherActor && OtherActor->IsA(APawn::StaticClass()))
	{
		Multicast_ToggleHighlight(OtherActor, false);
	}
}

void AItemBase::Multicast_ToggleHighlight_Implementation(AActor* PlayerActor, bool bTurnOn)
{
	if (PlayerActor)
	{
		APawn* Pawn = Cast<APawn>(PlayerActor);
		if (Pawn && Pawn->IsLocallyControlled())
		{
			CubeMesh->SetRenderCustomDepth(bTurnOn);
			if (bTurnOn)
			{
				CubeMesh->SetCustomDepthStencilValue(1);
			}
		}
	}
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