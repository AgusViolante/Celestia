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
	MaxStamina		UMETA(DisplayName = "Estamina Maxima"),
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatPointsChangedSignature, int32, NewStatPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxStaminaCalculatedSignature, float, NewMaxStamina);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CELESTIA_API UStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatsComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_PrimaryStats, Category = "Stats | Primary")
	FRPGStat Strength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_PrimaryStats, Category = "Stats | Primary")
	FRPGStat Dexterity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_PrimaryStats, Category = "Stats | Primary")
	FRPGStat Intelligence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_PrimaryStats, Category = "Stats | Primary")
	FRPGStat Wisdom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_PrimaryStats, Category = "Stats | Primary")
	FRPGStat Endurance;

	UFUNCTION()
	void OnRep_PrimaryStats();

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Secondary")
	FRPGStat MaxStamina;

public:
	UPROPERTY(BlueprintAssignable, Category = "Stats | Events")
	FOnMaxManaCalculatedSignature OnMaxManaCalculated;

	UPROPERTY(BlueprintAssignable, Category = "Stats | Events")
	FOnMaxHealthCalculatedSignature OnMaxHealthCalculated;

	UPROPERTY(BlueprintAssignable, Category = "Stats | Events")
	FOnStatChangedSignature OnStatChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stats | Events")
	FOnMaxStaminaCalculatedSignature OnMaxStaminaCalculated;

	UPROPERTY(BlueprintAssignable, Category = "Stats | Events")
	FOnStatPointsChangedSignature OnStatPointsChanged;

	UFUNCTION(BlueprintCallable, Category = "Stats | Logic")
	void RecalculateDerivedStats();

	UFUNCTION(BlueprintCallable, Category = "Stats | Logic")
	void ModifyStat(ERPGStatType Stat, float Amount, bool bIsAdding);

	UFUNCTION(BlueprintCallable, Category = "Stats | Progression")
	void OnLevelUp(int32 NewLevel);

	UFUNCTION(BlueprintPure, Category = "Stats | Logic")
	float GetStatValue(ERPGStatType StatType) const;

	UFUNCTION(BlueprintCallable, Category = "Stats | Logic")
	void SetPrimaryStats(float NewStr, float NewDex, float NewInt, float NewWis, float NewEnd);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_AvailableStatPoints, Category = "Stats | Progression")
	int32 AvailableStatPoints = 0;

	UFUNCTION()
	void OnRep_AvailableStatPoints();

	UFUNCTION(BlueprintCallable, Category = "Stats | Progression")
	bool AllocateStatPoint(ERPGStatType StatType);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_AllocateStatPoint(ERPGStatType StatType);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Stats | Progression")
	int32 CharacterLevel = 1;
};