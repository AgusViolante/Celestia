// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UINPCDialogue.generated.h"

class UTextBlock;
class UButton;
class ANPCBase;

UCLASS()
class CELESTIA_API UUINPCDialogue : public UUserWidget
{
	GENERATED_BODY()

public:
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

	// Inicializa la UI según los datos del NPC
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetupUI(ANPCBase* InNPC, AActor* InInteractor);

protected:
	virtual void NativeConstruct() override;

	// Eventos de botones
	UFUNCTION()
	void OnQuestClicked();

	UFUNCTION()
	void OnShopClicked();

	UFUNCTION()
	void OnCraftClicked();

	UFUNCTION()
	void OnLeaveClicked();

private:
	UPROPERTY()
	ANPCBase* CurrentNPC;

	UPROPERTY()
	AActor* CurrentInteractor;
};