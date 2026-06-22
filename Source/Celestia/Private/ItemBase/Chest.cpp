#include "ItemBase/Chest.h" 
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ProgressionComponent.h" 
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h" 
#include "Math/UnrealMathUtility.h" 
#include "Net/UnrealNetwork.h"

AChest::AChest()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
	RootComponent = BoxCollision;
	BoxCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	BoxCollision->SetBoxExtent(FVector(150.f, 150.f, 100.f));

	ChestMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Chest Mesh"));
	ChestMesh->SetupAttachment(RootComponent);
	ChestMesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void AChest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AChest, bIsLooted);
}

void AChest::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority() || bIsLooted) return;

	bIsLooted = true;
	OnRep_IsLooted();

	if (Interactor)
	{
		if (UProgressionComponent* ProgComp = Interactor->GetComponentByClass<UProgressionComponent>())
		{
			ProgComp->AddXP(XPReward);
		}

		if (GetWorld())
		{
			for (const FLootItem& Loot : LootTable)
			{
				if (Loot.ItemClass && FMath::FRandRange(0.0f, 100.0f) <= Loot.DropChance)
				{
					FVector RandomOffset = FVector(FMath::RandRange(-60.0f, 60.0f), FMath::RandRange(-60.0f, 60.0f), 0.0f);
					FVector PuntoInicio = GetActorLocation() + (GetActorForwardVector() * 100.0f) + FVector(0.0f, 0.0f, 100.0f) + RandomOffset;
					FVector PuntoFinal = PuntoInicio - FVector(0.0f, 0.0f, 500.0f);

					FHitResult ResultadoImpacto;
					FCollisionQueryParams ParametrosTrace;
					ParametrosTrace.AddIgnoredActor(this);

					FVector PosicionFinalSpawn = PuntoInicio;

					if (GetWorld()->LineTraceSingleByChannel(ResultadoImpacto, PuntoInicio, PuntoFinal, ECC_Visibility, ParametrosTrace))
					{
						PosicionFinalSpawn = ResultadoImpacto.ImpactPoint + FVector(0.0f, 0.0f, 20.0f);
					}

					GetWorld()->SpawnActor<AActor>(Loot.ItemClass, PosicionFinalSpawn, FRotator::ZeroRotator);
				}
			}
		}

		Multicast_OnChestOpened();
		GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &AChest::DestroyChest, DestroyDelay, false);
	}
}

void AChest::Multicast_OnChestOpened_Implementation()
{
	if (OpenSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, OpenSound, GetActorLocation());
	}
}

void AChest::OnRep_IsLooted()
{
	if (bIsLooted)
	{
		OnChestOpenedVisuals();
	}
}

void AChest::DestroyChest()
{
	Destroy();
}