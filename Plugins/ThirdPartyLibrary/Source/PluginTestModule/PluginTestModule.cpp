// Copyright Epic Games, Inc. All Rights Reserved.

#include "PluginTestModule.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(PluginTestModuleLog);

#define LOCTEXT_NAMESPACE "FPluginTestModule"

void FPluginTestModule::StartupModule()
{
	UE_LOG(PluginTestModuleLog, Warning, TEXT("FPluginTestModule module has started!"));

}

void FPluginTestModule::ShutdownModule()
{
	UE_LOG(PluginTestModuleLog, Warning, TEXT("FPluginTestModule module has shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FPluginTestModule, TestModule);
