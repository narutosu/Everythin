// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "StandaloneWindowStyle.h"

class FStandaloneWindowCommands : public TCommands<FStandaloneWindowCommands>
{
public:

	FStandaloneWindowCommands()
		: TCommands<FStandaloneWindowCommands>(TEXT("StandaloneWindow"), NSLOCTEXT("Contexts", "StandaloneWindow", "StandaloneWindow Plugin"), NAME_None, FStandaloneWindowStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};