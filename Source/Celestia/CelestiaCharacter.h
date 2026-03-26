// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Components/StatsComponent.h"
#include "Interfaces/I_PickUp.h"



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

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

#include "CelestiaCharacter.generated.h"


UCLASS(abstract)
class CELESTIA_API ACelestiaCharacter : public ACharacter, public II_PickUp
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


public:

	/** Constructor */
	ACelestiaCharacter();

	//Interface


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

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Components")
	UStaminaComponent* StaminaComponent;

	// Función que se ejecuta al apretar la 'E'
	void OnInteractInput();

	virtual void ReceiveItem_Implementation(int32 Amount, const FString& ItemName) override;


protected:
	virtual void BeginPlay() override;
	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	UFUNCTION()
	virtual void OnDeath(AActor* DeadOwner);

	// --- DAÑO DE CAÍDA ---
	// Variable oculta para llevar el registro
	UPROPERTY()
	float MaxZHeightDuringFall = 0.f;

	// Distancia mínima en el aire para empezar a sufrir daño (1000 unidades = 10 metros)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fall Damage")
	float MinFallDistance = 1000.f;

	// Distancia que causa el daño máximo / muerte instantánea
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fall Damage")
	float MaxFallDistance = 2000.f;

	// Daño que recibís si caés justo la distancia mínima
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fall Damage")
	float MinFallDamage = 10.f;

	// Daño que recibís si caés la distancia máxima (100 para matar a un PJ con 100 de vida)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fall Damage")
	float MaxFallDamage = 100.f;

	UFUNCTION()
	void OnStaminaExhausted();

	UFUNCTION()
	void OnMaxManaCalculated(float NewMaxMana);

	UFUNCTION()
	void OnMaxHealthCalculated(float NewMaxHealth);

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

	virtual void Tick(float DeltaTime) override;

	virtual void Landed(const FHitResult& Hit) override;
	

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

