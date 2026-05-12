// Fill out your copyright notice in the Description page of Project Settings.


#include "Quests/UIQuestSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UUIQuestSlot::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Select)
	{
		Btn_Select->OnClicked.AddDynamic(this, &UUIQuestSlot::OnButtonClicked);
	}
}

void UUIQuestSlot::SetupSlot(const FActiveQuest& InQuest)
{
	MyQuest = InQuest;

	if (Txt_QuestName && MyQuest.QuestData)
	{
		Txt_QuestName->SetText(MyQuest.QuestData->QuestName);
	}
}

void UUIQuestSlot::OnButtonClicked()
{
	OnQuestSlotClicked.Broadcast(MyQuest);
}
