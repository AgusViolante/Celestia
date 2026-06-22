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
	TObjectPtr<UTextBlock> Txt_QuestName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Select;

	UFUNCTION()
	void OnButtonClicked();
};