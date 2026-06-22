#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DeathInterface.generated.h"

UINTERFACE(MinimalAPI)
class UDeathInterface : public UInterface
{
	GENERATED_BODY()
};

class CELESTIA_API IDeathInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void Die();
};
