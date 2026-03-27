// Fill out your copyright notice in the Description page of Project Settings.
#include "Components/ManaComponent.h"
#include "Math/UnrealMathUtility.h"

UManaComponent::UManaComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	MaxMana = 50.0f;
	CurrentMana = MaxMana;
}

void UManaComponent::BeginPlay()
{
	Super::BeginPlay();
	// Eliminamos el SetTimer, ya no hay regeneración pasiva
}

bool UManaComponent::TryConsumeMana(float Cost)
{
	if (CurrentMana >= Cost)
	{
		CurrentMana -= Cost;
		OnManaChanged.Broadcast(CurrentMana, MaxMana);
		return true;
	}

	return false;
}

void UManaComponent::RestoreMana(float Amount)
{
	CurrentMana = FMath::Clamp(CurrentMana + Amount, 0.0f, MaxMana);
	OnManaChanged.Broadcast(CurrentMana, MaxMana);
}

void UManaComponent::UpdateMaxMana(float NewMaxMana)
{
	// 1. Calculamos cuánta maná extra estamos ganando por la Inteligencia
	float ManaDifference = NewMaxMana - MaxMana;

	// 2. Actualizamos el límite
	MaxMana = NewMaxMana;

	// 3. Le sumamos esa misma diferencia a la maná actual para dar el salto instantáneo
	if (ManaDifference > 0.0f)
	{
		CurrentMana += ManaDifference;
	}

	// Aseguramos que no se pase del límite
	CurrentMana = FMath::Clamp(CurrentMana, 0.0f, MaxMana);

	// 4. Disparamos el evento para que la barra azul de la UI se actualice de golpe
	OnManaChanged.Broadcast(CurrentMana, MaxMana);
}

bool UManaComponent::TryRestoreMana(float AmountToRestore)
{
	
	if (FMath::IsNearlyEqual(CurrentMana, MaxMana))
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Mana lleno. Pocion cancelada."));
		return false;
	}

	
	RestoreMana(AmountToRestore);
	return true;
}