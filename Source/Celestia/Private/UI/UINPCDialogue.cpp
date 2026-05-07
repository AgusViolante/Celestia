#include "UI/UINPCDialogue.h"
#include "Characters/NPC/NPCBase.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Widget.h"
#include "Quests/QuestComponent.h"
#include "GameFramework/PlayerController.h"

void UUINPCDialogue::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Quest) Btn_Quest->OnClicked.AddDynamic(this, &UUINPCDialogue::OnQuestClicked);
	if (Btn_Shop) Btn_Shop->OnClicked.AddDynamic(this, &UUINPCDialogue::OnShopClicked);
	if (Btn_Craft) Btn_Craft->OnClicked.AddDynamic(this, &UUINPCDialogue::OnCraftClicked);
	if (Btn_Leave) Btn_Leave->OnClicked.AddDynamic(this, &UUINPCDialogue::OnLeaveClicked);

	if (Btn_AcceptQuest) Btn_AcceptQuest->OnClicked.AddDynamic(this, &UUINPCDialogue::OnAcceptQuestClicked);
	if (Btn_DeclineQuest) Btn_DeclineQuest->OnClicked.AddDynamic(this, &UUINPCDialogue::OnDeclineQuestClicked);
}

void UUINPCDialogue::SetupUI(ANPCBase* InNPC, AActor* InInteractor)
{
	CurrentNPC = InNPC;
	CurrentInteractor = InInteractor;
	PendingQuest = nullptr;

	if (!CurrentNPC || !CurrentInteractor) return;

	// Estado inicial de los paneles
	if (Panel_Main) Panel_Main->SetVisibility(ESlateVisibility::Visible);
	if (Panel_QuestDetails) Panel_QuestDetails->SetVisibility(ESlateVisibility::Collapsed);

	if (Txt_NPCName) Txt_NPCName->SetText(CurrentNPC->NPC_Name);
	if (Txt_Greeting) Txt_Greeting->SetText(CurrentNPC->GreetingMessage);

	bool bShowQuestBtn = false;
	FString QuestBtnText = TEXT("Misiones");

	if (UQuestComponent* QuestComp = CurrentInteractor->FindComponentByClass<UQuestComponent>())
	{
		// 1. ¿Venimos a entregar una misión?
		for (const FActiveQuest& ActiveQuest : QuestComp->ActiveQuests)
		{
			if (ActiveQuest.bIsReadyToTurnIn && ActiveQuest.QuestData && ActiveQuest.QuestData->ReceiverNPC_ID == CurrentNPC->NPC_ID)
			{
				bShowQuestBtn = true;
				bIsTurningIn = true;
				PendingQuest = ActiveQuest.QuestData;
				QuestBtnText = TEXT("Entregar Misión");
				break;
			}
		}

		// 2. Si no entregamos nada, ¿hay una misión nueva para dar?
		if (!bShowQuestBtn)
		{
			for (UQuestDataAsset* QuestToGive : CurrentNPC->AvailableQuests)
			{
				if (QuestToGive && QuestToGive->GiverNPC_ID == CurrentNPC->NPC_ID)
				{
					// Verificamos que no la tenga ya, ni la haya completado antes
					bool bAlreadyHas = false;
					for (const FActiveQuest& AQ : QuestComp->ActiveQuests) { if (AQ.QuestData == QuestToGive) bAlreadyHas = true; }
					bool bCompleted = QuestComp->IsQuestCompleted(QuestToGive->QuestID);

					if (!bAlreadyHas && !bCompleted)
					{
						bShowQuestBtn = true;
						bIsTurningIn = false;
						PendingQuest = QuestToGive;
						QuestBtnText = TEXT("Nueva Misión");
						break;
					}
				}
			}
		}
	}

	if (Btn_Quest) Btn_Quest->SetVisibility(bShowQuestBtn ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (Txt_QuestBtn) Txt_QuestBtn->SetText(FText::FromString(QuestBtnText));

	if (Btn_Shop) Btn_Shop->SetVisibility(CurrentNPC->bHasShop ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (Btn_Craft) Btn_Craft->SetVisibility(CurrentNPC->bHasCrafting ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

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
	if (!PendingQuest) return;

	// Ocultamos el saludo y mostramos el Lore
	if (Panel_Main) Panel_Main->SetVisibility(ESlateVisibility::Collapsed);
	if (Panel_QuestDetails) Panel_QuestDetails->SetVisibility(ESlateVisibility::Visible);

	if (Txt_QuestTitle) Txt_QuestTitle->SetText(PendingQuest->QuestName);
	if (Txt_QuestLore) Txt_QuestLore->SetText(PendingQuest->QuestDescription);

	if (bIsTurningIn)
	{
		if (Txt_AcceptBtn) Txt_AcceptBtn->SetText(FText::FromString(TEXT("Recibir Recompensa")));
		if (Btn_DeclineQuest) Btn_DeclineQuest->SetVisibility(ESlateVisibility::Collapsed); // No puedes rechazar una entrega
	}
	else
	{
		if (Txt_AcceptBtn) Txt_AcceptBtn->SetText(FText::FromString(TEXT("Aceptar Misión")));
		if (Btn_DeclineQuest) Btn_DeclineQuest->SetVisibility(ESlateVisibility::Visible);
	}
}

void UUINPCDialogue::OnAcceptQuestClicked()
{
	if (UQuestComponent* QuestComp = CurrentInteractor->FindComponentByClass<UQuestComponent>())
	{
		if (bIsTurningIn)
		{
			QuestComp->TurnInQuest(PendingQuest);
		}
		else
		{
			QuestComp->AcceptQuest(PendingQuest);
		}
	}
	OnLeaveClicked(); // Cerramos la UI
}

void UUINPCDialogue::OnDeclineQuestClicked()
{
	// Si rechaza, volvemos al menú principal del NPC
	if (Panel_QuestDetails) Panel_QuestDetails->SetVisibility(ESlateVisibility::Collapsed);
	if (Panel_Main) Panel_Main->SetVisibility(ESlateVisibility::Visible);
}

void UUINPCDialogue::OnShopClicked() { if (CurrentNPC) CurrentNPC->OpenShop(CurrentInteractor); OnLeaveClicked(); }
void UUINPCDialogue::OnCraftClicked() { if (CurrentNPC) CurrentNPC->OpenCrafting(CurrentInteractor); OnLeaveClicked(); }

void UUINPCDialogue::OnLeaveClicked()
{
	if (APawn* InteractorPawn = Cast<APawn>(CurrentInteractor))
	{
		if (APlayerController* PC = Cast<APlayerController>(InteractorPawn->GetController()))
		{
			PC->SetShowMouseCursor(false);
			PC->SetInputMode(FInputModeGameOnly());
		}
	}
	RemoveFromParent();
}