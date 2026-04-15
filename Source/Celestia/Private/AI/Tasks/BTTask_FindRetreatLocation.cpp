// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_FindRetreatLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NavigationSystem.h" 
#include "GameFramework/Character.h"


UBTTask_FindRetreatLocation::UBTTask_FindRetreatLocation()
{
	NodeName = TEXT("Find Retreat Location");
}

EBTNodeResult::Type UBTTask_FindRetreatLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!AIController || !Blackboard) return EBTNodeResult::Failed;

	APawn* AIPawn = AIController->GetPawn();
	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject("TargetActor"));

	if (!AIPawn || !TargetActor) return EBTNodeResult::Failed;

	//Calculamos la distancia entre el enemigo y el jugador
	float DistanceToTarget = FVector::Dist(AIPawn->GetActorLocation(), TargetActor->GetActorLocation());

	// Si el jugador no esta cerca, fallamos para ir a la secuencia de ataque
	if (DistanceToTarget > MinimumDistanceToRetreat)
	{
		return EBTNodeResult::Failed;
	}

	// El jugador está muy cerca. Calculamos un vector que apunte hacia el lado contrario
	FVector DirectionAwayFromTarget = AIPawn->GetActorLocation() - TargetActor->GetActorLocation();
	DirectionAwayFromTarget.Z = 0.f; // Para que no intente volar o enterrarse
	DirectionAwayFromTarget.Normalize();

	FVector TargetRetreatLocation = AIPawn->GetActorLocation() + (DirectionAwayFromTarget * RetreatDistance);

	// Verificamos que ese punto hacia atrás sea pisable en el NavMesh
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSystem)
	{
		FNavLocation ValidRetreatLocation;
		// Buscamos el punto navegable más cercano a nuestra ubicación de escape calculada
		if (NavSystem->ProjectPointToNavigation(TargetRetreatLocation, ValidRetreatLocation, FVector(500.f, 500.f, 500.f)))
		{
			// Guardamos el punto en el Blackboard
			Blackboard->SetValueAsVector("RetreatLocation", ValidRetreatLocation.Location);
			return EBTNodeResult::Succeeded;
		}
	}

	// Si está acorralado contra la pared y no hay NavMesh detrás, falla y dispara de cerca
	return EBTNodeResult::Failed;
}
