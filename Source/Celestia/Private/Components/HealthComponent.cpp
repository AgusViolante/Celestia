#include "Components/HealthComponent.h"
#include "Interfaces/StunnableInterface.h"
#include "Interfaces/DeathInterface.h"
#include "Characters/EnemyBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h" 
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);

    MaxHealth = 100.f;
    Health = MaxHealth;
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UHealthComponent, Health);
    DOREPLIFETIME(UHealthComponent, MaxHealth);
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        if (Owner->HasAuthority())
        {
            Health = FMath::Clamp(Health, 0.f, MaxHealth);

            if (GetWorld())
            {
                LastDamageTime = GetWorld()->GetTimeSeconds();
            }

            InitializeAfterSpawn(bAutoRegen, RegenDelaySeconds, RegenPerSecond, RegenTickInterval);
        }
    }
}

void UHealthComponent::OnRep_Health(float OldHealth)
{
    float HealthDelta = Health - OldHealth;
    OnHealthChanged.Broadcast(this, Health, MaxHealth, HealthDelta);

    if (IsDead() && !bHasDied)
    {
        bHasDied = true;
        AActor* Owner = GetOwner();
        OnDeath.Broadcast(Owner);
    }
}

void UHealthComponent::TakeDamage(float Amount, bool bIsCritical)
{
    if (!GetOwner() || !GetOwner()->HasAuthority()) return;
    if (Amount <= 0.f) return;
    if (IsDead()) return;

    if (bIsCritical)
    {
        AActor* Owner = GetOwner();
        if (Owner && Owner->Implements<UStunnableInterface>())
        {
            IStunnableInterface::Execute_ApplyStun(Owner, 2.0f);
        }
    }

    Health = FMath::Clamp(Health - Amount, 0.f, MaxHealth);

    OnHealthChanged.Broadcast(this, Health, MaxHealth, -Amount);

    if (GetWorld())
    {
        LastDamageTime = GetWorld()->GetTimeSeconds();
    }

    if (IsDead() && !bHasDied)
    {
        bHasDied = true;
        AActor* Owner = GetOwner();
        OnDeath.Broadcast(Owner);
    }
}

void UHealthComponent::Heal(float Amount)
{
    if (!GetOwner() || !GetOwner()->HasAuthority()) return;
    if (Amount <= 0.f) return;
    if (IsDead()) return;

    Health = FMath::Clamp(Health + Amount, 0.f, MaxHealth);
    OnHealthChanged.Broadcast(this, Health, MaxHealth, Amount);
}

void UHealthComponent::UpdateMaxHealth(float NewMaxHealth)
{
    if (!GetOwner() || !GetOwner()->HasAuthority()) return;

    float HealthDifference = NewMaxHealth - MaxHealth;
    MaxHealth = NewMaxHealth;
    Health += HealthDifference;
    Health = FMath::Clamp(Health, 0.f, MaxHealth);

    OnHealthChanged.Broadcast(this, Health, MaxHealth, HealthDifference);
}

void UHealthComponent::RegenTick()
{
    if (!GetOwner() || !GetOwner()->HasAuthority()) return;
    if (!GetWorld()) return;

    const float Now = GetWorld()->GetTimeSeconds();
    const float SinceLastDamage = Now - LastDamageTime;

    if (!bAutoRegen || IsDead()) return;

    if (SinceLastDamage >= RegenDelaySeconds)
    {
        const float Amount = RegenPerSecond * RegenTickInterval;

        if (Health >= MaxHealth) return;

        Heal(Amount);
    }
}

void UHealthComponent::InitializeAfterSpawn(bool bEnableAutoRegen, float InRegenDelaySeconds, float InRegenPerSecond, float InRegenTickInterval)
{
    if (!GetOwner() || !GetOwner()->HasAuthority()) return;

    UWorld* W = GetWorld();
    const float Now = (W ? W->GetTimeSeconds() : 0.f);

    Health = FMath::Clamp(Health, 0.f, MaxHealth);

    LastDamageTime = Now;
    if (InRegenDelaySeconds >= 0.f) RegenDelaySeconds = InRegenDelaySeconds;
    if (InRegenPerSecond >= 0.f) RegenPerSecond = InRegenPerSecond;
    if (InRegenTickInterval >= 0.f) RegenTickInterval = InRegenTickInterval;

    bAutoRegen = bEnableAutoRegen;
    if (W)
    {
        W->GetTimerManager().ClearTimer(RegenTimerHandle);
        if (bAutoRegen && RegenTickInterval > 0.f)
        {
            W->GetTimerManager().SetTimer(RegenTimerHandle, this, &UHealthComponent::RegenTick, RegenTickInterval, true);
        }
    }
}

bool UHealthComponent::IsRegenTimerActive() const{
    if (!GetWorld()) return false;
    return GetWorld()->GetTimerManager().IsTimerActive(RegenTimerHandle);
}

FString UHealthComponent::GetOwnerTypeLabel() const
{
    if (!GetOwner()) return TEXT("Desconocido");

    if (GetOwner()->IsA(AEnemyBase::StaticClass()))
    {
        return TEXT("Enemigo");
    }
    else if (GetOwner()->IsA(ACharacter::StaticClass()))
    {
        return TEXT("Jugador");
    }

    return TEXT("Otro");
}