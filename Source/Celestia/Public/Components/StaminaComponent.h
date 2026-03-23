#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StaminaComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStaminaChangedSignature, class UStaminaComponent*, StaminaComp, float, CurrentStamina, float, MaxStamina);


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaExhaustedSignature);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CELESTIA_API UStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStaminaComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float MaxStamina = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
	float CurrentStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Rates")
	float SprintCostPerSecond = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Rates")
	float RegenPerSecond = 10.f;

	UPROPERTY(BlueprintAssignable, Category = "Stamina")
	FOnStaminaChangedSignature OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stamina")
	FOnStaminaExhaustedSignature OnStaminaExhausted;

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void StartDraining(float DrainRate);

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void StopDraining();

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	bool HasEnoughStamina(float MinimumRequired = 1.0f) const;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool bIsDraining = false;
	float CurrentDrainRate = 0.f;
};