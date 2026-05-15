// Fill out your copyright notice in the Description page of Project Settings.
#include "Components/StatsComponent.h"

UStatsComponent::UStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	Strength = FRPGStat(1.0f);
	Dexterity = FRPGStat(1.0f);
	Intelligence = FRPGStat(1.0f);
	Wisdom = FRPGStat(1.0f);
	Endurance = FRPGStat(1.0f);
}

void UStatsComponent::BeginPlay()
{
	Super::BeginPlay();
	RecalculateDerivedStats();
}

void UStatsComponent::RecalculateDerivedStats()
{
	// Vida: 90 Base + (Endurance * 10) = 100 a nivel 1. Sube 10 por cada punto.
	MaxHealth.BaseValue = 90.0f + (Endurance.GetTotal() * 10.0f);
	OnMaxHealthCalculated.Broadcast(MaxHealth.GetTotal());

	// Maná: 95 Base + (Intelligence * 5) = 100 a nivel 1. Sube 5 por cada punto.
	MaxMana.BaseValue = 95.0f + (Intelligence.GetTotal() * 5.0f);
	OnMaxManaCalculated.Broadcast(MaxMana.GetTotal());

	// Ataque Cuerpo a Cuerpo: 5 Base + (Strength * 2) = 7 a nivel 1.
	MeleeAttack.BaseValue = 5.0f + (Strength.GetTotal() * 2.0f);
	OnStatChanged.Broadcast(ERPGStatType::MeleeAttack, MeleeAttack.GetTotal());

	// Ataque a Distancia: 5 Base + (Dexterity * 2) = 7 a nivel 1.
	RangedAttack.BaseValue = 5.0f + (Dexterity.GetTotal() * 2.0f);
	OnStatChanged.Broadcast(ERPGStatType::RangedAttack, RangedAttack.GetTotal());

	// Ataque Mágico: 5 Base + (Intelligence * 2) = 7 a nivel 1.
	MagicAttack.BaseValue = 5.0f + (Intelligence.GetTotal() * 2.0f);
	OnStatChanged.Broadcast(ERPGStatType::MagicAttack, MagicAttack.GetTotal());

	// Defensa Física: (Endurance * 1.5)
	MeleeDefense.BaseValue = Endurance.GetTotal() * 1.5f;
	OnStatChanged.Broadcast(ERPGStatType::MeleeDefense, MeleeDefense.GetTotal());

	// Defensa Mágica: (Wisdom * 1.5)
	MagicDefense.BaseValue = Wisdom.GetTotal() * 1.5f;
	OnStatChanged.Broadcast(ERPGStatType::MagicDefense, MagicDefense.GetTotal());

	// Críticos
	MeleeCrit.BaseValue = Strength.GetTotal() * 0.5f;
	OnStatChanged.Broadcast(ERPGStatType::MeleeCrit, MeleeCrit.GetTotal());

	MagicCrit.BaseValue = Wisdom.GetTotal() * 0.5f;
	OnStatChanged.Broadcast(ERPGStatType::MagicCrit, MagicCrit.GetTotal());
}

void UStatsComponent::ModifyStat(ERPGStatType Stat, float Amount, bool bIsAdding)
{
	float ValorFinal = 0.0f;

	switch (Stat)
	{
	case ERPGStatType::Strength:
		bIsAdding ? Strength.AddModifier(Amount) : Strength.RemoveModifier(Amount);
		ValorFinal = Strength.GetTotal();
		RecalculateDerivedStats();
		break;

	case ERPGStatType::Dexterity:
		bIsAdding ? Dexterity.AddModifier(Amount) : Dexterity.RemoveModifier(Amount);
		ValorFinal = Dexterity.GetTotal();
		RecalculateDerivedStats();
		break;

	case ERPGStatType::Intelligence:
		bIsAdding ? Intelligence.AddModifier(Amount) : Intelligence.RemoveModifier(Amount);
		ValorFinal = Intelligence.GetTotal();
		RecalculateDerivedStats();
		break;

	case ERPGStatType::Wisdom:
		bIsAdding ? Wisdom.AddModifier(Amount) : Wisdom.RemoveModifier(Amount);
		ValorFinal = Wisdom.GetTotal();
		RecalculateDerivedStats();
		break;

	case ERPGStatType::Endurance:
		bIsAdding ? Endurance.AddModifier(Amount) : Endurance.RemoveModifier(Amount);
		ValorFinal = Endurance.GetTotal();
		RecalculateDerivedStats();
		break;

	case ERPGStatType::MagicAttack:
		bIsAdding ? MagicAttack.AddModifier(Amount) : MagicAttack.RemoveModifier(Amount);
		ValorFinal = MagicAttack.GetTotal();
		break;

	case ERPGStatType::MeleeAttack:
		bIsAdding ? MeleeAttack.AddModifier(Amount) : MeleeAttack.RemoveModifier(Amount);
		ValorFinal = MeleeAttack.GetTotal();
		break;

	case ERPGStatType::MeleeDefense:
		bIsAdding ? MeleeDefense.AddModifier(Amount) : MeleeDefense.RemoveModifier(Amount);
		ValorFinal = MeleeDefense.GetTotal();
		break;

	case ERPGStatType::MagicDefense:
		bIsAdding ? MagicDefense.AddModifier(Amount) : MagicDefense.RemoveModifier(Amount);
		ValorFinal = MagicDefense.GetTotal();
		break;
	}

	OnStatChanged.Broadcast(Stat, ValorFinal);
}

void UStatsComponent::OnLevelUp(int32 NewLevel)
{
	Strength.BaseValue += 1.0f;
	Dexterity.BaseValue += 1.0f;
	Intelligence.BaseValue += 1.0f;
	Wisdom.BaseValue += 1.0f;
	Endurance.BaseValue += 1.0f;

	RecalculateDerivedStats();

	OnStatChanged.Broadcast(ERPGStatType::Strength, Strength.GetTotal());
	OnStatChanged.Broadcast(ERPGStatType::Dexterity, Dexterity.GetTotal());
	OnStatChanged.Broadcast(ERPGStatType::Intelligence, Intelligence.GetTotal());
	OnStatChanged.Broadcast(ERPGStatType::Wisdom, Wisdom.GetTotal());
	OnStatChanged.Broadcast(ERPGStatType::Endurance, Endurance.GetTotal());
}
float UStatsComponent::GetStatValue(ERPGStatType StatType) const
{
	switch (StatType)
	{
	case ERPGStatType::Strength: return Strength.GetTotal();
	case ERPGStatType::Dexterity: return Dexterity.GetTotal();
	case ERPGStatType::Intelligence: return Intelligence.GetTotal();
	case ERPGStatType::Wisdom: return Wisdom.GetTotal();
	case ERPGStatType::Endurance: return Endurance.GetTotal();
	
	case ERPGStatType::MaxHealth: return MaxHealth.GetTotal();
	case ERPGStatType::MaxMana: return MaxMana.GetTotal();
	case ERPGStatType::MagicAttack: return MagicAttack.GetTotal();
	case ERPGStatType::MeleeAttack: return MeleeAttack.GetTotal();
	case ERPGStatType::MeleeDefense: return MeleeDefense.GetTotal();
	case ERPGStatType::MagicDefense: return MagicDefense.GetTotal();
	case ERPGStatType::MeleeCrit: return MeleeCrit.GetTotal();
	case ERPGStatType::MagicCrit: return MagicCrit.GetTotal();
	default: return 0.0f;
	}
} 
void UStatsComponent::SetPrimaryStats(float NewStr, float NewDex, float NewInt, float NewWis, float NewEnd)
{
	Strength.BaseValue = NewStr;
	Dexterity.BaseValue = NewDex;
	Intelligence.BaseValue = NewInt;
	Wisdom.BaseValue = NewWis;
	Endurance.BaseValue = NewEnd;

	
	RecalculateDerivedStats();
}