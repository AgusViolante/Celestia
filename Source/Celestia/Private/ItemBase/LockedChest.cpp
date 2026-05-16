// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemBase/LockedChest.h"

ALockedChest::ALockedChest()
{
	RequiredKeyName = TEXT("LlaveDorada");
}

void ALockedChest::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority()) return;
	if (bIsLooted) return;

	if (Interactor)
	{
		if (CheckPlayerHasKey(Interactor, RequiredKeyName))
		{
			ConsumePlayerKey(Interactor, RequiredKeyName);
			Super::Interact_Implementation(Interactor);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Se necesita la llave: %s"), *RequiredKeyName);
		}
	}
}
