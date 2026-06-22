#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestsDataAsset.h"
#include "QuestComponent.generated.h"

USTRUCT(BlueprintType)
struct FActiveQuest
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	TObjectPtr<UQuestDataAsset> QuestData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	TArray<FQuestObjective> CurrentObjectives;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bIsReadyToTurnIn;

	FActiveQuest() : QuestData(nullptr), bIsReadyToTurnIn(false) {}
	FActiveQuest(UQuestDataAsset* InQuestData) : QuestData(InQuestData), bIsReadyToTurnIn(false) {}
};

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

protected:
	virtual void BeginPlay() override;

	void CheckQuestCompletion(int32 QuestIndex);
	void CheckDungeonCompletion();

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quests")
	TArray<FActiveQuest> ActiveQuests;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quests")
	TArray<FName> CompletedQuestIDs;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quests")
	TObjectPtr<UQuestDataAsset> TrackedQuestData;

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

	UPROPERTY(BlueprintAssignable, Category = "Quests | Events")
	FOnQuestUntrackedSignature OnQuestUntracked;

	UPROPERTY(BlueprintAssignable, Category = "Quests | Events")
	FOnQuestAcceptedSignature OnQuestAccepted;

	UFUNCTION(BlueprintPure, Category = "Quests | UI")
	bool GetActiveQuestData(UQuestDataAsset* QuestToCheck, FActiveQuest& OutActiveQuest) const;

	UFUNCTION(BlueprintPure, Category = "Quests | UI")
	bool IsQuestCompleted(FName QuestIDToCheck) const;

	UFUNCTION(BlueprintPure, Category = "Quests | Validation")
	bool CanAcceptQuest(UQuestDataAsset* QuestToCheck) const;

	UPROPERTY(EditAnywhere, Category = "Quests | Auto")
	TArray<UQuestDataAsset*> StartingQuests;

	UPROPERTY(EditAnywhere, Category = "Quests | Auto")
	TArray<UQuestDataAsset*> AutoUnlockByLevelQuests;

	UFUNCTION()
	void OnPlayerLevelUp(int32 NewLevel);

	UFUNCTION(BlueprintCallable, Category = "Quests")
	void AcceptQuest(UQuestDataAsset* NewQuest);

	UFUNCTION(BlueprintCallable, Category = "Quests")
	void UpdateObjective(EObjectiveType Type, FName TargetID, TSubclassOf<class AItemBase> TargetItemClass, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Quests")
	void TurnInQuest(UQuestDataAsset* QuestToTurnIn);

	UFUNCTION(BlueprintCallable, Category = "Quests")
	void TrackQuest(UQuestDataAsset* QuestToTrack);

	UFUNCTION(BlueprintCallable, Category = "Quests | Network")
	void ClientInitializeQuests();

protected:

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestQuestData();

	UFUNCTION(Client, Reliable)
	void Client_ReceiveQuestData(const TArray<UQuestDataAsset*>& ServerActiveQuests, const TArray<int32>& ServerObjectiveProgress, const TArray<bool>& ServerQuestStatus, const TArray<FName>& ServerCompleted);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_AcceptQuest(UQuestDataAsset* NewQuest);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_UpdateObjective(EObjectiveType Type, FName TargetID, TSubclassOf<class AItemBase> TargetItemClass, int32 Amount);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TurnInQuest(UQuestDataAsset* QuestToTurnIn);

	void Internal_AcceptQuest(UQuestDataAsset* NewQuest);
	TArray<UQuestDataAsset*> Internal_UpdateObjective(EObjectiveType Type, FName TargetID, TSubclassOf<class AItemBase> TargetItemClass, int32 Amount);
	void Internal_TurnInQuest(UQuestDataAsset* QuestToTurnIn);
	void Internal_ForceSyncGlobalObjective(UQuestDataAsset* GlobalQuest, EObjectiveType Type, FName TargetID, TSubclassOf<class AItemBase> TargetItemClass, int32 Amount);

	UFUNCTION(Client, Reliable)
	void Client_SyncQuestAccepted(UQuestDataAsset* NewQuest);

	UFUNCTION(Client, Reliable)
	void Client_SyncObjectiveUpdated(UQuestDataAsset* QuestData, const TArray<int32>& NewProgress, bool bIsReadyToTurnIn);

	UFUNCTION(Client, Reliable)
	void Client_SyncQuestTurnedIn(UQuestDataAsset* QuestTurnedIn);

	UFUNCTION(Client, Reliable)
	void Client_NotifyQuestRewards(int32 Coins, const TArray<FItemReward>& Items);
};