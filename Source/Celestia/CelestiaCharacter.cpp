// Copyright Epic Games, Inc. All Rights Reserved.

#include "CelestiaCharacter.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "Components/HealthComponent.h" 
#include "Components/DashComponent.h" 
#include "EnhancedInputSubsystems.h"
#include "TimerManager.h"
#include "InputActionValue.h"
#include "Celestia.h"
#include "UI/UIPlayerHUD.h"
#include "Components/StaminaComponent.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

ACelestiaCharacter::ACelestiaCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
	
	//Components
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));


	DashComponent = CreateDefaultSubobject<UDashComponent>(TEXT("DashComponent"));
	DashComponent->DashStrength = 2000.f;
	DashComponent->DashCooldown = 1.0f;
	DashComponent->bUseTeleportDash = false;

	StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));
}

void ACelestiaCharacter::BeginPlay()
{

	Super::BeginPlay();


	if (IMC_Default)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (ULocalPlayer* LP = PC->GetLocalPlayer())
			{
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
				{
					Subsystem->AddMappingContext(IMC_Default, 0);
				}
			}
		}
	}

	if (IsLocallyControlled() && PlayerHUDClass)
	{
		PlayerHUDInstance = CreateWidget<UUIPlayerHUD>(GetWorld(), PlayerHUDClass);
		if (PlayerHUDInstance)
		{
			PlayerHUDInstance->AddToViewport();

			// Conectar el evento de vida
			if (HealthComponent)
			{
				HealthComponent->OnHealthChanged.AddDynamic(PlayerHUDInstance, &UUIPlayerHUD::UpdateHealth);

				// Forzar la primera actualización visual
				PlayerHUDInstance->UpdateHealth(HealthComponent, HealthComponent->Health, HealthComponent->MaxHealth, 0.f);
			}
			if (StaminaComponent)
			{
				StaminaComponent->OnStaminaChanged.AddDynamic(PlayerHUDInstance, &UUIPlayerHUD::UpdateStamina);
				PlayerHUDInstance->UpdateStamina(StaminaComponent, StaminaComponent->CurrentStamina, StaminaComponent->MaxStamina);
			}
		
		}
	}

	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &ACelestiaCharacter::OnDeath);
	}

	if (StaminaComponent)
	{
		StaminaComponent->OnStaminaExhausted.AddDynamic(this, &ACelestiaCharacter::OnStaminaExhausted);
	}
}
void ACelestiaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACelestiaCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ACelestiaCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACelestiaCharacter::Look);

		//Using

		//Sprint
		EnhancedInputComponent->BindAction(Sprint, ETriggerEvent::Started, this, &ACelestiaCharacter::Sprinting);
		EnhancedInputComponent->BindAction(Sprint, ETriggerEvent::Completed, this, &ACelestiaCharacter::StopSprinting);

		if (IA_Heal)
		{

			EnhancedInputComponent->BindAction(IA_Heal, ETriggerEvent::Started, this, &ACelestiaCharacter::Debug_UsePotionInput);

		}
		if (DashComponent && DashComponent->DashInputAction)
		{
			DashComponent->RegisterMappingContext();
			DashComponent->BindInput(EnhancedInputComponent);

		}

	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACelestiaCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void ACelestiaCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ACelestiaCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ACelestiaCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ACelestiaCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void ACelestiaCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void ACelestiaCharacter::AddPotion(int32 Amount)
{
	PotionCount += Amount;


	if (GEngine)
	{
		const FString Msg = FString::Printf(TEXT("Pociones: %d"), PotionCount);
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, Msg);
	}

	
}

bool ACelestiaCharacter::TryUsePotion(int32 NumPotions)
{
	if (NumPotions <= 0) return false;
	if (!HealthComponent)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("No HealthComponent found"));
		return false;
	}

	if (IsValid(HealthComponent) && HealthComponent->IsDead())
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("No se puede curar: estas muerto"));
		return false;
	}

	if (FMath::IsNearlyEqual(HealthComponent->Health, HealthComponent->MaxHealth))
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Ya tenes la vida al maximo"));
		return false;
	}

	if (PotionCount <= 0)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("No hay pociones disponibles"));
		return false;
	}


	const int32 Use = FMath::Clamp(NumPotions, 1, PotionCount);

	const float BeforeHP = HealthComponent->Health;
	const float AmountToHeal = Use * HealPerPotion;

	HealthComponent->Heal(AmountToHeal);

	PotionCount -= Use;



	const float AfterHP = HealthComponent->Health;


	FString AmountStr = FString::SanitizeFloat(AmountToHeal, 1);
	FString BeforeStr = FString::SanitizeFloat(BeforeHP, 1);
	FString AfterStr = FString::SanitizeFloat(AfterHP, 1);
	FString UseStr = FString::FromInt(Use);
	FString LeftStr = FString::FromInt(PotionCount);


	const FString Msg = FString::Format(
		TEXT("Usaste pocion: +{1} HP ({2} -> {3}). Pociones restantes: {4}"),
		{ UseStr, AmountStr, BeforeStr, AfterStr, LeftStr }
	);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, Msg);
	}

	UE_LOG(LogTemplateCharacter, Log, TEXT("%s"), *Msg);

	return true;
}
bool ACelestiaCharacter::UseOnePotion()
{
	return TryUsePotion(1);
}

void ACelestiaCharacter::Debug_UsePotionInput()
{
	UseOnePotion();
}
void ACelestiaCharacter::Sprinting()
{
	// Solo corremos si hay estamina disponible
	if (StaminaComponent && StaminaComponent->HasEnoughStamina())
	{
		GetCharacterMovement()->MaxWalkSpeed = 1000.f;
		StaminaComponent->StartDraining(StaminaComponent->SprintCostPerSecond);
	}
}

void ACelestiaCharacter::StopSprinting()
{
	GetCharacterMovement()->MaxWalkSpeed = 500.f;

	if (StaminaComponent)
	{
		StaminaComponent->StopDraining();
	}
}

void ACelestiaCharacter::OnStaminaExhausted()
{
	// Si la estamina llega a 0, forzamos mecánicamente el fin del sprint
	StopSprinting();
}

void ACelestiaCharacter::OnDeath(AActor* DeadOwner)
{

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}


	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->StopMovementImmediately();
	}


	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	}


	if (GetMesh())
	{
		TArray<USkeletalMeshComponent*> SkeletalMeshes;
		GetComponents<USkeletalMeshComponent>(SkeletalMeshes);

		
		for (USkeletalMeshComponent* SkelMesh : SkeletalMeshes)
		{
			if (SkelMesh)
			{
				SkelMesh->SetCollisionProfileName(TEXT("Ragdoll"));
				SkelMesh->SetSimulatePhysics(true);
				SkelMesh->WakeAllRigidBodies();
				SkelMesh->bBlendPhysics = true;
			}
		}
	}

}
	
void ACelestiaCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Si estamos cayendo/en el aire, registramos constantemente el punto más alto que alcanzamos
	if (GetCharacterMovement()->IsFalling())
	{
		MaxZHeightDuringFall = FMath::Max(MaxZHeightDuringFall, GetActorLocation().Z);
	}
	else
	{
		// Si estamos pisando suelo firme, anclamos el registro a nuestra altura actual
		MaxZHeightDuringFall = GetActorLocation().Z;
	}
}

void ACelestiaCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// Calculamos cuántas unidades caímos desde nuestro punto más alto en el aire
	float FallDistance = MaxZHeightDuringFall - GetActorLocation().Z;

	// Si la caída fue mayor a la distancia mínima configurada, aplicamos daño
	if (FallDistance >= MinFallDistance)
	{
		if (HealthComponent)
		{
			// FMath::GetMappedRangeValueClamped es perfecto para esto:
			// Convierte proporcionalmente el rango de Distancia en un rango de Daño.
			// Si caíste MaxFallDistance (3000), el daño será MaxFallDamage (100), activando tu lógica de muerte y ragdoll.
			float FallDamage = FMath::GetMappedRangeValueClamped(
				FVector2D(MinFallDistance, MaxFallDistance),
				FVector2D(MinFallDamage, MaxFallDamage),
				FallDistance
			);

			HealthComponent->TakeDamage(FallDamage);

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Orange,
					FString::Printf(TEXT("Caida de %.1f metros! Danio: %.1f"), FallDistance / 100.f, FallDamage));
			}
		}
	}

	// Reseteamos el punto más alto tras tocar el suelo para evitar errores en el próximo salto
	MaxZHeightDuringFall = GetActorLocation().Z;
}






