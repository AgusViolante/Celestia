#include "Characters/RangedEnemy/MagicProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/HealthComponent.h"
#include "Engine/DamageEvents.h"

AMagicProjectile::AMagicProjectile()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true);

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(15.0f);

    CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
    CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
    CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

    CollisionComp->BodyInstance.bUseCCD = true;
    RootComponent = CollisionComp;

    CollisionComp->OnComponentHit.AddDynamic(this, &AMagicProjectile::OnHit);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = 1000.f;
    ProjectileMovement->MaxSpeed = 2000.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.0f;

    InitialLifeSpan = 3.0f;
}

void AMagicProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (HasAuthority() && OtherActor && OtherActor != this && OtherActor != GetInstigator())
    {
        FDamageEvent DamageEvent;
        OtherActor->TakeDamage(DamageAmount, DamageEvent, GetInstigatorController(), this);
    }

    Destroy();
}