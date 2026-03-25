#include "ItemBase/Chest.h" 
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ProgressionComponent.h" 
#include "Engine/World.h"
#include "TimerManager.h" // Necesario para usar temporizadores
#include "Math/UnrealMathUtility.h" // Necesario para generar los números aleatorios

AChest::AChest()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
	RootComponent = BoxCollision;
	BoxCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	BoxCollision->SetBoxExtent(FVector(150.f, 150.f, 100.f));

	ChestMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Chest Mesh"));
	ChestMesh->SetupAttachment(RootComponent);
	ChestMesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void AChest::Interact_Implementation(AActor* Interactor)
{

	if (bIsLooted) return;

	bIsLooted = true;

	if (Interactor)
	{
		UProgressionComponent* ProgComp = Interactor->GetComponentByClass<UProgressionComponent>();
		if (ProgComp)
		{
			ProgComp->AddXP(XPReward);
		}

		if (GetWorld())
		{

			for (TSubclassOf<AActor> ItemClass : ItemsToSpawn)
			{

				if (ItemClass)
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

					GetWorld()->SpawnActor<AActor>(ItemClass, PosicionFinalSpawn, FRotator::ZeroRotator);
				}
			}
		}


		OnChestOpenedVisuals();

		GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &AChest::DestroyChest, DestroyDelay, false);
	}
}

void AChest::DestroyChest()
{
	Destroy();
}