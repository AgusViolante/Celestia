// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/RangedEnemy/MagicProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/HealthComponent.h"

// Sets default values
AMagicProjectile::AMagicProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(15.0f);

    CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
    CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
    CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore); // Ignoramos todo por defecto
    CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block); // Bloqueamos al jugador/enemigos
    CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block); // Bloqueamos paredes/pisos

    CollisionComp->BodyInstance.bUseCCD = true;
    RootComponent = CollisionComp;


    CollisionComp->BodyInstance.bUseCCD = true;
    RootComponent = CollisionComp;

    CollisionComp->OnComponentHit.AddDynamic(this, &AMagicProjectile::OnHit);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = 1000.f; // 1000 era un poco lento, 2000 está bien.
    ProjectileMovement->MaxSpeed = 2000.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.0f; 

    InitialLifeSpan = 3.0f;
}

void AMagicProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

    UE_LOG(LogTemp, Warning, TEXT("OnHit disparado! Choco contra: %s"), OtherActor ? *OtherActor->GetName() : TEXT("Nulo"));

    if (OtherActor && OtherActor != this && OtherActor != GetInstigator())
    {
      
        if (UHealthComponent* TargetHealthComp = OtherActor->FindComponentByClass<UHealthComponent>())
        {
            TargetHealthComp->TakeDamage(DamageAmount, false);

            if (OtherActor->ActorHasTag("Player"))
            {
                UE_LOG(LogTemp, Warning, TEXT("¡Magia impactó al jugador!"));
            }
        }
    }

    Destroy();
}
