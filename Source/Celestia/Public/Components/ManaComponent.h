// Fill out your copyright notice in the Description page of Project Settings.
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

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mana")
	float CurrentMana;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mana")
	float MaxMana;

public:
	UPROPERTY(BlueprintAssignable, Category = "Mana | Events")
	FOnManaChangedSignature OnManaChanged;

	UFUNCTION(BlueprintCallable, Category = "Mana")
	bool TryConsumeMana(float Cost);

	UFUNCTION(BlueprintCallable, Category = "Mana")
	void RestoreMana(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Mana")
	void UpdateMaxMana(float NewMaxMana);

	// Getters
	float GetCurrentMana() const { return CurrentMana; }
	float GetMaxMana() const { return MaxMana; }

	UFUNCTION(BlueprintCallable, Category = "Mana")
	bool TryRestoreMana(float AmountToRestore);
};