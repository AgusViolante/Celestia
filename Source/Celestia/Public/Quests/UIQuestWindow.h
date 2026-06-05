// Fill out your copyright notice in the Description page of Project Settings.
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
	UButton* Btn_TrackQuest;

	UFUNCTION()
	void OnTrackButtonClicked();
	// --- CONTENEDORES ---
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ActiveQuestsContainer;

	UPROPERTY(meta = (BindWidgetOptional))
	UScrollBox* AvailableQuestsContainer;

	UPROPERTY(meta = (BindWidgetOptional))
	UScrollBox* CompletedQuestsContainer;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* ObjectivesContainer;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* RewardsContainer;

	// --- TEXTOS PRINCIPALES ---
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_DetailName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_DetailDescription;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_GiverNPC;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_ReceiverNPC;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_QuestStatus;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_QuestType;

	UFUNCTION()
	void OnQuestSelected(const FActiveQuest& SelectedQuest);

	// --- ESTILOS DE LA INTERFAZ ---

	// Fuente para los Títulos (Misión y Encabezados)
	UPROPERTY(EditDefaultsOnly, Category = "Quests | Style")
	FSlateFontInfo TitleFont;

	// Fuente para el texto general (Descripciones, Objetivos)
	UPROPERTY(EditDefaultsOnly, Category = "Quests | Style")
	FSlateFontInfo BodyFont;

	// Color para el texto de Misión Completada
	UPROPERTY(EditDefaultsOnly, Category = "Quests | Style")
	FLinearColor CompletedColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f); // Gris

	// Color para el texto Activo/En Progreso
	UPROPERTY(EditDefaultsOnly, Category = "Quests | Style")
	FLinearColor ActiveColor = FLinearColor::White;

	// Color de realce (Ej: para recompensas o títulos)
	UPROPERTY(EditDefaultsOnly, Category = "Quests | Style")
	FLinearColor AccentColor = FLinearColor(0.8f, 0.6f, 0.1f, 1.0f); // Dorado/Amarillo pálido

private:
	void ClearDetailsUI();
	void CreateObjectiveRow(const FString& ObjectiveText, bool bIsCompleted);
	void CreateRewardRow(const FString& RewardName, int32 Quantity);

	UPROPERTY()
	UQuestDataAsset* CurrentlyViewedQuest;
};