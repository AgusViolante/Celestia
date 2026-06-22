#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ManaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManaChangedSignature, float, CurrentMana, float, MaxMana);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CELESTIA_API UManaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UManaComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentMana, Category = "Mana")
	float CurrentMana;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Mana")
	float MaxMana;

	UFUNCTION()
	void OnRep_CurrentMana();

	UFUNCTION()
	void OnRep_MaxMana();

public:
	UPROPERTY(BlueprintAssignable, Category = "Mana | Events")
	FOnManaChangedSignature OnManaChanged;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mana")
	float CalculateDynamicCost(float BaseCost, float Multiplier) const;

	UFUNCTION(BlueprintCallable, Category = "Mana")
	bool TryConsumeDynamicMana(float BaseCost, float LevelMultiplier);

	UFUNCTION(BlueprintCallable, Category = "Mana")
	void RestoreMana(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Mana")
	void UpdateMaxMana(float NewMaxMana);

	float GetCurrentMana() const { return CurrentMana; }
	float GetMaxMana() const { return MaxMana; }

	UFUNCTION(BlueprintCallable, Category = "Mana")
	bool TryRestoreMana(float AmountToRestore);
};