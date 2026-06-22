#include "Components/DashComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

UDashComponent::UDashComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bCanDash = true;
    bFrictionSaved = false;

    SetIsReplicatedByDefault(true);
}

void UDashComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UDashComponent::TriggerDash()
{
    if (!bCanDash) return;

    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar) return;

    if (OwnerChar->HasAuthority())
    {
        ExecuteDashPhysics();
        Multicast_DashVisuals();
    }
    else if (OwnerChar->IsLocallyControlled())
    {
        ExecuteDashPhysics();
        Server_TriggerDash();
    }
}

bool UDashComponent::Server_TriggerDash_Validate()
{
    return true;
}

void UDashComponent::Server_TriggerDash_Implementation()
{
    if (!bCanDash) return;

    ExecuteDashPhysics();
    Multicast_DashVisuals();
}

void UDashComponent::Multicast_DashVisuals_Implementation()
{

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