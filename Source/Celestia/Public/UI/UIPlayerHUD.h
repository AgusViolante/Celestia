#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Quests/QuestComponent.h"
#include "UIPlayerHUD.generated.h"

class UProgressBar;
class UTextBlock;
class UHealthComponent;
class UStaminaComponent;
class UNiagaraSystem;
class UWidgetAnimation;
class UWidget;

UCLASS()
class CELESTIA_API UUIPlayerHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HealthText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ManaBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ManaText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> XP_ProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Level_Text;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> XP_Text;

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

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_TrackedQuestName;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UVerticalBox> QuestObjectivesContainer;

	UFUNCTION(BlueprintCallable, Category = "Quests | UI")
	void UpdateTrackedQuest(const FActiveQuest& TrackedQuest);

	UFUNCTION(BlueprintCallable, Category = "Quests | UI")
	void ClearTrackedQuest();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> BossHealthBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BossNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> BossUIContainer;

	UFUNCTION(BlueprintCallable, Category = "UI | Boss")
	void ShowBossUI(const FString& BossName, float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintCallable, Category = "UI | Boss")
	void UpdateBossHealth(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintCallable, Category = "UI | Boss")
	void HideBossUI();

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> BossFadeInAnim;

	UFUNCTION()
	void ShowNewQuestNotification(const FActiveQuest& AcceptedQuest);

protected:
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> Anim_NewQuest;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_NewQuestName;
};