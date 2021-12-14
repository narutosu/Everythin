// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionRPGLoadingScreen.h"
#include "MoviePlayer.h"

// This module must be loaded "PreLoadingScreen" in the .uproject file, otherwise it will not hook in time!
struct FRPGLoadingScreenBrush : public FSlateDynamicImageBrush, public FGCObject
{
	FRPGLoadingScreenBrush(const FName InTextureName, const FVector2D& InImageSize)
		: FSlateDynamicImageBrush(InTextureName, InImageSize)
	{
		SetResourceObject(LoadObject<UObject>(NULL, *InTextureName.ToString()));
	}

	virtual void AddReferencedObjects(FReferenceCollector& Collector)
	{
		if (UObject* CachedResourceObject = GetResourceObject())
		{
			Collector.AddReferencedObject(CachedResourceObject);
		}
	}
};


void SRPGLoadingScreen::Construct(const FArguments& InArgs)
{
	// Load version of the logo with text baked in, path is hardcoded because this loads very early in startup
	static const FName LoadingScreenName(TEXT("/Game/UI/T_ActionRPG_TransparentLogo.T_ActionRPG_TransparentLogo"));

	LoadingScreenBrush = MakeShareable(new FRPGLoadingScreenBrush(LoadingScreenName, FVector2D(1024, 256)));

	FSlateBrush* BGBrush = new FSlateBrush();
	BGBrush->TintColor = FLinearColor(0.034f, 0.034f, 0.034f, 0.5f);

	ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(BGBrush)
		]
	+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SImage)
			.Image(LoadingScreenBrush.Get())
		]
	+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Right)
		.Padding(FMargin(10.0f))
		[
			SNew(SThrobber)
			.Visibility(this, &SRPGLoadingScreen::GetLoadIndicatorVisibility)
		]
		]
		];
}

/** Rather to show the ... indicator */
EVisibility SRPGLoadingScreen::GetLoadIndicatorVisibility() const
{
	bool Vis = GetMoviePlayer()->IsLoadingFinished();
	return GetMoviePlayer()->IsLoadingFinished() ? EVisibility::Collapsed : EVisibility::Visible;
}
