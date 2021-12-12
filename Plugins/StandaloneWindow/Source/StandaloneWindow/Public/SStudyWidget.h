#pragma once
#include "CoreMinimal.h"
#include "StandaloneWindow.h"
#include "Widgets/SUserWidget.h"

class SStudyWidget : public SUserWidget
{
public:
	SLATE_USER_ARGS(SStudyWidget)
	{}
	SLATE_END_ARGS()

	// MUST Provide this function for SNew to call!
	virtual void Construct(const FArguments& InArgs);
private:
	TSharedPtr<class SSlider> Slider;

	TArray<TSharedPtr<class SCheckBox>> CheckBoxs;
	TArray<TSharedPtr<FString>> Options;
	TSharedPtr<class STextBlock> ComboText;
};