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

	UQuestComponent* QuestComp = CurrentInteractor->FindComponentByClass<UQuestComponent>();
	if (!QuestComp) return;

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
		int32 PlayerCurrentLevel = 1;
		APawn* PlayerPawn = GetOwningPlayerPawn();
		if (PlayerPawn)
		{
			if (UProgressionComponent* ProgComp = PlayerPawn->FindComponentByClass<UProgressionComponent>())
			{
				PlayerCurrentLevel = ProgComp->CurrentLevel;
			}
		}

		for (UQuestDataAsset* QuestToGive : CurrentNPC->AvailableQuests)
		{
			if (QuestToGive && QuestToGive->GiverNPC_ID == CurrentNPC->NPC_ID)
			{
				if (PlayerCurrentLevel < QuestToGive->RequiredLevel) continue;

				bool bAlreadyHas = false;
				for (const FActiveQuest& AQ : QuestComp->ActiveQuests) { if (AQ.QuestData == QuestToGive) bAlreadyHas = true; }
				bool bCompleted = QuestComp->IsQuestCompleted(QuestToGive->QuestID);

				if (!bAlreadyHas && !bCompleted)
				{
					bShowQuestBtn = true;
					PendingQuest = QuestToGive;
					QuestBtnText = TEXT("Nueva Misión");
					break;
				}
			}
		}
	}

	if (Btn_Quest)
	{
		if (bShowQuestBtn)
		{
			Btn_Quest->SetVisibility(ESlateVisibility::Visible);
			if (Txt_AcceptBtn) Txt_AcceptBtn->SetText(FText::FromString(QuestBtnText));
		}
		else
		{
			Btn_Quest->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
} 

void UUINPCDialogue::OnQuestClicked()
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

void UUINPCDialogue::OnDeclineQuestClicked()
{
	
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