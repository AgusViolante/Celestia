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

    // Conectamos nuestros "ojos" a la función que escribimos
    EnemyPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetDetected);

    // Si le asignamos un Behavior Tree en el editor, lo arrancamos
    if (BehaviorTreeAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);
        if (AEnemyBase* Enemy = Cast<AEnemyBase>(InPawn))
        {
            if (UBlackboardComponent* BB = GetBlackboardComponent())
            {
                BB->SetValueAsEnum(FName("EnemyClass"), static_cast<uint8>(Enemy->EnemyType));
            }
        }

    }
}

void AEnemyAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    // Solo nos importa si lo que vimos es el Jugador
    if (ACelestiaCharacter* Player = Cast<ACelestiaCharacter>(Actor))
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            // EL ENEMIGO NOS VIO
            // Aquí le avisaremos al Blackboard en el futuro
            if (GetBlackboardComponent())
            {
                GetBlackboardComponent()->SetValueAsObject(FName("TargetActor"), Player);
            }
            UE_LOG(LogTemp, Warning, TEXT("El enemigo vio al jugador!"));
        }
        else
        {
            // EL ENEMIGO NOS PERDIÓ DE VISTA
            if (GetBlackboardComponent())
            {
                GetBlackboardComponent()->ClearValue(FName("TargetActor"));
            }
            UE_LOG(LogTemp, Warning, TEXT("El enemigo perdio al jugador!"));
        }
    }
}