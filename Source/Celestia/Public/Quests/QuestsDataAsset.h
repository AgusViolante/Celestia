#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemBase/ItemBase.h"
#include "Characters/EnemyBase.h"
#include "QuestsDataAsset.generated.h"

UENUM(BlueprintType)
enum class EQuestType : uint8
{
	Main		UMETA(DisplayName = "Misión Principal"),
	Secondary	UMETA(DisplayName = "Misión Secundaria")
};

UENUM(BlueprintType)
enum class EObjectiveType : uint8
{
	Kill		UMETA(DisplayName = "Matar Enemigo"),
	Collect		UMETA(DisplayName = "Recolectar Item"),
	Location	UMETA(DisplayName = "Alcanzar Ubicación"),
	Dungeon		UMETA(DisplayName = "Completar Dungeon"),
	Talk		UMETA(DisplayName = "Hablar con NPC")
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Objective")
	FText TargetDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Objective", meta = (EditCondition = "ObjectiveType == EObjectiveType::Talk"))
	TArray<FText> DialogueLines;
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
	FText ItemDisplayName;
};

USTRUCT(BlueprintType)
struct FQuestReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Reward")
	float Experience;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Reward")
	int32 BonusLevelsToGrant;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Dialogues")
	TArray<FText> QuestOfferDialogue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Dialogues")
	TArray<FText> QuestTurnInDialogue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Info")
	EQuestType QuestType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Info")
	FName GiverNPC_ID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Info")
	FText GiverName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Info")
	FName ReceiverNPC_ID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Info")
	FText ReceiverName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Objectives")
	TArray<FQuestObjective> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Rewards")
	FQuestReward Rewards;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Info")
	bool bIsRepeatable = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Info", meta = (ClampMin = "1"))
	int32 RequiredLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Info")
	TObjectPtr<UQuestDataAsset> PrerequisiteQuest;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Chain")
	TObjectPtr<UQuestDataAsset> NextQuest;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Info")
	bool bAutoTurnIn = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Info")
	bool bIsGlobalCoop = false;
};