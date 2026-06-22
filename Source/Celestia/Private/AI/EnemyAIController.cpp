#include "AI/EnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "../CelestiaCharacter.h" 
#include "Components/ProgressionComponent.h"
#include "Characters/EnemyTypes.h"
#include "Characters/EnemyBase.h"

AEnemyAIController::AEnemyAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    EnemyPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("EnemyPerceptionComp"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

    SightConfig->SightRadius = 800.f;
    SightConfig->LoseSightRadius = 1000.f;
    SightConfig->PeripheralVisionAngleDegrees = 60.f;
    SightConfig->SetMaxAge(5.0f);

    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    EnemyPerceptionComp->ConfigureSense(*SightConfig);
    EnemyPerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    EnemyPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetDetected);

    if (AEnemyBase* Enemy = Cast<AEnemyBase>(InPawn))
    {
        if (Enemy->EnemyType == EEnemyClassType::Ranged)
        {
            SightConfig->SightRadius = 1500.f;
            SightConfig->LoseSightRadius = 2000.f;
        }
        else if (Enemy->EnemyType == EEnemyClassType::Boss)
        {
            SightConfig->SightRadius = 2500.f;
            SightConfig->LoseSightRadius = 3000.f;
            SightConfig->PeripheralVisionAngleDegrees = 180.f;
        }
        else
        {
            SightConfig->SightRadius = 800.f;
            SightConfig->LoseSightRadius = 1200.f;
        }

        EnemyPerceptionComp->ConfigureSense(*SightConfig);

        if (BehaviorTreeAsset)
        {
            RunBehaviorTree(BehaviorTreeAsset);

            if (UBlackboardComponent* BB = GetBlackboardComponent())
            {
                BB->SetValueAsEnum(FName("EnemyClass"), static_cast<uint8>(Enemy->EnemyType));
            }
        }
    }
}

void AEnemyAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    if (ACelestiaCharacter* Player = Cast<ACelestiaCharacter>(Actor))
    {
        if (AEnemyBase* ControlledEnemy = Cast<AEnemyBase>(GetPawn()))
        {
            if (UProgressionComponent* PlayerProg = Player->FindComponentByClass<UProgressionComponent>())
            {
                int32 PlayerLevel = PlayerProg->CurrentLevel;
                int32 EnemyLvl = ControlledEnemy->EnemyLevel;

                if (PlayerLevel >= EnemyLvl + 2)
                {
                    return;
                }
            }
        }

        if (Stimulus.WasSuccessfullySensed())
        {
            if (GetBlackboardComponent())
            {
                GetBlackboardComponent()->SetValueAsObject(FName("TargetActor"), Player);
            }
        }
        else
        {
            if (!bHasAggro && GetBlackboardComponent())
            {
                GetBlackboardComponent()->ClearValue(FName("TargetActor"));
            }
        }
    }
}

void AEnemyAIController::ReceiveDamageAggro(AActor* Attacker)
{
    if (Attacker && GetBlackboardComponent())
    {
        bHasAggro = true;
        GetBlackboardComponent()->SetValueAsObject(FName("TargetActor"), Attacker);
    }
}