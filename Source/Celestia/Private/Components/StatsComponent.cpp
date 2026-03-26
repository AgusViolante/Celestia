// Fill out your copyright notice in the Description page of Project Settings.
#include "Components/StatsComponent.h"

UStatsComponent::UStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	Strength = FRPGStat(10.0f);
	Dexterity = FRPGStat(10.0f);
	Intelligence = FRPGStat(10.0f);
	Wisdom = FRPGStat(10.0f);
	Endurance = FRPGStat(10.0f);
}

void UStatsComponent::BeginPlay()
{
	Super::BeginPlay();
	RecalculateDerivedStats();
}

void UStatsComponent::RecalculateDerivedStats()
{
	MaxHealth.BaseValue = Endurance.GetTotal() * 10.0f;
	OnMaxHealthCalculated.Broadcast(MaxHealth.GetTotal());

	float CalculatedMaxMana = Intelligence.GetTotal() * 5.0f;
	OnMaxManaCalculated.Broadcast(CalculatedMaxMana);

	MeleeAttack.BaseValue = Strength.GetTotal() * 2.0f;
	OnStatChanged.Broadcast(ERPGStatType::MeleeAttack, MeleeAttack.GetTotal());

	RangedAttack.BaseValue = Dexterity.GetTotal() * 2.0f;
	OnStatChanged.Broadcast(ERPGStatType::RangedAttack, RangedAttack.GetTotal());

	MagicAttack.BaseValue = Intelligence.GetTotal() * 2.0f;
	OnStatChanged.Broadcast(ERPGStatType::MagicAttack, MagicAttack.GetTotal());

	MeleeDefense.BaseValue = Endurance.GetTotal() * 1.5f;
	OnStatChanged.Broadcast(ERPGStatType::MeleeDefense, MeleeDefense.GetTotal());

	MagicDefense.BaseValue = Wisdom.GetTotal() * 1.5f;
	OnStatChanged.Broadcast(ERPGStatType::MagicDefense, MagicDefense.GetTotal());

	MeleeCrit.BaseValue = Dexterity.GetTotal() * 0.5f;
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
	Strength.BaseValue += 2.0f;
	Dexterity.BaseValue += 2.0f;
	Intelligence.BaseValue += 2.0f;
	Wisdom.BaseValue += 2.0f;
	Endurance.BaseValue += 2.0f;

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
		// Si en el futuro querés leer secundarios, los agregás acá
	case ERPGStatType::MaxHealth: return MaxHealth.GetTotal();
	case ERPGStatType::MagicAttack: return MagicAttack.GetTotal();
	case ERPGStatType::MeleeAttack: return MeleeAttack.GetTotal();
	case ERPGStatType::MeleeDefense: return MeleeDefense.GetTotal();
	case ERPGStatType::MagicDefense: return MagicDefense.GetTotal();
	case ERPGStatType::MeleeCrit: return MeleeCrit.GetTotal();
	case ERPGStatType::MagicCrit: return MagicCrit.GetTotal();
	default: return 0.0f;
	}
}