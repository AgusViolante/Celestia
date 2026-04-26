

#pragma once

#include "CoreMinimal.h"
#include "EnemyTypes.generated.h" 

UENUM(BlueprintType)
enum class EEnemyClassType : uint8
{
    Melee   UMETA(DisplayName = "Melee"),
    Ranged  UMETA(DisplayName = "Ranged"),
    Boss    UMETA(DisplayName = "Boss"),
    Tank    UMETA(DisplayName = "Tank")
};