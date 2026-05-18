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


	// --- FUNCIONES DE ACTUALIZACIÓN ---
	UFUNCTION()
	void UpdateHealth(UHealthComponent* HealthComp, float CurrentHealth, float MaxHealth, float HealthDelta);

	UFUNCTION()
	void UpdateStamina(UStaminaComponent* StaminaComp, float CurrentStamina, float MaxStamina);

	UFUNCTION()
	void UpdateMana(float CurrentMana, float MaxMana);

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


	UPROPERTY(meta = (BindWidgetOptional))
	class UProgressBar* BossHealthBar;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* BossNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	class UWidget* BossUIContainer;

	UFUNCTION(BlueprintCallable, Category = "UI | Boss")
	void ShowBossUI(const FString& BossName, float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintCallable, Category = "UI | Boss")
	void UpdateBossHealth(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintCallable, Category = "UI | Boss")
	void HideBossUI();

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* BossFadeInAnim;

};