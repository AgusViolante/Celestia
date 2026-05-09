// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemBase/ItemBase.h"
#include "Characters/EnemyBase.h"
#include "QuestsDataAsset.generated.h"


// Tipos de misión para clasificar en la UIPlayerHUD
UENUM(BlueprintType)
enum class EQuestType : uint8
{
	Main		UMETA(DisplayName = "Misión Principal"),
	Secondary	UMETA(DisplayName = "Misión Secundaria")
};

// Tipos de objetivos para la lógica del QuestComponent
UENUM(BlueprintType)
enum class EObjectiveType : uint8
{
	Kill		UMETA(DisplayName = "Matar Enemigo"),
	Collect		UMETA(DisplayName = "Recolectar Item"),
	Location	UMETA(DisplayName = "Alcanzar Ubicación"),
	Dungeon		UMETA(DisplayName = "Completar Dungeon")
};

USTRUCT(BlueprintType)
struct FQuestObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Objective")
	EObjectiveType ObjectiveType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Objective", meta = (EditCondition = "ObjectiveType == EObjectiveType::Kill"))
	TSubclassOf<AEnemyBase> TargetEnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Objective", meta = (EditCondition = "ObjectiveType == EObjectiveType::Collect"))
	TSubclassOf<class AItemBase> TargetItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Objective")
	FName TargetID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Objective", meta = (ClampMin = "1"))
	int32 RequiredAmount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Objective")
	int32 CurrentAmount;

	FQuestObjective() : ObjectiveType(EObjectiveType::Kill), RequiredAmount(1), CurrentAmount(0) {}
};
USTRUCT(BlueprintType)
struct FItemReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
	TSubclassOf<class AItemBase> ItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward", meta = (ClampMin = "1"))
	int32 Quantity;

	FItemReward() : ItemClass(nullptr), Quantity(1) {}
};
USTRUCT(BlueprintType)
struct FQuestReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Reward")
	float Experience;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Reward")
	int32 Coins;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Reward")
	TArray<FItemReward> RewardItems;

	FQuestReward() : Experience(0.f), Coins(0) {}
};


UCLASS(BlueprintType)
class CELESTIA_API UQuestDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Info")
	FName QuestID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Info")
	FText QuestName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Info", meta = (MultiLine = "true"))
	FText QuestDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Info")
	EQuestType QuestType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Info")
	FName GiverNPC_ID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Info")
	FName ReceiverNPC_ID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Objectives")
	TArray<FQuestObjective> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Rewards")
	FQuestReward Rewards;
};