// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "DashComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CELESTIA_API UDashComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDashComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
    float DashStrength = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
    float DashTeleportDistance = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
    float DashCooldown = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
    bool bUseTeleportDash = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
    float AirDashMultiplier = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
    float FrictionRestoreDelay = 0.12f;

    // --- Input ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Input")
    UInputAction* DashInputAction = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Input")
    UInputMappingContext* MappingContext = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Dash|Input")
    void BindInput(class UEnhancedInputComponent* EnhancedInputComponent);

    UFUNCTION(BlueprintCallable, Category = "Dash|Input")
    void RegisterMappingContext();

    UFUNCTION()
    void OnDashInput(const FInputActionInstance& Instance);

    UFUNCTION(BlueprintCallable, Category = "Dash")
    void TriggerDash();

protected:
    virtual void BeginPlay() override;

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_TriggerDash();

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_DashVisuals();

    void ExecuteDashPhysics();

private:
    bool bCanDash = true;
    bool bIsBound = false;
    FTimerHandle DashCooldownTimer;
    FTimerHandle RestoreFrictionTimer;

    float SavedBrakingFriction = 0.f;
    float SavedGroundFriction = 0.f;
    bool bFrictionSaved = false;

    void ResetDash();
    void RestoreFriction();
};