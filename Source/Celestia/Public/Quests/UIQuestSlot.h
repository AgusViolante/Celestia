// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Quests/QuestComponent.h"
#include "UIQuestSlot.generated.h"

class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestSlotClickedSignature, const FActiveQuest&, QuestData);

UCLASS()
class CELESTIA_API UUIQuestSlot : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	FActiveQuest MyQuest;

	
	UPROPERTY(BlueprintAssignable, Category = "Quest | Events")
	FOnQuestSlotClickedSignature OnQuestSlotClicked;

	UFUNCTION(BlueprintCallable, Category = "Quest | UI")
	void SetupSlot(const FActiveQuest& InQuest);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_QuestName;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Select;

	UFUNCTION()
	void OnButtonClicked();
};