#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/DeathInterface.h"
#include "HealthComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathSignature, AActor*, DeadOwner);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthChangedSignature, UHealthComponent*, HealthComp, float, Health, float, MaxHealth, float, HealthDelta);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CELESTIA_API UHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHealthComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health")
    float MaxHealth = 100.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health|Regen")
    bool bAutoRegen = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health|Regen", meta=(EditCondition="bAutoRegen"))
    float RegenDelaySeconds = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health|Regen", meta=(EditCondition="bAutoRegen"))
    float RegenPerSecond = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health|Regen", meta=(EditCondition="bAutoRegen"))
    float RegenTickInterval = 1.f;

    UPROPERTY(BlueprintAssignable, Category="Health")
    FOnDeathSignature OnDeath;

    UPROPERTY(BlueprintAssignable, Category = "Health")
    FOnHealthChangedSignature OnHealthChanged;

    UFUNCTION(BlueprintCallable, Category = "Health")
    void TakeDamage(float Amount, bool bIgnoreDefense = false);

    UFUNCTION(BlueprintCallable, Category="Health")
    void Heal(float Amount);

    UFUNCTION(BlueprintCallable, Category="Health")
    bool IsDead() const { return Health <= 0.f; }

 
    UFUNCTION(BlueprintCallable, Category="Health")
    void InitializeAfterSpawn(bool bEnableAutoRegen = true, float InRegenDelaySeconds = -1.f, float InRegenPerSecond = -1.f, float InRegenTickInterval = -1.f);

    UFUNCTION(BlueprintCallable, Category="Health")
    bool IsRegenTimerActive() const;

    FString GetOwnerTypeLabel() const;

protected:
    virtual void BeginPlay() override;

private:
    float LastDamageTime = 0.f;
    FTimerHandle RegenTimerHandle;

    void RegenTick();
};