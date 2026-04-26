#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Interfaces/StunnableInterface.h"
#include "Interfaces/DeathInterface.h"
#include "Characters/EnemyBase.h"
#include "../../CelestiaCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h" 
#include "TimerManager.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    MaxHealth = 100.f;
    Health = MaxHealth;

}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    Health = FMath::Clamp(Health, 0.f, MaxHealth);

  
    if (GetWorld())
    {
        LastDamageTime = GetWorld()->GetTimeSeconds();
    }
    else
    {
        LastDamageTime = 0.f;
    }

  
    if (bAutoRegen && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(RegenTimerHandle, this, &UHealthComponent::RegenTick, RegenTickInterval, true);
    }

 
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan,
            FString::Printf(TEXT("Vida inicial: %.2f / %.2f"), Health, MaxHealth));
    }

    InitializeAfterSpawn(bAutoRegen, RegenDelaySeconds, RegenPerSecond, RegenTickInterval);
}

void UHealthComponent::TakeDamage(float Amount, bool bIsCritical, bool bIgnoreDefense)
{
    if (Amount <= 0.f) return;
    if (IsDead()) return;

    float FinalDamage = Amount;

    // Si no ignoramos la defensa, calculamos la mitigación
    if (!bIgnoreDefense)
    {
        if (AActor* OwnerActor = GetOwner())
        {
            // Buscamos si el que recibe daño tiene Stats (el jugador lo tiene, los enemigos normales tal vez no)
            if (UStatsComponent* StatsComp = OwnerActor->FindComponentByClass<UStatsComponent>())
            {
                // Por ahora usamos MeleeDefense como defensa general
                float Defense = StatsComp->GetStatValue(ERPGStatType::MeleeDefense);

                // Evitamos divisiones por cero o números negativos en caso de bugs
                Defense = FMath::Max(0.0f, Defense);

                // Fórmula: Multiplicador porcentual
                float MitigationMultiplier = 100.0f / (100.0f + Defense);
                FinalDamage = Amount * MitigationMultiplier;
            }
        }
    }

    if (bIsCritical)
    {
        AActor* Owner = GetOwner();
        

        if (Owner && Owner->Implements<UStunnableInterface>())
        {
     
            IStunnableInterface::Execute_ApplyStun(Owner, 2.0f);
        }
    }
    Health = FMath::Clamp(Health - FinalDamage, 0.f, MaxHealth);

    OnHealthChanged.Broadcast(this, Health, MaxHealth, -FinalDamage);

    if (GetWorld())
    {
        LastDamageTime = GetWorld()->GetTimeSeconds();
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Red,
            FString::Printf(TEXT("Recibio danio: -%.2f (Danio bruto: %.2f) | Vida: %.2f"), FinalDamage, Amount, Health));
    }

    if (IsDead())
    {
        AActor* Owner = GetOwner();
        OnDeath.Broadcast(Owner);

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Black,
                FString::Printf(TEXT("Murio (owner)")));
        }
    }
}
void UHealthComponent::Heal(float Amount)
{
    if (Amount <= 0.f) return;
    if (IsDead()) return;

    const float Old = Health;
    Health = FMath::Clamp(Health + Amount, 0.f, MaxHealth);

    OnHealthChanged.Broadcast(this, Health, MaxHealth, Amount);

    FString Tipo = GetOwnerTypeLabel();

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Green,
            FString::Printf(TEXT("%s curado: +%.2f | Vida actual: %.2f"), *Tipo, Amount, Health));
    }
}

void UHealthComponent::RegenTick()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("RegenTick: no World"));
        return;
    }

    const float Now = GetWorld()->GetTimeSeconds();
    const float SinceLastDamage = Now - LastDamageTime;
    const bool bTimerActive = GetWorld()->GetTimerManager().IsTimerActive(RegenTimerHandle);


    if (!bAutoRegen || IsDead()) return;

    if (SinceLastDamage >= RegenDelaySeconds)
    {
        const float Amount = RegenPerSecond * RegenTickInterval;

        if (Health >= MaxHealth) return;

        Heal(Amount);

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Blue,
                FString::Printf(TEXT("Auto-regeneracion: +%.2f | Vida actual: %.2f"), Amount, Health));
        }
    }
}

void UHealthComponent::InitializeAfterSpawn(bool bEnableAutoRegen, float InRegenDelaySeconds, float InRegenPerSecond, float InRegenTickInterval)
{
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
bool UHealthComponent::IsRegenTimerActive() const
{
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
    

