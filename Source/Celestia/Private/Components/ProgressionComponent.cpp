#include "Components/ProgressionComponent.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"
#include "Net/UnrealNetwork.h"

UProgressionComponent::UProgressionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	CurrentLevel = 1;
	CurrentXP = 0.0f;
}

void UProgressionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UProgressionComponent, CurrentLevel);
	DOREPLIFETIME(UProgressionComponent, CurrentXP);
	DOREPLIFETIME(UProgressionComponent, MaxXPForNextLevel);
	DOREPLIFETIME(UProgressionComponent, UnlockedPortals);
}

void UProgressionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		UpdateMaxXP();
	}
}

void UProgressionComponent::OnRep_CurrentLevel(int32 OldLevel)
{
	if (CurrentLevel > OldLevel)
	{
		OnLevelUp.Broadcast(CurrentLevel);
	}
}

void UProgressionComponent::OnRep_CurrentXP()
{
	OnXPGained.Broadcast(CurrentXP, MaxXPForNextLevel);
}

void UProgressionComponent::OnRep_MaxXP()
{
	OnXPGained.Broadcast(CurrentXP, MaxXPForNextLevel);
}

void UProgressionComponent::AddXP(float Amount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	if (Amount <= 0.0f) return;

	CurrentXP += Amount;

	while (MaxXPForNextLevel > 0.0f && CurrentXP >= MaxXPForNextLevel)
	{
		CurrentXP -= MaxXPForNextLevel;
		CurrentLevel++;
		UpdateMaxXP();
		LevelUp();
	}

	OnXPGained.Broadcast(CurrentXP, MaxXPForNextLevel);
}

void UProgressionComponent::LevelUp()
{
	OnLevelUp.Broadcast(CurrentLevel);
}

void UProgressionComponent::UpdateMaxXP()
{
	MaxXPForNextLevel = BaseXP * FMath::Pow((float)CurrentLevel, XPExponent);
}

void UProgressionComponent::UnlockPortal(FName NewPortalID)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	if (!UnlockedPortals.Contains(NewPortalID))
	{
		UnlockedPortals.Add(NewPortalID);
	}
}

bool UProgressionComponent::IsPortalUnlocked(FName PortalIDToCheck) const
{
	return UnlockedPortals.Contains(PortalIDToCheck);
}

void UProgressionComponent::ForceLevelUp(int32 LevelsToGrant)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	for (int32 i = 0; i < LevelsToGrant; i++)
	{
		float XPNeeded = MaxXPForNextLevel - CurrentXP;
		AddXP(XPNeeded);
	}
}