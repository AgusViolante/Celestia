#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/I_PickUp.h"
#include "DungeonEntrance.generated.h"

class UBoxComponent;
class UUserWidget;

UCLASS()
class CELESTIA_API ADungeonEntrance : public AActor, public II_PickUp
{
	GENERATED_BODY()

public:
	ADungeonEntrance();

	UPROPERTY(EditAnywhere, Category = "Dungeon Setup")
	FName DungeonID;

	UPROPERTY(EditAnywhere, Category = "Dungeon Setup")
	TObjectPtr<AActor> TeleportDestination;

	UPROPERTY(EditDefaultsOnly, Category = "Dungeon Setup | UI")
	TSubclassOf<UUserWidget> PromptWidgetClass;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerZone;

	UPROPERTY()
	TObjectPtr<UUserWidget> PromptInstance;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	bool HasDungeonQuest(AActor* PlayerActor);

public:
	virtual void Interact_Implementation(AActor* Interactor) override;
};