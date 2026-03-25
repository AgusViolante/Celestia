// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemBase/ItemBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
	RootComponent = BoxCollision;
	BoxCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	CubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube Mesh"));
	CubeMesh->SetupAttachment(RootComponent);
	CubeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AItemBase::Interact_Implementation(AActor* Interactor)
{
	if (Interactor && Interactor->GetClass()->ImplementsInterface(UI_PickUp::StaticClass()))
	{
		// Le enviamos nuestros datos al jugador
		II_PickUp::Execute_ReceiveItem(Interactor, Amount, ItemName);

		// Nos destruimos del mundo
		Destroy();
	}
}



