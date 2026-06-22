#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BillboardComponent.h"
#include "Components/WidgetComponent.h"
#include "../../Quests/QuestsDataAsset.h"
#include "NPCBase.generated.h"

class UUINPCDialogue;
class UQuestComponent;

UCLASS()
class CELESTIA_API ANPCBase : public ACharacter
{
	GENERATED_BODY()

public:
	ANPCBase();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quests")
	FName NPC_ID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quests")
	TArray<TObjectPtr<UQuestDataAsset>> AvailableQuests;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUINPCDialogue> DialogueWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC Info")
	FText NPC_Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC Info", meta = (MultiLine = "true"))
	FText GreetingMessage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC Services")
	bool bHasShop = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC Services")
	bool bHasCrafting = false;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* Interactor);
	virtual void Interact_Implementation(AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ProcessQuestInteraction(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OpenShop(AActor* Interactor);
	virtual void OpenShop_Implementation(AActor* Interactor) {}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OpenCrafting(AActor* Interactor);
	virtual void OpenCrafting_Implementation(AActor* Interactor) {}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UBillboardComponent> QuestIndicatorIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Quests | Icons")
	TObjectPtr<UTexture2D> Icon_NewQuest;

	UPROPERTY(EditDefaultsOnly, Category = "Quests | Icons")
	TObjectPtr<UTexture2D> Icon_TurnInQuest;

	UFUNCTION(BlueprintCallable, Category = "Quests | UI")
	void UpdateQuestIndicator(UQuestComponent* PlayerQuestComp);

protected:
	virtual void BeginPlay() override;

	FTimerHandle QuestIndicatorTimerHandle;

	UFUNCTION()
	void CheckQuestStatus();
};