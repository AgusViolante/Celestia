// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemBase/Pocion.h"
#include "Components/BoxComponent.h"
#include "Interfaces/I_PickUp.h"

// Sets default values
APocion::APocion()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CubeCollision"));
	RootComponent = BoxCollision;

	CubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeMesh"));
	CubeMesh->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void APocion::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APocion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APocion::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (OtherActor && OtherActor->GetClass()->ImplementsInterface(UI_PickUp::StaticClass()))
	{
		II_PickUp::Execute_AddPotion(OtherActor, SumaPotion);
		Destroy();
	}
}

