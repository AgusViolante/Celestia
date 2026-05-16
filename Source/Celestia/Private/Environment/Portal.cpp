// Fill out your copyright notice in the Description page of Project Settings.


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

	// Inicializamos la esfera de descubrimiento
	DiscoverySphere = CreateDefaultSubobject<USphereComponent>(TEXT("Discovery Sphere"));
	DiscoverySphere->SetupAttachment(RootComponent);
	DiscoverySphere->SetSphereRadius(500.0f); // Radio por defecto, se puede cambiar en BP
	DiscoverySphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void APortal::BeginPlay()
{
	Super::BeginPlay();

	// Solo el servidor detecta el descubrimiento
	if (HasAuthority())
	{
		DiscoverySphere->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnDiscoveryOverlap);
	}
}

void APortal::OnDiscoveryOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
	if (PlayerChar)
	{
		UProgressionComponent* ProgComp = PlayerChar->GetComponentByClass<UProgressionComponent>();
		if (ProgComp)
		{
			// Se desbloquea al entrar en el radio
			ProgComp->UnlockPortal(PortalID);
		}
	}
}

void APortal::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority()) return;

	// Evita que el spam de la tecla E reinicie el temporizador y cancele el Fade Out
	if (GetWorld()->GetTimerManager().IsTimerActive(TeleportTimerHandle))
	{
		return;
	}

	ACharacter* PlayerChar = Cast<ACharacter>(Interactor);
	if (!PlayerChar) return;

	if (!LinkedPortal)
	{
		UE_LOG(LogTemp, Warning, TEXT("El portal %s no tiene destino asignado en el nivel."), *PortalID.ToString());
		return;
	}

	UProgressionComponent* ProgComp = PlayerChar->GetComponentByClass<UProgressionComponent>();
	if (!ProgComp) return;

	if (ProgComp->IsPortalUnlocked(LinkedPortal->PortalID))
	{
		InteractingPlayer = PlayerChar;

		Client_StartFadeOut(InteractingPlayer, FadeDuration);

		GetWorld()->GetTimerManager().SetTimer(TeleportTimerHandle, this, &APortal::ExecuteTeleport, FadeDuration, false);
	}
	else
	{
		Client_ShowLockedMessage(LinkedPortal->PortalID);
	}
}

void APortal::ExecuteTeleport()
{
	if (InteractingPlayer && LinkedPortal)
	{
		// Spawn offset para no quedar trabado en el collider destino
		FVector DestLocation = LinkedPortal->GetActorLocation() + (LinkedPortal->GetActorForwardVector() * 150.0f);
		FRotator DestRotation = LinkedPortal->GetActorRotation();

		// Teletransportamos al jugador
		InteractingPlayer->SetActorLocationAndRotation(DestLocation, DestRotation, false, nullptr, ETeleportType::TeleportPhysics);

		// casteamos a tu clase específica y reseteamos el daño de caída
		if (ACelestiaCharacter* CelestiaChar = Cast<ACelestiaCharacter>(InteractingPlayer))
		{
			CelestiaChar->ResetFallDamageTracking();
		}

		// Avisamos al cliente que vuelva a mostrar el juego
		Client_StartFadeIn(InteractingPlayer, FadeDuration);
	}

	InteractingPlayer = nullptr;
}

void APortal::Client_StartFadeOut_Implementation(ACharacter* PlayerCharacter, float Duration)
{
	if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
	{
		APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController());
		if (PC && PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, Duration, FLinearColor::Black, false, true);
		}
	}
}

void APortal::Client_StartFadeIn_Implementation(ACharacter* PlayerCharacter, float Duration)
{
	if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
	{
		APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController());
		if (PC && PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, Duration, FLinearColor::Black, false, false);
		}
	}
}

void APortal::Client_ShowLockedMessage_Implementation(FName DestinationID)
{
	FString Msg = FString::Printf(TEXT("Aún no has descubierto el portal: %s"), *DestinationID.ToString());
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, Msg);
}