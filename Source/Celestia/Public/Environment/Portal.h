#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/I_PickUp.h" 
#include "Portal.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USphereComponent;
class ACharacter;

UCLASS()
class CELESTIA_API APortal : public AActor, public II_PickUp
{
	GENERATED_BODY()

public:
	APortal();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal | Components")
	TObjectPtr<UBoxComponent> BoxCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal | Components")
	TObjectPtr<UStaticMeshComponent> PortalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal | Components")
	TObjectPtr<USphereComponent> DiscoverySphere;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Portal | Connection")
	TObjectPtr<APortal> LinkedPortal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal | Connection")
	FName PortalID = "BasePortal";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal | Transition")
	float FadeDuration = 1.0f;

	FTimerHandle TeleportTimerHandle;

	UPROPERTY()
	TObjectPtr<ACharacter> InteractingPlayer;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartFadeOut(ACharacter* PlayerCharacter, float Duration);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartFadeIn(ACharacter* PlayerCharacter, float Duration);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShowLockedMessage(ACharacter* PlayerCharacter, FName DestinationID);

	void ExecuteTeleport();

	UFUNCTION()
	void OnDiscoveryOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	virtual void Interact_Implementation(AActor* Interactor) override;
};