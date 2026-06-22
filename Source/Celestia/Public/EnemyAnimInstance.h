#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyAnimInstance.generated.h"

UCLASS()
class CELESTIA_API UEnemyAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    float Speed;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    bool bIsMoving;
};