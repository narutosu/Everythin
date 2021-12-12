// Copyright Epic Games, Inc. All Rights Reserved.

#include "EditorToolbarButtonCommands.h"

#define LOCTEXT_NAMESPACE "FEditorToolbarButtonModule"

void FEditorToolbarButtonCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "EditorToolbarButton", "Execute EditorToolbarButton action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
