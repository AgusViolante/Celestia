// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponActor.generated.h"


USTRUCT(BlueprintType)
struct FWeaponStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float BaseDamage = 8.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float DamageMultiplier = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float Range = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float Radius = 40.f;

};
UCLASS()
class CELESTIA_API AWeaponActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponActor();


    // Mesh visible (arma física)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    USkeletalMeshComponent* WeaponMesh;

    // Stats del arma
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FWeaponStats WeaponStats;

    // Socket name donde se adjunta en el Character (por defecto "hand_rSocket" o el que uses)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FName AttachSocketName = FName("hand_rSocket");

    // OwnerCharacter seteado cuando el character equipa el arma
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    ACharacter* OwnerCharacter;

    // Llamado por el AnimNotify (o por Character) para ejecutar el hit
    UFUNCTION()
    void PerformHit(bool bIsAbility = false, float AbilityMultiplier = 1.f);

    // Resetear lista de ya golpeados al iniciar un nuevo swing
    UFUNCTION()
    void ResetAlreadyHit();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame

    TSet<TWeakObjectPtr<AActor>> AlreadyHit;

    float CalculateFinalDamage(AActor* Target, bool bIsAbility, float AbilityMultiplier) const;
};
