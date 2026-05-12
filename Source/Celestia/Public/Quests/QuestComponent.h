// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestsDataAsset.h"
#include "QuestComponent.generated.h"


USTRUCT(BlueprintType)
struct FActiveQuest
{
	GENERATED_BODY()

	// La misión original (lectura)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	UQuestDataAsset* QuestData;

	// El progreso actual de los objetivos. ¡Esto es lo que muta!
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	TArray<FQuestObjective> CurrentObjectives;

	// Estado simple de la misión
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bIsReadyToTurnIn;

	FActiveQuest() : QuestData(nullptr), bIsReadyToTurnIn(false) {}
	FActiveQuest(UQuestDataAsset* InQuestData) : QuestData(InQuestData), bIsReadyToTurnIn(false) {}
};

// Delegados para que la UI se entere de los cambios sin estar leyendo todo el tiempo
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestListUpdatedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveUpdatedSignature, const FActiveQuest&, Quest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestTrackedSignature, const FActiveQuest&, TrackedQuest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestRewardsGrantedSignature, int32, CoinsToGive, const TArray<FItemReward>&, ItemsToGive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestItemsConsumedSignature, const TArray<FQuestObjective>&, ConsumedObjectives);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestUntrackedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestAcceptedSignature, const FActiveQuest&, AcceptedQuest);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CELESTIA_API UQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuestComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	// Comprueba si una misión ya completó todos sus objetivos
	void CheckQuestCompletion(int32 QuestIndex);

public:
	// --- VARIABLES PRINCIPALES ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Quests")
	TArray<FActiveQuest> ActiveQuests;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Quests")
	TArray<FName> CompletedQuestIDs;

	// La misión que marcaste en la UI para ver en el HUD
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quests")
	UQuestDataAsset* TrackedQuestData;

	// --- EVENTOS PARA LA UI ---
	UPROPERTY(BlueprintAssignable, Category = "Quests | Events")
	FOnQuestListUpdatedSignature OnQuestListUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Quests | Events")
	FOnObjectiveUpdatedSignature OnObjectiveUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Quests | Events")
	FOnQuestTrackedSignature OnQuestTracked;

	UPROPERTY(BlueprintAssignable, Category = "Quests | Events")
	FOnQuestRewardsGrantedSignature OnQuestRewardsGranted;

	UPROPERTY(BlueprintAssignable, Category = "Quests | Events")
	FOnQuestItemsConsumedSignature OnQuestItemsConsumed;

	UFUNCTION(BlueprintPure, Category = "Quests | UI")
	bool GetActiveQuestData(UQuestDataAsset* QuestToCheck, FActiveQuest& OutActiveQuest) const;

	UFUNCTION(BlueprintPure, Category = "Quests | UI")
	bool IsQuestCompleted(FName QuestIDToCheck) const;

	UPROPERTY(BlueprintAssignable, Category = "Quests | Events")
	FOnQuestUntrackedSignature OnQuestUntracked;

	UPROPERTY(BlueprintAssignable, Category = "Quests | Events")
	FOnQuestAcceptedSignature OnQuestAccepted;


	// --- FUNCIONES CORE ---

	// Llamada por el NPC cuando aceptas una misión
	UFUNCTION(BlueprintCallable, Category = "Quests")
	void AcceptQuest(UQuestDataAsset* NewQuest);

	// Llamada por los enemigos, triggers, etc. para avanzar la misión
	UFUNCTION(BlueprintCallable, Category = "Quests")
	void UpdateObjective(EObjectiveType Type, FName TargetID, TSubclassOf<class AItemBase> TargetItemClass, int32 Amount);

	// Llamada por la UI cuando seleccionas una misión para seguir
	UFUNCTION(BlueprintCallable, Category = "Quests")
	void TrackQuest(UQuestDataAsset* QuestToTrack);

	UFUNCTION(BlueprintCallable, Category = "Quests")
	void TurnInQuest(UQuestDataAsset* QuestToTurnIn);

};