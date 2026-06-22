#include "Components/StatsComponent.h"
#include "Net/UnrealNetwork.h"

UStatsComponent::UStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	Strength = FRPGStat(1.0f);
	Dexterity = FRPGStat(1.0f);
	Intelligence = FRPGStat(1.0f);
	Wisdom = FRPGStat(1.0f);
	Endurance = FRPGStat(1.0f);
}

void UStatsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStatsComponent, Strength);
	DOREPLIFETIME(UStatsComponent, Dexterity);
	DOREPLIFETIME(UStatsComponent, Intelligence);
	DOREPLIFETIME(UStatsComponent, Wisdom);
	DOREPLIFETIME(UStatsComponent, Endurance);
	DOREPLIFETIME(UStatsComponent, AvailableStatPoints);
	DOREPLIFETIME(UStatsComponent, CharacterLevel);
}

void UStatsComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		RecalculateDerivedStats();
	}
}

void UStatsComponent::OnRep_PrimaryStats()
{
	RecalculateDerivedStats();

	OnStatChanged.Broadcast(ERPGStatType::Strength, Strength.GetTotal());
	OnStatChanged.Broadcast(ERPGStatType::Dexterity, Dexterity.GetTotal());
	OnStatChanged.Broadcast(ERPGStatType::Intelligence, Intelligence.GetTotal());
	OnStatChanged.Broadcast(ERPGStatType::Wisdom, Wisdom.GetTotal());
	OnStatChanged.Broadcast(ERPGStatType::Endurance, Endurance.GetTotal());
}

void UStatsComponent::OnRep_AvailableStatPoints()
{
	OnStatPointsChanged.Broadcast(AvailableStatPoints);
}

void UStatsComponent::RecalculateDerivedStats()
{
	float LevelBonuses = FMath::Max(0.0f, (float)(CharacterLevel - 1));

	MaxHealth.BaseValue = 90.0f + (LevelBonuses * 15.0f) + (Endurance.GetTotal() * 10.0f);
	OnMaxHealthCalculated.Broadcast(MaxHealth.GetTotal());

	MaxMana.BaseValue = 95.0f + (LevelBonuses * 10.0f) + (Intelligence.GetTotal() * 5.0f);
	OnMaxManaCalculated.Broadcast(MaxMana.GetTotal());

	MaxStamina.BaseValue = 50.0f + (LevelBonuses * 5.0f) + (Dexterity.GetTotal() * 3.0f);
	OnMaxStaminaCalculated.Broadcast(MaxStamina.GetTotal());

	MeleeAttack.BaseValue = 5.0f + (LevelBonuses * 1.5f) + (Strength.GetTotal() * 2.0f);
	OnStatChanged.Broadcast(ERPGStatType::MeleeAttack, MeleeAttack.GetTotal());

	RangedAttack.BaseValue = 5.0f + (LevelBonuses * 1.5f) + (Dexterity.GetTotal() * 2.0f);
	OnStatChanged.Broadcast(ERPGStatType::RangedAttack, RangedAttack.GetTotal());

	MagicAttack.BaseValue = 5.0f + (LevelBonuses * 1.5f) + (Intelligence.GetTotal() * 2.0f);
	OnStatChanged.Broadcast(ERPGStatType::MagicAttack, MagicAttack.GetTotal());

	MeleeDefense.BaseValue = (LevelBonuses * 2.0f) + (Endurance.GetTotal() * 1.5f);
	OnStatChanged.Broadcast(ERPGStatType::MeleeDefense, MeleeDefense.GetTotal());

	MagicDefense.BaseValue = (LevelBonuses * 2.0f) + (Wisdom.GetTotal() * 1.5f);
	OnStatChanged.Broadcast(ERPGStatType::MagicDefense, MagicDefense.GetTotal());

	MeleeCrit.BaseValue = (LevelBonuses * 0.2f) + (Strength.GetTotal() * 0.5f);
	OnStatChanged.Broadcast(ERPGStatType::MeleeCrit, MeleeCrit.GetTotal());

	MagicCrit.BaseValue = (LevelBonuses * 0.2f) + (Wisdom.GetTotal() * 0.5f);
	OnStatChanged.Broadcast(ERPGStatType::MagicCrit, MagicCrit.GetTotal());
}

void UStatsComponent::ModifyStat(ERPGStatType Stat, float Amount, bool bIsAdding)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

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
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	CharacterLevel = NewLevel;
	AvailableStatPoints++;

	OnStatPointsChanged.Broadcast(AvailableStatPoints);
	RecalculateDerivedStats();
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
	case ERPGStatType::MaxStamina: return MaxStamina.GetTotal();
	case ERPGStatType::MagicAttack: return MagicAttack.GetTotal();
	case ERPGStatType::RangedAttack: return RangedAttack.GetTotal();
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
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	Strength.BaseValue = NewStr;
	Dexterity.BaseValue = NewDex;
	Intelligence.BaseValue = NewInt;
	Wisdom.BaseValue = NewWis;
	Endurance.BaseValue = NewEnd;

	RecalculateDerivedStats();
}

bool UStatsComponent::AllocateStatPoint(ERPGStatType StatType)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		Server_AllocateStatPoint(StatType);
		return false;
	}

	if (AvailableStatPoints <= 0) return false;

	switch (StatType)
	{
	case ERPGStatType::Strength: Strength.BaseValue += 1.0f; break;
	case ERPGStatType::Dexterity: Dexterity.BaseValue += 1.0f; break;
	case ERPGStatType::Intelligence: Intelligence.BaseValue += 1.0f; break;
	case ERPGStatType::Wisdom: Wisdom.BaseValue += 1.0f; break;
	case ERPGStatType::Endurance: Endurance.BaseValue += 1.0f; break;
	default: return false;
	}

	AvailableStatPoints--;
	OnStatPointsChanged.Broadcast(AvailableStatPoints);
	RecalculateDerivedStats();
	OnStatChanged.Broadcast(StatType, GetStatValue(StatType));

	return true;
}

bool UStatsComponent::Server_AllocateStatPoint_Validate(ERPGStatType StatType)
{
	return true;
}

void UStatsComponent::Server_AllocateStatPoint_Implementation(ERPGStatType StatType)
{
	AllocateStatPoint(StatType);
}