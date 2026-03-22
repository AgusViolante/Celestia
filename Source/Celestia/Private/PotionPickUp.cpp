// Fill out your copyright notice in the Description page of Project Settings.


#include "PotionPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "../CelestiaCharacter.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

APotionPickup::APotionPickup()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetSphereRadius(50.f);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
    CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    CollisionSphere->SetGenerateOverlapEvents(true);
    RootComponent = CollisionSphere;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &APotionPickup::OnOverlapBegin);

    bCollected = false;
}

void APotionPickup::BeginPlay()
{
    Super::BeginPlay();

}

void APotionPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (bCollected) return;
    if (!OtherActor || OtherActor == this) return;

  
    ACelestiaCharacter* Player = Cast<ACelestiaCharacter>(OtherActor);
    if (!Player) return; 

    bCollected = true;
    SetActorEnableCollision(false);

    Player->AddPotion(Amount);

    Destroy();
}