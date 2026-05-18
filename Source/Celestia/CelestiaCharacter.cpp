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
#include "Components/ProgressionComponent.h"
#include "Components/StatsComponent.h"
#include "Quests/QuestComponent.h"
#include "Characters/NPC/NPCBase.h"
#include "Components/ManaComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"


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

	ProgressionComponent = CreateDefaultSubobject<UProgressionComponent>(TEXT("Progression Component"));

	StatsComponent = CreateDefaultSubobject<UStatsComponent>(TEXT("StatsComponent"));
	ManaComponent = CreateDefaultSubobject<UManaComponent>(TEXT("ManaComponent"));

	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanSwim = true;
	GetCharacterMovement()->MaxSwimSpeed = 400.f; // Velocidad bajo el agua
	GetCharacterMovement()->Buoyancy = 1.1f; // Flotabilidad (mayor a 1 flota hacia arriba, menor a 1 se hunde)
	GetCharacterMovement()->BrakingDecelerationSwimming = 1000.f; // Fricción del agua

}

void ACelestiaCharacter::BeginPlay()
{

	Super::BeginPlay();

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ACelestiaCharacter::OnWaterOverlapBegin);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ACelestiaCharacter::OnWaterOverlapEnd);

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
			if (ManaComponent)
			{
				ManaComponent->OnManaChanged.AddDynamic(PlayerHUDInstance, &UUIPlayerHUD::UpdateMana);
				PlayerHUDInstance->UpdateMana(ManaComponent->GetCurrentMana(), ManaComponent->GetMaxMana());
			}

			if (ProgressionComponent)
			{
				ProgressionComponent->OnXPGained.AddDynamic(PlayerHUDInstance, &UUIPlayerHUD::UpdateXP);
				ProgressionComponent->OnLevelUp.AddDynamic(PlayerHUDInstance, &UUIPlayerHUD::UpdateLevel);

				PlayerHUDInstance->UpdateXP(ProgressionComponent->CurrentXP, ProgressionComponent->MaxXPForNextLevel);
				PlayerHUDInstance->UpdateLevel(ProgressionComponent->CurrentLevel);

				if (HasAuthority())
				{
					ProgressionComponent->OnLevelUp.AddDynamic(this, &ACelestiaCharacter::TriggerLevelUpVFX);
				}
			}
			if (UQuestComponent* QuestComp = FindComponentByClass<UQuestComponent>())
			{
				QuestComp->OnObjectiveUpdated.AddDynamic(PlayerHUDInstance, &UUIPlayerHUD::UpdateTrackedQuest);

				QuestComp->OnQuestTracked.AddDynamic(PlayerHUDInstance, &UUIPlayerHUD::UpdateTrackedQuest);

				QuestComp->OnQuestRewardsGranted.AddDynamic(this, &ACelestiaCharacter::ReceiveQuestRewards);

				QuestComp->OnQuestUntracked.AddDynamic(PlayerHUDInstance, &UUIPlayerHUD::ClearTrackedQuest);

				PlayerHUDInstance->ClearTrackedQuest();
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

	if (StatsComponent)
	{
		StatsComponent->OnMaxManaCalculated.AddDynamic(this, &ACelestiaCharacter::OnMaxManaCalculated);
		StatsComponent->OnMaxHealthCalculated.AddDynamic(this, &ACelestiaCharacter::OnMaxHealthCalculated);
		StatsComponent->OnMaxStaminaCalculated.AddDynamic(this, &ACelestiaCharacter::OnMaxStaminaCalculated);
	}

	if (ProgressionComponent && StatsComponent)
	{
		ProgressionComponent->OnLevelUp.AddDynamic(StatsComponent, &UStatsComponent::OnLevelUp);
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


		//Sprint
		EnhancedInputComponent->BindAction(Sprint, ETriggerEvent::Started, this, &ACelestiaCharacter::Sprinting);
		EnhancedInputComponent->BindAction(Sprint, ETriggerEvent::Completed, this, &ACelestiaCharacter::StopSprinting);

		//Interact
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ACelestiaCharacter::OnInteractInput);

		if (IA_Heal)
		{

			EnhancedInputComponent->BindAction(IA_Heal, ETriggerEvent::Started, this, &ACelestiaCharacter::Debug_UsePotionInput);

		}
		if (DashComponent && DashComponent->DashInputAction)
		{

			DashComponent->RegisterMappingContext();
			DashComponent->BindInput(EnhancedInputComponent);

		}
		if (IA_Mana)
		{
			EnhancedInputComponent->BindAction(IA_Mana, ETriggerEvent::Started, this, &ACelestiaCharacter::Debug_UseManaPotionInput);
		}

	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}


}

void ACelestiaCharacter::Move(const FInputActionValue& Value)
{
	if (bIsStunned) return;
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
	if (bIsStunned || !GetController()) return;

	const FRotator Rotation = GetController()->GetControlRotation();

	if (bIsSwimmingCustom)
	{
		FVector ForwardDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);

		// --- EL CONTROL DE LÍMITE QUE PROPUSISTE ---
		float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		// Calculamos el tope: La superficie del agua menos un margen para que no asome toda la cápsula
		float WaterTopLimit = CurrentWaterSurfaceZ - (CapsuleHalfHeight * 0.2f);

		// Si el personaje ya está en la superficie (o más arriba)
		if (GetActorLocation().Z >= WaterTopLimit)
		{
			// Si intenta seguir subiendo (Z positivo), anulamos el eje Z
			if (ForwardDirection.Z > 0.f)
			{
				ForwardDirection.Z = 0.f;
				// Normalizamos para no perder velocidad al ir hacia adelante por la superficie
				ForwardDirection.Normalize();
			}
		}

		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);

		// Gravedad suave si nos quedamos sin estamina
		if (StaminaComponent && !StaminaComponent->HasEnoughStamina(0.1f))
		{
			AddMovementInput(FVector(0.f, 0.f, -1.f), 0.3f);
		}
	}
	else
	{
		// MODO TIERRA
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

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
	if (bIsStunned) return;
	// signal the character to jump
	Jump();
}

void ACelestiaCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void ACelestiaCharacter::OnInteractInput()
{
	TArray<AActor*> OverlappingActors;
	GetCapsuleComponent()->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor->GetAttachParentActor() != nullptr) continue;

		if (Actor && Actor->GetClass()->ImplementsInterface(UI_PickUp::StaticClass()))
		{
			if (HasAuthority())
			{
				II_PickUp::Execute_Interact(Actor, this);
			}
			else
			{
				Server_Interact(Actor);
			}
			return; // Si recogimos un ítem, cortamos la ejecución aquí
		}
	}

		FVector Start = GetActorLocation();
	FVector End = Start + (GetActorForwardVector() * 150.0f); // Busca hasta 1.5 metros hacia adelante
	FCollisionShape Sphere = FCollisionShape::MakeSphere(100.0f); // Grosor de la búsqueda

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // Evitamos que el jugador se detecte a sí mismo

	if (GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_Pawn, Sphere, QueryParams))
	{
		if (ANPCBase* NPC = Cast<ANPCBase>(HitResult.GetActor()))
		{
			NPC->Interact(this);
		}
	}
}
bool ACelestiaCharacter::Server_Interact_Validate(AActor* TargetActor)
{
	return true; 
}

void ACelestiaCharacter::Server_Interact_Implementation(AActor* TargetActor)
{
	if (TargetActor && TargetActor->GetClass()->ImplementsInterface(UI_PickUp::StaticClass()))
	{
		II_PickUp::Execute_Interact(TargetActor, this);
	}
}

void ACelestiaCharacter::AddPotion(int32 AmountToAdd)
{
	PotionCount += AmountToAdd;


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
	if (StaminaComponent && StaminaComponent->HasEnoughStamina())
	{
		GetCharacterMovement()->MaxWalkSpeed = 1000.f;
		StaminaComponent->StartDraining(StaminaComponent->SprintCostPerSecond);

		if (!HasAuthority())
		{
			Server_SetSprinting(true);
		}
	}
}

void ACelestiaCharacter::StopSprinting()
{
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	if (StaminaComponent)
	{
		StaminaComponent->StopDraining();
	}

	if (!HasAuthority())
	{
		Server_SetSprinting(false);
	}
}

bool ACelestiaCharacter::Server_SetSprinting_Validate(bool bIsSprinting)
{
	return true; 
}

void ACelestiaCharacter::Server_SetSprinting_Implementation(bool bIsSprinting)
{
	
	if (bIsSprinting)
	{
		
		if (StaminaComponent && StaminaComponent->HasEnoughStamina())
		{
			GetCharacterMovement()->MaxWalkSpeed = 1000.f;
			StaminaComponent->StartDraining(StaminaComponent->SprintCostPerSecond);
		}
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = 500.f;
		if (StaminaComponent)
		{
			StaminaComponent->StopDraining();
		}
	}
}
void ACelestiaCharacter::OnStaminaExhausted()
{
	StopSprinting();

	if (bIsSwimmingCustom)
	{
		// Solo reducimos drásticamente la velocidad, pero seguimos "Volando"
		GetCharacterMovement()->MaxFlySpeed = 150.f;

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, TEXT("Te estas hundiendo"));
		}

		if (!GetWorldTimerManager().IsTimerActive(DrownTimerHandle))
		{
			GetWorldTimerManager().SetTimer(DrownTimerHandle, this, &ACelestiaCharacter::DrownTick, DrownTickInterval, true);
		}
	}
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
	if (OverlappedWaterBodies > 0)
	{
		CheckWaterLevel();
	}

}

void ACelestiaCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	float FallDistance = MaxZHeightDuringFall - GetActorLocation().Z;

	
	if (FallDistance >= MinFallDistance)
	{
		if (HealthComponent)
		{
			float RealMinDamage = HealthComponent->MaxHealth * (MinFallDamagePercent / 100.f);
			float RealMaxDamage = HealthComponent->MaxHealth * (MaxFallDamagePercent / 100.f);

			float FallDamage = FMath::GetMappedRangeValueClamped(
				FVector2D(MinFallDistance, MaxFallDistance),
				FVector2D(RealMinDamage, RealMaxDamage),
				FallDistance
			);

			HealthComponent->TakeDamage(FallDamage, true);

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Orange,
					FString::Printf(TEXT("Caida de %.1f metros! Danio: %.1f (%.0f%% de vida)"),
						FallDistance / 100.f,
						FallDamage,
						(FallDamage / HealthComponent->MaxHealth) * 100.f));
			}
		}
	}


	MaxZHeightDuringFall = GetActorLocation().Z;
}

void ACelestiaCharacter::OnMaxManaCalculated(float NewMaxMana)
{
	if (ManaComponent)
	{
		ManaComponent->UpdateMaxMana(NewMaxMana);
	}
}
void ACelestiaCharacter::OnMaxHealthCalculated(float NewMaxHealth)
{
	if (HealthComponent)
	{
		// 1. Calculamos cuánta vida extra estamos ganando
		float HealthDifference = NewMaxHealth - HealthComponent->MaxHealth;

		// 2. Actualizamos el límite
		HealthComponent->MaxHealth = NewMaxHealth;

		// 3. Le sumamos esa misma diferencia a la vida actual para no "lastimar" al jugador
		HealthComponent->Health += HealthDifference;

		// Aseguramos que no se pase
		HealthComponent->Health = FMath::Clamp(HealthComponent->Health, 0.f, HealthComponent->MaxHealth);

		// 4. Disparamos el evento pasando la diferencia como un delta positivo (curación)
		HealthComponent->OnHealthChanged.Broadcast(HealthComponent, HealthComponent->Health, HealthComponent->MaxHealth, HealthDifference);
	}
}
void ACelestiaCharacter::ReceiveItem_Implementation(int32 Amount, const FString& ItemName)
{
	if (ItemName == "PocionVida")
	{
		AddPotion(Amount);
	}
	else if (ItemName == "PocionMana")
	{
		AddManaPotion(Amount);
	}

}
void ACelestiaCharacter::AddManaPotion(int32 AmountToAdd)
{
	ManaPotionCount += AmountToAdd;

	if (GEngine)
	{
		const FString Msg = FString::Printf(TEXT("Pociones de Mana: %d"), ManaPotionCount);
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, Msg);
	}
}

bool ACelestiaCharacter::TryUseManaPotion(int32 NumPotions)
{
	if (NumPotions <= 0) return false;

	if (!ManaComponent)
	{
		return false;
	}

	// Evitar usar si el maná ya está al máximo
	if (FMath::IsNearlyEqual(ManaComponent->GetCurrentMana(), ManaComponent->GetMaxMana()))
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Ya tenes el mana al maximo"));
		return false;
	}

	if (ManaPotionCount <= 0)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("No hay pociones de mana disponibles"));
		return false;
	}

	const int32 Use = FMath::Clamp(NumPotions, 1, ManaPotionCount);
	const float AmountToRestore = Use * RestorePerManaPotion;

	// Restaurar el maná a través del componente
	ManaComponent->RestoreMana(AmountToRestore);

	// Restar del inventario
	ManaPotionCount -= Use;

	if (GEngine)
	{
		const FString Msg = FString::Printf(TEXT("Usaste pocion de mana. Restantes: %d"), ManaPotionCount);
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Blue, Msg);
	}

	return true;
}
void ACelestiaCharacter::Debug_UseManaPotionInput()
{
	TryUseManaPotion();
}

void ACelestiaCharacter::TriggerLevelUpVFX(int32 NewLevel)
{
	Multicast_PlayLevelUpVFX();
}

void ACelestiaCharacter::Multicast_PlayLevelUpVFX_Implementation()
{
	
	if (LevelUpVFX && GetMesh())
	{
	
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			LevelUpVFX,
			GetMesh(),
			NAME_None,
			FVector(0.f, 0.f, 0.f), 
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}
}

void ACelestiaCharacter::ApplyStun_Implementation(float Duration)
{
	if (bIsStunned) return;

	bIsStunned = true;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->StopMovementImmediately();
	}

	if (StunMontage)
	{
		PlayAnimMontage(StunMontage);
	}

	if (StunVFX && GetMesh())
	{

		FRotator Rotation = FRotator(90.f, 0.f, 0.f);

		ActiveStunVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			StunVFX,
			GetMesh(),
			TEXT("head"), 
			FVector(0.f, 0.f, 20.f),
			Rotation,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}


	GetWorldTimerManager().SetTimer(StunTimerHandle, this, &ACelestiaCharacter::ReleaseStun, Duration, false);
}

void ACelestiaCharacter::ReleaseStun()
{
	bIsStunned = false;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	if (ActiveStunVFX)
	{
		ActiveStunVFX->DestroyComponent();
		ActiveStunVFX = nullptr;
	}
	OnStunEnded();
}

void ACelestiaCharacter::ReceiveQuestRewards(int32 CoinsReward, const TArray<FItemReward>& ItemsReward)
{
	if (GEngine && CoinsReward > 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("+%d Monedas por mision"), CoinsReward));
	}

	for (const FItemReward& Reward : ItemsReward)
	{
		if (Reward.ItemClass)
		{
			FString ItemName = Reward.ItemClass->GetName();

			ItemName.RemoveFromEnd(TEXT("_C"));
			ItemName.RemoveFromStart(TEXT("BP_"));

			ReceiveItem_Implementation(Reward.Quantity, ItemName);

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Recibiste: %s x%d"), *ItemName, Reward.Quantity));
			}
		}
	}
}
void ACelestiaCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	// Solo dejamos el Super, borramos todo el código del agua que había acá
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
}
void ACelestiaCharacter::DrownTick()
{
	if (HealthComponent && !HealthComponent->IsDead())
	{
		// Aplicamos daño. 
		// Amount = DrownDamagePerTick | bIsCritical = false | bIgnoreDefense = true (el ahogo ignora armadura)
		HealthComponent->TakeDamage(DrownDamagePerTick, false, true);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, TEXT("*Glup glup... Daño por ahogo*"));
		}
	}
	else
	{
		GetWorldTimerManager().ClearTimer(DrownTimerHandle);
	}
}
void ACelestiaCharacter::OnWaterOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && OtherComp)
	{
		FString ActorClassName = OtherActor->GetClass()->GetName();
		FName CollisionProfile = OtherComp->GetCollisionProfileName();

		if (ActorClassName.Contains(TEXT("WaterBody")) || CollisionProfile == FName("WaterBodyCollision"))
		{
			OverlappedWaterBodies++;

			if (OverlappedWaterBodies == 1)
			{
				bIsSwimmingCustom = true;
				GetCharacterMovement()->SetMovementMode(MOVE_Flying);
				GetCharacterMovement()->MaxFlySpeed = 400.f;
				GetCharacterMovement()->BrakingDecelerationFlying = 1500.f;

				// Iniciamos el drenaje de estamina acá
				if (StaminaComponent)
				{
					StaminaComponent->StartDraining(SwimStaminaCostPerSecond);
					StaminaComponent->bCanRegen = false;
				}
			}
		}
	}
}

void ACelestiaCharacter::OnWaterOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor != this && OtherComp)
	{
		FString ActorClassName = OtherActor->GetClass()->GetName();
		FName CollisionProfile = OtherComp->GetCollisionProfileName();

		if (ActorClassName.Contains(TEXT("WaterBody")) || CollisionProfile == FName("WaterBodyCollision"))
		{
			OverlappedWaterBodies--;

			if (OverlappedWaterBodies <= 0)
			{
				OverlappedWaterBodies = 0;
				bIsSwimmingCustom = false;
				GetCharacterMovement()->SetMovementMode(MOVE_Falling);

				// Restauramos la velocidad de caminar normal por si se había ahogado
				GetCharacterMovement()->MaxWalkSpeed = 500.f;

				// Detenemos la estamina acá
				if (StaminaComponent)
				{
					StaminaComponent->StopDraining();
					StaminaComponent->bCanRegen = true;
				}

				// FUNDAMENTAL: Obligamos a que se detenga el daño al salir del agua
				GetWorldTimerManager().ClearTimer(DrownTimerHandle);
			}
		}
	}
}
void ACelestiaCharacter::CheckWaterLevel()
{
	if (GetCharacterMovement()->MovementMode == MOVE_Walking && bIsSwimmingCustom)
	{
		bIsSwimmingCustom = false;
		if (StaminaComponent)
		{
			StaminaComponent->StopDraining();
			StaminaComponent->bCanRegen = true;
		}
		GetWorldTimerManager().ClearTimer(DrownTimerHandle);
		return; 
	}
	FVector Start = GetActorLocation() + FVector(0.f, 0.f, 5000.f);
	FVector End = GetActorLocation() - FVector(0.f, 0.f, 200.f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	// Trazamos buscando SOLO el agua
	if (GetWorld()->LineTraceSingleByProfile(HitResult, Start, End, FName("WaterBodyCollision"), Params))
	{
		CurrentWaterSurfaceZ = HitResult.ImpactPoint.Z;

		float WaterSurfaceZ = HitResult.ImpactPoint.Z;
		float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		float FeetZ = GetActorLocation().Z - (CapsuleHalfHeight * 1.1f);
		float ChestZ = GetActorLocation().Z + (CapsuleHalfHeight * 0.4f);

		if (!bIsSwimmingCustom && WaterSurfaceZ >= ChestZ)
		{
			bIsSwimmingCustom = true;
			if (GetCharacterMovement()->MovementMode != MOVE_Flying)
			{
				GetCharacterMovement()->SetMovementMode(MOVE_Flying);
				GetCharacterMovement()->BrakingDecelerationFlying = 400.f;
				GetCharacterMovement()->MaxFlySpeed = 400.f;
			}

			if (StaminaComponent)
			{
				StaminaComponent->StartDraining(SwimStaminaCostPerSecond);
				StaminaComponent->bCanRegen = false;
			}
		}
		else if (bIsSwimmingCustom && WaterSurfaceZ < FeetZ)
		{
			bIsSwimmingCustom = false;
			if (GetCharacterMovement()->MovementMode != MOVE_Falling)
			{
				GetCharacterMovement()->SetMovementMode(MOVE_Falling);
			}

			if (StaminaComponent)
			{
				StaminaComponent->StopDraining();
				StaminaComponent->bCanRegen = true;
			}
			GetWorldTimerManager().ClearTimer(DrownTimerHandle);
		}
	}
	else
	{
		// Si el rayo bajó desde 50 metros y NO encontró agua, forzamos la salida
		// por si el Overlap se quedó bugeado en la orilla.
		if (bIsSwimmingCustom)
		{
			bIsSwimmingCustom = false;
			GetCharacterMovement()->SetMovementMode(MOVE_Falling);

			if (StaminaComponent)
			{
				StaminaComponent->StopDraining();
				StaminaComponent->bCanRegen = true;
			}
			GetWorldTimerManager().ClearTimer(DrownTimerHandle);
		}
	}
}

void ACelestiaCharacter::ResetFallDamageTracking()
{
	MaxZHeightDuringFall = GetActorLocation().Z;
}
void ACelestiaCharacter::OnMaxStaminaCalculated(float NewMaxStamina)
{
	if (StaminaComponent)
	{
		float StaminaDifference = NewMaxStamina - StaminaComponent->MaxStamina;

		StaminaComponent->MaxStamina = NewMaxStamina;

		if (StaminaDifference > 0.0f)
		{
			StaminaComponent->CurrentStamina += StaminaDifference;
		}

		StaminaComponent->CurrentStamina = FMath::Clamp(StaminaComponent->CurrentStamina, 0.0f, StaminaComponent->MaxStamina);

		StaminaComponent->OnStaminaChanged.Broadcast(StaminaComponent, StaminaComponent->CurrentStamina, StaminaComponent->MaxStamina);
	}
}
