// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UIStatsMenu.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "GameFramework/Pawn.h"

void UUIStatsMenu::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_AddSTR) Btn_AddSTR->OnClicked.AddDynamic(this, &UUIStatsMenu::OnAddSTRClicked);
	if (Btn_AddDEX) Btn_AddDEX->OnClicked.AddDynamic(this, &UUIStatsMenu::OnAddDEXClicked);
	if (Btn_AddINT) Btn_AddINT->OnClicked.AddDynamic(this, &UUIStatsMenu::OnAddINTClicked);
	if (Btn_AddWIS) Btn_AddWIS->OnClicked.AddDynamic(this, &UUIStatsMenu::OnAddWISClicked);
	if (Btn_AddEND) Btn_AddEND->OnClicked.AddDynamic(this, &UUIStatsMenu::OnAddENDClicked);
}

void UUIStatsMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (APawn* PlayerPawn = GetOwningPlayerPawn())
	{
		PlayerStatsComp = PlayerPawn->FindComponentByClass<UStatsComponent>();
	}

	RefreshAllStats();
}

void UUIStatsMenu::RefreshAllStats()
{
	if (!PlayerStatsComp) return;

	if (Txt_AvailablePoints)
	{
		Txt_AvailablePoints->SetText(FText::FromString(FString::Printf(TEXT("Puntos Disponibles: %d"), PlayerStatsComp->AvailableStatPoints)));
	}

	bool bHasPoints = PlayerStatsComp->AvailableStatPoints > 0;
	if (Btn_AddSTR) Btn_AddSTR->SetIsEnabled(bHasPoints);
	if (Btn_AddDEX) Btn_AddDEX->SetIsEnabled(bHasPoints);
	if (Btn_AddINT) Btn_AddINT->SetIsEnabled(bHasPoints);
	if (Btn_AddWIS) Btn_AddWIS->SetIsEnabled(bHasPoints);
	if (Btn_AddEND) Btn_AddEND->SetIsEnabled(bHasPoints);

	// --- ACTUALIZAR TEXTOS PRIMARIOS ---
	if (Txt_STR) Txt_STR->SetText(FText::AsNumber(FMath::RoundToInt(PlayerStatsComp->GetStatValue(ERPGStatType::Strength))));
	if (Txt_DEX) Txt_DEX->SetText(FText::AsNumber(FMath::RoundToInt(PlayerStatsComp->GetStatValue(ERPGStatType::Dexterity))));
	if (Txt_INT) Txt_INT->SetText(FText::AsNumber(FMath::RoundToInt(PlayerStatsComp->GetStatValue(ERPGStatType::Intelligence))));
	if (Txt_WIS) Txt_WIS->SetText(FText::AsNumber(FMath::RoundToInt(PlayerStatsComp->GetStatValue(ERPGStatType::Wisdom))));
	if (Txt_END) Txt_END->SetText(FText::AsNumber(FMath::RoundToInt(PlayerStatsComp->GetStatValue(ERPGStatType::Endurance))));

	// --- ACTUALIZAR TEXTOS SECUNDARIOS ---
	if (Txt_MaxHealth) Txt_MaxHealth->SetText(FText::AsNumber(FMath::RoundToInt(PlayerStatsComp->GetStatValue(ERPGStatType::MaxHealth))));
	if (Txt_MaxMana) Txt_MaxMana->SetText(FText::AsNumber(FMath::RoundToInt(PlayerStatsComp->GetStatValue(ERPGStatType::MaxMana))));
	if (Txt_MaxStamina) Txt_MaxStamina->SetText(FText::AsNumber(FMath::RoundToInt(PlayerStatsComp->GetStatValue(ERPGStatType::MaxStamina)))); // Asegurate de que esto exista en tu Enum

	if (Txt_MeleeAttack) Txt_MeleeAttack->SetText(FText::AsNumber(FMath::RoundToInt(PlayerStatsComp->GetStatValue(ERPGStatType::MeleeAttack))));
	if (Txt_RangedAttack) Txt_RangedAttack->SetText(FText::AsNumber(FMath::RoundToInt(PlayerStatsComp->GetStatValue(ERPGStatType::RangedAttack))));
	if (Txt_MagicAttack) Txt_MagicAttack->SetText(FText::AsNumber(FMath::RoundToInt(PlayerStatsComp->GetStatValue(ERPGStatType::MagicAttack))));

	if (Txt_MeleeDefense) Txt_MeleeDefense->SetText(FText::AsNumber(FMath::RoundToInt(PlayerStatsComp->GetStatValue(ERPGStatType::MeleeDefense))));
	if (Txt_MagicDefense) Txt_MagicDefense->SetText(FText::AsNumber(FMath::RoundToInt(PlayerStatsComp->GetStatValue(ERPGStatType::MagicDefense))));

	if (Txt_MeleeCrit) Txt_MeleeCrit->SetText(FText::FromString(FString::Printf(TEXT("%.1f%%"), PlayerStatsComp->GetStatValue(ERPGStatType::MeleeCrit))));
	if (Txt_MagicCrit) Txt_MagicCrit->SetText(FText::FromString(FString::Printf(TEXT("%.1f%%"), PlayerStatsComp->GetStatValue(ERPGStatType::MagicCrit))));
}

// --- LÓGICA DE LOS BOTONES ---

void UUIStatsMenu::OnAddSTRClicked()
{
	if (PlayerStatsComp && PlayerStatsComp->AllocateStatPoint(ERPGStatType::Strength))
		RefreshAllStats();
}

void UUIStatsMenu::OnAddDEXClicked()
{
	if (PlayerStatsComp && PlayerStatsComp->AllocateStatPoint(ERPGStatType::Dexterity))
		RefreshAllStats();
}

void UUIStatsMenu::OnAddINTClicked()
{
	if (PlayerStatsComp && PlayerStatsComp->AllocateStatPoint(ERPGStatType::Intelligence))
		RefreshAllStats();
}

void UUIStatsMenu::OnAddWISClicked()
{
	if (PlayerStatsComp && PlayerStatsComp->AllocateStatPoint(ERPGStatType::Wisdom))
		RefreshAllStats();
}

void UUIStatsMenu::OnAddENDClicked()
{
	if (PlayerStatsComp && PlayerStatsComp->AllocateStatPoint(ERPGStatType::Endurance))
		RefreshAllStats();
}
