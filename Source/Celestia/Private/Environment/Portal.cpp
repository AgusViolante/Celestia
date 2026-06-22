#include "Environment/Portal.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ProgressionComponent.h" 
#include "../../CelestiaCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "TimerManager.h"

APortal::APortal()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
	RootComponent = BoxCollision;
	BoxCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	BoxCollision->SetBoxExtent(FVector(100.f, 100.f, 150.f));

	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Portal Mesh"));
	PortalMesh->SetupAttachment(RootComponent);
	PortalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	DiscoverySphere = CreateDefaultSubobject<USphereComponent>(TEXT("Discovery Sphere"));
	DiscoverySphere->SetupAttachment(RootComponent);
	DiscoverySphere->SetSphereRadius(500.0f);
	DiscoverySphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void APortal::BeginPlay()
{
	AActor::BeginPlay();

	if (HasAuthority())
	{
		DiscoverySphere->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnDiscoveryOverlap);
	}
}

void APortal::OnDiscoveryOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	if (ACharacter* PlayerChar = Cast<ACharacter>(OtherActor))
	{
		if (UProgressionComponent* ProgComp = PlayerChar->GetComponentByClass<UProgressionComponent>())
		{
			ProgComp->UnlockPortal(PortalID);
		}
	}
}

void APortal::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority()) return;

	if (GetWorld()->GetTimerManager().IsTimerActive(TeleportTimerHandle))
	{
		return;
	}

	ACharacter* PlayerChar = Cast<ACharacter>(Interactor);
	if (!PlayerChar || !LinkedPortal) return;

	if (UProgressionComponent* ProgComp = PlayerChar->GetComponentByClass<UProgressionComponent>())
	{
		if (ProgComp->IsPortalUnlocked(LinkedPortal->PortalID))
		{
			InteractingPlayer = PlayerChar;
			Multicast_StartFadeOut(InteractingPlayer, FadeDuration);
			GetWorld()->GetTimerManager().SetTimer(TeleportTimerHandle, this, &APortal::ExecuteTeleport, FadeDuration, false);
		}
		else
		{
			Multicast_ShowLockedMessage(PlayerChar, LinkedPortal->PortalID);
		}
	}
}

void APortal::ExecuteTeleport()
{
	if (InteractingPlayer && LinkedPortal)
	{
		FVector DestLocation = LinkedPortal->GetActorLocation() + (LinkedPortal->GetActorForwardVector() * 150.0f);
		FRotator DestRotation = LinkedPortal->GetActorRotation();

		InteractingPlayer->SetActorLocationAndRotation(DestLocation, DestRotation, false, nullptr, ETeleportType::TeleportPhysics);

		if (ACelestiaCharacter* CelestiaChar = Cast<ACelestiaCharacter>(InteractingPlayer))
		{
			CelestiaChar->ResetFallDamageTracking();
		}

		Multicast_StartFadeIn(InteractingPlayer, FadeDuration);
	}
	InteractingPlayer = nullptr;
}

void APortal::Multicast_StartFadeOut_Implementation(ACharacter* PlayerCharacter, float Duration)
{
	if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController()))
		{
			if (PC->PlayerCameraManager)
			{
				PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, Duration, FLinearColor::Black, false, true);
			}
		}
	}
}

void APortal::Multicast_StartFadeIn_Implementation(ACharacter* PlayerCharacter, float Duration)
{
	if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController()))
		{
			if (PC->PlayerCameraManager)
			{
				PC->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, Duration, FLinearColor::Black, false, false);
			}
		}
	}
}

void APortal::Multicast_ShowLockedMessage_Implementation(ACharacter* PlayerCharacter, FName DestinationID)
{
	if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
	{
		FString Msg = FString::Printf(TEXT("Aún no has descubierto el portal: %s"), *DestinationID.ToString());
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, Msg);
	}
}