#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/I_PickUp.h" 
#include "Chest.generated.h"

class UBoxComponent;
class USkeletalMeshComponent;
class USoundBase;

USTRUCT(BlueprintType)
struct FLootItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TSubclassOf<AActor> ItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float DropChance = 50.0f;
};

UCLASS()
class CELESTIA_API AChest : public AActor, public II_PickUp
{
	GENERATED_BODY()

public:
	AChest();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chest | Components")
	TObjectPtr<UBoxComponent> BoxCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chest | Components")
	TObjectPtr<USkeletalMeshComponent> ChestMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest | Rewards")
	TArray<FLootItem> LootTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest | Rewards")
	float XPReward = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest | Settings")
	float DestroyDelay = 1.5f;

	UPROPERTY(ReplicatedUsing = OnRep_IsLooted)
	bool bIsLooted = false;

	UFUNCTION()
	void OnRep_IsLooted();

	FTimerHandle DestroyTimerHandle;

	UFUNCTION(BlueprintImplementableEvent, Category = "Chest | Events")
	void OnChestOpenedVisuals();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnChestOpened();

	void DestroyChest();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest | Audio")
	TObjectPtr<USoundBase> OpenSound;

public:
	virtual void Interact_Implementation(AActor* Interactor) override;
};