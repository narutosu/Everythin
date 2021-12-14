// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "SlateBasics.h"
#include "SlateExtras.h"
class SRPGLoadingScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRPGLoadingScreen) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
private:
	EVisibility GetLoadIndicatorVisibility() const;
		/** Loading screen image brush */
	TSharedPtr<FSlateDynamicImageBrush> LoadingScreenBrush;
};
