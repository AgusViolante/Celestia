#include "Animation/Notifies/AnimNotify_FireMagic.h"
#include "Characters/RangedEnemy/RangedEnemy.h"

void UAnimNotify_FireMagic::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (ARangedEnemy* RangedEnemy = Cast<ARangedEnemy>(MeshComp->GetOwner()))
		{
			RangedEnemy->FireMagic();
		}
	}
}