// Copyright Epic Games, Inc. All Rights Reserved.

#include "SMyWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SButton.h"

void SMyWidget::Construct(const FArguments& InArgs)
{
    int32 BoxId = 0;
    CheckBoxs.SetNum(2);
    if (Options.Num() <= 0) {
        Options.Add(MakeShareable(new FString(TEXT("0"))));
        Options.Add(MakeShareable(new FString(TEXT("1"))));
        Options.Add(MakeShareable(new FString(TEXT("2"))));
    }
    SUserWidget::Construct(
        SUserWidget::FArguments().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
        [
            SNew(SGridPanel)
            .FillColumn(0, 1).FillColumn(1, 3)
            .FillRow(0, 1).FillRow(1, 1).FillRow(2, 1).FillRow(3, 1)
            + SGridPanel::Slot(0, 0).HAlign(HAlign_Center).VAlign(VAlign_Center)
            [
                SNew(STextBlock).Text(FText::FromString(TEXT("百分比")))
            ]
            + SGridPanel::Slot(0, 1).HAlign(HAlign_Center).VAlign(VAlign_Center)
            [
                SNew(STextBlock).Text(FText::FromString(TEXT("二选一")))
            ]
            + SGridPanel::Slot(0, 2).HAlign(HAlign_Center).VAlign(VAlign_Center)
            [
                SNew(STextBlock).Text(FText::FromString(TEXT("下拉选项")))
            ]
            + SGridPanel::Slot(1, 0).HAlign(HAlign_Fill).VAlign(VAlign_Center)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(3)
                [
                    SAssignNew(Slider, SSlider)
                ]
                + SHorizontalBox::Slot().FillWidth(1)
                [
                    SNew(STextBlock).Text_Lambda(
                        [this]()
                        {
                            return FText::FromString(FString::SanitizeFloat(Slider.IsValid() ? Slider->GetValue() : 0.f));
                        }
                    )
                ]
            ]
            + SGridPanel::Slot(1, 1).HAlign(HAlign_Fill).VAlign(VAlign_Fill)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1)
                [
                    SAssignNew(CheckBoxs[0], SCheckBox)
                    .IsChecked(ECheckBoxState::Checked)
                    .OnCheckStateChanged(FOnCheckStateChanged::CreateLambda(
                        [this](ECheckBoxState NewState)
                        {
                            switch (NewState)
                            {
                            case ECheckBoxState::Unchecked:
                                CheckBoxs[0]->SetIsChecked(ECheckBoxState::Checked);
                                break;
                            case ECheckBoxState::Checked:
                                for (int i = 0; i < CheckBoxs.Num(); ++i) {
                                    if (i != 0 && CheckBoxs[i].IsValid()) {
                                        CheckBoxs[i]->SetIsChecked(ECheckBoxState::Unchecked);
                                    }
                                }
                                break;
                            case ECheckBoxState::Undetermined:
                                break;
                            default:
                                break;
                            }
                        }
                    ))
                    [
                        SNew(STextBlock).Text(FText::FromString(TEXT("选项一")))
                    ]
                ]
                + SHorizontalBox::Slot().FillWidth(1)
                [
                    SAssignNew(CheckBoxs[1], SCheckBox)
                    .OnCheckStateChanged(FOnCheckStateChanged::CreateLambda(
                        [this](ECheckBoxState NewState)
                        {
                            switch (NewState)
                            {
                            case ECheckBoxState::Unchecked:
                                CheckBoxs[1]->SetIsChecked(ECheckBoxState::Checked);
                                break;
                            case ECheckBoxState::Checked:
                                for (int i = 0; i < CheckBoxs.Num(); ++i) {
                                    if (i != 1 && CheckBoxs[i].IsValid()) {
                                        CheckBoxs[i]->SetIsChecked(ECheckBoxState::Unchecked);
                                    }
                                }
                                break;
                            case ECheckBoxState::Undetermined:
                                break;
                            default:
                                break;
                            }
                        }
                    ))
                    [
                        SNew(STextBlock).Text(FText::FromString(TEXT("选项二")))
                    ]
                ]
            ]
            + SGridPanel::Slot(1, 2).HAlign(HAlign_Fill).VAlign(VAlign_Center)
            [
                SNew(SComboBox<TSharedPtr<FString>>)
                .OnGenerateWidget_Lambda(
                    [](TSharedPtr<FString> Item)
                    {
                        return SNew(STextBlock).Text(FText::FromString(*Item));
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
                .OptionsSource(&Options)
                [
                    SAssignNew(ComboText, STextBlock)
                ]
            ]
            + SGridPanel::Slot(1, 3).HAlign(HAlign_Center).VAlign(VAlign_Center)
            [
                SNew(SButton)
                [
                    SNew(STextBlock).Text(FText::FromString(TEXT("确定")))
                ]
            ]
        ]
    );
}

TSharedRef<SMyWidget> SMyWidget::New()
{
    return MakeShareable(new SMyWidget());
}
