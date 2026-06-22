#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProgressionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnXPGainedSignature, float, CurrentXP, float, MaxXP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUpSignature, int32, NewLevel);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CELESTIA_API UProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UProgressionComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	void UpdateMaxXP();
	void LevelUp();

public:
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void AddXP(float Amount);

	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnXPGainedSignature OnXPGained;

	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnLevelUpSignature OnLevelUp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentLevel, Category = "Progression|State")
	int32 CurrentLevel = 1;

	UFUNCTION()
	void OnRep_CurrentLevel(int32 OldLevel);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentXP, Category = "Progression|State")
	float CurrentXP = 0.0f;

	UFUNCTION()
	void OnRep_CurrentXP();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_MaxXP, Category = "Progression|State")
	float MaxXPForNextLevel;

	UFUNCTION()
	void OnRep_MaxXP();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression|Balance")
	float BaseXP = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression|Balance")
	float XPExponent = 1.5f;

	UFUNCTION(BlueprintCallable, Category = "Progression")
	void ForceLevelUp(int32 LevelsToGrant);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Progression | Fast Travel")
	TArray<FName> UnlockedPortals;

	void UnlockPortal(FName NewPortalID);
	bool IsPortalUnlocked(FName PortalIDToCheck) const;
};