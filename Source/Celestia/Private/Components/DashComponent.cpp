// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/DashComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "TimerManager.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"

UDashComponent::UDashComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bCanDash = true;
    bIsBound = false;
    bFrictionSaved = false;

    SetIsReplicatedByDefault(true);
}

void UDashComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UDashComponent::BindInput(UEnhancedInputComponent* EnhancedInputComponent)
{
  
    if (bIsBound) return;

    EnhancedInputComponent->BindAction(DashInputAction, ETriggerEvent::Started, this, &UDashComponent::OnDashInput);
    bIsBound = true;
}

void UDashComponent::RegisterMappingContext()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    ACharacter* Char = Cast<ACharacter>(Owner);
    if (!Char) return;

    APlayerController* PC = Cast<APlayerController>(Char->GetController());
    if (!PC) return;

    ULocalPlayer* LP = PC->GetLocalPlayer();
    if (!LP) return;

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
    {
        if (MappingContext)
        {
            const int32 Priority = 0;
            Subsystem->AddMappingContext(MappingContext, Priority);
        }
    }
}

void UDashComponent::OnDashInput(const FInputActionInstance& Instance)
{
    TriggerDash();
}

void UDashComponent::TriggerDash()
{

    if (!bCanDash) return;

    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar) return;

    if (OwnerChar->IsLocallyControlled() || OwnerChar->HasAuthority())
    {
        ExecuteDashPhysics();
    }

    if (!OwnerChar->HasAuthority() && OwnerChar->IsLocallyControlled())
    {
        Server_TriggerDash();
    }
}

bool UDashComponent::Server_TriggerDash_Validate()
{
    return true; 
}

void UDashComponent::Server_TriggerDash_Implementation()
{
    ExecuteDashPhysics();

    Multicast_DashVisuals();
}

void UDashComponent::Multicast_DashVisuals_Implementation()
{
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar) return;

   
}
void UDashComponent::ExecuteDashPhysics()
{
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar) return;

    const bool bIsFalling = OwnerChar->GetCharacterMovement() ? OwnerChar->GetCharacterMovement()->IsFalling() : false;
    const float Mult = bIsFalling ? FMath::Max(0.0f, AirDashMultiplier) : 1.0f;

    if (bUseTeleportDash)
    {
        FVector Forward = OwnerChar->GetActorForwardVector();
        Forward.Z = 0.f;
        Forward = Forward.GetSafeNormal();
        FVector NewLoc = OwnerChar->GetActorLocation() + Forward * DashTeleportDistance * Mult;
        OwnerChar->SetActorLocation(NewLoc, true);
    }
    else
    {
        UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement();
        if (MoveComp && !bFrictionSaved)
        {
            SavedBrakingFriction = MoveComp->BrakingFrictionFactor;
            SavedGroundFriction = MoveComp->GroundFriction;
            bFrictionSaved = true;

            MoveComp->BrakingFrictionFactor = 0.f;
            MoveComp->GroundFriction = 0.f;
        }

        FVector Forward = OwnerChar->GetActorForwardVector();
        Forward.Z = 0.f;
        Forward = Forward.GetSafeNormal();
        if (Forward.IsNearlyZero()) Forward = OwnerChar->GetActorForwardVector();

        FVector LaunchVel = Forward * (DashStrength * Mult);

        OwnerChar->LaunchCharacter(LaunchVel, true, false);

        if (GetWorld() && bFrictionSaved)
        {
            GetWorld()->GetTimerManager().ClearTimer(RestoreFrictionTimer);
            GetWorld()->GetTimerManager().SetTimer(RestoreFrictionTimer, this, &UDashComponent::RestoreFriction, FrictionRestoreDelay, false);
        }
    }

    bCanDash = false;
    if (GetWorld())
    {
        const float Interval = FMath::Max(0.001f, DashCooldown);
        GetWorld()->GetTimerManager().ClearTimer(DashCooldownTimer);
        GetWorld()->GetTimerManager().SetTimer(DashCooldownTimer, this, &UDashComponent::ResetDash, Interval, false);
    }
}

void UDashComponent::ResetDash()
{
    bCanDash = true;
}

void UDashComponent::RestoreFriction()
{
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar) return;

    UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement();
    if (!MoveComp) return;

    if (bFrictionSaved)
    {
        MoveComp->BrakingFrictionFactor = SavedBrakingFriction;
        MoveComp->GroundFriction = SavedGroundFriction;
        bFrictionSaved = false;

    }
}
