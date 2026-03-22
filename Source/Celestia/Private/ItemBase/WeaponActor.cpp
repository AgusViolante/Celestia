// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemBase/WeaponActor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Components/HealthComponent.h"

// Sets default values
AWeaponActor::AWeaponActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;

	OwnerCharacter = nullptr;
}

// Called when the game starts or when spawned
void AWeaponActor::BeginPlay()
{
    Super::BeginPlay();
}

void AWeaponActor::ResetAlreadyHit()
{
    AlreadyHit.Empty();
}

float AWeaponActor::CalculateFinalDamage(AActor* Target, bool bIsAbility, float AbilityMultiplier) const
{
    float charBase = 0.f;
    float charStrMult = 1.f;

    if (OwnerCharacter)
    {
    
    }

    float weaponBase = WeaponStats.BaseDamage;
    float wMult = WeaponStats.DamageMultiplier;
    float abilityMult = bIsAbility ? AbilityMultiplier : 1.f;

    float dmg = (charBase + weaponBase) * charStrMult * wMult * abilityMult;
    return FMath::Max(0.f, dmg);
}

void AWeaponActor::PerformHit(bool bIsAbility, float AbilityMultiplier)
{
    if (!OwnerCharacter || !GetWorld()) return;

    const FVector Start = OwnerCharacter->GetMesh()->GetSocketLocation(AttachSocketName);
    const FVector Forward = OwnerCharacter->GetActorForwardVector();
    const FVector End = Start + Forward * WeaponStats.Range;

    FCollisionShape Sphere = FCollisionShape::MakeSphere(WeaponStats.Radius);
    TArray<FHitResult> HitResults;

    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        Start,
        End,
        FQuat::Identity,
        ECC_Pawn,
        Sphere
    );

     DrawDebugSphere(GetWorld(), End, WeaponStats.Radius, 12, FColor::Red, false, 1.0f);

    if (!bHit) return;

    for (const FHitResult& Hit : HitResults)
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor || HitActor == OwnerCharacter) continue;

        if (AlreadyHit.Contains(HitActor)) continue;
        AlreadyHit.Add(HitActor);

        float finalDamage = CalculateFinalDamage(HitActor, bIsAbility, AbilityMultiplier);

        bool bApplied = false;
        if (HitActor)
        {
          
            if (UActorComponent* comp = HitActor->FindComponentByClass(UActorComponent::StaticClass()))
            {
               
                if (auto* HC = HitActor->FindComponentByClass<UHealthComponent>())
                {
                    HC->TakeDamage(finalDamage); 
                    bApplied = true;
                }
            }
        }

        if (!bApplied)
        {
            UGameplayStatics::ApplyDamage(HitActor, finalDamage, OwnerCharacter->GetController(), this, nullptr);
        }

    }
}