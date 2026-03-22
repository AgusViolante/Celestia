// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"



class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UHealthComponent;
class UDashComponent;
struct FInputActionValue;
struct FInputActionInstance;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

#include "CelestiaCharacter.generated.h"


UCLASS(abstract)
class CELESTIA_API ACelestiaCharacter : public ACharacter
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
	UInputAction* Using;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* Sprint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	UInputAction* IA_Dash;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	UInputAction* IA_Heal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	UInputMappingContext* IMC_Default;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dash")
	UDashComponent* DashComponent;
public:

	/** Constructor */
	ACelestiaCharacter();

	//Interface


	//Componentes
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Components")
	UHealthComponent* HealthComponent;


	//Variables
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventary") int Coins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventary") int Potion = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int CuraCharacter = 5.0f;

	UFUNCTION()
	void Debug_HealInput();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 PotionCount = 0;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddPotion(int32 Amount);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion")
	float HealPerPotion = 25.f;

	UFUNCTION(BlueprintCallable, Category = "Potion")
	bool TryUsePotion(int32 NumPotions = 1);

	UFUNCTION(BlueprintCallable, Category = "Potion")
	bool UseOnePotion();

	UFUNCTION()
	void Debug_UsePotionInput();

protected:
	virtual void BeginPlay() override;
	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

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




	

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

