#include "ItemBase/LockedChest.h"

ALockedChest::ALockedChest()
{
	RequiredKeyName = TEXT("LlaveDorada");
}

void ALockedChest::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority() || bIsLooted) return;

	if (Interactor)
	{
		if (CheckPlayerHasKey(Interactor, RequiredKeyName))
		{
			ConsumePlayerKey(Interactor, RequiredKeyName);
			Super::Interact_Implementation(Interactor);
		}
	}
}