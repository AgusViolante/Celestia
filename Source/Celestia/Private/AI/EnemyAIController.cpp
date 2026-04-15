// Fill out your copyright notice in the Description page of Project Settings.
#include "AI/EnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "../CelestiaCharacter.h" 
#include "Characters/EnemyTypes.h"
#include "Characters/EnemyBase.h"

AEnemyAIController::AEnemyAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Componente de percepción y la configuración de vista
    EnemyPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("EnemyPerceptionComp"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

    // qué tan lejos ve el enemigo
    SightConfig->SightRadius = 800.f; // Distancia de visión
    SightConfig->LoseSightRadius = 1000.f; // Distancia a la que te pierde de vista
    SightConfig->PeripheralVisionAngleDegrees = 60.f; // Qué tan amplio es su cono de visión
    SightConfig->SetMaxAge(5.0f); // Cuánto tiempo recuerda al objetivo después de perderlo

    // qué cosas puede detectar (en este caso, todo)
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    // 4. Asignamos la vista al componente principal
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
        if (Stimulus.WasSuccessfullySensed())
        {
            if (GetBlackboardComponent())
            {
                GetBlackboardComponent()->SetValueAsObject(FName("TargetActor"), Player);
            }
            UE_LOG(LogTemp, Warning, TEXT("El enemigo vio al jugador!"));
        }
        else
        {
            
            if (GetBlackboardComponent())
            {
                GetBlackboardComponent()->ClearValue(FName("TargetActor"));
            }
            UE_LOG(LogTemp, Warning, TEXT("El enemigo perdio al jugador!"));
        }
    }
}