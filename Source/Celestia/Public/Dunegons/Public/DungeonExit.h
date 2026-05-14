// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonExit.generated.h"

class UBoxComponent;
class UUserWidget;

UCLASS()
class CELESTIA_API ADungeonExit : public AActor
{
	GENERATED_BODY()

public:
	ADungeonExit();

	// Identificador de la mazmorra para verificar la misión
	UPROPERTY(EditAnywhere, Category = "Exit Setup")
	FName DungeonID;

	UPROPERTY(EditAnywhere, Category = "Exit Setup")
	AActor* TeleportDestination;

	UPROPERTY(EditDefaultsOnly, Category = "Exit Setup | UI")
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

	UFUNCTION()
	void InteractToExit();

private:
	// Nueva función para verificar si se completó
	bool IsDungeonQuestComplete(AActor* PlayerActor);
};