// Copyright Epic Games, Inc. All Rights Reserved.

#include "EditorToolbarButton.h"
#include "EditorToolbarButtonStyle.h"
#include "EditorToolbarButtonCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

static const FName EditorToolbarButtonTabName("EditorToolbarButton");

#define LOCTEXT_NAMESPACE "FEditorToolbarButtonModule"

void FEditorToolbarButtonModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
// 	FEditorToolbarButtonStyle::Initialize();
// 	FEditorToolbarButtonStyle::ReloadTextures();
// 
// 	FEditorToolbarButtonCommands::Register();
// 	
// 	PluginCommands = MakeShareable(new FUICommandList);
// 
// 	PluginCommands->MapAction(
// 		FEditorToolbarButtonCommands::Get().PluginAction,
// 		FExecuteAction::CreateRaw(this, &FEditorToolbarButtonModule::PluginButtonClicked),
// 		FCanExecuteAction());
// 
// 	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FEditorToolbarButtonModule::RegisterMenus));
}

void FEditorToolbarButtonModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

// 	UToolMenus::UnRegisterStartupCallback(this);
// 
// 	UToolMenus::UnregisterOwner(this);
// 
// 	FEditorToolbarButtonStyle::Shutdown();
// 
// 	FEditorToolbarButtonCommands::Unregister();
}

void FEditorToolbarButtonModule::PluginButtonClicked()
{
	// Put your "OnButtonClicked" stuff here
	FText DialogText = FText::Format(
							LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
							FText::FromString(TEXT("FEditorToolbarButtonModule::PluginButtonClicked()")),
							FText::FromString(TEXT("EditorToolbarButton.cpp"))
					   );
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);
}

void FEditorToolbarButtonModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FEditorToolbarButtonCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FEditorToolbarButtonCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEditorToolbarButtonModule, EditorToolbarButton)