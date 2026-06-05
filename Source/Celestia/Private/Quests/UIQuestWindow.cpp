// Fill out your copyright notice in the Description page of Project Settings.


#include "Quests/UIQuestWindow.h"
#include "Quests/UIQuestSlot.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Button.h"
#include "Components/ProgressionComponent.h"
#include "GameFramework/PlayerController.h"

void UUIQuestWindow::UpdateQuestList()
{
	if (!ActiveQuestsContainer || !QuestSlotClass) return;

	ActiveQuestsContainer->ClearChildren();
	if (AvailableQuestsContainer) AvailableQuestsContainer->ClearChildren();
	if (CompletedQuestsContainer) CompletedQuestsContainer->ClearChildren();

	ClearDetailsUI();

	APlayerController* PC = GetOwningPlayer();
	if (!PC || !PC->GetPawn()) return;

	UQuestComponent* QuestComp = PC->GetPawn()->FindComponentByClass<UQuestComponent>();
	if (!QuestComp) return;

    UProgressionComponent* ProgComp = PC->GetPawn()->FindComponentByClass<UProgressionComponent>();
    int32 PlayerLevel = ProgComp ? ProgComp->CurrentLevel : 1;

	for (UQuestDataAsset* QuestAsset : AllGameQuests)
	{
		if (!QuestAsset) continue;

		bool bIsCompleted = QuestComp->IsQuestCompleted(QuestAsset->QuestID);
		FActiveQuest ActiveQuestData;
		bool bIsActive = QuestComp->GetActiveQuestData(QuestAsset, ActiveQuestData);

		UUIQuestSlot* NewSlot = CreateWidget<UUIQuestSlot>(this, QuestSlotClass);
		if (NewSlot)
		{
			if (bIsActive)
			{
				NewSlot->SetupSlot(ActiveQuestData);
				ActiveQuestsContainer->AddChild(NewSlot);
			}
			else if (bIsCompleted && CompletedQuestsContainer)
			{
				FActiveQuest CompletedDummy(QuestAsset);
				NewSlot->SetupSlot(CompletedDummy);
				CompletedQuestsContainer->AddChild(NewSlot);
			}
			else if (!bIsActive && !bIsCompleted && AvailableQuestsContainer)
			{
                if (PlayerLevel >= QuestAsset->RequiredLevel)
                {
                    FActiveQuest AvailableDummy(QuestAsset);
                    NewSlot->SetupSlot(AvailableDummy);
                    AvailableQuestsContainer->AddChild(NewSlot);
                }
			}

			NewSlot->OnQuestSlotClicked.AddDynamic(this, &UUIQuestWindow::OnQuestSelected);
		}
	}
}

void UUIQuestWindow::OnQuestSelected(const FActiveQuest& SelectedQuest)
{
    if (!SelectedQuest.QuestData) return;

    CurrentlyViewedQuest = SelectedQuest.QuestData;

    ClearDetailsUI();

    FString StatusStr;
    FLinearColor StatusCol;
    UQuestComponent* QuestComp = GetOwningPlayerPawn()->FindComponentByClass<UQuestComponent>();

    if (QuestComp && QuestComp->IsQuestCompleted(SelectedQuest.QuestData->QuestID))
    {
        StatusStr = TEXT("Completada");
        StatusCol = CompletedColor; 
    }
    else if (SelectedQuest.bIsReadyToTurnIn)
    {
        StatusStr = TEXT("Lista para entregar");
        StatusCol = FLinearColor::Green;
    }
    else if (SelectedQuest.CurrentObjectives.Num() > 0)
    {
        StatusStr = TEXT("En progreso");
        StatusCol = ActiveColor; 
    }
    else
    {
        StatusStr = TEXT("Disponible");
        StatusCol = AccentColor; 
    }


    if (Txt_DetailName)
    {
        Txt_DetailName->SetText(SelectedQuest.QuestData->QuestName);
        Txt_DetailName->SetFont(TitleFont); 
        Txt_DetailName->SetColorAndOpacity(FSlateColor(AccentColor)); 
    }

    if (Txt_DetailDescription)
    {
        Txt_DetailDescription->SetText(SelectedQuest.QuestData->QuestDescription);
        Txt_DetailDescription->SetFont(BodyFont); 
        Txt_DetailDescription->SetColorAndOpacity(FSlateColor(ActiveColor));
    }

    if (Txt_GiverNPC)
    {
        Txt_GiverNPC->SetText(FText::FromString(FString::Printf(TEXT("Otorga: %s"), *SelectedQuest.QuestData->GiverNPC_ID.ToString())));
        Txt_GiverNPC->SetFont(BodyFont);
        Txt_GiverNPC->SetColorAndOpacity(FSlateColor(CompletedColor));
    }
    if (Txt_ReceiverNPC)
    {
        Txt_ReceiverNPC->SetText(FText::FromString(FString::Printf(TEXT("Entregar a: %s"), *SelectedQuest.QuestData->ReceiverNPC_ID.ToString())));
        Txt_ReceiverNPC->SetFont(BodyFont);
        Txt_ReceiverNPC->SetColorAndOpacity(FSlateColor(CompletedColor));
    }

    if (Txt_QuestType)
    {
        FString TypeStr = (SelectedQuest.QuestData->QuestType == EQuestType::Main) ? TEXT("Misión Principal") : TEXT("Misión Secundaria");
        Txt_QuestType->SetText(FText::FromString(TypeStr));
        Txt_QuestType->SetColorAndOpacity(FSlateColor(AccentColor));

        if (Txt_QuestStatus)
        {
            Txt_QuestStatus->SetText(FText::FromString(StatusStr));
            Txt_QuestStatus->SetFont(TitleFont);
            Txt_QuestStatus->SetColorAndOpacity(FSlateColor(StatusCol));
        }

        const TArray<FQuestObjective>& ObjectivesToRead = (SelectedQuest.CurrentObjectives.Num() > 0) ? SelectedQuest.CurrentObjectives : SelectedQuest.QuestData->Objectives;


        for (const FQuestObjective& Obj : ObjectivesToRead)
        {
            FString TargetName = Obj.TargetDisplayName.IsEmpty() ? TEXT("???") : Obj.TargetDisplayName.ToString();

            FString ObjString;
            if (StatusStr == TEXT("Completada"))
            {
                ObjString = FString::Printf(TEXT("• %s: %d / %d"), *TargetName, Obj.RequiredAmount, Obj.RequiredAmount);
            }
            else
            {
                ObjString = FString::Printf(TEXT("• %s: %d / %d"), *TargetName, Obj.CurrentAmount, Obj.RequiredAmount);
            }

            bool bObjCompleted = (StatusStr == TEXT("Completada")) || (Obj.CurrentAmount >= Obj.RequiredAmount);
            CreateObjectiveRow(ObjString, bObjCompleted);
        }

        if (SelectedQuest.QuestData->Rewards.Coins > 0) CreateRewardRow(TEXT("Monedas"), SelectedQuest.QuestData->Rewards.Coins);
        if (SelectedQuest.QuestData->Rewards.Experience > 0.f) CreateRewardRow(TEXT("Experiencia"), FMath::RoundToInt(SelectedQuest.QuestData->Rewards.Experience));

        for (const FItemReward& ItemReward : SelectedQuest.QuestData->Rewards.RewardItems)
        {
            if (ItemReward.ItemClass)
            {
                FString ItemName = ItemReward.ItemDisplayName.IsEmpty() ? ItemReward.ItemClass->GetName().Replace(TEXT("BP_"), TEXT("")).Replace(TEXT("_C"), TEXT("")) : ItemReward.ItemDisplayName.ToString();

                CreateRewardRow(ItemName, ItemReward.Quantity);
            }
        }
    }
}
    void UUIQuestWindow::CreateObjectiveRow(const FString & ObjectiveText, bool bIsCompleted)
    {
    if (!ObjectivesContainer) return;

    UHorizontalBox* RowBox = NewObject<UHorizontalBox>(this);

    UTextBlock* TxtObj = NewObject<UTextBlock>(this);
    TxtObj->SetText(FText::FromString(ObjectiveText));

    TxtObj->SetFont(BodyFont);

    if (bIsCompleted)
    {
        TxtObj->SetColorAndOpacity(FSlateColor(CompletedColor));
    }
    else
    {
        TxtObj->SetColorAndOpacity(FSlateColor(ActiveColor));
    }

    UHorizontalBoxSlot* TextSlot = RowBox->AddChildToHorizontalBox(TxtObj);
    if (TextSlot)
    {
        TextSlot->SetPadding(FMargin(15.f, 5.f, 0.f, 5.f));
        TextSlot->SetVerticalAlignment(VAlign_Center);
    }

    ObjectivesContainer->AddChild(RowBox);
}

void UUIQuestWindow::CreateRewardRow(const FString & RewardName, int32 Quantity)
{
    if (!RewardsContainer) return;

    UHorizontalBox* RowBox = NewObject<UHorizontalBox>(this);

    UTextBlock* TxtReward = NewObject<UTextBlock>(this);

    TxtReward->SetText(FText::FromString(FString::Printf(TEXT("x%d  %s"), Quantity, *RewardName)));

    TxtReward->SetFont(BodyFont);
    TxtReward->SetColorAndOpacity(FSlateColor(ActiveColor));

    UHorizontalBoxSlot* TextSlot = RowBox->AddChildToHorizontalBox(TxtReward);
    if (TextSlot)
    {
        TextSlot->SetPadding(FMargin(20.f, 4.f, 0.f, 4.f));
        TextSlot->SetVerticalAlignment(VAlign_Center);
    }

    RewardsContainer->AddChild(RowBox);
}

void UUIQuestWindow::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Btn_TrackQuest)
    {
        Btn_TrackQuest->OnClicked.AddDynamic(this, &UUIQuestWindow::OnTrackButtonClicked);
    }
}

void UUIQuestWindow::OnTrackButtonClicked()
{
    if (!CurrentlyViewedQuest) return;

    APlayerController* PC = GetOwningPlayer();
    if (!PC || !PC->GetPawn()) return;

    UQuestComponent* QuestComp = PC->GetPawn()->FindComponentByClass<UQuestComponent>();
    if (QuestComp)
    {
        QuestComp->TrackQuest(CurrentlyViewedQuest);
    }
}

void UUIQuestWindow::ClearDetailsUI()
{
	if (Txt_DetailName) Txt_DetailName->SetText(FText::FromString(TEXT("Selecciona una misión")));
	if (Txt_DetailDescription) Txt_DetailDescription->SetText(FText::GetEmpty());
	if (Txt_QuestType) Txt_QuestType->SetText(FText::GetEmpty());
	if (Txt_GiverNPC) Txt_GiverNPC->SetText(FText::GetEmpty());
	if (Txt_ReceiverNPC) Txt_ReceiverNPC->SetText(FText::GetEmpty());
	if (Txt_QuestStatus) Txt_QuestStatus->SetText(FText::GetEmpty());
	if (ObjectivesContainer) ObjectivesContainer->ClearChildren();
	if (RewardsContainer) RewardsContainer->ClearChildren();
}