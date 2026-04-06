// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatsComponent.generated.h"

UENUM(BlueprintType)
enum class ERPGStatType : uint8
{
	Strength		UMETA(DisplayName = "Fuerza (STR)"),
	Dexterity		UMETA(DisplayName = "Destreza (DEX)"),
	Intelligence	UMETA(DisplayName = "Inteligencia (INT)"),
	Wisdom			UMETA(DisplayName = "Sabiduría (WIS)"),
	Endurance		UMETA(DisplayName = "Resistencia (END)"),
	MaxHealth		UMETA(DisplayName = "Salud Maxima"),
	MaxMana			UMETA(DisplayName = "Mana Maximo"),
	MeleeAttack		UMETA(DisplayName = "Ataque Melee"),
	RangedAttack	UMETA(DisplayName = "Ataque a Distancia"),
	MagicAttack		UMETA(DisplayName = "Ataque Magico"),
	MeleeDefense	UMETA(DisplayName = "Defensa Melee"),
	MagicDefense	UMETA(DisplayName = "Defensa Magica"),
	MeleeCrit		UMETA(DisplayName = "Critico Fisico (%)"),
	MagicCrit		UMETA(DisplayName = "Critico Magico (%)")
};

USTRUCT(BlueprintType)
struct FRPGStat
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float BaseValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	float ModifierValue;

	FRPGStat() : BaseValue(0.f), ModifierValue(0.f) {}
	FRPGStat(float InBase) : BaseValue(InBase), ModifierValue(0.f) {}

	float GetTotal() const { return BaseValue + ModifierValue; }
	void AddModifier(float Value) { ModifierValue += Value; }
	void RemoveModifier(float Value) { ModifierValue -= Value; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxManaCalculatedSignature, float, NewMaxMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthCalculatedSignature, float, NewMaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatChangedSignature, ERPGStatType, StatType, float, NewTotalValue);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CELESTIA_API UStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatsComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Primary")
	FRPGStat Strength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Primary")
	FRPGStat Dexterity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Primary")
	FRPGStat Intelligence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Primary")
	FRPGStat Wisdom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Primary")
	FRPGStat Endurance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Secondary")
	FRPGStat MaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Secondary")
	FRPGStat MaxMana;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Secondary")
	FRPGStat MeleeAttack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Secondary")
	FRPGStat RangedAttack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Secondary")
	FRPGStat MagicAttack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Secondary")
	FRPGStat MeleeDefense;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Secondary")
	FRPGStat MagicDefense;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Secondary")
	FRPGStat MeleeCrit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Secondary")
	FRPGStat MagicCrit;

public:
	UPROPERTY(BlueprintAssignable, Category = "Stats | Events")
	FOnMaxManaCalculatedSignature OnMaxManaCalculated;

	UPROPERTY(BlueprintAssignable, Category = "Stats | Events")
	FOnMaxHealthCalculatedSignature OnMaxHealthCalculated;

	UPROPERTY(BlueprintAssignable, Category = "Stats | Events")
	FOnStatChangedSignature OnStatChanged;

	UFUNCTION(BlueprintCallable, Category = "Stats | Logic")
	void RecalculateDerivedStats();

	UFUNCTION(BlueprintCallable, Category = "Stats | Logic")
	void ModifyStat(ERPGStatType Stat, float Amount, bool bIsAdding);

	UFUNCTION(BlueprintCallable, Category = "Stats | Progression")
	void OnLevelUp(int32 NewLevel);

	float GetStatValue(ERPGStatType StatType) const;
};