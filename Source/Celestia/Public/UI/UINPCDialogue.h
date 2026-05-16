// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UINPCDialogue.generated.h"

class UTextBlock;
class UButton;
class ANPCBase;
class UQuestDataAsset;

UCLASS()
class CELESTIA_API UUINPCDialogue : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	class UWidget* Panel_Main;

	// --- ELEMENTOS VISUALES ---
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_NPCName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_Greeting;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Quest;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_QuestBtn; // Para cambiar el texto dinámicamente ("Hablar de Misiones", "Entregar", etc.)

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Shop;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Craft;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Leave;

	UPROPERTY(meta = (BindWidget))
	class UWidget* Panel_QuestDetails;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_QuestTitle;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_QuestLore;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_AcceptQuest;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_AcceptBtn;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_DeclineQuest;

	// Inicializa la UI según los datos del NPC
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetupUI(ANPCBase* InNPC, AActor* InInteractor);

protected:
	virtual void NativeConstruct() override;

	// Eventos de botones
	UFUNCTION()
	void OnQuestClicked();

	
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable, Category = "Interaction")
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
	ANPCBase* CurrentNPC;

	UPROPERTY()
	AActor* CurrentInteractor;

	UPROPERTY() 
	UQuestDataAsset* PendingQuest;

	bool bIsTurningIn = false;

	bool bIsTalking = false;
	TArray<FText> ActiveDialogueLines;
	int32 CurrentLineIndex = 0;
	FName TalkNPC_ID;
};