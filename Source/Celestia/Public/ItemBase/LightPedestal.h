
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LightPedestal.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class CELESTIA_API ALightPedestal : public AActor
{
	GENERATED_BODY()

public:
	ALightPedestal();

	// Identificador para el objetivo de la misión (Ej: "Pedestal_Pueblo")
	UPROPERTY(EditAnywhere, Category = "Pedestal Setup")
	FName PedestalID;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* PedestalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* CrystalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* TriggerZone;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void InteractToPlaceCrystal();

	// --- EL PUENTE HACIA BLUEPRINTS ---
	UFUNCTION(BlueprintImplementableEvent, Category = "Pedestal Logic")
	bool TryConsumeCrystalFromInventory(AActor* PlayerActor);

private:
	bool bCrystalPlaced = false;
};