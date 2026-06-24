// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyOverheadWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class CELESTIA_API UEnemyOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LevelText;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetupWidget(const FText& InName, int32 InLevel);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHealth(float HealthPercent);
};