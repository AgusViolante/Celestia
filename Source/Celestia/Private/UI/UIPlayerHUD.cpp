#include "UI/UIPlayerHUD.h"
#include "Components/ProgressBar.h"
#include "Components/StaminaComponent.h"
#include "Quests/QuestComponent.h"
#include "Animation/WidgetAnimation.h"
#include "Components/TextBlock.h"


void UUIPlayerHUD::UpdateHealth(UHealthComponent* HealthComp, float CurrentHealth, float MaxHealth, float HealthDelta)
{
	if (HealthBar && MaxHealth > 0.f)
	{
		HealthBar->SetPercent(CurrentHealth / MaxHealth);
	}

	if (HealthText)
	{
		FString HPString = FString::Printf(TEXT("%d / %d"), FMath::RoundToInt(CurrentHealth), FMath::RoundToInt(MaxHealth));
		HealthText->SetText(FText::FromString(HPString));
	}
}

void UUIPlayerHUD::UpdateStamina(UStaminaComponent* StaminaComp, float CurrentStamina, float MaxStamina)
{
	if (StaminaBar && MaxStamina > 0.f)
	{
		StaminaBar->SetPercent(CurrentStamina / MaxStamina);
	}
}

void UUIPlayerHUD::UpdateMana(float CurrentMana, float MaxMana)
{
	if (ManaBar && MaxMana > 0.f)
	{
		ManaBar->SetPercent(CurrentMana / MaxMana);
	}

	if (ManaText)
	{
		FString ManaString = FString::Printf(TEXT("%d / %d"), FMath::RoundToInt(CurrentMana), FMath::RoundToInt(MaxMana));
		ManaText->SetText(FText::FromString(ManaString));
	}
}

void UUIPlayerHUD::UpdateXP(float CurrentXP, float MaxXP)
{
	
	if (XP_ProgressBar && MaxXP > 0.0f)
	{
		float Percent = CurrentXP / MaxXP;
		XP_ProgressBar->SetPercent(Percent);
	}

	if (XP_Text)
	{
		
		int32 IntCurrentXP = FMath::FloorToInt(CurrentXP);
		int32 IntMaxXP = FMath::FloorToInt(MaxXP);

		
		FString XPString = FString::Printf(TEXT("%d / %d"), IntCurrentXP, IntMaxXP);
		XP_Text->SetText(FText::FromString(XPString));
	}
}

void UUIPlayerHUD::UpdateLevel(int32 NewLevel)
{
	if (Level_Text)
	{
	Level_Text->SetText(FText::AsNumber(NewLevel));
	}
}
void UUIPlayerHUD::UpdateTrackedQuest(const FActiveQuest& TrackedQuest)
{
	if (Txt_TrackedQuestName && TrackedQuest.QuestData)
	{
		Txt_TrackedQuestName->SetText(TrackedQuest.QuestData->QuestName);
	}

	if (QuestObjectivesContainer)
	{
		QuestObjectivesContainer->ClearChildren();

		for (const FQuestObjective& Obj : TrackedQuest.CurrentObjectives)
		{
			UTextBlock* ObjText = NewObject<UTextBlock>(this);

			FString TargetName = Obj.TargetDisplayName.IsEmpty() ? TEXT("???") : Obj.TargetDisplayName.ToString();

			FString ObjString = FString::Printf(TEXT("- %s: %d / %d"), *TargetName, Obj.CurrentAmount, Obj.RequiredAmount);
			ObjText->SetText(FText::FromString(ObjString));

			FSlateFontInfo FontInfo = ObjText->GetFont();
			FontInfo.Size = 14;
			ObjText->SetFont(FontInfo);

			QuestObjectivesContainer->AddChild(ObjText);
		}

		if (TrackedQuest.bIsReadyToTurnIn)
		{
			UTextBlock* CompleteText = NewObject<UTextBlock>(this);

			CompleteText->SetText(FText::FromString(TEXT(">> Mision Lista para Entregar <<")));

			FSlateFontInfo CompleteFontInfo = CompleteText->GetFont();
			CompleteFontInfo.Size = 16;
			CompleteText->SetFont(CompleteFontInfo);
			CompleteText->SetColorAndOpacity(FSlateColor(FLinearColor::Green));

			QuestObjectivesContainer->AddChild(CompleteText);
		}
	}
}

void UUIPlayerHUD::ClearTrackedQuest()
{
	if (Txt_TrackedQuestName)
	{
		Txt_TrackedQuestName->SetText(FText::FromString(TEXT("")));
	}
	if (QuestObjectivesContainer)
	{
		QuestObjectivesContainer->ClearChildren();
	}
}
void UUIPlayerHUD::ShowBossUI(const FString& BossName, float CurrentHealth, float MaxHealth)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, TEXT("HUD: ShowBossUI ejecutado"));

	if (BossUIContainer)
	{
		BossUIContainer->SetVisibility(ESlateVisibility::Visible);
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, TEXT("HUD: Contenedor puesto en VISIBLE"));
	}

	if (BossNameText)
	{
		BossNameText->SetText(FText::FromString(BossName));
	}

	UpdateBossHealth(CurrentHealth, MaxHealth);

	if (BossUIContainer)
	{
		BossUIContainer->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		if (BossHealthBar) BossHealthBar->SetVisibility(ESlateVisibility::Visible);
		if (BossNameText) BossNameText->SetVisibility(ESlateVisibility::Visible);
	}

	if (BossFadeInAnim)
	{
		PlayAnimation(BossFadeInAnim);
	}
}

void UUIPlayerHUD::UpdateBossHealth(float CurrentHealth, float MaxHealth)
{
	if (BossHealthBar && MaxHealth > 0.f)
	{
		BossHealthBar->SetPercent(CurrentHealth / MaxHealth);
	}
}

void UUIPlayerHUD::HideBossUI()
{
	if (BossUIContainer)
	{
		BossUIContainer->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		if (BossHealthBar) BossHealthBar->SetVisibility(ESlateVisibility::Collapsed);
		if (BossNameText) BossNameText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UUIPlayerHUD::ShowNewQuestNotification(const FActiveQuest& AcceptedQuest)
{
	if (Txt_NewQuestName && AcceptedQuest.QuestData)
	{
		Txt_NewQuestName->SetText(AcceptedQuest.QuestData->QuestName);
	}

	if (Anim_NewQuest)
	{
		PlayAnimation(Anim_NewQuest);
	}
}