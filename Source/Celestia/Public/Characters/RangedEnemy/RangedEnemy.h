#pragma once

#include "CoreMinimal.h"
#include "Characters/EnemyBase.h"
#include "RangedEnemy.generated.h"

class AMagicProjectile;
class UAnimMontage;

UCLASS()
class CELESTIA_API ARangedEnemy : public AEnemyBase
{
	GENERATED_BODY()

public:
	ARangedEnemy();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat | Magic")
	TSubclassOf<AMagicProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Combat | Magic")
	FName MuzzleSocketName = TEXT("RightHandSocket");

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformRangedAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void FireMagic();

protected:
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayRangedMontage();
};