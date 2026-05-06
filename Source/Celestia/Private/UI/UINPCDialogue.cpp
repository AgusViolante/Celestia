// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UINPCDialogue.h"

#include "Characters/NPC/NPCBase.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Quests/QuestComponent.h"
#include "GameFramework/PlayerController.h"

void UUINPCDialogue::NativeConstruct()
{
	Super::NativeConstruct();

	// Conectamos los botones a nuestras funciones
	if (Btn_Quest) Btn_Quest->OnClicked.AddDynamic(this, &UUINPCDialogue::OnQuestClicked);
	if (Btn_Shop) Btn_Shop->OnClicked.AddDynamic(this, &UUINPCDialogue::OnShopClicked);
	if (Btn_Craft) Btn_Craft->OnClicked.AddDynamic(this, &UUINPCDialogue::OnCraftClicked);
	if (Btn_Leave) Btn_Leave->OnClicked.AddDynamic(this, &UUINPCDialogue::OnLeaveClicked);
}

void UUINPCDialogue::SetupUI(ANPCBase* InNPC, AActor* InInteractor)
{
	CurrentNPC = InNPC;
	CurrentInteractor = InInteractor;

	if (!CurrentNPC || !CurrentInteractor) return;

	// 1. Configuramos los textos principales
	if (Txt_NPCName) Txt_NPCName->SetText(CurrentNPC->NPC_Name);
	if (Txt_Greeting) Txt_Greeting->SetText(CurrentNPC->GreetingMessage);

	// 2. Lógica del Botón de Misiones (Verificamos si hay algo que hacer)
	bool bShowQuestBtn = false;
	if (UQuestComponent* QuestComp = CurrentInteractor->FindComponentByClass<UQuestComponent>())
	{
		// ¿El NPC tiene misiones para dar?
		if (CurrentNPC->AvailableQuests.Num() > 0) bShowQuestBtn = true;

		// ¿El jugador viene a entregar una misión?
		for (const FActiveQuest& ActiveQuest : QuestComp->ActiveQuests)
		{
			if (ActiveQuest.QuestData && ActiveQuest.QuestData->ReceiverNPC_ID == CurrentNPC->NPC_ID)
			{
				bShowQuestBtn = true;
				if (Txt_QuestBtn) Txt_QuestBtn->SetText(FText::FromString(TEXT("Entregar Misión")));
				break;
			}
		}
	}
	// Ocultamos el botón si este NPC no tiene nada que ver con misiones
	if (Btn_Quest) Btn_Quest->SetVisibility(bShowQuestBtn ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	// 3. Ocultar/Mostrar Botones de Servicios
	if (Btn_Shop) Btn_Shop->SetVisibility(CurrentNPC->bHasShop ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (Btn_Craft) Btn_Craft->SetVisibility(CurrentNPC->bHasCrafting ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	// 4. Pausar el input del jugador y mostrar el mouse
	if (APawn* InteractorPawn = Cast<APawn>(CurrentInteractor))
	{
		if (APlayerController* PC = Cast<APlayerController>(InteractorPawn->GetController()))
		{
			PC->SetShowMouseCursor(true);
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(TakeWidget());
			PC->SetInputMode(InputMode);
		}
	}
}

void UUINPCDialogue::OnQuestClicked()
{
	if (CurrentNPC && CurrentInteractor)
	{
		// Llamamos a la lógica de misiones que movimos al NPC
		CurrentNPC->ProcessQuestInteraction(CurrentInteractor);
	}
	OnLeaveClicked(); // Cerramos la ventana
}

void UUINPCDialogue::OnShopClicked()
{
	if (CurrentNPC && CurrentInteractor)
	{
		CurrentNPC->OpenShop(CurrentInteractor);
	}
	OnLeaveClicked();
}

void UUINPCDialogue::OnCraftClicked()
{
	if (CurrentNPC && CurrentInteractor)
	{
		CurrentNPC->OpenCrafting(CurrentInteractor);
	}
	OnLeaveClicked();
}

void UUINPCDialogue::OnLeaveClicked()
{
	// Restaurar el control al jugador y ocultar el mouse
	if (CurrentInteractor)
	{
		if (APawn* InteractorPawn = Cast<APawn>(CurrentInteractor))
		{
			if (APlayerController* PC = Cast<APlayerController>(InteractorPawn->GetController()))
			{
				PC->SetShowMouseCursor(false);
				PC->SetInputMode(FInputModeGameOnly());
			}
		}
	}

	RemoveFromParent();
}