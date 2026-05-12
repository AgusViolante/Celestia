#include "UI/UIPlayerHUD.h"
#include "Components/ProgressBar.h"
#include "Components/StaminaComponent.h"
#include "Quests/QuestComponent.h"
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

void UUIPlayerHUD::UpdateStat(ERPGStatType StatType, float NewValue)
{
	
	int32 RoundedValue = FMath::RoundToInt(NewValue);
	FString FormattedString;

	switch (StatType)
	{
	case ERPGStatType::Strength:
		if (Txt_STR)
		{
			FormattedString = FString::Printf(TEXT("STR : %d"), RoundedValue);
			Txt_STR->SetText(FText::FromString(FormattedString));
		}
		break;

	case ERPGStatType::Dexterity:
		if (Txt_DEX)
		{
			FormattedString = FString::Printf(TEXT("DEX : %d"), RoundedValue);
			Txt_DEX->SetText(FText::FromString(FormattedString));
		}
		break;

	case ERPGStatType::Intelligence:
		if (Txt_INT)
		{
			FormattedString = FString::Printf(TEXT("INT : %d"), RoundedValue);
			Txt_INT->SetText(FText::FromString(FormattedString));
		}
		break;

	case ERPGStatType::Wisdom:
		if (Txt_WIS)
		{
			FormattedString = FString::Printf(TEXT("WIS : %d"), RoundedValue);
			Txt_WIS->SetText(FText::FromString(FormattedString));
		}
		break;

	case ERPGStatType::Endurance:
		if (Txt_END)
		{
			FormattedString = FString::Printf(TEXT("END : %d"), RoundedValue);
			Txt_END->SetText(FText::FromString(FormattedString));
		}
		break;

	default:
		break;
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