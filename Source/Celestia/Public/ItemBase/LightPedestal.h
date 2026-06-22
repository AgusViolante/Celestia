#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/I_PickUp.h"
#include "LightPedestal.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class CELESTIA_API ALightPedestal : public AActor, public II_PickUp
{
	GENERATED_BODY()

public:
	ALightPedestal();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, Category = "Pedestal Setup")
	FName PedestalID;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PedestalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CrystalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerZone;

	UFUNCTION(BlueprintImplementableEvent, Category = "Pedestal Logic")
	bool TryConsumeCrystalFromInventory(AActor* PlayerActor);

	UPROPERTY(ReplicatedUsing = OnRep_CrystalPlaced)
	bool bCrystalPlaced = false;

	UFUNCTION()
	void OnRep_CrystalPlaced();

public:
	virtual void Interact_Implementation(AActor* Interactor) override;
};