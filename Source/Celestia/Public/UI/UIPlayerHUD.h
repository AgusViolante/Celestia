#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UIPlayerHUD.generated.h"

class UProgressBar;
class UTextBlock;
class UHealthComponent;
class UStaminaComponent;

UCLASS()
class CELESTIA_API UUIPlayerHUD : public UUserWidget
{
    GENERATED_BODY()

public:
   
    UPROPERTY(meta = (BindWidget))
    UProgressBar* HealthBar;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* HealthText;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* StaminaBar;

    UFUNCTION()
    void UpdateStamina(UStaminaComponent* StaminaComp, float CurrentStamina, float MaxStamina);
   
    UFUNCTION()
    void UpdateHealth(UHealthComponent* HealthComp, float CurrentHealth, float MaxHealth, float HealthDelta);

   
};

