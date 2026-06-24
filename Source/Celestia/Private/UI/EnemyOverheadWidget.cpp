// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/EnemyOverheadWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UEnemyOverheadWidget::SetupWidget(const FText& InName, int32 InLevel)
{
	if (NameText)
	{
		NameText->SetText(InName);
	}
	if (LevelText)
	{
		FText LvlStr = FText::FromString(FString::Printf(TEXT("Lv. %d"), InLevel));
		LevelText->SetText(LvlStr);
	}
}

void UEnemyOverheadWidget::UpdateHealth(float HealthPercent)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(HealthPercent);
	}
}
