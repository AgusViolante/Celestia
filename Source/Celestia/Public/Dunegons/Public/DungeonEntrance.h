// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonEntrance.generated.h"

class UBoxComponent;
class UUserWidget; 

UCLASS()
class CELESTIA_API ADungeonEntrance : public AActor
{
	GENERATED_BODY()

public:
	ADungeonEntrance();

	UPROPERTY(EditAnywhere, Category = "Dungeon Setup")
	FName DungeonID;

	UPROPERTY(EditAnywhere, Category = "Dungeon Setup")
	FName MapName;

	// Ahora usamos un UUserWidget estándar
	UPROPERTY(EditDefaultsOnly, Category = "Dungeon Setup | UI")
	TSubclassOf<UUserWidget> PromptWidgetClass;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* TriggerZone;

	UPROPERTY()
	UUserWidget* PromptInstance;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Función que ejecutará el viaje
	UFUNCTION()
	void InteractToEnter();

private:
	bool HasDungeonQuest(AActor* PlayerActor);
};