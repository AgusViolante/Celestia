#include "EnemyAnimInstance.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    APawn* OwningPawn = TryGetPawnOwner();
    if (!OwningPawn)
    {
        Speed = 0.f;
        bIsMoving = false;
        return;
    }

    
    FVector Velocity = OwningPawn->GetVelocity();
    Velocity.Z = 0.f;
    Speed = Velocity.Size();


    const float MovingThreshold = 10.f;
    bIsMoving = Speed > MovingThreshold;
}