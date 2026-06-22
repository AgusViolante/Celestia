#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UINPCDialogue.generated.h"

class UTextBlock;
class UButton;
class ANPCBase;
class UQuestDataAsset;
class UWidget;

UCLASS()
class CELESTIA_API UUINPCDialogue : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> Panel_Main;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_NPCName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Greeting;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Quest;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_QuestBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Shop;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Craft;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Leave;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> Panel_QuestDetails;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_QuestTitle;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_QuestLore;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_AcceptQuest;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_AcceptBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_DeclineQuest;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetupUI(ANPCBase* InNPC, AActor* InInteractor);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnQuestClicked();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnShopClicked();

	UFUNCTION()
	void OnCraftClicked();

	UFUNCTION()
	void OnLeaveClicked();

	UFUNCTION()
	void OnAcceptQuestClicked();

	UFUNCTION()
	void OnDeclineQuestClicked();

private:
	UPROPERTY()
	TObjectPtr<ANPCBase> CurrentNPC;

	UPROPERTY()
	TObjectPtr<AActor> CurrentInteractor;

	UPROPERTY()
	TObjectPtr<UQuestDataAsset> PendingQuest;

	bool bIsTurningIn = false;
	bool bIsTalking = false;
	TArray<FText> ActiveDialogueLines;
	int32 CurrentLineIndex = 0;
	FName TalkNPC_ID;
};