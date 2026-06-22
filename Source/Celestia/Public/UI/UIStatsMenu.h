#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/StatsComponent.h"
#include "UIStatsMenu.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class CELESTIA_API UUIStatsMenu : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_AvailablePoints;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_STR;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Btn_AddSTR;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_DEX;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Btn_AddDEX;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_INT;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Btn_AddINT;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_WIS;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Btn_AddWIS;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_END;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Btn_AddEND;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_MaxHealth;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_MaxMana;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_MaxStamina;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_MeleeAttack;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_RangedAttack;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_MagicAttack;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_MeleeDefense;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_MagicDefense;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_MeleeCrit;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_MagicCrit;

	UFUNCTION(BlueprintCallable, Category = "UI | Stats")
	void RefreshAllStats();

private:
	UPROPERTY()
	TObjectPtr<UStatsComponent> PlayerStatsComp;

	UFUNCTION() void OnAddSTRClicked();
	UFUNCTION() void OnAddDEXClicked();
	UFUNCTION() void OnAddINTClicked();
	UFUNCTION() void OnAddWISClicked();
	UFUNCTION() void OnAddENDClicked();

	UFUNCTION() void HandleStatPointsChanged(int32 NewStatPoints);
	UFUNCTION() void HandleStatChanged(ERPGStatType StatType, float NewTotalValue);
};