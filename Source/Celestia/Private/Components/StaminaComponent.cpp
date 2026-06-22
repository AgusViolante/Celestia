#include "Components/StaminaComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Net/UnrealNetwork.h"

UStaminaComponent::UStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UStaminaComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStaminaComponent, CurrentStamina);
	DOREPLIFETIME(UStaminaComponent, MaxStamina);
}

void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		CurrentStamina = MaxStamina;
	}
}

void UStaminaComponent::OnRep_CurrentStamina()
{
	OnStaminaChanged.Broadcast(this, CurrentStamina, MaxStamina);
}

void UStaminaComponent::OnRep_MaxStamina()
{
	OnStaminaChanged.Broadcast(this, CurrentStamina, MaxStamina);
}

void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

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
	else if (CurrentStamina < MaxStamina && bCanRegen)
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
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	if (CurrentStamina > 0.f)
	{
		bIsDraining = true;
		CurrentDrainRate = DrainRate;
	}
}

void UStaminaComponent::StopDraining()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	bIsDraining = false;
	CurrentDrainRate = 0.f;
}

bool UStaminaComponent::HasEnoughStamina(float MinimumRequired) const
{
	return CurrentStamina >= MinimumRequired;
}