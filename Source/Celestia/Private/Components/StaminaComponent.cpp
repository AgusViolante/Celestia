#include "Components/StaminaComponent.h"
#include "Math/UnrealMathUtility.h"

UStaminaComponent::UStaminaComponent()
{
	// Usamos Tick para un drenaje y regeneración fluidos
	PrimaryComponentTick.bCanEverTick = true;
	CurrentStamina = MaxStamina;
}

void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentStamina = MaxStamina;
}

void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const float OldStamina = CurrentStamina;

	if (bIsDraining)
	{
		CurrentStamina -= CurrentDrainRate * DeltaTime;
		CurrentStamina = FMath::Max(0.f, CurrentStamina);

		if (CurrentStamina <= 0.f)
		{
			StopDraining();
			OnStaminaExhausted.Broadcast();
		}
	}
	else if (CurrentStamina < MaxStamina)
	{
		CurrentStamina += RegenPerSecond * DeltaTime;
		CurrentStamina = FMath::Min(MaxStamina, CurrentStamina);
	}

	
	if (!FMath::IsNearlyEqual(OldStamina, CurrentStamina))
	{
		OnStaminaChanged.Broadcast(this, CurrentStamina, MaxStamina);
	}
}

void UStaminaComponent::StartDraining(float DrainRate)
{
	if (CurrentStamina > 0.f)
	{
		bIsDraining = true;
		CurrentDrainRate = DrainRate;
	}
}

void UStaminaComponent::StopDraining()
{
	bIsDraining = false;
	CurrentDrainRate = 0.f;
}

bool UStaminaComponent::HasEnoughStamina(float MinimumRequired) const
{
	return CurrentStamina >= MinimumRequired;
}