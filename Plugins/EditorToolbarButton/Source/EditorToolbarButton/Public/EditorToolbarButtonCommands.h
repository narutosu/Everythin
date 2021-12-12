// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "EditorToolbarButtonStyle.h"

class FEditorToolbarButtonCommands : public TCommands<FEditorToolbarButtonCommands>
{
public:

	FEditorToolbarButtonCommands()
		: TCommands<FEditorToolbarButtonCommands>(TEXT("EditorToolbarButton"), NSLOCTEXT("Contexts", "EditorToolbarButton", "EditorToolbarButton Plugin"), NAME_None, FEditorToolbarButtonStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
