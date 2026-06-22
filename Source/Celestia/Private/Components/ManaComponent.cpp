#include "Components/ManaComponent.h"
#include "Components/ProgressionComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Net/UnrealNetwork.h"

UManaComponent::UManaComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	MaxMana = 100.0f;
	CurrentMana = MaxMana;
}

void UManaComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UManaComponent, CurrentMana);
	DOREPLIFETIME(UManaComponent, MaxMana);
}

void UManaComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		CurrentMana = MaxMana;
	}
}

void UManaComponent::OnRep_CurrentMana()
{
	OnManaChanged.Broadcast(CurrentMana, MaxMana);
}

void UManaComponent::OnRep_MaxMana()
{
	OnManaChanged.Broadcast(CurrentMana, MaxMana);
}

float UManaComponent::CalculateDynamicCost(float BaseCost, float Multiplier) const
{
	float FinalCost = BaseCost;

	if (AActor* MyOwner = GetOwner())
	{
		if (UProgressionComponent* ProgComp = MyOwner->FindComponentByClass<UProgressionComponent>())
		{
			int32 PlayerLevel = ProgComp->CurrentLevel;
			FinalCost = BaseCost + (PlayerLevel * Multiplier);
		}
	}

	return FinalCost;
}

bool UManaComponent::TryConsumeDynamicMana(float BaseCost, float LevelMultiplier)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return false;

	float ActualCost = CalculateDynamicCost(BaseCost, LevelMultiplier);

	if (CurrentMana >= ActualCost)
	{
		CurrentMana -= ActualCost;
		OnManaChanged.Broadcast(CurrentMana, MaxMana);
		return true;
	}

	return false;
}

void UManaComponent::RestoreMana(float Amount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	CurrentMana = FMath::Clamp(CurrentMana + Amount, 0.0f, MaxMana);
	OnManaChanged.Broadcast(CurrentMana, MaxMana);
}

void UManaComponent::UpdateMaxMana(float NewMaxMana)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	float ManaDifference = NewMaxMana - MaxMana;
	MaxMana = NewMaxMana;

	if (ManaDifference > 0.0f)
	{
		CurrentMana += ManaDifference;
	}

	CurrentMana = FMath::Clamp(CurrentMana, 0.0f, MaxMana);
	OnManaChanged.Broadcast(CurrentMana, MaxMana);
}

bool UManaComponent::TryRestoreMana(float AmountToRestore)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return false;

	if (FMath::IsNearlyEqual(CurrentMana, MaxMana))
	{
		return false;
	}

	RestoreMana(AmountToRestore);
	return true;
}