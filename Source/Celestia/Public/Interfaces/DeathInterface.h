#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DeathInterface.generated.h"

UINTERFACE(MinimalAPI)
class UDeathInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * Interfaz para objetos que pueden morir.
 * Usamos BlueprintNativeEvent para permitir implementación nativa en C++.
 */
class IDeathInterface
{
    GENERATED_BODY()

public:
    // Genera IDeathInterface::Execute_Die(...) automáticamente.
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Death")
    void Die();
};