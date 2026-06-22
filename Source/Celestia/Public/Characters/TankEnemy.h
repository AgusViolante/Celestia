#pragma once

#include "CoreMinimal.h"
#include "Characters/MeleeEnemy.h" 
#include "TankEnemy.generated.h"

UCLASS()
class CELESTIA_API ATankEnemy : public AMeleeEnemy
{
	GENERATED_BODY()

public:
	ATankEnemy();

protected:
	virtual void BeginPlay() override;
};