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
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

ACelestiaCharacter::ACelestiaCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

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
	GetCharacterMovement()->MaxSwimSpeed = 400.f;
	GetCharacterMovement()->Buoyancy = 1.1f;
	GetCharacterMovement()->BrakingDecelerationSwimming = 1000.f;
}

void ACelestiaCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACelestiaCharacter, bIsStunned);
	DOREPLIFETIME(ACelestiaCharacter, bIsSwimmingCustom);
}

void ACelestiaCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ACelestiaCharacter::HandleTakeAnyDamage);
	}

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

			if (HealthComponent)
			{
				HealthComponent->OnHealthChanged.AddDynamic(PlayerHUDInstance, &UUIPlayerHUD::UpdateHealth);
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
				QuestComp->OnQuestAccepted.AddDynamic(PlayerHUDInstance, &UUIPlayerHUD::ShowNewQuestNotification);

				if (QuestComp->TrackedQuestData)
				{
					FActiveQuest TrackedActiveQuest;
					if (QuestComp->GetActiveQuestData(QuestComp->TrackedQuestData, TrackedActiveQuest))
					{
						PlayerHUDInstance->UpdateTrackedQuest(TrackedActiveQuest);
					}
				}
				else
				{
					PlayerHUDInstance->ClearTrackedQuest();
				}
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

void ACelestiaCharacter::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (!HasAuthority() || !HealthComponent || HealthComponent->IsDead()) return;

	float FinalDamage = Damage;

	if (StatsComponent)
	{
		float Defense = StatsComponent->GetStatValue(ERPGStatType::MeleeDefense);
		Defense = FMath::Max(0.0f, Defense);
		float MitigationMultiplier = 100.0f / (100.0f + Defense);
		FinalDamage = Damage * MitigationMultiplier;
	}

	HealthComponent->TakeDamage(FinalDamage);
}

void ACelestiaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACelestiaCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ACelestiaCharacter::Look);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACelestiaCharacter::Look);

		EnhancedInputComponent->BindAction(Sprint, ETriggerEvent::Started, this, &ACelestiaCharacter::Sprinting);
		EnhancedInputComponent->BindAction(Sprint, ETriggerEvent::Completed, this, &ACelestiaCharacter::StopSprinting);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ACelestiaCharacter::OnInteractInput);

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
	if (bIsStunned) return;
	FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void ACelestiaCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
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

		float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		float WaterTopLimit = CurrentWaterSurfaceZ - (CapsuleHalfHeight * 0.2f);

		if (GetActorLocation().Z >= WaterTopLimit)
		{
			if (ForwardDirection.Z > 0.f)
			{
				ForwardDirection.Z = 0.f;
				ForwardDirection.Normalize();
			}
		}

		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);

		if (StaminaComponent && !StaminaComponent->HasEnoughStamina(0.1f))
		{
			AddMovementInput(FVector(0.f, 0.f, -1.f), 0.3f);
		}
	}
	else
	{
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
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ACelestiaCharacter::DoJumpStart()
{
	if (bIsStunned) return;
	Jump();
}

void ACelestiaCharacter::DoJumpEnd()
{
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
			return;
		}
	}

	FVector Start = GetActorLocation();
	FVector End = Start + (GetActorForwardVector() * 150.0f);
	FCollisionShape Sphere = FCollisionShape::MakeSphere(100.0f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

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

	if (GetCharacterMovement()->IsFalling())
	{
		MaxZHeightDuringFall = FMath::Max(MaxZHeightDuringFall, GetActorLocation().Z);
	}
	else
	{
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
		HealthComponent->UpdateMaxHealth(NewMaxHealth);
	}
}

void ACelestiaCharacter::ReceiveItem_Implementation(int32 Amount, const FString& ItemName)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString::Printf(TEXT("Item recogido: %s x%d"), *ItemName, Amount));
	}
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
	if (bIsStunned || !HasAuthority()) return;

	bIsStunned = true;
	OnRep_IsStunned();

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->StopMovementImmediately();
	}

	GetWorldTimerManager().SetTimer(StunTimerHandle, this, &ACelestiaCharacter::ReleaseStun, Duration, false);
}

void ACelestiaCharacter::OnRep_IsStunned()
{
	if (bIsStunned)
	{
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
	}
	else
	{
		if (ActiveStunVFX)
		{
			ActiveStunVFX->DestroyComponent();
			ActiveStunVFX = nullptr;
		}
		OnStunEnded();
	}
}

void ACelestiaCharacter::ReleaseStun()
{
	bIsStunned = false;
	OnRep_IsStunned();

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
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
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
}

void ACelestiaCharacter::DrownTick()
{
	if (HealthComponent && !HealthComponent->IsDead())
	{
		HealthComponent->TakeDamage(DrownDamagePerTick);

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
				GetCharacterMovement()->MaxWalkSpeed = 500.f;

				if (StaminaComponent)
				{
					StaminaComponent->StopDraining();
					StaminaComponent->bCanRegen = true;
				}

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