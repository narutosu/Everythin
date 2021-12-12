// Copyright Epic Games, Inc. All Rights Reserved.

#include "StandaloneWindowCommands.h"

#define LOCTEXT_NAMESPACE "FStandaloneWindowModule"

void FStandaloneWindowCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "StandaloneWindow", "Bring up StandaloneWindow window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
