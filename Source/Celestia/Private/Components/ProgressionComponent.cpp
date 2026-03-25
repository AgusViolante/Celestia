#include "Components/ProgressionComponent.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h" 

UProgressionComponent::UProgressionComponent()
{
	PrimaryComponentTick.bCanEverTick = false; 

	CurrentLevel = 1;
	CurrentXP = 0.0f;
}

void UProgressionComponent::BeginPlay()
{
	Super::BeginPlay();

	
	UpdateMaxXP();
}

void UProgressionComponent::AddXP(float Amount)
{
	if (Amount <= 0.0f) return;

	CurrentXP += Amount;
	bool bDidLevelUp = false;

	
	while (MaxXPForNextLevel > 0.0f && CurrentXP >= MaxXPForNextLevel)
	{
		CurrentXP -= MaxXPForNextLevel; 
		CurrentLevel++;                 

		UpdateMaxXP();                  
		bDidLevelUp = true;
	}

	
	if (bDidLevelUp)
	{
		LevelUp();
	}


	OnXPGained.Broadcast(CurrentXP, MaxXPForNextLevel);
}

void UProgressionComponent::LevelUp()
{

	OnLevelUp.Broadcast(CurrentLevel);

	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("LEVEL UP! Nivel %d"), CurrentLevel));
	}
}

void UProgressionComponent::UpdateMaxXP()
{

	MaxXPForNextLevel = BaseXP * FMath::Pow((float)CurrentLevel, XPExponent);
}
