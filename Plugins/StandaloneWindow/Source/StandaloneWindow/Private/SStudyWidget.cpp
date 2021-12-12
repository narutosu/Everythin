// Copyright Epic Games, Inc. All Rights Reserved.

#include "SStudyWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SButton.h"
#include "Chaos/Array.h"
#include "Input/Reply.h"
void SStudyWidget::Construct(const FArguments& InArgs)
{
	CheckBoxs.SetNum(2);
	if (Options.Num() <= 0) {
		Options.Add(MakeShareable(new FString(TEXT("choice1"))));
		Options.Add(MakeShareable(new FString(TEXT("choice2"))));
		Options.Add(MakeShareable(new FString(TEXT("choice3"))));
	}
	SUserWidget::Construct(
		SUserWidget::FArguments().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
		[
			SNew(SGridPanel)
			.FillColumn(0, 1).FillColumn(1, 2).FillColumn(2, 1)
			.FillRow(0, 1).FillRow(1, 1).FillRow(2, 1).FillRow(3, 1)
			+ SGridPanel::Slot(0, 0).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("percent")))
			]
			+ SGridPanel::Slot(0, 1).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("one from twp")))
			]
			+ SGridPanel::Slot(0, 2).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("pull choice")))
			]
			+ SGridPanel::Slot(1, 0).HAlign(HAlign_Fill).VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1).HAlign(HAlign_Fill)
				[
					SAssignNew(Slider,SSlider)
				]
				+ SHorizontalBox::Slot().FillWidth(1).HAlign(HAlign_Center)
				[
					SNew(STextBlock).Text_Lambda(
						[this]()
						{
							return FText::FromString(FString::SanitizeFloat(Slider.IsValid() ? Slider->GetValue() : 0.f));
						}
					)
				]
			]
			+ SGridPanel::Slot(1, 1).HAlign(HAlign_Fill).VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1).HAlign(HAlign_Center)
				[
					SAssignNew(CheckBoxs[0],SCheckBox)
					.OnCheckStateChanged(FOnCheckStateChanged::CreateLambda(
						[this](ECheckBoxState state) {
							UE_LOG(StandaloneWindow, Warning, TEXT("check box1 change %d"), state);
							CheckBoxs[0]->SetIsChecked(state==ECheckBoxState::Checked? state: ECheckBoxState::Unchecked);
							CheckBoxs[1]->SetIsChecked(state == ECheckBoxState::Checked ?ECheckBoxState::Unchecked: state);
						}
					)
					)
				]
				+ SHorizontalBox::Slot().FillWidth(1).HAlign(HAlign_Center)
				[
					SAssignNew(CheckBoxs[1], SCheckBox)
					.OnCheckStateChanged(FOnCheckStateChanged::CreateLambda(
						[this](ECheckBoxState state) {
							UE_LOG(StandaloneWindow, Warning, TEXT("check box2 change %d"), state);
							CheckBoxs[1]->SetIsChecked(state == ECheckBoxState::Checked ? state : ECheckBoxState::Unchecked);
							CheckBoxs[0]->SetIsChecked(state == ECheckBoxState::Checked ? ECheckBoxState::Unchecked : state);
						}
					)
					)
				]
			]
			+ SGridPanel::Slot(1, 2).HAlign(HAlign_Fill).VAlign(VAlign_Center)
			[
				SNew(SComboBox < TSharedPtr<FString>>).OptionsSource(&Options)
				.OnGenerateWidget_Lambda(
					[](TSharedPtr<FString> Item)
					{
						UE_LOG(StandaloneWindow, Warning, TEXT("OnGenerateWidget_Lambda %s"), **Item);
						return SNew(STextBlock).Text(FText::FromString(TEXT("not choice")));
					}
				)
				.OnSelectionChanged_Lambda(
					[this](TSharedPtr<FString> Item, ESelectInfo::Type Type)
					{
						if (ComboText.IsValid()) {
							ComboText->SetText(FText::FromString(*Item));
						}
					}
				)
				[
					SAssignNew(ComboText, STextBlock)
				]
			]
			+ SGridPanel::Slot(1, 3).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(SButton)
				.OnClicked_Lambda([&](){
					UE_LOG(StandaloneWindow, Warning, TEXT("Ok click"));
					return FReply::Handled();
				})
				[
					SNew(STextBlock).Text(FText::FromString("OK"))
				]
			]
		]
	);
}

TSharedRef<SStudyWidget> SStudyWidget::New()
{
	return MakeShareable(new SStudyWidget());
}