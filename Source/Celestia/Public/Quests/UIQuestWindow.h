#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Quests/QuestComponent.h"
#include "UIQuestWindow.generated.h"

class UScrollBox;
class UVerticalBox;
class UTextBlock;
class UUIQuestSlot;
class UButton;

UCLASS()
class CELESTIA_API UUIQuestWindow : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Quests | UI")
	void UpdateQuestList();

	UPROPERTY(EditDefaultsOnly, Category = "Quests | UI")
	TSubclassOf<UUIQuestSlot> QuestSlotClass;

	UPROPERTY(EditDefaultsOnly, Category = "Quests | Database")
	TArray<UQuestDataAsset*> AllGameQuests;

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_TrackQuest;

	UFUNCTION()
	void OnTrackButtonClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ActiveQuestsContainer;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UScrollBox> AvailableQuestsContainer;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UScrollBox> CompletedQuestsContainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> ObjectivesContainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> RewardsContainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_DetailName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_DetailDescription;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_GiverNPC;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_ReceiverNPC;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_QuestStatus;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_QuestType;

	UFUNCTION()
	void OnQuestSelected(const FActiveQuest& SelectedQuest);

	UPROPERTY(EditDefaultsOnly, Category = "Quests | Style")
	FSlateFontInfo TitleFont;

	UPROPERTY(EditDefaultsOnly, Category = "Quests | Style")
	FSlateFontInfo BodyFont;

	UPROPERTY(EditDefaultsOnly, Category = "Quests | Style")
	FLinearColor CompletedColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Quests | Style")
	FLinearColor ActiveColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category = "Quests | Style")
	FLinearColor AccentColor = FLinearColor(0.8f, 0.6f, 0.1f, 1.0f);

private:
	void ClearDetailsUI();
	void CreateObjectiveRow(const FString& ObjectiveText, bool bIsCompleted);
	void CreateRewardRow(const FString& RewardName, int32 Quantity);

	UPROPERTY()
	TObjectPtr<UQuestDataAsset> CurrentlyViewedQuest;
};