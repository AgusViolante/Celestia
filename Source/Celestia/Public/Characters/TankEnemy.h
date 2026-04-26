// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Characters/MeleeEnemy.h" // Heredamos de MeleeEnemy
#include "TankEnemy.generated.h"

UCLASS()
class CELESTIA_API ATankEnemy : public AMeleeEnemy
{
	GENERATED_BODY()

public:
	ATankEnemy();

protected:
	virtual void BeginPlay() override;
};