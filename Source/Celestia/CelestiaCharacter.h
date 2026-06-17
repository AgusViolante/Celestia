#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Components/StatsComponent.h"
#include "Interfaces/I_PickUp.h"
#include "Interfaces/StunnableInterface.h"
#include "CelestiaCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UHealthComponent;
class UDashComponent;
struct FInputActionValue;
class UUIPlayerHUD;
class UStaminaComponent;
class UProgressionComponent;
class UStatsComponent;
class UManaComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class UAnimMontage;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(abstract)
class CELESTIA_API ACelestiaCharacter : public ACharacter, public II_PickUp, public IStunnableInterface
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

protected:

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MouseLookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> Sprint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> IA_Dash;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputMappingContext> IMC_Default;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dash")
	TObjectPtr<UDashComponent> DashComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProgressionComponent> ProgressionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStatsComponent> StatsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UManaComponent> ManaComponent;

public:

	ACelestiaCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Components")
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUIPlayerHUD> PlayerHUDClass;

	UPROPERTY()
	TObjectPtr<UUIPlayerHUD> PlayerHUDInstance;

	UFUNCTION()
	void ReceiveQuestRewards(int32 CoinsReward, const TArray<FItemReward>& ItemsReward);

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaminaComponent> StaminaComponent;

	void OnInteractInput();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Interact(AActor* TargetActor);

	virtual void ReceiveItem_Implementation(int32 Amount, const FString& ItemName) override;

	virtual void ApplyStun_Implementation(float Duration) override;

	UFUNCTION(BlueprintPure, Category = "Abilities | Stun")
	bool IsStunned() const { return bIsStunned; }

	UFUNCTION(BlueprintCallable, Category = "Fall Damage")
	void ResetFallDamageTracking();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable)
	virtual void OnDeath(AActor* DeadOwner);

	UPROPERTY()
	float MaxZHeightDuringFall = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fall Damage")
	float MinFallDistance = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fall Damage")
	float MaxFallDistance = 2000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fall Damage")
	float MinFallDamagePercent = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fall Damage")
	float MaxFallDamagePercent = 100.f;

	UFUNCTION()
	void OnStaminaExhausted();

	UFUNCTION()
	void OnMaxManaCalculated(float NewMaxMana);

	UFUNCTION()
	void OnMaxStaminaCalculated(float NewMaxStamina);

	UFUNCTION()
	void OnMaxHealthCalculated(float NewMaxHealth);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> LevelUpVFX;

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayLevelUpVFX();

	UFUNCTION()
	void TriggerLevelUpVFX(int32 NewLevel);

	UPROPERTY(EditAnywhere, Category = "Abilities | Stun")
	TObjectPtr<UAnimMontage> StunMontage;

	FTimerHandle StunTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_IsStunned, Category = "Abilities | Stun")
	bool bIsStunned = false;

	UFUNCTION()
	void OnRep_IsStunned();

	UPROPERTY(EditAnywhere, Category = "Abilities | Stun")
	TObjectPtr<UNiagaraSystem> StunVFX;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> ActiveStunVFX;

	UFUNCTION(BlueprintImplementableEvent, Category = "Abilities | Stun")
	void OnStunEnded();

	void ReleaseStun();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stamina|Swimming")
	float DrownDamagePerTick = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stamina|Swimming")
	float DrownTickInterval = 1.0f;

	FTimerHandle DrownTimerHandle;

	UFUNCTION()
	void DrownTick();

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stamina|Swimming")
	float SwimStaminaCostPerSecond = 0.5f;

	UFUNCTION()
	void OnWaterOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnWaterOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY()
	int32 OverlappedWaterBodies = 0;

	void CheckWaterLevel();

	UPROPERTY()
	float CurrentWaterSurfaceZ = 0.f;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

public:

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoLook(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpStart();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void Sprinting();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void StopSprinting();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetSprinting(bool bIsSprinting);

	virtual void Tick(float DeltaTime) override;
	virtual void Landed(const FHitResult& Hit) override;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Swimming")
	bool bIsSwimmingCustom = false;

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};