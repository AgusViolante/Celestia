// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Components/StatsComponent.h"
#include "Interfaces/I_PickUp.h"
#include "Interfaces/StunnableInterface.h"



class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UHealthComponent;
class UDashComponent;
struct FInputActionValue;
struct FInputActionInstance;
class UUIPlayerHUD;
class UStaminaComponent;
class UProgressionComponent;
class UStatsComponent;
class UManaComponent;
class UNiagaraSystem;
class UNiagaraComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

#include "CelestiaCharacter.generated.h"


UCLASS(abstract)
class CELESTIA_API ACelestiaCharacter : public ACharacter, public II_PickUp, public IStunnableInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* Sprint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	UInputAction* IA_Dash;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	UInputAction* IA_Heal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	UInputMappingContext* IMC_Default;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dash")
	UDashComponent* DashComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProgressionComponent> ProgressionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStatsComponent> StatsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UManaComponent> ManaComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	UInputAction* IA_Mana;

	UFUNCTION()
	void Debug_UseManaPotionInput();

public:

	/** Constructor */
	ACelestiaCharacter();


	//Componentes
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Components")
	UHealthComponent* HealthComponent;


	//Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 PotionCount = 0;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddPotion(int32 AmountToAdd);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion")
	float HealPerPotion = 25.f;

	UFUNCTION(BlueprintCallable, Category = "Potion")
	bool TryUsePotion(int32 NumPotions = 1);

	UFUNCTION(BlueprintCallable, Category = "Potion")
	bool UseOnePotion();

	UFUNCTION()
	void Debug_UsePotionInput();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUIPlayerHUD> PlayerHUDClass;

	UPROPERTY()
	UUIPlayerHUD* PlayerHUDInstance;

	UFUNCTION()
	void ReceiveQuestRewards(int32 CoinsReward, const TArray<FItemReward>& ItemsReward);

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Components")
	UStaminaComponent* StaminaComponent;

//E input
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
	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	virtual void OnDeath(AActor* DeadOwner);

	// --- DAÑO DE CAÍDA ---
	// Variable oculta para llevar el registro
	UPROPERTY()
	float MaxZHeightDuringFall = 0.f;

	// Distancia mínima en el aire para empezar a sufrir daño
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fall Damage")
	float MinFallDistance = 1000.f;

	// Distancia que causa el daño máximo
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fall Damage")
	float MaxFallDistance = 2000.f;

	// PORCENTAJE de daño si caés la distancia mínima (Ej: 10.0 = 10% de tu vida máxima)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fall Damage")
	float MinFallDamagePercent = 10.f;

	// PORCENTAJE de daño si caés la distancia máxima (Ej: 100.0 = Muerte instantánea)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fall Damage")
	float MaxFallDamagePercent = 100.f;

	UFUNCTION()
	void OnStaminaExhausted();

	UFUNCTION()
	void OnMaxManaCalculated(float NewMaxMana);

	UFUNCTION()
	void OnMaxHealthCalculated(float NewMaxHealth);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 ManaPotionCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion")
	float RestorePerManaPotion = 25.f;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddManaPotion(int32 AmountToAdd);

	UFUNCTION(BlueprintCallable, Category = "Potion")
	bool TryUseManaPotion(int32 NumPotions = 1);

	// --- VFX DE LEVEL UP ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VFX")
	UNiagaraSystem* LevelUpVFX;

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayLevelUpVFX();

	UFUNCTION()
	void TriggerLevelUpVFX(int32 NewLevel);

	// --- VARIABLES DE STUN ---
	UPROPERTY(EditAnywhere, Category = "Abilities | Stun")
	UAnimMontage* StunMontage;

	FTimerHandle StunTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities | Stun")
	bool bIsStunned = false;

	UPROPERTY(EditAnywhere, Category = "Abilities | Stun")
	UNiagaraSystem* StunVFX; 

	UPROPERTY()
	class UNiagaraComponent* ActiveStunVFX;

	// --- FUNCION DE LIBERACION ---

	UFUNCTION(BlueprintImplementableEvent, Category = "Abilities | Stun")
	void OnStunEnded();

	void ReleaseStun();
	//AHOGO

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

	//DETECCIÓN DE AGUA
	UFUNCTION()
	void OnWaterOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnWaterOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY()
	int32 OverlappedWaterBodies = 0;

	void CheckWaterLevel();

	UPROPERTY()
	float CurrentWaterSurfaceZ = 0.f;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void Sprinting();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void StopSprinting();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetSprinting(bool bIsSprinting);

	virtual void Tick(float DeltaTime) override;

	virtual void Landed(const FHitResult& Hit) override;
	
	UPROPERTY(BlueprintReadOnly, Category = "Swimming")
	bool bIsSwimmingCustom = false;

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

