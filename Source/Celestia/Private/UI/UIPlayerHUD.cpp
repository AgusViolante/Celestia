#include "UI/UIPlayerHUD.h"
#include "Components/ProgressBar.h"
#include "Components/StaminaComponent.h"
#include "Components/TextBlock.h"

void UUIPlayerHUD::UpdateHealth(UHealthComponent* HealthComp, float CurrentHealth, float MaxHealth, float HealthDelta)
{
    if (HealthBar && MaxHealth > 0.f)
    {
        HealthBar->SetPercent(CurrentHealth / MaxHealth);
    }

    if (HealthText)
    {
        FString HPString = FString::Printf(TEXT("%d / %d"), FMath::RoundToInt(CurrentHealth), FMath::RoundToInt(MaxHealth));
        HealthText->SetText(FText::FromString(HPString));
    }
}

void UUIPlayerHUD::UpdateStamina(UStaminaComponent* StaminaComp, float CurrentStamina, float MaxStamina)
{
    if (StaminaBar && MaxStamina > 0.f)
    {
        StaminaBar->SetPercent(CurrentStamina / MaxStamina);
    }
}

