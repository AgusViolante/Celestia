#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h" 
#include "EnemyAIController.generated.h"

class UBehaviorTree;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UCLASS()
class CELESTIA_API AEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    AEnemyAIController();

    UFUNCTION(BlueprintCallable, Category = "AI | Logic")
    void ReceiveDamageAggro(AActor* Attacker);

protected:
    virtual void OnPossess(APawn* InPawn) override;

    UPROPERTY(EditDefaultsOnly, Category = "AI | Logic")
    TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI | Perception")
    TObjectPtr<UAIPerceptionComponent> EnemyPerceptionComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI | Perception")
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    UFUNCTION()
    void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI | Logic")
    bool bHasAggro = false;
};