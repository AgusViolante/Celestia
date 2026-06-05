#include "UI/UINPCDialogue.h"
#include "Characters/NPC/NPCBase.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Widget.h"
#include "Quests/QuestComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/ProgressionComponent.h"

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

	bool bShowQuestBtn = false;
	bIsTurningIn = false;
	bIsTalking = false;
	PendingQuest = nullptr;
	FString QuestBtnText = TEXT("");

	if (!CurrentNPC || !CurrentInteractor) return;

	if (Panel_Main) Panel_Main->SetVisibility(ESlateVisibility::Visible);
	if (Panel_QuestDetails) Panel_QuestDetails->SetVisibility(ESlateVisibility::Collapsed);

	if (Txt_NPCName) Txt_NPCName->SetText(CurrentNPC->NPC_Name);

	if (UQuestComponent* QuestComp = CurrentInteractor->FindComponentByClass<UQuestComponent>())
	{
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

		bIsTalking = false;
		if (!bShowQuestBtn)
		{
			for (const FActiveQuest& ActiveQuest : QuestComp->ActiveQuests)
			{
				if (ActiveQuest.bIsReadyToTurnIn) continue;

				for (const FQuestObjective& Obj : ActiveQuest.CurrentObjectives)
				{
					if (Obj.ObjectiveType == EObjectiveType::Talk && Obj.TargetID == CurrentNPC->NPC_ID && Obj.CurrentAmount < Obj.RequiredAmount)
					{
						bShowQuestBtn = true;
						bIsTalking = true;
						ActiveDialogueLines = Obj.DialogueLines;
						CurrentLineIndex = 0;
						TalkNPC_ID = CurrentNPC->NPC_ID;
						QuestBtnText = TEXT("Hablar");
						break;
					}
				}
				if (bIsTalking) break;
			}
		}

		if (!bShowQuestBtn)
		{
			for (UQuestDataAsset* QuestToGive : CurrentNPC->AvailableQuests)
			{
				if (QuestToGive && QuestToGive->GiverNPC_ID == CurrentNPC->NPC_ID)
				{
					if (QuestComp->CanAcceptQuest(QuestToGive))
					{
						bShowQuestBtn = true;
						PendingQuest = QuestToGive;
						QuestBtnText = TEXT("Nueva Misión");
						break;
					}
				}
			}
		}
	}

	if (Txt_Greeting)
	{
		Txt_Greeting->SetText(CurrentNPC->GreetingMessage);
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
}void UUINPCDialogue::OnQuestClicked()
{
	if (bIsTalking)
	{
		if (Panel_Main) Panel_Main->SetVisibility(ESlateVisibility::Collapsed);
		if (Panel_QuestDetails) Panel_QuestDetails->SetVisibility(ESlateVisibility::Visible);

		if (Txt_QuestTitle) Txt_QuestTitle->SetText(CurrentNPC->NPC_Name); 

		if (ActiveDialogueLines.IsValidIndex(CurrentLineIndex) && Txt_QuestLore)
		{
			Txt_QuestLore->SetText(ActiveDialogueLines[CurrentLineIndex]);
		}

		if (Btn_DeclineQuest) Btn_DeclineQuest->SetVisibility(ESlateVisibility::Collapsed); 

		if (Txt_AcceptBtn)
		{
			if (ActiveDialogueLines.Num() > 1) Txt_AcceptBtn->SetText(FText::FromString(TEXT("Siguiente...")));
			else Txt_AcceptBtn->SetText(FText::FromString(TEXT("Terminar")));
		}
		return; 
	}
	if (!PendingQuest) return;

	if (Panel_Main) Panel_Main->SetVisibility(ESlateVisibility::Collapsed);
	if (Panel_QuestDetails) Panel_QuestDetails->SetVisibility(ESlateVisibility::Visible);

	if (Txt_QuestTitle) Txt_QuestTitle->SetText(CurrentNPC->NPC_Name);

	// 1. CARGAR EL DIÁLOGO CORRECTO SEGÚN EL ESTADO
	if (bIsTurningIn)
	{
		ActiveDialogueLines = PendingQuest->QuestTurnInDialogue;
		if (Btn_DeclineQuest) Btn_DeclineQuest->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		ActiveDialogueLines = PendingQuest->QuestOfferDialogue;
		if (Btn_DeclineQuest) Btn_DeclineQuest->SetVisibility(ESlateVisibility::Visible);
	}

	if (ActiveDialogueLines.Num() == 0)
	{
		ActiveDialogueLines.Add(PendingQuest->QuestDescription);
	}

	CurrentLineIndex = 0;
	if (Txt_QuestLore) Txt_QuestLore->SetText(ActiveDialogueLines[CurrentLineIndex]);

	if (ActiveDialogueLines.Num() > 1)
	{
		if (Txt_AcceptBtn) Txt_AcceptBtn->SetText(FText::FromString(TEXT("Siguiente...")));
	}
	else
	{
		if (Txt_AcceptBtn) Txt_AcceptBtn->SetText(bIsTurningIn ? FText::FromString(TEXT("Recibir Recompensa")) : FText::FromString(TEXT("Aceptar Misión")));
	}
}

void UUINPCDialogue::OnAcceptQuestClicked()
{
	if (bIsTalking)
	{
		CurrentLineIndex++;

		if (CurrentLineIndex < ActiveDialogueLines.Num())
		{
			if (Txt_QuestLore) Txt_QuestLore->SetText(ActiveDialogueLines[CurrentLineIndex]);

			if (CurrentLineIndex == ActiveDialogueLines.Num() - 1)
			{
				if (Txt_AcceptBtn) Txt_AcceptBtn->SetText(FText::FromString(TEXT("Terminar")));
			}
		}
		else
		{
			if (UQuestComponent* QuestComp = CurrentInteractor->FindComponentByClass<UQuestComponent>())
			{
				QuestComp->UpdateObjective(EObjectiveType::Talk, TalkNPC_ID, nullptr, 1);
			}
			OnLeaveClicked();
		}
		return; 
	}
	if (PendingQuest)
	{
	
		CurrentLineIndex++;

		
		if (CurrentLineIndex < ActiveDialogueLines.Num())
		{
			if (Txt_QuestLore) Txt_QuestLore->SetText(ActiveDialogueLines[CurrentLineIndex]);

			
			if (CurrentLineIndex == ActiveDialogueLines.Num() - 1)
			{
				if (Txt_AcceptBtn) Txt_AcceptBtn->SetText(bIsTurningIn ? FText::FromString(TEXT("Recibir Recompensa")) : FText::FromString(TEXT("Aceptar Misión")));
			}
			return; 
		}
		else
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
			OnLeaveClicked();
		}
	}
}

void UUINPCDialogue::OnDeclineQuestClicked()
{
	
	if (Panel_QuestDetails) Panel_QuestDetails->SetVisibility(ESlateVisibility::Collapsed);
	if (Panel_Main) Panel_Main->SetVisibility(ESlateVisibility::Visible);
}

void UUINPCDialogue::OnShopClicked_Implementation() { 
	OnLeaveClicked();
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
	if (CurrentNPC) CurrentNPC->OpenShop(CurrentInteractor);
}
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