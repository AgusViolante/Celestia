#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/StatsComponent.h"
#include "Components/VerticalBox.h"
#include "UIPlayerHUD.generated.h"

class UProgressBar;
class UTextBlock;
class UHealthComponent;
class UStaminaComponent;
class UNiagaraSystem;
class UQuestComponent;

UCLASS()
class CELESTIA_API UUIPlayerHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	// --- BARRAS Y RECURSOS ---
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* StaminaBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ManaBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ManaText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* XP_ProgressBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Level_Text;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* XP_Text;

	// --- TEXTOS DE ESTADÍSTICAS PRIMARIAS ---
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_STR;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_DEX;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_INT;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_WIS;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_END;

	// --- FUNCIONES DE ACTUALIZACIÓN ---
	UFUNCTION()
	void UpdateHealth(UHealthComponent* HealthComp, float CurrentHealth, float MaxHealth, float HealthDelta);

	UFUNCTION()
	void UpdateStamina(UStaminaComponent* StaminaComp, float CurrentStamina, float MaxStamina);

	UFUNCTION()
	void UpdateMana(float CurrentMana, float MaxMana);

	UFUNCTION()
	void UpdateStat(ERPGStatType StatType, float NewValue);

	UFUNCTION()
	void UpdateXP(float CurrentXP, float MaxXP);

	UFUNCTION()
	void UpdateLevel(int32 NewLevel);

	// --- QUEST TRACKER ---
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Txt_TrackedQuestName;

	UPROPERTY(meta = (BindWidgetOptional))
	UVerticalBox* QuestObjectivesContainer;

	UFUNCTION(BlueprintCallable, Category = "Quests | UI")
	void UpdateTrackedQuest(const FActiveQuest& TrackedQuest);

	UFUNCTION(BlueprintCallable, Category = "Quests | UI")
	void ClearTrackedQuest();

};