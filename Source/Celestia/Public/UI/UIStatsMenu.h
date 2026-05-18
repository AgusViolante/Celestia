// Fill out your copyright notice in the Description page of Project Settings.
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
	// --- PUNTOS DISPONIBLES ---
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_AvailablePoints;

	// --- STATS PRIMARIOS Y BOTONES ---
	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_STR;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_AddSTR;

	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_DEX;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_AddDEX;

	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_INT;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_AddINT;

	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_WIS;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_AddWIS;

	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_END;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_AddEND;

	// --- STATS SECUNDARIOS (Solo lectura) ---
	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_MaxHealth;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_MaxMana;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_MaxStamina;

	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_MeleeAttack;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_RangedAttack;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_MagicAttack;

	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_MeleeDefense;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_MagicDefense;

	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_MeleeCrit;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_MagicCrit;

	// Función para actualizar todos los textos de golpe
	UFUNCTION(BlueprintCallable, Category = "UI | Stats")
	void RefreshAllStats();

private:
	// Referencia guardada al componente del jugador
	UPROPERTY()
	UStatsComponent* PlayerStatsComp;

	// Funciones de clic para cada botón
	UFUNCTION() void OnAddSTRClicked();
	UFUNCTION() void OnAddDEXClicked();
	UFUNCTION() void OnAddINTClicked();
	UFUNCTION() void OnAddWISClicked();
	UFUNCTION() void OnAddENDClicked();
};